// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using Microsoft.UI;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Navigation;
using Render.Source;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using WinUIEx;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace WinUISample
{
    /// <summary>
    /// An empty window that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainWindow : WindowEx
    {
        public MainWindow()
        {
            this.InitializeComponent();
            VideoGrid.Loaded += VideoGrid_Loaded;
        }

        private void VideoGrid_Loaded(object sender, RoutedEventArgs e)
        {
            this.Activate();

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
                            VideoView2 videoView = new();
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
