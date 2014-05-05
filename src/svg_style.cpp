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

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

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
			const ptree & attributes = pt.get_child("<xmlattr>", ptree());
            auto font_weight = attributes.get_child_optional("font-weight");
			if(font_weight) {
				const std::string& fw = font_weight->data();
				if(fw == "inherit") {
					weight_ = FontWeight::INHERIT;
				} else if(fw == "bold") {
					weight_ = FontWeight::WEIGHT_700;
				} else if(fw == "normal") {
					weight_ = FontWeight::WEIGHT_400;
				} else if(fw == "bolder") {
					weight_ = FontWeight::BOLDER;
				} else if(fw == "lighter") {
					weight_ = FontWeight::LIGHTER;
				} else if(fw == "100") {
					weight_ = FontWeight::WEIGHT_100;
				} else if(fw == "200") {
					weight_ = FontWeight::WEIGHT_200;
				} else if(fw == "300") {
					weight_ = FontWeight::WEIGHT_300;
				} else if(fw == "400") {
					weight_ = FontWeight::WEIGHT_400;
				} else if(fw == "500") {
					weight_ = FontWeight::WEIGHT_500;
				} else if(fw == "600") {
					weight_ = FontWeight::WEIGHT_600;
				} else if(fw == "700") {
					weight_ = FontWeight::WEIGHT_700;
				} else if(fw == "800") {
					weight_ = FontWeight::WEIGHT_800;
				} else if(fw == "900") {
					weight_ = FontWeight::WEIGHT_900;
				}
			}

            auto font_variant = attributes.get_child_optional("font-variant");
			if(font_variant) {
				const std::string& fv = font_variant->data();
				if(fv == "inherit") {
					variant_ = FontVariant::INHERIT;
				} else if(fv == "normal") {
					variant_ = FontVariant::NORMAL;
				} else if(fv == "small-caps") {
					variant_ = FontVariant::SMALL_CAPS;
				}
			}

			auto font_style = attributes.get_child_optional("font-style");
			if(font_style) {
				const std::string& fs = font_style->data();
				if(fs == "inherit") {
					style_ = FontStyle::INHERIT;
				} else if(fs == "normal") {
					style_ = FontStyle::NORMAL;
				} else if(fs == "italic") {
					style_ = FontStyle::ITALIC;
				} else if(fs == "oblique") {
					style_ = FontStyle::OBLIQUE;
				}
			}

			auto font_stretch = attributes.get_child_optional("font-stretch");
			if(font_variant) {
				const std::string& fs = font_stretch->data();
				if(fs == "inherit") {
					stretch_ = FontStretch::INHERIT;
				} else if(fs == "normal") {
					stretch_ = FontStretch::NORMAL;
				} else if(fs == "wider") {
					stretch_ = FontStretch::WIDER;
				} else if(fs == "narrower") {
					stretch_ = FontStretch::NARROWER;
				} else if(fs == "ultra-condensed") {
					stretch_ = FontStretch::ULTRA_CONDENSED;
				} else if(fs == "extra-condensed") {
					stretch_ = FontStretch::EXTRA_CONDENSED;
				} else if(fs == "condensed") {
					stretch_ = FontStretch::CONDENSED;
				} else if(fs == "semi-condensed") {
					stretch_ = FontStretch::SEMI_CONDENSED;
				} else if(fs == "semi-expanded") {
					stretch_ = FontStretch::SEMI_EXPANDED;
				} else if(fs == "expanded") {
					stretch_ = FontStretch::EXPANDED;
				} else if(fs == "extra-expanded") {
					stretch_ = FontStretch::EXTRA_EXPANDED;
				} else if(fs == "ultra-expanded") {
					stretch_ = FontStretch::ULTRA_EXPANDED;
				}
			}

			auto font_size = attributes.get_child_optional("font-size");
			if(font_size) {
				const std::string& fs = font_size->data();
				if(fs == "inherit") {
					size_ = FontSize::INHERIT;
				} else if(fs == "xx-small") {
					size_ = FontSize::XX_SMALL;
				} else if(fs == "x-small") {
					size_ = FontSize::X_SMALL;
				} else if(fs == "small") {
					size_ = FontSize::SMALL;
				} else if(fs == "medium") {
					size_ = FontSize::MEDIUM;
				} else if(fs == "large") {
					size_ = FontSize::LARGE;
				} else if(fs == "x-large") {
					size_ = FontSize::X_LARGE;
				} else if(fs == "xx-large") {
					size_ = FontSize::XX_LARGE;
				} else if(fs == "larger") {
					size_ = FontSize::LARGER;
				} else if(fs == "smaller") {
					size_ = FontSize::SMALLER;
				} else {
					size_ = FontSize::VALUE;
					size_value_ = svg_length(fs);
				}
			}

			auto font_size_adjust = attributes.get_child_optional("font-size-adjust");
			if(font_size_adjust) {
				const std::string& fsa = font_size_adjust->data();
				if(fsa == "inherit") {
					size_adjust_ = FontSizeAdjust::INHERIT;
				} else if(fsa == "none") {
					size_adjust_ = FontSizeAdjust::NONE;
				} else {
					size_adjust_ = FontSizeAdjust::VALUE;
					size_adjust_value_ = svg_length(fsa);
				}
			}

			if(attributes.get_child_optional("font")) {
				ASSERT_LOG(false, "'font' attribute unimplemented.");
			}
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
			const ptree & attributes = pt.get_child("<xmlattr>", ptree());
            auto direction = attributes.get_child_optional("direction");
			if(direction) {
				const std::string& dir = direction->data();
				if(dir == "inherit") {
					direction_ = TextDirection::INHERIT;
				} else if(dir == "ltr") {
					direction_ = TextDirection::LTR;
				} else if(dir == "rtl") {
					direction_ = TextDirection::RTL;
				}
			}

			auto unicode_bidi = attributes.get_child_optional("unicode-bidi");
			if(unicode_bidi) {
				const std::string& bidi = unicode_bidi->data();
				if(bidi == "inherit") {
					bidi_ = UnicodeBidi::INHERIT;
				} else if(bidi == "normal") {
					bidi_ = UnicodeBidi::NORMAL;
				} else if(bidi == "embed") {
					bidi_ = UnicodeBidi::EMBED;
				} else if(bidi == "bidi-override") {
					bidi_ = UnicodeBidi::BIDI_OVERRIDE;
				}
			}

			auto letter_spacing = attributes.get_child_optional("letter-spacing");
			if(letter_spacing) {
				const std::string& ls = letter_spacing->data();
				if(ls == "inherit") {
					letter_spacing_ = TextSpacing::INHERIT;
				} else if(ls == "normal") {
					letter_spacing_ = TextSpacing::NORMAL;
				} else {
					letter_spacing_ = TextSpacing::VALUE;
					letter_spacing_value_ = svg_length(ls);
				}
			}

			auto word_spacing = attributes.get_child_optional("word-spacing");
			if(word_spacing) {
				const std::string& ws = word_spacing->data();
				if(ws == "inherit") {
					word_spacing_ = TextSpacing::INHERIT;
				} else if(ws == "normal") {
					word_spacing_ = TextSpacing::NORMAL;
				} else {
					word_spacing_ = TextSpacing::VALUE;
					word_spacing_value_ = svg_length(ws);
				}
			}

			auto kerning = attributes.get_child_optional("kerning");
			if(kerning) {
				const std::string& kern = kerning->data();
				if(kern == "inherit") {
					kerning_ = Kerning::INHERIT;
				} else if(kern == "auto") {
					kerning_ = Kerning::AUTO;
				} else {
					kerning_ = Kerning::VALUE;
					kerning_value_ = svg_length(kern);
				}
			}

			auto text_decoration = attributes.get_child_optional("text-decoration");
			if(text_decoration) {
				const std::string& td = text_decoration->data();
				if(td == "inherit") {
					decoration_ = TextDecoration::INHERIT;
				} else if(td == "none") {
					decoration_ = TextDecoration::NONE;
				} else if(td == "underline") {
					decoration_ = TextDecoration::UNDERLINE;
				} else if(td == "overline") {
					decoration_ = TextDecoration::OVERLINE;
				} else if(td == "blink") {
					decoration_ = TextDecoration::BLINK;
				} else if(td == "line-through") {
					decoration_ = TextDecoration::LINE_THROUGH;
				}
			}

			auto writing_mode = attributes.get_child_optional("writing-mode");
			if(writing_mode) {
				const std::string& wm = writing_mode->data();
				if(wm == "inherit") {
					writing_mode_ = WritingMode::INHERIT;
				} else if(wm == "lr-tb") {
					writing_mode_ = WritingMode::LR_TB;
				} else if(wm == "rl-tb") {
					writing_mode_ = WritingMode::RL_TB;
				} else if(wm == "tb-rl") {
					writing_mode_ = WritingMode::TB_RL;
				} else if(wm == "lr") {
					writing_mode_ = WritingMode::LR;
				} else if(wm == "rl") {
					writing_mode_ = WritingMode::RL;
				} else if(wm == "tb") {
					writing_mode_ = WritingMode::TB;
				}
			}

			auto baseline_alignment = attributes.get_child_optional("alignment-baseline");
			if(baseline_alignment) {
				const std::string& ba = baseline_alignment->data();
				if(ba == "inherit") {
					baseline_alignment_ = TextAlignmentBaseline::INHERIT;
				} else if(ba == "auto") {
					baseline_alignment_ = TextAlignmentBaseline::AUTO;
				} else if(ba == "baseline") {
					baseline_alignment_ = TextAlignmentBaseline::BASELINE;
				} else if(ba == "before-edge") {
					baseline_alignment_ = TextAlignmentBaseline::BEFORE_EDGE;
				} else if(ba == "text-before-edge") {
					baseline_alignment_ = TextAlignmentBaseline::TEXT_BEFORE_EDGE;
				} else if(ba == "middle") {
					baseline_alignment_ = TextAlignmentBaseline::MIDDLE;
				} else if(ba == "central") {
					baseline_alignment_ = TextAlignmentBaseline::CENTRAL;
				} else if(ba == "after-edge") {
					baseline_alignment_ = TextAlignmentBaseline::AFTER_EDGE;
				} else if(ba == "text-after-edge") {
					baseline_alignment_ = TextAlignmentBaseline::TEXT_AFTER_EDGE;
				} else if(ba == " ideographic") {
					baseline_alignment_ = TextAlignmentBaseline::IDEOGRAPHIC;
				} else if(ba == "alphabetic") {
					baseline_alignment_ = TextAlignmentBaseline::ALPHABETIC;
				} else if(ba == "hanging") {
					baseline_alignment_ = TextAlignmentBaseline::HANGING;
				} else if(ba == "mathematical") {
					baseline_alignment_ = TextAlignmentBaseline::MATHEMATICAL;
				}
			}

			auto baseline_shift = attributes.get_child_optional("baseline-shift");
			if(baseline_shift) {
				const std::string& bs = baseline_shift->data();
				if(bs == "inherit") {
					baseline_shift_ = TextBaselineShift::INHERIT;
				} else if(bs == "baseline") {
					baseline_shift_ = TextBaselineShift::BASELINE;
				} else if(bs == "sub") {
					baseline_shift_ = TextBaselineShift::SUB;
				} else if(bs == "super") {
					baseline_shift_ = TextBaselineShift::SUPER;
				}
			}

			auto dominant_baseline = attributes.get_child_optional("dominant-baseline");
			if(dominant_baseline) {
				const std::string& db = dominant_baseline->data();
				if(db == "inherit") {
					dominant_baseline_ = TextDominantBaseline::INHERIT;
				} else if(db == "auto") {
					dominant_baseline_ = TextDominantBaseline::AUTO;
				} else if(db == "use-script") {
					dominant_baseline_ = TextDominantBaseline::USE_SCRIPT;
				} else if(db == "no-change") {
					dominant_baseline_ = TextDominantBaseline::NO_CHANGE;
				} else if(db == "reset-size") {
					dominant_baseline_ = TextDominantBaseline::RESET_SIZE;
				} else if(db == " ideographic") {
					dominant_baseline_ = TextDominantBaseline::IDEOGRAPHIC;
				} else if(db == "alphabetic") {
					dominant_baseline_ = TextDominantBaseline::ALPHABETIC;
				} else if(db == "hanging") {
					dominant_baseline_ = TextDominantBaseline::HANGING;
				} else if(db == "mathematical") {
					dominant_baseline_ = TextDominantBaseline::MATHEMATICAL;
				} else if(db == "central") {
					dominant_baseline_ = TextDominantBaseline::CENTRAL;
				} else if(db == "middle") {
					dominant_baseline_ = TextDominantBaseline::MIDDLE;
				} else if(db == "text-after-edge") {
					dominant_baseline_ = TextDominantBaseline::TEXT_AFTER_EDGE;
				} else if(db == "text-before-edge") {
					dominant_baseline_ = TextDominantBaseline::TEXT_BEFORE_EDGE;
				}
			}

			auto glyph_orientation_vert = attributes.get_child_optional("glyph-orientation-vertical");
			if(glyph_orientation_vert) {
				const std::string& go = glyph_orientation_vert->data();
				if(go == "inherit") {
					glyph_orientation_vertical_ = GlyphOrientation::INHERIT;
				} else if(go == "auto") {
					glyph_orientation_vertical_ = GlyphOrientation::AUTO;
				} else {
					glyph_orientation_vertical_ = GlyphOrientation::VALUE;
					try {
						glyph_orientation_vertical_value_ = boost::lexical_cast<double>(go);
						// test for 0,90,180,270 degree values
					} catch(boost::bad_lexical_cast&) {
						ASSERT_LOG(false, "Bad numeric conversion of glyph-orientation-vertical value: " << go);
					}
				}
			}

			auto glyph_orientation_horz = attributes.get_child_optional("glyph-orientation-horizontal");
			if(glyph_orientation_horz) {
				const std::string& go = glyph_orientation_horz->data();
				if(go == "inherit") {
					glyph_orientation_horizontal_ = GlyphOrientation::INHERIT;
				} else if(go == "auto") {
					glyph_orientation_horizontal_ = GlyphOrientation::AUTO;
				} else {
					glyph_orientation_horizontal_ = GlyphOrientation::VALUE;
					try {
						glyph_orientation_horizontal_value_ = boost::lexical_cast<double>(go);
						// test for 0,90,180,270 degree values
					} catch(boost::bad_lexical_cast&) {
						ASSERT_LOG(false, "Bad numeric conversion of glyph-orientation-horizontal value: " << go);
					}
				}
			}
		}

		text_attribs::~text_attribs()
		{
		}

		visual_attribs::visual_attribs(const ptree& pt)
			: overflow_(Overflow::VISIBLE),
			clip_(Clip::AUTO),
			display_(Display::INLINE),
			visibility_(Visibility::VISIBLE),
			current_color_value_(0,0,0),
			cursor_(Cursor::AUTO)
		{
			const ptree & attributes = pt.get_child("<xmlattr>", ptree());
			
			auto overflow = attributes.get_child_optional("overflow");
			if(overflow) {
				const std::string& ovf = overflow->data();
				if(ovf == "inherit") {
					overflow_ = Overflow::INHERIT;
				} else if(ovf == "visible") {
					overflow_ = Overflow::VISIBLE;
				} else if(ovf == "hidden") {
					overflow_ = Overflow::HIDDEN;
				} else if(ovf == "scroll") {
					overflow_ = Overflow::SCROLL;
				}
			}

			auto clip = attributes.get_child_optional("clip");
			if(clip) {
				const std::string& clp = clip->data();
				if(clp == "inherit") {
					clip_ = Clip::INHERIT;
				} else if(clp == "auto") {
					clip_ = Clip::AUTO;
				} else {
					clip_ = Clip::SHAPE;
					boost::char_separator<char> seperators(" \n\t\r,", "();");
					boost::tokenizer<boost::char_separator<char>> tok(clp, seperators);
					auto it = tok.begin();
					ASSERT_LOG(*it == "rect", "Only supported clip shape is 'rect'");
					++it;
					ASSERT_LOG(it != tok.end(), "Inufficient values given to clip 'rect' shape.");
					if(*it == "auto") {
						clip_y1_ = svg_length(0,svg_length::SVG_LENGTHTYPE_NUMBER);
					} else {
						clip_y1_ = svg_length(*it);
					}
					++it;
					ASSERT_LOG(it != tok.end(), "Inufficient values given to clip 'rect' shape.");
					if(*it == "auto") {
						clip_x2_ = svg_length(0,svg_length::SVG_LENGTHTYPE_NUMBER);
					} else {
						clip_x2_ = svg_length(*it);
					}
					++it;
					ASSERT_LOG(it != tok.end(), "Inufficient values given to clip 'rect' shape.");
					if(*it == "auto") {
						clip_y2_ = svg_length(0,svg_length::SVG_LENGTHTYPE_NUMBER);
					} else {
						clip_y2_ = svg_length(*it);
					}
					++it;
					ASSERT_LOG(it != tok.end(), "Inufficient values given to clip 'rect' shape.");
					if(*it == "auto") {
						clip_x1_ = svg_length(0,svg_length::SVG_LENGTHTYPE_NUMBER);
					} else {
						clip_x1_ = svg_length(*it);
					}
				}
			}

			auto cursor = attributes.get_child_optional("cursor");
			if(cursor) {
				const std::string& curs = cursor->data();
				if(curs == "inherit") {
					cursor_ = Cursor::INHERIT;
				} else {
					boost::char_separator<char> seperators(" \n\t\r,");
					boost::tokenizer<boost::char_separator<char>> tok(curs, seperators);
					for(auto it = tok.begin(); it != tok.end(); ++it) {
						if(*it == "auto") {
							cursor_ = Cursor::AUTO;
						} else if(*it == "crosshair") {
							cursor_ = Cursor::CROSSHAIR;
						} else if(*it == "default") {
							cursor_ = Cursor::DEFAULT;
						} else if(*it == "pointer") {
							cursor_ = Cursor::POINTER;
						} else if(*it == "move") {
							cursor_ = Cursor::MOVE;
						} else if(*it == "e-resize") {
							cursor_ = Cursor::E_RESIZE;
						} else if(*it == "ne-resize") {
							cursor_ = Cursor::NE_RESIZE;
						} else if(*it == "nw-resize") {
							cursor_ = Cursor::NW_RESIZE;
						} else if(*it == "n-resize") {
							cursor_ = Cursor::N_RESIZE;
						} else if(*it == "se-resize") {
							cursor_ = Cursor::SE_RESIZE;
						} else if(*it == "sw-resize") {
							cursor_ = Cursor::SW_RESIZE;
						} else if(*it == "s-resize") {
							cursor_ = Cursor::S_RESIZE;
						} else if(*it == "w-resize") {
							cursor_ = Cursor::W_RESIZE;
						} else if(*it == "text") {
							cursor_ = Cursor::TEXT;
						} else if(*it == "wait") {
							cursor_ = Cursor::WAIT;
						} else if(*it == "help") {
							cursor_ = Cursor::HELP;
						} else {
							if(it->substr(0,4) == "url(" && it->back() == ')') {
								cursor_funciri_.emplace_back(it->substr(4,it->size()-5));
							}
						}
					}
				}
			}

			auto display = attributes.get_child_optional("display");
			if(display) {
				const std::string& disp = display->data();
				if(disp == "inherit") {
					display_ = Display::INHERIT;
				} else if(disp == "inline") {
					display_ = Display::INLINE;
				} else if(disp == "block") {
					display_ = Display::BLOCK;
				} else if(disp == "list-item") {
					display_ = Display::LIST_ITEM;
				} else if(disp == "run-in") {
					display_ = Display::RUN_IN;
				} else if(disp == "compact") {
					display_ = Display::COMPACT;
				} else if(disp == "marker") {
					display_ = Display::MARKER;
				} else if(disp == "table") {
					display_ = Display::TABLE;
				} else if(disp == "inline-table") {
					display_ = Display::INLINE_TABLE;
				} else if(disp == "table-row-group") {
					display_ = Display::TABLE_ROW_GROUP;
				} else if(disp == "table-header-group") {
					display_ = Display::TABLE_HEADER_GROUP;
				} else if(disp == "table-footer-group") {
					display_ = Display::TABLE_FOOTER_GROUP;
				} else if(disp == "table-row") {
					display_ = Display::TABLE_ROW;
				} else if(disp == "table-column-group") {
					display_ = Display::TABLE_COLUMN_GROUP;
				} else if(disp == "table-column") {
					display_ = Display::TABLE_COLUMN;
				} else if(disp == "table-cell") {
					display_ = Display::TABLE_CELL;
				} else if(disp == "table-caption") {
					display_ = Display::TABLE_CAPTION;
				} else if(disp == "none") {
					display_ = Display::NONE;
				}
			}

			auto visibility = attributes.get_child_optional("visibility");
			if(visibility) {
				const std::string& vis = visibility->data();
				if(vis == "inherit") {
					visibility_ = Visibility::INHERIT;
				} else if(vis == "visible") {
					visibility_ = Visibility::VISIBLE;
				} else if(vis == "hidden") {
					visibility_ = Visibility::HIDDEN;
				} else if(vis == "collapse") {
					visibility_ = Visibility::COLLAPSE;
				}
			}

			auto color = attributes.get_child_optional("color");
			if(color) {
				const std::string& c = color->data();
				if(c == "inherit") {
					current_color_ = ColorAttrib::INHERIT;
				} else {
					current_color_ = ColorAttrib::VALUE;
					current_color_value_ = paint(c);
				}
			}
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
			const ptree & attributes = pt.get_child("<xmlattr>", ptree());

			auto mask = attributes.get_child_optional("mask");
			if(mask) {
				const std::string& msk = mask->data();
				if(msk == "inherit") {
					mask_ = FuncIriValue::INHERIT;
				} else if(msk == "none") {
					mask_ = FuncIriValue::NONE;
				} else {
					mask_ = FuncIriValue::FUNC_IRI;
					if(msk.substr(0,4) == "url(" && msk.back() == ')') {
						mask_ref_ = msk.substr(4,msk.size()-5);
					}
				}
			}

			auto path = attributes.get_child_optional("clip-path");
			if(path) {
				const std::string& pth = path->data();
				if(pth == "inherit") {
					path_ = FuncIriValue::INHERIT;
				} else if(pth == "none") {
					path_ = FuncIriValue::NONE;
				} else {
					path_ = FuncIriValue::FUNC_IRI;
					if(pth.substr(0,4) == "url(" && pth.back() == ')') {
						path_ref_ = pth.substr(4,pth.size()-5);
					}
				}
			}

			auto rule = attributes.get_child_optional("clip-rule");
			if(rule) {
				const std::string& r = rule->data();
				if(r == "inherit") {
					rule_ = ClipRule::INHERIT;
				} else if(r == "nonzero") {
					rule_ = ClipRule::NON_ZERO;
				} else if(r == "evenodd") {
					rule_ = ClipRule::EVEN_ODD;
				}
			}

			auto opacity = attributes.get_child_optional("opacity");
			if(opacity) {
				const std::string& o = opacity->data();
				if(o == "inherit") {
					opacity_ = Opacity::INHERIT;
				} else {
					opacity_ = Opacity::VALUE;
					try {
						opacity_value_ = boost::lexical_cast<double>(o);
					} catch(boost::bad_lexical_cast&) {
						ASSERT_LOG(false, "Unable to obtain object/group opacity value: " << o);
					}
				}
			}
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
			const ptree & attributes = pt.get_child("<xmlattr>", ptree());
		}

		filter_effect_attribs::~filter_effect_attribs()
		{
		}

		painting_properties::painting_properties(const ptree& pt)
			: stroke_(ColorAttrib::NONE),
			stroke_value_(0,0,0),
			stroke_opacity_(OpacityAttrib::VALUE),
			stroke_opacity_value_(1.0),
			stroke_width_(StrokeWidthAttrib::VALUE),
			stroke_width_value_(1.0),
			stroke_linecap_(LineCapAttrib::BUTT),
			stroke_linejoin_(LineJoinAttrib::MITER),
			stroke_miter_limit_(MiterLimitAttrib::VALUE),
			stroke_miter_limit_value_(4.0),
			stroke_dash_array_(DashArrayAttrib::NONE),
			stroke_dash_offset_(DashOffsetAttrib::VALUE),
			stroke_dash_offset_value_(0),
			fill_(ColorAttrib::VALUE),
			fill_value_(0,0,0),
			fill_rule_(FillRuleAttrib::EVENODD),
			fill_opacity_(OpacityAttrib::VALUE),
			fill_opacity_value_(1.0),
			color_interpolation_(ColorInterpolationAttrib::sRGBA),
			color_interpolation_filters_(ColorInterpolationAttrib::linearRGBA),
			color_rendering_(RenderingAttrib::AUTO),
			shape_rendering_(ShapeRenderingAttrib::AUTO),
			text_rendering_(TextRenderingAttrib::AUTO),
			image_rendering_(RenderingAttrib::AUTO),
			color_profile_(ColorProfileAttrib::AUTO)
		{
			const ptree & attributes = pt.get_child("<xmlattr>", ptree());
		}

		painting_properties::~painting_properties()
		{
		}

	}
}
