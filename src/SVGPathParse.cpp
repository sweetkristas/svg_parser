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

/*
	From www.w3.org/TR/SVG/implnote.html#PathElementImplementationNotes

The S/s commands indicate that the first control point of the given cubic Bézier segment 
is calculated by reflecting the previous path segments second control point relative to 
the current point. The exact math is as follows. If the current point is (curx, cury) 
and the second control point of the previous path segment is (oldx2, oldy2), then the 
reflected point (i.e., (newx1, newy1), the first control point of the current path segment) is:

(newx1, newy1) = (curx - (oldx2 - curx), cury - (oldy2 - cury))
               = (2*curx - oldx2, 2*cury - oldy2)

*/

#ifndef _USE_MATH_DEFINES
#	define _USE_MATH_DEFINES	1
#endif

#include <cmath>
#include <iostream>
#include <list>

#include "asserts.hpp"
#include "SVGPathParse.hpp"

namespace KRE
{
	namespace SVG
	{
		PathCommand::PathCommand(PathInstruction ins, bool absolute)
			: ins_(ins), absolute_(absolute)
		{
		}

		PathCommand::~PathCommand()
		{
		}

		void PathCommand::CairoRender(CommandContext& ctx)
		{
			HandleCairoRender(ctx);

			auto status = cairo_status(ctx.Cairo());
			ASSERT_LOG(status == CAIRO_STATUS_SUCCESS, "Cairo error: " << cairo_status_to_string(status));
		}

		class MoveToCommand : public PathCommand
		{
		public:
			MoveToCommand(bool absolute, double x, double y)
				: PathCommand(PathInstruction::MOVETO, absolute), 
				x_(x), 
				y_(y) 
			{
			}
			virtual ~MoveToCommand() {}
		private:
			void HandleCairoRender(CommandContext& ctx) override {
				if(IsAbsolute()) {
					cairo_move_to(ctx.Cairo(), x_, y_); 
				} else {
					cairo_rel_move_to(ctx.Cairo(), x_, y_);
				}
				ctx.ClearControlPoints();
			}
			double x_;
			double y_;
		};

		class LineToCommand : public PathCommand
		{
		public:
			LineToCommand(bool absolute, double x, double y)
				: PathCommand(PathInstruction::LINETO, absolute),
				x_(x),
				y_(y)
			{
			}
			virtual ~LineToCommand() {}
		private:
			void HandleCairoRender(CommandContext& ctx) override {
				if(IsAbsolute()) {
					cairo_line_to(ctx.Cairo(), x_, y_);
				} else {
					cairo_rel_line_to(ctx.Cairo(), x_, y_);
				}
				ctx.ClearControlPoints();
			}
			double x_;
			double y_;
		};

		class ClosePathCommand : public PathCommand
		{
		public:
			ClosePathCommand() : PathCommand(PathInstruction::CLOSEPATH, true)
			{
			}
			virtual ~ClosePathCommand() {}
		private:
			void HandleCairoRender(CommandContext& ctx) override {
				cairo_close_path(ctx.Cairo());
				ctx.ClearControlPoints();
			}
		};

		class LineToHCommand : public PathCommand
		{
		public:
			LineToHCommand(bool absolute, double x)
				: PathCommand(PathInstruction::LINETO_H, absolute),
				x_(x)
			{
			}
			virtual ~LineToHCommand() {}
		private:
			void HandleCairoRender(CommandContext& ctx) override {
				if(IsAbsolute()) {
					double cx, cy;
					cairo_get_current_point(ctx.Cairo(), &cx, &cy);
					cairo_line_to(ctx.Cairo(), x_, cy);
				} else {
					cairo_rel_line_to(ctx.Cairo(), x_, 0.0);
				}
			}
			double x_;
		};

