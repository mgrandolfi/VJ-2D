#include <iostream>
#include <cmath>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"
#include <cstdio>
#include <GLFW/glfw3.h>

namespace {
	glm::ivec2 keyPickupPos(int ts, int ksz, int tx, int surfaceRow)
	{
		return glm::ivec2(tx * ts + (ts - ksz) / 2, surfaceRow * ts - ksz - 4);
	}

	glm::ivec2 itemPickupPos(int ts, int tx, int surfaceRow)
	{
		return glm::ivec2(tx * ts, (surfaceRow - 1) * ts);
	}

	// Row-major 1-based index (1 = top-left cell). Clamps to atlas bounds.
	glm::vec2 atlasUv1Based(int n, int cols, int rows, const glm::vec2 &cell)
	{
		if (cols < 1) cols = 1;
		if (rows < 1) rows = 1;
		int z = std::max(0, n - 1);
		const int maxCell = cols * rows - 1;
		if (z > maxCell) z = maxCell;
		const int col = z % cols;
		const int row = z / cols;
		return glm::vec2(float(col) * cell.x, float(row) * cell.y);
	}

	// ItemType order: WEIGHT, BOMB, BOOTS, CLOCK → atlas cells 1, 6, 2, 4
	int itemAnimToAtlasNum(int animIdx)
	{
		static const int m[4] = {1, 6, 2, 4};
		if (animIdx < 0 || animIdx > 3) return 1;
		return m[animIdx];
	}
}

#define SCREEN_X 0
#define SCREEN_Y 16

#define CAMERA_W 200.f
#define CAMERA_H 150.f

#define HUD_ICON_SIZE       24.f  // lives (hearts) only
#define ITEM_HUD_ICON_SIZE  48.f  // carried item + god HUD (2× atlas icon)
#define KEY_HUD_ICON_SIZE    55   // 25% larger than 44px base key HUD
#define KEY_HUD_ICON_NUDGE_UP 17.f // raise HUD key (atlas padding vs digit + outline)
#define GOD_AURA_PIXEL_SIZE  28   // 2× previous orbit sprites
#define HUD_MARGIN    8.f

#define SECRET_DOOR_COL 8
#define SECRET_DOOR_ROW 13
#define HUD_SPACING  28.f


// UV coords for door sprites in images/items.png (320x320, 10x10 grid of 32x32 cells)
// Row 2, col 0 = closed door;  row 2, col 1 = open door
#define DOOR_CELL_UV   glm::vec2(0.1f, 0.1f)
#define DOOR_UV_CLOSED glm::vec2(0.0f, 0.2f)
#define DOOR_UV_OPEN   glm::vec2(0.1f, 0.2f)


Scene::Scene()
{
	map            = NULL;
	mainMap        = NULL;
	secretMap      = NULL;
	player         = NULL;
	heartSprite    = NULL;
	itemSprite     = NULL;
	itemHudSprite  = NULL;
	keyWorldSprite = NULL;
	keyHudSprite   = NULL;
	godHudSprite   = NULL;
	doorSprite      = NULL;
	bombLitSprite   = NULL;
	bombSmokeSprite = NULL;
	chestSprite     = NULL;
	for (int i = 0; i < 3; ++i)
		godAuraSprites[i] = NULL;
	for (int i = 0; i < MAX_ENEMIES; ++i)
		enemies[i] = NULL;
	activeEnemies      = 0;
	keyWorldPixelSize  = 16;
	levelIndex         = 1;
	inSecretRoom       = false;
	secretAnimTimer    = 0;
	secretEnterPending = false;
	secretLootTaken    = true;
	secretIsChest      = false;
	chestOpening       = false;
	chestOpenTimer     = 0.f;
	secretDoorIndex    = 0;
	secretExitCooldown = 0;
	levelBack          = false;
	itemAtlasCols     = 10;
	itemAtlasRows     = 10;
	itemAtlasCellUv   = glm::vec2(0.1f, 0.1f);
	bombActive        = false;
	bombExploding     = false;
	bombTimer         = 0.f;
	bombSmokeTimer    = 0.f;
	weightCount       = 0;
	for (int i = 0; i < MAX_EXPLOSIONS; ++i)
		explosions[i].active = false;
}

Scene::~Scene()
{
	if (secretMap)      delete secretMap;
	if (mainMap)        delete mainMap;
	map = NULL;
	if (player)         delete player;
	if (heartSprite)    delete heartSprite;
	if (itemSprite)     delete itemSprite;
	if (itemHudSprite)  delete itemHudSprite;
	if (keyWorldSprite) delete keyWorldSprite;
	if (keyHudSprite)   delete keyHudSprite;
	if (godHudSprite)   delete godHudSprite;
	if (doorSprite)     delete doorSprite;
	if (bombLitSprite)  delete bombLitSprite;
	if (bombSmokeSprite) delete bombSmokeSprite;
	if (chestSprite)    delete chestSprite;
	for (int i = 0; i < 3; ++i)
		if (godAuraSprites[i]) delete godAuraSprites[i];
	for (int i = 0; i < MAX_ENEMIES; ++i)
		if (enemies[i]) delete enemies[i];
}

