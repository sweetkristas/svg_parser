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

#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/tokenizer.hpp>
#include <set>
#include <sstream>
#include <stack>

#include "asserts.hpp"
#include "filesystem.hpp"
#include "geometry.hpp"
#include "svg_paint.hpp"
#include "svg_parse.hpp"
#include "svg_path_parse.hpp"
#include "SvgTransform.hpp"

#ifndef M_PI
#	define M_PI		3.1415926535897932384626433832795
#endif

namespace svg
{
	using namespace boost::property_tree;

	namespace 
	{
		void display_ptree(ptree const& pt)
		{
			for(auto& v : pt) {
				std::cout << v.first << ": " << v.second.get_value<std::string>() << "\n";
				display_ptree( v.second );
			}
		}

		std::stack<paint>& get_fill_color_stack()
		{
			static std::stack<paint> res;
			return res;
		}
		std::stack<paint>& get_stroke_color_stack()
		{
			static std::stack<paint> res;
			return res;
		}
	}

	class svg_length
	{
	public:
		enum LengthUnit {
			SVG_LENGTHTYPE_UNKNOWN,
			SVG_LENGTHTYPE_NUMBER,
			SVG_LENGTHTYPE_PERCENTAGE,
			SVG_LENGTHTYPE_EMS,			// the 'font-size' of the relevant font 
			SVG_LENGTHTYPE_EXS,			// the 'x-height' of the relevant font 
			SVG_LENGTHTYPE_PX,			// pixels, relative to the viewing device
			SVG_LENGTHTYPE_CM,			// centimetres
			SVG_LENGTHTYPE_MM,			// millimetres
			SVG_LENGTHTYPE_IN,			// inches
			SVG_LENGTHTYPE_PT,			// points -- equal to 1/72th of an inch
			SVG_LENGTHTYPE_PC,			// picas -- 1 pica is equal to 12 points
		};
		svg_length() : value_(100.0f), units_(SVG_LENGTHTYPE_PERCENTAGE) {			
		}
		svg_length(float value, LengthUnit unit) : value_(value), units_(unit) {
		}
		svg_length(const std::string& length) {
			from_string(length);
		};
		void from_string(const std::string& length) {
			if(length.empty()) {
				units_ = SVG_LENGTHTYPE_UNKNOWN;
				value_ = 0.0f;
				return;
			}
			std::stringstream ss(length);
			std::string unit;
			ss >> value_;
			ss >> unit;
			if(unit.empty()) {
				units_ = SVG_LENGTHTYPE_NUMBER;
			} else if(unit == "em") {
				units_ = SVG_LENGTHTYPE_EMS;
			} else if(unit == "ex") {
				units_ = SVG_LENGTHTYPE_EXS;
			} else if(unit == "px") {
				units_ = SVG_LENGTHTYPE_PX;
			} else if(unit == "cm") {
				units_ = SVG_LENGTHTYPE_CM;
			} else if(unit == "mm") {
				units_ = SVG_LENGTHTYPE_MM;
			} else if(unit == "in") {
				units_ = SVG_LENGTHTYPE_IN;
			} else if(unit == "pt") {
				units_ = SVG_LENGTHTYPE_PT;
			} else if(unit == "pc") {
				units_ = SVG_LENGTHTYPE_PC;
			} else if(unit == "%") {
				units_ = SVG_LENGTHTYPE_PERCENTAGE;
			} else {
				ASSERT_LOG(false, "Unrecognised length unit: " << unit);
			}
		}
		float value_in_specified_units(LengthUnit units) const {
			switch(units_) {
				case SVG_LENGTHTYPE_UNKNOWN:
					ASSERT_LOG(false, "Unrecognished type SVG_LENGTHTYPE_UNKNOWN");
				case SVG_LENGTHTYPE_NUMBER:		return convert_number(units);
				case SVG_LENGTHTYPE_PERCENTAGE:	return convert_percentage(units);
				case SVG_LENGTHTYPE_EMS:		return convert_ems(units);
				case SVG_LENGTHTYPE_EXS:		return convert_exs(units);
				case SVG_LENGTHTYPE_PX:			return convert_px(units);
				case SVG_LENGTHTYPE_CM:			return convert_cm(units);
				case SVG_LENGTHTYPE_MM:			return convert_mm(units);
				case SVG_LENGTHTYPE_IN:			return convert_in(units);
				case SVG_LENGTHTYPE_PT:			return convert_pt(units);
				case SVG_LENGTHTYPE_PC:			return convert_pc(units);
			}
			return 0.0f;
		}
	private:
		float convert_number(LengthUnit units) const {
			switch(units) {
				case SVG_LENGTHTYPE_UNKNOWN:
					ASSERT_LOG(false, "Unhandled units value: SVG_LENGTHTYPE_UNKNOWN");
				case SVG_LENGTHTYPE_NUMBER:		return value_;
				case SVG_LENGTHTYPE_PERCENTAGE:	return 0;
				case SVG_LENGTHTYPE_EMS:		return 0;
				case SVG_LENGTHTYPE_EXS:		return 0;
				case SVG_LENGTHTYPE_PX:			return 0;
				case SVG_LENGTHTYPE_CM:			return 0;
				case SVG_LENGTHTYPE_MM:			return 0;
				case SVG_LENGTHTYPE_IN:			return 0;
				case SVG_LENGTHTYPE_PT:			return 0;
				case SVG_LENGTHTYPE_PC:			return 0;
				default:
					ASSERT_LOG(false, "Unrecognished units value: " << units);
			}
			return 0;
		}
		float convert_percentage(LengthUnit units) const {
			return 0;
		}
		float convert_ems(LengthUnit units) const {
			return 0;
		}
		float convert_exs(LengthUnit units) const {
			return 0;
		}
		float convert_px(LengthUnit units) const {
			return 0;
		}
		float convert_cm(LengthUnit units) const {
			return 0;
		}
		float convert_mm(LengthUnit units) const {
			return 0;
		}
		float convert_in(LengthUnit units) const {
			return 0;
		}
		float convert_pt(LengthUnit units) const {
			return 0;
		}
		float convert_pc(LengthUnit units) const {
			return 0;
		}

