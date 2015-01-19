#pragma once
#include <SDL2\SDL.h>
#include <unordered_map>
#include <iostream>
#include <string>
#include "scalar.h"
#define signs :

namespace Zen {
	struct AreaContract {
		virtual SDL_Rect* getArea() = 0;
		SDL_Rect output;

		virtual bool contains(const int& px, const int& py) {
			SDL_Rect* rect = getArea();
			return rect->x < px && px < rect->x + rect->w
				&& rect->y < py && py < rect->y + rect->h;
		}
		template<typename _Composite>
		bool contains(const _Composite& pos) const { return contains(pos.x, pos.y); }

		virtual bool consume(const int& px, const int& py) {
			return contains(px, py);
		}

	}; using AreaSigned = AreaContract; /*Used for static if checks*/

	struct RenderContext {

	};

	struct RenderPackage {
		//Take a number from the render context;
		int number;
		SDL_Color* fg = nullptr;
		SDL_Color* bg = nullptr;
	};

	enum axis {
		X, Y, Z, W,
		A, B, C, D,
		E, F, G, H,
		I, J, K, L
	};

	template<typename T>
	struct Area signs AreaContract
	{
		using type = T;
		template <typename T> static type cast(T t) { return static_cast<type>(t); }
		type x, y, w, h;

		Area() : x{ 0 }, y{ 0 }, w{ 0 }, h{ 0 } {}

		Area(double p_x, double p_y, double p_w, double p_h) :
			x{ cast<type>(p_x) },
			y{ cast<type>(p_y) },
			w{ cast<type>(p_w) },
			h{ cast<type>(p_h) } {}

		template<typename base>
		Area(const Area<base>& a) : Area(a.x, a.y, a.w, a.h) {}

		type left() const { return x; }
		type top() const { return y; }
		type right() const { return x + w; }
		type bottom() const { return y + h; }
		type center(axis axis = X) const { return (axis == X) ? cast(x + w / 2) : (axis == Y) ? cast(y + h / 2) : 0; }

		template<typename T>
		Area<type> operator*(const T& val) const {
			return Area<type>{x*val, y*val, w*val, h*val};
		}

		template<typename T>
		Area<type> operator+(const Area<T>& val) const {
			return Area<type>(x + val.x, y + val.y, w + val.w, h + val.h);
		}
		SDL_Rect* getArea() {
			//output is stored internally from areacontract
			output = { x, y, w, h };
			return &output;
		}
	}; using Areaf = Area<double>;

	SDL_Rect transform(SDL_Rect* rect, scalar& offset, scalar& scale) {
		SDL_Rect out;
		out.x = rect->x += offset.x;
		out.y = rect->y + offset.y;
		out.w = rect->w * scale.x;
		out.h = rect->h * scale.y;
		return out;
	}

	//template<typename _Parent>
	struct RelativeArea signs AreaContract {
		//using parent_type = _Parent;
		using parent_ptr = SDL_Rect*;
		parent_ptr relative;
		scalar offset;
		scalar scale = scalar(1, 1);
		RelativeArea() : relative{ nullptr } {}
		RelativeArea(parent_ptr to) : relative{ to } {}
		SDL_Rect* getArea() {
			//output is stored internally when you sign an area contract
			output = transform(relative, offset, scale);
			return &output;
		}
	};

	//template<typename T> void draw(T& t, SDL_Renderer* r) { t.draw(r); }
	SDL_Renderer* current_renderer;
	void setRenderer(SDL_Renderer* r) { current_renderer = r; }
	void setColor(SDL_Color c) { SDL_SetRenderDrawColor(current_renderer, c.r, c.g, c.b, c.a); }
	template<typename T>
	void drawArea(Area<T>& area) { SDL_RenderFillRect(current_renderer, area.getArea()); }
	void drawArea(SDL_Rect* rect) { SDL_RenderFillRect(current_renderer, rect); }

	template<typename T>
	struct interpolate {
		T* from;
		double speed = 0.05;
		interpolate(T* it, const double& change) : from{ it }, speed{ change } {}
		void operator()(const T& to) {
			*from = *from*(1 - speed) + to*(speed);
		}
		void operator=(interpolate<T>& other) {
			from = other.from;
			speed = other.speed;
		}
	};

