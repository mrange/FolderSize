   
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

namespace FolderSize.Common
{
   /// <summary>
   /// CountAndSize (A named tuple class)
   /// </summary>
   public partial struct CountAndSize : IEquatable<CountAndSize>
   {
      const int DefaultHash = 0x55555555;
      
      
      /// <summary>
      /// Constructs a CountAndSize instance
      /// <param name="count">count is stored in Count property</param>
      /// <param name="size">size is stored in Size property</param>
            
      /// </summary>
      public CountAndSize (
            long count
         ,  long size
      
         )
      {
         m_count = count;
         m_size = size;
      
      }

      /// <summary>
      /// Creats a CountAndSize instance
      /// <param name="count">count is stored in Count property</param>
      /// <param name="size">size is stored in Size property</param>
            
      /// </summary>
      public static CountAndSize Create (
            long count
         ,  long size
      
         )
      {
      
         return new CountAndSize (
                  count
               ,  size
      
            );
      }

      long m_count;
      /// <summary>
      /// Gets and sets Count (long)
      /// </summary>
      public long Count 
      { 
         get
         {
            return m_count;
         }
         set
         {
            m_count = value;
         }
      }
      
      long m_size;
      /// <summary>
      /// Gets and sets Size (long)
      /// </summary>
      public long Size 
      { 
         get
         {
            return m_size;
         }
         set
         {
            m_size = value;
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
      public bool Equals (CountAndSize other)
      {
                  
            bool equals = true;
            equals = 
                  equals && (Count != default (long) & other.Count != default (long)) 
               ?  Count.Equals(other.Count)
               :  Count == default (long) & other.Count == default (long);
            equals = 
                  equals && (Size != default (long) & other.Size != default (long)) 
               ?  Size.Equals(other.Size)
               :  Size == default (long) & other.Size == default (long);
            
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
         
         if (other is CountAndSize)
         {
            return Equals ((CountAndSize)other);
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
            result = (result * 397) ^ (Count != default (long) ? Count.GetHashCode() : DefaultHash);
            result = (result * 397) ^ (Size != default (long) ? Size.GetHashCode() : DefaultHash);
            
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
               TypeName = "CountAndSize",
               Count,
               Size,
            
            }.ToString ();
      
      }
      
      
   }
   /// <summary>
   /// FolderInfo (A named tuple class)
   /// </summary>
   partial struct FolderInfo : IEquatable<FolderInfo>
   {
      const int DefaultHash = 0x55555555;
      
      
      /// <summary>
      /// Constructs a FolderInfo instance
      /// <param name="depth">depth is stored in Depth property</param>
      /// <param name="count">count is stored in Count property</param>
      /// <param name="size">size is stored in Size property</param>
            
      /// </summary>
      public FolderInfo (
            int depth
         ,  long count
         ,  long size
      
         )
      {
         m_depth = depth;
         m_count = count;
         m_size = size;
      
      }

      /// <summary>
      /// Creats a FolderInfo instance
      /// <param name="depth">depth is stored in Depth property</param>
      /// <param name="count">count is stored in Count property</param>
      /// <param name="size">size is stored in Size property</param>
            
      /// </summary>
      public static FolderInfo Create (
            int depth
         ,  long count
         ,  long size
      
         )
      {
      
         return new FolderInfo (
                  depth
               ,  count
               ,  size
      
            );
      }

      int m_depth;
      /// <summary>
      /// Gets and sets Depth (int)
      /// </summary>
      public int Depth 
      { 
         get
         {
            return m_depth;
         }
         set
         {
            m_depth = value;
         }
      }
      
      long m_count;
      /// <summary>
      /// Gets and sets Count (long)
      /// </summary>
      public long Count 
      { 
         get
         {
            return m_count;
         }
         set
         {
            m_count = value;
         }
      }
      
      long m_size;
      /// <summary>
      /// Gets and sets Size (long)
      /// </summary>
      public long Size 
      { 
         get
         {
            return m_size;
         }
         set
         {
            m_size = value;
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
      public bool Equals (FolderInfo other)
      {
                  
            bool equals = true;
            equals = 
                  equals && (Depth != default (int) & other.Depth != default (int)) 
               ?  Depth.Equals(other.Depth)
               :  Depth == default (int) & other.Depth == default (int);
            equals = 
                  equals && (Count != default (long) & other.Count != default (long)) 
               ?  Count.Equals(other.Count)
               :  Count == default (long) & other.Count == default (long);
            equals = 
                  equals && (Size != default (long) & other.Size != default (long)) 
               ?  Size.Equals(other.Size)
               :  Size == default (long) & other.Size == default (long);
            
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
         
         if (other is FolderInfo)
         {
            return Equals ((FolderInfo)other);
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
            result = (result * 397) ^ (Depth != default (int) ? Depth.GetHashCode() : DefaultHash);
            result = (result * 397) ^ (Count != default (long) ? Count.GetHashCode() : DefaultHash);
            result = (result * 397) ^ (Size != default (long) ? Size.GetHashCode() : DefaultHash);
            
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
               TypeName = "FolderInfo",
               Depth,
               Count,
               Size,
            
            }.ToString ();
      
      }
      
      
   }
   /// <summary>
   /// SizeIndex (A named tuple class)
   /// </summary>
   public partial struct SizeIndex : IEquatable<SizeIndex>
   {
      const int DefaultHash = 0x55555555;
      
      
      /// <summary>
      /// Constructs a SizeIndex instance
      /// <param name="depth">depth is stored in Depth property</param>
      /// <param name="countsAndSizes">countsAndSizes is stored in CountsAndSizes property</param>
            
      /// </summary>
      public SizeIndex (
            int depth
         ,  System.Collections.Generic.Dictionary<Folder, CountAndSize> countsAndSizes
      
         )
      {
         m_depth = depth;
         m_countsAndSizes = countsAndSizes;
      
      }

      /// <summary>
      /// Creats a SizeIndex instance
      /// <param name="depth">depth is stored in Depth property</param>
      /// <param name="countsAndSizes">countsAndSizes is stored in CountsAndSizes property</param>
            
      /// </summary>
      public static SizeIndex Create (
            int depth
         ,  System.Collections.Generic.Dictionary<Folder, CountAndSize> countsAndSizes
      
         )
      {
      
         return new SizeIndex (
                  depth
               ,  countsAndSizes
      
            );
      }

      int m_depth;
      /// <summary>
      /// Gets and sets Depth (int)
      /// </summary>
      public int Depth 
      { 
         get
         {
            return m_depth;
         }
         set
         {
            m_depth = value;
         }
      }
      
      System.Collections.Generic.Dictionary<Folder, CountAndSize> m_countsAndSizes;
      /// <summary>
      /// Gets and sets CountsAndSizes (System.Collections.Generic.Dictionary<Folder, CountAndSize>)
      /// </summary>
      public System.Collections.Generic.Dictionary<Folder, CountAndSize> CountsAndSizes 
      { 
         get
         {
            return m_countsAndSizes;
         }
         set
         {
            m_countsAndSizes = value;
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
      public bool Equals (SizeIndex other)
      {
                  
            bool equals = true;
            equals = 
                  equals && (Depth != default (int) & other.Depth != default (int)) 
               ?  Depth.Equals(other.Depth)
               :  Depth == default (int) & other.Depth == default (int);
            equals = 
                  equals && (CountsAndSizes != default (System.Collections.Generic.Dictionary<Folder, CountAndSize>) & other.CountsAndSizes != default (System.Collections.Generic.Dictionary<Folder, CountAndSize>)) 
               ?  CountsAndSizes.Equals(other.CountsAndSizes)
               :  CountsAndSizes == default (System.Collections.Generic.Dictionary<Folder, CountAndSize>) & other.CountsAndSizes == default (System.Collections.Generic.Dictionary<Folder, CountAndSize>);
            
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
         
         if (other is SizeIndex)
         {
            return Equals ((SizeIndex)other);
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
            result = (result * 397) ^ (Depth != default (int) ? Depth.GetHashCode() : DefaultHash);
            result = (result * 397) ^ (CountsAndSizes != default (System.Collections.Generic.Dictionary<Folder, CountAndSize>) ? CountsAndSizes.GetHashCode() : DefaultHash);
            
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
               TypeName = "SizeIndex",
               Depth,
               CountsAndSizes,
            
            }.ToString ();
      
      }
      
      
   }
}