		class LineToVCommand : public PathCommand
		{
		public:
			LineToVCommand(bool absolute, double y)
				: PathCommand(PathInstruction::LINETO_V, absolute),
				y_(y)
			{
			}
			virtual ~LineToVCommand() {}
		private:
			void HandleCairoRender(CommandContext& ctx) override {
				if(IsAbsolute()) {
					double cx, cy;
					cairo_get_current_point(ctx.Cairo(), &cx, &cy);
					cairo_line_to(ctx.Cairo(), cx, y_);
				} else {
					cairo_rel_line_to(ctx.Cairo(), 0.0, y_);
				}
				ctx.ClearControlPoints();
			}
			double y_;
		};

		class CubicBezierCommand : public PathCommand
		{
		public:
			CubicBezierCommand(bool absolute, bool smooth, double x, double y, double cp1x, double cp1y, double cp2x, double cp2y)
				: PathCommand(PathInstruction::CUBIC_BEZIER, absolute),
				smooth_(smooth),
				x_(x), y_(y), 
				cp1x_(cp1x), cp1y_(cp1y), 
				cp2x_(cp2x), cp2y_(cp2y)
			{
			}
			virtual ~CubicBezierCommand() {}
		private:
			void HandleCairoRender(CommandContext& ctx) override {
				double c0x, c0y;
				cairo_get_current_point(ctx.Cairo(), &c0x, &c0y);
				if(smooth_) {
					ctx.GetControlPoints(&cp1x_, &cp1y_);
					cp1x_ = 2.0*c0x - cp1x_;
					cp1y_ = 2.0*c0y - cp1y_;
					if(!IsAbsolute()) {
						cp1x_ -= c0x;
						cp1y_ -= c0y;
					}
				}
				if(IsAbsolute()) {
					cairo_curve_to(ctx.Cairo(), cp1x_, cp1y_, cp2x_, cp2y_, x_, y_);
				} else {
					cairo_rel_curve_to(ctx.Cairo(), cp1x_, cp1y_, cp2x_, cp2y_, x_, y_);
				}
				// we always write control points in absolute co-ords
				ctx.SetControlPoints(IsAbsolute() ? cp2x_ : cp2x_ + c0x, IsAbsolute() ? cp2y_ : cp2y_ + c0y);
			}
			bool smooth_;
			double x_;
			double y_;
			// control point one for cubic bezier
			double cp1x_;
			double cp1y_;
			// control point two for cubic bezier
			double cp2x_;
			double cp2y_;
		};

		class QuadraticBezierCommand : public PathCommand
		{
		public:
			QuadraticBezierCommand(bool absolute, bool smooth, double x, double y, double cp1x, double cp1y)
				: PathCommand(PathInstruction::CUBIC_BEZIER, absolute),
				smooth_(smooth),
				x_(x), y_(y), 
				cp1x_(cp1x), cp1y_(cp1y)
			{
			}
			virtual ~QuadraticBezierCommand() {}
		private:
			void HandleCairoRender(CommandContext& ctx) override {
				double c0x, c0y;
				cairo_get_current_point(ctx.Cairo(), &c0x, &c0y);
				if(smooth_) {
					double cp1x, cp1y;
					ctx.GetControlPoints(&cp1x, &cp1y);
					cp1x_ = 2.0*c0x - cp1x;
					cp1y_ = 2.0*c0y - cp1y;
					if(!IsAbsolute()) {
						cp1x_ -= c0x;
						cp1y_ -= c0y;
					}
				}
				double dx, dy;
				double acp1x, acp1y;
				// Simple quadratic -> cubic conversion.
				dx = x_;
				dy = y_;
				acp1x = cp1x_;
				acp1y = cp1y_;
				if(!IsAbsolute()) {
					dx += c0x;
					dy += c0y;
					acp1x += c0x;
					acp1y += c0y;
				}
				const double cpx1 = c0x + 2.0/3.0 * (acp1x - c0x);
				const double cpy1 = c0y + 2.0/3.0 * (acp1y - c0y);
				const double cpx2 = dx + 2.0/3.0 * (acp1x - dx);
				const double cpy2 = dy + 2.0/3.0 * (acp1y - dy);

				cairo_curve_to(ctx.Cairo(), cpx1, cpy1, cpx2, cpy2, x_, y_);

				// we always write control points in absolute co-ords
				ctx.SetControlPoints(IsAbsolute() ? cp1x_ : cp1x_ + c0x, IsAbsolute() ? cp1y_ : cp1y_ + c0y);
			}
			bool smooth_;
			double x_;
			double y_;
			// control point for quadratic bezier
			double cp1x_;
			double cp1y_;
		};

