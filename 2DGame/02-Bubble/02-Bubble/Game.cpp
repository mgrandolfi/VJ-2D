#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>
#include "Game.h"


// Button layout on the menu.png background (measured from menu.png pixel positions)
static const float MENU_BTN_X = 172.f;
static const float MENU_BTN_W = 296.f;
static const float MENU_BTN_H = 52.f;
static const float MENU_BTN_Y[] = { 204.f, 282.f, 360.f };

// Instructions screen back button (bottom center of instructions.png)
static const float INSTR_BTN_X = 230.f;
static const float INSTR_BTN_Y = 430.f;
static const float INSTR_BTN_W = 180.f;
static const float INSTR_BTN_H = 36.f;

// Pause overlay — button positions measured from pause.png pixel analysis
static const float PAUSE_BTN_X = 205.f;
static const float PAUSE_BTN_W = 250.f;
static const float PAUSE_BTN_H = 32.f;
static const float PAUSE_BTN_Y[] = { 213.f, 273.f, 333.f };


void Game::init()
{
	bPlay = true;
	godMode = false;
	memset(keys, 0, sizeof(keys));
	currentLevel   = 1;
	state          = STATE_MENU;
	menuSelection  = 0;
	pauseSelection = 0;
	mouseX = 0;
	mouseY = 0;

	glClearColor(0.f, 0.f, 0.f, 1.0f);

	scene.initShaders();
	initUI();
}

