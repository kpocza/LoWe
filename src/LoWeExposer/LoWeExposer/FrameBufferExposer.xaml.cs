using System;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media.Imaging;
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
        private IntPtr _hKl;
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

        #region Graphics

        private void FrameBufferExposer_Loaded(object sender, EventArgs e)
        {
            if (!_initialized)
            {
                _frameBufferHandler = new FrameBufferHandler(1280, 720, 4, this);
                _frameBufferHandler.Initialize();
                _initialized = true;

                this.KeyDown += FrameBufferExposer_KeyDown;
                this.KeyUp += FrameBufferExposer_KeyUp;
            }
        }

        private void FrameBufferExposer_OnClosing(object sender, CancelEventArgs e)
        {
            _frameBufferHandler.Stop();
        }

        public void Update(WriteableBitmap writeableBitmap)
        {
            this.capture.Source = null;
            this.capture.Source = writeableBitmap;
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

            if (!_mouseEnabled || !_mouseCaptured)
                return;

            _miceState.LeftButtonDown = false;
            EnqueueState();
        }

        private void Capture_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (!_mouseEnabled || !_mouseCaptured)
                return;

            _miceState.LeftButtonDown = true;
            EnqueueState();
        }

        private void Capture_MouseRightButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (!_mouseEnabled || !_mouseCaptured)
                return;

            _miceState.RightButtonDown = true;
            EnqueueState();
        }

        private void Capture_MouseRightButtonUp(object sender, MouseButtonEventArgs e)
        {
            if (!_mouseEnabled || !_mouseCaptured)
                return;

            _miceState.RightButtonDown = false;
            EnqueueState();
        }

        private void FrameBufferExposer_MouseMove(object sender, MouseEventArgs e)
        {
            if (!_mouseEnabled || !_mouseCaptured)
                return;

            var pos = e.GetPosition(this.capture);
            _lastPosition = pos;

            _miceState.X = (int) (1280.0*pos.X/capture.RenderSize.Width);
            _miceState.Y = (int) (720.0*pos.Y/capture.RenderSize.Height);
            EnqueueState();
        }

        private void Capture_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            if (!_mouseEnabled || !_mouseCaptured)
                return;

            _miceState.Wheel = e.Delta;
            EnqueueState();
            _miceState.Wheel = 0;
        }

        [DllImport("User32.dll")]
        private static extern bool SetCursorPos(int X, int Y);

        private void Capture_MouseLeave(object sender, MouseEventArgs e)
        {
            if (!_mouseEnabled || !_mouseCaptured)
                return;

            SetCursor(_lastPosition.X, _lastPosition.Y);
        }

        private void SetCursor(double x, double y)
        {
            var capturePoint = capture.PointToScreen(new Point(0, 0));
            SetCursorPos((int) (x + capturePoint.X), (int) (y + capturePoint.Y));
        }

        private void EnqueueState()
        {
            _miceExposer.AddState(_miceState.Clone());
        }

        #endregion

        #region Keyboard support

        public void EnableKeyboardSupport(KbdExposer kbdExposer)
        {
            _kbdExposer = kbdExposer;
            _kbdEnabled = true;
            _hKl = GetKeyboardLayout(0);
        }

        private void FrameBufferExposer_KeyDown(object sender, KeyEventArgs e)
        {
            if (!_kbdEnabled)
                return;

            var scanCode = ResolveScanCode(e);
            _kbdExposer.KeyDownPerformed((byte)scanCode);
        }

        private void FrameBufferExposer_KeyUp(object sender, KeyEventArgs e)
        {
            if (Keyboard.IsKeyDown(Key.LeftAlt) && e.Key == Key.System && e.SystemKey == Key.F12)
            {
                SwitchWindowMode();
            }

            if (!_kbdEnabled)
                return;

            var scanCode = ResolveScanCode(e);
            _kbdExposer.KeyUpPerformed((byte)scanCode);
        }

        private uint ResolveScanCode(KeyEventArgs e)
        {
            var virtualKey = (uint)KeyInterop.VirtualKeyFromKey(e.Key != Key.System ? e.Key : e.SystemKey);
            return MapVirtualKeyEx(virtualKey, 4, _hKl);
        }

        [DllImport("user32.dll")]
        static extern IntPtr GetKeyboardLayout(uint idThread);

        [DllImport("user32.dll")]
        static extern uint MapVirtualKeyEx(uint uCode, uint uMapType, IntPtr dwhkl);

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
    }
}

