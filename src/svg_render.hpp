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

#include <cairo/cairo.h>
#include <stack>

#include "asserts.hpp"
#include "svg_paint.hpp"

namespace KRE
{
	namespace SVG
	{
		class render_context
		{
		public:
			render_context(cairo_t* cairo)
				: cairo_(cairo)
			{}
			~render_context() {
				ASSERT_LOG(fill_color_stack_.empty(), "Fill color stack in rendering context not empty at exit");
				ASSERT_LOG(stroke_color_stack_.empty(), "Stroke color stack in rendering context not empty at exit");
			}

			cairo_t* cairo() { return cairo_; }
			
			void fill_color_push(const paint& p) {
				fill_color_stack_.emplace(p);
			}
			paint fill_color_pop() {
				auto p = fill_color_stack_.top();
				fill_color_stack_.pop();
				return p;
			}
			paint fill_color_top() const {
				return fill_color_stack_.top();
			}

			void stroke_color_push(const paint& p) {
				stroke_color_stack_.emplace(p);
			}
			paint stroke_color_pop() {
				auto p = stroke_color_stack_.top();
				stroke_color_stack_.pop();
				return p;
			}
			paint stroke_color_top() const {
				return stroke_color_stack_.top();
			}

			void opacity_push(double alpha) {
				opacity_stack_.push(alpha);
			}
			double opacity_pop() {
				double alpha = opacity_stack_.top();
				opacity_stack_.pop();
				return alpha;
			}
			double opacity_top() const {
				return opacity_stack_.top();
			}
		private:
			cairo_t* cairo_;
			std::stack<paint> fill_color_stack_;
			std::stack<paint> stroke_color_stack_;
			std::stack<double> opacity_stack_;
		};

	}
}
