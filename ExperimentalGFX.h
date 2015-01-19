
/*------------------------------------------------------------------------------------------------------------*
HELPER CODE
*-------------------------------------------------------------------------------------------------------------*/
namespace std {
	void default_delete<SDL_Texture>::operator()(SDL_Texture* _ptr) const { SDL_DestroyTexture(_ptr); }
}

namespace modern {

	struct Context {
		SDL_Window* window;
		SDL_Renderer* renderer;
		SDL_Window* getWindow() const { return window; }
		SDL_Renderer* getRenderer() const { return renderer; }
		SDL_Renderer* operator*() const { return getRenderer(); }
		Context() = delete;
		Context(SDL_Window* w, SDL_Renderer* r) { window = w; renderer = r; }
		Context(Context&& other) : window{ std::move(other.window) }, renderer{ std::move(other.renderer) } {}
		Context(const scalar& size, Uint32 windowFlags = 0, Uint32 renderFlags = SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_ACCELERATED) {
			window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, static_cast<int>(size.x), static_cast<int>(size.y), windowFlags);
			renderer = SDL_CreateRenderer(window, -1, renderFlags);
		}
	};

	struct Area {
		SDL_Rect rect;
		bool empty = false;
		SDL_Rect* operator*() {
			if (empty)
				return NULL;
			return &rect;
		}
		Area() {
			rect = SDL_Rect{ 0, 0, 0, 0 };
			empty = true;
		}
		Area(scalar size) {
			scalar offset = size / -2;
			rect = SDL_Rect{ static_cast<int>(offset.x), static_cast<int>(offset.y), static_cast<int>(size.x), static_cast<int>(size.y) };
		}
		Area(int x, int y, int w, int h) {
			rect = SDL_Rect{ x, y, w, h };
		}
		bool Area::contains(int x, int y)
		{
			return (x > rect.x && x < rect.x + rect.w && y > rect.y && y < rect.y + rect.w);
		}
	};

	/*Wrapper around SDL_Texture, allowing for textures to be automatically cleaned up, and be bound to their window/renderer.*/
	struct Texture {
		std::unique_ptr<SDL_Texture> data;
		const Context* context;
		SDL_Texture* operator*() const { return data.get(); }
		SDL_Texture* makeTexture(const Context* context, const std::string& path) {
			auto temp = IMG_Load(path.c_str());
			auto ptr = SDL_CreateTextureFromSurface(**context, temp);
			SDL_FreeSurface(temp);
			return ptr;
		}
		Texture(){}
		Texture(const Context* ctx, const std::string& path) : data{ makeTexture(ctx, path) } {
			context = ctx;
		}
		Texture(const Context* ctx, SDL_Texture* t) : data{ t } {
			context = ctx;
		}
		Texture(Texture&& other) : data{ std::move(other.data) }, context{ std::move(other.context) } {}
		Texture(std::unique_ptr<Context>& ctx, const std::string& path) :
			data{ makeTexture(ctx.get(), path) }
		{
			context = ctx.get();
		}
		Area getRect(int x = 0, int y = 0) {
			Area area = Area{ x, y, 16, 16 };
			SDL_QueryTexture(data.get(), NULL, NULL, &area.rect.w, &area.rect.h);
			return area;
		}
		int draw(Area& dst = Area(), Area& src = Area()) {
			return SDL_RenderCopy(**context, data.get(), *src, *dst);
		}
	};

	struct Transform {
		scalar position;
		scalar scale;
		double rotation;
		Transform() {
			position = scalar{};
			scale = scalar{};
			rotation = 0;
		}
		Transform(scalar _position, scalar _scale = scalar{ 0 }, double _rotation = 0) {
			position = _position;
			scale = _scale;
			rotation = _rotation;
		}
	};

	struct Config {
		using ptree = boost::property_tree::ptree;
		ptree data;
		ptree* operator*(){ return &data; }
		Config(const std::string& path) { boost::property_tree::read_info(path, data); }
		Config(const ptree& input) { data = input; }
	};

	struct StaticImage {
		/*texture points to a Texture. One Texture per image. Texture becomes invalid at the end of its scope(unique_ptr)
		Can change the pointer, but not the texture.*/
		const Texture*	texture;
		Area		area;
		Transform	transform;

		const Context* context() {
			return texture->context;
		}
		StaticImage() {
			texture = NULL;
			area = Area();
			transform = Transform();
		}
		StaticImage(Texture* image, Area a, Transform&& t = Transform()) {
			texture = image;
			area = a;
			transform = t;
		}
		Area applyTransform() {
			Area result = area;
			result.rect.x += static_cast<int>(transform.position.x);
			result.rect.y += static_cast<int>(transform.position.y);
			result.rect.w += static_cast<int>(result.rect.w * transform.scale.x);
			result.rect.h += static_cast<int>(result.rect.h * transform.scale.y);
			result.rect.x -= static_cast<int>((result.rect.w - area.rect.w) / 2);
			result.rect.y -= static_cast<int>((result.rect.h - area.rect.h) / 2);
			return result;
		}
		int draw() {
			return SDL_RenderCopy(texture->context->getRenderer(), texture->data.get(), NULL, *applyTransform());
		}
	};

	struct Text : public Texture {
		std::string content;
		SDL_Color color;
		int x;
		int y;
		int w;
		int h;
		Text(Context* context, std::string text, int px, int py, TTF_Font* font, SDL_Color c = SDL_Color{ 255, 255, 255, 255 }) : Texture(context, makeText(context, text, font, c))
		{
			x = px;
			y = py;
			content = text;
			color = c;
		}
		SDL_Texture* makeText(Context* context, std::string text, TTF_Font* font, SDL_Color& c)
		{
			SDL_Surface* temp = TTF_RenderText_Blended_Wrapped(font, text.c_str(), c, 512);
			if (temp){
				w = temp->w;
				h = temp->h;
				SDL_Texture* tex = SDL_CreateTextureFromSurface(context->getRenderer(), temp);
				SDL_FreeSurface(temp);
				return tex;
			}
			w = 0;
			h = 0;
			return nullptr;
		}
		int draw() {
			return Texture::draw(getRect(x, y));
		}
	};

	struct Button {
		Context* ctx;
		TTF_Font* font;
		SDL_Colour backgroundColour;
		Area area;
		bool hovering;
		boost::optional<StaticImage> backgroundImage;
		std::vector<Text*> texts;
		Button(Context* context, Area a, SDL_Color bg = SDL_Color{ 0, 0, 0, 0 }, TTF_Font* f = NULL){ ctx = context; area = a; font = f; backgroundColour = bg; }
		void addText(int x, int y, std::string content, SDL_Color color = SDL_Color{ 255, 255, 255, 255 }) {
			if (font != NULL)
				texts.push_back(new Text(ctx, content, x, y, font, color));
		}
		void update(int mx, int my) {
			hovering = area.contains(mx, my);
		}
		int draw() {
			SDL_SetRenderDrawColor(ctx->renderer, backgroundColour.r, backgroundColour.g, backgroundColour.b, backgroundColour.a);
			SDL_RenderFillRect(ctx->renderer, *area);
			if (backgroundImage.is_initialized()) {
				backgroundImage->draw();
			}
			if (hovering) {
				SDL_SetRenderDrawColor(ctx->renderer, 60, 60, 60, 60);
				SDL_RenderFillRect(ctx->renderer, *area);
			}
			for (auto& text : texts)
			{
				text->draw();
			}
			return 1;
		}
		~Button() {
			for (auto ptr : texts)
			{
				delete ptr;
			}
		}
	};

	template <int Tilesize>
	struct Tile {
		StaticImage image;
		bool empty = true;
		Tile(int x = 0, int y = 0, Texture* t = NULL)
		{
			if (t != NULL)
			{
				empty = false;
				image = StaticImage(t, Area(x*Tilesize, y*Tilesize, Tilesize, Tilesize), Transform());
			}
		}
		void draw()
		{
			if (!empty)
				image.draw();
		}
	};
}
namespace std {
	/*The cleanup for @SDL_Texture is defined here for unique_ptrs. */
	//	void default_delete<SDL_Texture>::operator()(SDL_Texture* _ptr) const { SDL_DestroyTexture(_ptr); }
	/*The cleanup for @Context is defined here for unique_ptrs. */
	inline void default_delete<modern::Context>::operator()(modern::Context* _ptr) const {
		SDL_DestroyRenderer(_ptr->renderer);
		SDL_DestroyWindow(_ptr->window);
	}
}

