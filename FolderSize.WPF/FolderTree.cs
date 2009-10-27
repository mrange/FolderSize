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
using System.Linq;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Globalization;
using System.Windows.Threading;
using FolderSize.Common;

namespace FolderSize.WPF
{
   public class FolderEventArgs : EventArgs
   {
      public Folder Folder { get; set; }
   }

   partial class FolderTree : FrameworkElement
   {

      // ----------------------------------------------------------------------

      const double s_minDim = 4.0;
      const double s_width = 0.5;

      // ----------------------------------------------------------------------

      readonly Pen m_backgroundPen;
      readonly Brush m_backgroundBrush;
      readonly Brush m_folderBrush;
      readonly Pen m_folderPen;
      readonly Typeface m_folderTypeFace;
      readonly DispatcherTimer m_dispatcher = new DispatcherTimer(
         DispatcherPriority.SystemIdle);

      // ----------------------------------------------------------------------

      JobProgress? m_jobProgress;

      Transform m_viewTransform = Transform.Identity;
      SizeIndex? m_buildSizeIndex;

      DateTime m_leftMouseDownDateTime = DateTime.Now;
      Point? m_dragPosition;

      // ----------------------------------------------------------------------

      public event EventHandler<FolderEventArgs> FolderClick;
      public event EventHandler<FolderEventArgs> FolderDoubleClick; 

      // ----------------------------------------------------------------------

      static FolderTreeDisplayMode[] GetDefaultDisplayModes()
      {
         return
            Enum
               .GetValues(typeof(FolderTreeDisplayMode))
               .Cast<FolderTreeDisplayMode>()
               .ToArray();

      }

      // ----------------------------------------------------------------------

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

      // ----------------------------------------------------------------------

      public FolderTree()
      {
         m_backgroundPen = (Pen)Application.Current.Resources["FolderTreeBorderPen"];
         m_backgroundBrush = (Brush)Application.Current.Resources["FolderTreeBackGroundBrush"];
         m_folderPen = (Pen)Application.Current.Resources["FolderTreeFolderBorderPen"];
         m_folderBrush = (Brush)Application.Current.Resources["FolderTreeFolderGradient"];

         m_folderTypeFace = new Typeface("Segoe UI");

         m_dispatcher.Interval = new TimeSpan(0, 0, 0, 0, 500);
         m_dispatcher.Tick += DispatcherTick;
      }

      // ----------------------------------------------------------------------

      void DispatcherTick(object sender, EventArgs e)
      {
         var job = Job;

         if (job != null)
         {

            ProgressInfo = string.Format(
               "FolderSize.WPF [{0}, {1}]",
               (job.IsRunning ? "Running" : "Finished"),
               job.Jobs - job.FinishedJobs);


            var newJob = JobProgress.Create(
               job.Id,
               job.Jobs,
               job.FinishedJobs);

            if (!m_jobProgress.HasValue || !m_jobProgress.Equals(newJob))
            {
               RefreshFolderTree();
            }

            m_jobProgress = newJob;

            if (!job.IsRunning)
            {
               m_dispatcher.Stop();
            }
         }

         InvalidateCommands ();
      }

      // ----------------------------------------------------------------------

      void RefreshFolderTree()
      {
         var job = Job;
         if (job != null)
         {
            m_buildSizeIndex = job.BuildSizeIndex();
         }
         else
         {
            m_buildSizeIndex = null;
         }
         InvalidateVisual();
      }

      // ----------------------------------------------------------------------


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

      // ======================================================================
      // General folder tree visit functions
      // ======================================================================

      static double? VisitFolder(
         GeneralTransform generalTransform,
         IDictionary<Folder, CountAndSize> folderCountAndSizes,
         double x,
         double y,
         double xRatio,
         double yRatio,
         Folder folder,
         Func<CountAndSize, long> measurementPicker,
         Func<long, double, Folder, Rect, bool> visitor
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

         var visitResult = visitor(measurement, heightSquared, folder, rect);

         if (!visitResult)
         {
            return null;
         }

         var runningY = y;

         foreach (var childFolder in folder.Children)
         {
            var visitFolderResult = VisitFolder(
               generalTransform,
               folderCountAndSizes,
               x + xRatio,
               runningY,
               xRatio,
               yRatio,
               childFolder,
               measurementPicker,
               visitor);

            if(visitFolderResult == null)
            {
               return null;
            }

            runningY += visitFolderResult.Value;
         }

         return rect.Height;
      }

      double? VisitFolder(
         Func<long, double, Folder, Rect, bool> visitor)
      {
         var job = Job;
         var sizeIndex = m_buildSizeIndex;

         if (job == null || sizeIndex == null)
         {
            return null;
         }

         if (sizeIndex.Value.Depth <= 0)
         {
            return null;
         }

         var measurementPicker = GetMeasurementPicker ();

         var size =
            measurementPicker (sizeIndex.Value.CountsAndSizes[job.Root]);

         var xRatio = ActualWidth / sizeIndex.Value.Depth;
         var yRatio = ActualHeight / size;

         return VisitFolder (
            m_viewTransform,
            sizeIndex.Value.CountsAndSizes,
            0,
            0,
            xRatio,
            yRatio,
            job.Root,
            measurementPicker,
            visitor);
      }

