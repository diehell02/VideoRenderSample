using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace WPFSample
{
    internal class WriteableBitmapElement : VideoElement
    {
        public WriteableBitmapElement(bool autoResize)
        {
            _render = new WriteableBitmapSource();
            _autoResize = autoResize;
        }
    }
}
