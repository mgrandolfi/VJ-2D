#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdint>
#include <cstring>
#include <cctype>
#include "Game.h"
#include <SDL2/SDL.h>      
#include <SDL2/SDL_mixer.h>

Mix_Music* musicMenu;
Mix_Music* musicLevel1;
Mix_Music* musicLevel2;
Mix_Music* musicLevel3;
Mix_Music* musicLevel4;
Mix_Music* musicLevel5;
Mix_Music* musicWin;
Mix_Music* musicPause;
Mix_Music* musicCredits;

// Button slots on menu.png (640x480) — inner dark panels between gold rails ~x214–424
static const float MENU_BTN_X = 222.f;
static const float MENU_BTN_W = 196.f;
static const float MENU_BTN_H = 55.f;
static const float MENU_BTN_Y[] = { 160.f, 265.f, 365.f };

// Instructions — inner panel of bottom gold frame (below y≈436 bar, above y≈475)
static const float INSTR_BTN_X = 258.f;
static const float INSTR_BTN_Y = 440.f;
static const float INSTR_BTN_W = 126.f;
static const float INSTR_BTN_H = 34.f;

static const float PAUSE_ROW_X = 100.f;
static const float PAUSE_ROW_W = 440.f;
static const float PAUSE_ROW_H = 44.f;
static const float PAUSE_ROW_Y[] = { 196.f, 264.f, 332.f };

static const float BTN_PLAY_CROP[] = { 157.f, 94.f, 353.f, 179.f };       // 506x274
static const float BTN_INSTR_CROP[] = { 134.f, 217.f, 506.f, 258.f };     // 640x476
static const float BTN_CRED_CROP[] = { 67.f, 101.f, 418.f, 160.f };       // 486x263
static const float BTN_BACK_CROP[] = { 8.f, 8.f, 573.f, 208.f };         // 582x217


namespace {

const uint8_t G_A[] = {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
const uint8_t G_B[] = {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E};
const uint8_t G_C[] = {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E};
// Flat left spine, open right — reads as D (not O)
const uint8_t G_D[] = {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E};
const uint8_t G_0[] = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
const uint8_t G_1[] = {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E};
const uint8_t G_2[] = {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F};
const uint8_t G_3[] = {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E};
const uint8_t G_E[] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F};
const uint8_t G_H[] = {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
const uint8_t G_I[] = {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E};
const uint8_t G_K[] = {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11};
const uint8_t G_L[] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F};
const uint8_t G_M[] = {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11};
const uint8_t G_N[] = {0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11};
const uint8_t G_O[] = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
const uint8_t G_P[] = {0x0E, 0x11, 0x11, 0x0E, 0x10, 0x10, 0x10};
const uint8_t G_R[] = {0x0E, 0x11, 0x11, 0x0E, 0x14, 0x12, 0x11};
const uint8_t G_S[] = {0x1F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E};
const uint8_t G_T[] = {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
const uint8_t G_U[] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
const uint8_t G_X[] = {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11};
const uint8_t G_F[] = {0x1E, 0x10, 0x10, 0x1C, 0x10, 0x10, 0x10};
const uint8_t G_G[] = {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0E};
const uint8_t G_J[] = {0x04, 0x04, 0x04, 0x04, 0x04, 0x11, 0x0E};
const uint8_t G_Q[] = {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D};
const uint8_t G_Z[] = {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F};
const uint8_t G_V[] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04};
const uint8_t G_W[] = {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A};
const uint8_t G_Y[] = {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04};

const uint8_t *glyphLookup(char c)
{
	switch (std::toupper(static_cast<unsigned char>(c)))
	{
	case 'A': return G_A;
	case 'B': return G_B;
	case 'C': return G_C;
	case 'D': return G_D;
	case 'E': return G_E;
	case 'F': return G_F;
	case 'G': return G_G;
	case 'J': return G_J;
	case 'Q': return G_Q;
	case 'Z': return G_Z;
	case 'V': return G_V;
	case 'W': return G_W;
	case 'Y': return G_Y;
	case '0': return G_0;
	case '1': return G_1;
	case '2': return G_2;
	case '3': return G_3;
	case 'H': return G_H;
	case 'I': return G_I;
	case 'K': return G_K;
	case 'L': return G_L;
	case 'M': return G_M;
	case 'N': return G_N;
	case 'O': return G_O;
	case 'P': return G_P;
	case 'R': return G_R;
	case 'S': return G_S;
	case 'T': return G_T;
	case 'U': return G_U;
	case 'X': return G_X;
	default:  return nullptr;
	}
}

} 


