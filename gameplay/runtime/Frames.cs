using System;
using System.Buffers.Binary;
using System.IO;

namespace Game.Runtime;

public static class Frames
{
    // TCP/replay framing: 4-byte unsigned big-endian length, then protobuf bytes.
    public static byte[]? Read(Stream stream)
    {
        byte[] header = new byte[4];
        int first = stream.ReadByte();
        if (first < 0) return null;
        header[0] = (byte)first;
        stream.ReadExactly(header.AsSpan(1));
        uint length = BinaryPrimitives.ReadUInt32BigEndian(header);
        if (length == 0 || length > ProtocolSession.MaxFrameBytes) throw new InvalidDataException("Invalid frame length");
        byte[] body = new byte[length];
        stream.ReadExactly(body);
        return body;
    }
    public static void Write(Stream stream, byte[] body)
    {
        if (body.Length == 0 || body.Length > ProtocolSession.MaxFrameBytes) throw new InvalidDataException("Invalid frame length");
        Span<byte> header = stackalloc byte[4];
        BinaryPrimitives.WriteUInt32BigEndian(header, (uint)body.Length);
        stream.Write(header); stream.Write(body); stream.Flush();
    }
}
