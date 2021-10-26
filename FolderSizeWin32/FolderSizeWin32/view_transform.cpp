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

// ----------------------------------------------------------------------------
#include "StdAfx.h"
// ----------------------------------------------------------------------------
#include "view_transform.hpp"
// ----------------------------------------------------------------------------
#include <functional>
// ----------------------------------------------------------------------------
namespace view_transform
{
   namespace   s  = std       ;
   namespace   l  = linear    ;

   extended_vector const create_extended_vector (double const x, double const y)
   {
      extended_vector v (l::no_initialize::value);
      v.values [0] = x;
      v.values [1] = y;
      v.values [2] = 1.0;
      return v;
   }

   vector const create_vector (double const x, double const y)
   {
      vector v (l::no_initialize::value);
      v.values [0] = x;
      v.values [1] = y;
      return v;
   }

   namespace
   {
      struct transform_state
      {
         transform_state (
               transform_direction::type const  direction_
            ,  dimension const &                size_
            ,  vector const &                   centre_
            ,  vector const &                   zoom_
            )
            :  direction   (direction_ )
            ,  size        (size_      )
            ,  centre      (centre_    )
            ,  zoom        (zoom_      )
         {
         }

         transform_direction::type const  direction;
         dimension const &                size;
         vector const &                   centre;
         vector const &                   zoom;
      };

      typedef s::function<transform const (transform_state const & state)>
         transform_functor;

      transform_functor const translate (
            s::function<vector const (transform_state const & state)> const predicate
         )
      {
         return [predicate] (transform_state const & state) -> transform const
         {
            auto v = predicate (state);
            switch (state.direction)
            {
            case transform_direction::forward:
               return linear::translating_matrix (v);
            case transform_direction::reverse:
               return linear::translating_matrix (-v);
            default:
               FS_ASSERT (false);
               return linear::identity_matrix<double, 3> ();
            }
         };
      }

      transform_functor const scale (
            s::function<vector const (transform_state const & state)> const predicate
         )
      {
         return [predicate] (transform_state const & state) -> transform const
         {
            auto v = predicate (state);
            switch (state.direction)
            {
            case transform_direction::forward:
               return linear::scaling_matrix (v);
            case transform_direction::reverse:
               return linear::scaling_matrix (linear::invert_vector (v));
            default:
               FS_ASSERT (false);
               return linear::identity_matrix<double, 3> ();
            }
         };
      }

      std::size_t const count_configuration_items (transform_functor const * const configuration)
      {
         if (!configuration)
         {
            return 0;
         }

         for (auto iter = 0; ;++iter)
         {
            transform_functor const tc = configuration[iter];

            if (!tc)
            {
               return iter;
            }
         }
      }

      transform_functor const create_from_configuration (transform_functor const * const configuration)
      {
         auto count = count_configuration_items (configuration);

         if (count < 1)
         {
            return [] (transform_state const & state) -> transform const
            {
               return linear::identity_matrix<double, 3> ();
            };
         }

            return [count, configuration] (transform_state const & state) -> transform const
            {
               auto result = linear::identity_matrix<double, 3> ();
               switch (state.direction)
               {
               case transform_direction::forward:
                  {
                     for (auto iter = 0u; iter < count; ++iter)
                     {
                        result = configuration[iter] (state) * result;
                     }
                     break;
                  }
               case transform_direction::reverse:
                  {
                     for (auto iter = count; iter > 0u; --iter)
                     {
                        result = configuration[iter - 1] (state) * result;
                     }
                     break;
                  }
               default:
                  FS_ASSERT (false);
                  break;
               }
               return result;
            };
      }

      transform_functor const terminator;

      transform_functor const bitmap_to_view_transform_configuration [] =
      {
            // Scale to square with side 1
            scale       ([] (transform_state const & state) { return invert_vector (state.size)                   ;})
            // Translate square to center
         ,  translate   ([] (transform_state const & state) { return view_transform::create_vector (-0.5 , -0.5)  ;})
         ,  terminator
      };

      transform_functor const modify_view_transform_configuration [] =
      {
            // Translate according to centre indicator
            translate   ([] (transform_state const & state) { return -state.centre                                ;})
            // scale according to zoom
         ,  scale       ([] (transform_state const & state) { return state.zoom                                   ;})
         ,  terminator
      };

      transform_functor const modified_view_to_screen_transform_configuration [] =
      {
            // translate to centre
            translate   ([] (transform_state const & state) { return view_transform::create_vector (0.5  , 0.5)   ;})
            // restore size
         ,  scale       ([] (transform_state const & state) { return state.size                                   ;})
         ,  terminator
      };

      transform_functor const bitmap_to_view_functor = create_from_configuration (
            bitmap_to_view_transform_configuration
         );

      transform_functor const modify_view_functor = create_from_configuration (
            modify_view_transform_configuration
         );

      transform_functor const modified_view_to_screen_functor = create_from_configuration (
            modified_view_to_screen_transform_configuration
         );

      transform_functor const bitmap_to_screen_transform_configuration [] =
      {
            bitmap_to_view_functor
         ,  modify_view_functor
         ,  modified_view_to_screen_functor
         ,  transform_functor ()
      };

      transform_functor const view_to_screen_transform_configuration [] =
      {
            modify_view_functor
         ,  modified_view_to_screen_functor
         ,  transform_functor ()
      };

      transform_functor const view_to_screen_functor = create_from_configuration (
            view_to_screen_transform_configuration
         );

      transform_functor const bitmap_to_screen_transform_functor = create_from_configuration (
            bitmap_to_screen_transform_configuration
         );
   }

   transform const view_to_screen (
         transform_direction::type const direction
      ,  dimension const & size
      ,  vector const & centre
      ,  vector const & zoom
      )
   {
      return view_to_screen_functor (transform_state (
            direction
         ,  size
         ,  centre
         ,  zoom));
   }

   transform const bitmap_to_screen_transform (
         transform_direction::type const direction
      ,  dimension const & size
      ,  vector const & centre
      ,  vector const & zoom
      )
   {
      return bitmap_to_screen_transform_functor (transform_state (
            direction
         ,  size
         ,  centre
         ,  zoom));
   }
}
// ----------------------------------------------------------------------------