void Game::init()
{
	bPlay = true;
	godMode = false;
	memset(keys, 0, sizeof(keys));
	currentLevel   = 1;
	state          = STATE_MENU;
	menuSelection  = 0;
	pauseSelection = 0;
	instrBackHover = false;
	mouseX = 0;
	mouseY = 0;

	glClearColor(0.f, 0.f, 0.f, 1.0f);

	scene.initShaders();
	initUI();

	SDL_Init(SDL_INIT_AUDIO);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

	musicMenu    = Mix_LoadMUS("sound/menu.mp3");
	musicLevel1  = Mix_LoadMUS("sound/level1.mp3");
	musicLevel2  = Mix_LoadMUS("sound/level2.mp3");
	musicLevel3  = Mix_LoadMUS("sound/level3.mp3");
	musicLevel4  = Mix_LoadMUS("sound/level4.mp3");
	musicLevel5  = Mix_LoadMUS("sound/level5.mp3");
	musicPause   = Mix_LoadMUS("sound/pause.mp3");
	musicWin     = Mix_LoadMUS("sound/win.mp3");
	musicCredits = Mix_LoadMUS("sound/credits.mp3");

	previousState = (GameState)-1;
	previousLevel = -1;
}

void Game::initUI()
{
	menuTex.loadFromFile("images/ui/menu.png", TEXTURE_PIXEL_FORMAT_RGBA);
	winTex.loadFromFile("images/ui/win.png", TEXTURE_PIXEL_FORMAT_RGBA);
	instructionsTex.loadFromFile("images/ui/instructions.png", TEXTURE_PIXEL_FORMAT_RGBA);
	creditsTex.loadFromFile("images/ui/credits.png", TEXTURE_PIXEL_FORMAT_RGBA);

	btnPlayTex.loadFromFile("images/ui/btn_play.png", TEXTURE_PIXEL_FORMAT_RGBA);
	btnInstrTex.loadFromFile("images/ui/btn_instructions.png", TEXTURE_PIXEL_FORMAT_RGBA);
	btnCreditsTex.loadFromFile("images/ui/btn_credits.png", TEXTURE_PIXEL_FORMAT_RGBA);
	btnBackTex.loadFromFile("images/ui/btn_back.png", TEXTURE_PIXEL_FORMAT_RGBA);

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

	glGenVertexArrays(1, &uiRectVao);
	glBindVertexArray(uiRectVao);
	glGenBuffers(1, &uiRectVbo);
	glBindBuffer(GL_ARRAY_BUFFER, uiRectVbo);
	glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(float), nullptr, GL_STREAM_DRAW);
	glEnableVertexAttribArray(uiPosLoc);
	glVertexAttribPointer(uiPosLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(uiTexLoc);
	glVertexAttribPointer(uiTexLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
	glBindVertexArray(0);

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
	glBindVertexArray(0);
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
	glBindVertexArray(0);
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
	glBindVertexArray(0);
}

void Game::renderTexturedRect(Texture &tex, float x, float y, float w, float h,
                              float u0, float v0, float u1, float v1)
{
	float vd[] = {
		x,     y,     u0, v0,
		x + w, y,     u1, v0,
		x + w, y + h, u1, v1,
		x,     y,     u0, v0,
		x + w, y + h, u1, v1,
		x,     y + h, u0, v1
	};

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
	glBindVertexArray(uiRectVao);
	glBindBuffer(GL_ARRAY_BUFFER, uiRectVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vd), vd, GL_STREAM_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glBindVertexArray(0);
}

void Game::renderSubrectFitted(Texture &tex, float x, float y, float boxW, float boxH,
                               int texW, int texH,
                               float px0, float py0, float px1, float py1,
                               float stretchX,
                               float stretchY)
{
	float cw = px1 - px0;
	float ch = py1 - py0;
	if (cw <= 0.f || ch <= 0.f)
		return;
	float aspect = cw / ch;
	float bw = boxW;
	float bh = boxH;
	float dw, dh;
	if (aspect > bw / bh)
	{
		dw = bw;
		dh = bw / aspect;
	}
	else
	{
		dh = bh;
		dw = bh * aspect;
	}
	if (stretchX != 1.f)
	{
		dw *= stretchX;
		if (dw > bw)
			dw = bw;
	}
	if (stretchY != 1.f)
	{
		dh *= stretchY;
		if (dh > bh)
			dh = bh;
	}
	float ox = x + (boxW - dw) * 0.5f;
	float oy = y + (boxH - dh) * 0.5f;
	float u0 = px0 / float(texW);
	float u1 = px1 / float(texW);
	float v0 = py0 / float(texH);
	float v1 = py1 / float(texH);
	renderTexturedRect(tex, ox, oy, dw, dh, u0, v0, u1, v1);
}

void Game::drawGlyphRows(const uint8_t *rows, float x, float y, float ps,
                        float r, float g, float b)
{
	if (!rows)
		return;
	for (int row = 0; row < 7; ++row)
	{
		uint8_t bits = rows[row];
		for (int col = 0; col < 5; ++col)
		{
			if (bits & (1 << (4 - col)))
				renderColorQuad(x + col * ps, y + row * ps, ps, ps, r, g, b, 1.f);
		}
	}
}

float Game::measureBitmapTextWidth(const char *text, float pixel)
{
	float w = 0.f;
	for (const char *t = text; *t; ++t)
	{
		if (*t == ' ')
			w += 4.f * pixel;
		else if (glyphLookup(*t))
			w += 6.f * pixel;
	}
	return w;
}

void Game::renderBitmapTextCenter(const char *text, float cx, float cy, float pixel,
                                float r, float g, float b)
{
	float w = measureBitmapTextWidth(text, pixel);
	float x0 = cx - w * 0.5f;
	float y0 = cy - (7.f * pixel) * 0.5f;

	for (const char *t = text; *t; ++t)
	{
		if (*t == ' ')
		{
			x0 += 4.f * pixel;
			continue;
		}
		const uint8_t *gr = glyphLookup(*t);
		drawGlyphRows(gr, x0, y0, pixel, r, g, b);
		x0 += 6.f * pixel;
	}
}

void Game::renderBitmapTextHud(const char *text, float x, float y, float pixel,
                               float r, float g, float b)
{
	float y0 = y;
	for (const char *t = text; *t; ++t)
	{
		if (*t == ' ')
		{
			x += 4.f * pixel;
			continue;
		}
		const uint8_t *gr = glyphLookup(*t);
		drawGlyphRows(gr, x, y0, pixel, r, g, b);
		x += 6.f * pixel;
	}
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
			if (scene.isGameOver()) state = STATE_MENU;
			else if (scene.isLevelComplete())
			{
				if (currentLevel < 5)
				loadLevel(currentLevel + 1);
				else state = STATE_WIN;
			}
			break;
		default:
			break;
	}
	updateMusic();
	return bPlay;
}

