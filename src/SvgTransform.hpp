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

#include <memory>
#include <string>
#include <vector>
#include <cairo/cairo.h>

namespace KRE
{
	class Transform;
	typedef std::shared_ptr<Transform> TransformPtr;

	class Transform
	{
	public:
		virtual ~Transform();
		virtual std::string as_string() const = 0;
		static std::vector<TransformPtr> Factory(const std::string& s);
		void Apply(cairo_t* cairo);
	protected:
		enum class TransformType {
			MATRIX,
			TRANSLATE,
			SCALE,
			ROTATE,
			SKEW_X,
			SKEW_Y,
		};
		Transform(TransformType tt);
	private:
		virtual void HandleApply(cairo_t* cairo) = 0;

		TransformType type_;
	};
}
