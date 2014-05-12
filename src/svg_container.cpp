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

#include "svg_container.hpp"
#include "svg_shapes.hpp"

namespace KRE
{
	namespace SVG
	{
		using namespace boost::property_tree;

        container::container(element* parent, const ptree& pt)
            : element(parent, pt)
		{
			// can contain graphics elements and other container elements.
			// 'a', 'defs', 'glyph', 'g', 'marker', 'mask', 'missing-glyph', 'pattern', 'svg', 'switch' and 'symbol'.
			// 'circle', 'ellipse', 'image', 'line', 'path', 'polygon', 'polyline', 'rect', 'text' and 'use'.

            //const ptree & attributes = pt.get_child("<xmlattr>", ptree());
			for(auto& v : pt) {
				if(v.first == "path") {
					elements_.emplace_back(new shape(this, v.second));
				} else if(v.first == "g") {
					elements_.emplace_back(new group(this, v.second));
				} else if(v.first == "rect") {
					elements_.emplace_back(new rectangle(this, v.second));
				} else if(v.first == "text") {
					elements_.emplace_back(new text(this, v.second));
				} else if(v.first == "line") {
					elements_.emplace_back(new line(this,v.second));
				} else if(v.first == "circle") {
					elements_.emplace_back(new circle(this,v.second));
				} else if(v.first == "polyline") {
					elements_.emplace_back(new polyline(this,v.second));
				} else if(v.first == "ellipse") {
					elements_.emplace_back(new ellipse(this,v.second));
				} else if(v.first == "desc") {
					// ignore
				} else if(v.first == "title") {
					// ignore
				} else if(v.first == "use") {
					elements_.emplace_back(new use_element(this,v.second));
				} else if(v.first == "defs") {
					elements_.emplace_back(new defs(this,v.second));
				} else if(v.first == "clipPath") {
					elements_.emplace_back(new clip_path(this,v.second));
				} else if(v.first == "<xmlattr>") {
					// ignore
				} else if(v.first == "<xmlcomment>") {
					// ignore
				} else {
					std::cerr << "SVG: svg unhandled child element: " << v.first << " : " << v.second.data() << std::endl;
				}
			}
		}

		container::~container()
		{
		}

		void container::handle_render(render_context& ctx) const
		{
			// XXXX
			/*
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
			*/
		}

		element_ptr container::handle_find_child(const std::string& id) const
		{
			for(auto e : elements_) {
				if(e->id() == id) {
					return e;
				}
				auto child = e->find_child(id);
				if(child) {
					return child;
				}
			}
			return element_ptr();
		}

		svg::svg(element* parent, const ptree& pt)
			: container(parent, pt)
		{
			auto attributes = pt.get_child_optional("<xmlattr>");

			if(attributes) {
				auto version = attributes->get_child_optional("version");
				if(version) {
					version_ = version->data();
				}

				auto base_profile = attributes->get_child_optional("baseProfile");
				if(base_profile) {
					base_profile_ = base_profile->data();
				}

				auto content_script_type = attributes->get_child_optional("contentScriptType");
				if(content_script_type) {
					content_script_type_ = content_script_type->data();
				}

				auto content_style_type = attributes->get_child_optional("contentStyleType");
				if(content_style_type) {
					content_style_type_ = content_style_type->data();
				}

				auto xml_ns = attributes->get_child_optional("xml:ns");
				if(xml_ns) {
					xmlns_ = xml_ns->data();
				}

				// todo: zoom_and_pan_
				// todo: preserve_aspect_ratio_

				/*
				auto version = attributes->get_child_optional("version");
				if(version) {
					version_ = version->data();
				}
				*/
			}
		}

		svg::~svg()
		{
		}

		void svg::handle_render(render_context& ctx) const
		{
			/// XXX
			container::render(ctx);
		}

		group::group(element* parent, const ptree& pt)
			: container(parent, pt)
		{
		}

		group::~group()
		{
		}

		void group::handle_render(render_context& ctx) const
		{
			/// XXX
			// call base-class
			//container::render(ctx);
		}

		defs::defs(element* parent, const ptree& pt)
			: container(parent, pt)
		{
		}

		defs::~defs()
		{
		}

		void defs::handle_render(render_context& ctx) const
		{
			/// XXX
			// call base-class
			//container::render(ctx);
		}

		clip_path::clip_path(element* parent, const ptree& pt)
			: container(parent, pt)
		{
		}

		clip_path::~clip_path()
		{
		}

		void clip_path::handle_render(render_context& ctx) const
		{
			/// XXX
			// call base-class
			//container::render(ctx);
		}
	}
}
