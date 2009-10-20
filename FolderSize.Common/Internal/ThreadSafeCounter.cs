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

using System.Threading;

namespace FolderSize.Common.Internal
{
   struct ThreadSafeCounter
   {

      // ----------------------------------------------------------------------

      long m_counter;

      // ----------------------------------------------------------------------

      public ThreadSafeCounter(long c)
      {
         m_counter = c;
      }

      // ----------------------------------------------------------------------

      public long Add(long c)
      {
         return Interlocked.Add(ref m_counter, c);
      }

      // ----------------------------------------------------------------------

      public long Value
      {
         get
         {
            return Interlocked.Read(ref m_counter);
         }
         set
         {
            Interlocked.Exchange(ref m_counter, value);
         }
      }

      // ----------------------------------------------------------------------

      public override string ToString()
      {
         return Value.ToString();
      }

      // ----------------------------------------------------------------------

      public override int GetHashCode()
      {
         return Value.GetHashCode();
      }

      // ----------------------------------------------------------------------

   }
}