void Game::initUI()
{
	menuTex.loadFromFile("images/ui/menu.png", TEXTURE_PIXEL_FORMAT_RGBA);
	gameoverTex.loadFromFile("images/ui/gameover.png", TEXTURE_PIXEL_FORMAT_RGBA);
	winTex.loadFromFile("images/ui/win.png", TEXTURE_PIXEL_FORMAT_RGBA);
	instructionsTex.loadFromFile("images/ui/instructions.png", TEXTURE_PIXEL_FORMAT_RGBA);
	pauseTex.loadFromFile("images/ui/pause.png", TEXTURE_PIXEL_FORMAT_RGBA);
	creditsTex.loadFromFile("images/ui/credits.png", TEXTURE_PIXEL_FORMAT_RGBA);

	btnPlayTex.loadFromFile("images/ui/btn_play.png", TEXTURE_PIXEL_FORMAT_RGBA);
	btnInstrTex.loadFromFile("images/ui/btn_instructions.png", TEXTURE_PIXEL_FORMAT_RGBA);
	btnCreditsTex.loadFromFile("images/ui/btn_credits.png", TEXTURE_PIXEL_FORMAT_RGBA);

	Shader vShader, fShader;
	vShader.initFromFile(VERTEX_SHADER, "shaders/texture.vert");
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/texture.frag");
	uiProgram.init();
	uiProgram.addShader(vShader);
	uiProgram.addShader(fShader);
	uiProgram.link();
	uiProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();

	float quadData[] = {
		  0.f,   0.f, 0.f, 0.f,
		640.f,   0.f, 1.f, 0.f,
		640.f, 480.f, 1.f, 1.f,
		  0.f,   0.f, 0.f, 0.f,
		640.f, 480.f, 1.f, 1.f,
		  0.f, 480.f, 0.f, 1.f
	};

	glGenVertexArrays(1, &uiVao);
	glBindVertexArray(uiVao);
	glGenBuffers(1, &uiVbo);
	glBindBuffer(GL_ARRAY_BUFFER, uiVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_STATIC_DRAW);
	uiPosLoc = uiProgram.bindVertexAttribute("position", 2, 4 * sizeof(float), 0);
	uiTexLoc = uiProgram.bindVertexAttribute("texCoord", 2, 4 * sizeof(float), (void *)(2 * sizeof(float)));

	// 1x1 white texture for colored quad rendering
	unsigned char whitePixel[] = {255, 255, 255, 255};
	glGenTextures(1, &whiteTex);
	glBindTexture(GL_TEXTURE_2D, whiteTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Game::renderUI(Texture &tex)
{
	glm::mat4 proj = glm::ortho(0.f, 640.f, 480.f, 0.f);
	glm::mat4 mv   = glm::mat4(1.f);

	uiProgram.use();
	uiProgram.setUniformMatrix4f("projection", proj);
	uiProgram.setUniformMatrix4f("modelview", mv);
	uiProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	uiProgram.setUniform2f("texCoordDispl", 0.f, 0.f);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	tex.use();
	glBindVertexArray(uiVao);
	glEnableVertexAttribArray(uiPosLoc);
	glEnableVertexAttribArray(uiTexLoc);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}

void Game::renderUIAt(Texture &tex, float x, float y, float w, float h)
{
	glm::mat4 proj = glm::ortho(0.f, 640.f, 480.f, 0.f);
	glm::mat4 mv = glm::translate(glm::mat4(1.f), glm::vec3(x, y, 0.f)) *
	               glm::scale(glm::mat4(1.f), glm::vec3(w / 640.f, h / 480.f, 1.f));

	uiProgram.use();
	uiProgram.setUniformMatrix4f("projection", proj);
	uiProgram.setUniformMatrix4f("modelview", mv);
	uiProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	uiProgram.setUniform2f("texCoordDispl", 0.f, 0.f);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	tex.use();
	glBindVertexArray(uiVao);
	glEnableVertexAttribArray(uiPosLoc);
	glEnableVertexAttribArray(uiTexLoc);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}

void Game::renderColorQuad(float x, float y, float w, float h,
                           float r, float g, float b, float a)
{
	glm::mat4 proj = glm::ortho(0.f, 640.f, 480.f, 0.f);
	glm::mat4 mv = glm::translate(glm::mat4(1.f), glm::vec3(x, y, 0.f)) *
	               glm::scale(glm::mat4(1.f), glm::vec3(w / 640.f, h / 480.f, 1.f));

	uiProgram.use();
	uiProgram.setUniformMatrix4f("projection", proj);
	uiProgram.setUniformMatrix4f("modelview", mv);
	uiProgram.setUniform4f("color", r, g, b, a);
	uiProgram.setUniform2f("texCoordDispl", 0.f, 0.f);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, whiteTex);
	glBindVertexArray(uiVao);
	glEnableVertexAttribArray(uiPosLoc);
	glEnableVertexAttribArray(uiTexLoc);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}

void Game::changeState(GameState s)
{
	state = s;
}

void Game::loadLevel(int n)
{
	currentLevel = n;
	scene.loadLevel(n);
	state = STATE_PLAYING;
}

bool Game::update(int deltaTime)
{
	switch (state)
	{
	case STATE_PLAYING:
		scene.update(deltaTime);
		if (scene.isGameOver())
			state = STATE_GAME_OVER;
		else if (scene.isLevelComplete())
		{
			if (currentLevel < 5)
				loadLevel(currentLevel + 1);
			else
				state = STATE_WIN;
		}
		break;

	default:
		break;
	}

	return bPlay;
}

void Game::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch (state)
	{
	case STATE_MENU:
		renderUI(menuTex);
		// Render button label textures inside the button frames
		renderUIAt(btnPlayTex,     MENU_BTN_X + 2.f, MENU_BTN_Y[0] + 2.f, MENU_BTN_W - 4.f, MENU_BTN_H - 4.f);
		renderUIAt(btnInstrTex,    MENU_BTN_X + 2.f, MENU_BTN_Y[1] + 2.f, MENU_BTN_W - 4.f, MENU_BTN_H - 4.f);
		renderUIAt(btnCreditsTex,  MENU_BTN_X + 2.f, MENU_BTN_Y[2] + 2.f, MENU_BTN_W - 4.f, MENU_BTN_H - 4.f);
		// Highlight selected button (gold tint + border)
		renderColorQuad(MENU_BTN_X, MENU_BTN_Y[menuSelection],
		                MENU_BTN_W, MENU_BTN_H,
		                1.f, 0.85f, 0.f, 0.25f);
		// Gold border lines
		renderColorQuad(MENU_BTN_X, MENU_BTN_Y[menuSelection] - 2.f,
		                MENU_BTN_W, 2.f, 1.f, 0.85f, 0.f, 1.f);
		renderColorQuad(MENU_BTN_X, MENU_BTN_Y[menuSelection] + MENU_BTN_H,
		                MENU_BTN_W, 2.f, 1.f, 0.85f, 0.f, 1.f);
		renderColorQuad(MENU_BTN_X - 2.f, MENU_BTN_Y[menuSelection],
		                2.f, MENU_BTN_H, 1.f, 0.85f, 0.f, 1.f);
		renderColorQuad(MENU_BTN_X + MENU_BTN_W, MENU_BTN_Y[menuSelection],
		                2.f, MENU_BTN_H, 1.f, 0.85f, 0.f, 1.f);
		break;

	case STATE_PLAYING:
		scene.render();
		break;

	case STATE_PAUSED:
	{
		// Frozen game scene underneath
		scene.render();
		// Slight dark tint so the pause overlay reads clearly
		renderColorQuad(0.f, 0.f, 640.f, 480.f, 0.f, 0.f, 0.f, 0.35f);
		// pauseTex at ~90% opacity — it is fully opaque but we blend it so
		// the game scene is faintly visible, and it carries the button labels.
		{
			glm::mat4 proj = glm::ortho(0.f, 640.f, 480.f, 0.f);
			glm::mat4 mv   = glm::mat4(1.f);
			uiProgram.use();
			uiProgram.setUniformMatrix4f("projection", proj);
			uiProgram.setUniformMatrix4f("modelview", mv);
			uiProgram.setUniform4f("color", 1.f, 1.f, 1.f, 0.92f);
			uiProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			pauseTex.use();
			glBindVertexArray(uiVao);
			glEnableVertexAttribArray(uiPosLoc);
			glEnableVertexAttribArray(uiTexLoc);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisable(GL_BLEND);
			glDisable(GL_TEXTURE_2D);
		}
		// Gold highlight on the selected button row
		renderColorQuad(PAUSE_BTN_X - 2.f, PAUSE_BTN_Y[pauseSelection] - 2.f,
		                PAUSE_BTN_W + 4.f, PAUSE_BTN_H + 4.f,
		                1.f, 0.85f, 0.f, 0.30f);
		// Gold border lines (top / bottom / left / right)
		renderColorQuad(PAUSE_BTN_X - 2.f, PAUSE_BTN_Y[pauseSelection] - 2.f,
		                PAUSE_BTN_W + 4.f, 2.f, 1.f, 0.85f, 0.f, 1.f);
		renderColorQuad(PAUSE_BTN_X - 2.f, PAUSE_BTN_Y[pauseSelection] + PAUSE_BTN_H,
		                PAUSE_BTN_W + 4.f, 2.f, 1.f, 0.85f, 0.f, 1.f);
		renderColorQuad(PAUSE_BTN_X - 2.f, PAUSE_BTN_Y[pauseSelection],
		                2.f, PAUSE_BTN_H, 1.f, 0.85f, 0.f, 1.f);
		renderColorQuad(PAUSE_BTN_X + PAUSE_BTN_W, PAUSE_BTN_Y[pauseSelection],
		                2.f, PAUSE_BTN_H, 1.f, 0.85f, 0.f, 1.f);
		break;
	}

	case STATE_GAME_OVER:
		renderUI(gameoverTex);
		break;

	case STATE_WIN:
		renderUI(winTex);
		break;

	case STATE_INSTRUCTIONS:
		renderUI(instructionsTex);
		// Back button highlight at bottom center
		renderColorQuad(INSTR_BTN_X, INSTR_BTN_Y,
		                INSTR_BTN_W, INSTR_BTN_H,
		                0.2f, 0.2f, 0.4f, 0.5f);
		// Gold border
		renderColorQuad(INSTR_BTN_X, INSTR_BTN_Y - 2.f,
		                INSTR_BTN_W, 2.f, 1.f, 0.85f, 0.f, 1.f);
		renderColorQuad(INSTR_BTN_X, INSTR_BTN_Y + INSTR_BTN_H,
		                INSTR_BTN_W, 2.f, 1.f, 0.85f, 0.f, 1.f);
		renderColorQuad(INSTR_BTN_X - 2.f, INSTR_BTN_Y,
		                2.f, INSTR_BTN_H, 1.f, 0.85f, 0.f, 1.f);
		renderColorQuad(INSTR_BTN_X + INSTR_BTN_W, INSTR_BTN_Y,
		                2.f, INSTR_BTN_H, 1.f, 0.85f, 0.f, 1.f);
		break;

	case STATE_CREDITS:
		renderUI(creditsTex);
		break;
	}
}

