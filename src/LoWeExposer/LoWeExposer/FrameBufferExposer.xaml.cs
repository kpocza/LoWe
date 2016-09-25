using System;
using System.Windows;
using System.Windows.Media.Imaging;
using LoWeExposer.Handlers;

namespace LoWeExposer
{
    public partial class FrameBufferExposer : Window
    {
        private bool _initialized;
        private FrameBufferHandler _frameBufferHandler;

        public FrameBufferExposer()
        {
            InitializeComponent();
            _initialized = false;
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
    }
}