void Game::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch (state)
	{
	case STATE_MENU:
		renderUI(menuTex);
		renderColorQuad(MENU_BTN_X, MENU_BTN_Y[menuSelection],
		                MENU_BTN_W, MENU_BTN_H,
		                1.f, 0.85f, 0.f, 0.15f);
		renderSubrectFitted(btnPlayTex, MENU_BTN_X + 4.f, MENU_BTN_Y[0] + 4.f,
		                    MENU_BTN_W - 8.f, MENU_BTN_H - 8.f,
		                    506, 274,
		                    BTN_PLAY_CROP[0], BTN_PLAY_CROP[1], BTN_PLAY_CROP[2], BTN_PLAY_CROP[3],
		                    1.22f, 1.f);
		renderSubrectFitted(btnInstrTex, MENU_BTN_X + 4.f, MENU_BTN_Y[1] + 4.f,
		                    MENU_BTN_W - 8.f, MENU_BTN_H - 8.f,
		                    640, 476,
		                    BTN_INSTR_CROP[0], BTN_INSTR_CROP[1], BTN_INSTR_CROP[2], BTN_INSTR_CROP[3]);
		renderSubrectFitted(btnCreditsTex, MENU_BTN_X + 4.f, MENU_BTN_Y[2] + 4.f,
		                    MENU_BTN_W - 8.f, MENU_BTN_H - 8.f,
		                    486, 263,
		                    BTN_CRED_CROP[0], BTN_CRED_CROP[1], BTN_CRED_CROP[2], BTN_CRED_CROP[3]);
		renderColorQuad(MENU_BTN_X - 2.f, MENU_BTN_Y[menuSelection] - 2.f,
		                MENU_BTN_W + 4.f, 2.f, 1.f, 0.85f, 0.f, 1.f);
		renderColorQuad(MENU_BTN_X - 2.f, MENU_BTN_Y[menuSelection] + MENU_BTN_H,
		                MENU_BTN_W + 4.f, 2.f, 1.f, 0.85f, 0.f, 1.f);
		renderColorQuad(MENU_BTN_X - 2.f, MENU_BTN_Y[menuSelection],
		                2.f, MENU_BTN_H, 1.f, 0.85f, 0.f, 1.f);
		renderColorQuad(MENU_BTN_X + MENU_BTN_W, MENU_BTN_Y[menuSelection],
		                2.f, MENU_BTN_H, 1.f, 0.85f, 0.f, 1.f);
		break;

	case STATE_PLAYING:
		scene.render();
		break;

	case STATE_PAUSED:
		scene.render();
		renderColorQuad(0.f, 0.f, 640.f, 480.f, 0.f, 0.f, 0.f, 0.55f);
		renderBitmapTextCenter("PAUSED", 320.f, 88.f, 5.f, 1.f, 1.f, 1.f);
		renderBitmapTextCenter("CONTINUE", 320.f, PAUSE_ROW_Y[0] + PAUSE_ROW_H * 0.5f, 4.f,
		                    1.f, 1.f, 1.f);
		renderBitmapTextCenter("RESTART", 320.f, PAUSE_ROW_Y[1] + PAUSE_ROW_H * 0.5f, 4.f,
		                    1.f, 1.f, 1.f);
		renderBitmapTextCenter("EXIT", 320.f, PAUSE_ROW_Y[2] + PAUSE_ROW_H * 0.5f, 4.f,
		                    1.f, 1.f, 1.f);
		renderColorQuad(PAUSE_ROW_X - 2.f, PAUSE_ROW_Y[pauseSelection] - 2.f,
		                PAUSE_ROW_W + 4.f, 2.f, 1.f, 0.85f, 0.f, 1.f);
		renderColorQuad(PAUSE_ROW_X - 2.f, PAUSE_ROW_Y[pauseSelection] + PAUSE_ROW_H,
		                PAUSE_ROW_W + 4.f, 2.f, 1.f, 0.85f, 0.f, 1.f);
		renderColorQuad(PAUSE_ROW_X - 2.f, PAUSE_ROW_Y[pauseSelection],
		                2.f, PAUSE_ROW_H, 1.f, 0.85f, 0.f, 1.f);
		renderColorQuad(PAUSE_ROW_X + PAUSE_ROW_W, PAUSE_ROW_Y[pauseSelection],
		                2.f, PAUSE_ROW_H, 1.f, 0.85f, 0.f, 1.f);
		break;

	case STATE_WIN:
		renderUI(winTex);
		break;

	case STATE_INSTRUCTIONS:
		renderUI(instructionsTex);
		if (instrBackHover)
			renderColorQuad(INSTR_BTN_X, INSTR_BTN_Y, INSTR_BTN_W, INSTR_BTN_H,
			                1.f, 0.85f, 0.f, 0.12f);
		renderSubrectFitted(btnBackTex, INSTR_BTN_X + 2.f, INSTR_BTN_Y + 2.f,
		                    INSTR_BTN_W - 4.f, INSTR_BTN_H - 4.f,
		                    582, 217,
		                    BTN_BACK_CROP[0], BTN_BACK_CROP[1], BTN_BACK_CROP[2], BTN_BACK_CROP[3],
		                    1.18f, 1.18f);
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
			case 0: loadLevel(1);               break;
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
			case 0: state = STATE_PLAYING;     break;
			case 1: loadLevel(currentLevel);   break;
			case 2: state = STATE_MENU;       break;
			}
		}
		else if (key == GLFW_KEY_P || key == GLFW_KEY_ESCAPE)
			state = STATE_PLAYING;
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
			if (x >= PAUSE_ROW_X && x <= PAUSE_ROW_X + PAUSE_ROW_W &&
			    y >= PAUSE_ROW_Y[i] && y <= PAUSE_ROW_Y[i] + PAUSE_ROW_H)
			{
				pauseSelection = i;
				break;
			}
		}
	}
	else if (state == STATE_INSTRUCTIONS)
	{
		instrBackHover = (x >= INSTR_BTN_X && x <= INSTR_BTN_X + INSTR_BTN_W &&
		                y >= INSTR_BTN_Y && y <= INSTR_BTN_Y + INSTR_BTN_H);
	}
}

