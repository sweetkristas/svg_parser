/*
	Copyright (C) 2003-2013 by David White <davewx7@gmail.com>
	
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "SDL.h"
#include <iostream>
#include <string>

namespace profile 
{
	struct manager
	{
		Uint64 frequency;
		Uint64 t1, t2;
		double elapsedTime;
		const char* name;

		manager(const char* const str) : name(str)
		{
			frequency = SDL_GetPerformanceFrequency();
			t1 = SDL_GetPerformanceCounter();
		}

		~manager()
		{
			t2 = SDL_GetPerformanceCounter();
			elapsedTime = (t2 - t1) * 1000.0 / frequency;
			std::cerr << name << ": " << elapsedTime << " milliseconds" << std::endl;
		}
	};
}
