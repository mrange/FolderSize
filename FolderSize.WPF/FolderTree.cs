
/* ****************************************************************************
 *
 * Copyright (c) Mårten Rånge.
 *
 * This source code is subject to terms and conditions of the Microsoft Public License. A 
 * copy of the license can be found in the License.html file at the root of this distribution. If 
 * you cannot locate the  Microsoft Public License, please send an email to 
 * dlr@microsoft.com. By using this source code in any fashion, you are agreeing to be bound 
 * by the terms of the Microsoft Public License.
 *
 * You must not remove this notice, or any other, from this software.
 *
 *
 * ***************************************************************************/

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Globalization;
using FolderSize.Common;

namespace FolderSize.WPF
{
   public class FolderTree : FrameworkElement
   {
      public static readonly DependencyProperty DisplayModeProperty =
         DependencyProperty.Register (
            "DisplayMode",
            typeof (FolderTreeDisplayMode),
            typeof (FolderTree),
            new PropertyMetadata (
               FolderTreeDisplayMode.Size,
               (source, e) =>
                  {
                     var folderTree = source as FolderTree;

                     if (folderTree != null)
                     {
                        folderTree.InvalidateVisual ();
                     }

                  }));

      static ObservableCollection<FolderTreeDisplayMode> GetDefaultDisplayModes ()
      {
         return new ObservableCollection<FolderTreeDisplayMode> (
            Enum
               .GetValues (typeof (FolderTreeDisplayMode))
               .Cast<FolderTreeDisplayMode> ());

      }

      public static readonly DependencyProperty DisplayModesProperty =
         DependencyProperty.Register (
            "DisplayModes",
            typeof (ObservableCollection<FolderTreeDisplayMode>),
            typeof (FolderTree),
            new PropertyMetadata (
               GetDefaultDisplayModes ()));

      const double s_minDim = 4.0;
      const double s_width = 0.5;

      readonly Pen m_backGroundPen;
      readonly Brush m_folderBrush;
      readonly Pen m_folderPen;
      readonly Typeface m_folderTypeFace;

      FolderTraverserJob m_job;

      Transform m_viewTransform = Transform.Identity;

      public FolderTree()
      {
         m_backGroundPen = new Pen(
             Brushes.Black,
             1.0);

         m_folderBrush = (Brush) Application.Current.Resources["FolderGradient"];

         

         m_folderPen = new Pen(
             new  SolidColorBrush(Color.FromRgb (0x00, 0x6E, 0xFF)),
             s_width);

         m_folderTypeFace = new Typeface(
             "Segoe UI");
      }

      public FolderTreeDisplayMode DisplayMode
      {
         get
         {
            return (FolderTreeDisplayMode)GetValue(DisplayModeProperty);
         }
         set
         {
            SetValue(DisplayModeProperty, value);
         }
      }

      public ObservableCollection<FolderTreeDisplayMode> DisplayModes
      {
         get
         {
            return (ObservableCollection<FolderTreeDisplayMode>) GetValue(DisplayModesProperty);
         }
      }

      public FolderTraverserJob Job
      {
         get
         {
            return m_job;
         }
         set
         {
            m_job = value;
         }
      }

      static string ToString(long s)
      {
         if (s > 500000000)
         {
            return string.Format("{0}G", Math.Round(s / 1000000000.0, 1));
         }
         else if (s > 500000)
         {
            return string.Format("{0}M", Math.Round(s / 1000000.0, 1));
         }
         else if (s > 500)
         {
            return string.Format("{0}k", Math.Round(s / 1000.0, 1));
         }
         else
         {
            return s.ToString();
         }
      }

      long PickProperty(CountAndSize v)
      {
         switch (DisplayMode)
         {
            case FolderTreeDisplayMode.Count:
               return v.Count;
            case FolderTreeDisplayMode.Size:
            default:
               return v.Size;
         }
      }

