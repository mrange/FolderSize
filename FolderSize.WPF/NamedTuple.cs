   
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
   /// JobId (A named tuple class)
   /// </summary>
   partial struct JobId : IEquatable<JobId>
   {
      const int DefaultHash = 0x55555555;
      
      
      /// <summary>
      /// Constructs a JobId instance
      /// <param name="id">id is stored in Id property</param>
      /// <param name="numberOfJobs">numberOfJobs is stored in NumberOfJobs property</param>
            
      /// </summary>
      public JobId (
            Guid id
         ,  long numberOfJobs
      
         )
      {
         m_id = id;
         m_numberOfJobs = numberOfJobs;
      
      }

      /// <summary>
      /// Creats a JobId instance
      /// <param name="id">id is stored in Id property</param>
      /// <param name="numberOfJobs">numberOfJobs is stored in NumberOfJobs property</param>
            
      /// </summary>
      public static JobId Create (
            Guid id
         ,  long numberOfJobs
      
         )
      {
      
         return new JobId (
                  id
               ,  numberOfJobs
      
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
      
      
      /// <summary>
      /// Indicates whether the current object is equal to another object of the same type.
      /// </summary>
      /// <returns>
      /// true if the current object is equal to the <paramref name="other"/> parameter; otherwise, false.
      /// </returns>
      /// <param name="other">An object to compare with this object.
      ///                 </param>
      public bool Equals (JobId other)
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
         
         if (other is JobId)
         {
            return Equals ((JobId)other);
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
               TypeName = "JobId",
               Id,
               NumberOfJobs,
            
            }.ToString ();
      
      }
      
      
   }
}

