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

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include "svg_styles.hpp"

namespace KRE
{
	namespace SVG
	{
		fill::fill() 
			: fill_rule_(FillRule::INHERIT), 
			stroke_linejoin_(LineJoin::INHERIT),
			stroke_linecap_(LineCap::INHERIT),
			stroke_miterlimit_(0.0),
			stroke_dashoffset_(0.0),
			opacity_set_(false),
			opacity_(1.0)
		{
		}

		fill::~fill() 
		{
		}

		void fill::set_fill_rule(const std::string& rule) 
		{
			if(rule == "nonzero") {
				fill_rule_ = FillRule::NONZERO;
			} else if(rule == "evenodd") {
				fill_rule_ = FillRule::EVENODD;
			} else {
				fill_rule_ = FillRule::INHERIT;
				std::cerr << "WARNING: Unrecognised fill-rule: " << rule << std::endl;
			}
		}
		
		void fill::apply_fill_rule(render_context& ctx) const 
		{
			if(fill_rule_ != FillRule::INHERIT) {
				switch(fill_rule_) {
				case FillRule::EVENODD:	cairo_set_fill_rule(ctx.cairo(), CAIRO_FILL_RULE_EVEN_ODD); break;
				case FillRule::NONZERO:	cairo_set_fill_rule(ctx.cairo(), CAIRO_FILL_RULE_WINDING); break;
				}
			}
		}

		void fill::set_fill_color(const std::string& color_string) 
		{
			fill_color_.reset(new paint(color_string));
		}

		void fill::apply_colors(render_context& ctx) const 
		{
			if(opacity_set_) {
				ctx.opacity_push(opacity_);
			}
			apply_fill_color(ctx);
			apply_stroke_color(ctx);
		}

		void fill::apply_fill_color(render_context& ctx) const 
		{
			if(fill_color_) {
				if(fill_color_->has_color()) {
					ctx.fill_color_push(paint(fill_color_->r(), fill_color_->g(), fill_color_->b(), int(ctx.opacity_top()*255.0)));

				/*auto pattern = cairo_get_source(cairo);
				switch(cairo_pattern_get_type(pattern)) {
					case CAIRO_PATTERN_TYPE_SOLID: {
						double r, g, b, a;
						cairo_pattern_get_rgba(pattern, &r, &g, &b, &a);
						std::cerr << "XXX: solid pattern set (" << r << "," << g << "," << b << "," << a << ")" << std::endl;
						break;
					}
					case CAIRO_PATTERN_TYPE_SURFACE:
						std::cerr << "XXX: surface pattern set" << std::endl;
						break;
					case CAIRO_PATTERN_TYPE_LINEAR:
						std::cerr << "XXX: linear pattern set" << std::endl;
						break;
					case CAIRO_PATTERN_TYPE_RADIAL:
						std::cerr << "XXX: radial pattern set" << std::endl;
						break;
					case CAIRO_PATTERN_TYPE_MESH:
						std::cerr << "XXX: mesh pattern set" << std::endl;
						break;
					case CAIRO_PATTERN_TYPE_RASTER_SOURCE:
						std::cerr << "XXX: raster source pattern set" << std::endl;
						break;
					default:
						ASSERT_LOG(false, "Unrecognised pattern type: " << cairo_pattern_get_type(pattern));
				}*/
				} else {
					ctx.fill_color_push(paint(0,0,0,0));
				}
			}
		}
		
		void fill::set_stroke_color(const std::string& color_string) 
		{
			stroke_color_.reset(new paint(color_string));
		}

		void fill::apply_stroke_color(render_context& ctx) const 
		{
			if(stroke_color_) {
				if(stroke_color_->has_color()) {
					ctx.stroke_color_push(paint(stroke_color_->r(), stroke_color_->g(), stroke_color_->b(), int(ctx.opacity_top()*255.0)));
				} else {
					ctx.stroke_color_push(paint(0,0,0,0));
				}
			}
		}
		void fill::unapply_colors(render_context& ctx) const {
			if(fill_color_) {
				ctx.fill_color_pop();
			}
			if(stroke_color_) {
				ctx.stroke_color_pop();
			}
			if(opacity_set_) {
				ctx.opacity_pop();
			}
		}
		
