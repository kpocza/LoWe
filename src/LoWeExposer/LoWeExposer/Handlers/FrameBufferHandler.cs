using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Threading;

namespace LoWeExposer.Handlers
{
    class FrameBufferHandler
    {
        private readonly int _width;
        private readonly int _height;
        private readonly int _bytesPerPixel;
        private readonly IFramebufferDrawer _framebufferDrawer;
        private int _stride;
        private int _size;
        private byte[] _data;
        private BinaryReader _binaryReader;
        private WriteableBitmap _writeableBitmap;
        private DispatcherTimer _dispatcherTimer;
        private bool _hasFilePath;

        public FrameBufferHandler(int width, int height, int bytesPerPixel, IFramebufferDrawer framebufferDrawer)
        {
            _width = width;
            _height = height;
            _bytesPerPixel = bytesPerPixel;
            _framebufferDrawer = framebufferDrawer;
            _hasFilePath = false;
        }

        public bool Initialize()
        {
            _stride = _width * _bytesPerPixel;
            _size = _width * _height * _bytesPerPixel;

            _data = new byte[_size];

            var bitmapSource = BitmapSource.Create(_width, _height, 0, 0, PixelFormats.Bgr32, null, _data, _stride);
            _writeableBitmap = new WriteableBitmap(bitmapSource);
            _dispatcherTimer = new DispatcherTimer();
            _dispatcherTimer.Tick += DispatcherTimer_Tick;
            _dispatcherTimer.Interval = new TimeSpan(0, 0, 0, 0, 40);
            _dispatcherTimer.Start();

            return true;
        }

        public void Stop()
        {
            _dispatcherTimer?.Stop();
        }

        private string GuessPath()
        {
            var localAppData = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);

            var lxssTemp = Path.Combine(localAppData, @"lxss\temp");

            if (!Directory.Exists(lxssTemp))
                return null;

            var di = new DirectoryInfo(lxssTemp);
            foreach (var tmp in di.GetDirectories("*_tmpfs"))
            {
                var filePath = Path.Combine(tmp.FullName, "fb0");
                if (File.Exists(filePath))
                {
                    return filePath;
                }
            }

            return null;
        }

        private void DispatcherTimer_Tick(object sender, EventArgs e)
        {
            if (!_hasFilePath)
            {
                var path = GuessPath();

                if (path == null)
                    return;

                FileStream fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
                _binaryReader = new BinaryReader(fs);
                _hasFilePath = true;
            }

            _binaryReader.BaseStream.Seek(0, SeekOrigin.Begin);
            _binaryReader.Read(_data, 0, _size);

            try
            {
                _writeableBitmap.Lock();

                Marshal.Copy(_data, 0, _writeableBitmap.BackBuffer, _size);
                _writeableBitmap.AddDirtyRect(new Int32Rect(0, 0, _width, _height));
            }
            finally
            {
                _writeableBitmap.Unlock();
            }

            _framebufferDrawer.Update(_writeableBitmap);
        }
    }
}
