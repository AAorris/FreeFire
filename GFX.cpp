#include "GFX.h"

GFX::GFX() {
}

void GFX::init()
{
	init("Random window", 100, 100, false);
}

/*void GFX::init(std::string _name, int _w, int _h, bool borderless)
{
	Uint32 flags = 0;
	w = _w;
	h = _h;
	if(borderless) flags |= SDL_WINDOW_BORDERLESS;
	win = SDL_CreateWindow(_name.c_str(),SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,_w,_h,flags|SDL_WINDOW_HIDDEN);
	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_TARGETTEXTURE);
	std::string error = SDL_GetError();
	loadFont("default", "font/Roboto-Regular.ttf", 24);
	loadFont("robotoblack12", "font/Roboto-Black.ttf", 12);
	font = fonts.at("default");
	SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
	SDL_ShowWindow(win);
}*/

void GFX::init(std::string _name, int _w, int _h, Uint32 flags)
{
	w = _w;
	h = _h;
	win = SDL_CreateWindow(_name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _w, _h, flags | SDL_WINDOW_HIDDEN);
	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	std::string error = SDL_GetError();
	loadFont("default", "font/Roboto-Regular.ttf", 24);
	loadFont("robotoblack12", "font/Roboto-Black.ttf", 12);
	font = fonts.at("default");
	SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
	SDL_ShowWindow(win);
}

bool GFX::drawTile(pos& p, const std::string& key)
{
	int s = 32;
	SDL_Rect rect = { p.x*s, p.y*s, s, s };

	if (key != "")
	{
		std::unordered_map<std::string, SDL_Texture*>::iterator it = textures.find(key);
		if (it!=textures.end())
		{
			SDL_Texture* tex = it->second;
			SDL_RenderCopy(ren, tex, NULL, &rect);
			return true;
		}
		else
			return false;
	}
	else
	{
		SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
		SDL_RenderDrawRect(ren, &rect);
		return false;
	}
}

void GFX::cleanup()
{
	SDL_DestroyWindow(win);
	SDL_DestroyRenderer(ren);
}

GFX::~GFX() {
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
}

void GFX::loadFont(std::string key, std::string path, int ptSize)
{
	TTF_Font* f = TTF_OpenFont(path.c_str(),ptSize);
	fonts.insert(std::make_pair(key,f));
}

void GFX::fontWrite(std::string font, std::string text, int x, int y)
{
	SDL_Color alpha = {255,255,255,0};
	SDL_Color black = BLACK;
	SDL_Surface *temps = TTF_RenderText_Blended(fonts[font],text.c_str(),black);
	drawImage(SDL_CreateTextureFromSurface(ren,temps),x,y);
}

void GFX::loadAsset(std::string key, std::string path)
{
	SDL_Texture* tex = SDL_CreateTextureFromSurface(	
							ren,
							IMG_Load(path.c_str())
						);
	//<std::string, SDL_Texture*>
	textures.insert(std::make_pair(key,tex));
}

bool GFX::loadAsset(std::string path)
{
	if (path == "")
		return false;
	SDL_Texture* tex = SDL_CreateTextureFromSurface(
		ren,
		IMG_Load(path.c_str())
		);
	if (tex == NULL)
		return false;
	//<std::string, SDL_Texture*>
	textures.insert(std::make_pair(path, tex));
	return true;
}

void GFX::fill(std::string assetName)
{
	SDL_RenderCopy(ren,textures[assetName],NULL,NULL);
}

void GFX::fill(SDL_Texture* texture)
{
	SDL_RenderCopy(ren,texture,NULL,NULL);
}

void GFX::fill(int r, int  g, int b, int a)
{
	SDL_SetRenderDrawColor(ren, r, g, b, a);
	SDL_RenderFillRect(ren, NULL);
}

