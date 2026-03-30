#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"


#define SCREEN_X 0
#define SCREEN_Y 0

#define HUD_ICON_SIZE 24.f
#define HUD_MARGIN    8.f
#define HUD_SPACING  28.f


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

	gameOver       = false;
	levelComplete  = false;
	enemiesFrozen  = false;
	freezeTimer    = 0.f;
	respawnTimer   = 0.f;
	keysCollected  = 0;
	itemCount      = 0;
	hasItem        = false;
	currentTime    = 0.f;

	initMap(level);
	recreateWorldPickupSprites(map->getTileSize());
	spawnEntities(level);

	// Pick zoom: 16px-tile levels need more zoom, 32px levels less
	const int ts = map->getTileSize();
	if (ts <= 16)
		camZoom = 2.0f;
	else
		camZoom = 1.5f;

	camX = 0.f;
	camY = 0.f;
}

// ---------------------------------------------------------------------------
// Map loading + tile-type setup per level
// ---------------------------------------------------------------------------

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

	if (level == 1)
	{
		// Carol's tileset_level1.png (25x14 tilesheet)
		// Walls and platforms
		int blockIds[] = {0, 3, 4, 7, 10, 13, 14, 15, 17,
		                  25, 28, 35, 36, 41, 42, 57, 62, 64, 65, 61, -1};
		for (int i = 0; blockIds[i] != -1; ++i)
			map->setTileType(blockIds[i], TILE_BLOCK);
		// Floor / stone art used as solid ground (tile 1 is common in level_1)
		map->setTileType(1, TILE_BLOCK);
		map->setTileType(5, TILE_BLOCK);
		// Platform tops drawn with these ids — must collide or the player falls through
		map->setTileType(29, TILE_BLOCK);
		map->setTileType(30, TILE_BLOCK);
		map->setTileType(31, TILE_BLOCK);
		map->setTileType(32, TILE_BLOCK);
		// Ladders
		int ladderIds[] = {9, 12, 37, -1};
		for (int i = 0; ladderIds[i] != -1; ++i)
			map->setTileType(ladderIds[i], TILE_LADDER);
		// Door
		map->setTileType(2, TILE_DOOR);
		// Tile 6 = spider web — stay pass-through (decoration)
		int moreBlocks[] = { 8, 16, 26, 27, 33, 34, 38, 39, 40, -1 };
		for (int i = 0; moreBlocks[i] != -1; ++i)
			map->setTileType(moreBlocks[i], TILE_BLOCK);
	}
	else if (level == 3)
	{
		// Carol's tileset_level3.png (30x30 tilesheet)
		int blockIds[] = {0, 3, 6, 7, 8, 10, 13, 14, 15,
		                  35, 36, 37, 38, 40, 41, 42, 44,
		                  66, 67, 68, 69, 71, 72, 73, -1};
		for (int i = 0; blockIds[i] != -1; ++i)
			map->setTileType(blockIds[i], TILE_BLOCK);
		// Ladders
		int ladderIds[] = {9, 11, 12, 43, 70, -1};
		for (int i = 0; ladderIds[i] != -1; ++i)
			map->setTileType(ladderIds[i], TILE_LADDER);
		// Door
		map->setTileType(2, TILE_DOOR);
		// Common floor tiles in level_3 not in blockIds (exclude ladder ids 9,11,12,43,70)
		int moreBlocks3[] = { 1, 30, 31, 32, 39, 60, 61, 71, 72, -1 };
		for (int i = 0; moreBlocks3[i] != -1; ++i)
			map->setTileType(moreBlocks3[i], TILE_BLOCK);
	}
	else
	{
		// Levels 2/4/5: dungeon 8x2 tilesheet
		// Solid blocks (multiple visual variants for level variety)
		map->setTileType(0, TILE_BLOCK);  // plain stone block
		map->setTileType(1, TILE_BLOCK);  // stone with chain/pipe decoration
		map->setTileType(9, TILE_BLOCK);  // stone brick (platform floors)
		// Tile 6 = spider web: TILE_EMPTY (visual decoration only, player passes through)
		map->setTileType(2, TILE_LADDER);
		map->setTileType(3, TILE_DOOR);
		map->setTileType(4, TILE_JUMP);
		map->setTileType(5, TILE_WARP);
	}
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

// ---------------------------------------------------------------------------
// Spawn player, enemies and items
// ---------------------------------------------------------------------------

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
		setSpawn(18, 16);
		spawnEnemy(0, PIOLIN, 4, 16);
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
		spawnEnemy(0, LUCAS,  3, 13);
		spawnEnemy(1, PIOLIN, 10, 7);
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
		spawnEnemy(0, PIOLIN,    4, 16);
		spawnEnemy(1, SILVESTRE, 10, 10);
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
		spawnEnemy(0, LUCAS,    3,  13);
		spawnEnemy(1, TASMANIA, 10,  7);
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
		spawnEnemy(0, SILVESTRE, 3,  13);
		spawnEnemy(1, TASMANIA,  10,  7);
		spawnEnemy(2, LUCAS,     15,  2);
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

// ---------------------------------------------------------------------------

void Scene::render()
{
	// Compute camera: viewport centered on player, clamped to map edges
	const float mapW = float(map->getMapWidth()  * map->getTileSize());
	const float mapH = float(map->getMapHeight() * map->getTileSize());
	const float viewW = 640.f / camZoom;
	const float viewH = 480.f / camZoom;

	if (player)
	{
		glm::ivec2 pp = player->getPosition();
		glm::ivec2 ps = player->getSpriteSize();
		camX = float(pp.x + ps.x / 2) - viewW / 2.f;
		camY = float(pp.y + ps.y / 2) - viewH / 2.f;
	}

	// Clamp to map edges (or center if map smaller than view)
	if (mapW <= viewW)
		camX = (mapW - viewW) / 2.f;
	else
	{
		if (camX < 0.f)          camX = 0.f;
		if (camX > mapW - viewW) camX = mapW - viewW;
	}
	if (mapH <= viewH)
		camY = (mapH - viewH) / 2.f;
	else
	{
		if (camY < 0.f)          camY = 0.f;
		if (camY > mapH - viewH) camY = mapH - viewH;
	}

	projection = glm::ortho(camX, camX + viewW, camY + viewH, camY);

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