void Scene::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "shaders/texture.vert");
	if (!vShader.isCompiled())
		cout << "Vertex Shader Error: " << vShader.log() << endl;

	fShader.initFromFile(FRAGMENT_SHADER, "shaders/texture.frag");
	if (!fShader.isCompiled())
		cout << "Fragment Shader Error: " << fShader.log() << endl;

	texProgram.init();
	texProgram.addShader(vShader);
	texProgram.addShader(fShader);
	texProgram.link();
	if (!texProgram.isLinked())
		cout << "Shader Linking Error: " << texProgram.log() << endl;

	texProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();

	// HUD sprites
	heartTex.loadFromFile("images/sprites/heart.png", TEXTURE_PIXEL_FORMAT_RGBA);
	heartSprite = Sprite::createSprite(glm::ivec2(HUD_ICON_SIZE, HUD_ICON_SIZE),
	                                   glm::vec2(1.f, 1.f), &heartTex, &texProgram);
	heartSprite->setNumberAnimations(1);
	heartSprite->setAnimationSpeed(0, 1);
	heartSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	heartSprite->changeAnimation(0);

	itemTex.loadFromFile("images/sprites/items.png", TEXTURE_PIXEL_FORMAT_RGBA);
	itemTex.setWrapS(GL_CLAMP_TO_EDGE);
	itemTex.setWrapT(GL_CLAMP_TO_EDGE);
	itemTex.setMinFilter(GL_NEAREST);
	itemTex.setMagFilter(GL_NEAREST);
	configureItemsAtlas();

	const glm::vec2 cell = itemAtlasCellUv;
	const int ac = itemAtlasCols;
	const int ar = itemAtlasRows;

	itemHudSprite = Sprite::createSprite(glm::ivec2(int(ITEM_HUD_ICON_SIZE), int(ITEM_HUD_ICON_SIZE)),
	                                     cell, &itemTex, &texProgram);
	itemHudSprite->setNumberAnimations(4);
	for (int i = 0; i < 4; ++i) {
		itemHudSprite->setAnimationSpeed(i, 1);
		itemHudSprite->addKeyframe(i, atlasUv1Based(itemAnimToAtlasNum(i), ac, ar, cell));
	}
	itemHudSprite->changeAnimation(0);

	keyHudSprite = Sprite::createSprite(glm::ivec2(KEY_HUD_ICON_SIZE, KEY_HUD_ICON_SIZE),
	                                    cell, &itemTex, &texProgram);
	keyHudSprite->setNumberAnimations(1);
	keyHudSprite->setAnimationSpeed(0, 1);
	keyHudSprite->addKeyframe(0, atlasUv1Based(3, ac, ar, cell));
	keyHudSprite->changeAnimation(0);

	godHudSprite = Sprite::createSprite(glm::ivec2(int(ITEM_HUD_ICON_SIZE), int(ITEM_HUD_ICON_SIZE)),
	                                    cell, &itemTex, &texProgram);
	godHudSprite->setNumberAnimations(1);
	godHudSprite->setAnimationSpeed(0, 1);
	godHudSprite->addKeyframe(0, atlasUv1Based(5, ac, ar, cell));
	godHudSprite->changeAnimation(0);

	for (int i = 0; i < 3; ++i)
	{
		godAuraSprites[i] = Sprite::createSprite(glm::ivec2(GOD_AURA_PIXEL_SIZE, GOD_AURA_PIXEL_SIZE),
		                                         cell, &itemTex, &texProgram);
		godAuraSprites[i]->setNumberAnimations(1);
		godAuraSprites[i]->setAnimationSpeed(0, 1);
		godAuraSprites[i]->addKeyframe(0, atlasUv1Based(8, ac, ar, cell));
		godAuraSprites[i]->changeAnimation(0);
	}

	// Door sprite: images/items.png (320x320, 10x10 grid)
	// Row 2 col 0 = closed, row 2 col 1 = open
	// Size is rebuilt per level in recreateWorldPickupSprites(); 16x16 as placeholder.
	doorTex.loadFromFile("images/sprites/items.png", TEXTURE_PIXEL_FORMAT_RGBA);
	doorSprite = Sprite::createSprite(glm::ivec2(16, 16), DOOR_CELL_UV, &doorTex, &texProgram);
	doorSprite->setNumberAnimations(2);
	doorSprite->setAnimationSpeed(0, 1);
	doorSprite->addKeyframe(0, DOOR_UV_CLOSED);
	doorSprite->setAnimationSpeed(1, 1);
	doorSprite->addKeyframe(1, DOOR_UV_OPEN);
	doorSprite->changeAnimation(0);
}

void Scene::configureItemsAtlas()
{
	const int w = itemTex.width();
	const int h = itemTex.height();
	// Sprites use normalized UV quad size = one cell; cell must match real tile size in the PNG.
	// items.png is 320×320 with 32×32 icons (10×10). Using w/5 (64px) samples 2×2 icons at once.
	const int cellPxPrefer = 32;
	itemAtlasCols   = 10;
	itemAtlasRows   = 10;
	itemAtlasCellUv = glm::vec2(0.1f, 0.1f);
	if (w > 0 && h > 0 && (w % cellPxPrefer) == 0 && (h % cellPxPrefer) == 0)
	{
		itemAtlasCols   = w / cellPxPrefer;
		itemAtlasRows   = h / cellPxPrefer;
		itemAtlasCellUv = glm::vec2(float(cellPxPrefer) / float(w), float(cellPxPrefer) / float(h));
		return;
	}
	// Fallback: exactly 5 columns of square cells (e.g. non–32-divisible width)
	if (w >= 5 && h >= 5 && (w % 5) == 0)
	{
		const int cellPx = w / 5;
		if (cellPx > 0 && (h % cellPx) == 0)
		{
			itemAtlasCols   = 5;
			itemAtlasRows   = h / cellPx;
			itemAtlasCellUv = glm::vec2(1.f / 5.f, float(cellPx) / float(h));
			return;
		}
	}
	if (w > 0 && h > 0)
	{
		const int guess = std::max(1, w / 32);
		itemAtlasCols   = guess;
		itemAtlasRows   = std::max(1, h / 32);
		itemAtlasCellUv = glm::vec2(1.f / float(itemAtlasCols), 1.f / float(itemAtlasRows));
	}
}

void Scene::loadLevel(int level)
{
	if (secretMap) { delete secretMap; secretMap = NULL; }
	if (mainMap)   { delete mainMap;   mainMap   = NULL; }
	map = NULL;
	if (player) { delete player; player = NULL; }
	for (int i = 0; i < MAX_ENEMIES; ++i) {
		if (enemies[i]) { delete enemies[i]; enemies[i] = NULL; }
	}
	activeEnemies = 0;

	gameOver               = false;
	levelComplete          = false;
	levelBack              = false;
	playerEnteringElevator = false;
	playerWarpingOut       = false;
	enemiesFrozen          = false;
	freezeTimer            = 0.f;
	respawnTimer           = 0.f;
	keysCollected          = 0;
	itemCount              = 0;
	hasItem                = false;
	bombActive             = false;
	bombExploding          = false;
	bombTimer              = 0.f;
	bombSmokeTimer         = 0.f;
	weightCount            = 0;
	for (int i = 0; i < MAX_EXPLOSIONS; ++i)
		explosions[i].active = false;
	currentTime            = 0.f;
	levelIndex             = level;
	inSecretRoom           = false;
	doors.clear();
	secretAnimTimer        = 0;
	secretEnterPending     = false;
	secretLootTaken        = true;
	secretIsChest          = false;
	chestOpening           = false;
	chestOpenTimer         = 0.f;
	secretDoorIndex        = 0;
	secretExitCooldown     = 0;

	initMap(level);
	recreateWorldPickupSprites(map->getTileSize());
	spawnEntities(level);

	const int ts = map->getTileSize();
	camZoom = (ts <= 16) ? 2.0f : 1.5f;
	camX = 0.f;
	camY = 0.f;
}

