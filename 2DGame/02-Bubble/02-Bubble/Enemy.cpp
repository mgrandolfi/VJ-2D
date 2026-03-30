#include <cmath>
#include <cstdlib>
#include <iostream>
#include <GL/glew.h>
#include "Enemy.h"
#include "Game.h"


#define ENEMY_FALL 4


enum EnemyAnims { WALK_RIGHT = 0, WALK_LEFT = 1 };

static float enemyRowY(EnemyTypes type, int animIdx)
{
	switch (type)
	{
	case TASMANIA:  return (animIdx == WALK_RIGHT) ? 0.0f : 0.1f;
	case LUCAS:     return (animIdx == WALK_RIGHT) ? 0.2f : 0.3f;
	case SILVESTRE: return (animIdx == WALK_RIGHT) ? 0.4f : 0.5f;
	case PIOLIN:    return (animIdx == WALK_RIGHT) ? 0.6f : 0.7f;
	}
	return 0.f;
}


Enemy::Enemy()
{
	sprite     = NULL;
	map        = NULL;
	alive      = true;
	spriteSize = 32;
}

Enemy::~Enemy()
{
	if (sprite != NULL)
		delete sprite;
}

void Enemy::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram,
                 EnemyTypes enemyType, int tileSize)
{
	type       = enemyType;
	spriteSize = tileSize;
	alive      = true;
	dir        = 1.f;
	isStopped  = false;
	timeWait   = 0.f;
	steps      = 0;
	maxSteps   = 80;
	climbVy    = 0.f;
	onLadder   = false;

	switch (type)
	{
	case PIOLIN:    speed = 1.5f; break;
	case LUCAS:     speed = 2.0f; break;
	case SILVESTRE: speed = 1.0f; break;
	case TASMANIA:  speed = 3.5f; break;
	}

	spritesheet.loadFromFile("images/enemies.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(glm::ivec2(spriteSize, spriteSize),
	                              glm::vec2(0.09f, 0.09f),
	                              &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(2);

	float yr = enemyRowY(type, WALK_RIGHT);
	sprite->setAnimationSpeed(WALK_RIGHT, 6);
	for (int i = 0; i < 5; ++i)
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.1f * i, yr));

	float yl = enemyRowY(type, WALK_LEFT);
	sprite->setAnimationSpeed(WALK_LEFT, 6);
	for (int i = 0; i < 5; ++i)
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.1f * i, yl));

	sprite->changeAnimation(WALK_RIGHT);

	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x),
	                              float(tileMapDispl.y + posEnemy.y)));

	patrolMin = posEnemy.x - 100.f;
	patrolMax = posEnemy.x + 100.f;
}

void Enemy::update(int deltaTime)
{
	if (!alive) return;

	sprite->update(deltaTime);

	switch (type)
	{
	case PIOLIN:    patrolMovement(deltaTime);          break;
	case LUCAS:     chasingPlayer_Lucas(deltaTime);     break;
	case SILVESTRE: chasingPlayer_Silvestre(deltaTime);  break;
	case TASMANIA:  chasingPlayer_Tasmania(deltaTime);   break;
	}

	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x),
	                              float(tileMapDispl.y + posEnemy.y)));
}

void Enemy::render()
{
	if (!alive) return;
	sprite->render();
}

void Enemy::setTileMap(TileMap *tileMap)
{
	map = tileMap;
}

void Enemy::setPosition(const glm::vec2 &pos)
{
	posEnemy = pos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x),
	                              float(tileMapDispl.y + posEnemy.y)));
	patrolMin = posEnemy.x - 100.f;
	patrolMax = posEnemy.x + 100.f;
}

static void applyGravity(glm::ivec2 &pos, TileMap *map, int sz)
{
	if (!map) return;
	glm::ivec2 size(sz, sz);
	pos.y += ENEMY_FALL;
	if (map->collisionMoveDown(pos, size, &pos.y)) return;

	// Prevent falling through TILE_LADDER tiles at platform rows.
	// Standard collisionMoveDown ignores ladders (passable), but enemies
	// don't climb — they must treat those tiles as solid ground.
	const int ts = map->getTileSize();
	const int x0 = pos.x / ts;
	const int x1 = (pos.x + sz - 1) / ts;
	const int y  = (pos.y + sz - 1) / ts;
	for (int x = x0; x <= x1; ++x)
	{
		if (map->tileTypeAt(x * ts + ts / 2, y * ts + ts / 2) == TILE_LADDER)
		{
			const int overlap = pos.y + sz - y * ts;
			if (overlap >= 0 && overlap <= ENEMY_FALL)
			{
				pos.y = y * ts - sz;
				return;
			}
		}
	}
}

void Enemy::patrolMovement(int deltaTime)
{
	const glm::ivec2 size(spriteSize, spriteSize);
	const int mapW = map->getMapWidth() * map->getTileSize();

	if (isStopped)
	{
		timeWait -= deltaTime;
		if (timeWait <= 0.f)
		{
			steps     = 0;
			isStopped = false;
		}
		return;
	}

	applyGravity(posEnemy, map, spriteSize);

	posEnemy.x += int(dir * speed);
	if (posEnemy.x < 0)              posEnemy.x = 0;
	if (posEnemy.x > mapW - spriteSize) posEnemy.x = mapW - spriteSize;

	if (dir > 0 && map->collisionMoveRight(posEnemy, size, true))
	{
		posEnemy.x -= int(speed);
		dir = -1.f;
		sprite->changeAnimation(WALK_LEFT);
	}
	else if (dir < 0 && map->collisionMoveLeft(posEnemy, size, true))
	{
		posEnemy.x += int(speed);
		dir = 1.f;
		sprite->changeAnimation(WALK_RIGHT);
	}

	if (posEnemy.x >= patrolMax)       { dir = -1.f; sprite->changeAnimation(WALK_LEFT);  }
	else if (posEnemy.x <= patrolMin)  { dir =  1.f; sprite->changeAnimation(WALK_RIGHT); }

	++steps;
	if (steps >= maxSteps)
	{
		timeWait  = 800.f;
		isStopped = true;
	}
}

