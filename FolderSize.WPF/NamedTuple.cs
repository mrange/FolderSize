   
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

namespace FolderSize.WPF
{
   /// <summary>
   /// JobProgress (A named tuple class)
   /// </summary>
   partial struct JobProgress : IEquatable<JobProgress>
   {
      const int DefaultHash = 0x55555555;
      
      
      /// <summary>
      /// Constructs a JobProgress instance
      /// <param name="id">id is stored in Id property</param>
      /// <param name="numberOfJobs">numberOfJobs is stored in NumberOfJobs property</param>
      /// <param name="numberOfFinishedJobs">numberOfFinishedJobs is stored in NumberOfFinishedJobs property</param>
            
      /// </summary>
      public JobProgress (
            Guid id
         ,  long numberOfJobs
         ,  long numberOfFinishedJobs
      
         )
      {
         m_id = id;
         m_numberOfJobs = numberOfJobs;
         m_numberOfFinishedJobs = numberOfFinishedJobs;
      
      }

      /// <summary>
      /// Creats a JobProgress instance
      /// <param name="id">id is stored in Id property</param>
      /// <param name="numberOfJobs">numberOfJobs is stored in NumberOfJobs property</param>
      /// <param name="numberOfFinishedJobs">numberOfFinishedJobs is stored in NumberOfFinishedJobs property</param>
            
      /// </summary>
      public static JobProgress Create (
            Guid id
         ,  long numberOfJobs
         ,  long numberOfFinishedJobs
      
         )
      {
      
         return new JobProgress (
                  id
               ,  numberOfJobs
               ,  numberOfFinishedJobs
      
            );
      }

      Guid m_id;
      /// <summary>
      /// Gets and sets Id (Guid)
      /// </summary>
      public Guid Id 
      { 
         get
         {
            return m_id;
         }
         set
         {
            m_id = value;
         }
      }
      
      long m_numberOfJobs;
      /// <summary>
      /// Gets and sets NumberOfJobs (long)
      /// </summary>
      public long NumberOfJobs 
      { 
         get
         {
            return m_numberOfJobs;
         }
         set
         {
            m_numberOfJobs = value;
         }
      }
      
      long m_numberOfFinishedJobs;
      /// <summary>
      /// Gets and sets NumberOfFinishedJobs (long)
      /// </summary>
      public long NumberOfFinishedJobs 
      { 
         get
         {
            return m_numberOfFinishedJobs;
         }
         set
         {
            m_numberOfFinishedJobs = value;
         }
      }
      
      
      /// <summary>
      /// Indicates whether the current object is equal to another object of the same type.
      /// </summary>
      /// <returns>
      /// true if the current object is equal to the <paramref name="other"/> parameter; otherwise, false.
      /// </returns>
      /// <param name="other">An object to compare with this object.
      ///                 </param>
      public bool Equals (JobProgress other)
      {
                  
            bool equals = true;
            equals = 
                  equals && (Id != default (Guid) & other.Id != default (Guid)) 
               ?  Id.Equals(other.Id)
               :  Id == default (Guid) & other.Id == default (Guid);
            equals = 
                  equals && (NumberOfJobs != default (long) & other.NumberOfJobs != default (long)) 
               ?  NumberOfJobs.Equals(other.NumberOfJobs)
               :  NumberOfJobs == default (long) & other.NumberOfJobs == default (long);
            equals = 
                  equals && (NumberOfFinishedJobs != default (long) & other.NumberOfFinishedJobs != default (long)) 
               ?  NumberOfFinishedJobs.Equals(other.NumberOfFinishedJobs)
               :  NumberOfFinishedJobs == default (long) & other.NumberOfFinishedJobs == default (long);
            
            return equals;
                  
      }
      
      /// <summary>
      /// Determines whether the specified <see cref="T:System.Object"/> is equal to the current <see cref="T:System.Object"/>.
      /// </summary>
      /// <returns>
      /// true if the specified <see cref="T:System.Object"/> is equal to the current <see cref="T:System.Object"/>; otherwise, false.
      /// </returns>
      /// <param name="other">The <see cref="T:System.Object"/> to compare with the current <see cref="T:System.Object"/>. 
      ///                 </param><exception cref="T:System.NullReferenceException">The <paramref name="obj"/> parameter is null.
      ///                 </exception><filterpriority>2</filterpriority>
      public override bool Equals(object other)
      {
         
         if (other is JobProgress)
         {
            return Equals ((JobProgress)other);
         }
         else
         {
            return false;
         }
         
      }
      
      /// <summary>
      /// Serves as a hash function for a particular type. 
      /// </summary>
      /// <returns>
      /// A hash code for the current <see cref="T:System.Object"/>.
      /// </returns>
      /// <filterpriority>2</filterpriority>
      public override int GetHashCode ()
      {
         unchecked
         {
            var result = 0;
            result = (result * 397) ^ (Id != default (Guid) ? Id.GetHashCode() : DefaultHash);
            result = (result * 397) ^ (NumberOfJobs != default (long) ? NumberOfJobs.GetHashCode() : DefaultHash);
            result = (result * 397) ^ (NumberOfFinishedJobs != default (long) ? NumberOfFinishedJobs.GetHashCode() : DefaultHash);
            
            return result;
         }
      }
      
      /// <summary>
      /// Returns a <see cref="T:System.String"/> that represents the current <see cref="T:System.Object"/>.
      /// </summary>
      /// <returns>
      /// A <see cref="T:System.String"/> that represents the current <see cref="T:System.Object"/>.
      /// </returns>
      /// <filterpriority>2</filterpriority>
      public override string ToString ()
      {
         return new 
            {
               TypeName = "JobProgress",
               Id,
               NumberOfJobs,
               NumberOfFinishedJobs,
            
            }.ToString ();
      
      }
      

      public static bool operator== (JobProgress left, JobProgress right)
      {
            
            return left.Equals(right);
            
      }

      public static bool operator!= (JobProgress left, JobProgress right)
      {
            
            return !left.Equals(right);
            
      }

      
   }
}