		float value_;
		LengthUnit units_;
	};

	class fill
	{
	public:
		enum FillRule {
			FILL_INHERIT,
			FILL_NONZERO,
			FILL_EVENODD,
		};
		enum LineJoin {
			LINEJOIN_INHERIT,
			LINEJOIN_MITER,
			LINEJOIN_ROUND,
			LINEJOIN_BEVEL,
		};
		enum LineCap {
			LINECAP_INHERIT,
			LINECAP_BUTT,
			LINECAP_ROUND,
			LINECAP_SQUARE,
		};
		fill() 
			: fill_rule_(FILL_INHERIT), 
			stroke_linejoin_(LINEJOIN_INHERIT),
			stroke_linecap_(LINECAP_INHERIT),
			stroke_miterlimit_(0.0),
			stroke_dashoffset_(0.0),
			alpha_(1.0)
		{
		}
		virtual ~fill() {
		}
		void set_fill_rule(FillRule rule) { fill_rule_ = rule; }
		void set_fill_rule(const std::string& rule) {
			if(rule == "nonzero") {
				fill_rule_ = FILL_NONZERO;
			} else if(rule == "evenodd") {
				fill_rule_ = FILL_NONZERO;
			} else {
				fill_rule_ = FILL_INHERIT;
				std::cerr << "WARNING: Unrecognised fill-rule: " << rule << std::endl;
			}
		}
		FillRule get_fill_rule(FillRule current_rule) const { fill_rule_ != FILL_INHERIT ? fill_rule_ : current_rule; }
		void apply_fill_rule(cairo_t* cairo) const {
			if(fill_rule_ != FILL_INHERIT) {
				switch(fill_rule_) {
					case FILL_EVENODD:	cairo_set_fill_rule(cairo, CAIRO_FILL_RULE_EVEN_ODD); break;
					case FILL_NONZERO:	cairo_set_fill_rule(cairo, CAIRO_FILL_RULE_WINDING); break;
				}
			}
		}

