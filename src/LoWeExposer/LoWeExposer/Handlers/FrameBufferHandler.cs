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
        private int _stride;
        private int _size;
        private byte[] _data;
        private BinaryReader _binaryReader;
        private WriteableBitmap _writeableBitmap;
        private Action<WriteableBitmap> _update;

        public FrameBufferHandler(int width, int height, int bytesPerPixel)
        {
            _width = width;
            _height = height;
            _bytesPerPixel = bytesPerPixel;
        }

        public bool Initialize(Action<WriteableBitmap> update)
        {
            _update = update;
            _stride = _width * _bytesPerPixel;
            _size = _width * _height * _bytesPerPixel;

            var path = GuessPath();

            if (path == null)
                return false;

            FileStream fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
            _data = new byte[_size];
            _binaryReader = new BinaryReader(fs);

            var bitmapSource = BitmapSource.Create(_width, _height, 0, 0, PixelFormats.Bgr32, null, _data, _stride);
            _writeableBitmap = new WriteableBitmap(bitmapSource);
            var dispatcherTimer = new DispatcherTimer();
            dispatcherTimer.Tick += DispatcherTimer_Tick;
            dispatcherTimer.Interval = new TimeSpan(0, 0, 0, 0, 40);
            dispatcherTimer.Start();

            return true;
        }

        private string GuessPath()
        {
            var localAppData = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);

            var lxssTemp = Path.Combine(localAppData, @"lxss\temp");

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

            _update(_writeableBitmap);
        }
    }
}
