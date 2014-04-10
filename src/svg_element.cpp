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

#include "svg_element.hpp"
#include "svg_shapes.hpp"

namespace KRE
{
	namespace SVG
	{
		using namespace boost::property_tree;

		element::element(const ptree& svg_data) 
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
					shapes_.emplace_back(new path(this, v.second));
				} else if(v.first == "g") {
					shapes_.emplace_back(new group(this, v.second));
				} else if(v.first == "rect") {
					shapes_.emplace_back(new rectangle(this, v.second));
				} else if(v.first == "text") {
					shapes_.emplace_back(new text(this, v.second));
				} else if(v.first == "line") {
					shapes_.emplace_back(new line(this,v.second));
				} else if(v.first == "polyline") {
					shapes_.emplace_back(new polyline(this,v.second));
				} else if(v.first == "desc") {
					// ignore
				} else if(v.first == "title") {
					// ignore
				/*} else if(v.first == "use") {
					// XXX
				*/
				} else if(v.first == "<xmlattr>") {
					// ignore
				} else if(v.first == "<xmlcomment>") {
					// ignore
				} else {
					std::cerr << "SVG: svg unhandled child element: " << v.first << " : " << v.second.data() << std::endl;
				}
			}

		}

		element::~element()
		{
		}

		const_shapes_ptr element::find_child_id(const std::string& id) const
		{
			for(auto& s : shapes_) {
				if(s->id() == id) {
					return s;
				}

				auto v = s->find_child_id(id);
				if(v) {
					return v;
				}
			}
			return shapes_ptr();
		}

		void element::cairo_render(render_context& ctx) const {
			for(auto& s : shapes_) {
				s->cairo_render(ctx);
			}
		}
	}
}
