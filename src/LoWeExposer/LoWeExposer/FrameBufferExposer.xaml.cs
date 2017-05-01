using System;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Threading;
using LoWeExposer.Handlers;

namespace LoWeExposer
{
    public partial class FrameBufferExposer : Window, IFramebufferDrawer
    {
        private bool _initialized;
        private FrameBufferHandler _frameBufferHandler;

        private bool _mouseEnabled;
        private MiceExposer _miceExposer;
        private bool _kbdEnabled;
        private KbdExposer _kbdExposer;
        private bool _mouseCaptured;
        private Point _lastPosition;
        private readonly MiceState _miceState;
        private bool _isFullScreen;

        public FrameBufferExposer()
        {
            InitializeComponent();
            _initialized = false;
            _mouseEnabled = false;
            _mouseCaptured = false;
            _miceState = new MiceState();
            _kbdEnabled = false;
            _isFullScreen = false;
        }

        public int Port => _frameBufferHandler.Port;

        #region Graphics

        private void FrameBufferExposer_Loaded(object sender, EventArgs e)
        {
            if (!_initialized)
            {
                _initialized = true;
                _frameBufferHandler = new FrameBufferHandler(this);
                _frameBufferHandler.Start();

                this.KeyDown += FrameBufferExposer_KeyDown;
                this.KeyUp += FrameBufferExposer_KeyUp;
            }
        }

        private void FrameBufferExposer_OnClosing(object sender, CancelEventArgs e)
        {
            _frameBufferHandler.Stop();
        }

        private int _stride;
        private int _size;
        private byte[] _data;
        private WriteableBitmap _writeableBitmap;
        private DispatcherTimer _dispatcherTimer;

        private int _width;
        private int _height;
        private int _bytesPerPixel;

        public void Initialize(int width, int height, int bytesPerPixel)
        {
            Dispatcher.Invoke(() =>
            {
                _width = width;
                _height = height;
                _bytesPerPixel = bytesPerPixel;

                _stride = _width*_bytesPerPixel;
                _size = _width*_height*_bytesPerPixel;

                _data = new byte[_size];

                var bitmapSource = BitmapSource.Create(_width, _height, 0, 0, PixelFormats.Bgr32, null, _data, _stride);
                _writeableBitmap = new WriteableBitmap(bitmapSource);

                _dispatcherTimer = new DispatcherTimer();
                _dispatcherTimer.Tick += DispatcherTimer_Tick;
                _dispatcherTimer.Interval = new TimeSpan(0, 0, 0, 0, 20);
                _dispatcherTimer.Start();
            });
        }

        public byte[] Data => _data;

        public void Stop()
        {
            Dispatcher.Invoke(() => {
                _dispatcherTimer?.Stop();
            });
        }

        private void DispatcherTimer_Tick(object sender, EventArgs e)
        {
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

            this.capture.Source = null;
            this.capture.Source = _writeableBitmap;
        }


        protected override void OnRenderSizeChanged(SizeChangedInfo sizeInfo)
        {
            float aspect = (float)16 / 9;
            double captionHeight = SystemParameters.WindowCaptionHeight;

            if (sizeInfo.WidthChanged)
                Width = (sizeInfo.NewSize.Height - captionHeight) * aspect;
            else
                Height = sizeInfo.NewSize.Width / aspect + captionHeight;
        }

        #endregion

        #region Mouse support

        public void EnableMouseSupport(MiceExposer miceExposer)
        {
            _miceExposer = miceExposer;
            _mouseEnabled = true;

            this.MouseMove += FrameBufferExposer_MouseMove;
            this.capture.MouseLeftButtonDown += Capture_MouseLeftButtonDown;
            this.capture.MouseLeftButtonUp += Capture_MouseLeftButtonUp;
            this.capture.MouseRightButtonDown += Capture_MouseRightButtonDown;
            this.capture.MouseRightButtonUp += Capture_MouseRightButtonUp;
            this.capture.MouseWheel += Capture_MouseWheel;
            this.capture.MouseLeave += Capture_MouseLeave;
        }

        private void Capture_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            if (Keyboard.IsKeyDown(Key.LeftCtrl) && Keyboard.IsKeyDown(Key.LeftAlt))
            {
                _mouseCaptured = !_mouseCaptured;

                if (_mouseCaptured)
                {
                    _miceState.Reset();
                    _miceExposer.ClearQueue();
                }
                return;
            }

            if (IsMouseEnabledCaptured)
            {
                _miceState.LeftButtonDown = false;
                EnqueueMouseState();
            }
        }

