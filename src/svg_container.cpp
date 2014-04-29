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
					elements_.emplace_back(new group(this,v.second));
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
	}
}
