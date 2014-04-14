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
#include <set>
#include "svg_fwd.hpp"
#include "svg_element.hpp"
#include "svg_path_parse.hpp"
#include "svg_render.hpp"
#include "svg_styles.hpp"
#include "svg_transform.hpp"

namespace KRE
{
	namespace SVG
	{
		typedef std::vector<std::pair<svg_length,svg_length>> point_list;

		class shapes : public element
		{
		public:
			shapes(element* doc, const boost::property_tree::ptree& pt);
			virtual ~shapes();			
		private:
			virtual void handle_render(render_context& ctx) const override;
			void render_sub_paths(render_context& ctx) const;
			std::vector<path_commandPtr> path_;
		};

		class rectangle : public shapes
		{
		public:
			rectangle(element* doc, const boost::property_tree::ptree& pt);
			virtual ~rectangle();
		private:
			virtual void handle_render(render_context& ctx) const override;
			svg_length x_;
			svg_length y_;
			svg_length rx_;
			svg_length ry_;
			svg_length width_;
			svg_length height_;
			bool is_rounded_;
		};
		
		class circle : public shapes
		{
		public:
			// list_of here is a hack because MSVC doesn't support C++11 initialiser_lists
			circle(element* doc, const boost::property_tree::ptree& pt);
			virtual ~circle();
		private:
			virtual void handle_render(render_context& ctx) const override;
			svg_length cx_;
			svg_length cy_;
			svg_length radius_;
		};

		class line : public shapes
		{
		public:
			line(element* doc, const boost::property_tree::ptree& pt);
			virtual ~line();
		private:
			virtual void handle_render(render_context& ctx) const override;
			svg_length x1_;
			svg_length y1_;
			svg_length x2_;
			svg_length y2_;
		};

		class polyline : public shapes
		{
		public:
			polyline(element* doc, const boost::property_tree::ptree& pt);
			virtual ~polyline();
		private:
			virtual void handle_render(render_context& ctx) const override;
			point_list points_;
		};

		class polygon : public shapes
		{
		public:
			polygon(element* doc, const boost::property_tree::ptree& pt);
			virtual ~polygon();
		private:
			virtual void handle_render(render_context& ctx) const override;
			point_list points_;
		};

		class text : public shapes
		{
		public:
			text(element* doc, const boost::property_tree::ptree& pt);
			virtual ~text();
		private:
			virtual void handle_render(render_context& ctx) const override;
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
	}
}