void Game::keyPressed(int key)
{
	if (key < 0 || key > GLFW_KEY_LAST) return;
	keys[key] = true;

	switch (state)
	{
	case STATE_MENU:
		if (key == GLFW_KEY_UP)
			menuSelection = (menuSelection + 2) % 3;
		else if (key == GLFW_KEY_DOWN)
			menuSelection = (menuSelection + 1) % 3;
		else if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
		{
			switch (menuSelection)
			{
			case 0: loadLevel(1);              break;
			case 1: state = STATE_INSTRUCTIONS; break;
			case 2: state = STATE_CREDITS;      break;
			}
		}
		else if (key == GLFW_KEY_ESCAPE)
			bPlay = false;
		break;

	case STATE_PLAYING:
		if (key == GLFW_KEY_ESCAPE)
			state = STATE_MENU;
		else if (key == GLFW_KEY_P)
		{
			state = STATE_PAUSED;
			pauseSelection = 0;
		}
		else if (key == GLFW_KEY_G)
		{
			godMode = !godMode;
			scene.setGodMode(godMode);
		}
		else if (key == GLFW_KEY_K)
			scene.collectAllKeys();
		else if (key == GLFW_KEY_F9)
			scene.killAllEnemies();
		else if (key >= GLFW_KEY_1 && key <= GLFW_KEY_5)
			loadLevel(key - GLFW_KEY_0);
		break;

	case STATE_PAUSED:
		if (key == GLFW_KEY_UP)
			pauseSelection = (pauseSelection + 2) % 3;
		else if (key == GLFW_KEY_DOWN)
			pauseSelection = (pauseSelection + 1) % 3;
		else if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
		{
			switch (pauseSelection)
			{
			case 0: state = STATE_PLAYING;  break; // Continue
			case 1: loadLevel(currentLevel); break; // Restart
			case 2: state = STATE_MENU;     break; // Exit
			}
		}
		else if (key == GLFW_KEY_P || key == GLFW_KEY_ESCAPE)
			state = STATE_PLAYING;
		break;

	case STATE_GAME_OVER:
		if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
			loadLevel(1);
		else if (key == GLFW_KEY_ESCAPE)
			state = STATE_MENU;
		break;

	case STATE_WIN:
		if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE || key == GLFW_KEY_ESCAPE)
			state = STATE_MENU;
		break;

	case STATE_INSTRUCTIONS:
	case STATE_CREDITS:
		if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_ENTER || key == GLFW_KEY_BACKSPACE)
			state = STATE_MENU;
		break;
	}
}

