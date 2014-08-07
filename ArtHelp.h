#include <SDL2\SDL.h>
#include <SDL2\SDL_image.h>
#include <iostream>
#include <memory>

#define gfx sdl

namespace sdl {
	using artist = SDL_Renderer;
	using frame = SDL_Window;
	using surface = SDL_Surface;
	using canvas = SDL_Texture;
	using area = SDL_Rect;
	using context = std::pair<frame, artist>;
}

namespace mem {

	template <typename T>
	struct free {};
	template <typename T>
	struct make {};

	template <typename T, typename D>
	using wrapped = std::unique_ptr<T, D>;

	template<> struct make<sdl::surface*> {
		sdl::surface* operator()(const std::string& s) const {
			return IMG_Load(s.c_str());
		}
	};

	template<> struct free<sdl::surface> {
		void operator()(sdl::surface* surface) const {
			SDL_FreeSurface(surface);
		}
	};
	template<> struct free<sdl::canvas> {
		void operator()(sdl::canvas* ptr) const {
			SDL_DestroyTexture(ptr);
		}
	};

	template <typename T>
	struct wrap {
		wrapped<T, free<T>> operator()(T* t) const {
			return wrapped<T, free<T>>(t, free<T>());
		}
	};
}