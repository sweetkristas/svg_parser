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

#include <boost/tokenizer.hpp>
#include <set>

#include "svg_shapes.hpp"
#include "svg_element.hpp"

namespace KRE
{
	namespace SVG
	{
		using namespace boost::property_tree;

		namespace
		{
			point_list create_point_list(const std::string& s)
			{
				std::vector<svg_length> res;
				boost::char_separator<char> seperators(" \n\t\r,");
				boost::tokenizer<boost::char_separator<char>> tok(s, seperators);
				for(auto it = tok.begin(); it != tok.end(); ++it) {
					res.emplace_back(*it);
				}
				ASSERT_LOG(res.size() % 2 == 0, "point list has an odd number of points.");
				auto it = res.begin();
				point_list points;
				while(it != res.end()) {
					svg_length p1 = *it;
					++it;
					svg_length p2 = *it;
					++it;
					points.emplace_back(p1, p2);
				}
				return points;
			}
		}

		shape::shape(element* doc, const ptree& pt)
				: element(doc, pt)
		{
			auto attributes = pt.get_child_optional("<xmlattr>");
			if(attributes) {
				for(auto& attr : *attributes) {
					if(attr.first == "d") {
						auto& d = attr.second.data();
						if(!d.empty()) {
							path_ = parse_path(d);
						}
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
						// XXX There is some question about this attribute effect things.
						fill_.set_fill_color(attr.second.data());
						fill_.set_stroke_color(attr.second.data());
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
					} else if(attr.first == "overflow") {
						// ignore overflow element. it should only apply to 'svg','symbol','image','pattern','marker'
					} else if(attr.first == "clip-path") {
						// only internal references of the form "url(#some_id)" are supported
						// invalid url's are treated as not existing.
						std::string funciri = attr.second.data();
						if(funciri.substr(0,5) == "url(#" && funciri.back() == ')') {
							clip_path_ref_ = funciri.substr(5, funciri.size()-6);
						} else {
							std::cerr << "clip-path iri not in expected format: " << funciri << std::endl;
						}
					} else {
						if(exclusions.find(attr.first) == exclusions.end()) {
							std::cerr << "SVG: path unhandled attribute: '" << attr.first << "' : '" << attr.second.data() << "'" << std::endl;
						}
					}
				}
			}
		}

		shape::~shape() 
		{
		}

		void shape::handle_render(render_context& ctx) const 
		{
			// if(!cairo_has_current_point(ctx.cairo())) {
			//		cairo_move_to(ctx.cairo(), 0, 0);
			// }
			
			if(!path_.empty()) {
				path_cmd_context path_ctx(ctx.cairo());
				for(auto p : path_) {
					p->cairo_render(path_ctx);
				}
			}
		}

		// list_of here is a hack because MSVC doesn't support C++11 initialiser_lists
		circle::circle(element* doc, const ptree& pt) 
			: shape(doc, pt) 
		{
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

		circle::~circle() 
		{
		}
		
		void circle::handle_render(render_context& ctx) const 
		{
			double cx = cx_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double cy = cy_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double r  = radius_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			cairo_arc(ctx.cairo(), cx, cy, r, 0.0, 2 * M_PI);

			shape::render(ctx);
		}

		ellipse::ellipse(element* doc, const ptree& pt)
			: shape(doc, pt),
			cx_(0, svg_length::SVG_LENGTHTYPE_NUMBER),
			cy_(0, svg_length::SVG_LENGTHTYPE_NUMBER),
			rx_(0, svg_length::SVG_LENGTHTYPE_NUMBER),
			ry_(0, svg_length::SVG_LENGTHTYPE_NUMBER)
		{
			const ptree & attributes = pt.get_child("<xmlattr>", ptree());
            auto cx = attributes.get_child_optional("cx");
            if(cx) {
				cx_ = svg_length(cx->data());
			}
            auto cy = attributes.get_child_optional("cy");
            if(cy) {
				cy_ = svg_length(cy->data());
			}
            auto rx = attributes.get_child_optional("rx");
            if(rx) {
				rx_ = svg_length(rx->data());
			}
            auto ry = attributes.get_child_optional("ry");
            if(ry) {
				ry_ = svg_length(ry->data());
			}
		}

		ellipse::~ellipse()
		{
		}

		void ellipse::handle_render(render_context& ctx) const 
		{
			double cx = cx_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double cy = cy_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double rx = rx_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double ry = ry_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);

			cairo_save(ctx.cairo());
			cairo_translate(ctx.cairo(), cx+rx, cy+ry);
			cairo_scale(ctx.cairo(), rx, ry);
			cairo_arc_negative(ctx.cairo(), 0.0, 0.0, 1.0, 0.0, 2*M_PI);
			cairo_restore(ctx.cairo());

			shape::render(ctx);
		}

