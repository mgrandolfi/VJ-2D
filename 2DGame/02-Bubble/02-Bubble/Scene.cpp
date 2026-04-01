#include <iostream>
#include <cmath>
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
}

#define SCREEN_X 0
#define SCREEN_Y 16

#define CAMERA_W 200.f
#define CAMERA_H 150.f

#define HUD_ICON_SIZE 24.f
#define HUD_MARGIN    8.f

#define INIT_PLAYER_X_TILES 17
#define INIT_PLAYER_Y_TILES 17
#define HUD_SPACING  28.f

// Level 1
#define L1_PIOLIN_X      16
#define L1_PIOLIN_Y       3
#define L1_PIOLIN_RANGE   4
#define L1_LUCAS_X       16
#define L1_LUCAS_Y      3
#define L1_TASMANIA_X     3
#define L1_TASMANIA_Y    17

// Level 2
#define L2_LUCAS_X     3
#define L2_LUCAS_Y    13
#define L2_PIOLIN_X   10
#define L2_PIOLIN_Y    7

// Level 3
#define L3_PIOLIN_X    4
#define L3_PIOLIN_Y   16
#define L3_SILVESTRE_X 10
#define L3_SILVESTRE_Y 10

// Level 4
#define L4_LUCAS_X     3
#define L4_LUCAS_Y    13
#define L4_TASMANIA_X 10
#define L4_TASMANIA_Y  7

