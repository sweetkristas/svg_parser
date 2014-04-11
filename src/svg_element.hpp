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

#include <boost/property_tree/ptree.hpp>
#include "geometry.hpp"
#include "svg_fwd.hpp"
#include "svg_length.hpp"
#include "svg_render.hpp"

namespace KRE
{
	namespace SVG
	{
		class element
		{
		public:
			element(const boost::property_tree::ptree& svg_data);
			virtual ~element();
			const_shapes_ptr find_child_id(const std::string& id) const;
			void cairo_render(render_context& ctx) const;

			double x() { return x_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER); };
			double y() { return y_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER); };
			double width() { return width_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER); };
			double height() { return height_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER); };
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
	}
}
