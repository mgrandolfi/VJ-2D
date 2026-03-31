#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"


#define SCREEN_X 0
#define SCREEN_Y 16

#define HUD_ICON_SIZE 24.f
#define HUD_MARGIN    8.f

#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 3
#define HUD_SPACING  28.f

// Level 1
#define L1_PIOLIN_X      16
#define L1_PIOLIN_Y     3
#define L1_PIOLIN_RANGE  4   

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


Scene::Scene()
{
	map           = NULL;
	player        = NULL;
	heartSprite   = NULL;
	keySprite     = NULL;
	itemSprite     = NULL;
	itemHudSprite  = NULL;
	keyWorldSprite = NULL;
	for (int i = 0; i < MAX_ENEMIES; ++i)
		enemies[i] = NULL;
	activeEnemies = 0;
	keyWorldPixelSize = 16;
}

Scene::~Scene()
{
	if (map)           delete map;
	if (player)        delete player;
	if (heartSprite)   delete heartSprite;
	if (keySprite)     delete keySprite;
	if (itemSprite)     delete itemSprite;
	if (itemHudSprite)  delete itemHudSprite;
	if (keyWorldSprite) delete keyWorldSprite;
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

	// HUD sprites — fixed screen-space size
	heartTex.loadFromFile("images/sprites/heart.png", TEXTURE_PIXEL_FORMAT_RGBA);
	keyIconTex.loadFromFile("images/sprites/key.png", TEXTURE_PIXEL_FORMAT_RGBA);

	heartSprite = Sprite::createSprite(glm::ivec2(HUD_ICON_SIZE, HUD_ICON_SIZE),
	                                   glm::vec2(1.f, 1.f), &heartTex, &texProgram);
	heartSprite->setNumberAnimations(1);
	heartSprite->setAnimationSpeed(0, 1);
	heartSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	heartSprite->changeAnimation(0);

	keySprite = Sprite::createSprite(glm::ivec2(HUD_ICON_SIZE, HUD_ICON_SIZE),
	                                 glm::vec2(1.f, 1.f), &keyIconTex, &texProgram);
	keySprite->setNumberAnimations(1);
	keySprite->setAnimationSpeed(0, 1);
	keySprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	keySprite->changeAnimation(0);

	// Item texture — world quads created per level; HUD uses fixed-size sprite
	itemTex.loadFromFile("images/sprites/items.png", TEXTURE_PIXEL_FORMAT_RGBA);
	itemHudSprite = Sprite::createSprite(glm::ivec2(HUD_ICON_SIZE, HUD_ICON_SIZE),
	                                     glm::vec2(0.25f, 1.f), &itemTex, &texProgram);
	itemHudSprite->setNumberAnimations(4);
	for (int i = 0; i < 4; ++i)
	{
		itemHudSprite->setAnimationSpeed(i, 1);
		itemHudSprite->addKeyframe(i, glm::vec2(0.25f * i, 0.f));
	}
	itemHudSprite->changeAnimation(0);
}

void Scene::loadLevel(int level)
{
	if (map)    { delete map;    map    = NULL; }
	if (player) { delete player; player = NULL; }
	for (int i = 0; i < MAX_ENEMIES; ++i)
	{
		if (enemies[i]) { delete enemies[i]; enemies[i] = NULL; }
	}
	activeEnemies = 0;

	gameOver               = false;
	levelComplete          = false;
	playerEnteringElevator = false;
	enemiesFrozen          = false;
	freezeTimer    = 0.f;
	respawnTimer   = 0.f;
	keysCollected  = 0;
	itemCount      = 0;
	hasItem        = false;
	currentTime    = 0.f;

	initMap(level);
	recreateWorldPickupSprites(map->getTileSize());
	spawnEntities(level);

	const int ts = map->getTileSize();
	if (ts <= 16)
		camZoom = 2.0f;
	else
		camZoom = 1.5f;

	camX = 0.f;
	camY = 0.f;
}