		void set_fill_color(const std::string& color_string) {
			fill_color_.reset(new paint(color_string));
		}
		void apply_colors() const {
			apply_fill_color();
			apply_stroke_color();
		}
		void apply_fill_color() const {
			if(fill_color_) {
				if(fill_color_->has_color()) {
					get_fill_color_stack().emplace(fill_color_->r(), fill_color_->g(), fill_color_->b(), alpha_);
					//cairo_set_source_rgba(cairo, fill_color_->r()/255.0, fill_color_->g()/255.0, fill_color_->b()/255.0, alpha_);

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
					get_fill_color_stack().emplace(0,0,0,0);
					//cairo_set_source_rgba(cairo, 0,0,0,0);
				}
			}
		}
		
		void set_stroke_color(const std::string& color_string) {
			stroke_color_.reset(new paint(color_string));
		}
		const paint_ptr& get_stroke_color(const paint_ptr& current_color) const { return stroke_color_ ? stroke_color_ : current_color; }
		void apply_stroke_color() const {
			if(stroke_color_) {
				if(stroke_color_->has_color()) {
					get_stroke_color_stack().emplace(stroke_color_->r(), stroke_color_->g(), stroke_color_->b(), alpha_);
					//cairo_set_source_rgba(cairo, stroke_color_->r()/255.0, stroke_color_->g()/255.0, stroke_color_->b()/255.0, alpha_);
				} else {
					get_stroke_color_stack().emplace(0,0,0,0);
					//cairo_set_source_rgba(cairo, 0,0,0,0);
				}
			}
		}
		void unapply_colors() const {
			if(fill_color_) {
				get_fill_color_stack().pop();
			}
			if(stroke_color_) {
				get_stroke_color_stack().pop();
			}
		}
		
