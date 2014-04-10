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
#include <boost/tokenizer.hpp>
#include <set>

#include "svg_shapes.hpp"
#include "svg_element.hpp"

namespace KRE
{
	namespace SVG
	{
		using namespace boost::property_tree;
		using namespace boost::assign;

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
				ASSERT_LOG(res.size() % 2 == 0, "'polygon' element has an odd number of points.");
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

		shapes::shapes(element* doc, const ptree& pt, const std::set<std::string>& exclusions)
				: doc_(doc)
		{
			auto attributes = pt.get_child_optional("<xmlattr>");
			if(attributes) {
				for(auto& attr : *attributes) {
					if(attr.first == "d") {
						auto& d = attr.second.data();
						if(!d.empty()) {
							path_ = parse_path(d);
						}
					} else if(attr.first == "id") {
						id_ = attr.second.data();
					} else if(attr.first == "transform") {
						transform_list_ = transform::factory(attr.second.data());
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

		shapes::~shapes() 
		{
		}

		void shapes::cairo_render(render_context& ctx) const 
		{
			cairo_save(ctx.cairo());
			cairo_new_path(ctx.cairo());

			fill_.apply_colors(ctx);

			apply_fill(ctx);
			apply_font_properties(ctx);
			apply_transforms(ctx);

			handle_cairo_render(ctx);

			render_sub_paths(ctx);
			
			fill_.unapply_colors(ctx);
			cairo_restore(ctx.cairo());
		}

		void shapes::clip_render(render_context& ctx) const 
		{
			// XXX do we need to apply transforms and fill rules here?
			handle_clip_render(ctx);

			if(!path_.empty()) {
				path_cmd_context ctx(ctx.cairo());
				for(auto p : path_) {
					p->cairo_render(ctx);
				}
			}
		}

		void shapes::apply_fill(render_context& ctx) const 
		{
			fill_.set_cairo_values(ctx);
		}

		void shapes::apply_transforms(render_context& ctx) const 
		{
			for(auto t : transform_list_) {
				t->apply(ctx);
			}
		}

		void shapes::apply_fill_color(render_context& ctx) const 
		{
			//fill_.apply_fill_color(cairo);
			auto& fc = ctx.fill_color_top();
			cairo_set_source_rgba(ctx.cairo(), fc.r()/255.0, fc.g()/255.0, fc.b()/255.0, fc.a()/255.0);
		}

		void shapes::apply_stroke_color(render_context& ctx) const 
		{
			//fill_.apply_stroke_color(cairo);
			auto& sc = ctx.stroke_color_top();
			cairo_set_source_rgba(ctx.cairo(), sc.r()/255.0, sc.g()/255.0, sc.b()/255.0, sc.a()/255.0);
		}

		void shapes::apply_font_properties(render_context& ctx) const 
		{
			font_.apply(ctx);
		}

		const_shapes_ptr shapes::find_child(const std::string& id) const
		{
			if(doc_ != NULL) {
				return doc_->find_child_id(id);
			}
			return shapes_ptr();
		}

		const_shapes_ptr shapes::find_child_id(const std::string& id) const
		{
			return handle_find_child_id(id);
		}

		void shapes::render_sub_paths(render_context& ctx) const
		{
			if(!cairo_has_current_point(ctx.cairo())) {
				cairo_move_to(ctx.cairo(), 0, 0);
			}
			
			if(!path_.empty()) {
				path_cmd_context path_ctx(ctx.cairo());
				for(auto p : path_) {
					p->cairo_render(path_ctx);
				}

				apply_fill_color(ctx);
				cairo_fill_preserve(ctx.cairo());
				apply_stroke_color(ctx);
				cairo_stroke(ctx.cairo());
			}
		}

		class circle : public shapes
		{
		public:
			// list_of here is a hack because MSVC doesn't support C++11 initialiser_lists
			circle(element* doc, const ptree& pt) 
				: shapes(doc, pt, list_of("cx")("cy")("r")) 
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
			virtual ~circle() {}
		private:
			void render_shape_internal(render_context& ctx) const {
				double cx = cx_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
				double cy = cy_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
				double r  = radius_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
				cairo_arc(ctx.cairo(), cx, cy, r, 0.0, 2 * M_PI);
			}
			virtual void handle_cairo_render(render_context& ctx) const override {
				render_shape_internal(ctx);

				apply_fill_color(ctx);
				cairo_fill(ctx.cairo());
				cairo_fill_preserve(ctx.cairo());
				apply_stroke_color(ctx);
				cairo_stroke(ctx.cairo());
			}
			virtual void handle_clip_render(render_context& ctx) const override {
				render_shape_internal(ctx);
			}
			const_shapes_ptr handle_find_child_id(const std::string& id) const override {
				return shapes_ptr();
			}

			svg_length cx_;
			svg_length cy_;
			svg_length radius_;
		};

		rectangle::rectangle(element* doc, const ptree& pt) 
			: shapes(doc, pt, list_of("x")("y")("width")("height")("rx")("ry")), 
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

		rectangle::~rectangle() 
		{
		}

		void rectangle::render_shape_internal(render_context& ctx) const 
		{
			ASSERT_LOG(is_rounded_ == false, "XXX we don't support rounded rectangles -- yet");
			double x = x_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double y = y_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double rx = rx_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double ry = ry_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double w  = width_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double h  = height_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);

			cairo_rectangle(ctx.cairo(), x, y, w, h);
		}

		void rectangle::handle_cairo_render(render_context& ctx) const 
		{
			render_shape_internal(ctx);

			apply_fill_color(ctx);
			cairo_fill_preserve(ctx.cairo());
			apply_stroke_color(ctx);
			cairo_stroke(ctx.cairo());
		}

		void rectangle::handle_clip_render(render_context& ctx) const 
		{
			render_shape_internal(ctx);
		}

		const_shapes_ptr rectangle::handle_find_child_id(const std::string& id) const 
		{
			return shapes_ptr();
		}

		class polygon : public shapes
		{
		public:
			polygon(element* doc, const ptree& pt) : shapes(doc, pt, list_of("points"))
			{
				auto attributes = pt.get_child_optional("<xmlattr>");
				if(attributes) {
					for(auto& attr : *attributes) {
						if(attr.first == "points") {
							points_ = create_point_list(attr.second.data());
						}
					}
				}
			}
			virtual ~polygon() {}
		private:
			void render_shape_internal(render_context& ctx) const {
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
			}
			virtual void handle_cairo_render(render_context& ctx) const override {
				render_shape_internal(ctx);

				apply_fill_color(ctx);
				cairo_fill_preserve(ctx.cairo());
				apply_stroke_color(ctx);
				cairo_stroke(ctx.cairo());
			}
			virtual void handle_clip_render(render_context& ctx) const override {
				render_shape_internal(ctx);
			}
			const_shapes_ptr handle_find_child_id(const std::string& id) const override {
				return shapes_ptr();
			}
			point_list points_;
		};

		text::text(element* doc, const ptree& pt) 
			: shapes(doc, pt, list_of("x")("y")("dx")("dy")("rotate")("textLength")("lengthAdjust")),
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

		text::~text() 
		{
		}

		void text::render_shape_internal(render_context& ctx) const 
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
		}

		void text::handle_cairo_render(render_context& ctx) const 
		{
			apply_fill_color(ctx);
			render_shape_internal(ctx);
		}
		
		void text::handle_clip_render(render_context& ctx) const 
		{
			render_shape_internal(ctx);
		}

		const_shapes_ptr text::handle_find_child_id(const std::string& id) const 
		{
			return shapes_ptr();
		}

		class use : public shapes
		{
		public:
			use(element* doc, const ptree& pt) 
				: shapes(doc,pt,list_of("xlink:href")("x")("y")("width")("height")) 
			{
				auto attributes = pt.get_child_optional("<xmlattr>");
				if(attributes) {
					for(auto& attr : *attributes) {
						if(attr.first == "xlink:href") {
							// Basically only supporting inter-document references
							// so "#some_id" for example.
							xref_id_ = attr.second.data();
							if(!xref_id_.empty()) {
								if(xref_id_[0] != '#') {
									std::cerr << "Only supporting inter-document cross-references: " << xref_id_ << std::endl;
								} else {
									xref_id_ = xref_id_.substr(1);
								}
							}
						} else if(attr.first == "x") {
							x_ = svg_length(attr.second.data());
						} else if(attr.first == "y") {
							y_ = svg_length(attr.second.data());
						} else if(attr.first == "width") {
							width_ = svg_length(attr.second.data());
						} else if(attr.first == "height") {
							height_ = svg_length(attr.second.data());
						}
					}
				}
			}
			virtual ~use() {}
		private:
			void render_shape_internal(render_context& ctx) const {
				if(xref_id_.empty()) {
					return;
				}

				// Acts as a <g ...> attribute when rendered.
				double x = x_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
				double y = y_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
				double w = width_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
				double h = height_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
				if(x != 0 || y != 0) {
					// The whole list_of could be more eloquently replaced by an
					// initialiser list. If certain compilers would actually bother supporting
					// C++11 features.
					auto tfr = transform::factory(TransformType::TRANSLATE,list_of(x)(y));
					tfr->apply(ctx);
				}
				/// XXX search for xref_id_ in current svg document, then render it.
				auto s = find_child(xref_id_);
				if(s) {
					s->clip_render(ctx);
				} else {
					std::cerr << "WARNING: Couldn't find element '" << xref_id_ << "' in document." << std::endl;
				}
			}
			virtual void handle_clip_render(render_context& ctx) const override {
				render_shape_internal(ctx);
			}
			virtual void handle_cairo_render(render_context& ctx) const override {
				render_shape_internal(ctx);
			}
			const_shapes_ptr handle_find_child_id(const std::string& id) const override {
				return shapes_ptr();
			}
			svg_length x_;
			svg_length y_;
			svg_length width_;
			svg_length height_;
			std::string xref_id_;
		};

		group::group(element* doc, const ptree& pt) 
			: shapes(doc,pt,std::set<std::string>())
		{
			for(auto& v : pt) {
				if(v.first == "path") {
					shapes_.emplace_back(new path(doc,v.second));
				} else if(v.first == "circle") {
					shapes_.emplace_back(new circle(doc,v.second));
				} else if(v.first == "rect") {
					shapes_.emplace_back(new rectangle(doc,v.second));
				} else if(v.first == "text") {
					shapes_.emplace_back(new text(doc,v.second));
				//} else if(v.first == "ellipse") {
				//	shapes_.emplace_back(new ellipse(doc,v.second));
				} else if(v.first == "line") {
					shapes_.emplace_back(new line(doc,v.second));
				} else if(v.first == "polyline") {
					shapes_.emplace_back(new polyline(doc,v.second));
				} else if(v.first == "polygon") {
					shapes_.emplace_back(new polygon(doc,v.second));
				} else if(v.first == "g") {
					shapes_.emplace_back(new group(doc,v.second));
				} else if(v.first == "clipPath") {
					clip_path_.emplace_back(new group(doc,v.second));
				} else if(v.first == "defs") {
					defs_.emplace_back(new group(doc,v.second));
				} else if(v.first == "use") {
					shapes_.emplace_back(new use(doc,v.second));
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
			for(auto s : shapes_) {
				s->clip_render(ctx);
			}
		}

		void group::handle_cairo_render(render_context& ctx) const 
		{
			if(!clip_path_id().empty()) {
				auto cp = find_child(clip_path_id());
				if(cp) {
					cp->clip_render(ctx);
					cairo_clip(ctx.cairo());
				} else {
					std::cerr << "WARNING: Couldn't find specified 'clip-path' element '" << clip_path_id() << "'" << std::endl;
				}
			}

			for(auto s : shapes_) {
				s->cairo_render(ctx);
			}
		}

		const_shapes_ptr group::handle_find_child_id(const std::string& id) const
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
			for(auto& s : shapes_) {
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
			return shapes_ptr();
		}


		line::line(element* doc, const ptree& pt)
			: shapes(doc,pt,list_of("x1")("y1")("x2")("y2")),
			x1_(0, svg_length::SVG_LENGTHTYPE_NUMBER),
			y1_(0, svg_length::SVG_LENGTHTYPE_NUMBER),
			x2_(0, svg_length::SVG_LENGTHTYPE_NUMBER),
			y2_(0, svg_length::SVG_LENGTHTYPE_NUMBER)
		{
			// XXX We should probably directly access the following attributes 
			// but meh.
			auto attributes = pt.get_child_optional("<xmlattr>");
			if(attributes) {
				for(auto& attr : *attributes) {
					if(attr.first == "x1") {
						x1_ = svg_length(attr.second.data());
					} else if(attr.first == "y1") {
						y1_ = svg_length(attr.second.data());
					} else if(attr.first == "x2") {
						x2_ = svg_length(attr.second.data());
					} else if(attr.first == "y2") {
						y2_ = svg_length(attr.second.data());
					}
				}
			}
		}

		line::~line()
		{
		}

		void line::render_shape_internal(render_context& ctx) const
		{
			double x1 = x1_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double y1 = y1_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double x2 = x2_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			double y2 = y2_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER);
			
			cairo_move_to(ctx.cairo(), x1, y1);
			cairo_line_to(ctx.cairo(), x2, y2);
		}

		void line::handle_cairo_render(render_context& ctx) const
		{
			render_shape_internal(ctx);

			apply_stroke_color(ctx);
			cairo_stroke(ctx.cairo());
		}

		void line::handle_clip_render(render_context& ctx) const
		{
			render_shape_internal(ctx);
		}

		const_shapes_ptr line::handle_find_child_id(const std::string& id) const
		{
			return shapes_ptr();
		}


		polyline::polyline(element* doc, const ptree& pt)
			: shapes(doc,pt,list_of("points"))
		{
			// XXX We should probably directly access the following attributes 
			// but meh.
			auto attributes = pt.get_child_optional("<xmlattr>");
			if(attributes) {
				for(auto& attr : *attributes) {
					if(attr.first == "points") {
						points_ = create_point_list(attr.second.data());
					}
				}
			}
		}

		polyline::~polyline()
		{
		}

		void polyline::render_shape_internal(render_context& ctx) const
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
		}

		void polyline::handle_cairo_render(render_context& ctx) const
		{
			render_shape_internal(ctx);
			apply_stroke_color(ctx);
			cairo_stroke_preserve(ctx.cairo());
			apply_fill_color(ctx);
			cairo_fill(ctx.cairo());
		}

		void polyline::handle_clip_render(render_context& ctx) const
		{
			render_shape_internal(ctx);
		}

		const_shapes_ptr polyline::handle_find_child_id(const std::string& id) const
		{
			return shapes_ptr();
		}


	}
}