void Scene::initMap(int level)
{
	string levelFile;
	switch (level) {
	case 1: levelFile = "levels/level_1.txt"; break;
	case 2: levelFile = "levels/level_2.txt"; break;
	case 3: levelFile = "levels/level_3.txt"; break;
	case 4: levelFile = "levels/level_4.txt"; break;
	default: levelFile = "levels/level_5.txt"; break;
	}

	mainMap = TileMap::createTileMap(levelFile, glm::vec2(SCREEN_X, SCREEN_Y), texProgram);
	map     = mainMap;

	tileBlocks.clear();
	tileCliffs.clear();
	tileLadders.clear();
	tileDoors.clear();
	tileJumps.clear();
	tileWarps.clear();
	tileElevators.clear();
	elevatorPairs.clear();
	warpTiles.clear();

	if (level == 1) {
		tileBlocks    = {0, 3, 4, 7, 10, 13, 14, 15, 25, 28, 35, 37, 42, 62};
		tileCliffs    = {2, 8, 12, 17};
		tileLadders   = {9, 34};
		tileWarps     = {36, 61};
		tileJumps     = {11};

		warpTiles = { glm::ivec2(15, 18), glm::ivec2(17, 7) };
		doors = {
			{ glm::ivec2(17, 18), DOOR_ENTRY,  true  },
			{ glm::ivec2(14, 4),  DOOR_EXIT,   false },
			{ glm::ivec2(6, 13), DOOR_SECRET, false },
			{ glm::ivec2(6, 16), DOOR_SECRET, false },
		};
	}
	else if (level == 2) {
		tileBlocks  = {0, 3, 4, 5, 6, 7, 21, 24, 46, 48, 49};
		tileLadders = {51, 30};
		tileCliffs  = {42, 44};

		doors = {
			{ glm::ivec2(17, 18), DOOR_ENTRY,  true  },
			{ glm::ivec2(14, 4),  DOOR_EXIT,   false },
			{ glm::ivec2(5, 6), DOOR_SECRET, false },
			{ glm::ivec2(6,  16), DOOR_SECRET, false },
		};
	}
	else if (level == 3) {
		tileBlocks  = {0, 3, 6, 7, 8, 10, 13, 30, 31, 33, 35, 41, 42, 44, 45, 64, 70, 71, 72, 73};
		tileLadders = {39};
		tileJumps   = {2};
		tileWarps   = {69};
		tileCliffs  = {4, 5, 11, 12, 14, 15, 46};

		warpTiles = { glm::ivec2(14, 11), glm::ivec2(17, 13) };
		doors = {
			{ glm::ivec2(16, 19), DOOR_ENTRY,  true  },
			{ glm::ivec2(2, 8),  DOOR_EXIT,   false },
			{ glm::ivec2(13, 6), DOOR_SECRET, false },
			{ glm::ivec2(2, 15), DOOR_SECRET, false },
		};
	}
	else if (level == 4) {
		tileBlocks = {2, 3, 21};
		tileJumps  = {23};
		tileWarps  = {20};
		tileCliffs = {7, 8, 28};

		warpTiles  = { glm::ivec2(8, 10), glm::ivec2(9, 19) };
		doors = {
			{ glm::ivec2(2, 19), DOOR_ENTRY,  true  },
			{ glm::ivec2(2, 12),  DOOR_EXIT,  false },
			{ glm::ivec2(13, 7), DOOR_SECRET, false },
			{ glm::ivec2(15, 13), DOOR_SECRET, false },
		};
	}
	else if (level == 5) {
		tileBlocks  = {2, 3, 4, 6, 7, 8, 31, 32, 33, 34, 35, 62, 63, 91};
		tileLadders = {38, 60, 90};
		tileJumps   = {64};
		tileWarps   = {36, 61, 65};
		tileCliffs  = {66, 67, 68, 69, 71, 92};

		warpTiles   = { glm::ivec2(7, 11), glm::ivec2(13, 14) };
		doors = {
			{ glm::ivec2(18, 14), DOOR_ENTRY,  true  },
			{ glm::ivec2(15, 6),  DOOR_EXIT,   false },
			{ glm::ivec2(13, 19), DOOR_SECRET, false },
			{ glm::ivec2(3, 11), DOOR_SECRET, false },
		};
	}
	applyTileTypes();
}

void Scene::applyTileTypes() {
	for (int id : tileBlocks)    map->setTileType(id, TILE_BLOCK);
	for (int id : tileCliffs)    map->setTileType(id, TILE_CLIFF);
	for (int id : tileLadders)   map->setTileType(id, TILE_LADDER);
	for (int id : tileDoors)     map->setTileType(id, TILE_DOOR);
	for (int id : tileJumps)     map->setTileType(id, TILE_JUMP);
	for (int id : tileWarps)     map->setTileType(id, TILE_WARP);
}

