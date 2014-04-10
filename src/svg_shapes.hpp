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
#include "svg_path_parse.hpp"
#include "svg_render.hpp"
#include "svg_styles.hpp"
#include "svg_transform.hpp"

namespace KRE
{
	namespace SVG
	{
		class shapes
		{
		public:
			shapes(element* doc, 
				const boost::property_tree::ptree& pt, 
				const std::set<std::string>& exclusions);
			virtual ~shapes();
			
			void cairo_render(render_context& ctx) const;
			void clip_render(render_context& ctx) const;
			
			void apply_fill(render_context& ctx) const;
			void apply_transforms(render_context& ctx) const;
			void apply_fill_color(render_context& ctx) const;
			void apply_stroke_color(render_context& ctx) const;
			void apply_font_properties(render_context& ctx) const;
			
			const std::string& id() const { return id_; }
			const std::string& clip_path_id() const { return clip_path_ref_; }
			
			const_shapes_ptr find_child(const std::string& id) const;
			const_shapes_ptr find_child_id(const std::string& id) const;
		protected:
			const font_properties& GetFontProperties() const { return font_; }
		private:
			virtual void handle_cairo_render(render_context& ctx) const = 0;
			virtual void handle_clip_render(render_context& ctx) const = 0;
			virtual const_shapes_ptr handle_find_child_id(const std::string& id) const = 0;

			void render_sub_paths(render_context& ctx) const;

			std::string id_;
			fill fill_;
			font_properties font_;
			std::vector<transform_ptr> transform_list_;
			std::vector<path_commandPtr> path_;
			std::string clip_path_ref_;
			element* doc_;
		};

		class path : public shapes
		{
		public:
			path(element* doc, const boost::property_tree::ptree& pt) 
				: shapes(doc, pt, std::set<std::string>())
			{
			}
			virtual ~path() {}
		private:
			virtual void handle_cairo_render(render_context& ctx) const override {
				// doesn't need to do anything -- sub-path rendering is handled in base class
			}
			virtual void handle_clip_render(render_context& ctx) const override {
				// doesn't need to do anything -- sub-path rendering is handled in base class
			}
			const_shapes_ptr handle_find_child_id(const std::string& id) const override {
				return shapes_ptr();
			}
		};

		class group : public shapes
		{
		public:
			group(element* doc, const boost::property_tree::ptree& pt);
			virtual ~group();
		private:
			virtual void handle_clip_render(render_context& ctx) const override;
			virtual void handle_cairo_render(render_context& ctx) const override;
			const_shapes_ptr handle_find_child_id(const std::string& id) const override;

			std::vector<shapes_ptr> shapes_;
			std::vector<shapes_ptr> defs_;
			std::vector<shapes_ptr> clip_path_;
		};
	
	}
}