      // ----------------------------------------------------------------------

      Folder FindFolderFromPoint(Point position)
      {
         var viewPosition = m_viewTransform.Inverse.Transform(position);

         Folder result = null;

         VisitFolder(
            (measurement,
             heightSquared,
             folder,
             rect) =>
            {
               if (!rect.Contains (viewPosition))
               {
                  return true;
               }

               result = folder;
               return false;
            });

         return result;
      }


      // ======================================================================
      // Mouse event handlers
      // ======================================================================

      protected override void OnMouseRightButtonUp(MouseButtonEventArgs e)
      {
         m_viewTransform = Transform.Identity;

         InvalidateVisual();

         base.OnMouseRightButtonUp (e);
      }

      // ----------------------------------------------------------------------

      protected override void OnMouseLeftButtonDown(MouseButtonEventArgs e)
      {
         m_dragPosition = e.MouseDevice.GetPosition (this);

         var now = DateTime.Now;
         var diff = now - m_leftMouseDownDateTime;
         if (diff.TotalMilliseconds < 200)
         {
            var position = e.MouseDevice.GetPosition(this);
            var folder = FindFolderFromPoint(position);

            if (folder != null)
            {
               if (FolderDoubleClick != null)
               {
                  FolderDoubleClick(
                     this,
                     new FolderEventArgs
                     {
                        Folder = folder
                     });
               }
            }
            
         }

         m_leftMouseDownDateTime = DateTime.Now;
         base.OnMouseLeftButtonDown(e);
      }

      // ----------------------------------------------------------------------

      protected override void OnMouseLeftButtonUp(MouseButtonEventArgs e)
      {
         m_dragPosition = null;

         var diff = DateTime.Now - m_leftMouseDownDateTime;
         if (diff.TotalMilliseconds < 400.0)
         {
            var position = e.MouseDevice.GetPosition (this);
            var folder = FindFolderFromPoint (position);

            if (folder != null)
            {
               Path = System.IO.Path.GetFullPath (folder.Path);
               if (FolderClick != null)
               {
                  FolderClick (
                     this, 
                     new FolderEventArgs
                        {
                           Folder = folder
                        });
               }
            }
         }

         base.OnMouseLeftButtonUp (e);
      }

      // ----------------------------------------------------------------------

      protected override void OnMouseMove(MouseEventArgs e)
      {
         var dragPosition = m_dragPosition;

         if (dragPosition != null)
         {
            var currentPosition = e.MouseDevice.GetPosition (this);

            var diff = currentPosition - dragPosition.Value;

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

      // ----------------------------------------------------------------------

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

      // ======================================================================
      // Render folder tree
      // ======================================================================

      protected override void OnRender(DrawingContext drawingContext)
      {
         var job = Job;
         var sizeIndex = m_buildSizeIndex;

         if (job == null || sizeIndex == null)
         {
            return;
         }

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

         VisitFolder (
            (measurement, heightSquared, folder, r) => DrawFolderImpl (
               drawingContext,
               measurement,
               heightSquared,
               folder,
               r));

         drawingContext.Pop();
         drawingContext.Pop();
      }

      // ----------------------------------------------------------------------

      bool DrawFolderImpl(
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
                     ToString(measurement),
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

      void StopJob()
      {
         var job = Job;
         if (job != null)
         {
            job.StopJob = true;
         }
      }

      // ======================================================================
      // Dependency properties change callbacks
      // ======================================================================

      partial void OnDisplayModePropertyChangedPartial(FolderTreeDisplayMode ldValue, FolderTreeDisplayMode newValue)
      {
         RefreshFolderTree();
      }
      
      partial void OnJobPropertyChangedPartial(FolderTraverserJob oldValue, FolderTraverserJob newValue)
      {
         RefreshFolderTree();
         if (newValue != null)
         {
            m_dispatcher.Start();
         }
         else
         {
            m_dispatcher.Stop();
         }
      }

      // ======================================================================
      // Commands
      // ======================================================================

      // ----------------------------------------------------------------------
      // Stop Command
      // ----------------------------------------------------------------------

      partial void OnCanExecuteStopCommandPartial(object parameter, ref bool canExecute)
      {
         var job = Job;
         if (job != null)
         {
            canExecute = job.IsRunning;
         }
      }

      partial void OnExecuteStopCommandPartial(object parameter)
      {
         StopJob();
         InvalidateCommands();
      }

      // ----------------------------------------------------------------------
      // Go Command
      // ----------------------------------------------------------------------

      partial void OnCanExecuteGoCommandPartial(object parameter, ref bool canExecute)
      {
         canExecute = true;
         var job = Job;
         if (job != null)
         {
            canExecute = !job.IsRunning;
         }
      }

      partial void OnExecuteGoCommandPartial(object parameter)
      {
         OnExecuteStopCommand(parameter);

         var job = FolderTraverser.StartTraverse(Path);
         Job = job;

         InvalidateCommands();
      }

      // ----------------------------------------------------------------------

      void InvalidateCommands()
      {
         RaiseCanExecuteStopCommandChanged();
         RaiseCanExecuteGoCommandChanged();
      }

      // ----------------------------------------------------------------------
   }
}
   