void Scene::recreateWorldPickupSprites(int ts) {
	if (keyWorldSprite) {
		delete keyWorldSprite;
		keyWorldSprite = NULL;
	}
	if (itemSprite) { 
		delete itemSprite;     
		itemSprite = NULL; 
	}

	keyWorldPixelSize = (ts * 5) / 8;
	if (keyWorldPixelSize < 10)
		keyWorldPixelSize = 10;
	if (keyWorldPixelSize > ts - 2)
		keyWorldPixelSize = ts - 2;
	// Allow up to full tile so +25% below is not stuck at ts-2 (was invisible before)
	keyWorldPixelSize = std::min(keyWorldPixelSize * 2, ts);
	if (keyWorldPixelSize < 10)
		keyWorldPixelSize = std::min(10, ts);
	// Same +25% as HUD key; cap at tile size (pickup hitbox is still ts×ts in update())
	keyWorldPixelSize = std::min((keyWorldPixelSize * 5 + 3) / 4, ts);
	if (keyWorldPixelSize < 10)
		keyWorldPixelSize = std::min(10, ts);

	const glm::vec2 cell = itemAtlasCellUv;
	const int ac = itemAtlasCols;
	const int ar = itemAtlasRows;
	keyWorldSprite = Sprite::createSprite(glm::ivec2(keyWorldPixelSize, keyWorldPixelSize),
	                                    cell, &itemTex, &texProgram);
	keyWorldSprite->setNumberAnimations(1);
	keyWorldSprite->setAnimationSpeed(0, 1);
	keyWorldSprite->addKeyframe(0, atlasUv1Based(3, ac, ar, cell));
	keyWorldSprite->changeAnimation(0);

	itemSprite = Sprite::createSprite(glm::ivec2(ts * 2, ts * 2), cell, &itemTex, &texProgram);
	itemSprite->setNumberAnimations(5);  // 0-3 = items, 4 = chest
	for (int i = 0; i < 4; ++i) {
		itemSprite->setAnimationSpeed(i, 1);
		itemSprite->addKeyframe(i, atlasUv1Based(itemAnimToAtlasNum(i), ac, ar, cell));
	}
	itemSprite->setAnimationSpeed(4, 1);
	itemSprite->addKeyframe(4, atlasUv1Based(9, ac, ar, cell));  // chest = atlas block 9
	itemSprite->changeAnimation(0);

	// Chest sprite (smaller than items — 1.5× tile)
	if (chestSprite) { delete chestSprite; chestSprite = NULL; }
	int chestSz = (ts * 3) / 2;  // 24px when ts=16
	chestSprite = Sprite::createSprite(glm::ivec2(chestSz, chestSz), cell, &itemTex, &texProgram);
	chestSprite->setNumberAnimations(1);
	chestSprite->setAnimationSpeed(0, 1);
	chestSprite->addKeyframe(0, atlasUv1Based(9, ac, ar, cell));
	chestSprite->changeAnimation(0);

	// Bomb sprites
	if (bombLitSprite)  { delete bombLitSprite;  bombLitSprite  = NULL; }
	if (bombSmokeSprite){ delete bombSmokeSprite; bombSmokeSprite = NULL; }

	bombLitSprite = Sprite::createSprite(glm::ivec2(ts * 2, ts * 2), cell, &itemTex, &texProgram);
	bombLitSprite->setNumberAnimations(1);
	bombLitSprite->setAnimationSpeed(0, 1);
	bombLitSprite->addKeyframe(0, atlasUv1Based(7, ac, ar, cell));  // lit bomb = atlas block 7
	bombLitSprite->changeAnimation(0);

	bombSmokeSprite = Sprite::createSprite(glm::ivec2(ts * 2, ts * 2), cell, &itemTex, &texProgram);
	bombSmokeSprite->setNumberAnimations(1);
	bombSmokeSprite->setAnimationSpeed(0, 8);  // 8 fps for smoke
	for (int i = 11; i <= 18; ++i)
		bombSmokeSprite->addKeyframe(0, atlasUv1Based(i, ac, ar, cell));
	bombSmokeSprite->changeAnimation(0);

	// Rebuild door sprite at the correct tile size
	if (doorSprite) { delete doorSprite; doorSprite = NULL; }
	doorSprite = Sprite::createSprite(glm::ivec2(ts, ts), DOOR_CELL_UV, &doorTex, &texProgram);
	doorSprite->setNumberAnimations(2);
	doorSprite->setAnimationSpeed(0, 1);
	doorSprite->addKeyframe(0, DOOR_UV_CLOSED);
	doorSprite->setAnimationSpeed(1, 1);
	doorSprite->addKeyframe(1, DOOR_UV_OPEN);
	doorSprite->changeAnimation(0);
}

void Scene::spawnEntities(int level)
{
	const int ts = map->getTileSize();

	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram, ts);
	player->setTileMap(map);

	auto spawnEnemy = [&](int idx, EnemyTypes type, int tileX, int tileY) {
		enemies[idx] = new Enemy();
		enemies[idx]->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram, type, ts);
		enemies[idx]->setPosition(glm::vec2(tileX * ts, tileY * ts));
		enemies[idx]->setTileMap(map);
		activeEnemies = idx + 1;
	};

	auto setSpawn = [&](int tileX, int tileY) {
		spawnPos = glm::ivec2(tileX * ts, tileY * ts);
		player->setPosition(glm::vec2(spawnPos));
	};

	const int kz = keyWorldPixelSize;

	switch (level) {
	case 1:
		setSpawn(17, 17); 
		spawnEnemy(0, PIOLIN, 16, 3);
		enemies[0]->setPatrolRange(4 * ts);
		keysRequired = 3;
		//Colocacion de llaves
		keys[0] = { keyPickupPos(ts, kz, 7,  5),  false };
		keys[1] = { keyPickupPos(ts, kz, 13, 10), false };
		keys[2] = { keyPickupPos(ts, kz, 2,  3), false };
		// Items on mid platforms and main floor for coverage
		items[0] = { ITEM_BOMB,   itemPickupPos(ts, 8,  7),  false };
		items[1] = { ITEM_BOOTS,  itemPickupPos(ts, 4,  13), false };
		items[2] = { ITEM_CLOCK,  itemPickupPos(ts, 16, 15), false };
		itemCount = 3;
		break;

	case 2:
		setSpawn(17, 17);
		spawnEnemy(0, LUCAS,  3,  5);
		spawnEnemy(1, PIOLIN, 10,  7);
		keysRequired = 3;

		keys[0] = { keyPickupPos(ts, kz, 7,  4),  false };
		keys[1] = { keyPickupPos(ts, kz, 9,  14), false };
		keys[2] = { keyPickupPos(ts, kz, 16, 14), false };
		items[0] = { ITEM_BOMB,   itemPickupPos(ts, 15, 4),  false };
		items[1] = { ITEM_BOOTS,  itemPickupPos(ts, 10, 16), false };
		items[2] = { ITEM_CLOCK,  itemPickupPos(ts, 17, 16), false };
		itemCount = 3;
		break;

	case 3:
		setSpawn(16, 18);
		spawnEnemy(0, PIOLIN, 4,  16);
		spawnEnemy(1, GHOST,  16, 13);
		spawnEnemy(2, LUCAS,  6, 7); 
		keysRequired = 3;
		keys[0] = { keyPickupPos(ts, kz, 7,  8),  false };
		keys[1] = { keyPickupPos(ts, kz, 14, 8),  false };
		keys[2] = { keyPickupPos(ts, kz, 2,  19), false };
		items[0] = { ITEM_WEIGHT, itemPickupPos(ts, 11, 15), false };  // elevated platform
		items[1] = { ITEM_BOMB,   itemPickupPos(ts, 12, 15), false };
		items[2] = { ITEM_BOOTS,  itemPickupPos(ts, 15, 19), false };
		items[3] = { ITEM_CLOCK,  itemPickupPos(ts, 16, 8),  false };
		itemCount = 4;
		break;

	case 4:
		setSpawn(2, 18);
		spawnEnemy(0, LUCAS,    3,  13);
		spawnEnemy(1, TASMANIA, 10,  7);
		keysRequired = 3;

		keys[0] = { keyPickupPos(ts, kz, 9,  7),  false };
		keys[1] = { keyPickupPos(ts, kz, 5,  13), false };
		keys[2] = { keyPickupPos(ts, kz, 17, 19), false };
		// Items on bottom floor and mid platforms (row16)
		items[0] = { ITEM_BOMB,   itemPickupPos(ts, 14, 13), false };
		items[1] = { ITEM_BOOTS,  itemPickupPos(ts, 11, 16), false };
		items[2] = { ITEM_WEIGHT,  itemPickupPos(ts, 2,  16), false };
		itemCount = 3;
		break;

	default: // nivel 5
		setSpawn(18, 13);
		spawnEnemy(0, GHOST,    17,  7);
		spawnEnemy(1, TASMANIA,  9, 12);
		spawnEnemy(2, LUCAS,     4, 10);
		keysRequired = 3;

		keys[0] = { keyPickupPos(ts, kz, 17, 9),  false };
		keys[1] = { keyPickupPos(ts, kz, 4,  11), false };
		keys[2] = { keyPickupPos(ts, kz, 9,  19), false };
		// Items spread: bottom (weight/clock), mid-right platform (bomb), mid-left (boots)
		items[0] = { ITEM_BOMB,   itemPickupPos(ts, 16, 14), false };
		items[1] = { ITEM_BOOTS,  itemPickupPos(ts, 6,  11), false };
		items[2] = { ITEM_CLOCK,  itemPickupPos(ts, 13, 19), false };
		itemCount = 3;
		break;
	}

	// Extract ITEM_WEIGHT entries into the weights array (they are pushable, not pickable)
	weightCount = 0;
	for (int i = 0; i < itemCount; ++i) {
		if (items[i].type == ITEM_WEIGHT && weightCount < MAX_WEIGHTS) {
			weights[weightCount].pos       = items[i].pos;
			weights[weightCount].pos.y    -= ts;  // adjust for 2-tile height (itemPickupPos is for 1-tile items)
			weights[weightCount].active    = true;
			weights[weightCount].falling   = false;
			weights[weightCount].fallSpeed = 0.f;
			weightCount++;
			items[i].collected = true;  // mark as collected so it won't render/pickup as normal item
		}
	}
}

