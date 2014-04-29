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
#include "svg_attribs.hpp"
#include "svg_fwd.hpp"
#include "svg_gradient.hpp"
#include "svg_render.hpp"
#include "svg_transform.hpp"

namespace KRE
{
	namespace SVG
	{
		enum class ZoomAndPan {
			DISABLE,
			MAGNIFY,
		};

		class container : public element
		{
		public:
			container(element* parent, const boost::property_tree::ptree& pt);
			virtual ~container();

			const_shapes_ptr find_child_id(const std::string& id) const;
		private:
			virtual void handle_render(render_context& ctx) const override;
			// Shape/Structural/Gradient elements
			std::vector<element_ptr> elements_;
		};

		class svg : public container
		{
		public:
			svg(element* parent, const boost::property_tree::ptree& pt);
			virtual ~svg();

			double x() { return x_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER); };
			double y() { return y_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER); };
			double width() { return width_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER); };
			double height() { return height_.value_in_specified_units(svg_length::SVG_LENGTHTYPE_NUMBER); };
		private:
			virtual void handle_render(render_context& ctx) const override;

			std::string version_;
			std::string base_profile_;
			std::string content_script_type_;
			std::string content_style_type_;
			std::string xmlns_;
			view_box_rect view_box_;
			svg_length x_;
			svg_length y_;
			svg_length width_;
			svg_length height_;
			//PreserveAspectRatio preserve_aspect_ratio_;
			ZoomAndPan zoom_and_pan_;
		};

		// Not rendered directly. Only rendered when called from a 'use' element.
		class symbol : public container
		{
		public:
			symbol(element* parent, const boost::property_tree::ptree& pt);
			virtual ~symbol();
		private:
			virtual void handle_render(render_context& ctx) const override;
		};

		class group : public container
		{
		public:
			group(element* parent, const boost::property_tree::ptree& pt);
			virtual ~group();
		private:
			virtual void handle_render(render_context& ctx) const override;
		};

		// Used only for looking up child elements. Not rendered directly.
		class defs : public container
		{
		public:
			defs(element* parent, const boost::property_tree::ptree& pt);
			virtual ~defs();
		private:
			virtual void handle_render(render_context& ctx) const override;
		};
	}
}
