using System.ComponentModel;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace LoWeExposer.Handlers
{
    abstract class HandlerBase
    {
        public int Port { get; protected set; }
        private BackgroundWorker _backgroundWorker;
        private CancellationTokenSource _cancellationTokenSource;
        protected CancellationToken _cancellationToken;
        protected TcpListener _tcpListener;

        protected HandlerBase(int port = 0)
        {
            Port = port;
        }

        public void Start()
        {
            _cancellationTokenSource = new CancellationTokenSource();
            _cancellationToken = _cancellationTokenSource.Token;

            _tcpListener = new TcpListener(IPAddress.Parse("127.0.0.1"), Port);
            _tcpListener.Start();
            Port = ((IPEndPoint)_tcpListener.LocalEndpoint).Port;

            _backgroundWorker = new BackgroundWorker();
            _backgroundWorker.DoWork += backgroundWorker_DoWork;
            _backgroundWorker.RunWorkerAsync();

        }

        public void Stop()
        {
            _cancellationTokenSource.Cancel();
            _tcpListener.Stop();
            Port = 0;
        }

        protected abstract void backgroundWorker_DoWork(object sender, DoWorkEventArgs e);

        protected bool IsOperation(byte[] buffer, string chars)
        {
            if (buffer.Length != chars.Length)
                return false;

            for (var i = 0; i < buffer.Length; i++)
            {
                if (buffer[i] != chars[i])
                    return false;
            }
            return true;
        }

        protected bool ReadAll(NetworkStream networkStream, byte[] buffer)
        {
            if (!networkStream.CanRead)
                return false;

            try
            {
                int offset = 0;
                int count = buffer.Length;

                while (count > 0)
                {
                    int size = networkStream.Read(buffer, offset, count);
                    if (size < 1)
                        return false;
                    offset += size;
                    count -= size;
                }
                return true;
            }
            catch (IOException)
            {
                return false;
            }
        }

        protected void WriteAll(NetworkStream networkStream, byte[] data)
        {
            networkStream.Write(data, 0, data.Length);
        }

    }
}