bool Scene::checkCollision(const glm::ivec2 &posA, const glm::ivec2 &posB, const glm::ivec2 &sizeA, const glm::ivec2 &sizeB) const {
	return (posA.x <= posB.x + sizeB.x && posA.x + sizeA.x >= posB.x && posA.y <= posB.y + sizeB.y && posA.y + sizeA.y >= posB.y);
}

void Scene::update(int deltaTime)
{
	currentTime += deltaTime;

	if (enemiesFrozen) {
		freezeTimer -= deltaTime;
		if (freezeTimer <= 0.f) enemiesFrozen = false;
	}

	if (respawnTimer > 0.f) {
		respawnTimer -= deltaTime;
		if (respawnTimer <= 0.f) player->setPosition(glm::vec2(spawnPos));
		return;
	}

	player->update(deltaTime);

	const glm::ivec2 playerPos  = player->getPosition();
	const glm::ivec2 playerSize = player->getSpriteSize();
	const int ts = map->getTileSize();

	//warp floors
	if (playerWarpingOut && !player->isWarpDisappearing()) {
		player->startWarpAppear(warpDestPos);
		playerWarpingOut = false;
	}
	else if (warpTiles.size() == 2 && !player->isWarping()) {
		glm::ivec2 belowTile(
			(playerPos.x + playerSize.x / 2) / ts,
			(playerPos.y + playerSize.y) / ts
		);
		if (Game::instance().getKey(GLFW_KEY_UP) || Game::instance().getKey(GLFW_KEY_DOWN)) {
			for (int i = 0; i < 2; ++i) {
				if (belowTile == warpTiles[i]) {
					int dest = 1 - i;
					warpDestPos = glm::ivec2(
						warpTiles[dest].x * ts,
						warpTiles[dest].y * ts - ts
					);
					player->startWarpDisappear();
					playerWarpingOut = true;
					Game::instance().playSfx(GameSfx::Warp);
					break;
				}
			}
		}
	}

	//gestion acceso a salas secretas y las puertas
	if (secretExitCooldown > 0) secretExitCooldown -= deltaTime;
	if (!inSecretRoom) {
		if (secretEnterPending) {
			secretAnimTimer -= deltaTime;
			if (secretAnimTimer <= 0)
				beginEnterSecretRoom();
		}
		else if (!playerWarpingOut && !playerEnteringElevator
		         && Game::instance().getKey(GLFW_KEY_UP)) {
			int playerTileX = (playerPos.x + playerSize.x / 2) / ts;
			int playerTileY = (playerPos.y + playerSize.y) / ts;
			for (auto &door : doors) {
				if (!door.open && door.tilePos == glm::ivec2(playerTileX, playerTileY)) {
					if (door.type == DOOR_EXIT && keysCollected < keysRequired)
						break;   // no tiene todas las llaves — puerta sigue cerrada
					door.open = true;
					if (door.type == DOOR_EXIT) {
						if (levelIndex == 5) {
							// Level 5: enter final chest room instead of winning
							secretReturnPos    = playerPos;
							secretEnterPending = true;
							secretAnimTimer    = 400;
							secretDoorIndex    = -1;  // special: final room
						} else {
							levelComplete = true;
						}
					} else if (door.type == DOOR_SECRET) {
						secretReturnPos    = playerPos;
						secretEnterPending = true;
						secretAnimTimer    = 400;
						// Find which secret door index this is (0-based among DOOR_SECRET)
						secretDoorIndex = 0;
						for (const auto &d : doors) {
							if (&d == &door) break;
							if (d.type == DOOR_SECRET) secretDoorIndex++;
						}
					} else if (door.type == DOOR_ENTRY && levelIndex > 1) {
						levelBack = true;
					}
					break;
				}
			}
		}
	} else {
		// Chest opening timer (must run before interaction to avoid same-frame completion)
		if (chestOpening) {
			chestOpenTimer -= deltaTime;
			if (chestOpenTimer <= 0.f && !player->isOpeningChest()) {
				chestOpening = false;
				secretLoot.collected = true;
				secretLootTaken = true;
				if (levelIndex == 5 && secretDoorIndex == -1)
					levelComplete = true;
			}
		}

		// Sala secreta: interact with loot
		if (!secretLootTaken && !secretLoot.collected && !chestOpening) {
			const glm::ivec2 pickupSize(ts * 2, ts * 2);  // match sprite render size
			if (checkCollision(playerPos, secretLoot.pos, playerSize, pickupSize)) {
				if (secretIsChest) {
					// Chest: need to press UP to open
					if (Game::instance().getKey(GLFW_KEY_UP)) {
						chestOpening   = true;
						chestOpenTimer = 1200.f;  // 1.2s for animation
						player->startOpenChest();
					}
				} else {
					// Normal item: pick up on contact
					secretLoot.collected = true;
					secretLootTaken = true;
					hasItem     = true;
					carriedItem = secretLoot.type;
				}
			}
		}
		// Sala secreta: puerta de salida en (SECRET_DOOR_COL, SECRET_DOOR_ROW)
		if (secretExitCooldown <= 0 && Game::instance().getKey(GLFW_KEY_UP)) {
			int playerTileX = (playerPos.x + playerSize.x / 2) / ts;
			int playerTileY = (playerPos.y + playerSize.y) / ts;
			if (playerTileX == SECRET_DOOR_COL && playerTileY == SECRET_DOOR_ROW)
				exitSecretRoom();
		}
	}

	// ---- Enemigos, items, llaves (solo nivel principal) ----
	if (!inSecretRoom) {
		const glm::ivec2 enemySize(ts, ts);
		const glm::ivec2 pickupSize(ts, ts);

		for (int i = 0; i < activeEnemies; ++i) {
			if (!enemies[i]->isAlive()) continue;
			enemies[i]->setTarget(playerPos);
			if (!enemiesFrozen) enemies[i]->update(deltaTime);

			glm::ivec2 pHitPos  = playerPos;
			glm::ivec2 pHitSize = playerSize;
			if (map->isOnLadder(pHitPos, pHitSize)) {
				pHitPos.x  -= 6;
				pHitSize.x += 12;
			}
			if (!player->isGodMode() && !player->isHurt() &&
			    checkCollision(pHitPos, enemies[i]->getPosition(), pHitSize, enemySize)) {
				player->dies();
				if (player->getLives() <= 0) gameOver = true;
				else respawnTimer = 1500.f;
			}
		}

		for (int i = 0; i < itemCount; ++i) {
			if (items[i].collected) continue;
			if (items[i].type == ITEM_WEIGHT) continue;  // weights are pushable, not pickable
			if (checkCollision(playerPos, items[i].pos, playerSize, pickupSize)) {
				items[i].collected = true;
				hasItem     = true;
				carriedItem = items[i].type;
				Game::instance().playSfx(GameSfx::ItemPickup);
			}
		}

		for (int i = 0; i < keysRequired; ++i) {
			if (keys[i].collected) continue;
			if (checkCollision(playerPos, keys[i].pos, playerSize, pickupSize)) {
				keys[i].collected = true;
				keysCollected++;
				Game::instance().playSfx(GameSfx::KeyPickup);
			}
		}

		if (hasItem && Game::instance().getKey(GLFW_KEY_Z)) {
			hasItem = false;
			switch (carriedItem) {
			case ITEM_CLOCK:
				enemiesFrozen = true;
				freezeTimer   = 5000.f;
				Game::instance().playSfx(GameSfx::Freeze);
				break;
			case ITEM_BOOTS:
				player->applyBoots(5000);
				Game::instance().playSfx(GameSfx::Boots);
				break;
			case ITEM_BOMB:
				// Place lit bomb on ground at player position
				bombActive    = true;
				bombPos       = playerPos;
				bombTimer     = 2000.f;  // 2 seconds fuse
				bombExploding = false;
				bombSmokeTimer = 0.f;
				break;
			case ITEM_WEIGHT:
				// Weight is pushable, not usable from inventory
				break;
			}
		}

		// ---- Bomb timer & explosion ----
		if (bombActive && !bombExploding) {
			bombTimer -= deltaTime;
			if (bombTimer <= 0.f) {
				bombExploding  = true;
				bombSmokeTimer = 0.f;
				Game::instance().playSfx(GameSfx::Explosion);
				// Kill enemies in blast radius (3 tiles)
				for (int i = 0; i < activeEnemies; ++i) {
					if (!enemies[i]->isAlive()) continue;
					glm::ivec2 diff = enemies[i]->getPosition() - bombPos;
					if (abs(diff.x) < 3 * ts && abs(diff.y) < 3 * ts)
						enemies[i]->kill();
				}
			}
		}
		if (bombExploding) {
			bombSmokeTimer += deltaTime;
			if (bombSmokeSprite) bombSmokeSprite->update(deltaTime);
			if (bombSmokeTimer >= 1000.f) {  // 1s smoke animation
				bombActive    = false;
				bombExploding = false;
			}
		}

		// ---- Pushable weights ----
		for (int w = 0; w < weightCount; ++w) {
			if (!weights[w].active) continue;

			// Weight collision box = full sprite (2×2 tiles)
			const glm::ivec2 wPos  = weights[w].pos;
			const glm::ivec2 wSize(ts * 2, ts * 2);

			if (!weights[w].falling) {
				// Push: resolve overlap by moving weight away from player
				if (checkCollision(playerPos, wPos, playerSize, wSize)) {
					int pCx = playerPos.x + playerSize.x / 2;
					int wCx = wPos.x + wSize.x / 2;

					// Horizontal overlap amount
					int overlapLeft  = (playerPos.x + playerSize.x) - wPos.x;
					int overlapRight = (wPos.x + wSize.x) - playerPos.x;
					// Vertical overlap amount
					int overlapTop   = (playerPos.y + playerSize.y) - wPos.y;
					int overlapBot   = (wPos.y + wSize.y) - playerPos.y;

					// Only push horizontally if horizontal overlap is smaller (side collision)
					int minOverlapH = std::min(overlapLeft, overlapRight);
					int minOverlapV = std::min(overlapTop, overlapBot);

					if (minOverlapH < minOverlapV) {
						int pushDir = (pCx < wCx) ? 1 : -1;
						int pushAmt = (pushDir > 0) ? overlapLeft : overlapRight;

						glm::ivec2 newPos = weights[w].pos;
						newPos.x += pushDir * pushAmt;

						// Check wall at new position
						int checkX = (pushDir > 0) ? (newPos.x + wSize.x) : (newPos.x - 1);
						int checkY = newPos.y + wSize.y - 1;
						TileType t = map->tileTypeAt(checkX, checkY);
						if (t != TILE_BLOCK) {
							weights[w].pos = newPos;
						}
					}
				}

				// Check if weight has ground beneath it
				int belowY  = wPos.y + wSize.y;
				int centerX = wPos.x + ts;  // center of sprite
				TileType tBelow = map->tileTypeAt(centerX, belowY);
				if (tBelow != TILE_BLOCK && tBelow != TILE_JUMP && tBelow != TILE_WARP) {
					weights[w].falling   = true;
					weights[w].fallSpeed = 0.f;
				}
			}

			if (weights[w].falling) {
				weights[w].fallSpeed += 0.15f * deltaTime;  // gravity
				if (weights[w].fallSpeed > 4.f * ts) weights[w].fallSpeed = 4.f * ts;
				weights[w].pos.y += int(weights[w].fallSpeed);

				// Check ground collision
				int belowY  = weights[w].pos.y + wSize.y;
				int centerX = weights[w].pos.x + ts;
				TileType tBelow = map->tileTypeAt(centerX, belowY);
				if (tBelow == TILE_BLOCK || tBelow == TILE_JUMP || tBelow == TILE_WARP) {
					int belowRow = belowY / ts;
					weights[w].pos.y = (belowRow * ts) - wSize.y;  // snap sprite bottom to ground top
					weights[w].falling   = false;
					weights[w].fallSpeed = 0.f;
				}

				// Deactivate if fell off map
				if (weights[w].pos.y > map->getMapHeight() * ts)
					weights[w].active = false;
			}

			// Check if weight hits an enemy (pushed or falling)
			for (int e = 0; e < activeEnemies; ++e) {
				if (!enemies[e]->isAlive()) continue;
				glm::ivec2 ePos = enemies[e]->getPosition();
				glm::ivec2 eSize(ts, ts);
				if (checkCollision(weights[w].pos, ePos, wSize, eSize)) {
					enemies[e]->kill();
					Game::instance().playSfx(GameSfx::Explosion);
					for (int x = 0; x < MAX_EXPLOSIONS; ++x) {
						if (!explosions[x].active) {
							explosions[x].active = true;
							explosions[x].pos    = ePos;
							explosions[x].timer  = 0.f;
							break;
						}
					}
				}
			}
		}

		// ---- Explosion effects (from weight kills, etc.) ----
		for (int i = 0; i < MAX_EXPLOSIONS; ++i) {
			if (!explosions[i].active) continue;
			explosions[i].timer += deltaTime;
			if (explosions[i].timer >= 1000.f)
				explosions[i].active = false;
		}
	}
}