void Enemy::chasingPlayer_Lucas(int deltaTime)
{
	if (!map) return;
	const glm::ivec2 size(spriteSize, spriteSize);
	const int mapW = map->getMapWidth() * map->getTileSize();

	applyGravity(posEnemy, map, spriteSize);

	int dx = targetPos.x - posEnemy.x;
	dir = (dx > 0) ? 1.f : -1.f;
	sprite->changeAnimation((dir > 0) ? WALK_RIGHT : WALK_LEFT);

	posEnemy.x += int(dir * speed);
	if (posEnemy.x < 0)              posEnemy.x = 0;
	if (posEnemy.x > mapW - spriteSize) posEnemy.x = mapW - spriteSize;

	if (dir > 0 && map->collisionMoveRight(posEnemy, size, true))
	{
		posEnemy.x -= int(speed);
		dir = -1.f;
		sprite->changeAnimation(WALK_LEFT);
	}
	else if (dir < 0 && map->collisionMoveLeft(posEnemy, size, true))
	{
		posEnemy.x += int(speed);
		dir = 1.f;
		sprite->changeAnimation(WALK_RIGHT);
	}
}

void Enemy::chasingPlayer_Silvestre(int deltaTime)
{
	if (!map) return;
	const glm::ivec2 size(spriteSize, spriteSize);
	const int mapW = map->getMapWidth() * map->getTileSize();
	const int mapH = map->getMapHeight() * map->getTileSize();

	onLadder = map->isOnLadder(posEnemy, size);

	if (onLadder)
	{
		int dy = targetPos.y - posEnemy.y;
		if (abs(dy) > 4)
		{
			climbVy = (dy > 0) ? speed : -speed;
			posEnemy.y += int(climbVy);
			if (posEnemy.y < 0) posEnemy.y = 0;
			if (posEnemy.y > mapH - spriteSize) posEnemy.y = mapH - spriteSize;
			int dx = targetPos.x - posEnemy.x;
			dir = (dx > 0) ? 1.f : -1.f;
			sprite->changeAnimation((dir > 0) ? WALK_RIGHT : WALK_LEFT);
		}
		else
		{
			int dx = targetPos.x - posEnemy.x;
			dir = (dx > 0) ? 1.f : -1.f;
			sprite->changeAnimation((dir > 0) ? WALK_RIGHT : WALK_LEFT);
			posEnemy.x += int(dir * speed);
			if (posEnemy.x < 0)              posEnemy.x = 0;
			if (posEnemy.x > mapW - spriteSize) posEnemy.x = mapW - spriteSize;
			// On ladder rungs: do not block on ladder cells horizontally
			if (dir > 0 && map->collisionMoveRight(posEnemy, size, false))
				posEnemy.x -= int(speed);
			else if (dir < 0 && map->collisionMoveLeft(posEnemy, size, false))
				posEnemy.x += int(speed);
		}
	}
	else
	{
		applyGravity(posEnemy, map, spriteSize);

		int dx = targetPos.x - posEnemy.x;
		dir = (dx > 0) ? 1.f : -1.f;
		sprite->changeAnimation((dir > 0) ? WALK_RIGHT : WALK_LEFT);

		posEnemy.x += int(dir * speed);
		if (posEnemy.x < 0)              posEnemy.x = 0;
		if (posEnemy.x > mapW - spriteSize) posEnemy.x = mapW - spriteSize;

		if (dir > 0 && map->collisionMoveRight(posEnemy, size, true))
			posEnemy.x -= int(speed);
		else if (dir < 0 && map->collisionMoveLeft(posEnemy, size, true))
			posEnemy.x += int(speed);
	}
}

void Enemy::chasingPlayer_Tasmania(int deltaTime)
{
	if (!map) return;
	const glm::ivec2 size(spriteSize, spriteSize);
	const int mapW = map->getMapWidth() * map->getTileSize();

	applyGravity(posEnemy, map, spriteSize);

	posEnemy.x += int(dir * speed);
	if (posEnemy.x < 0)              posEnemy.x = 0;
	if (posEnemy.x > mapW - spriteSize) posEnemy.x = mapW - spriteSize;

	bool hitWall = false;
	if (dir > 0 && map->collisionMoveRight(posEnemy, size, true))
	{
		posEnemy.x -= int(speed);
		hitWall = true;
	}
	else if (dir < 0 && map->collisionMoveLeft(posEnemy, size, true))
	{
		posEnemy.x += int(speed);
		hitWall = true;
	}

	if (hitWall || posEnemy.x <= 0 || posEnemy.x >= mapW - spriteSize)
	{
		dir = -dir;
		if (rand() % 3 == 0)
			dir = (targetPos.x > posEnemy.x) ? 1.f : -1.f;
	}

	sprite->changeAnimation((dir > 0) ? WALK_RIGHT : WALK_LEFT);
}
