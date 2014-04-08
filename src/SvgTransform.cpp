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

#include <boost/tokenizer.hpp>
#include <sstream>

#include <cmath>
#include "asserts.hpp"
#include "SvgTransform.hpp"

namespace KRE
{
	class MatrixTransform : public Transform
	{
	public:
		MatrixTransform(const std::vector<double>& params) : Transform(TransformType::MATRIX) {
			// Ordering of matrix parameters is as follows.
			// [ 0  2  4 ]
			// [ 1  3  5 ]
			// which neatly is the same order as cairo expects.
			ASSERT_LOG(params.size() == 6, 
				"Parsing transform:matrix found " 
				<< params.size() 
				<< " parameter(s), expected 6");
			cairo_matrix_init(&mat_, params[0], params[1], params[2], params[3], params[4], params[5]);
		}
		virtual ~MatrixTransform() {}
		std::string as_string() const override {
			std::stringstream str;
			str << "matrix(" 
				<< mat_.xx << " " << mat_.yx << " " 
				<< mat_.xy << " " << mat_.yy << " " 
				<< mat_.x0 << " " << mat_.y0 
				<< ")";
			return str.str();
		}
	private:
		void HandleApply(cairo_t* cairo) override {
			cairo_transform(cairo, &mat_);
		}
		cairo_matrix_t mat_;
	};

	class TranslateTransform : public Transform
	{
	public:
		TranslateTransform(double x, double y) : Transform(TransformType::TRANSLATE), x_(x), y_(y) {
		}
		virtual ~TranslateTransform() {}
		std::string as_string() const override {
			std::stringstream str;
			str << "translate(" << x_ << " " << y_ << ")";
			return str.str();
		}
	private:
		void HandleApply(cairo_t* cairo) override {
			//std::cerr << "XXX: " << as_string() << std::endl;
			cairo_translate(cairo, x_, y_);
		}
		double x_;
		double y_;
	};

	class RotationTransform : public Transform
	{
	public:
		RotationTransform(double angle, double cx=0, double cy=0) 
			: Transform(TransformType::ROTATE), 
			angle_(angle),
			cx_(cx),
			cy_(cy) {
		}
		virtual ~RotationTransform() {}
		std::string as_string() const override {
			std::stringstream str;
			if(std::abs(cx_) < DBL_EPSILON && std::abs(cy_) < DBL_EPSILON) {
				str << "rotate(" << angle_ << ")";
			} else {
				str << "rotate(" << angle_ << " " << cx_ << " " << cy_ << ")";
			}
			return str.str();
		}
	private:
		void HandleApply(cairo_t* cairo) override {
			if(std::abs(cx_) < DBL_EPSILON && std::abs(cy_) < DBL_EPSILON) {
				cairo_rotate(cairo, angle_);
			} else {
				cairo_translate(cairo, cx_, cy_);
				cairo_rotate(cairo, angle_);
				cairo_translate(cairo, -cx_, -cy_);
			}
		}
		double angle_;
		double cx_;
		double cy_;
	};

	class ScaleTransform : public Transform
	{
	public:
		ScaleTransform(double sx, double sy) : Transform(TransformType::SCALE), sx_(sx), sy_(sy) {
		}
		virtual ~ScaleTransform() {}
		std::string as_string() const override {
			std::stringstream str;
			str << "scale(" << sx_ << " " << sy_ << ")";
			return str.str();
		}
	private:
		void HandleApply(cairo_t* cairo) override {
			cairo_scale(cairo, sx_, sy_);
		}
		double sx_;
		double sy_;
	};

	class SkewXTransform : public Transform
	{
	public:
		SkewXTransform(double sx) : Transform(TransformType::SKEW_X), sx_(sx) {
			cairo_matrix_init(&mat_, 1, 0, sx_, 1, 0, 0);
		}
		virtual ~SkewXTransform() {}
		std::string as_string() const override {
			std::stringstream str;
			str << "skewX(" << sx_ << ")";
			return str.str();
		}
	private:
		void HandleApply(cairo_t* cairo) override {
			cairo_transform(cairo, &mat_);
		}
		double sx_;
		cairo_matrix_t mat_;
	};

	class SkewYTransform : public Transform
	{
	public:
		SkewYTransform(double sy) : Transform(TransformType::SKEW_Y), sy_(sy) {
			cairo_matrix_init(&mat_, 1, sy_, 0, 1, 0, 0);
		}
		virtual ~SkewYTransform() {}
		std::string as_string() const override {
			std::stringstream str;
			str << "skewY(" << sy_ << ")";
			return str.str();
		}
	private:
		void HandleApply(cairo_t* cairo) override {
			cairo_transform(cairo, &mat_);
		}
		double sy_;
		cairo_matrix_t mat_;
	};

