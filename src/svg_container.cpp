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

#include "svg_container.hpp"

namespace KRE
{
	namespace SVG
	{
		using namespace boost::property_tree;

		container::container(container* parent, const ptree& pt)
			: element(parent, pt),
			parent_(parent),
			external_resources_required_(false)
		{
			const ptree & attributes = pt.get_child("<xmlattr>", ptree());
			auto trf = attributes.get_child_optional("transform");
			auto exts = attributes.get_child_optional("externalResourcesRequired");

			if(trf) {
				transforms_ = transform::factory(trf->data());
			}
			if(exts) {
				const std::string& s = exts->data();
				if(s == "true") {
					external_resources_required_ = true;
				} else if(s == "false") {
					external_resources_required_ = false;
				} else {
					ASSERT_LOG(false, "Unrecognised value in 'externalResourcesRequired' attribute: " << s);
				}
			}
			ASSERT_LOG(!external_resources_required_, "We don't support getting external resources.");
		}

		container::~container()
		{
		}

		void container::apply_transforms(render_context& ctx) const
		{
			for(auto& trf : transforms_) {
				trf->apply(ctx);
			}
		}
	}
}