// Level 5
#define L5_SILVESTRE_X  3
#define L5_SILVESTRE_Y 13
#define L5_TASMANIA_X  10
#define L5_TASMANIA_Y   7
#define L5_LUCAS_X     15
#define L5_LUCAS_Y      2

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
	doorSprite     = NULL;
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
	secretExitCooldown = 0;
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

	// Atlas items/HUD (images/sprites/items.png, 160x64, cell 0.2x0.5)
	itemTex.loadFromFile("images/sprites/items.png", TEXTURE_PIXEL_FORMAT_RGBA);
	const glm::vec2 cell(0.2f, 0.5f);

	itemHudSprite = Sprite::createSprite(glm::ivec2(HUD_ICON_SIZE, HUD_ICON_SIZE),
	                                     cell, &itemTex, &texProgram);
	itemHudSprite->setNumberAnimations(4);
	for (int i = 0; i < 4; ++i) {
		itemHudSprite->setAnimationSpeed(i, 1);
		itemHudSprite->addKeyframe(i, glm::vec2(0.2f * float(i), 0.f));
	}
	itemHudSprite->changeAnimation(0);

	keyHudSprite = Sprite::createSprite(glm::ivec2(22, 22), cell, &itemTex, &texProgram);
	keyHudSprite->setNumberAnimations(1);
	keyHudSprite->setAnimationSpeed(0, 1);
	keyHudSprite->addKeyframe(0, glm::vec2(0.8f, 0.f));
	keyHudSprite->changeAnimation(0);

	godHudSprite = Sprite::createSprite(glm::ivec2(HUD_ICON_SIZE, HUD_ICON_SIZE),
	                                    cell, &itemTex, &texProgram);
	godHudSprite->setNumberAnimations(1);
	godHudSprite->setAnimationSpeed(0, 1);
	godHudSprite->addKeyframe(0, glm::vec2(0.f, 0.5f));
	godHudSprite->changeAnimation(0);

	for (int i = 0; i < 3; ++i) {
		godAuraSprites[i] = Sprite::createSprite(glm::ivec2(14, 14), cell, &itemTex, &texProgram);
		godAuraSprites[i]->setNumberAnimations(1);
		godAuraSprites[i]->setAnimationSpeed(0, 1);
		godAuraSprites[i]->addKeyframe(0, glm::vec2(0.2f * float(i + 1), 0.5f));
		godAuraSprites[i]->changeAnimation(0);
	}

	// Door sprite: images/items.png (320x320, 10x10 grid)
	// Row 2 col 0 = closed, row 2 col 1 = open
	// Size is rebuilt per level in recreateWorldPickupSprites(); 16x16 as placeholder.
	doorTex.loadFromFile("images/items.png", TEXTURE_PIXEL_FORMAT_RGBA);
	doorSprite = Sprite::createSprite(glm::ivec2(16, 16), DOOR_CELL_UV, &doorTex, &texProgram);
	doorSprite->setNumberAnimations(2);
	doorSprite->setAnimationSpeed(0, 1);
	doorSprite->addKeyframe(0, DOOR_UV_CLOSED);
	doorSprite->setAnimationSpeed(1, 1);
	doorSprite->addKeyframe(1, DOOR_UV_OPEN);
	doorSprite->changeAnimation(0);
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
	playerEnteringElevator = false;
	playerWarpingOut       = false;
	enemiesFrozen          = false;
	freezeTimer            = 0.f;
	respawnTimer           = 0.f;
	keysCollected          = 0;
	itemCount              = 0;
	hasItem                = false;
	currentTime            = 0.f;
	levelIndex             = level;
	inSecretRoom           = false;
	doors.clear();
	secretAnimTimer        = 0;
	secretEnterPending     = false;
	secretLootTaken        = true;
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
		tileBlocks    = {0, 3, 4, 7, 10, 13, 14, 15, 28, 37, 42, 62};
		tileCliffs    = {2, 8, 12, 17};
		tileLadders   = {9, 34};
		tileWarps     = {36, 61};
		tileJumps     = {11};

		warpTiles = { glm::ivec2(15, 18), glm::ivec2(17, 7) };

		// Puertas nivel 1: (col, row), tipo, abierta
		//   DOOR_ENTRY  → spawn del jugador, empieza abierta
		//   DOOR_EXIT   → requiere todas las llaves, pasa al siguiente nivel
		//   DOOR_SECRET → entra a la sala secreta
		doors = {
			{ glm::ivec2(INIT_PLAYER_X_TILES, INIT_PLAYER_Y_TILES + 1), DOOR_ENTRY,  true  },
			{ glm::ivec2(14, 4),  DOOR_EXIT,   false },
			{ glm::ivec2(6,  13), DOOR_SECRET, false },
			{ glm::ivec2(6,  16), DOOR_SECRET, false },
		};

	}
	else if (level == 2) {
		tileBlocks  = {0, 3, 4, 5, 6, 7, 21, 24, 46, 48, 49};
		tileLadders = {51, 30};
		tileCliffs  = {42, 44};
		tileElevators = {47, 8, 9};
	}
	else if (level == 3) {
		tileBlocks  = {0, 3, 6, 7, 8, 10, 13, 30, 31, 33, 35, 41, 42, 44, 45, 64, 70, 71, 72, 73};
		tileLadders = {39};
		tileJumps   = {2};
		tileWarps   = {69};
		tileCliffs  = {4, 5, 11, 12, 14, 15, 46};
		tileElevators = {66, 67, 68};
		warpTiles = { glm::ivec2(14, 11), glm::ivec2(17, 13) };
	}
	else if (level == 4) {
		tileBlocks = {2, 3, 21};
		tileJumps  = {23};
		tileWarps  = {20};
		tileCliffs = {7, 8, 28};
		warpTiles  = { glm::ivec2(15, 18), glm::ivec2(17, 7) };
	}
	else if (level == 5) {
		tileBlocks  = {2, 3, 4, 6, 7, 8, 31, 32, 33, 34, 35, 62, 63, 91};
		tileLadders = {38, 60, 90};
		tileJumps   = {64};
		tileWarps   = {36, 61, 65};
		tileCliffs  = {66, 67, 68, 69, 71, 92};
		warpTiles   = { glm::ivec2(15, 18), glm::ivec2(17, 7) };
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
	for (int id : tileElevators) map->setTileType(id, TILE_ELEVATOR);
}