		void set_stroke_width(const std::string& w) {
			stroke_width_.reset(new svg_length(w));
		}
		float get_stroke_width(float current_width) const { return stroke_width_ ? stroke_width_->value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER) : current_width; }
		void apply_stroke_width(cairo_t* cairo) const {
			if(stroke_width_) {
				cairo_set_line_width(cairo, stroke_width_->value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER));
			}
		}

		void set_stroke_linejoin(const std::string& lj) {
			if(lj == "miter") {
				stroke_linejoin_ = LINEJOIN_MITER;
			} else if(lj == "round") {
				stroke_linejoin_ = LINEJOIN_ROUND;
			} else if(lj == "bevel") {
				stroke_linejoin_ = LINEJOIN_BEVEL;
			} else {
				stroke_linejoin_ = LINEJOIN_INHERIT;
			}
		}
		LineJoin get_stroke_linejoin(LineJoin def) const { return stroke_linejoin_ != LINEJOIN_INHERIT ? stroke_linejoin_ : def; }
		void apply_stroke_linejoin(cairo_t* cairo) const {
			switch (stroke_linejoin_) {
				case LINEJOIN_MITER: cairo_set_line_join(cairo, CAIRO_LINE_JOIN_MITER); break;
				case LINEJOIN_ROUND: cairo_set_line_join(cairo, CAIRO_LINE_JOIN_ROUND); break;
				case LINEJOIN_BEVEL: cairo_set_line_join(cairo, CAIRO_LINE_JOIN_BEVEL); break;
				default: break;
			}
		}

		void set_stroke_linecap(const std::string& lc) {
			if(lc == "butt") {
				stroke_linecap_ = LINECAP_BUTT;
			} else if(lc == "round") {
				stroke_linecap_ = LINECAP_ROUND;
			} else if(lc == "square") {
				stroke_linecap_ = LINECAP_SQUARE;
			} else {
				stroke_linecap_ = LINECAP_INHERIT;
			}
		}
		LineCap get_stroke_linecap(LineCap def) const { return stroke_linecap_!= LINECAP_INHERIT ? stroke_linecap_ : def; }
		void apply_stroke_linecap(cairo_t* cairo) const {
			switch(stroke_linecap_) {
				case LINECAP_BUTT:		cairo_set_line_cap(cairo, CAIRO_LINE_CAP_BUTT); break;
				case LINECAP_ROUND:		cairo_set_line_cap(cairo, CAIRO_LINE_CAP_ROUND); break;
				case LINECAP_SQUARE:	cairo_set_line_cap(cairo, CAIRO_LINE_CAP_SQUARE); break;
				default: break;
			}
		}

		void set_stroke_miterlimit(const std::string& ml) {
			try {
				stroke_miterlimit_ = boost::lexical_cast<double>(ml);
			} catch(const boost::bad_lexical_cast&) {
				ASSERT_LOG(false, "Unable to convert value: '" << ml << "' to a number");
			}
		}
		double get_stroke_miterlimit(double def) const { return stroke_miterlimit_ ? stroke_miterlimit_ : def; }
		void apply_stroke_miterlimit(cairo_t* cairo) const {
			if(stroke_miterlimit_ != 0.0) {
				cairo_set_miter_limit(cairo, stroke_miterlimit_);
			}
		}

		void set_stroke_dashoffset(const std::string& offs) {
			try {
				stroke_dashoffset_ = boost::lexical_cast<double>(offs);
			} catch(const boost::bad_lexical_cast&) {
				ASSERT_LOG(false, "Unable to convert value: '" << offs << "' to a number");
			}
		}

		void set_stroke_dasharray(const std::string& da) {
			std::vector<svg_length> res;
			boost::tokenizer<boost::char_separator<char>> tok(da);
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
		const std::vector<svg_length>& get_stroke_dasharray(const std::vector<svg_length>& def) const {
			if(stroke_dasharray_.empty()) {
				return def;
			}
			return stroke_dasharray_;
		}
		void apply_stroke_dasharray(cairo_t* cairo) {
			if(!stroke_dasharray_.empty()) {
				std::vector<double> dashes;
				for(auto& l : stroke_dasharray_) {
					dashes.emplace_back(l.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER));
				}
				cairo_set_dash(cairo, &dashes[0], dashes.size(), stroke_dashoffset_);
			}
		}

		void set_opacity(const std::string& opacity) {
			try {
				alpha_ = boost::lexical_cast<double>(opacity);
			} catch(const boost::bad_lexical_cast&) {
				ASSERT_LOG(false, "Unable to convert value: '" << opacity << "' to a number");
			}
		}
		double get_opactity() const { return alpha_; }

		void set_cairo_values(cairo_t* cairo) const {
			apply_fill_rule(cairo);
			apply_stroke_width(cairo);
			apply_stroke_linejoin(cairo);
			apply_stroke_linecap(cairo);
			apply_stroke_miterlimit(cairo);		
		}
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
		double alpha_;
	};

	class font_properties
	{
	public:
		enum class FontStyle {
			INHERIT,
			NORMAL,
			ITALIC,
			OBLIQUE,
		};
		enum class FontVariant {
			INHERIT,
			NORMAL,
			SMALL_CAPS,
		};
		font_properties() 
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
		~font_properties() {}
		void set_font_family(const std::string& font_family) {
			font_family_ = font_family;
		}
		const std::string& get_font_family() const { return font_family_; }
		void set_font_style(const std::string& style) {
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
		FontStyle get_font_style() const { return font_style_; }
		void set_font_variant(const std::string& var) {
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
		FontVariant get_font_variant() const { return font_variant_; }
		void set_font_size(const std::string& size) {
			if(size == "inherit") {
				font_size_inherited_ = true;
			} else {
				font_size_.from_string(size);
			}
		}
		double get_font_size(double parent_value) const { 
			if(font_size_inherited_) {
				return parent_value;
			}
			return font_size_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
		}
		void set_letter_spacing(const std::string& spacing) {
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
		double get_letter_spacing(double parent_value) const {
			if(letter_spacing_inherited_) { 
				return parent_value;
			} 
			return letter_spacing_value_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
		}
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

	class shapes
	{
	public:
		shapes(const ptree& pt, const std::set<std::string>& exclusions) {
			auto attributes = pt.get_child_optional("<xmlattr>");
			if(attributes) {
				for(auto& attr : *attributes) {
					if(attr.first == "d") {
						auto& d = attr.second.data();
						if(!d.empty()) {
							path_ = parse_path(d);
						}
					} else if(attr.first == "transform") {
						transform_list_ = Transform::Factory(attr.second.data());
					} else if(attr.first == "fill") {
						fill_.set_fill_color(attr.second.data());
					} else if(attr.first == "stroke") {
						fill_.set_stroke_color(attr.second.data());
					} else if(attr.first == "stroke-width") {
						fill_.set_stroke_width(attr.second.data());
					} else if(attr.first == "fill-rule") {
						fill_.set_fill_rule(attr.second.data());
					} else if(attr.first == "stroke-linejoin") {
						fill_.set_stroke_linejoin(attr.second.data());
					} else if(attr.first == "stroke-linecap") {
						fill_.set_stroke_linecap(attr.second.data());
					} else if(attr.first == "stroke-miterlimit") {
						fill_.set_stroke_miterlimit(attr.second.data());
					} else if(attr.first == "stroke-dasharray") {
						fill_.set_stroke_dasharray(attr.second.data());
					} else if(attr.first == "color") {
						fill_.set_fill_color(attr.second.data());
					} else if(attr.first == "stroke-dashoffset") {
						fill_.set_stroke_dashoffset(attr.second.data());
					} else if(attr.first == "opacity") {
						fill_.set_opacity(attr.second.data());
					} else if(attr.first == "font-family") {
						font_.set_font_family(attr.second.data());
					} else if(attr.first == "font-size") {
						font_.set_font_size(attr.second.data());
					} else if(attr.first == "letter-spacing") {
						font_.set_letter_spacing(attr.second.data());
					} else {
						if(exclusions.find(attr.first) == exclusions.end()) {
							std::cerr << "SVG: path unhandled attribute: '" << attr.first << "' : '" << attr.second.data() << "'" << std::endl;
						}
					}
				}
			}
		}
		virtual ~shapes() {}
		void CairoRender(cairo_t* cairo) const {
			cairo_save(cairo);
			fill_.apply_colors();

			ApplyFill(cairo);
			ApplyFontProperties(cairo);
			ApplyTransforms(cairo);

			HandleCairoRender(cairo);

			RenderSubPaths(cairo);
			
			fill_.unapply_colors();
			cairo_restore(cairo);
		}
		void ApplyFill(cairo_t* cairo) const {
			fill_.set_cairo_values(cairo);
		}
		void ApplyTransforms(cairo_t* cairo) const {
			for(auto t : transform_list_) {
				t->Apply(cairo);
			}
		}
		void ApplyFillColor(cairo_t* cairo) const {
			//fill_.apply_fill_color(cairo);
			auto& fc = get_fill_color_stack().top();
			cairo_set_source_rgba(cairo, fc.r()/255.0, fc.g()/255.0, fc.b()/255.0, fc.a()/255.0);
		}
		void ApplyStrokeColor(cairo_t* cairo) const {
			//fill_.apply_stroke_color(cairo);
			auto& sc = get_stroke_color_stack().top();
			cairo_set_source_rgba(cairo, sc.r()/255.0, sc.g()/255.0, sc.b()/255.0, sc.a()/255.0);
		}
		void ApplyFontProperties(cairo_t* cairo) const {
			// font_.apply_font(cairo);
			auto style = font_.get_font_style();
			cairo_font_slant_t slant;
			switch(style) {
			case font_properties::FontStyle::NORMAL: slant = CAIRO_FONT_SLANT_NORMAL; break;
			case font_properties::FontStyle::ITALIC: slant = CAIRO_FONT_SLANT_ITALIC; break;
			case font_properties::FontStyle::OBLIQUE: slant = CAIRO_FONT_SLANT_OBLIQUE; break;
			default:
				ASSERT_LOG(false, "Bad font style given.");
			}
			cairo_select_font_face(cairo, font_.get_font_family().c_str(), slant, CAIRO_FONT_WEIGHT_NORMAL);
			/// XXX this is wrong because we need to inherit the parent values.
			// This applies to most stuff.
			cairo_set_font_size(cairo, font_.get_font_size(12));
		}
	protected:
		const font_properties& GetFontProperties() const { return font_; }
	private:
		virtual void HandleCairoRender(cairo_t* cairo) const = 0;

		void RenderSubPaths(cairo_t* cairo) const {
			if(!cairo_has_current_point(cairo)) {
				cairo_move_to(cairo, 0, 0);
			}
			
			if(!path_.empty()) {
				for(auto p : path_) {
					p->cairo_render(cairo);
				}

				ApplyFillColor(cairo);
				cairo_fill_preserve(cairo);
				ApplyStrokeColor(cairo);
				cairo_stroke(cairo);
			}
		}

		fill fill_;
		font_properties font_;
		std::vector<TransformPtr> transform_list_;
		std::vector<path_command_ptr> path_;
	};
	typedef std::shared_ptr<shapes> shapes_ptr;

	class path : public shapes
	{
	public:
		path(const ptree& pt) : shapes(pt, std::set<std::string>()) {
		}
		virtual ~path() {
		}
	private:
		virtual void HandleCairoRender(cairo_t* cairo) const override {
			// doesn't need to do anything -- sub-path rendering is handled in base class
		}
	};

	class circle : public shapes
	{
	public:
		// boost::assign::list_of here is a hack because MSVC doesn't support C++11 initialiser_lists
		circle(const ptree& pt) : shapes(pt, boost::assign::list_of("cx")("cy")("r")) {
			// XXX We should probably directly access the following attributes 
			// but meh.
			auto attributes = pt.get_child_optional("<xmlattr>");
			if(attributes) {
				for(auto& attr : *attributes) {
					if(attr.first == "cx") {
						cx_ = svg_length(attr.second.data());
					} else if(attr.first == "cy") {
						cy_ = svg_length(attr.second.data());
					} else if(attr.first == "r") {
						radius_ = svg_length(attr.second.data());
					}
				}
			}
			if(0) {
				std::cerr << "SVG: CIRCLE(" << cx_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER)
					<< "," << cy_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER)
					<< "," << radius_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER)
					<< ")" << std::endl;
			}
		}
		virtual ~circle() {}
	private:
		virtual void HandleCairoRender(cairo_t* cairo) const override {
			double cx = cx_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double cy = cy_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double r  = radius_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			cairo_arc(cairo, cx, cy, r, 0.0, 2 * M_PI);

			ApplyFillColor(cairo);
			cairo_fill_preserve(cairo);
			ApplyStrokeColor(cairo);
			cairo_stroke(cairo);
		}

		svg_length cx_;
		svg_length cy_;
		svg_length radius_;
	};

	class rectangle : public shapes
	{
	public:
		rectangle(const ptree& pt) 
			: shapes(pt, boost::assign::list_of("x")("y")("width")("height")("rx")("ry")), 
			is_rounded_(false) {
			// XXX We should probably directly access the following attributes 
			// but meh.
			auto attributes = pt.get_child_optional("<xmlattr>");
			if(attributes) {
				for(auto& attr : *attributes) {
					if(attr.first == "x") {
						x_ = svg_length(attr.second.data());
					} else if(attr.first == "y") {
						y_ = svg_length(attr.second.data());
					} else if(attr.first == "width") {
						width_ = svg_length(attr.second.data());
					} else if(attr.first == "height") {
						height_ = svg_length(attr.second.data());
					} else if(attr.first == "rx") {
						rx_ = svg_length(attr.second.data());
						is_rounded_ = true;
					} else if(attr.first == "ry") {
						ry_ = svg_length(attr.second.data());
						is_rounded_ = true;
					}
				}
			}
			if(0) {
				std::cerr << "SVG: RECTANGLE(" << x_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER)
					<< "," << y_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER)
					<< "," << width_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER)
					<< "," << height_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER)
					<< ")" << std::endl;
			}
		}
		virtual ~rectangle() {}
	private:
		virtual void HandleCairoRender(cairo_t* cairo) const override {
			ASSERT_LOG(is_rounded_ == false, "XXX we don't support rounded rectangles -- yet");
			double x = x_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double y = y_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double rx = rx_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double ry = ry_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double w  = width_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double h  = height_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);

			cairo_rectangle(cairo, x, y, w, h);

			ApplyFillColor(cairo);
			cairo_fill_preserve(cairo);
			ApplyStrokeColor(cairo);
			cairo_stroke(cairo);
		}
		svg_length x_;
		svg_length y_;
		svg_length rx_;
		svg_length ry_;
		svg_length width_;
		svg_length height_;
		bool is_rounded_;
		std::vector<TransformPtr> transform_list_;
		std::vector<path_command_ptr> path_;
	};

	class text : public shapes
	{
	public:
		text(const ptree& pt) 
			: shapes(pt, boost::assign::list_of("x")("y")("dx")("dy")("rotate")("textLength")("lengthAdjust")),
			adjust_(LengthAdjust::SPACING)
		{
			// XXX should we use provided <xmltext> instead?
			text_ = pt.get_value<std::string>();

			auto attributes = pt.get_child_optional("<xmlattr>");
			if(attributes) {
				for(auto& attr : *attributes) {
					if(attr.first == "x") {
						//x_ = svg_length(attr.second.data());
					} else if(attr.first == "y") {
					} else if(attr.first == "dx") {
					} else if(attr.first == "dy") {
					} else if(attr.first == "rotate") {
					} else if(attr.first == "textLength") {
					} else if(attr.first == "lengthAdjust") {
						if(attr.second.data() == "spacing") {
							adjust_ = LengthAdjust::SPACING;
						} else if(attr.second.data() == "spacingAndGlyphs") {
							adjust_ = LengthAdjust::SPACING_AND_GLYPHS;
						} else {
							ASSERT_LOG(false, "Unrecognised spacing value: " << attr.second.data());
						}
					}
				}
			}

		}
		virtual ~text() {}
	private:
		virtual void HandleCairoRender(cairo_t* cairo) const override {
			ApplyFillColor(cairo);
			/// need parent value.
			double letter_spacing = GetFontProperties().get_letter_spacing(0);
			if(letter_spacing > 0) {
				for(auto c : text_) {
					const char s[2] = {c,0};
					cairo_show_text(cairo, s);
					cairo_rel_move_to(cairo, letter_spacing, 0);
				}
			} else {
				cairo_show_text(cairo, text_.c_str());
			}
		}
		std::string text_;
		std::vector<svg_length> x_;
		std::vector<svg_length> y_;
		std::vector<svg_length> dx_;
		std::vector<svg_length> dy_;
		std::vector<double> rotate_;
		svg_length text_length_;
		enum class LengthAdjust {
			SPACING,
			SPACING_AND_GLYPHS,
		};
		LengthAdjust adjust_;
	};

	class group : public shapes
	{
	public:
		group(const ptree& pt) : shapes(pt,std::set<std::string>()) {
			for(auto& v : pt) {
				if(v.first == "path") {
					shapes_.emplace_back(new path(v.second));
				} else if(v.first == "circle") {
					shapes_.emplace_back(new circle(v.second));
				} else if(v.first == "rect") {
					shapes_.emplace_back(new rectangle(v.second));
				} else if(v.first == "text") {
					shapes_.emplace_back(new text(v.second));
				//} else if(v.first == "ellipse") {
				//	shapes_.emplace_back(new ellipse(v.second));
				//} else if(v.first == "polyline") {
				//	shapes_.emplace_back(new polyline(v.second));
				//} else if(v.first == "polygon") {
				//	shapes_.emplace_back(new polygon(v.second));
				} else if(v.first == "g") {
					shapes_.emplace_back(new group(v.second));
				} else if(v.first == "use") {
					// XXX
				} else if(v.first == "<xmlattr>") {
					// ignore
				} else if(v.first == "<xmlcomment>") {
					// ignore
				} else {
					std::cerr << "SVG: group unhandled child element: '" << v.first << "' : '" << v.second.data() << "'" << std::endl;
				}
			}
		}
		virtual ~group() {
		}
		virtual void HandleCairoRender(cairo_t* cairo) const override {
			for(auto s : shapes_) {
				s->CairoRender(cairo);
			}
		}
	private:
		std::vector<shapes_ptr> shapes_;
	};

	class svg
	{
	public:
		svg(const ptree& svg_data) 
			: x_(0, svg_length::SVG_LENGTHTYPE_PX), 
			y_(0, svg_length::SVG_LENGTHTYPE_PX)
		{
			const ptree & attributes = svg_data.get_child("<xmlattr>", ptree());
			for(auto& attr : attributes) {
				if(attr.first == "viewBox") {
					std::stringstream ss(attr.second.data());
					float x, y, w, h;
					ss >> x >> y >> w >> h;
					view_box_.from_area(x,y,w,h);
				} else if(attr.first == "x") {
					x_ = svg_length(attr.second.data());
				} else if(attr.first == "y") {
					y_ = svg_length(attr.second.data());
				} else if(attr.first == "width") {
					width_ = svg_length(attr.second.data());
				} else if(attr.first == "height") {
					height_ = svg_length(attr.second.data());
				} else if(attr.first == "xmlns") {
					// ignore
				} else {
					std::cerr << "SVG: svg unhandled attribute: " << attr.first << " : " << attr.second.data() << std::endl;
				}
			}

			for(auto& v : svg_data) {
				if(v.first == "path") {
					shapes_.emplace_back(new path(v.second));
				} else if(v.first == "g") {
					shapes_.emplace_back(new group(v.second));
				} else if(v.first == "use") {
					// XXX
				} else if(v.first == "<xmlattr>") {
					// ignore
				} else if(v.first == "<xmlcomment>") {
					// ignore
				} else {
					std::cerr << "SVG: svg unhandled child element: " << v.first << " : " << v.second.data() << std::endl;
				}
			}

		}
		void CairoRender(cairo_t* cairo) const {
			for(auto s : shapes_) {
				s->CairoRender(cairo);
			}
		}
	private:
		std::string xmlns_;
		rectf view_box_;
		svg_length x_;
		svg_length y_;
		svg_length width_;
		svg_length height_;
		std::vector<shapes_ptr> shapes_;
		//PrserveAspectRatio preserve_aspect_ratio_;
		enum ZoomAndPan {
			ZOOM_AND_PAN_DISABLE,
			ZOOM_AND_PAN_MAGNIFY,
		} zoom_and_pan_;
	};


	parse::parse(const std::string& filename)
	{
		ptree pt;
		read_xml(filename, pt);
		//display_ptree(pt);

		for(auto& node : pt) {
			if(node.first == "svg") {
				svg_data_.emplace_back(new svg(node.second));
			}
		}

		/*for(auto& node : pt.get_child("svg")) {
			if(node.first == "g") {
				const auto& g = node.second;
				if(g.count("path") != 0) {
					auto bounds = g.equal_range("path");
					for(auto it = bounds.first; it != bounds.second; ++it) {
						std::cerr  << it->first << " : ";
						const ptree& d = it->second.get_child("<xmlattr>.d");
						std::cerr << d.get_value<std::string>() << std::endl;
					}
				}
			}
		}*/
	}

	parse::~parse()
	{
	}

	void parse::CairoRender(cairo_t* cairo) const
	{
		cairo_set_source_rgb(cairo, 0.0, 0.0, 0.0);
		cairo_set_line_cap(cairo, CAIRO_LINE_CAP_BUTT);
		cairo_set_line_join(cairo, CAIRO_LINE_JOIN_MITER);
		cairo_set_miter_limit(cairo, 4.0);
		cairo_set_fill_rule(cairo, CAIRO_FILL_RULE_EVEN_ODD);
		cairo_set_line_width(cairo, 1.0);
		get_fill_color_stack().emplace(0,0,0,1);
		get_stroke_color_stack().emplace(0,0,0,0);

		for(auto p : svg_data_) {
			p->CairoRender(cairo);
		}
		ASSERT_LOG(get_fill_color_stack().size() == 1, "get_fill_color_stack() stack bug on exit");
		ASSERT_LOG(get_stroke_color_stack().size() == 1, "get_stroke_color_stack() stack bug on exit");
	}
}
