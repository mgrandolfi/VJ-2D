#ifndef _GAME_INCLUDE
#define _GAME_INCLUDE


#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Texture.h"
#include "ShaderProgram.h"
#include "Scene.h"


#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480


enum GameState
{
	STATE_MENU,
	STATE_PLAYING,
	STATE_PAUSED,
	STATE_GAME_OVER,
	STATE_WIN,
	STATE_INSTRUCTIONS,
	STATE_CREDITS
};


// Game is a singleton (a class with a single instance) that represents our whole application

class Game
{

private:
	Game() {}

public:
	static Game &instance()
	{
		static Game G;
		return G;
	}

	void init();
	bool update(int deltaTime);
	void render();

	// Input callback methods
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMove(int x, int y);
	void mousePress(int button);
	void mouseRelease(int button);

	bool getKey(int key) const;

	GameState getState() const { return state; }
	int getLevel() const { return currentLevel; }
	bool isGodMode() const { return godMode; }
	void changeState(GameState s);
	void loadLevel(int n);

private:
	void initUI();
	void renderUI(Texture &tex);
	void renderUIAt(Texture &tex, float x, float y, float w, float h);
	void renderColorQuad(float x, float y, float w, float h,
	                     float r, float g, float b, float a);

private:
	bool bPlay;
	bool keys[GLFW_KEY_LAST + 1];

	GameState state;
	int currentLevel;
	bool godMode;

	Scene scene;

	// UI full-screen textures
	Texture menuTex, gameoverTex, winTex, instructionsTex, pauseTex, creditsTex;

	// Menu button overlays
	Texture btnPlayTex, btnInstrTex, btnCreditsTex;

	// Shader and geometry for fullscreen quad rendering
	ShaderProgram uiProgram;
	GLuint uiVao, uiVbo;
	GLint uiPosLoc, uiTexLoc;

	// 1x1 white texture for drawing colored quads
	GLuint whiteTex;

	// Menu / pause selection
	int menuSelection;   // 0=Play, 1=Instructions, 2=Credits
	int pauseSelection;  // 0=Continue, 1=Restart, 2=Exit

	// Last known mouse position (for click detection)
	int mouseX, mouseY;
};


#endif // _GAME_INCLUDE