void Scene::initMap(int level)
{
	string levelFile;
	switch (level)
	{
	case 1: levelFile = "levels/level_1.txt"; break;
	case 2: levelFile = "levels/level_2.txt"; break;
	case 3: levelFile = "levels/level_3.txt"; break;
	case 4: levelFile = "levels/level_4.txt"; break;
	default: levelFile = "levels/level_5.txt"; break;
	}

	map = TileMap::createTileMap(levelFile, glm::vec2(SCREEN_X, SCREEN_Y), texProgram);

	tileBlocks.clear();
	tileCliffs.clear();
	tileLadders.clear();
	tileDoors.clear();
	tileJumps.clear();
	tileWarps.clear();
	tileElevators.clear();
	elevatorPairs.clear();

	if (level == 1)
	{
		tileBlocks    = {0, 3, 4, 7, 10, 13, 14, 15, 28, 37, 42, 62}; // bloques solidos
		tileCliffs    = {2, 8, 12, 17};                                 // rampas
		tileLadders   = {9, 34};                                        // plantas / escaleras
		tileWarps     = {36, 61};                                       // suelo teletransporte
		tileJumps     = {11};                                           // plataforma up
		tileElevators = {64, 65};                                       // tile IDs to mark solid

		// Elevator pairs: entry tile (col,row) → exit tile (col,row)
		elevatorPairs = {
			{ glm::ivec2(6, 11), glm::ivec2(7,  8) },
			{ glm::ivec2(18,14), glm::ivec2(19,16) }
		};
	}
	else if (level == 3)
	{
		tileBlocks  = {0, 3, 6, 7, 8, 10, 13, 14, 15,
		               35, 36, 37, 38, 40, 41, 42, 44,
		               66, 67, 68, 69, 71, 72, 73};
		tileLadders = {9, 11, 12, 43, 70};
		tileDoors   = {2};
	}
	else
	{
		tileBlocks  = {0, 1, 9};
		tileLadders = {2};
		tileDoors   = {3};
		tileJumps   = {4};
		tileWarps   = {5};
	}

	applyTileTypes();
}

void Scene::applyTileTypes()
{
	for (int id : tileBlocks)    map->setTileType(id, TILE_BLOCK);
	for (int id : tileCliffs)    map->setTileType(id, TILE_CLIFF);
	for (int id : tileLadders)   map->setTileType(id, TILE_LADDER);
	for (int id : tileDoors)     map->setTileType(id, TILE_DOOR);
	for (int id : tileJumps)     map->setTileType(id, TILE_JUMP);
	for (int id : tileWarps)     map->setTileType(id, TILE_WARP);
	for (int id : tileElevators) map->setTileType(id, TILE_ELEVATOR);
}

void Scene::recreateWorldPickupSprites(int ts)
{
	if (keyWorldSprite) { delete keyWorldSprite; keyWorldSprite = NULL; }
	if (itemSprite)     { delete itemSprite;     itemSprite     = NULL; }

	keyWorldPixelSize = (ts * 5) / 8;
	if (keyWorldPixelSize < 10)
		keyWorldPixelSize = 10;
	if (keyWorldPixelSize > ts - 2)
		keyWorldPixelSize = ts - 2;

	keyWorldSprite = Sprite::createSprite(glm::ivec2(keyWorldPixelSize, keyWorldPixelSize),
	                                    glm::vec2(1.f, 1.f), &keyIconTex, &texProgram);
	keyWorldSprite->setNumberAnimations(1);
	keyWorldSprite->setAnimationSpeed(0, 1);
	keyWorldSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	keyWorldSprite->changeAnimation(0);

	itemSprite = Sprite::createSprite(glm::ivec2(ts, ts),
	                                glm::vec2(0.25f, 1.f), &itemTex, &texProgram);
	itemSprite->setNumberAnimations(4);
	for (int i = 0; i < 4; ++i)
	{
		itemSprite->setAnimationSpeed(i, 1);
		itemSprite->addKeyframe(i, glm::vec2(0.25f * i, 0.f));
	}
	itemSprite->changeAnimation(0);
}