		void fill::set_stroke_width(const std::string& w) 
		{
			stroke_width_.reset(new svg_length(w));
		}

		void fill::apply_stroke_width(render_context& ctx) const 
		{
			if(stroke_width_) {
				cairo_set_line_width(ctx.cairo(), stroke_width_->value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER));
			}
		}

		void fill::set_stroke_linejoin(const std::string& lj) 
		{
			if(lj == "miter") {
				stroke_linejoin_ = LineJoin::MITER;
			} else if(lj == "round") {
				stroke_linejoin_ = LineJoin::ROUND;
			} else if(lj == "bevel") {
				stroke_linejoin_ = LineJoin::BEVEL;
			} else {
				stroke_linejoin_ = LineJoin::INHERIT;
			}
		}

		void fill::apply_stroke_linejoin(render_context& ctx) const 
		{
			switch (stroke_linejoin_) {
				case LineJoin::MITER: cairo_set_line_join(ctx.cairo(), CAIRO_LINE_JOIN_MITER); break;
				case LineJoin::ROUND: cairo_set_line_join(ctx.cairo(), CAIRO_LINE_JOIN_ROUND); break;
				case LineJoin::BEVEL: cairo_set_line_join(ctx.cairo(), CAIRO_LINE_JOIN_BEVEL); break;
				default: break;
			}
		}

		void fill::set_stroke_linecap(const std::string& lc) 
		{
			if(lc == "butt") {
				stroke_linecap_ = LineCap::BUTT;
			} else if(lc == "round") {
				stroke_linecap_ = LineCap::ROUND;
			} else if(lc == "square") {
				stroke_linecap_ = LineCap::SQUARE;
			} else {
				stroke_linecap_ = LineCap::INHERIT;
			}
		}

		void fill::apply_stroke_linecap(render_context& ctx) const 
		{
			switch(stroke_linecap_) {
				case LineCap::BUTT:		cairo_set_line_cap(ctx.cairo(), CAIRO_LINE_CAP_BUTT); break;
				case LineCap::ROUND:	cairo_set_line_cap(ctx.cairo(), CAIRO_LINE_CAP_ROUND); break;
				case LineCap::SQUARE:	cairo_set_line_cap(ctx.cairo(), CAIRO_LINE_CAP_SQUARE); break;
				default: break;
			}
		}

		void fill::set_stroke_miterlimit(const std::string& ml) 
		{
			try {
				stroke_miterlimit_ = boost::lexical_cast<double>(ml);
			} catch(const boost::bad_lexical_cast&) {
				ASSERT_LOG(false, "Unable to convert value: '" << ml << "' to a number");
			}
		}

		void fill::apply_stroke_miterlimit(render_context& ctx) const 
		{
			if(stroke_miterlimit_ != 0.0) {
				cairo_set_miter_limit(ctx.cairo(), stroke_miterlimit_);
			}
		}

		void fill::set_stroke_dashoffset(const std::string& offs) 
		{
			try {
				stroke_dashoffset_ = boost::lexical_cast<double>(offs);
			} catch(const boost::bad_lexical_cast&) {
				ASSERT_LOG(false, "Unable to convert value: '" << offs << "' to a number");
			}
		}

		void fill::set_stroke_dasharray(const std::string& da)
		{
			std::vector<svg_length> res;
			boost::char_separator<char> seperators(" \n\t\r,");
			boost::tokenizer<boost::char_separator<char>> tok(da, seperators);
			for(auto it = tok.begin(); it != tok.end(); ++it) {
				res.emplace_back(*it);
			}
			if(res.size() % 2 == 0) {
				stroke_dasharray_.swap(res);
			} else {
				stroke_dasharray_.resize(res.size() * 2);
				std::copy(res.begin(), res.end(), stroke_dasharray_.begin());
				std::copy(res.begin(), res.end(), stroke_dasharray_.begin() + res.size());
			}
		}