		class EllipticalArcCommand : public PathCommand
		{
		public:
			EllipticalArcCommand(bool absolute, double x, double y, double rx, double ry, double x_axis_rot, bool large_arc, bool sweep)
				: PathCommand(PathInstruction::CUBIC_BEZIER, absolute),
				x_(x), y_(y), 
				rx_(rx), ry_(ry), 
				x_axis_rotation_(x_axis_rot), 
				large_arc_flag_(large_arc), 
				sweep_flag_(sweep) 
			{
			}
			virtual ~EllipticalArcCommand() {}
		private:
			void HandleCairoRender(CommandContext& ctx) override {
				cairo_save(ctx.Cairo());
				double x1, y1;
				cairo_get_current_point(ctx.Cairo(), &x1, &y1);

				//ASSERT_LOG(rx_ > ry_, "Length of major axis is smaller than minor axis");
				if(rx_ < ry_) {
					std::swap(rx_, ry_);
				}

				// calculate some ellipse stuff
				// a is the length of the major axis
				// b is the length of the minor axis
				const double a = rx_;
				const double b = ry_;
				const double x2 = IsAbsolute() ? x_ : x_ + x1;
				const double y2 = IsAbsolute() ? y_ : y_ + y1;
				
				// http://stackoverflow.com/questions/197649/how-to-calculate-center-of-an-ellipse-by-two-points-and-radius-sizes
				const double r1 = (x1 - x2) / (2 * a);
				const double r2 = (y2 - y1) / (2 * b);
				const double a1 = std::atan2(r1, r2);
				const double a2 = std::asin(std::sqrt(r1*r1+r2*r2));
				// t1 is the angle to the first point
				double t1 = a1+a2;
				// t2 is the angle to the second point.
				double t2 = a1-a2;
				// (xc,yc) is the centre of the ellipse 
				const double xc = x1 + a*cos(t1);
				const double yc = y1 + b*sin(t1);

				// prevent drawing a line from current position to start of arc.
				cairo_new_sub_path(ctx.Cairo());

				cairo_matrix_t mxy;
				cairo_matrix_init_identity(&mxy);
				if((large_arc_flag_ && sweep_flag_ ) || (!large_arc_flag_ && !sweep_flag_ )) {
					cairo_matrix_translate(&mxy, xc, yc);
				} else {
					cairo_matrix_translate(&mxy, xc-a, yc+b);
				}
				cairo_matrix_rotate(&mxy, x_axis_rotation_);
				cairo_matrix_scale(&mxy, a, b);
				cairo_transform(ctx.Cairo(), &mxy);
				// since we're going to scale/translate the cairo arc, we make it based on a unit circle.
				if(large_arc_flag_) {
					if(sweep_flag_) {
						cairo_arc_negative(ctx.Cairo(), 0.0, 0.0, 1.0, M_PI/2.0-t1, M_PI/2.0-t2);
					} else {
						cairo_arc(ctx.Cairo(), 0.0, 0.0, 1.0, t1, t2);
					}
				} else {
					if(sweep_flag_) {
						cairo_arc_negative(ctx.Cairo(), 0.0, 0.0, 1.0, t1, t2);
					} else {
						cairo_arc(ctx.Cairo(), 0.0, 0.0, 1.0, M_PI/2.0-t1, M_PI/2.0-t2);
					}
				}

				cairo_restore(ctx.Cairo());
				cairo_move_to(ctx.Cairo(), x2, y2);
				ctx.ClearControlPoints();
			}
			bool smooth_;
			double x_;
			double y_;
			// elliptical arc radii
			double rx_;
			double ry_;
			// arc x axis rotation
			double x_axis_rotation_;
			bool large_arc_flag_;
			bool sweep_flag_;
		};