		rectangle::rectangle(element* doc, const ptree& pt) 
			: shape(doc, pt), 
			is_rounded_(false) 
		{
			const ptree & attributes = pt.get_child("<xmlattr>", ptree());
            auto x = attributes.get_child_optional("x");
			if(x) {
				x_ = svg_length(x->data());
			}
            auto y = attributes.get_child_optional("y");
			if(y) {
				y_ = svg_length(y->data());
			}
            auto w = attributes.get_child_optional("width");
			if(w) {
				width_ = svg_length(w->data());
			}
            auto h = attributes.get_child_optional("height");
			if(h) {
				height_ = svg_length(h->data());
			}
            auto rx = attributes.get_child_optional("rx");
			if(rx) {
				rx_ = svg_length(rx->data());
			}
            auto ry = attributes.get_child_optional("ry");
			if(ry) {
				ry_ = svg_length(ry->data());
			}
			if(rx || ry) {
				is_rounded_ = true;
			}
		}

		rectangle::~rectangle() 
		{
		}

		void rectangle::handle_render(render_context& ctx) const 
		{
			ASSERT_LOG(is_rounded_ == false, "XXX we don't support rounded rectangles -- yet");
			double x = x_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double y = y_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double rx = rx_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double ry = ry_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double w  = width_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double h  = height_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);

			cairo_rectangle(ctx.cairo(), x, y, w, h);

			shape::render(ctx);
		}

		polygon::polygon(element* doc, const ptree& pt) 
			: shape(doc, pt)
		{
			const ptree & attributes = pt.get_child("<xmlattr>", ptree());
            auto points = attributes.get_child_optional("points");
			if(points) {
				points_ = create_point_list(points->data());
			}
		}

		polygon::~polygon() 
		{
		}

