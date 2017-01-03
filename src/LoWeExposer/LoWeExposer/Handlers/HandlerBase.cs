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
        protected Socket _socket;

        protected HandlerBase(int port = 0)
        {
            Port = port;
        }

        public void Start()
        {
            _cancellationTokenSource = new CancellationTokenSource();
            _cancellationToken = _cancellationTokenSource.Token;

            _tcpListener = new TcpListener(IPAddress.Parse("127.0.0.1"), Port);
            _tcpListener.Server.NoDelay = true;

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

        protected bool ReadAllImpatient(byte[] buffer)
        {
            return ReadAll(buffer, 10*1000);
        }

        protected bool ReadAllPatient(byte[] buffer)
        {
            return ReadAll(buffer, 1000*1000);
        }

        private bool ReadAll(byte[] buffer, int timeout)
        {
            if (!_socket.Poll(timeout, SelectMode.SelectRead))
                return false;

            if (_socket.Available == 0)
            {
                Thread.Sleep(10);
                return false;
            }

            try
            {
                int offset = 0;
                int count = buffer.Length;

                while (count > 0)
                {
                    int size = _socket.Receive(buffer, offset, count, SocketFlags.None);
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

        protected void WriteAll(byte[] buffer)
        {
            int offset = 0;
            int count = buffer.Length;

            while (count > 0)
            {
                int size = _socket.Send(buffer, offset, count, SocketFlags.None);

                offset += size;
                count -= size;
            }
        }
    }
}