SDL_Texture* GFX::getText(std::string content, std::string f="")
{
	SDL_Color color = WHITE;//{ 0, 0, 0, 255 };
	SDL_Surface* temp;
	SDL_Texture* tex;
	temp = TTF_RenderText_Blended(fonts[f], content.c_str(), color);
	tex = SDL_CreateTextureFromSurface(ren, temp);
	SDL_FreeSurface(temp);
	return tex;
}

void GFX::write(std::string text, int x, int y)
{
	SDL_Color black = BLACK;
	SDL_Color white = WHITE;
	SDL_Surface* temps = TTF_RenderText_Shaded(font, text.c_str(), black, white);
	drawImage(SDL_CreateTextureFromSurface(ren, temps), x, y);
}

void GFX::drawImage(SDL_Texture* t, int x, int y)
{
	SDL_Rect rect = { x, y, 0, 0 };
	SDL_QueryTexture(t, 0, 0, &rect.w, &rect.h); // get w and h
	SDL_RenderCopy(ren, t, NULL, &rect);
}

void GFX::show()
{
	SDL_RenderPresent(ren);
}

void GFX::drawSprite(sprite& s, int x, int y)
{
	//c = cell
	//s = sprite
	//i = image
	SDL_Rect		r_canvas	= { x, y, s.cw, s.ch };
					s.frame		= s.cur->frames[ s.subframe ];
	int				sheetX		= ( s.frame * s.cw ) % s.iw;
	int				sheetY		= ( (s.frame * s.cw) / s.iw ) * s.cw;
	SDL_Rect		r_sprite	= { sheetX, sheetY, s.cw, s.ch };
	SDL_RenderCopy(ren, textures[s.sheet], &r_sprite, &r_canvas);
}

//assumes it has a valid sheet path inside it
/**Loads a sprite using graphics functions. Adds the loaded texture
to the graphics storage and initializes the cell width and height of the image.*/
void GFX::loadSprite(sprite&s)
{
	SDL_Surface* temp = IMG_Load(s.sheet.c_str());
	SDL_Texture* tex = SDL_CreateTextureFromSurface(ren,temp);
	textures.insert(std::make_pair(s.sheet,tex)); //<std::string,SDL_Texture*>
	SDL_QueryTexture(tex,NULL,NULL,&(s.iw),&(s.ih));
}

void GFX::drawButton(SDLButton& b)
{
	//drawing background
	SDL_Rect bg = { b.x, b.y, b.w, b.h };
	SDL_Color bgCol = { 255, 0, 0, 255 };
	if (b.clicking)
		SDL_SetRenderDrawColor(ren, b.clickCol.r, b.clickCol.g, b.clickCol.b, b.clickCol.a);
	else if (b.hovering)
		SDL_SetRenderDrawColor(ren, b.hoverCol.r, b.hoverCol.g, b.hoverCol.b, b.hoverCol.a);
	else
		SDL_SetRenderDrawColor(ren, b.idleCol.r, b.idleCol.g, b.idleCol.b, b.idleCol.a);
	SDL_RenderFillRect(ren, &bg);

	//drawing label (?)
	if (b.textLabel != "")
	{
		if (b.t_label != NULL)
			SDL_RenderCopy(ren, b.t_label, NULL, &b.r_label);
		else
		{
			SDL_Texture* tex = getText(b.textLabel, "robotoblack12");
			b.t_label = tex;
			int tw = 0;
			int th = 0;
			SDL_QueryTexture(tex, NULL, NULL, &tw, &th);
			//line up the two centers
			// c = center
			// b = background
			// t = text
			int cbx = b.x + b.w / 2;
			int cby = b.y + b.h / 2;
			int ctx = b.x + tw / 2;
			int cty = b.y + th / 2;
			//offset from normal position is the difference
			//between their centers.
			int r_x = b.x + (cbx - ctx);
			int r_y = b.y + (cby - cty);

			b.r_label = { r_x, r_y, tw, th };
			SDL_RenderCopy(ren, b.t_label, NULL, &b.r_label);
		}
	}
}