using System;
using Game.Combat;
using Game.Protocol;
using Google.Protobuf;

namespace Game.Runtime;

public sealed class ProtocolSession
{
    public const int MaxFrameBytes = 4 * 1024 * 1024;
    private readonly object gate = new object();
    private readonly GameWorld world;
    private bool closed;
    public void Close() { lock (gate) { closed = true; } }
    public ProtocolSession(GameCatalog catalog) { world = new GameWorld(catalog); }
    public byte[] Exchange(byte[] input)
    {
        lock (gate)
        {
            if (closed) throw new ObjectDisposedException(nameof(ProtocolSession));
            Response response;
            if (input.Length > MaxFrameBytes) throw new ArgumentException("Frame too large");
            try { response = world.Handle(Request.Parser.ParseFrom(input)); }
            catch (InvalidProtocolBufferException)
            { response = new Response { Version = 1, Status = ErrorCode.InvalidArgument, Error = "Malformed protobuf request" }; }
            return response.ToByteArray();
        }
    }
}