	Transform::Transform(TransformType tt)
		: type_(tt)
	{
	}

	void Transform::Apply(cairo_t* cairo)
	{
		HandleApply(cairo);
	}

	std::vector<TransformPtr> Transform::Factory(const std::string& s)
	{
		std::vector<TransformPtr> results;
		enum {
			STATE_TYPE,
			STATE_NUMBER,
		} state = STATE_TYPE;
		
		std::vector<double> parameters;

		TransformType type;

		boost::char_separator<char> seperators(" \n\t\r,", "()");
		boost::tokenizer<boost::char_separator<char>> tok(s, seperators);
		for(auto it = tok.begin(); it != tok.end(); ++it) {
			if(state == STATE_TYPE) {
				if(*it == "matrix") {
					type = TransformType::MATRIX;
				} else if(*it == "translate") {
					type = TransformType::TRANSLATE;
				} else if(*it == "scale") {
					type = TransformType::SCALE;
				} else if(*it == "rotate") {
					type = TransformType::ROTATE;
				} else if(*it == "skewX") {
					type = TransformType::SKEW_X;
				} else if(*it == "skewY") {
					type = TransformType::SKEW_Y;
				} else if(*it == "(") {
					parameters.clear();
					state = STATE_NUMBER;
				} else {
					ASSERT_LOG(false, "Unexpected token while looking for a type: " << *it << " : " << s);
				}
			} else if(state == STATE_NUMBER) {
				if(*it == ")") {
					switch(type) {
						case TransformType::MATRIX: {
							MatrixTransform* mtrf = new MatrixTransform(parameters);
							results.emplace_back(mtrf);
							break;
						}
						case TransformType::TRANSLATE: {
							ASSERT_LOG(parameters.size() == 1 || parameters.size() == 2, "Parsing transform:translate found " << parameters.size() << " parameter(s), expected 1 or 2");
							double tx = parameters[0];
							double ty = parameters.size() == 2 ? parameters[1] : 0.0f;
							TranslateTransform * ttrf = new TranslateTransform(tx, ty);
							results.emplace_back(ttrf);
							break;
						}
						case TransformType::SCALE: {
							ASSERT_LOG(parameters.size() == 1 || parameters.size() == 2, "Parsing transform:scale found " << parameters.size() << " parameter(s), expected 1 or 2");
							double sx = parameters[0];
							double sy = parameters.size() == 2 ? parameters[1] : sx;
							ScaleTransform * strf = new ScaleTransform(sx, sy);
							results.emplace_back(strf);
							break;
						}
						case TransformType::ROTATE: {
							ASSERT_LOG(parameters.size() == 1 || parameters.size() == 3, "Parsing transform:rotate found " << parameters.size() << " parameter(s), expected 1 or 3");
							double angle = parameters[0] / 180.0 * M_PI;
							double cx = parameters.size() == 3 ? parameters[1] : 0;
							double cy = parameters.size() == 3 ? parameters[2] : 0;
							RotationTransform* rtrf = new RotationTransform(angle, cx, cy);
							results.emplace_back(rtrf);
							break;
						}
						case TransformType::SKEW_X: {
							ASSERT_LOG(parameters.size() == 1, "Parsing transform:skewX found " << parameters.size() << " parameter(s), expected 1");
							double sa = tan(parameters[0]);
							SkewXTransform* sxtrf = new SkewXTransform(sa);
							results.emplace_back(sxtrf);
							break;
						}
						case TransformType::SKEW_Y: {
							ASSERT_LOG(parameters.size() == 1, "Parsing transform:skewY found " << parameters.size() << " parameter(s), expected 1");
							double sa = tan(parameters[0]);
							SkewYTransform* sxtrf = new SkewYTransform(sa);
							results.emplace_back(sxtrf);
							break;
						}
					}
					state = STATE_TYPE;					
				} else {
					char* end = NULL;
					double value = strtod(it->c_str(), &end);
					if(value == 0 && it->c_str() == end) {
						ASSERT_LOG(false, "Invalid number value: " << *it);
					}
					ASSERT_LOG(errno != ERANGE, "Parsed numeric value out-of-range: " << *it);					
					parameters.push_back(value);
				}
			}
		}

		//std::cerr << "Transforms: ";
		//for(auto& res : results) {
		//	std::cerr << res->as_string() << " ";
		//}
		//std::cerr << std::endl;

		return results;
	}

	Transform::~Transform()
	{
	}
}