        private void Capture_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (IsMouseEnabledCaptured)
            {
                _miceState.LeftButtonDown = true;
                EnqueueMouseState();
            }
        }

        private void Capture_MouseRightButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (IsMouseEnabledCaptured)
            {
                _miceState.RightButtonDown = true;
                EnqueueMouseState();
            }
        }

        private void Capture_MouseRightButtonUp(object sender, MouseButtonEventArgs e)
        {
            if (!_mouseCaptured)
            {
                this.contextMenu.IsOpen = true;
                this.contextMenu.Visibility = Visibility.Visible;
                return;
            }

            if (IsMouseEnabledCaptured)
            {
                _miceState.RightButtonDown = false;
                EnqueueMouseState();
            }
        }

        private void FrameBufferExposer_MouseMove(object sender, MouseEventArgs e)
        {
            if (IsMouseEnabledCaptured)
            {
                var pos = e.GetPosition(this.capture);
                _lastPosition = pos;

                _miceState.X = (int) (1280.0*pos.X/capture.RenderSize.Width);
                _miceState.Y = (int) (720.0*pos.Y/capture.RenderSize.Height);
                EnqueueMouseState();
            }
        }

        private void Capture_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            if (IsMouseEnabledCaptured)
            {
                _miceState.Wheel = e.Delta;
                EnqueueMouseState();
                _miceState.Wheel = 0;
            }
        }

        [DllImport("User32.dll")]
        private static extern bool SetCursorPos(int x, int y);

        private void Capture_MouseLeave(object sender, MouseEventArgs e)
        {
            if (IsMouseEnabledCaptured)
            {
                SetCursor(_lastPosition.X, _lastPosition.Y);
            }
        }

        private void SetCursor(double x, double y)
        {
            var capturePoint = capture.PointToScreen(new Point(0, 0));
            SetCursorPos((int) (x + capturePoint.X), (int) (y + capturePoint.Y));
        }

        private void EnqueueMouseState()
        {
            _miceExposer.AddState(_miceState.Clone());
        }

        private bool IsMouseEnabledCaptured => _mouseEnabled && _mouseCaptured;

        #endregion

        #region Keyboard support

        public void EnableKeyboardSupport(KbdExposer kbdExposer)
        {
            _kbdExposer = kbdExposer;
            _kbdEnabled = true;
        }

        private void FrameBufferExposer_KeyDown(object sender, KeyEventArgs e)
        {
            if (_kbdEnabled)
            {
                _kbdExposer.KeyDownPerformed(e);
            }
        }

        private void FrameBufferExposer_KeyUp(object sender, KeyEventArgs e)
        {
            if (Keyboard.IsKeyDown(Key.LeftAlt) && e.Key == Key.System && e.SystemKey == Key.F12)
            {
                SwitchWindowMode();
            }

            if (_kbdEnabled)
            {
                _kbdExposer.KeyUpPerformed(e);
            }
        }

        #endregion

        #region Window resize

        private void SwitchWindowMode()
        {
            _isFullScreen = !_isFullScreen;

            if(_isFullScreen)
                Fullscreen();
            else
                NormalSize();
        }

        private void NormalSize()
        {
            Topmost = false;
            WindowState = WindowState.Normal;
            ResizeMode = ResizeMode.CanResize;
            WindowStyle = WindowStyle.SingleBorderWindow;
        }

        private void Fullscreen()
        {
            Topmost = true;
            ResizeMode = ResizeMode.NoResize;
            WindowStyle = WindowStyle.None;
            WindowState = WindowState.Maximized;
        }

        #endregion

        #region Context menu

        private void ContextMenuPaste_OnClick(object sender, RoutedEventArgs e)
        {
            if (!_mouseCaptured && _kbdEnabled)
            {
                var text = Clipboard.GetText();
                if (string.IsNullOrEmpty(text))
                    return;

                if (text.Length > 1000)
                {
                    MessageBox.Show("Text is too long on the clipboard to paste", "LoWe");
                    return;
                }

                foreach (var keyEvtArg in String2KeyEventConverter.ToEvents(text))
                {
                    int tryCount = 0;
                    while (_kbdExposer.QueueLength > 20)
                    {
                        System.Threading.Thread.Sleep(10);
                        tryCount++;

                        if (tryCount > 200)
                            return;
                    }
                    InputManager.Current.ProcessInput(keyEvtArg);
                }
            }
        }

        #endregion
    }
}

