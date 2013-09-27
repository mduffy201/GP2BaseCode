//header guard, so this file is only compiled once
#pragma once


#include <string>

//We have this here so we dont need to prefix all
//standard library types with std::
using namespace std;

//Forward declarations

//Window Interface
class IWindow;
//Renderer Interface
class IRenderer;

//Structure for holding GameOptions, this will be loaded from config files 
struct GameOptionsDesc
{
	wstring gameName;
	int width;
	int height;
	bool fullscreen;
};



//Game Application class
class CGameApplication
{
	//public functions
public:
	//Constructor
	CGameApplication(void);
	//virtual deconstructor
	virtual ~CGameApplication(void);
	//Virtual function, can be overridden by method in inheriting class with same signature
	virtual bool init();
	void run();
	virtual void render();
	virtual void update();
	//private functions
private:
	bool parseConfigFile();
	bool initInput();
	bool initGame();
	bool initGraphics();
	bool initPhysics();
	bool initWindow();
	//private variables
private:
	//pointer types
	IWindow * m_pWindow;
	IRenderer * m_pRenderer;

	GameOptionsDesc m_GameOptionDesc;
	wstring m_ConfigFileName;

};