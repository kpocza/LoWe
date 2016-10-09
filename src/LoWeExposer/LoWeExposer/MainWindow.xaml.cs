using System.Windows;

namespace LoWeExposer
{
    public partial class MainWindow : Window
    {
        private FrameBufferExposer _frameBufferExposer;

        public MainWindow()
        {
            InitializeComponent();
        }

        private void btnFrameBuffer_Click(object sender, RoutedEventArgs e)
        {
            _frameBufferExposer = new FrameBufferExposer();
            _frameBufferExposer.Show();
            _frameBufferExposer.Closed += _frameBufferExposer_Closed;
        }

        private void btnALSA_Click(object sender, RoutedEventArgs e)
        {
            var alsaExposer = new AlsaExposer();
            alsaExposer.Show();
        }

        private void _frameBufferExposer_Closed(object sender, System.EventArgs e)
        {
            _frameBufferExposer = null;
        }

        private void btnMice_Click(object sender, RoutedEventArgs e)
        {
            var w = new MiceExposer(_frameBufferExposer);
            w.Show();
        }
    }
}
