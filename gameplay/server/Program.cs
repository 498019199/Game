using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;
using Game.Combat;
using Game.Protocol;
using Game.Runtime;
using Google.Protobuf;

try
{
    if (args.Length == 3 && args[0] == "--bake")
    {
        File.WriteAllBytes(args[2], CatalogLoader.FromJsonDirectory(args[1]).ToByteArray());
        Console.WriteLine("Baked " + args[2]); return 0;
    }
    GameCatalog catalog = CatalogLoader.Read(Path.Combine(AppContext.BaseDirectory, "game.catalog.pb"));
    if (args.Length == 0 || (args.Length == 1 && args[0] == "--smoke"))
    {
        SmokeChecks.Run(catalog, false); SmokeChecks.Run(catalog, true);
        Console.WriteLine("C# GAS smoke + skill timeline: OK"); return 0;
    }
    if (args.Length == 3 && args[0] == "--replay")
    {
        var session = new ProtocolSession(catalog);
        using var input = File.OpenRead(args[1]);
        using var output = File.Create(args[2]);
        byte[]? frame;
        while ((frame = Frames.Read(input)) != null) Frames.Write(output, session.Exchange(frame));
        return 0;
    }
    if (args.Length == 3 && args[0] == "--listen")
    {
        var listener = new TcpListener(IPAddress.Parse(args[1]), int.Parse(args[2]));
        using var stop = new CancellationTokenSource();
        Console.CancelKeyPress += (_, e) => { e.Cancel = true; stop.Cancel(); };
        using var shutdown = OperatingSystem.IsWindows() ? null : System.Runtime.InteropServices.PosixSignalRegistration.Create(
            System.Runtime.InteropServices.PosixSignal.SIGTERM, context => { context.Cancel = true; stop.Cancel(); });
        listener.Start();
        Console.WriteLine("LISTENING " + listener.LocalEndpoint);
        Console.Out.Flush();
        var clients = new List<Task>();
        try
        {
            while (!stop.IsCancellationRequested)
            {
                TcpClient client = await listener.AcceptTcpClientAsync(stop.Token);
                clients.RemoveAll(task => task.IsCompleted);
                if (clients.Count >= 64) { client.Dispose(); continue; }
                clients.Add(Task.Run(() => Serve(client, catalog, stop.Token)));
            }
        }
        catch (OperationCanceledException) { }
        finally { listener.Stop(); stop.Cancel(); await Task.WhenAll(clients); }
        return 0;
    }
    throw new ArgumentException("Usage: Game.Server [--smoke | --bake JSON_DIR OUTPUT.pb | --replay INPUT OUTPUT | --listen IP PORT]");
}
catch (Exception error)
{
    Console.Error.WriteLine(error.Message); return 2;
}

static void Serve(TcpClient client, GameCatalog catalog, CancellationToken stop)
{
    using (client)
    using (stop.Register(client.Dispose))
    {
        try
        {
            client.NoDelay = true;
            client.ReceiveTimeout = 30000; client.SendTimeout = 30000;
            using NetworkStream stream = client.GetStream();
            var session = new ProtocolSession(catalog); // independent development room per connection
            byte[]? frame;
            while (!stop.IsCancellationRequested && (frame = Frames.Read(stream)) != null)
                Frames.Write(stream, session.Exchange(frame));
        }
        catch (Exception error) when (error is IOException || error is SocketException || error is ObjectDisposedException || error is InvalidDataException)
        { Console.Error.WriteLine("Connection closed: " + error.Message); }
    }
}
