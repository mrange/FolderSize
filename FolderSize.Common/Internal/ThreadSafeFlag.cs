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
   struct ThreadSafeFlag
   {
      long m_flag;

      public ThreadSafeFlag(bool b)
      {
         m_flag = b ? 1 : 0;
      }

      public bool Value
      {
         get
         {
            return Interlocked.Read(ref m_flag) != 0;
         }
         set
         {
            Interlocked.Exchange(ref m_flag, value ? 1 : 0);
         }
      }

      public override string ToString()
      {
         return Value.ToString();
      }

      public override int GetHashCode()
      {
         return Value.GetHashCode();
      }
   }
}