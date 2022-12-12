using System;

namespace Render.Source
{
    public static class VideoSourceFactory
    {
        public static IVideoSource GetVideoSource() => VideoSource.Create();
    }
}
