#pragma once
/*
#include <iostream>
#include <string>
#include <memory>

#include <map>
#include <unordered_map>
#include <vector>

#include <SDL2\SDL.h>
#include <SDL2\SDL_net.h>
#include <SDL2\SDL_ttf.h>
//template <typename TNet, typename TGfx, typename TCfg, typename TSim, typename TUi>

using namespace std;

class Client
{
private:
	class NET; unique_ptr<NET> net;
	class GFX; unique_ptr<NET> net;
	class NET; unique_ptr<NET> net;
	class NET; unique_ptr<NET> net;
	class NET; unique_ptr<NET> net;
	NETBase* net;
	GFXBase* gfx;
	CFGBase* config;
	SIMBase* sim;
	UIBase * ui;
public:
	Client();

	void init();
	//void loadConfig(std::string file);
	void startNet();
	void drawSim();
	void drawUI();
	void update();
};

class GFXBase : Base
{
private:
	SDL_Window* _main_window;
	SDL_Renderer* _main_renderer;
	TTF_Font* _main_font;

	std::unordered_map<string, SDL_Texture*> _assets;
	
protected:
public:
};

	class Asset
	{
	private:
	protected:
	public:
	};

class CFGBase : Base
{
private:
protected:
public:
};

class SIMBase : Base
{
private:
protected:
public:
};

class UIBase : Base
{
private:
protected:
public:
};

class NETBase : Base
{
private:
	Messenger* messenger;
	int packetSize;
public:
	NetBase();
	virtual void send() = 0;
	virtual void put(std::string string) = 0;
	virtual std::string get() = 0;
};

	class Messenger
	{
	private:
	protected:
	public:
	};

class Base
{
protected:
	std::string identifier;
};
*/