void Scene::recreateWorldPickupSprites(int ts) {
	if (keyWorldSprite) { delete keyWorldSprite; keyWorldSprite = NULL; }
	if (itemSprite)     { delete itemSprite;     itemSprite     = NULL; }

	keyWorldPixelSize = (ts * 5) / 8;
	if (keyWorldPixelSize < 10)     keyWorldPixelSize = 10;
	if (keyWorldPixelSize > ts - 2) keyWorldPixelSize = ts - 2;

	const glm::vec2 cell(0.2f, 0.5f);
	keyWorldSprite = Sprite::createSprite(glm::ivec2(keyWorldPixelSize, keyWorldPixelSize),
	                                      cell, &itemTex, &texProgram);
	keyWorldSprite->setNumberAnimations(1);
	keyWorldSprite->setAnimationSpeed(0, 1);
	keyWorldSprite->addKeyframe(0, glm::vec2(0.8f, 0.f));
	keyWorldSprite->changeAnimation(0);

	itemSprite = Sprite::createSprite(glm::ivec2(ts, ts), cell, &itemTex, &texProgram);
	itemSprite->setNumberAnimations(4);
	for (int i = 0; i < 4; ++i) {
		itemSprite->setAnimationSpeed(i, 1);
		itemSprite->addKeyframe(i, glm::vec2(0.2f * float(i), 0.f));
	}
	itemSprite->changeAnimation(0);

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
		setSpawn(INIT_PLAYER_X_TILES, INIT_PLAYER_Y_TILES);
		spawnEnemy(0, PIOLIN, L1_PIOLIN_X, L1_PIOLIN_Y);
		enemies[0]->setPatrolRange(L1_PIOLIN_RANGE * ts);
		spawnEnemy(1, LUCAS, L1_LUCAS_X, L1_LUCAS_Y);
		keysRequired = 3;
		keys[0] = { keyPickupPos(ts, kz, 5, 15),  false };
		keys[1] = { keyPickupPos(ts, kz, 10, 11), false };
		keys[2] = { keyPickupPos(ts, kz, 15, 6),  false };
		items[0] = { ITEM_WEIGHT, itemPickupPos(ts, 3, 15),  false };
		items[1] = { ITEM_BOMB,   itemPickupPos(ts, 12, 11), false };
		items[2] = { ITEM_BOOTS,  itemPickupPos(ts, 8, 7),   false };
		items[3] = { ITEM_CLOCK,  itemPickupPos(ts, 16, 7),  false };
		itemCount = 4;
		break;

	case 2:
		setSpawn(18, 13);
		spawnEnemy(0, LUCAS,  L2_LUCAS_X,  L2_LUCAS_Y);
		spawnEnemy(1, PIOLIN, L2_PIOLIN_X, L2_PIOLIN_Y);
		keysRequired = 3;
		keys[0] = { keyPickupPos(ts, kz, 5, 14),  false };
		keys[1] = { keyPickupPos(ts, kz, 10, 8),  false };
		keys[2] = { keyPickupPos(ts, kz, 17, 3),  false };
		items[0] = { ITEM_WEIGHT, itemPickupPos(ts, 7, 14),  false };
		items[1] = { ITEM_BOMB,   itemPickupPos(ts, 15, 8),  false };
		items[2] = { ITEM_BOOTS,  itemPickupPos(ts, 4, 8),   false };
		items[3] = { ITEM_CLOCK,  itemPickupPos(ts, 12, 3),  false };
		itemCount = 4;
		break;

	case 3:
		setSpawn(18, 16);
		spawnEnemy(0, PIOLIN, L3_PIOLIN_X,    L3_PIOLIN_Y);
		spawnEnemy(1, GHOST,  L3_SILVESTRE_X, L3_SILVESTRE_Y);
		keysRequired = 3;
		keys[0] = { keyPickupPos(ts, kz, 5, 15),  false };
		keys[1] = { keyPickupPos(ts, kz, 10, 9),  false };
		keys[2] = { keyPickupPos(ts, kz, 15, 5),  false };
		items[0] = { ITEM_WEIGHT, itemPickupPos(ts, 3, 15),  false };
		items[1] = { ITEM_BOMB,   itemPickupPos(ts, 12, 9),  false };
		items[2] = { ITEM_BOOTS,  itemPickupPos(ts, 8, 6),   false };
		items[3] = { ITEM_CLOCK,  itemPickupPos(ts, 16, 5),  false };
		itemCount = 4;
		break;

	case 4:
		setSpawn(18, 13);
		spawnEnemy(0, LUCAS,    L4_LUCAS_X,    L4_LUCAS_Y);
		spawnEnemy(1, TASMANIA, L4_TASMANIA_X, L4_TASMANIA_Y);
		keysRequired = 3;
		keys[0] = { keyPickupPos(ts, kz, 7, 14),  false };
		keys[1] = { keyPickupPos(ts, kz, 10, 8),  false };
		keys[2] = { keyPickupPos(ts, kz, 16, 3),  false };
		items[0] = { ITEM_WEIGHT, itemPickupPos(ts, 5, 14),  false };
		items[1] = { ITEM_BOMB,   itemPickupPos(ts, 14, 8),  false };
		items[2] = { ITEM_BOOTS,  itemPickupPos(ts, 4, 8),   false };
		items[3] = { ITEM_CLOCK,  itemPickupPos(ts, 12, 3),  false };
		itemCount = 4;
		break;

	default: // nivel 5
		setSpawn(18, 13);
		spawnEnemy(0, GHOST,    L5_SILVESTRE_X, L5_SILVESTRE_Y);
		spawnEnemy(1, TASMANIA, L5_TASMANIA_X,  L5_TASMANIA_Y);
		spawnEnemy(2, LUCAS,    L5_LUCAS_X,     L5_LUCAS_Y);
		keysRequired = 3;
		keys[0] = { keyPickupPos(ts, kz, 7, 14),  false };
		keys[1] = { keyPickupPos(ts, kz, 10, 8),  false };
		keys[2] = { keyPickupPos(ts, kz, 16, 3),  false };
		items[0] = { ITEM_WEIGHT, itemPickupPos(ts, 5, 14),  false };
		items[1] = { ITEM_BOMB,   itemPickupPos(ts, 14, 8),  false };
		items[2] = { ITEM_BOOTS,  itemPickupPos(ts, 4, 8),   false };
		items[3] = { ITEM_CLOCK,  itemPickupPos(ts, 12, 3),  false };
		itemCount = 4;
		break;
	}
}