//Spawn de enemigos, objetos...
void Scene::spawnEntities(int level)
{
	const int ts = map->getTileSize();

	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram, ts);
	player->setTileMap(map);

	auto spawnEnemy = [&](int idx, EnemyTypes type, int tileX, int tileY)
	{
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

	switch (level)
	{
	case 1:
		setSpawn(INIT_PLAYER_X_TILES, INIT_PLAYER_Y_TILES);
		spawnEnemy(0, PIOLIN, L1_PIOLIN_X, L1_PIOLIN_Y);
		enemies[0]->setPatrolRange(L1_PIOLIN_RANGE * ts);
		keysRequired = 3;
		keys[0] = { glm::ivec2(5  * ts, 14 * ts), false };
		keys[1] = { glm::ivec2(10 * ts, 10 * ts), false };
		keys[2] = { glm::ivec2(15 * ts, 4  * ts), false };
		items[0] = { ITEM_WEIGHT, glm::ivec2(3  * ts, 14 * ts), false };
		items[1] = { ITEM_BOMB,   glm::ivec2(12 * ts, 10 * ts), false };
		items[2] = { ITEM_BOOTS,  glm::ivec2(8  * ts, 6  * ts), false };
		items[3] = { ITEM_CLOCK,  glm::ivec2(16 * ts, 6  * ts), false };
		itemCount = 4;
		break;

	case 2:
		setSpawn(18, 13);
		spawnEnemy(0, LUCAS,  L2_LUCAS_X,  L2_LUCAS_Y);
		spawnEnemy(1, PIOLIN, L2_PIOLIN_X, L2_PIOLIN_Y);
		keysRequired = 3;
		keys[0] = { glm::ivec2(5  * ts, 13 * ts), false };
		keys[1] = { glm::ivec2(10 * ts,  7 * ts), false };
		keys[2] = { glm::ivec2(17 * ts,  2 * ts), false };
		items[0] = { ITEM_WEIGHT, glm::ivec2(7  * ts, 13 * ts), false };
		items[1] = { ITEM_BOMB,   glm::ivec2(15 * ts,  7 * ts), false };
		items[2] = { ITEM_BOOTS,  glm::ivec2( 4 * ts,  7 * ts), false };
		items[3] = { ITEM_CLOCK,  glm::ivec2(12 * ts,  2 * ts), false };
		itemCount = 4;
		break;

	case 3:
		setSpawn(18, 16);
		spawnEnemy(0, PIOLIN,    L3_PIOLIN_X,    L3_PIOLIN_Y);
		spawnEnemy(1, GHOST, L3_SILVESTRE_X, L3_SILVESTRE_Y);
		keysRequired = 3;
		keys[0] = { glm::ivec2(5  * ts, 14 * ts), false };
		keys[1] = { glm::ivec2(10 * ts, 8  * ts), false };
		keys[2] = { glm::ivec2(15 * ts, 4  * ts), false };
		items[0] = { ITEM_WEIGHT, glm::ivec2(3  * ts, 14 * ts), false };
		items[1] = { ITEM_BOMB,   glm::ivec2(12 * ts, 8  * ts), false };
		items[2] = { ITEM_BOOTS,  glm::ivec2(8  * ts, 5  * ts), false };
		items[3] = { ITEM_CLOCK,  glm::ivec2(16 * ts, 4  * ts), false };
		itemCount = 4;
		break;

	case 4:
		setSpawn(18, 13);
		spawnEnemy(0, LUCAS,    L4_LUCAS_X,    L4_LUCAS_Y);
		spawnEnemy(1, TASMANIA, L4_TASMANIA_X, L4_TASMANIA_Y);
		keysRequired = 3;
		keys[0] = { glm::ivec2(7  * ts, 13 * ts), false };
		keys[1] = { glm::ivec2(10 * ts,  7 * ts), false };
		keys[2] = { glm::ivec2(16 * ts,  2 * ts), false };
		items[0] = { ITEM_WEIGHT, glm::ivec2(5  * ts, 13 * ts), false };
		items[1] = { ITEM_BOMB,   glm::ivec2(14 * ts,  7 * ts), false };
		items[2] = { ITEM_BOOTS,  glm::ivec2( 4 * ts,  7 * ts), false };
		items[3] = { ITEM_CLOCK,  glm::ivec2(12 * ts,  2 * ts), false };
		itemCount = 4;
		break;

	default: // level 5
		setSpawn(18, 13);
		spawnEnemy(0, GHOST, L5_SILVESTRE_X, L5_SILVESTRE_Y);
		spawnEnemy(1, TASMANIA,  L5_TASMANIA_X,  L5_TASMANIA_Y);
		spawnEnemy(2, LUCAS,     L5_LUCAS_X,     L5_LUCAS_Y);
		keysRequired = 3;
		keys[0] = { glm::ivec2(7  * ts, 13 * ts), false };
		keys[1] = { glm::ivec2(10 * ts,  7 * ts), false };
		keys[2] = { glm::ivec2(16 * ts,  2 * ts), false };
		items[0] = { ITEM_WEIGHT, glm::ivec2(5  * ts, 13 * ts), false };
		items[1] = { ITEM_BOMB,   glm::ivec2(14 * ts,  7 * ts), false };
		items[2] = { ITEM_BOOTS,  glm::ivec2( 4 * ts,  7 * ts), false };
		items[3] = { ITEM_CLOCK,  glm::ivec2(12 * ts,  2 * ts), false };
		itemCount = 4;
		break;
	}
}

// ---------------------------------------------------------------------------

