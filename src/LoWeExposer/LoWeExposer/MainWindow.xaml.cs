using System.Windows;

namespace LoWeExposer
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private void btnFrameBuffer_Click(object sender, RoutedEventArgs e)
        {
            var w = new FrameBufferExposer();
            w.Show();
        }

        private void btnALSA_Click(object sender, RoutedEventArgs e)
        {
            var w = new AlsaExposer();
            w.Show();
        }
    }
}
