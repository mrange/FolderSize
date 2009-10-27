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

using System.Windows;
using System.Windows.Media.Animation;

namespace FolderSize.WPF
{
   class StoryboardHolder
   {

      // ----------------------------------------------------------------------

      readonly FrameworkElement m_containingElement;
      readonly Storyboard m_storyboard;

      // ----------------------------------------------------------------------

      bool m_isStarted;

      // ----------------------------------------------------------------------

      public StoryboardHolder (
         FrameworkElement containingElement,
         string name)
      {
         m_containingElement = containingElement;
         m_storyboard = (Storyboard) containingElement.Resources[name];
      }

      // ----------------------------------------------------------------------

      public void Stop()
      {
         if (m_isStarted)
         {
            m_storyboard.Stop (m_containingElement);
         }
      }

      // ----------------------------------------------------------------------

      public void Restart()
      {
         if (m_storyboard != null)
         {
            m_storyboard.Begin(m_containingElement, true);
            m_isStarted = true;
         }
      }

      // ----------------------------------------------------------------------

   }
}