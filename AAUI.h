#pragma once
//TODO : don't inline class and use forward declarations
#include <boost\property_tree\ptree.hpp>
#include <queue>
#include <SDL2\SDL.h>
#include <GL\Matrices.h>
#include <GL\Vectors.h>
/*
Aaron Morris
Draft Started - Sep 20 2014
Updated - Sep 20 2014
*/
class AAUI
{
	/*--------------------------------------------------------------------------protected
	*/
protected:
	bool _alive;
public:
	/*--------------------------------------------------------------------------static
	*/
	using tree = boost::property_tree::ptree;
	using queue = std::queue<std::string>;
	/*DATA is the public data pool for user interfaces to read from.
	To allow a UI to access information, add it to DATA using boost::property_tree::ptree*/
	static tree DATA;
	/*INSTRUCTIONS are the public instructions for the rest of the application.
	If your UI needs to send out data, create a string message that your application can
	interpret, and add it to INSTRUCTIONS with push().*/
	static queue INSTRUCTIONS;
	//mouse
	static SDL_Window* MOUSE_FOCUS;
	static unsigned int MOUSE_BUTTONS;
	static int MOUSE_X;
	static int MOUSE_Y;
	static void Init() {
		DATA = tree();
		INSTRUCTIONS = queue();
		UpdateMouse();
	}
	static void UpdateMouse() {
		MOUSE_BUTTONS = SDL_GetMouseState(&MOUSE_X, &MOUSE_Y);
		MOUSE_FOCUS = SDL_GetMouseFocus();
	}
	/*--------------------------------------------------------------------------relative
	*/
	sdl::Window* window;
	AAUI()
	{
		_alive = true;
	}
	virtual void init() = 0;
	virtual void update() = 0;
	virtual void draw() = 0;
	virtual bool alive() {
		return _alive;
	}
	virtual void color(const char& R, const char& G, const char& B, const char& A=0xFF) {
		SDL_SetRenderDrawColor(window->renderer(), R, G, B, A);
	}
	virtual void rect(const sdl::Box& b) {
		SDL_Rect r = b.rect();
		SDL_RenderFillRect(window->renderer(), &r);
	}
	virtual void rect(const float& x, const float& y, const float& w, const float& h) {
		SDL_Rect r = SDL_Rect{ x, y, w, h };
		SDL_RenderFillRect(window->renderer(), &r);
	}
	~AAUI()
	{
	}
};

namespace sdl {
		//box transform
		class Box {
		public:
			Vector4 content;
			Box() : content{ 0, 0, 0, 0 } {}
			Box(Vector4 v) : content{ v[0], v[1],v[2],v[3] } {}
			Box(std::vector<float> v) : content{ v[0], v[1], v[2], v[3] } {}
			Box(const float& x, const float& y, const float& w, const float& h) : content{ x, y, w, h } {}
			Box operator+(const Box& other) {
				return Box{ content.x+other.content.x, content.y+other.content.y, content[2]+other.content[2], content[3]+other.content[3]};
			}
			Box operator*(const Matrix4& transform) {
				//x, y, z, (0==no translation, 1==translation)
				Vector4 scale = transform * Vector4(content[2],content[3],0,0);
				Vector4 position = transform * Vector4(content.x, content.y, 0, 1);
				return Box{ position[0], position[1], scale[0], scale[1] };
			}
			SDL_Rect rect() const {
				return SDL_Rect{ content.x, content.y, content.z, content.w };
			}
		};
		class Window {
		public:
			enum WINDOW_TYPES {
				APP_WINDOW = SDL_WINDOW_RESIZABLE,
				UI_WINDOW = SDL_WINDOW_BORDERLESS
			};
			SDL_Window* _win;
			SDL_Renderer* _ren;
			Window* parent;
			Box box;
			Matrix4 transform;
			Uint32 windowFlags;
			SDL_Window* window() { return _win; }
			SDL_Renderer* renderer() { return _ren; }
			/*Use a unique_ptr for this class, as it does it's init and quit in constructor and destructor.
			You can grab flags from Window::WINDOW_TYPES*/
			Window(Box p_box, Uint32 flags = APP_WINDOW) {
				parent = nullptr;
				box = p_box;
				transform = Matrix4();
			}
			/*You can grab flags from Window::WINDOW_TYPES*/
			Window(Window* p_parent, Box p_box, Uint32 flags) : Window(p_box, flags) {
				parent = p_parent;
			}
			~Window() {
				if (parent != nullptr){
					SDL_DestroyRenderer(_ren);
					SDL_DestroyWindow(_win);
					parent = nullptr;
				}
				else {
					_win = nullptr;
					_ren = nullptr;
					parent = nullptr;
				}
			}
			Box transformedBox() {
				return box*transform;
			}
			void update() {
			}
			void move(const float& set_x, const float& set_y) {
				transform.setColumn(3, Vector3(set_x, set_y, 0));
			}
			void move_r(const float& dx, const float& dy) {
				transform.translate(Vector3(dx, dy, 0));
			}
			void scale(const float& set_x, const float& set_y) {
				transform.setColumn(0, Vector3(set_x,	0,		0));
				transform.setColumn(1, Vector3(0,		set_y,	0));
				transform.setColumn(3, Vector3(0,		0,		1));
			}
			void scale_r(const float& dx, const float& dy) {
				transform.scale(dx, dy, 1);
			}
		};
	}
}