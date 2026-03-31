#ifndef _GAME_INCLUDE
#define _GAME_INCLUDE

#include <cstdint>

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
	void renderTexturedRect(Texture &tex, float x, float y, float w, float h,
	                        float u0, float v0, float u1, float v1);
	void renderSubrectFitted(Texture &tex, float x, float y, float boxW, float boxH,
	                         int texW, int texH,
	                         float px0, float py0, float px1, float py1);
	void renderBitmapTextCenter(const char *text, float cx, float cy, float pixel,
	                            float r, float g, float b);
	float measureBitmapTextWidth(const char *text, float pixel);
	void drawGlyphRows(const uint8_t *rows, float x, float y, float ps,
	                   float r, float g, float b);
	void updateMusic();

private:
	bool bPlay;
	bool keys[GLFW_KEY_LAST + 1];

	GameState state, previousState;
	int currentLevel;
	bool godMode;

	Scene scene;

	Texture menuTex, winTex, instructionsTex, creditsTex;
	Texture btnPlayTex, btnInstrTex, btnCreditsTex, btnBackTex;

	ShaderProgram uiProgram;
	GLuint uiVao, uiVbo;
	GLuint uiRectVao, uiRectVbo;
	GLint uiPosLoc, uiTexLoc;

	GLuint whiteTex;

	int menuSelection;
	int pauseSelection;

	bool instrBackHover;

	int mouseX, mouseY;
};


#endif // _GAME_INCLUDE
