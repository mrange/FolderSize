using System.Windows;
using System.Windows.Controls;

namespace FolderSize.WPF
{
   partial class BubbleContentControl : ContentControl
   {
      static BubbleContentControl()
      {
         DefaultStyleKeyProperty.OverrideMetadata(typeof(BubbleContentControl), new FrameworkPropertyMetadata(typeof(BubbleContentControl)));
      }

      public override void OnApplyTemplate()
      {
         base.OnApplyTemplate();

         var contentGrid = (FrameworkElement)GetTemplateChild("ContentGrid");
         contentGrid.SizeChanged += ContentGridSizeChanged;
      }

      void ContentGridSizeChanged(object sender, SizeChangedEventArgs e)
      {
         var margin = ContentRectMargin;

         ContentRect = new Rect(
            margin.Left, 
            margin.Top,
            e.NewSize.Width - margin.Left - margin.Right,
            e.NewSize.Height - margin.Bottom - margin.Top);
      }

      partial void OnContentRectMarginPropertyChangedPartial(Thickness oldValue, Thickness newValue)
      {
         var contentRect = ContentRect;

         ContentRect = new Rect(
            newValue.Left,
            newValue.Right,
            contentRect.Width - newValue.Left - newValue.Right + oldValue.Left + oldValue.Right,
            contentRect.Height - newValue.Top - newValue.Bottom + oldValue.Top + oldValue.Bottom
            );

      }

   }
}