bool Scene::checkCollision(const glm::ivec2 &posA, const glm::ivec2 &posB,
                        const glm::ivec2 &sizeA, const glm::ivec2 &sizeB) const
{
	// Inclusive on all edges so touching boxes (no gap) still count as colliding.
	// This is needed for pickups whose top edge equals the walking player's top edge.
	return (posA.x <= posB.x + sizeB.x &&
	        posA.x + sizeA.x >= posB.x &&
	        posA.y <= posB.y + sizeB.y &&
	        posA.y + sizeA.y >= posB.y);
}

// ---------------------------------------------------------------------------

void Scene::update(int deltaTime)
{
	currentTime += deltaTime;

	if (enemiesFrozen)
	{
		freezeTimer -= deltaTime;
		if (freezeTimer <= 0.f)
			enemiesFrozen = false;
	}

	if (respawnTimer > 0.f)
	{
		respawnTimer -= deltaTime;
		if (respawnTimer <= 0.f)
			player->setPosition(glm::vec2(spawnPos));
		return;
	}

	player->update(deltaTime);

	const glm::ivec2 playerPos  = player->getPosition();
	const glm::ivec2 playerSize = player->getSpriteSize();

	const int ts = map->getTileSize();

	// --- Elevator ---
	if (playerEnteringElevator && !player->isEnteringElevator())
	{
		// ENTER animation done → teleport and play EXIT
		player->startElevatorExit(elevatorExitPos);
		playerEnteringElevator = false;
	}
	else if (!player->isInElevator())
	{
		// Tile directly below the player's center
		glm::ivec2 belowTile(
			(playerPos.x + playerSize.x / 2) / ts,
			(playerPos.y + playerSize.y) / ts
		);

		for (const auto &ep : elevatorPairs)
		{
			if (belowTile == ep.entryTile && Game::instance().getKey(GLFW_KEY_UP))
			{
				// entry → exit (UP)
				elevatorExitPos = glm::ivec2(ep.exitTile.x * ts,
				                             ep.exitTile.y * ts - ts);
				player->startElevatorEnter();
				playerEnteringElevator = true;
				break;
			}
			else if (belowTile == ep.exitTile && Game::instance().getKey(GLFW_KEY_DOWN))
			{
				// exit → entry (DOWN)
				elevatorExitPos = glm::ivec2(ep.entryTile.x * ts,
				                             ep.entryTile.y * ts - ts);
				player->startElevatorEnter();
				playerEnteringElevator = true;
				break;
			}
		}
	}

	const glm::ivec2 enemySize(ts, ts);
	const glm::ivec2 pickupSize(ts, ts);

	for (int i = 0; i < activeEnemies; ++i)
	{
		if (!enemies[i]->isAlive()) continue;

		enemies[i]->setTarget(playerPos);
		if (!enemiesFrozen)
			enemies[i]->update(deltaTime);

		glm::ivec2 pHitPos = playerPos;
		glm::ivec2 pHitSize = playerSize;
		if (map->isOnLadder(pHitPos, pHitSize))
		{
			pHitPos.x -= 6;
			pHitSize.x += 12;
		}

		if (!player->isGodMode() && !player->isHurt() &&
		    checkCollision(pHitPos, enemies[i]->getPosition(), pHitSize, enemySize))
		{
			player->dies();
			if (player->getLives() <= 0)
				gameOver = true;
			else
				respawnTimer = 1500.f;
		}
	}

	for (int i = 0; i < itemCount; ++i)
	{
		if (items[i].collected) continue;
		if (checkCollision(playerPos, items[i].pos, playerSize, pickupSize))
		{
			items[i].collected = true;
			hasItem     = true;
			carriedItem = items[i].type;
		}
	}

	for (int i = 0; i < keysRequired; ++i)
	{
		if (keys[i].collected) continue;
		if (checkCollision(playerPos, keys[i].pos, playerSize, pickupSize))
		{
			keys[i].collected = true;
			keysCollected++;
		}
	}

	// Use carried item (Z key)
	if (hasItem && Game::instance().getKey(GLFW_KEY_Z))
	{
		hasItem = false;
		switch (carriedItem)
		{
		case ITEM_CLOCK:
			enemiesFrozen = true;
			freezeTimer   = 5000.f;
			break;

		case ITEM_BOOTS:
			player->applyBoots(5000);
			break;

		case ITEM_BOMB:
			for (int i = 0; i < activeEnemies; ++i)
			{
				if (!enemies[i]->isAlive()) continue;
				glm::ivec2 diff = enemies[i]->getPosition() - playerPos;
				if (abs(diff.x) < 3 * ts && abs(diff.y) < 3 * ts)
					enemies[i]->kill();
			}
			break;

		case ITEM_WEIGHT:
			for (int i = 0; i < activeEnemies; ++i)
			{
				if (!enemies[i]->isAlive()) continue;
				glm::ivec2 ePos = enemies[i]->getPosition();
				int dx = abs(ePos.x - playerPos.x);
				int dy = ePos.y - playerPos.y;
				if (dx < ts * 3 && dy > 0 && dy < ts * 8)
					enemies[i]->kill();
			}
			break;
		}
	}

	if (keysCollected >= keysRequired && map->isOnDoor(playerPos, playerSize))
		levelComplete = true;
}