void Scene::render()
{
	const float mapW = float(map->getMapWidth()  * map->getTileSize());
	const float mapH = float(map->getMapHeight() * map->getTileSize());

	const float tsCam = float(map->getTileSize());
	glm::ivec2  pp    = player->getPosition();
	float px = float(SCREEN_X + pp.x) + tsCam * 0.5f;
	float py = float(SCREEN_Y + pp.y) + tsCam * 0.5f;
	camX = px - CAMERA_W * 0.5f;
	camY = py - CAMERA_H * 0.5f;
	camX = std::max(0.f, std::min(camX, mapW - CAMERA_W));
	camY = std::max(0.f, std::min(camY, mapH - CAMERA_H));

	projection = glm::ortho(camX, camX + CAMERA_W, camY + CAMERA_H, camY);
	glm::mat4 modelview = glm::mat4(1.f);

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);

	map->render();

	const int ts = map->getTileSize();

	// Dibujar puertas (solo en nivel principal)
	if (!inSecretRoom && doorSprite) {
		texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
		for (auto &door : doors) {
			doorSprite->changeAnimation(door.open ? 1 : 0);
			doorSprite->setPosition(glm::vec2(door.tilePos.x * ts, door.tilePos.y * ts));
			doorSprite->render();
		}
	}

	// Puerta de salida de la sala secreta
	if (inSecretRoom && doorSprite) {
		texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
		doorSprite->changeAnimation(0);
		doorSprite->setPosition(glm::vec2(SECRET_DOOR_COL * ts, SECRET_DOOR_ROW * ts));
		doorSprite->render();
	}

	if (!inSecretRoom) {
		for (int i = 0; i < keysRequired; ++i) {
			if (keys[i].collected) continue;
			texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
			keyWorldSprite->setPosition(glm::vec2(keys[i].pos));
			keyWorldSprite->render();
		}
		texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);

		for (int i = 0; i < itemCount; ++i) {
			if (items[i].collected) continue;
			itemSprite->changeAnimation(int(items[i].type));
			itemSprite->setPosition(glm::vec2(items[i].pos));
			itemSprite->render();
		}

		// Render pushable weights
		for (int w = 0; w < weightCount; ++w) {
			if (!weights[w].active) continue;
			texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
			itemSprite->changeAnimation(int(ITEM_WEIGHT));
			itemSprite->setPosition(glm::vec2(weights[w].pos));
			itemSprite->render();
		}
	}

	// Render secret room loot (hide chest once opening animation starts)
	if (inSecretRoom && !secretLootTaken && !secretLoot.collected && !chestOpening) {
		texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
		if (secretIsChest && chestSprite) {
			chestSprite->setPosition(glm::vec2(secretLoot.pos));
			chestSprite->render();
		} else if (itemSprite) {
			itemSprite->changeAnimation(int(secretLoot.type));
			itemSprite->setPosition(glm::vec2(secretLoot.pos));
			itemSprite->render();
		}
	}

	// Render active bomb (lit or smoke)
	if (bombActive && !inSecretRoom) {
		texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
		if (bombExploding && bombSmokeSprite) {
			bombSmokeSprite->setPosition(glm::vec2(bombPos));
			bombSmokeSprite->render();
		} else if (bombLitSprite) {
			bombLitSprite->setPosition(glm::vec2(bombPos));
			bombLitSprite->render();
		}
	}

	// Render explosion effects (weight kills etc.)
	for (int i = 0; i < MAX_EXPLOSIONS; ++i) {
		if (!explosions[i].active || !bombSmokeSprite) continue;
		texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
		bombSmokeSprite->setPosition(glm::vec2(explosions[i].pos));
		bombSmokeSprite->render();
	}

	player->render();

	if (!inSecretRoom) {
		for (int i = 0; i < activeEnemies; ++i)
			if (enemies[i] && enemies[i]->isAlive())
				enemies[i]->render();
	}

	if (player->isGodMode()) {
		const glm::ivec2 pp = player->getPosition();
		const glm::ivec2 ps = player->getSpriteSize();
		const float cx = float(pp.x) + float(ps.x) * 0.5f;
		const float cy = float(pp.y) + float(ps.y) * 0.5f;
		const float orbitR = float(ps.x) * 0.9f;
		const float t = currentTime * 0.003f;
		const float ah = float(GOD_AURA_PIXEL_SIZE) * 0.5f;
		texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
		for (int i = 0; i < 3; ++i) {
			float ang = t + float(i) * (2.f * float(M_PI) / 3.f);
			glm::vec2 orb(cx + std::cos(ang) * orbitR - ah,
			              cy + std::sin(ang) * orbitR - ah);
			godAuraSprites[i]->setPosition(orb);
			godAuraSprites[i]->render();
		}
	}
	renderHUD();
}

