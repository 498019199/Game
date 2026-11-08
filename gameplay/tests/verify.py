#!/usr/bin/env python3
"""Protocol replay against C++ hosting, C# server, native TCP and optional Docker TCP."""
import argparse
import hashlib
import os
from pathlib import Path
import socket
import struct
import subprocess
import time

parser = argparse.ArgumentParser()
parser.add_argument('--client', required=True)
parser.add_argument('--managed', required=True)
parser.add_argument('--dotnet', required=True)
parser.add_argument('--server', required=True)
parser.add_argument('--tests', required=True)
parser.add_argument('--work', required=True)
parser.add_argument('--endpoint', help='Also replay against an already running Docker TCP endpoint host:port')
a = parser.parse_args()
work = Path(a.work); work.mkdir(parents=True, exist_ok=True)
env = dict(os.environ, GAMEPLAY_MANAGED_DIR=a.managed)

def run(command):
    result = subprocess.run(command, env=env, text=True, capture_output=True, timeout=60)
    if result.returncode:
        raise AssertionError(f'{command}\n{result.stdout}\n{result.stderr}')
    if result.stdout.strip(): print(result.stdout.strip())
    return result

def frames(path):
    data = Path(path).read_bytes(); result = []
    while data:
        if len(data) < 4: raise AssertionError('Truncated output header')
        size, = struct.unpack('!I', data[:4])
        if not 0 < size <= 4 * 1024 * 1024 or len(data) < 4 + size:
            raise AssertionError('Invalid output frame')
        result.append(data[4:4+size]); data = data[4+size:]
    return result

def exact(sock, size):
    result = b''
    while len(result) < size:
        chunk = sock.recv(size - len(result))
        if not chunk: raise AssertionError('Connection closed inside a frame')
        result += chunk
    return result

def receive(sock):
    size, = struct.unpack('!I', exact(sock, 4))
    if not 0 < size <= 4 * 1024 * 1024: raise AssertionError('Invalid TCP response size')
    return exact(sock, size)

def tcp_test(host, port, requests, expected):
    # Two interleaved connections create the same entity IDs. Their rooms must
    # remain independent. Fragmented headers/bodies exercise real stream framing.
    with socket.create_connection((host, port), timeout=10) as one, socket.create_connection((host, port), timeout=10) as two:
        for index, (request, response) in enumerate(zip(requests, expected)):
            packet = struct.pack('!I', len(request)) + request
            for connection in (one, two):
                if index == 0:
                    for piece in (packet[:1], packet[1:3], packet[3:5], packet[5:]):
                        connection.sendall(piece)
                else:
                    connection.sendall(packet)
                if receive(connection) != response:
                    raise AssertionError(f'TCP response mismatch at {index}')
    # Oversized frame is rejected before allocating/processing it; the server
    # must still accept a fresh valid connection afterwards.
    with socket.create_connection((host, port), timeout=10) as bad:
        bad.sendall(struct.pack('!I', 4 * 1024 * 1024 + 1))
        try:
            assert bad.recv(1) == b''
        except ConnectionResetError:
            pass
    with socket.create_connection((host, port), timeout=10) as fresh:
        fresh.sendall(struct.pack('!I', len(requests[0])) + requests[0])
        assert receive(fresh) == expected[0]
    print(f'PASS TCP {host}:{port}: {len(requests)} requests x 2 isolated rooms, fragmentation, frame limit and reconnect')

run([a.dotnet, a.tests, str(Path(a.managed)/'game.catalog.pb'), str(work)])
for name, command in [('client', [a.client]), ('server', [a.dotnet, a.server])]:
    run(command + ['--replay', str(work/'requests.pb'), str(work/(name+'.pb'))])
    assert (work/(name+'.pb')).read_bytes() == (work/'expected.pb').read_bytes(), name+' response stream mismatch'
    print('PASS '+name+' protobuf snapshots/events match independent assertions')
run([a.client, '--check-isolation'])
for filename in ('Game.Combat.dll', 'Game.Protocol.dll', 'Game.Runtime.dll', 'game.catalog.pb'):
    native = (Path(a.managed)/filename).read_bytes()
    server = (Path(a.server).parent/filename).read_bytes()
    assert hashlib.sha256(native).digest() == hashlib.sha256(server).digest(), filename
print('PASS identical gameplay, protocol, runtime assemblies and catalog')
requests = frames(work/'requests.pb'); expected = frames(work/'expected.pb')
assert len(requests) == len(expected)
log = open(work/'server-stderr.log', 'w')
process = subprocess.Popen([a.dotnet, a.server, '--listen', '127.0.0.1', '0'], env=env,
                           stdout=subprocess.PIPE, stderr=log, text=True)
try:
    # Bound the startup wait without blocking indefinitely on readline.
    import selectors
    with selectors.DefaultSelector() as selector:
        selector.register(process.stdout, selectors.EVENT_READ)
        if not selector.select(15): raise AssertionError('Server startup timed out')
    line = process.stdout.readline().strip()
    assert line.startswith('LISTENING '), line
    host, port = line[len('LISTENING '):].rsplit(':', 1)
    tcp_test(host, int(port), requests, expected)
finally:
    process.terminate()
    try: process.wait(timeout=10)
    except subprocess.TimeoutExpired:
        process.kill(); process.wait(); raise AssertionError('Server did not stop gracefully')
    log.close()
    assert process.returncode == 0, f'Server shutdown exit code: {process.returncode}'
if a.endpoint:
    host, port = a.endpoint.rsplit(':', 1)
    tcp_test(host, int(port), requests, expected)
print('ALL PROTOCOL MIGRATION TESTS PASSED')
