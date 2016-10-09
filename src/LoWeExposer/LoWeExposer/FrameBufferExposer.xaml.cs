using System;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media.Imaging;
using LoWeExposer.Handlers;

namespace LoWeExposer
{
    public partial class FrameBufferExposer : Window
    {
        private bool _initialized;
        private FrameBufferHandler _frameBufferHandler;

        private bool _mouseEnabled;
        private MiceExposer _miceExposer;
        private bool _mouseCaptured;
        private Point _lastPosition;
        private MiceState _miceState;

        public FrameBufferExposer()
        {
            InitializeComponent();
            _initialized = false;
            _mouseEnabled = false;
            _mouseCaptured = false;
            _miceState = new MiceState();
        }

        private void FrameBufferExposer_Loaded(object sender, EventArgs e)
        {
            if (!_initialized)
            {
                try
                {
                    _frameBufferHandler = new FrameBufferHandler(1280, 720, 4);

                    if (!_frameBufferHandler.Initialize(Update))
                    {
                        MessageBox.Show("Initialization error");
                        Close();
                        return;
                    }
                }
                catch(Exception ex)
                {
                    MessageBox.Show("Initialization error: " + ex);
                    Close();
                    return;
                }
                _initialized = true;
            }
        }

        private void Update(WriteableBitmap writeableBitmap)
        {
            this.capture.Source = null;
            this.capture.Source = writeableBitmap;
        }

        public void EnableMouseSupport(MiceExposer miceExposer)
        {
            _miceExposer = miceExposer;
            _mouseEnabled = true;

            this.MouseMove += FrameBufferExposer_MouseMove;
            this.capture.MouseLeftButtonDown += Capture_MouseLeftButtonDown;
            this.capture.MouseLeftButtonUp += Capture_MouseLeftButtonUp;
            this.capture.MouseRightButtonDown += Capture_MouseRightButtonDown;
            this.capture.MouseRightButtonUp += Capture_MouseRightButtonUp;
            this.capture.MouseLeave += Capture_MouseLeave;
        }

        private void Capture_MouseLeftButtonUp(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            if (Keyboard.IsKeyDown(Key.LeftCtrl) && Keyboard.IsKeyDown(Key.LeftAlt))
            {
                _mouseCaptured = !_mouseCaptured;

                if (_mouseCaptured)
                {
                    _miceState.Reset();
                }
                
                return;
            }

            if (!_mouseEnabled || !_mouseCaptured)
                return;

            _miceState.LeftButtonDown = false;
            _miceExposer.SetState(_miceState);
        }

        private void Capture_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (!_mouseEnabled || !_mouseCaptured)
                return;

            _miceState.LeftButtonDown = true;
            _miceExposer.SetState(_miceState);
        }

        private void Capture_MouseRightButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (!_mouseEnabled || !_mouseCaptured)
                return;

            _miceState.RightButtonDown = true;
            _miceExposer.SetState(_miceState);
        }

        private void Capture_MouseRightButtonUp(object sender, MouseButtonEventArgs e)
        {
            if (!_mouseEnabled || !_mouseCaptured)
                return;

            _miceState.RightButtonDown = false;
            _miceExposer.SetState(_miceState);
        }

        private void FrameBufferExposer_MouseMove(object sender, MouseEventArgs e)
        {
            if (!_mouseEnabled || !_mouseCaptured)
                return;

            var pos = e.GetPosition(this.capture);
            _lastPosition = pos;

            _miceState.X = (int) (1280.0*pos.X/capture.RenderSize.Width);
            _miceState.Y = (int) (720.0*pos.Y/capture.RenderSize.Height);
            _miceExposer.SetState(_miceState);
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
            SetCursorPos((int)(x + capturePoint.X), (int)(y + capturePoint.Y));
        }
    }
}

