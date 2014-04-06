#include <cairo/cairo.h>

#include "asserts.hpp"
#include "filesystem.hpp"
#include "profile_timer.hpp"
#include "SVGParse.hpp"
#include "SDLWrapper.hpp"

namespace 
{
	const int width = 512;
	const int height = 512;
}

int main(int argc, char* argv[])
{
	if(argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
		return 1;
	}
	KRE::SVG::Parse p(argv[1]);
	//svg::parse p("c:\\projects\\svg_parser\\icons\\spider-face.svg");
	//svg::parse p("c:\\projects\\svg_parser\\icons\\logo-faith.svg");
	//svg::parse p("c:\\projects\\svg_parser\\icons\\zigzag-tune.svg");
	//svg::parse p("c:\\projects\\svg_parser\\icons\\test-arc.svg");
	//svg::parse p("c:\\projects\\svg_parser\\icons\\soccer-ball.svg");
	/*const std::string root_path("c:\\projects\\svg_parser\\icons\\");
	std::vector<std::string> files;
	sys::get_files_in_dir(root_path, &files);
	for(auto& f : files) {
		std::string filename = root_path + f;
		//std::cerr << "Parsing: " << filename << std::endl;
		svg::parse p(filename);
	}*/

	SDL::SDLPtr manager(new SDL::SDL());
	
	cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	cairo_t* cairo = cairo_create(surface);
	{
		profile::manager pman("cairo_render");
		p.CairoRender(cairo);
	}

	cairo_surface_write_to_png(surface, "c:\\projects\\svg_parser\\test.png");

	typedef std::shared_ptr<SDL_Window> SDL_WindowPtr;
	SDL_WindowPtr window_;
	SDL_GLContext context_ = NULL;
	SDL_Renderer* renderer_ = NULL;
	std::string title("SVG PARSER");

	int x = SDL_WINDOWPOS_CENTERED;
	int y = SDL_WINDOWPOS_CENTERED;
	int w = width;
	int h = height;
	Uint32 wnd_flags = 0/*= SDL_WINDOW_OPENGL*/;

	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

	if(SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "opengl", SDL_HINT_OVERRIDE) != SDL_TRUE) {
		ASSERT_LOG(false, "Couldn't set render driver to opengl");
	}
	if(SDL_SetHintWithPriority(SDL_HINT_RENDER_OPENGL_SHADERS, "1", SDL_HINT_OVERRIDE) != SDL_TRUE) {
		ASSERT_LOG(false, "Couldn't set opengl shaders");
	}
	if(SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "1") != SDL_TRUE) {
		ASSERT_LOG(false, "Couldn't set framebuffer acceleration");
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	{
		window_.reset(SDL_CreateWindow(title.c_str(), x, y, w, h, wnd_flags), [&](SDL_Window* wnd){
			SDL_DestroyRenderer(renderer_);
			SDL_DestroyWindow(wnd);
		});
		ASSERT_LOG(window_ != NULL, "Failed to create window: " << SDL_GetError());

		Uint32 rnd_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
		renderer_ = SDL_CreateRenderer(window_.get(), -1, rnd_flags);
		ASSERT_LOG(renderer_ != NULL, "Failed to create renderer: " << SDL_GetError());				

		SDL_RenderSetLogicalSize(renderer_, width, height);

		auto tex_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
		SDL_SetRenderDrawColor(renderer_, 255, 255, 0, 128);
		if(SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND) != 0) {
			ASSERT_LOG(false, "Unable to set blend mode: " << SDL_GetError());
		}
		SDL_UpdateTexture(tex_, NULL, cairo_image_surface_get_data(surface), cairo_image_surface_get_stride(surface));
		SDL_SetTextureBlendMode(tex_, SDL_BLENDMODE_BLEND);

		SDL_Event e;
		bool done = false;
		while(!done) {
			while(SDL_PollEvent(&e)) {
				if(e.type == SDL_KEYUP && e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					done = true;
				} else if(e.type == SDL_QUIT) {
					done = true;
				}
			}

			SDL_RenderClear(renderer_);
			SDL_RenderCopy(renderer_, tex_, NULL, NULL);
			SDL_RenderPresent(renderer_);
		
			SDL_Delay(500);
		}

		cairo_destroy(cairo);
		cairo_surface_destroy(surface);

		SDL_DestroyTexture(tex_);
	}
	SDL_Quit();
	return 0;
}