void Game::mousePress(int button)
{
	if (button != 0) return;

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
				case 0: loadLevel(1);               break;
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
			if (mouseX >= PAUSE_ROW_X && mouseX <= PAUSE_ROW_X + PAUSE_ROW_W &&
			    mouseY >= PAUSE_ROW_Y[i] && mouseY <= PAUSE_ROW_Y[i] + PAUSE_ROW_H)
			{
				pauseSelection = i;
				switch (i)
				{
				case 0: state = STATE_PLAYING;     break;
				case 1: loadLevel(currentLevel);   break;
				case 2: state = STATE_MENU;         break;
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

void Game::updateMusic()
{
	if(state != previousState || currentLevel != previousLevel)
	{
		Mix_HaltMusic();

		switch(state)
		{
		case STATE_MENU:
			Mix_PlayMusic(musicMenu,-1);
			break;

		case STATE_PLAYING:
			switch(currentLevel)
			{
			case 1: Mix_PlayMusic(musicLevel1,-1); break;
			case 2: Mix_PlayMusic(musicLevel2,-1); break;
			case 3: Mix_PlayMusic(musicLevel3,-1); break;
			case 4: Mix_PlayMusic(musicLevel4,-1); break;
			case 5: Mix_PlayMusic(musicLevel5,-1); break;
			}
			break;

		case STATE_PAUSED:
			Mix_PlayMusic(musicPause,-1);
			break;

		case STATE_WIN:
			Mix_PlayMusic(musicWin,-1);
			break;

		case STATE_CREDITS:
			Mix_PlayMusic(musicCredits,-1);
			break;

		default:
			Mix_HaltMusic();
			break;
		}

		previousState = state;
		previousLevel = currentLevel;
	}
}