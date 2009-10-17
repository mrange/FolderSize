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
using System.Threading;
using FolderSize.Common.Internal;

namespace FolderSize.Common
{
   public class FolderTraverserJob
   {
      public readonly Folder Root;
      public readonly Guid Id = Guid.NewGuid();

      ThreadSafeCounter m_jobCount;
      ThreadSafeCounter m_finishedJobCount;
      ThreadSafeCounter m_failedJobCount;
      ThreadSafeCounter m_failedFolderCount;
      ThreadSafeFlag m_stopJob;

      Folder CreateFolder(
         FileData fd)
      {
         var fileDatas = new List<FileData>();

         using( var files = new FindFiles(
            fd.FullName) )
         {
            long totalSize = 0;
            long count = 0;
            while(files.MoveNext())
            {
               var currentFile = files.Current;

               if(currentFile.Name == "." || currentFile.Name == ".." )
               {

               }
               else if (currentFile.IsDictionary)
               {
                  fileDatas.Add(currentFile);
               }
               else
               {
                  totalSize += currentFile.Length;
                  ++count;
               }
            }

            var folder = new Folder(
               fd.FullName,
               fd.Name,
               totalSize,
               count);

            Traverse(folder, fileDatas);

            return folder;

         }
      }

      public bool StopJob
      {
         set
         {
            m_stopJob.Value = value;
         }
      }

      public bool IsRunning
      {
         get
         {
            return
               !m_stopJob.Value
               && (Jobs - FinishedJobs) != 0;
         }
      }

      public long Jobs
      {
         get
         {
            return m_jobCount.Value;
         }
      }

      public long FinishedJobs
      {
         get
         {
            return m_finishedJobCount.Value;
         }
      }

      void Traverse(
         Folder parentFolder,
         IList<FileData> fds)
      {
         if (m_stopJob.Value || fds.Count <= 0)
         {
            return;
         }

         m_jobCount.Add(1);

         ThreadPool.QueueUserWorkItem(
            state =>
               {
                  try
                  {
                     var folders = new List<Folder>(fds.Count);

                     for (var index = 0;
                          index < fds.Count && !m_stopJob.Value;
                          ++index)
                     {
                        try
                        {
                           folders.Add(CreateFolder(fds[index]));
                        }
                        catch (Exception)
                        {
                           m_failedFolderCount.Add(1);
                        }
                     }

                     parentFolder.SetFolder(folders);
                  }
                  catch (Exception)
                  {
                     m_failedJobCount.Add(1);
                  }
                  finally
                  {
                     m_finishedJobCount.Add(1);
                  }
               });
      }

      internal FolderTraverserJob(
         string path)
      {
         Root = CreateFolder(
            new FileData(
               path,
               path,
               true,
               0));
      }

      static FolderInfo BuildSizeIndex(
         IDictionary<Folder, CountAndSize> folderDictionary, 
         Folder folder)
      {
         if (folder == null)
         {
            return new FolderInfo();
         }

         var depth = 0;

         var size = folder.Size;
         var count = folder.Count;

         foreach (var childFolder in folder.Children)
         {
            var buildSizeIndex = BuildSizeIndex(
               folderDictionary,
               childFolder);
            depth = Math.Max(
               depth,
               buildSizeIndex.Depth);

            count += buildSizeIndex.Count;
            size += buildSizeIndex.Size;
         }

         var countAndSize = CountAndSize.Create(
            count,
            size);

         folderDictionary.Add(
            folder,
            countAndSize);

         return FolderInfo.Create(
            depth + 1,
            count,
            size);
      }

      public SizeIndex BuildSizeIndex()
      {
         var countAndSizes = new Dictionary<Folder, CountAndSize>();

         var buildSizeIndex = BuildSizeIndex(countAndSizes, Root);

         return SizeIndex.Create(buildSizeIndex.Depth, countAndSizes);
      }

   }
}