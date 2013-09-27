//header guard, so this file is only compiled once
#pragma once

#include <string>

using namespace std;

//Forward declaration of our Window Interface
class IWindow;
class IRenderer;

//Structure for holding GameOptions, this will be loaded from config files 
struct GameOptionsDesc
{
	wstring gameName;
	int width;
	int height;
	bool fullscreen;
};

//We have this here so we dont need to prefix all
//standard library types with std::
using namespace std;

//Our Game Application class
class CGameApplication
{
public:
	CGameApplication(void);
	//virtual deconstructor, so this can be overridden
	virtual ~CGameApplication(void);
	//Virtual function, can be overridden
	virtual bool init();
	void run();
	//Virtual function, can be overridden
	virtual void render();
	//Virtual function, can be overridden
	virtual void update();
private:
	bool parseConfigFile();
	bool initInput();
	bool initGame();
	bool initGraphics();
	bool initPhysics();

	bool initWindow();
private:
	IWindow * m_pWindow;
	IRenderer * m_pRenderer;
	GameOptionsDesc m_GameOptionDesc;
	wstring m_ConfigFileName;

};