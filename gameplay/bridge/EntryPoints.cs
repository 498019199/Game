using System;
using System.Collections.Concurrent;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Threading;
using Game.Combat;
using Game.Protocol;
using Game.Runtime;

namespace Game.NativeBridge;

// ABI transports opaque handles and protobuf bytes only; no gameplay structs.
public static unsafe class EntryPoints
{
    private static readonly ConcurrentDictionary<long, ProtocolSession> Sessions = new();
    private static long nextSession;

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    public static long Create(byte* catalogBytes, int length)
    {
        if (catalogBytes == null || length <= 0 || length > ProtocolSession.MaxFrameBytes) return 0;
        try
        {
            Catalog catalog = Catalog.Parser.ParseFrom(new ReadOnlySpan<byte>(catalogBytes, length));
            var session = new ProtocolSession(new GameCatalog(catalog));
            long id = Interlocked.Increment(ref nextSession);
            return Sessions.TryAdd(id, session) ? id : 0;
        }
        catch (Exception) { return 0; }
    }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    public static int Exchange(long sessionId, byte* input, int length, byte** output, int* outputLength)
    {
        if (output == null || outputLength == null) return -1;
        *output = null; *outputLength = 0;
        if (input == null || length <= 0 || length > ProtocolSession.MaxFrameBytes) return -1;
        if (!Sessions.TryGetValue(sessionId, out var session)) return -2;
        try
        {
            byte[] response = session.Exchange(new ReadOnlySpan<byte>(input, length).ToArray());
            if (response.Length > ProtocolSession.MaxFrameBytes) return -3;
            byte* allocated = (byte*)NativeMemory.Alloc((nuint)response.Length);
            if (allocated == null) return -4;
            response.CopyTo(new Span<byte>(allocated, response.Length));
            *output = allocated; *outputLength = response.Length;
            return 0;
        }
        catch (Exception) { return -4; } // Never unwind across the native ABI.
    }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    public static void Release(byte* output) { NativeMemory.Free(output); }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    public static int Destroy(long id)
    {
        if (!Sessions.TryRemove(id, out var session)) return -2;
        session.Close();
        return 0;
    }
}