bool Scene::checkCollision(const glm::ivec2 &posA, const glm::ivec2 &posB,
                           const glm::ivec2 &sizeA, const glm::ivec2 &sizeB) const {
	return (posA.x <= posB.x + sizeB.x && posA.x + sizeA.x >= posB.x &&
	        posA.y <= posB.y + sizeB.y && posA.y + sizeA.y >= posB.y);
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

	// ---- Ascensores ----
	if (playerEnteringElevator && !player->isEnteringElevator()) {
		player->startElevatorExit(elevatorExitPos);
		playerEnteringElevator = false;
	}
	else if (!player->isInElevator()) {
		glm::ivec2 belowTile(
			(playerPos.x + playerSize.x / 2) / ts,
			(playerPos.y + playerSize.y) / ts
		);
		for (const auto &ep : elevatorPairs) {
			if (belowTile == ep.entryTile && Game::instance().getKey(GLFW_KEY_UP)) {
				elevatorExitPos = glm::ivec2(ep.exitTile.x * ts, ep.exitTile.y * ts - ts);
				player->startElevatorEnter();
				playerEnteringElevator = true;
				break;
			} else if (belowTile == ep.exitTile && Game::instance().getKey(GLFW_KEY_DOWN)) {
				elevatorExitPos = glm::ivec2(ep.entryTile.x * ts, ep.entryTile.y * ts - ts);
				player->startElevatorEnter();
				playerEnteringElevator = true;
				break;
			}
		}
	}

	// ---- Teletransporte ----
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
					break;
				}
			}
		}
	}

	// ---- Puertas ----
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
						levelComplete = true;
					} else if (door.type == DOOR_SECRET) {
						secretReturnPos    = playerPos;
						secretEnterPending = true;
						secretAnimTimer    = 400;
					}
					// DOOR_ENTRY: solo abre visualmente
					break;
				}
			}
		}
	} else {
		// Sala secreta: UP en fila <= 6 para salir (ajustar segun el mapa)
		int playerTileY = (playerPos.y + playerSize.y) / ts;
		if (secretExitCooldown <= 0 && playerTileY <= 6
		    && Game::instance().getKey(GLFW_KEY_UP)) {
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
			if (checkCollision(playerPos, items[i].pos, playerSize, pickupSize)) {
				items[i].collected = true;
				hasItem     = true;
				carriedItem = items[i].type;
			}
		}

		for (int i = 0; i < keysRequired; ++i) {
			if (keys[i].collected) continue;
			if (checkCollision(playerPos, keys[i].pos, playerSize, pickupSize)) {
				keys[i].collected = true;
				keysCollected++;
			}
		}

		if (hasItem && Game::instance().getKey(GLFW_KEY_Z)) {
			hasItem = false;
			switch (carriedItem) {
			case ITEM_CLOCK:
				enemiesFrozen = true;
				freezeTimer   = 5000.f;
				break;
			case ITEM_BOOTS:
				player->applyBoots(5000);
				break;
			case ITEM_BOMB:
				for (int i = 0; i < activeEnemies; ++i) {
					if (!enemies[i]->isAlive()) continue;
					glm::ivec2 diff = enemies[i]->getPosition() - playerPos;
					if (abs(diff.x) < 3 * ts && abs(diff.y) < 3 * ts) enemies[i]->kill();
				}
				break;
			case ITEM_WEIGHT:
				for (int i = 0; i < activeEnemies; ++i) {
					if (!enemies[i]->isAlive()) continue;
					glm::ivec2 ePos = enemies[i]->getPosition();
					int dx = abs(ePos.x - playerPos.x);
					int dy = ePos.y - playerPos.y;
					if (dx < ts * 3 && dy > 0 && dy < ts * 8) enemies[i]->kill();
				}
				break;
			}
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
	}

	player->render();

	if (!inSecretRoom) {
		for (int i = 0; i < activeEnemies; ++i)
			if (enemies[i] && enemies[i]->isAlive())
				enemies[i]->render();
	}

	if (player->isGodMode()) {
		const glm::ivec2 pp2 = player->getPosition();
		const int ps = player->getSpriteSize().x;
		const float t = currentTime * 0.004f;
		for (int i = 0; i < 3; ++i) {
			float ang = t + float(i) * (2.f * float(M_PI) / 3.f);
			glm::vec2 orb(pp2.x + std::cos(ang) * 26.f + ps / 2.f - 7.f,
			              pp2.y + std::sin(ang) * 16.f + ps / 2.f - 7.f);
			texProgram.setUniform4f("color", 1.f, 0.92f, 0.35f, 0.85f);
			godAuraSprites[i]->setPosition(orb);
			godAuraSprites[i]->render();
		}
		texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
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
		float ky = HUD_MARGIN + HUD_SPACING;
		texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
		keyHudSprite->setPosition(glm::vec2(HUD_MARGIN, ky));
		keyHudSprite->render();
		char buf[8];
		snprintf(buf, sizeof(buf), "%d", keysCollected);
		Game::instance().renderBitmapTextHud(buf, HUD_MARGIN + 28.f, ky + 2.f, 4.f, 1.f, 0.95f, 0.35f);
	}

	if (hasItem) {
		texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
		itemHudSprite->changeAnimation(int(carriedItem));
		itemHudSprite->setPosition(glm::vec2(HUD_MARGIN, 480.f - HUD_MARGIN - HUD_ICON_SIZE));
		itemHudSprite->render();
	}

	if (player->isGodMode()) {
		texProgram.setUniform4f("color", 0.f, 1.f, 0.f, 0.7f);
		heartSprite->setPosition(glm::vec2(640.f - HUD_MARGIN - HUD_ICON_SIZE, HUD_MARGIN));
		heartSprite->render();
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
	switch (levelIndex) {
	case 1:  secretFile = "secrets/secret1.txt"; break;
	default: secretFile = "secrets/secret1.txt"; break;
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

	// Posicion inicial en sala secreta — ajustar segun el mapa
	const int ts = map->getTileSize();
	player->setPosition(glm::vec2(8 * ts, 11 * ts));

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