namespace get {
	using boost::property_tree::ptree;
	UI::art::frame		frame(const UI::art::context& ctx)	{ return ctx.first; }
	UI::art::frame		window(const UI::art::context& ctx)	{ return ctx.first; } // alias
	UI::art::artist		artist(const UI::art::context& ctx)	{ return ctx.second; }
	UI::art::artist		renderer(const UI::art::context& ctx)	{ return ctx.second; } // alias
	scalar				size(UI::art::area area)			{ return scalar(area.w, area.h); }
	scalar				position(UI::art::area area)			{ return scalar(area.x, area.y); }
}
namespace make {
	inline UI::art::canvas canvas(UI::art::artist artist, const scalar& size) {
		return SDL_CreateTexture(artist, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, static_cast<int>(size.x), static_cast<int>(size.y));
	}
	inline UI::art::area area(const scalar& size, const scalar& position = scalar(0, 0)) {
		return UI::art::area{ static_cast<int>(position.x), static_cast<int>(position.y), static_cast<int>(size.x), static_cast<int>(size.y) };
	}
	inline void target(UI::art::artist a, UI::art::canvas c) {
		SDL_SetRenderTarget(a, c);
	}
}
namespace render {
	inline int image(UI::Image* image, UI::art::area* frame = NULL) {
		return SDL_RenderCopy(get::artist(image->context), image->canvas, frame, &image->area);
	}
	inline int circle(const UI::art::context& context, scalar const&position, double const&radius) {

		std::vector<SDL_Point> points;

		for (int y = static_cast<int>(position.y - radius); y <= static_cast<int>(position.y + radius); y++)
		for (int x = static_cast<int>(position.x - radius); x <= static_cast<int>(position.x + radius); x++)
			points.push_back(SDL_Point{ x, y });

		return SDL_RenderDrawPoints(get::renderer(context), points.data(), points.size());
	}
}

