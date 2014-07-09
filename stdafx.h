#pragma once

#include <SDL2\SDL.h>
// -main.cpp

#include <memory>
//#include <math.h>
#include <iostream>
#include <string>
#include <sstream>

#include <vector>
#include <map>
#include <unordered_map>

#include <math.h>
#include <functional>

using std::unique_ptr;
using std::shared_ptr;

namespace AA {
	template <typename T>
	struct serialize {
		std::string operator()(const T& t) const {
			return t.serialize();
		}
	};
}
namespace SDL {
	template <typename T> struct default_delete {
		void operator()(T* t) { std::default_delete(t); }
	};
}

/**Wraps a raw pointer into a unique pointer*/
template <typename T>
unique_ptr<T> wrap(T* t)
{
	return unique_ptr<T>{t};
}

template <typename T>
unique_ptr<T,SDL::default_delete<T>> sdlWrap(T* t)
{
	return unique_ptr<T, SDL::default_delete<T>>{t};
}