void Scene::render()
{
	const float mapW = float(map->getMapWidth()  * map->getTileSize());
	const float mapH = float(map->getMapHeight() * map->getTileSize());
	projection = glm::ortho(0.f, mapW, mapH, 0.f);

	glm::mat4 modelview = glm::mat4(1.f);

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);

	map->render();

	const int ts = map->getTileSize();
	// Render uncollected keys in the world
	for (int i = 0; i < keysRequired; ++i)
	{
		if (keys[i].collected) continue;
		texProgram.setUniform4f("color", 1.f, 1.f, 0.f, 1.f);
		glm::ivec2 off((ts - keyWorldPixelSize) / 2, (ts - keyWorldPixelSize) / 2);
		keyWorldSprite->setPosition(glm::vec2(keys[i].pos + off));
		keyWorldSprite->render();
	}
	texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);

	// Render uncollected items in the world
	for (int i = 0; i < itemCount; ++i)
	{
		if (items[i].collected) continue;
		itemSprite->changeAnimation(int(items[i].type));
		itemSprite->setPosition(glm::vec2(items[i].pos));
		itemSprite->render();
	}

	player->render();

	for (int i = 0; i < activeEnemies; ++i)
		if (enemies[i]->isAlive())
			enemies[i]->render();

	renderHUD();
}

// ---------------------------------------------------------------------------
// HUD rendered in screen-space (always 640x480) for consistent sizing
// ---------------------------------------------------------------------------

void Scene::renderHUD()
{
	if (!player) return;

	glm::mat4 hudProj = glm::ortho(0.f, 640.f, 480.f, 0.f);
	glm::mat4 identity = glm::mat4(1.f);

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", hudProj);
	texProgram.setUniformMatrix4f("modelview", identity);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Hearts (lives)
	texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	for (int i = 0; i < player->getLives(); ++i)
	{
		heartSprite->setPosition(glm::vec2(HUD_MARGIN + i * HUD_SPACING, HUD_MARGIN));
		heartSprite->render();
	}

	// Key counter
	float ky = HUD_MARGIN + HUD_SPACING;
	for (int i = 0; i < keysRequired; ++i)
	{
		if (keys[i].collected)
			texProgram.setUniform4f("color", 1.f, 1.f, 0.f, 1.f);
		else
			texProgram.setUniform4f("color", 0.35f, 0.35f, 0.35f, 0.6f);

		keySprite->setPosition(glm::vec2(HUD_MARGIN + i * HUD_SPACING, ky));
		keySprite->render();
	}

	// Carried item indicator (bottom-left)
	if (hasItem)
	{
		texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
		itemHudSprite->changeAnimation(int(carriedItem));
		itemHudSprite->setPosition(glm::vec2(HUD_MARGIN, 480.f - HUD_MARGIN - HUD_ICON_SIZE));
		itemHudSprite->render();
	}

	// God mode indicator
	if (player->isGodMode())
	{
		texProgram.setUniform4f("color", 0.f, 1.f, 0.f, 0.7f);
		heartSprite->setPosition(glm::vec2(640.f - HUD_MARGIN - HUD_ICON_SIZE, HUD_MARGIN));
		heartSprite->render();
	}

	texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	glDisable(GL_BLEND);

	// Restore game projection for subsequent draw calls
	texProgram.setUniformMatrix4f("projection", projection);
}

// ---------------------------------------------------------------------------

void Scene::setGodMode(bool g)
{
	if (player) player->setGodMode(g);
}

void Scene::killAllEnemies()
{
	for (int i = 0; i < activeEnemies; ++i)
		if (enemies[i]) enemies[i]->kill();
}

void Scene::collectAllKeys()
{
	for (int i = 0; i < keysRequired; ++i)
	{
		if (!keys[i].collected)
		{
			keys[i].collected = true;
			keysCollected++;
		}
	}
}
