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
using System.Windows;
using System.Windows.Media.Animation;

namespace FolderSize.WPF
{
   class StoryboardHolder
   {
      readonly FrameworkElement m_containingElement;
      readonly Storyboard m_storyboard;
      bool m_isStarted;


      public StoryboardHolder (
         FrameworkElement containingElement,
         string name)
      {
         m_containingElement = containingElement;
         m_storyboard = (Storyboard) containingElement.Resources[name];
      }

      public void Stop ()
      {
         if (m_isStarted)
         {
            m_storyboard.Stop (m_containingElement);
         }
      }

      public void Restart ()
      {
         if (m_storyboard != null)
         {
            if (m_isStarted)
            {
               var currentState = m_storyboard.GetCurrentState (m_containingElement);

               switch (currentState)
               {
                  case ClockState.Active:
                     m_storyboard.SeekAlignedToLastTick(m_containingElement, TimeSpan.Zero, TimeSeekOrigin.BeginTime);
                     break;
                  case ClockState.Filling:
                  case ClockState.Stopped:
                     m_storyboard.SeekAlignedToLastTick(m_containingElement, TimeSpan.Zero, TimeSeekOrigin.BeginTime);
                     m_storyboard.Begin(m_containingElement, true);
                     break;
                  default:
                     throw new ArgumentOutOfRangeException ();
               }

            }
            else
            {
               m_isStarted = true;
               m_storyboard.Begin(m_containingElement, true);
            }
         }
      }

   }
}