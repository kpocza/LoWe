using System;
using System.ComponentModel;
using System.Text;

namespace LoWeExposer.Handlers
{
    internal class FrameBufferHandler : HandlerBase
    {
        private readonly IFramebufferDrawer _framebufferDrawer;

        public FrameBufferHandler(IFramebufferDrawer framebufferDrawer)
        {
            _framebufferDrawer = framebufferDrawer;
        }

        protected override void backgroundWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            try
            {
                while (!_cancellationToken.IsCancellationRequested)
                {
                    _socket = _tcpListener.AcceptSocket();
                    _socket.NoDelay = true;

                    while (!_cancellationToken.IsCancellationRequested && !_tcpListener.Pending() && _socket.Connected)
                    {
                        var opCode = new byte[4];
                        if (!ReadAllImpatient(opCode))
                            continue;

                        if (IsOperation(opCode, "FBUF"))
                        {
                            WriteAll(Encoding.ASCII.GetBytes("FUBF"));
                            break;
                        }

                        if (IsOperation(opCode, "INIT"))
                        {
                            var initData = new byte[3*4];
                            if (!ReadAllPatient(initData))
                                break;

                            var width = BitConverter.ToInt32(initData, 0);
                            var height = BitConverter.ToInt32(initData, 4);
                            var bytesPerPixel = BitConverter.ToInt32(initData, 8)/8;

                            _framebufferDrawer.Initialize(width, height, bytesPerPixel);
                        }
                        else if (IsOperation(opCode, "FRAM"))
                        {
                            if (!ReadAllPatient(_framebufferDrawer.Data))
                                break;
                        }
                    }

                    _socket.Close();
                    _framebufferDrawer.Stop();
                }
            }
            catch
            {
                _tcpListener.Stop();
            }
        }
    }
}