      double DrawFolder(
         DrawingContext drawingContext,
         GeneralTransform generalTransform,
         IDictionary<Folder, CountAndSize> folderCountAndSizes,
         double x,
         double y,
         double xRatio,
         double yRatio,
         Folder folder)
      {
         CountAndSize countAndSize;

         if (!folderCountAndSizes.TryGetValue(
                 folder,
                 out countAndSize))
         {
            return 0;
         }

         var measurement = PickProperty(countAndSize);

         var width = xRatio - s_width;
         var height = yRatio*measurement - s_width;

         if (width <= 0
             || height <= 0)
         {
            return 0;
         }

         var rect = new Rect(
            x,
            y,
            width,
            height);

         var topLeft = generalTransform.Transform(rect.TopLeft);
         var bottomLeft = generalTransform.Transform(rect.BottomLeft);
         var topRight = generalTransform.Transform(rect.TopRight);

         var l_diff_height = bottomLeft - topLeft;
         var l_diff_width = topRight - topLeft;

         var heightSquared = l_diff_height.LengthSquared;

         if (heightSquared <= s_minDim*s_minDim
             || l_diff_width.LengthSquared <= s_minDim*s_minDim)
         {
            return 0;
         }

         drawingContext.DrawRectangle(
            m_folderBrush,
            m_folderPen,
            rect);

         if (heightSquared > s_minDim*s_minDim*2.0*2.0)
         {
            drawingContext.PushClip(
               new RectangleGeometry(rect));

            drawingContext.DrawText(
               new FormattedText(
                  string.Format(
                     "{0}\r\n{1}\r\n{2}",
                     folder.Name,
                     ToString(PickProperty(folder.CountAndSize)),
                     ToString(measurement)),
                  CultureInfo.CurrentUICulture,
                  FlowDirection.LeftToRight,
                  m_folderTypeFace,
                  12.0*height/Math.Sqrt(heightSquared),
                  Brushes.White),
               new Point(
                  x + 2,
                  y + 2));

            drawingContext.Pop();
         }

         var runningY = y;

         foreach (var l_f in folder.Children)
         {
            runningY += DrawFolder(
               drawingContext,
               generalTransform,
               folderCountAndSizes,
               x + xRatio,
               runningY,
               xRatio,
               yRatio,
               l_f);
         }

         return rect.Height;
      }

      protected override void OnMouseRightButtonUp(MouseButtonEventArgs e)
      {
         m_viewTransform = Transform.Identity;

         InvalidateVisual();
      }

      protected override void OnMouseWheel(MouseWheelEventArgs e)
      {
         var position = e.MouseDevice.GetPosition(this);

         var zoomFactor = Math.Pow(1.1, e.Delta / 120.0);

         var transformToOrigo = new TranslateTransform(
             -position.X,
             -position.Y);

         var transformScale = new ScaleTransform(
             zoomFactor,
             zoomFactor);

         var transformFromOrigo = new TranslateTransform(
             position.X,
             position.Y);

         var transformGroup = new TransformGroup();

         transformGroup.Children.Add(m_viewTransform);
         transformGroup.Children.Add(transformToOrigo);
         transformGroup.Children.Add(transformScale);
         transformGroup.Children.Add(transformFromOrigo);

         m_viewTransform = new MatrixTransform(transformGroup.Value);

         InvalidateVisual();
      }

      protected override void OnRender(DrawingContext drawingContext)
      {
         // base.OnRender(drawingContext);

         if (m_job != null)
         {
            var rect = new Rect(
                0,
                0,
                ActualWidth,
                ActualHeight);

            drawingContext.PushClip(new RectangleGeometry(rect));

            drawingContext.DrawRectangle(
                null,
                m_backGroundPen,
                rect);

            drawingContext.PushTransform(m_viewTransform);

            var buildSizeIndex = m_job.BuildSizeIndex();

            if (buildSizeIndex.Depth > 0)
            {
               var size = PickProperty(buildSizeIndex.CountsAndSizes[m_job.Root]);

               var xRatio = ActualWidth / buildSizeIndex.Depth;
               var yRatio = ActualHeight / size;

               DrawFolder(
                   drawingContext,
                   m_viewTransform,
                   buildSizeIndex.CountsAndSizes,
                   0,
                   0,
                   xRatio,
                   yRatio,
                   m_job.Root);
            }

            drawingContext.Pop();
            drawingContext.Pop();
         }
      }
   }
}