void Scene::renderHUD()
{
	if (!player) return;

	glm::mat4 hudProj  = glm::ortho(0.f, 640.f, 480.f, 0.f);
	glm::mat4 identity = glm::mat4(1.f);

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", hudProj);
	texProgram.setUniformMatrix4f("modelview", identity);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	for (int i = 0; i < player->getLives(); ++i) {
		heartSprite->setPosition(glm::vec2(HUD_MARGIN + i * HUD_SPACING, HUD_MARGIN));
		heartSprite->render();
	}

	{
		const float ky = HUD_MARGIN + HUD_SPACING;
		const float keyTextPixel = 4.f;
		const float textY = ky + 2.f;
		// Vertically center key icon with digits, then nudge up slightly
		const float keyIconY = textY + (7.f * keyTextPixel) * 0.5f - float(KEY_HUD_ICON_SIZE) * 0.5f
		                       - KEY_HUD_ICON_NUDGE_UP;
		texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
		keyHudSprite->setPosition(glm::vec2(HUD_MARGIN, keyIconY));
		keyHudSprite->render();
		char buf[8];
		snprintf(buf, sizeof(buf), "%d", keysCollected);
		Game::instance().renderBitmapTextHudOutlined(buf,
		                                             HUD_MARGIN + float(KEY_HUD_ICON_SIZE) + 6.f,
		                                             textY,
		                                             keyTextPixel,
		                                             1.f, 1.f, 1.f);
	}

	if (hasItem) {
		texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
		itemHudSprite->changeAnimation(int(carriedItem));
		itemHudSprite->setPosition(glm::vec2(HUD_MARGIN, 480.f - HUD_MARGIN - ITEM_HUD_ICON_SIZE));
		itemHudSprite->render();
	}

	if (player->isGodMode())
	{
		texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
		godHudSprite->setPosition(glm::vec2(640.f - HUD_MARGIN - ITEM_HUD_ICON_SIZE,
		                                    480.f - HUD_MARGIN - ITEM_HUD_ICON_SIZE));
		godHudSprite->render();
	}

	texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	glDisable(GL_BLEND);
	texProgram.setUniformMatrix4f("projection", projection);
}

