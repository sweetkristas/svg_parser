#pragma once

#include <string>
#include <vector>

struct point 
{
	explicit point(const std::string& str);
	explicit point(int x=0, int y=0);
	explicit point(float x, float y);
	explicit point(const std::vector<int>& v);

	std::string to_string() const;

	union 
	{
		struct { float x, y; };
		float buf[2];
	};
};

class rectf
{
public:
	static rectf from_coordinates(float x1, float y1, float x2, float y2);
	static rectf from_area(float x, float y, float w, float h);
	explicit rectf(const std::string& str);
	explicit rectf(int x=0, int y=0, int w=0, int h=0);
	explicit rectf(float x, float y, float w, float h);
	explicit rectf(const std::vector<float>& v);
	int x() const { return int(top_left_.x); }
	int y() const { return int(top_left_.y); }
	int x2() const { return int(bottom_right_.x); }
	int y2() const { return int(bottom_right_.y); }
	int w() const { return int(bottom_right_.x - top_left_.x); }
	int h() const { return int(bottom_right_.y - top_left_.y); }

	float xf() const { return top_left_.x; }
	float yf() const { return top_left_.y; }
	float x2f() const { return bottom_right_.x; }
	float y2f() const { return bottom_right_.y; }
	float wf() const { return bottom_right_.x - top_left_.x; }
	float hf() const { return bottom_right_.y - top_left_.y; }

	int mid_x() const { return (x() + x2())/2; }
	int mid_y() const { return (y() + y2())/2; }

	point& top_left() { return top_left_; }
	point& bottom_right() { return bottom_right_; }
	const point& top_left() const { return top_left_; }
	const point& bottom_right() const { return bottom_right_; }

	rectf& operator+= (const point& p);
	std::string to_string() const;
	//SDL_Rect sdl_rect() const { SDL_Rect r = {x(), y(), w(), h()}; return r; }

	bool empty() const { return w() == 0 || h() == 0; }
private:
	point top_left_, bottom_right_;
};