		class path_parser
		{
		public:
			path_parser(const std::string& s) : path_(s.begin(), s.end()) {
				do {
					if(path_.empty()) {
						throw parsing_exception("Found empty string");
					}
				} while(match_wsp_opt());
				match_moveto_drawto_command_groups();
				while(match_wsp_opt()) {
				}
				if(path_.size() > 0) {
					throw parsing_exception(formatter() << "Input data left after parsing: " << std::string(path_.begin(), path_.end()));
				}
			}
			bool match_wsp_opt()
			{
				if(path_.empty()) {
					return false;
				}
				char c = path_.front();
				if(c == ' ' || c == '\t' || c == '\r' || c == '\n') {
					path_.pop_front();
					return true;
				}
				return false;
			}
			void match_wsp_star()
			{
				while(match_wsp_opt()) {
				}
			}
			void match_wsp_star_or_die()
			{
				do {
					if(path_.empty()) {
						throw parsing_exception("Found empty string");
					}
				} while(match_wsp_opt());
			}
			bool match(char c)
			{
				if(path_.empty()) {
					return false;
				}
				if(path_.front() == c) {
					path_.pop_front();
					return true;
				}
				return false;
			}
			bool match_moveto_drawto_command_groups()
			{
				if(path_.empty()) {
					return false;
				}
				match_moveto_drawto_command_group();
				match_wsp_star();
				return match_moveto_drawto_command_groups();
			}
			bool match_moveto_drawto_command_group()
			{
				if(!match_moveto()) {
					return false;
				}
				match_wsp_star();
				return match_drawto_commands();
			}
			bool match_moveto()
			{
				if(path_.empty()) {
					return false;
				}
				// ( "M" | "m" ) wsp* moveto-argument-sequence
				char c = path_.front();
				if(c == 'M' || c == 'm') {
					path_.pop_front();
					match_wsp_star_or_die();
					match_moveto_argument_sequence(c == 'M' ? true : false);
				} else {
					throw parsing_exception("Expected 'M' or 'm'");
				}
				return true;
			}
			bool match_moveto_argument_sequence(bool absolute)
			{
				double x, y;
				match_coordinate_pair(x, y);
				// emit
				cmds_.emplace_back(new MoveToCommand(absolute, x, y));
				match_comma_wsp_opt();
				return match_lineto_argument_sequence(absolute);
			}
			bool match_lineto_argument_sequence(bool absolute)
			{
				double x, y;
				if(match_coordinate_pair(x, y)) {
					// emit
					cmds_.emplace_back(new LineToCommand(absolute, x, y));
					match_comma_wsp_opt();
					match_lineto_argument_sequence(absolute);
				}
				return true;
			}
			bool match_coordinate_pair(double& x, double& y)
			{
				if(!match_coordinate(x)) {
					return false;
				}
				match_comma_wsp_opt();
				if(!match_coordinate(y)) {
					throw parsing_exception(formatter() << "Expected a second co-ordinate while parsing value: " << std::string(path_.begin(), path_.end()));
				}
				return true;
			}
			bool match_coordinate(double& v)
			{
				return match_number(v);
			}
			bool match_number(double& d)
			{
				std::string s(path_.begin(), path_.end());
				char* end;
				d = strtod(s.c_str(), &end);
				if(errno == ERANGE) {
					throw parsing_exception(formatter() << "Decode of numeric value out of range. " << s);
				}
				if(d == 0 && end == s.c_str()) {
					// No number to convert.
					return false;
				}
				auto it = path_.begin();
				std::advance(it, end - s.c_str());
				path_.erase(path_.begin(), it);
				return true;
			}
			bool match_comma_wsp_opt()
			{
				if(path_.empty()) {
					return false;
				}
				char c = path_.front();
				if(c == ',') {
					path_.pop_front();
					match_wsp_star();
				} else {
					if(!match_wsp_opt()) {
						return true;
					}
					match_wsp_star();
					c = path_.front();
					if(c != ',') {
						//throw parsing_exception("Expected COMMA");
						return true;
					}
					path_.pop_front();
					match_wsp_star();
				}
				return true;
			}
			void match_comma_wsp_or_die() {
				if(!match_comma_wsp_opt()) {
					throw parsing_exception("End of string found");
				}
			}
			bool match_drawto_commands()
			{
				if(!match_drawto_command()) {
					return false;
				}
				match_wsp_star();
				return match_drawto_commands();
			}
			bool match_drawto_command()
			{
				if(path_.empty()) {
					return false;
				}
				char c = path_.front();
				if(c == 'M' || c == 'm') {
					return false;
				}
				path_.pop_front();
				switch(c) {
					case 'Z': case 'z': 
						cmds_.emplace_back(new ClosePathCommand()); 
						break;
					case 'L':  case 'l': 
						match_wsp_star();
						match_lineto_argument_sequence(c == 'L' ? true : false);
						break;
					case 'H': case 'h':
						match_wsp_star();
						match_single_coordinate_argument_sequence(PathInstruction::LINETO_H, c == 'H' ? true : false);
						break;
					case 'V': case 'v':
						match_wsp_star();
						match_single_coordinate_argument_sequence(PathInstruction::LINETO_V, c == 'V' ? true : false);
						break;
					case 'C': case 'c': case 'S': case 's':
						match_wsp_star();
						match_curveto_argument_sequence(c=='C'||c=='S'?true:false, c=='S'||c=='s'?true:false);
						break;
					case 'Q': case 'q': case 'T': case 't':
						match_wsp_star();
						match_bezierto_argument_sequence(c=='Q'||c=='T'?true:false, c=='T'||c=='t'?true:false);
						break;
					case 'A': case 'a':
						match_arcto_argument_sequence(c == 'A' ? true : false);
						break;
					default:
						throw parsing_exception(formatter() << "Unrecognised draw-to symbol: " << c);
				}			
				return true;
			}
			bool match_single_coordinate_argument_sequence(PathInstruction ins, bool absolute)
			{
				double v;
				if(!match_coordinate(v)) {
					return false;
				}
				// emit
				if(ins == PathInstruction::LINETO_H) {
					cmds_.emplace_back(new LineToHCommand(absolute, v));
				} else if(ins == PathInstruction::LINETO_V) {
					cmds_.emplace_back(new LineToVCommand(absolute, v));
				} else {
					ASSERT_LOG(false, "Unexpected command given.");
				}
				match_wsp_star();
				return match_single_coordinate_argument_sequence(ins, absolute);
			}
			bool match_curveto_argument_sequence(bool absolute, bool smooth)
			{
				double x, y;
				double cp1x, cp1y;
				double cp2x, cp2y;
				if(!match_curveto_argument(smooth, x, y, cp1x, cp1y, cp2x, cp2y)) {
					return false;
				}
				// emit
				cmds_.emplace_back(new CubicBezierCommand(absolute, smooth, x, y, cp1x, cp1y, cp2x, cp2y));
				match_wsp_star();
				return match_curveto_argument_sequence(absolute, smooth);
			}
			bool match_curveto_argument(bool smooth, double& x, double& y, double& cp1x, double& cp1y, double& cp2x, double& cp2y) 
			{
				if(!smooth) {
					if(!match_coordinate_pair(cp1x, cp1y)) {
						return false;
					}
					if(!match_comma_wsp_opt()) {
						throw parsing_exception("End of string found");
					}
				} else {
					cp1x = cp1y = 0;
				}
				if(!match_coordinate_pair(cp2x, cp2y)) {
					if(smooth) {
						return false;
					} else {
						throw parsing_exception(formatter() << "Expected first pair of control points in curve: " << std::string(path_.begin(), path_.end()));
					}
				}
				if(!match_comma_wsp_opt()) {
					throw parsing_exception("End of string found");
				}
				if(!match_coordinate_pair(x, y)) {
					throw parsing_exception(formatter() << "Expected second pair of control points in curve: " << std::string(path_.begin(), path_.end()));
				}
				return true;
			}
			bool match_bezierto_argument_sequence(bool absolute, bool smooth)
			{
				double x, y;
				double cp1x, cp1y;
				if(!match_bezierto_argument(smooth, x, y, cp1x, cp1y)) {
					return false;
				}
				// emit
				cmds_.emplace_back(new QuadraticBezierCommand(absolute, smooth, x, y, cp1x, cp1y));
				match_wsp_star();
				return match_bezierto_argument_sequence(absolute, smooth);
			}
			bool match_bezierto_argument(bool smooth, double& x, double& y, double& cp1x, double& cp1y) 
			{
				if(smooth) {
					cp1x = cp1y = 0;
				} else {
					if(!match_coordinate_pair(cp1x, cp1y)) {
						return false;
					}
					if(!match_comma_wsp_opt()) {
						throw parsing_exception("End of string found");
					}
				}
				if(!match_coordinate_pair(x, y)) {
					if(smooth) {
						return false;
					} else {
						throw parsing_exception(formatter() << "Expected first pair of control points in curve: " << std::string(path_.begin(), path_.end()));
					}
				}
				return true;
			}
			bool match_arcto_argument_sequence(bool absolute)
			{
				double x, y;
				double rx, ry;
				double x_axis_rot;
				bool large_arc;
				bool sweep;
				if(!match_arcto_argument(x, y, rx, ry, x_axis_rot, large_arc, sweep)) {
					return false;
				}
				// emit
				cmds_.emplace_back(new EllipticalArcCommand(absolute, x, y, rx, ry, x_axis_rot, large_arc, sweep));
				match_wsp_star();
				return match_arcto_argument_sequence(absolute);
			}
			bool match_arcto_argument(double& x, double& y, double& rx, double& ry, double& x_axis_rot, bool& large_arc, bool& sweep) 
			{
				if(!match_coordinate(rx)) {
					return false;
				}
				if(rx < 0) {
					throw parsing_exception(formatter() << "While parsing elliptic arc command found negative RX value: " << rx);
				}
				match_comma_wsp_or_die();
				if(!match_coordinate(ry)) {
					throw parsing_exception("Unmatched RY value while parsing elliptic arc command");
				}
				if(ry < 0) {
					throw parsing_exception(formatter() << "While parsing elliptic arc command found negative RY value: " << y);
				}
				match_comma_wsp_or_die();
				if(!match_coordinate(x_axis_rot)) {
					throw parsing_exception("Unmatched x_axis_rotation value while parsing elliptic arc command");
				}
				match_comma_wsp_or_die();
				double large_arc_flag;
				if(!match_number(large_arc_flag)) {
					throw parsing_exception("Unmatched large_arc_flag value while parsing elliptic arc command");
				}
				large_arc = large_arc_flag > 0 ? true : false;
				match_comma_wsp_or_die();
				double sweep_flag;
				if(!match_number(sweep_flag)) {
					throw parsing_exception("Unmatched sweep_flag value while parsing elliptic arc command");
				}
				sweep = sweep_flag > 0 ? true : false;
				match_comma_wsp_or_die();
				if(!match_coordinate_pair(x, y)) {
					throw parsing_exception(formatter() << "Expected X,Y points in curve: " << std::string(path_.begin(), path_.end()));
				}
				return true;
			}
			const std::vector<PathCommandPtr>& get_command_list() const { return cmds_; }
		private:
			std::list<char> path_;
			std::vector<PathCommandPtr> cmds_;
		};

		std::vector<PathCommandPtr> parse_path(const std::string& s)
		{
			path_parser pp(s);
			return pp.get_command_list();
		}
	}
}