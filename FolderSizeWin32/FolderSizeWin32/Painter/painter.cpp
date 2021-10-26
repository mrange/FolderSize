/* ****************************************************************************
 *
 * Copyright (c) Mårten Rånge.
 *
 * This source code is subject to terms and conditions of the Microsoft Public License. A
 * copy of the license can be found in the License.html folder_infole at the root of this distribution. If
 * you cannot locate the  Microsoft Public License, please send an email to
 * dlr@microsoft.com. By using this source code in any fashion, you are agreeing to be bound
 * by the terms of the Microsoft Public License.
 *
 * You must not remove this notice, or any other, from this software.
 *
 *
 * ***************************************************************************/

// ----------------------------------------------------------------------------
#include "stdafx.h"
// ----------------------------------------------------------------------------
#undef max
#undef min
// ----------------------------------------------------------------------------
#  include <boost/pool/pool_alloc.hpp>
// ----------------------------------------------------------------------------
#include <algorithm>
#include <functional>
#include <vector>
// ----------------------------------------------------------------------------
#include "../linear.hpp"
#include "../utility.hpp"
#include "../theme.hpp"
#include "../view_transform.hpp"
#include "../win32.hpp"
// ----------------------------------------------------------------------------
#include "painter.hpp"
// ----------------------------------------------------------------------------
namespace painter
{
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   namespace b    = boost           ;
   namespace f    = folder          ;
   namespace s    = std             ;
   namespace w    = win32           ;
   namespace l    = linear          ;
   namespace u    = utility         ;
   namespace vt   = view_transform  ;
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   namespace
   {
      s::size_t const   buffer_size    = 256;
      double const      cut_off_y      = 10.0;

      typedef s::vector<rendered_folder> rendered_folders;

      struct update_request : boost::noncopyable
      {
         typedef s::auto_ptr<update_request> ptr;

         update_request (
               folder::folder const * const           root_
            ,  new_frame_available_callback const &   new_frame_available_
            ,  s::size_t const                        processed_folder_count_
            ,  s::size_t const                        unprocessed_folder_count_
            ,  select_property::type const            select_property_
            ,  coordinate const &                     centre_
            ,  zoom_factor const &                    zoom_
            ,  dimension const &                      screen_size_
            ,  int const                              bitmap_bits_
            ,  int const                              bitmap_planes_
            )
            :  root                          (root_                           )
            ,  new_frame_available           (new_frame_available_            )
            ,  processed_folder_count        (processed_folder_count_         )
            ,  unprocessed_folder_count      (unprocessed_folder_count_       )
            ,  select_property               (select_property_                )
            ,  centre                        (centre_                         )
            ,  zoom                          (zoom_                           )
            ,  bitmap_size                   (screen_size_                    )
            ,  bitmap_bits                   (bitmap_bits_                    )
            ,  bitmap_planes                 (bitmap_planes_                  )
         {
         }

         folder::folder const * const  root                                ;

         new_frame_available_callback const  new_frame_available           ;
         s::size_t const                     processed_folder_count        ;
         s::size_t const                     unprocessed_folder_count      ;
         select_property::type const         select_property               ;
         coordinate const                    centre                        ;
         zoom_factor const                   zoom                          ;
         dimension const                     bitmap_size                   ;
         int const                           bitmap_bits                   ;
         int const                           bitmap_planes                 ;
      };

      HBITMAP const create_bitmap (
            int const bitmap_bits
         ,  int const bitmap_planes
         ,  dimension const & bitmap_size
         )
      {
         auto cx = bitmap_size.x ();
         auto cy = bitmap_size.y ();

#ifdef _DEBUG
         TCHAR buffer[buffer_size] = {0};
         _stprintf_s (
               buffer
            ,  WIN32_PRELUDE _T ("Creating bitmap %f, %f, %d, %d")
            , cx
            , cy
            , bitmap_bits
            , bitmap_planes
         );
         WIN32_DEBUG_STRING (buffer);
#endif

         return
            CreateBitmap (
                  IMPLICIT_CAST (cx)
               ,  IMPLICIT_CAST (cy)
               ,  bitmap_planes
               ,  bitmap_bits
               ,  nullptr);
      }

      struct update_response : boost::noncopyable
      {
         typedef s::auto_ptr<update_response> ptr;

         update_response (
               update_request    &  update_request_
            ,  s::size_t const      frame_count_
            )
            :  select_property         (update_request_.select_property       )
            ,  centre                  (update_request_.centre                )
            ,  zoom                    (update_request_.zoom                  )
            ,  bitmap_size             (update_request_.bitmap_size           )
            ,  bitmap                  (create_bitmap (
                  update_request_.bitmap_bits
               ,  update_request_.bitmap_planes
               ,  update_request_.bitmap_size
               )                                                              )
            ,  processed_folder_count  (update_request_.processed_folder_count)
            ,  frame_count             (frame_count_                          )
         {
            rendered_folders.reserve (64);
         }

         select_property::type const      select_property         ;
         coordinate const                 centre                  ;
         zoom_factor const                zoom                    ;
         dimension const                  bitmap_size             ;
         win32::gdi_object<HBITMAP> const bitmap                  ;
         rendered_folders                 rendered_folders        ;
         s::size_t const                  processed_folder_count  ;
         s::size_t const                  frame_count             ;
      };

//      typedef st::function<big_size const (f::folder const &)> TPropertyPickerPredicate;
      typedef big_size const (*TPropertyPickerPredicate)(f::folder const &);

      big_size const size_picker (
         f::folder const & f
         )
      {
         return f.get_total_size ();
      }

      big_size const physical_size_picker (
         f::folder const & f
         )
      {
         return f.get_total_physical_size ();
      }

      big_size const count_picker (
         f::folder const & f
         )
      {
         return f.get_total_file_count ();
      }


      big_size const inaccessible_picker (
         f::folder const & f
         )
      {
         return f.get_total_inaccessible_folder_count ();
      }


      struct folder_traverser_context : b::noncopyable
      {
         folder_traverser_context (
               HDC const                           hdc_
            ,  rendered_folders &                  rendered_folders_
            ,  vt::transform const &               transform_
            ,  dimension const &                   size_
            ,  w::file_time const                  current_time_
            ,  TPropertyPickerPredicate const &    property_picker_
            )
            :  hdc               (hdc_             )
            ,  rendered_folders  (rendered_folders_)
            ,  transform         (transform_       )
            ,  size              (size_            )
            ,  current_time      (current_time_    )
            ,  property_picker   (property_picker_ )
         {
         }

         HDC const                        hdc               ;
         rendered_folders &               rendered_folders  ;
         vt::transform const              transform         ;
         dimension const                  size              ;
         w::file_time const               current_time      ;
         TPropertyPickerPredicate const   property_picker   ;
      };

      struct background_painter
      {
         background_painter ()
            :  new_frame_request (w::event_type::auto_reset       )
            ,  shutdown_request  (w::event_type::manual_reset     )
            ,  thread            (_T ("painter"), create_proc ()  )
            ,  frame_count       (0                               )
         {
         }

         ~background_painter () noexcept
         {
            shutdown_request.set ();
            thread.join (10000);
         }

         w::thread_safe_scoped_ptr<update_request>    update_request_value    ;
         w::thread_safe_scoped_ptr<update_response>   update_response_value   ;
         w::event                                     new_frame_request       ;

      private:
         w::thread::proc create_proc () noexcept
         {
            return s::bind (&background_painter::proc, this);
         }

         enum folder_operation_result
         {
            yes_folder_painted   ,
            no_folder_too_small  ,
            no_other_reason      ,
            enum_count           ,
         };

         template<
               typename TPaintPredicate
            >
         static folder_operation_result const folder_traverser_paint_impl (
               folder_traverser_context & context
            ,  s::size_t                  remaining_levels
            ,  big_size const             property
            ,  double const               x
            ,  double const               y
            ,  double const               x_step_ratio
            ,  double const               y_step_ratio
            ,  TPaintPredicate            paint_predicate
            )
         {
            if (remaining_levels == 0)
            {
               return no_other_reason;
            }

            auto current_x             = x;
            auto current_y             = y;
            auto height                = property * y_step_ratio;

            auto current_left_top      = context.transform * vt::create_extended_vector (
                  current_x
               ,  current_y
               );
            auto current_right_bottom  = context.transform * vt::create_extended_vector (
                  current_x + x_step_ratio
                , current_y + height
                );
            auto adjusted_height       = current_right_bottom.y () - current_left_top.y ();

            if (adjusted_height < cut_off_y)
            {
               return no_folder_too_small;
            }

            if (current_left_top.x () > context.size.x ())
            {
               return no_other_reason;
            }

            if (current_left_top.y () > context.size.y ())
            {
               return no_other_reason;
            }

            if (
                  current_right_bottom.x () > 0.0
               && current_right_bottom.y () > 0.0
               && current_left_top.x () < context.size.x ()
               && current_left_top.y () < context.size.y ()
               )
            {
               RECT bitmap_rect  = {0}                      ;
               bitmap_rect.left         = IMPLICIT_CAST (current_left_top.x ()       );
               bitmap_rect.top          = IMPLICIT_CAST (current_left_top.y ()       );
               bitmap_rect.right        = IMPLICIT_CAST (current_right_bottom.x ()   );
               bitmap_rect.bottom       = IMPLICIT_CAST (current_right_bottom.y ()   );

               view_rect view_rect                                            ;
               view_rect.values[0]      = current_x                           ;
               view_rect.values[1]      = current_y                           ;
               view_rect.values[2]      = current_x + x_step_ratio            ;
               view_rect.values[3]      = current_y + height                  ;

               paint_predicate (
                     bitmap_rect
                  ,  view_rect
                  );
            }

            return yes_folder_painted;
         }
         static folder_operation_result const folder_traverser_impl (
               folder_traverser_context & context
            ,  s::size_t const            remaining_levels
            ,  double const               x
            ,  double const               y
            ,  double const               x_step_ratio
            ,  double const               y_step_ratio
            ,  f::folder const * const    folder
            )
         {
            if (remaining_levels == 0)
            {
               return no_other_reason;
            }

            if (!folder)
            {
               return no_other_reason;
            }

            auto property              = context.property_picker (*folder);

            auto folder_painter = [&context, folder, property] (
                  RECT const &      bitmap_rect
               ,  view_rect const & view_rect
               )
               {
                  context.rendered_folders.push_back (
                     rendered_folder (
                           bitmap_rect
                        ,  view_rect
                        ,  folder
                        ));

                  folder_painter_impl (
                        context
                     ,  property
                     ,  bitmap_rect
                     ,  *folder
                     );
               };
            auto folder_paint_result = folder_traverser_paint_impl (
                  context
               ,  remaining_levels
               ,  property
               ,  x
               ,  y
               ,  x_step_ratio
               ,  y_step_ratio
               ,  folder_painter
               );

            if (folder_paint_result != yes_folder_painted)
            {
               return folder_paint_result;
            }

            auto folder_count = folder->folder_count;

            typedef std::pair<f::folder const *, big_size> sort_folder;

            s::vector<
                     sort_folder
                  ,  b::pool_allocator<sort_folder>
               > sorted_folders;
            sorted_folders.resize (static_cast<s::size_t> (folder_count));

            s::transform (
                  folder->sub_folders.get ()
               ,  folder->sub_folders.get () + folder_count
               ,  sorted_folders.begin ()
               ,  [&context] (f::folder const * const f)
                  {
                     return f
                        ?  sort_folder (f, context.property_picker (*f))
                        :  sort_folder (f, std::numeric_limits<big_size>::min ());
                  }
               );

            s::stable_sort (
                  sorted_folders.begin ()
               ,  sorted_folders.end ()
               ,  [] (sort_folder const & left, sort_folder right)
                     {
                        return right.second < left.second;
                     }
               );

            auto current_x                = x;
            auto current_y                = y;
            auto next_x                   = current_x + x_step_ratio;
            big_size accumulated_property = 0;

            for (s::size_t iter = 0; iter < folder_count; ++iter)
            {
               sort_folder const & current_sort_folder   = sorted_folders[iter];
               auto sub_folder                           = current_sort_folder.first;

               if (!sub_folder)
               {
                  continue;
               }

               auto sub_property                         = current_sort_folder.second;
               auto y_step                               = y_step_ratio * sub_property;
               auto next_y                               = current_y + y_step;

               auto folder_traverse_result   = folder_traverser_impl (
                     context
                  ,  remaining_levels - 1
                  ,  next_x
                  ,  current_y
                  ,  x_step_ratio
                  ,  y_step_ratio
                  ,  sub_folder
                  );

               if (folder_traverse_result == no_folder_too_small)
               {
                  accumulated_property += sub_property;
               }
               else
               {
                  current_y = next_y;
               }
            }

            if (accumulated_property > 0)
            {
               auto remaining_painter = [&context, accumulated_property] (
                     RECT const &      bitmap_rect
                  ,  view_rect const & view_rect
                  )
                  {
                     remaining_folder_painter_impl (
                           context
                        ,  accumulated_property
                        ,  bitmap_rect
                        );
                  };
               folder_traverser_paint_impl (
                     context
                  ,  remaining_levels - 1
                  ,  accumulated_property
                  ,  next_x
                  ,  current_y
                  ,  x_step_ratio
                  ,  y_step_ratio
                  ,  remaining_painter
                  );
            }

            return yes_folder_painted;
         }

         static void folder_traverser (
               folder_traverser_context & context
            ,  f::folder const * const    root
            )
         {
            if (!root)
            {
               return;
            }
            if (!(context.size.x () > 0 && context.size.y () > 0))
            {
               return;
            }

            auto depth     = root->get_depth ();
            auto property  = context.property_picker (*root);

            if (property > 0 && depth > 0)
            {
               auto x_step_ratio = 1.0 / depth;
               auto y_step_ratio = 1.0 / property;

               folder_traverser_impl (
                     context
                  ,  depth
                  ,  -0.5
                  ,  -0.5
                  ,  x_step_ratio
                  ,  y_step_ratio
                  ,  root
                  );
            }
         }

         static void remaining_folder_painter_impl (
               folder_traverser_context & context
            ,  big_size const             total_size
            ,  RECT const &               rect
            )
         {
            auto fill_predicate = [] (HDC const hdc, RECT const & rect)
            {
               FillRect (
                     hdc
                  ,  &rect
                  ,  theme::folder_tree::merged_folder_background_brush.value
                  );
            };

            painter_impl (
                  context
               ,  total_size
               ,  rect
               ,  fill_predicate
               ,  theme::folder_tree::merged_folder_string.c_str ()
               );
         }

         static void folder_painter_impl (
               folder_traverser_context & context
            ,  big_size const             total_size
            ,  RECT const &               rect
            ,  f::folder const &          folder
            )
         {

            auto use_color = theme::folder_tree::no_activity;
            auto ticks_since_last_activity = context.current_time - folder.last_activity;

            if (ticks_since_last_activity < w::ticks_per_day / 24)
            {
               use_color = theme::folder_tree::last_hour    ;
            }
            else if (ticks_since_last_activity < w::ticks_per_day * 1)
            {
               use_color = theme::folder_tree::last_day     ;
            }
            else if (ticks_since_last_activity < w::ticks_per_day * 7)
            {
               use_color = theme::folder_tree::last_7day    ;
            }
            else if (ticks_since_last_activity < w::ticks_per_day * 31)
            {
               use_color = theme::folder_tree::last_31day   ;
            }
            else if (ticks_since_last_activity < w::ticks_per_day * 365)
            {
               use_color = theme::folder_tree::last_365day  ;
            }

            if (folder.size > folder.physical_size)
            {
               auto fill_predicate =
                  [use_color] (HDC const hdc, RECT const & rect)
                  {
                     auto top_color    = theme::folder_tree::folder_background_color[use_color];
                     auto bottom_color = RGB (
                           (3 * GetRValue (top_color)) / 4
                        ,  (3 * GetGValue (top_color)) / 4
                        ,  (3 * GetBValue (top_color)) / 4
                        );
                     w::gradient_fill (
                           hdc
                        ,  rect
                        ,  top_color
                        ,  bottom_color
                        );
                  };

               painter_impl (
                     context
                  ,  total_size
                  ,  rect
                  ,  fill_predicate
                  ,  folder.name.c_str ()
                  );
            }
            else
            {
               auto fill_predicate =
                  [use_color] (HDC const hdc, RECT const & rect)
                  {
                     FillRect (
                           hdc
                        ,  &rect
                        ,  theme::folder_tree::folder_background_brush[use_color].value
                        );
                  };

               painter_impl (
                     context
                  ,  total_size
                  ,  rect
                  ,  fill_predicate
                  ,  folder.name.c_str ()
                  );
            }
         }

         template<typename TFillPredicate>
         static void painter_impl (
               folder_traverser_context & context
            ,  big_size const             total_size
            ,  RECT const &               rect
            ,  TFillPredicate             fill_predicate
            ,  LPCTSTR const              description
            )
         {
            FS_ASSERT (description);

            RECT copy_rect = rect;

            TCHAR buffer[buffer_size] = {0};

            int cch = 0;

            if (total_size > 1E8)
            {
               cch = _stprintf_s (
                     buffer
                  ,  _T ("%s\r\n%.1fG")
                  ,  description
                  ,  total_size / 1E9
                  );
            }
            else if (total_size > 1E5)
            {
               cch = _stprintf_s (
                     buffer
                  ,  _T ("%s\r\n%.1fM")
                  ,  description
                  ,  total_size / 1E6
                  );
            }
            else if (total_size > 1E2)
            {
               cch = _stprintf_s (
                     buffer
                  ,  _T ("%s\r\n%.1fk")
                  ,  description
                  ,  total_size / 1E3
                  );
            }
            else
            {
               cch = _stprintf_s (
                     buffer
                  ,  _T ("%s\r\n%I64d")
                  ,  description
                  ,  total_size
                  );
            }

            fill_predicate (context.hdc, copy_rect);

            DrawText (
                  context.hdc
               ,  buffer
               ,  cch
               ,  &copy_rect
               ,  DT_WORD_ELLIPSIS
               );

            copy_rect.right   += 1;
            copy_rect.bottom  += 1;

            FrameRect (
                  context.hdc
               ,  &copy_rect
               ,  theme::folder_tree::folder_foreground_brush.value
               );

         }

         unsigned int proc ()
         {
            HANDLE wait_handles[] =
               {
                     new_frame_request.value.value
                  ,  shutdown_request.value.value
               };

            auto continue_loop   = true   ;
            DWORD wait_result    = 0      ;

            do
            {
               wait_result = WaitForMultipleObjects (
                     2
                  ,  wait_handles
                  ,  false
                  ,  1000);


               update_request::ptr request_ptr;

               switch (wait_result)
               {
               case WAIT_OBJECT_0 + 0:
                  while ((request_ptr = update_request_value.reset ()).get ())
                  {

#ifdef _DEBUG
                     TCHAR buffer[buffer_size] = {0};
#endif

                     ++frame_count;

#ifdef _DEBUG
                     _stprintf_s (
                           buffer
                        ,  WIN32_PRELUDE _T ("Received : New view request: %d")
                        ,  frame_count
                        );
                     WIN32_DEBUG_STRING (buffer);
#endif

                     if (request_ptr->root)
                     {
                        auto response_ptr = update_response::ptr (
                           new update_response (*request_ptr, frame_count));

                        {
                           w::device_context bitmap_dc (CreateCompatibleDC (nullptr));
                           w::select_object const select_bitmap (
                                 bitmap_dc.value
                              ,  response_ptr->bitmap.value
                              );

                           SetBkMode (
                                 bitmap_dc.value
                              ,  TRANSPARENT
                              );

                           RECT rect   = {0};
                           rect.right  = IMPLICIT_CAST (request_ptr->bitmap_size.x ());
                           rect.bottom = IMPLICIT_CAST (request_ptr->bitmap_size.y ());

                           FillRect (
                                 bitmap_dc.value
                              ,  &rect
                              ,  theme::folder_tree::background_brush.value
                              );

                           SetBkColor (
                                 bitmap_dc.value
                              ,  theme::folder_tree::background_color);

                           SetTextColor (
                                 bitmap_dc.value
                              ,  theme::folder_tree::foreground_color);

                           {
                              w::select_object const select_font (
                                    bitmap_dc.value
                                    ,  theme::default_monospace_font.value
                                 );

                              TCHAR buffer [buffer_size * 8] = {0};
                              auto cch = _stprintf_s (
                                    buffer
                                 ,  theme::folder_tree::progress_string.c_str ()
                                 ,  request_ptr->unprocessed_folder_count
                                 ,  request_ptr->processed_folder_count
                                 ,  request_ptr->root->get_depth ()
                                 );

                              DrawText (
                                    bitmap_dc.value
                                 ,  buffer
                                 ,  cch
                                 ,  &rect
                                 , DT_RIGHT
                                 );
                           }

                           w::select_object const select_font (
                                 bitmap_dc.value
                              ,  theme::default_font.value
                              );

                           {
                              auto top = 100;
                              for (auto iter = 0; iter < u::size_of_array (theme::folder_tree::color_legends); ++iter)
                              {
                                 {
                                    RECT sub_rect = {0};
                                    sub_rect.top      = top + 4;
                                    sub_rect.left     = rect.right - 24;
                                    sub_rect.bottom   = top + 16 + 4;
                                    sub_rect.right    = rect.right - 8;

                                    FillRect (
                                          bitmap_dc.value
                                       ,  &sub_rect
                                       ,  theme::folder_tree::color_legends[iter].brush
                                       );
                                 }
                                 {
                                    RECT sub_rect = {0};
                                    sub_rect.top      = top;
                                    sub_rect.left     = 0;
                                    sub_rect.right    = rect.right - 32;
                                    sub_rect.bottom   = top + 24;

                                    DrawText (
                                          bitmap_dc.value
                                       ,  theme::folder_tree::color_legends[iter].text.c_str ()
                                       ,  -1
                                       ,  &sub_rect
                                       , DT_RIGHT | DT_VCENTER | DT_SINGLELINE
                                       );
                                 }
                                 top += 24;
                              }
                           }

                           if (theme::brand_bitmap.is_valid ())
                           {
                              w::device_context brand_bitmap_dc (CreateCompatibleDC (bitmap_dc.value));

                              w::select_object select_bitmap (
                                    brand_bitmap_dc.value
                                    ,  theme::brand_bitmap.value
                                    );

                              auto brand_bitmap_size = w::get_bitmap_size (
                                    bitmap_dc.value
                                 ,  theme::brand_bitmap.value
                                 );

                              BitBlt (
                                    bitmap_dc.value
                                 ,  rect.right - brand_bitmap_size.cx - 8
                                 ,  rect.bottom - brand_bitmap_size.cy - 16
                                 ,  brand_bitmap_size.cx
                                 ,  brand_bitmap_size.cy
                                 ,  brand_bitmap_dc.value
                                 ,  0
                                 ,  0
                                 ,  SRCCOPY
                                 );
                           }


                           DrawText (
                                 bitmap_dc.value
                              ,  theme::folder_tree::info_string.c_str ()
                              ,  -1
                              ,  &rect
                              , DT_RIGHT | DT_BOTTOM | DT_SINGLELINE
                              );

                           SetTextColor (
                                 bitmap_dc.value
                              ,  theme::folder_tree::folder_foreground_color);

                           auto bitmap_size  = request_ptr->bitmap_size;
                           auto centre       = request_ptr->centre;
                           auto zoom         = request_ptr->zoom;

                           auto current_transform = vt::view_to_screen (
                                 vt::transform_direction::forward
                              ,  bitmap_size
                              ,  centre
                              ,  zoom);

                           auto property_picker = &size_picker;

                           switch (request_ptr->select_property)
                           {
                           case select_property::size:
                              property_picker = &size_picker;
                              break;
                           case select_property::physical_size:
                              property_picker = &physical_size_picker;
                              break;
                           case select_property::count:
                              property_picker = &count_picker;
                              break;
                           case select_property::inaccessible:
                              property_picker = &inaccessible_picker;
                              break;
                           default:
                              FS_ASSERT (false);
                              break;
                           }

                           folder_traverser_context folder_traverser_context (
                                 bitmap_dc.value
                              ,  response_ptr->rendered_folders
                              ,  current_transform
                              ,  request_ptr->bitmap_size
                              ,  w::to_file_time (w::get_current_time ())
                              ,  property_picker
                              );

                           folder_traverser (
                                 folder_traverser_context
                              ,  request_ptr->root
                              );

#ifdef _DEBUG
                           SetTextColor (
                                 bitmap_dc.value
                                 ,  theme::folder_tree::foreground_color);

                           _stprintf_s (
                                 buffer
                              ,   _T ("frame_count : %d")
                              ,  frame_count
                              );

                           DrawText (
                                 bitmap_dc.value
                              ,  buffer
                              ,  -1
                              ,  &rect
                              , DT_BOTTOM | DT_SINGLELINE
                              );
#endif
                        }
                        update_response_value.reset (response_ptr.release ());

#ifdef _DEBUG
                        _stprintf_s (
                              buffer
                           ,  WIN32_PRELUDE _T ("Sending  messages::new_view_available (%d)")
                           ,  frame_count
                           );
                        WIN32_DEBUG_STRING (buffer);
#endif

                        if (request_ptr->new_frame_available)
                        {
                           request_ptr->new_frame_available ();
                        }
                     }
                  }
                  continue_loop = true;
                  break;
               case WAIT_TIMEOUT:
                  continue_loop = true;
                  break;
               case WAIT_OBJECT_0 + 1:
                  WIN32_DEBUG_STRING (WIN32_PRELUDE _T ("Received : Terminate painter"));
                  continue_loop = false;
                  break;
               case WAIT_ABANDONED:
                  WIN32_DEBUG_STRING (WIN32_PRELUDE _T ("Received : Abandon painter"));
                  continue_loop = false;
                  break;
               default:
                  {
                  TCHAR buffer[buffer_size] = {0};
                  _stprintf_s (
                        buffer
                     ,  _T ("Received : Unknown request : %X (@err = %X)")
                     ,  wait_result
                     ,  GetLastError ()
                     );
                  w::trace_string (buffer);
                  continue_loop = false;
                  }
                  break;
               }
            }
            while (continue_loop);

            return EXIT_SUCCESS;
         }

         w::event                                     shutdown_request  ;
         w::thread                                    thread            ;

         int                                          frame_count       ;
      };
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct painter::impl
   {
      void do_request (
            folder::folder const * const           root
         ,  new_frame_available_callback const &   new_frame_available
         ,  s::size_t const                        processed_folder_count
         ,  s::size_t const                        unprocessed_folder_count
         ,  select_property::type                  select_property
         ,  RECT const &                           rect
         ,  int const                              bitmap_bits
         ,  int const                              bitmap_planes
         ,  coordinate const &                     centre
         ,  zoom_factor const &                    zoom
         )
      {
         dimension screen_size;
         screen_size.x (rect.right - rect.left);
         screen_size.y (rect.bottom - rect.top);

         auto request = update_request::ptr (
               new update_request (
                     root
                  ,  new_frame_available
                  ,  processed_folder_count
                  ,  unprocessed_folder_count
                  ,  select_property
                  ,  centre
                  ,  zoom
                  ,  screen_size
                  ,  bitmap_bits
                  ,  bitmap_planes
                  ));

         background_painter.update_request_value.reset (request.release ());
         WIN32_DEBUG_STRING (WIN32_PRELUDE _T ("Sending : New view request"));
         background_painter.new_frame_request.set ();
      }

      void paint (
            folder::folder const * const           root
         ,  new_frame_available_callback const &   new_frame_available
         ,  HDC const                              hdc
         ,  s::size_t const                        processed_folder_count
         ,  s::size_t const                        unprocessed_folder_count
         ,  select_property::type                  select_property
         ,  RECT const &                           rect
         ,  coordinate const &                     centre
         ,  zoom_factor const &                    zoom
         )
      {
         {
            auto response = background_painter.update_response_value.reset ();
            if (response.get ())
            {
               update_response = response;
            }
         }

         dimension screen_size;
         screen_size.x (rect.right - rect.left);
         screen_size.y (rect.bottom - rect.top);

         if (
               update_response.get ()
            && update_response->select_property          == select_property
            && update_response->centre                   == centre
            && update_response->zoom                     == zoom
            && update_response->bitmap_size              == screen_size
            && update_response->processed_folder_count   == processed_folder_count
            )
         {
         }
         else
         {
            do_request (
                  root
               ,  new_frame_available
               ,  processed_folder_count
               ,  unprocessed_folder_count
               ,  select_property
               ,  rect
               ,  GetDeviceCaps (hdc   , BITSPIXEL )
               ,  GetDeviceCaps (hdc   , PLANES    )
               ,  centre
               ,  zoom
               );
         }

         if (update_response.get ())
         {
            w::device_context src_dc      (CreateCompatibleDC (hdc)                    );
            w::select_object  src_select  (src_dc.value, update_response->bitmap.value );

            if (update_response->bitmap_size.x () < screen_size.x ())
            {
               RECT sub_rect = {0};
               sub_rect.left  = rect.left + EXPLICIT_CAST (LONG, update_response->bitmap_size.x ())   ;
               sub_rect.top   = rect.top  + 0                                                   ;
               sub_rect.right = rect.left + EXPLICIT_CAST (LONG, screen_size.x ())                    ;
               sub_rect.bottom= rect.top  + EXPLICIT_CAST (LONG, screen_size.y ())                    ;
               FillRect (
                     hdc
                  ,  &sub_rect
                  ,  theme::folder_tree::background_brush.value
                  );
            }

            if (update_response->bitmap_size.y () < screen_size.y ())
            {
               RECT sub_rect = {0};
               sub_rect.left  = rect.left + 0                                                   ;
               sub_rect.top   = rect.top  + EXPLICIT_CAST (LONG, update_response->bitmap_size.y ())   ;
               sub_rect.right = rect.left + EXPLICIT_CAST (LONG, screen_size.x ())                    ;
               sub_rect.bottom= rect.top  + EXPLICIT_CAST (LONG, screen_size.y ())                    ;
               FillRect (
                     hdc
                  ,  &sub_rect
                  ,  theme::folder_tree::background_brush.value
                  );
            }

            BitBlt (
                  hdc
               ,  rect.left
               ,  rect.top
               ,  IMPLICIT_CAST (screen_size.x ())
               ,  IMPLICIT_CAST (screen_size.y ())
               ,  src_dc.value
               ,  0
               ,  0
               ,  SRCCOPY
               );
#ifdef _DEBUG
               TCHAR buffer[buffer_size] = {0};
               _stprintf_s (
                     buffer
                  ,  WIN32_PRELUDE _T ("BitBlt : Painting : %d")
                  ,  update_response->frame_count
                  );
               WIN32_DEBUG_STRING (buffer);
#endif

         }
         else
         {
            FillRect (
                  hdc
               ,  &rect
               ,  theme::folder_tree::background_brush.value
               );

         }
      }

      rendered_folder _empty_rendered_folder;

      rendered_folder const & hit_test (
            POINT const & offset
         )
      {
         if (!update_response.get ())
         {
            return _empty_rendered_folder;
         }

         rendered_folders const & current_rendered_folders = update_response->rendered_folders;

         auto end    = current_rendered_folders.end ();
         auto find   = s::find_if (
               current_rendered_folders.begin ()
            ,  end
            ,  [offset] (rendered_folder const & rf)
               {
                  return w::is_inside (rf.render_rect, offset);
               });

         if (find == end)
         {
            return _empty_rendered_folder;
         }

         return *find;
      }

      background_painter                              background_painter;
      update_response::ptr                            update_response   ;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   rendered_folder::rendered_folder ()
      :  render_rect (w::zero_rect ()  )
      ,  folder      (nullptr          )
   {
   }

   rendered_folder::rendered_folder (
         RECT const &                  render_rect_
      ,  ::painter::view_rect const  & view_rect_
      ,  f::folder const * const       folder_
      )
      :  render_rect (render_rect_  )
      ,  view_rect   (view_rect_    )
      ,  folder      (folder_       )
   {
   }

   rendered_folder::rendered_folder (rendered_folder const && rf)
      :  render_rect (rf.render_rect   )
      ,  view_rect   (rf.view_rect     )
      ,  folder      (rf.folder        )
   {
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   painter::painter ()
      :  m_impl (new impl ())
   {
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   painter::~painter ()
   {
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   void painter::do_request (
         folder::folder const * const           root
      ,  new_frame_available_callback const &   new_frame_available
      ,  s::size_t const                        processed_folder_count
      ,  s::size_t const                        unprocessed_folder_count
      ,  select_property::type                  select_property
      ,  RECT const &                           rect
      ,  coordinate const &                     centre
      ,  zoom_factor const &                    zoom
      )
   {
      w::device_context dc (CreateCompatibleDC (nullptr));
      m_impl->do_request (
            root
         ,  new_frame_available
         ,  processed_folder_count
         ,  unprocessed_folder_count
         ,  select_property
         ,  rect
         ,  GetDeviceCaps (dc.value , BITSPIXEL )
         ,  GetDeviceCaps (dc.value , PLANES    )
         ,  centre
         ,  zoom
         );
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   void painter::paint (
         folder::folder const * const           root
      ,  new_frame_available_callback const &   new_frame_available
      ,  HDC const                              hdc
      ,  s::size_t const                        processed_folder_count
      ,  s::size_t const                        unprocessed_folder_count
      ,  select_property::type                  select_property
      ,  RECT const &                           rect
      ,  coordinate const &                     centre
      ,  zoom_factor const &                    zoom
      )
   {
      m_impl->paint (
            root
         ,  new_frame_available
         ,  hdc
         ,  processed_folder_count
         ,  unprocessed_folder_count
         ,  select_property
         ,  rect
         ,  centre
         ,  zoom
         );
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   rendered_folder const & painter::hit_test (
         POINT const & offset
      )
   {
      return m_impl->hit_test (
            offset
         );
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