UI::Image::Image(UI::art::context p_context, int const& w, int const& h)
: context{ p_context }, area(make::area(scalar(w, h))), canvas{ make::canvas(get::artist(p_context), get::size(area)) }
{
}

UI::Image::Image(UI::art::context p_context, const UI::info& a)
{
	context.first = p_context.first;
	context.second = p_context.second;
	int x, y, w, h;
	int screenw, screenh;
	SDL_GetWindowSize(p_context.first, &screenw, &screenh);

	auto offset = [](int& val, int& relto, UI::info& item) {
		auto _data = item.data();
		bool relative = _data[_data.size() - 1] == '%';
		double value = relative ? std::stod(_data.substr(0, _data.size() - 1)) : std::stod(_data);
		if (relative)
			value = value* relative / 100.0;
		val += value;
	};

	for (auto i : a)
	{
		if (i.first == "x")
			offset(x, screenw, i.second);
		if (i.first == "y")
			offset(y, screenh, i.second);
		if (i.first == "w")
			offset(w, screenw, i.second);
		if (i.first == "h")
			offset(h, screenh, i.second);
	}

	area = UI::art::area{ x, y, w, h };
	canvas = make::canvas(get::artist(p_context), get::size(area));
}

SDL_Point UI::Image::center()
{
	return SDL_Point{ area.w / 2, area.h / 2 };
}

UI::Image::Image() : context{ NULL, NULL }, canvas{ NULL }
{
	area = { 0, 0, 0, 0 };
}

/*------------------------------------------------------------------------------------------------------------*
*-------------------------------------------------------------------------------------------------------------*/