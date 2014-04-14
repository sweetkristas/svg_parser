/*
	Copyright (C) 2003-2013 by Kristina Simpson <sweet.kristas@gmail.com>
	
	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	   1. The origin of this software must not be misrepresented; you must not
	   claim that you wrote the original software. If you use this software
	   in a product, an acknowledgment in the product documentation would be
	   appreciated but is not required.

	   2. Altered source versions must be plainly marked as such, and must not be
	   misrepresented as being the original software.

	   3. This notice may not be removed or altered from any source
	   distribution.
*/

#pragma once

#include <cairo/cairo.h>
#include <string>
#include <vector>

#include "svg_length.hpp"
#include "svg_paint.hpp"
#include "svg_render.hpp"

namespace KRE
{
	namespace SVG
	{
		enum class FillRule {
			INHERIT,
			NONZERO,
			EVENODD,
		};
		enum class LineJoin {
			INHERIT,
			MITER,
			ROUND,
			BEVEL,
		};
		enum class LineCap {
			INHERIT,
			BUTT,
			ROUND,
			SQUARE,
		};

		class fill
		{
		public:
			fill();
			~fill();
			void set_fill_rule(FillRule rule) { fill_rule_ = rule; }
			void set_fill_rule(const std::string& rule);
			void apply_fill_rule(render_context& ctx) const;

			void apply_colors(render_context& ctx) const;
			void unapply_colors(render_context& ctx) const;

			void set_fill_color(const std::string& color_string);
			void apply_fill_color(render_context& ctx) const;
		
			void set_stroke_color(const std::string& color_string);
			void apply_stroke_color(render_context& ctx) const;
		
			void set_stroke_width(const std::string& w);
			void apply_stroke_width(render_context& ctx) const;

			void set_stroke_linejoin(const std::string& lj);
			void apply_stroke_linejoin(render_context& ctx) const;

			void set_stroke_linecap(const std::string& lc);
			void apply_stroke_linecap(render_context& ctx) const;

			void set_stroke_miterlimit(const std::string& ml);
			void apply_stroke_miterlimit(render_context& ctx) const;

			void set_stroke_dashoffset(const std::string& offs);
			
			void set_stroke_dasharray(const std::string& da);
			void apply_stroke_dasharray(render_context& ctx) const;

			void set_opacity(const std::string& opacity);
	
			void set_cairo_values(render_context& ctx) const;
		private:
			paint_ptr fill_color_;
			paint_ptr stroke_color_;
			FillRule fill_rule_;
			std::shared_ptr<svg_length> stroke_width_;
			LineJoin stroke_linejoin_;
			LineCap stroke_linecap_;
			double stroke_miterlimit_;
			std::vector<svg_length> stroke_dasharray_;
			double stroke_dashoffset_;
			bool opacity_set_;
			double opacity_;
		};


		class font_properties
		{
		public:
			font_properties();
			~font_properties();

			void set_font_family(const std::string& font_family);
			const std::string& get_font_family() const { return font_family_; }

			void set_font_style(const std::string& style);
			FontStyle get_font_style() const { return font_style_; }

			void set_font_variant(const std::string& var);
			FontVariant get_font_variant() const { return font_variant_; }

			void set_font_size(const std::string& size);
			double get_font_size(double parent_value) const;

			void set_letter_spacing(const std::string& spacing);
			double get_letter_spacing(double parent_value) const;

			void apply(render_context& ctx) const;
		private:
			std::string font_family_;
			FontStyle font_style_;
			FontVariant font_variant_;
			bool font_size_inherited_;
			svg_length font_size_;
			// font-weight : 	normal | bold | bolder | lighter | 100 | 200 | 300 | 400 | 500 | 600 | 700 | 800 | 900 | inherit
			// font-stretch :	normal | wider | narrower | ultra-condensed | extra-condensed | condensed | semi-condensed | semi-expanded | expanded | extra-expanded | ultra-expanded | inherit
			// font-size-adjust : 	<number> | none | inherit
			// kerning : auto | <length> | inherit
			bool letter_spacing_inherited_;
			svg_length letter_spacing_value_;
			// word-spacing : 	normal | <length> | inherit
			// text-decoration : none | [ underline || overline || line-through || blink ] | inherit
		};
	}
}
