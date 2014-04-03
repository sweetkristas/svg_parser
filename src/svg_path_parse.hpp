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
#include <exception>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "formatter.hpp"

namespace svg
{
	enum class PathInstruction {
		MOVETO,
		LINETO,
		LINETO_H,
		LINETO_V,
		CLOSEPATH,
		CUBIC_BEZIER,
		QUADRATIC_BEZIER,
		ARC,
	};

	class PathCommand
	{
	public:
		virtual ~PathCommand();

		void CairoRender(cairo_t* cairo);

		bool IsAbsolute() const { return absolute_; }
		bool IsRelative() const { return !absolute_; }
	protected:
		PathCommand(PathInstruction ins, bool absolute);
	private:
		virtual void HandleCairoRender(cairo_t* cairo) = 0;
		PathInstruction ins_;
		bool absolute_;
	};
	typedef std::shared_ptr<PathCommand> PathCommandPtr;

	class parsing_exception : public std::exception
	{
	public:
		parsing_exception(const std::string& ss) : s_(ss) {}
		virtual ~parsing_exception() override {}
		virtual const char* what() const override {
			return s_.c_str();
		}
	private:
		std::string s_;
	};

	std::vector<PathCommandPtr> parse_path(const std::string& s);
}