		void fill::apply_stroke_dasharray(render_context& ctx) const 
		{
			if(!stroke_dasharray_.empty()) {
				std::vector<double> dashes;
				for(auto& l : stroke_dasharray_) {
					dashes.emplace_back(l.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER));
				}
				cairo_set_dash(ctx.cairo(), &dashes[0], dashes.size(), stroke_dashoffset_);
			}
		}

		void fill::set_opacity(const std::string& opacity) {

			try {
				opacity_ = boost::lexical_cast<double>(opacity);
				opacity_set_ = true;
			} catch(const boost::bad_lexical_cast&) {
				ASSERT_LOG(false, "Unable to convert value: '" << opacity << "' to a number");
			}
		}

		void fill::set_cairo_values(render_context& ctx) const 
		{
			apply_fill_rule(ctx);
			apply_stroke_width(ctx);
			apply_stroke_linejoin(ctx);
			apply_stroke_linecap(ctx);
			apply_stroke_miterlimit(ctx);
			apply_stroke_dasharray(ctx);
		}

		font_properties::font_properties() 
			: font_style_(FontStyle::NORMAL),
			font_variant_(FontVariant::NORMAL),
			letter_spacing_inherited_(false),
			// XXX move this to a constants section
			letter_spacing_value_(0, svg_length::SVG_LENGTHTYPE_NUMBER),
			font_size_inherited_(false),
			// XXX move this to a constants section
			font_size_(12, svg_length::SVG_LENGTHTYPE_NUMBER)
		{
		}

		font_properties::~font_properties() 
		{
		}

		void font_properties::set_font_family(const std::string& font_family) 
		{
			font_family_ = font_family;
		}

		void font_properties::set_font_style(const std::string& style) 
		{
			if(style == "inherit") {
				font_style_ = FontStyle::INHERIT;
			} else if(style == "normal") {
				font_style_ = FontStyle::NORMAL;
			} else if(style == "italic") {
				font_style_ = FontStyle::ITALIC;
			} else if(style == "oblique") {
				font_style_ = FontStyle::OBLIQUE;
			} else {
				ASSERT_LOG(false, "Unrecognised font-style: " << style);
			}
		}

		void font_properties::set_font_variant(const std::string& var) 
		{
			if(var == "inherit") {
				font_variant_ = FontVariant::INHERIT;
			} else if(var == "normal") {
				font_variant_ = FontVariant::NORMAL;
			} else if(var == "small-caps") {
				font_variant_ = FontVariant::SMALL_CAPS;
			} else {
				ASSERT_LOG(false, "Unrecognised font-variant: " << var);
			}
		}

		void font_properties::set_font_size(const std::string& size) 
		{
			if(size == "inherit") {
				font_size_inherited_ = true;
			} else {
				font_size_.from_string(size);
			}
		}

		double font_properties::get_font_size(double parent_value) const 
		{ 
			if(font_size_inherited_) {
				return parent_value;
			}
			return font_size_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
		}

		void font_properties::set_letter_spacing(const std::string& spacing) 
		{
			if(spacing == "normal") {
				// XXX move this to a constants section
				letter_spacing_value_ = svg_length(0, svg_length::SVG_LENGTHTYPE_NUMBER);
				letter_spacing_inherited_ = false;
			} else if(spacing == "inherit") {
				letter_spacing_inherited_ = true;
			} else {
				letter_spacing_value_.from_string(spacing);
				letter_spacing_inherited_ = false;
			}
		}

		double font_properties::get_letter_spacing(double parent_value) const 
		{
			if(letter_spacing_inherited_) { 
				return parent_value;
			} 
			return letter_spacing_value_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
		}

		void font_properties::apply(render_context& ctx) const 
		{
			// XXX this is broken -- fix at a later date.
			cairo_font_slant_t slant = CAIRO_FONT_SLANT_NORMAL;
			switch(font_style_) {
			case FontStyle::INHERIT: break;
			case FontStyle::NORMAL: slant = CAIRO_FONT_SLANT_NORMAL; break;
			case FontStyle::ITALIC: slant = CAIRO_FONT_SLANT_ITALIC; break;
			case FontStyle::OBLIQUE: slant = CAIRO_FONT_SLANT_OBLIQUE; break;
			default: ASSERT_LOG(false, "Unknown font style");
			}
			cairo_select_font_face(ctx.cairo(), get_font_family().c_str(), slant, CAIRO_FONT_WEIGHT_NORMAL);
			/// XXX this is wrong because we need to inherit the parent values.
			// This applies to most stuff.
			cairo_set_font_size(ctx.cairo(), get_font_size(12));
		}
	}
}