	template<typename T>
	struct basic_consumer {
		T* const plate;
		basic_consumer(T* ptr) : plate{ ptr } {}
		bool operator()(const int& x, const int& y) const {
			return plate->contains(x, y);
		}
	};

	struct DynamicArea signs AreaContract {
		//Typedefs
		using container = std::unordered_map<std::string, Area<double>>;
		//Members
		container states;
		Area<double> current;
		std::string goal;
		interpolate<Area<double>> lerp;
		SDL_Color backgroundColor = SDL_Color{ 255, 255, 255, 255 };
		//Constructors
		DynamicArea() : lerp{ &current, 0.05 } {
			addStateArea("default", { 1 << 4, 1 << 4, 1 << 6, 1 << 6 });
			goal = "default";
			fixState("default");
		}
		DynamicArea(Area<double> default) : DynamicArea() { setState("default", default); fixState("default"); }
		void operator=(DynamicArea& other) {
			states = other.states;
			current = other.current;
			goal = other.goal;
			lerp = other.lerp;
			backgroundColor = other.backgroundColor;
		}
		//Functions
		void				addStateArea(std::string key, Area<double> area) {
			states.insert(std::make_pair(key, area));
		}
		void				addState(std::string key, std::vector<double> list) {
			states.insert(std::make_pair(key, Area<double>(list[0], list[1], list[2], list[3])));
		}
		void				setState(std::string key, Area<double> area) { states[key] = area; }
		Area<double>&		iteratorValue(std::string key) { return getState(key)->second; }
		void				fixState(std::string key)	{ goal = (key);  current = iteratorValue(goal); }
		void				update()					{
			current = current * (1 - 0.05) + getState(goal)->second * (0.05);
		}
		void				setTarget(std::string key)	{ goal = key; }
		bool				contains(int x, int y)		{ return current.contains(x, y); }
		virtual SDL_Rect*	getArea()					{ return current.getArea(); }
		container::iterator getState(std::string key)	{ return states.find(key); }
		void				setGoal(std::string key)	{ getState(key); }
		bool				consume(int x, int y)		{
			if (!contains(x, y)) return false;
			if (goal == "default") setTarget("small");
			else setTarget("default");
			return true;
		}
		void				draw(SDL_Renderer* r)	{
			Zen::setRenderer(r);
			Zen::setColor(backgroundColor);
			Zen::drawArea(current);
		}
	};

	struct AccordianArea : public DynamicArea {
		RelativeArea clickable;
		AccordianArea() : DynamicArea(), clickable{ getArea() } {
			clickable.relative = &current.output;
		}
		void operator=(AccordianArea& other) {
			DynamicArea::operator=(other);
			clickable = other.clickable;
			clickable.relative = &current.output;
		}
		AccordianArea(Area<double> default, Area<double> small) : DynamicArea(default), clickable{ getArea() } {
			addStateArea("small", small);
			setClickableArea(0, 0, 0.1, 1);
		}
		void setClickableArea(int fromX, int fromY, double scaleX, double scaleY) {
			clickable.offset.x = fromX;
			clickable.offset.y = fromY;
			clickable.scale.x = scaleX;
			clickable.scale.y = scaleY;
			clickable.relative = &current.output;
		}
		void draw(SDL_Renderer* r) {
			DynamicArea::draw(r);
			//draw clickable area
			Zen::setColor(SDL_Color{ 0, 0, 0, 128 });
			Zen::drawArea(clickable.getArea());
		}
		void update() { DynamicArea::update(); }
		bool contains(int x, int y) { return DynamicArea::contains(x, y); }
		bool consume(int x, int y) {
			if (!contains(x, y)) return false;
			if (clickable.consume(x, y))
			{
				if (goal == "default") setTarget("small");
				else setTarget("default");
				return true;
			}
			return false;
		}
		virtual SDL_Rect* getArea() {
			return DynamicArea::getArea();
		}
	};
}