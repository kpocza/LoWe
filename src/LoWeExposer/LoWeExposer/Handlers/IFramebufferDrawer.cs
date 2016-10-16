using System.Windows.Media.Imaging;

namespace LoWeExposer.Handlers
{
    interface IFramebufferDrawer
    {
        void Update(WriteableBitmap writeableBitmap);
    }
}