		void polygon::handle_render(render_context& ctx) const 
		{
			auto it = points_.begin();
			cairo_move_to(ctx.cairo(), 
				it->first.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER), 
				it->second.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER));
			while(it != points_.end()) {
				cairo_line_to(ctx.cairo(), 
					it->first.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER), 
					it->second.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER));
				++it;
			}
			cairo_close_path(ctx.cairo());

			shape::render(ctx);
		}

		text::text(element* doc, const ptree& pt) 
			: shape(doc, pt),
			adjust_(LengthAdjust::SPACING)
		{
			// XXX should we use provided <xmltext> instead?
			text_ = pt.get_value<std::string>();

			const ptree & attributes = pt.get_child("<xmlattr>", ptree());
            auto x = attributes.get_child_optional("x");
			if(x) {
				//x_ = svg_length(x->data());
			}
            auto y = attributes.get_child_optional("y");
			if(y) {
				//y_ = svg_length(y->data());
			}
            auto dx = attributes.get_child_optional("dx");
			if(dx) {
				//dx_ = svg_length(dx->data());
			}
            auto dy = attributes.get_child_optional("dy");
			if(dy) {
				//dy_ = svg_length(dy->data());
			}
            auto rotate = attributes.get_child_optional("rotate");
			if(rotate) {
				//rotate_ = svg_length(rotate->data());
			}
            auto text_length = attributes.get_child_optional("textLength");
			if(text_length) {
				//text_length_ = svg_length(text_length->data());
			}
            auto length_adjust = attributes.get_child_optional("lengthAdjust");
			if(length_adjust) {
				if(length_adjust->data() == "spacing") {
					adjust_ = LengthAdjust::SPACING;
				} else if(length_adjust->data() == "spacingAndGlyphs") {
					adjust_ = LengthAdjust::SPACING_AND_GLYPHS;
				} else {
					ASSERT_LOG(false, "Unrecognised spacing value: " << length_adjust->data());
				}
			}
		}

		text::~text() 
		{
		}

		void text::handle_render(render_context& ctx) const 
		{
			/// need parent value.
			double letter_spacing = GetFontProperties().get_letter_spacing(0);
			if(letter_spacing > 0) {
				for(auto c : text_) {
					const char s[2] = {c,0};
					cairo_show_text(ctx.cairo(), s);
					cairo_rel_move_to(ctx.cairo(), letter_spacing, 0);
				}
			} else {
				cairo_show_text(ctx.cairo(), text_.c_str());
			}

			shape::render(ctx);
		}

		group::group(element* doc, const ptree& pt) 
			: shape(doc,pt,std::set<std::string>())
		{
			for(auto& v : pt) {
				if(v.first == "path") {
					shape_.emplace_back(new path(doc,v.second));
				} else if(v.first == "circle") {
					shape_.emplace_back(new circle(doc,v.second));
				} else if(v.first == "rect") {
					shape_.emplace_back(new rectangle(doc,v.second));
				} else if(v.first == "text") {
					shape_.emplace_back(new text(doc,v.second));
				//} else if(v.first == "ellipse") {
				//	shape_.emplace_back(new ellipse(doc,v.second));
				} else if(v.first == "line") {
					shape_.emplace_back(new line(doc,v.second));
				} else if(v.first == "polyline") {
					shape_.emplace_back(new polyline(doc,v.second));
				} else if(v.first == "polygon") {
					shape_.emplace_back(new polygon(doc,v.second));
				} else if(v.first == "g") {
					shape_.emplace_back(new group(doc,v.second));
				} else if(v.first == "clipPath") {
					clip_path_.emplace_back(new group(doc,v.second));
				} else if(v.first == "defs") {
					defs_.emplace_back(new group(doc,v.second));
				} else if(v.first == "use") {
					shape_.emplace_back(new use_stmt(doc,v.second));
				} else if(v.first == "linearGradient") {
					gradient_list_.emplace_back(new linear_gradient(doc, v.second));
				} else if(v.first == "desc") {
					// ignore
				} else if(v.first == "title") {
					// ignore
				} else if(v.first == "<xmlattr>") {
					// ignore
				} else if(v.first == "<xmlcomment>") {
					// ignore
				} else {
					std::cerr << "SVG: group unhandled child element: '" << v.first << "' : '" << v.second.data() << "'" << std::endl;
				}
			}
		}

		group::~group() 
		{
		}

		void group::handle_clip_render(render_context& ctx) const
		{
			for(auto s : shape_) {
				s->clip_render(ctx);
			}
		}

		void group::handle_cairo_render(render_context& ctx) const 
		{
			cairo_push_group(ctx.cairo());
			if(!clip_path_id().empty()) {
				auto cp = find_child(clip_path_id());
				if(cp) {
					cp->clip_render(ctx);
					cairo_clip(ctx.cairo());
				} else {
					std::cerr << "WARNING: Couldn't find specified 'clip-path' element '" << clip_path_id() << "'" << std::endl;
				}
			}

			for(auto s : shape_) {
				s->cairo_render(ctx);
			}
			cairo_pop_group_to_source(ctx.cairo());
			cairo_paint_with_alpha(ctx.cairo(), ctx.opacity_top());
		}

		const_shape_ptr group::handle_find_child_id(const std::string& id) const
		{
			for(auto& d : defs_) {
				if(d->id() == id) {
					return d;
				}
				auto sp = d->find_child_id(id);
				if(sp) {
					return sp;
				}
			}
			for(auto& s : shape_) {
				if(s->id() == id) {
					return s;
				}
				auto sp = s->find_child_id(id);
				if(sp) {
					return sp;
				}
			}
			for(auto& cp : clip_path_) {
				if(cp->id() == id) {
					return cp;
				}
				auto sp = cp->find_child_id(id);
				if(sp) {
					return sp;
				}
			}
			for(auto& grad : gradient_list_) {
				if(grad->id() == id) {
					return grad;
				}
				auto gr = grad->find_child_id(id);
				if(gr) {
					return gr;
				}
			}
			return shape_ptr();
		}


		line::line(element* doc, const ptree& pt)
			: shape(doc, pt),
			x1_(0, svg_length::SVG_LENGTHTYPE_NUMBER),
			y1_(0, svg_length::SVG_LENGTHTYPE_NUMBER),
			x2_(0, svg_length::SVG_LENGTHTYPE_NUMBER),
			y2_(0, svg_length::SVG_LENGTHTYPE_NUMBER)
		{
			const ptree & attributes = pt.get_child("<xmlattr>", ptree());
            auto x1 = attributes.get_child_optional("x1");
			if(x1) {
				x1_ = svg_length(x1->data());
			}
            auto y1 = attributes.get_child_optional("y1");
			if(y1) {
				y1_ = svg_length(y1->data());
			}
            auto x2 = attributes.get_child_optional("x2");
			if(x2) {
				x2_ = svg_length(x2->data());
			}
            auto y2 = attributes.get_child_optional("y2");
			if(y2) {
				y2_ = svg_length(y2->data());
			}
		}

		line::~line()
		{
		}

		void line::handle_render(render_context& ctx) const
		{
			double x1 = x1_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double y1 = y1_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double x2 = x2_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double y2 = y2_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			
			cairo_move_to(ctx.cairo(), x1, y1);
			cairo_line_to(ctx.cairo(), x2, y2);
		}

		polyline::polyline(element* doc, const ptree& pt)
			: shape(doc,pt)
		{
            auto points = attributes.get_child_optional("points");
			if(points) {
				points_ = create_point_list(points->data());
			}
		}

		polyline::~polyline()
		{
		}

		void polyline::handle_render(render_context& ctx) const
		{
			bool is_first = true;
			for(auto& p : points_) {
				double x = p.first.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
				double y = p.second.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
				if(is_first) {
					is_first = false;
					cairo_move_to(ctx.cairo(), x, y);
				} else {
					cairo_line_to(ctx.cairo(), x, y);
				}
			}
			shape::render(ctx);
		}
	}
}