void Game::keyReleased(int key)
{
	if (key < 0 || key > GLFW_KEY_LAST) return;
	keys[key] = false;
}

void Game::mouseMove(int x, int y)
{
	mouseX = x;
	mouseY = y;

	if (state == STATE_MENU)
	{
		for (int i = 0; i < 3; ++i)
		{
			if (x >= MENU_BTN_X && x <= MENU_BTN_X + MENU_BTN_W &&
			    y >= MENU_BTN_Y[i] && y <= MENU_BTN_Y[i] + MENU_BTN_H)
			{
				menuSelection = i;
				break;
			}
		}
	}
	else if (state == STATE_PAUSED)
	{
		for (int i = 0; i < 3; ++i)
		{
			if (x >= PAUSE_BTN_X && x <= PAUSE_BTN_X + PAUSE_BTN_W &&
			    y >= PAUSE_BTN_Y[i] && y <= PAUSE_BTN_Y[i] + PAUSE_BTN_H)
			{
				pauseSelection = i;
				break;
			}
		}
	}
}

void Game::mousePress(int button)
{
	if (button != 0) return; // left click only

	if (state == STATE_MENU)
	{
		for (int i = 0; i < 3; ++i)
		{
			if (mouseX >= MENU_BTN_X && mouseX <= MENU_BTN_X + MENU_BTN_W &&
			    mouseY >= MENU_BTN_Y[i] && mouseY <= MENU_BTN_Y[i] + MENU_BTN_H)
			{
				menuSelection = i;
				switch (i)
				{
				case 0: loadLevel(1);              break;
				case 1: state = STATE_INSTRUCTIONS; break;
				case 2: state = STATE_CREDITS;      break;
				}
				return;
			}
		}
	}
	else if (state == STATE_PAUSED)
	{
		for (int i = 0; i < 3; ++i)
		{
			if (mouseX >= PAUSE_BTN_X && mouseX <= PAUSE_BTN_X + PAUSE_BTN_W &&
			    mouseY >= PAUSE_BTN_Y[i] && mouseY <= PAUSE_BTN_Y[i] + PAUSE_BTN_H)
			{
				pauseSelection = i;
				switch (i)
				{
				case 0: state = STATE_PLAYING;   break;
				case 1: loadLevel(currentLevel); break;
				case 2: state = STATE_MENU;      break;
				}
				return;
			}
		}
	}
	else if (state == STATE_INSTRUCTIONS)
	{
		if (mouseX >= INSTR_BTN_X && mouseX <= INSTR_BTN_X + INSTR_BTN_W &&
		    mouseY >= INSTR_BTN_Y && mouseY <= INSTR_BTN_Y + INSTR_BTN_H)
		{
			state = STATE_MENU;
			return;
		}
	}
}

void Game::mouseRelease(int button)
{
}

bool Game::getKey(int key) const
{
	if (key < 0 || key > GLFW_KEY_LAST) return false;
	return keys[key];
}
