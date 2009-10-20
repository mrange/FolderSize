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

using System.Collections.Generic;

namespace FolderSize.Common
{
   public class Folder
   {

      // ----------------------------------------------------------------------

      public readonly string Path;
      public readonly string Name;
      public readonly long Size;
      public readonly long Count;
      public readonly CountAndSize CountAndSize;

      // ----------------------------------------------------------------------

      List<Folder> m_folders = new List<Folder>();

      // ----------------------------------------------------------------------

      public Folder(
         string path,
         string name,
         long size,
         long count)
      {
         Path = path;
         Name = name;
         Size = size;
         Count = count;
         CountAndSize = CountAndSize.Create(Count, Size);
      }

      // ----------------------------------------------------------------------

      public void SetFolder(List<Folder> folders)
      {
         m_folders = folders;
      }

      // ----------------------------------------------------------------------

      public override string ToString()
      {
         return Path;
      }

      // ----------------------------------------------------------------------

      public IEnumerable<Folder> Children
      {
         get
         {
            return m_folders;
         }
      }

      // ----------------------------------------------------------------------

   }
}