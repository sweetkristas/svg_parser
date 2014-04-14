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

#include "svg_style.hpp"

namespace KRE
{
	namespace SVG
	{
		using namespace boost::property_tree;
		
		// synonym for wider.
		FontStretch operator++(FontStretch& f) {
			switch(f) {
			case FontStretch::ULTRA_CONDENSED:	f = FontStretch::EXTRA_CONDENSED; break;
			case FontStretch::EXTRA_CONDENSED:	f = FontStretch::CONDENSED; break;
			case FontStretch::CONDENSED:		f = FontStretch::SEMI_CONDENSED; break;
			case FontStretch::SEMI_CONDENSED:	f = FontStretch::NORMAL; break;
			case FontStretch::NORMAL:			f = FontStretch::SEMI_EXPANDED; break;
			case FontStretch::SEMI_EXPANDED:	f = FontStretch::EXPANDED; break;
			case FontStretch::EXPANDED:			f = FontStretch::EXTRA_EXPANDED; break;
			case FontStretch::EXTRA_EXPANDED:	f = FontStretch::ULTRA_CONDENSED; break;
			case FontStretch::ULTRA_EXPANDED:	break;
			}
			return f;
		}

		// synonym for narrower.
		FontStretch operator--(FontStretch& f) {
			switch(f) {
			case FontStretch::ULTRA_CONDENSED:	break;
			case FontStretch::EXTRA_CONDENSED:	f = FontStretch::ULTRA_CONDENSED; break;
			case FontStretch::CONDENSED:		f = FontStretch::EXTRA_CONDENSED; break;
			case FontStretch::SEMI_CONDENSED:	f = FontStretch::CONDENSED; break;
			case FontStretch::NORMAL:			f = FontStretch::SEMI_CONDENSED; break;
			case FontStretch::SEMI_EXPANDED:	f = FontStretch::NORMAL; break;
			case FontStretch::EXPANDED:			f = FontStretch::SEMI_EXPANDED; break;
			case FontStretch::EXTRA_EXPANDED:	f = FontStretch::EXPANDED; break;
			case FontStretch::ULTRA_EXPANDED:	f = FontStretch::EXTRA_CONDENSED; break;
			}
			return f;
		}

		font_attribs::font_attribs(const ptree& pt)
			: style_(FontStyle::NORMAL),
			variant_(FontVariant::NORMAL),
			stretch_(FontStretch::NORMAL),
			weight_(FontWeight::WEIGHT_400),
			size_(FontSize::MEDIUM),
			size_adjust_(FontSizeAdjust::NONE)
		{
		}

		font_attribs::~font_attribs()
		{
		}

		text_attribs::text_attribs(const ptree& pt)
			: direction_(TextDirection::LTR),
			bidi_(UnicodeBidi::NORMAL),
			letter_spacing_(TextSpacing::NORMAL),
			word_spacing_(TextSpacing::NORMAL),
			decoration_(TextDecoration::NONE),
			baseline_alignment_(TextAlignmentBaseline::AUTO),
			baseline_shift_(TextBaselineShift::BASELINE),
			dominant_baseline_(TextDominantBaseline::AUTO),
			glyph_orientation_vertical_(GlyphOrientation::AUTO),
			glyph_orientation_vertical_value_(0),
			glyph_orientation_horizontal_(GlyphOrientation::AUTO),
			glyph_orientation_horizontal_value_(0),
			writing_mode_(WritingMode::LR_TB),
			kerning_(Kerning::AUTO)
		{

		}

		text_attribs::~text_attribs()
		{
		}

		visual_attribs::visual_attribs(const ptree& pt)
			: overflow_(Overflow::VISIBLE),
			clip_(Clip::AUTO),
			cursor_(Cursor::AUTO),
			display_(Display::INLINE),
			visibility_(Visibility::VISIBLE),
			current_color_value_(0,0,0)
		{
		}

		visual_attribs::~visual_attribs()
		{
		}

		clipping_attribs::clipping_attribs(const ptree& pt)
			: path_(FuncIriValue::NONE),
			rule_(ClipRule::NON_ZERO),
			mask_(FuncIriValue::NONE),
			opacity_(Opacity::VALUE),
			opacity_value_(1.0)
		{
		}

		clipping_attribs::~clipping_attribs()
		{
		}

		filter_effect_attribs::filter_effect_attribs(const ptree& pt)
			: enable_background_(Background::ACCUMULATE),
			filter_(FuncIriValue::NONE),
			flood_color_(ColorAttrib::VALUE),
			flood_color_value_(0,0,0),
			flood_opacity_(Opacity::VALUE),
			flood_opacity_value_(1.0),
			lighting_color_(ColorAttrib::VALUE),
			lighting_color_value_("white")
		{
		}

		filter_effect_attribs::~filter_effect_attribs()
		{
		}

	}
}
