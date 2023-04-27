// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Render.Source;

namespace MauiSample
{
    public partial class MainPage : ContentPage
    {
        public MainPage()
        {
            InitializeComponent();
#if !ANDROID
            VideoGrid.Loaded += VideoGrid_Loaded;
#endif
        }

        private async Task LoadMauiAsset()
        {
            using var stream = await FileSystem.OpenAppPackageFileAsync("Config.json");
            using var reader = new StreamReader(stream);

            var contents = reader.ReadToEnd();
            Render.Source.Config.Reload(contents);
        }

        private async void VideoGrid_Loaded(object? sender, EventArgs e)
        {
            await LoadMauiAsset();
            int count = Config.Instance.VideoViewNumber;
            int itemCount = 1;

            while (true)
            {
                if (count <= itemCount * itemCount)
                {
                    for (int i = 0; i < itemCount; i++)
                    {
                        VideoGrid.ColumnDefinitions.Add(new ColumnDefinition());
                    }
                    for (int i = 0; i < itemCount; i++)
                    {
                        if (count == 0)
                        {
                            break;
                        }
                        VideoGrid.RowDefinitions.Add(new RowDefinition());
                        for (int j = 0; j < itemCount; j++)
                        {
                            if (count == 0)
                            {
                                break;
                            }
                            VideoView videoView = new();
                            videoView.SetValue(Grid.RowProperty, i);
                            videoView.SetValue(Grid.ColumnProperty, j);
                            VideoGrid.Children.Add(videoView);
                            count--;
                        }
                    }
                    break;
                }
                itemCount++;
            }
        }
    }
}
