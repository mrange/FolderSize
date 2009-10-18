
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

      readonly Pen m_backgroundPen;
      readonly Brush m_backgroundBrush;
      readonly Brush m_folderBrush;
      readonly Pen m_folderPen;
      readonly Typeface m_folderTypeFace;

      FolderTraverserJob m_job;

      Transform m_viewTransform = Transform.Identity;
      Point? m_dragPosition;
      SizeIndex? m_buildSizeIndex;

      public FolderTree()
      {
         m_backgroundPen = (Pen)Application.Current.Resources["FolderTreeBorderPen"];
         m_backgroundBrush = (Brush)Application.Current.Resources["FolderTreeBackGroundBrush"];
         m_folderPen = (Pen)Application.Current.Resources["FolderTreeFolderBorderPen"];
         m_folderBrush = (Brush)Application.Current.Resources["FolderTreeFolderGradient"];

         m_folderTypeFace = new Typeface("Segoe UI");
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
            m_buildSizeIndex = null;
         }
      }

      public void Refresh()
      {
         m_buildSizeIndex = m_job.BuildSizeIndex ();
         InvalidateVisual ();
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

      Func<CountAndSize, long> GetMeasurementPicker()
      {

         switch (DisplayMode)
         {
            case FolderTreeDisplayMode.Count:
               return cs => cs.Count;
            case FolderTreeDisplayMode.Size:
            default:
               return cs => cs.Size;
         }
      }

      static double VisitFolder(
         GeneralTransform generalTransform,
         IDictionary<Folder, CountAndSize> folderCountAndSizes,
         double x,
         double y,
         double xRatio,
         double yRatio,
         Folder folder,
         Func<CountAndSize, long> measurementPicker,
         Func<long, double, Folder, Rect, bool> visit
         )
      {
         CountAndSize countAndSize;

         if (!folderCountAndSizes.TryGetValue(
                 folder,
                 out countAndSize))
         {
            return 0;
         }

         var measurement = measurementPicker(countAndSize);

         var width = xRatio - s_width;
         var height = yRatio * measurement - s_width;

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

         var diffHeight = bottomLeft - topLeft;
         var diffWidth = topRight - topLeft;

         var heightSquared = diffHeight.LengthSquared;

         if (heightSquared <= s_minDim * s_minDim
             || diffWidth.LengthSquared <= s_minDim * s_minDim)
         {
            return 0;
         }

         visit(measurement, heightSquared, folder, rect);

         var runningY = y;

         foreach (var childFolder in folder.Children)
         {
            runningY += VisitFolder(
               generalTransform,
               folderCountAndSizes,
               x + xRatio,
               runningY,
               xRatio,
               yRatio,
               childFolder,
               measurementPicker,
               visit);
         }

         return rect.Height;
      }

      protected override void OnMouseRightButtonUp(MouseButtonEventArgs e)
      {
         m_viewTransform = Transform.Identity;

         InvalidateVisual();

         base.OnMouseRightButtonUp (e);
      }

      protected override void OnMouseLeftButtonDown(MouseButtonEventArgs e)
      {
         m_dragPosition = e.MouseDevice.GetPosition (this);
         base.OnMouseLeftButtonDown (e);
      }

      protected override void OnMouseLeftButtonUp(MouseButtonEventArgs e)
      {
         m_dragPosition = null;
         base.OnMouseLeftButtonUp(e);
      }

      protected override void OnMouseMove(MouseEventArgs e)
      {
         if(m_dragPosition != null)
         {
            var currentPosition = e.MouseDevice.GetPosition (this);

            var diff = currentPosition - m_dragPosition.Value;

            var translate = new TranslateTransform(
                diff.X,
                diff.Y);

            var transformGroup = new TransformGroup();

            transformGroup.Children.Add(m_viewTransform);
            transformGroup.Children.Add(translate);
            m_viewTransform = new MatrixTransform(transformGroup.Value);

            m_dragPosition = currentPosition;

            InvalidateVisual();
         }
         base.OnMouseMove(e);
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

         base.OnMouseWheel(e);
      }

      protected override void OnRender(DrawingContext drawingContext)
      {
         // base.OnRender(drawingContext);

         if (m_job != null && m_buildSizeIndex != null)
         {
            var rect = new Rect(
                0,
                0,
                ActualWidth,
                ActualHeight);

            drawingContext.PushClip(new RectangleGeometry(rect));

            drawingContext.DrawRectangle(
                m_backgroundBrush,
                m_backgroundPen,
                rect);

            drawingContext.PushTransform(m_viewTransform);

            if (m_buildSizeIndex.Value.Depth > 0)
            {
               var measurementPicker = GetMeasurementPicker ();

               var size = measurementPicker(m_buildSizeIndex.Value.CountsAndSizes[m_job.Root]);

               var xRatio = ActualWidth / m_buildSizeIndex.Value.Depth;
               var yRatio = ActualHeight / size;

               VisitFolder(
                   m_viewTransform,
                   m_buildSizeIndex.Value.CountsAndSizes,
                   0,
                   0,
                   xRatio,
                   yRatio,
                   m_job.Root,
                   measurementPicker,
                   (measurement, heightSquared, folder, r) => DrawFolderImpl(drawingContext, measurement, heightSquared, folder, r));
            }

            drawingContext.Pop();
            drawingContext.Pop();
         }
      }

      bool DrawFolderImpl (
         DrawingContext drawingContext,
         long measurement,
         double heightSquared,
         Folder folder,
         Rect rect)
      {
         drawingContext.DrawRectangle(
            m_folderBrush,
            m_folderPen,
            rect);

         if (heightSquared > s_minDim * s_minDim * 2.0 * 2.0)
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
                  12.0 * rect.Height / Math.Sqrt(heightSquared),
                  Brushes.White),
               new Point(
                  rect.X + 2,
                  rect.Y + 2));

            drawingContext.Pop();
         }

         return true;
      }
   }
}
   