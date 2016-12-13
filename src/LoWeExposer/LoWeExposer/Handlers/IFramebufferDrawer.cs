
namespace LoWeExposer.Handlers
{
    interface IFramebufferDrawer
    {
        void Initialize(int width, int height, int bytesPerPixel);

        byte[] Data { get; }

        void Stop();
    }
}