void Scene::setGodMode(bool g) {
	if (player) player->setGodMode(g);
}

void Scene::killAllEnemies() {
	for (int i = 0; i < activeEnemies; ++i)
		if (enemies[i]) enemies[i]->kill();
}

void Scene::collectAllKeys() {
	for (int i = 0; i < keysRequired; ++i) {
		if (!keys[i].collected) {
			keys[i].collected = true;
			keysCollected++;
		}
	}
}

// ---- Secret room ----

bool Scene::playerOnSecretDoor(const glm::ivec2 &playerPos, const glm::ivec2 &playerSize) const {
	const int ts = map->getTileSize();
	int px = playerPos.x + playerSize.x / 2;
	int py = playerPos.y + playerSize.y / 2;
	for (const auto &door : doors) {
		if (door.type != DOOR_SECRET) continue;
		int dx = door.tilePos.x * ts;
		int dy = door.tilePos.y * ts;
		if (px >= dx && px < dx + ts && py >= dy && py < dy + ts)
			return true;
	}
	return false;
}

void Scene::beginEnterSecretRoom() {
	if (secretMap) { delete secretMap; secretMap = NULL; }

	string secretFile;
	glm::ivec2 secretSpawn;
	switch (levelIndex) {
	case 1:  secretFile = "secrets/secret1.txt"; secretSpawn = {8, 11}; break;
	case 2:  secretFile = "secrets/secret2.txt"; secretSpawn = {5, 11}; break;
	case 3:  secretFile = "secrets/secret3.txt"; secretSpawn = {5, 11}; break;
	case 4:  secretFile = "secrets/secret4.txt"; secretSpawn = {5, 11}; break;
	case 5:  secretFile = "secrets/secret5.txt"; secretSpawn = {5, 11}; break;
	default: secretFile = "secrets/secret1.txt"; secretSpawn = {8, 11}; break;
	}
	secretMap = TileMap::createTileMap(secretFile, glm::vec2(SCREEN_X, SCREEN_Y), texProgram);

	for (int id : tileBlocks)    secretMap->setTileType(id, TILE_BLOCK);
	for (int id : tileCliffs)    secretMap->setTileType(id, TILE_CLIFF);
	for (int id : tileLadders)   secretMap->setTileType(id, TILE_LADDER);
	for (int id : tileJumps)     secretMap->setTileType(id, TILE_JUMP);
	for (int id : tileWarps)     secretMap->setTileType(id, TILE_WARP);
	for (int id : tileElevators) secretMap->setTileType(id, TILE_ELEVATOR);

	inSecretRoom = true;
	map = secretMap;
	player->setTileMap(map);

	const int ts = map->getTileSize();
	player->setPosition(glm::vec2(secretSpawn.x * ts, secretSpawn.y * ts));

	// Place loot in secret room
	secretLootTaken = false;
	secretLoot.collected = false;
	secretIsChest   = false;
	chestOpening    = false;
	chestOpenTimer  = 0.f;
	switch (levelIndex) {
	case 1:  secretLoot = { ITEM_BOMB,   itemPickupPos(ts, 10, 13), false }; break;
	case 2:  secretLoot = { ITEM_BOOTS,  itemPickupPos(ts, 10, 13), false }; break;
	case 3:  secretLoot = { ITEM_CLOCK,  itemPickupPos(ts, 10, 13), false }; break;
	case 4:  secretLoot = { ITEM_BOMB,   itemPickupPos(ts, 10, 13), false }; break;
	default: // level 5
		if (secretDoorIndex == -1) {
			// Final room (via exit door): chest on the right platform after stairs
			secretLoot    = { ITEM_CLOCK, glm::ivec2(13 * ts + 6, 11 * ts + 6), false };
			secretIsChest = true;
		} else {
			// Regular secret rooms: normal item
			secretLoot = { ITEM_BOOTS, itemPickupPos(ts, 10, 13), false };
		}
		break;
	}

	secretEnterPending = false;
	secretExitCooldown = 1000;
}

void Scene::finishEnterSecretRoom() {
	// Reservado para animaciones de transicion
}

void Scene::exitSecretRoom() {
	inSecretRoom = false;
	map = mainMap;
	player->setTileMap(map);
	player->setPosition(glm::vec2(secretReturnPos));

	if (secretMap) { delete secretMap; secretMap = NULL; }
	secretExitCooldown = 1000;
}

void Scene::markSecretDoorTiles(int /*level*/) {
	// Deteccion posicional — stub por compatibilidad
}
