#include <cmath>
#include <cstdlib>
#include <iostream>
#include <GL/glew.h>
#include "Enemy.h"
#include "Game.h"


#define ENEMY_FALL 4


enum EnemyAnims
{
	WALK_RIGHT,
	WALK_LEFT,
	STAND_RIGHT,
	STAND_LEFT,
	WALK_FRONT,
	WALK_BEHIND,
	LAND_RIGHT,
	LAND_LEFT,
	DISAPPEAR,   
	TORNADO      
};

Enemy::Enemy()
{
	sprite     = NULL;
	map        = NULL;
	alive      = true;
	spriteSize = 32;
	posXfrac   = 0.f;
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
	maxSteps   = 400;
	climbVy    = 0.f;
	onLadder   = false;
	posXfrac   = 0.f;

	switch (type)
	{
	case PIOLIN:    speed = 0.7f; break;
	case LUCAS:     speed = 2.0f; break;
	case SILVESTRE: speed = 1.0f; break;
	case TASMANIA:  speed = 3.5f; break;
	default:        speed = 1.5f; break;
	}

	spritesheet.loadFromFile("images/characters/enemies.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(glm::ivec2(spriteSize, spriteSize),
	                            glm::vec2(0.1f, 0.1f),
	                            &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(10);

	if (type == TASMANIA)
	{
		sprite->setAnimationSpeed(WALK_RIGHT, 6);
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.0f, 0.0f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.1f, 0.0f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.2f, 0.0f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.3f, 0.0f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.4f, 0.0f));

		sprite->setAnimationSpeed(WALK_LEFT, 6);
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.0f, 0.1f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.1f, 0.1f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.2f, 0.1f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.3f, 0.1f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.4f, 0.1f));

		sprite->setAnimationSpeed(STAND_RIGHT, 1);
		sprite->addKeyframe(STAND_RIGHT, glm::vec2(0.0f, 0.0f));

		sprite->setAnimationSpeed(STAND_LEFT, 1);
		sprite->addKeyframe(STAND_LEFT, glm::vec2(0.0f, 0.1f));

		sprite->setAnimationSpeed(WALK_FRONT, 1);
		sprite->addKeyframe(WALK_FRONT, glm::vec2(0.0f, 0.0f)); // TODO

		sprite->setAnimationSpeed(WALK_BEHIND, 1);
		sprite->addKeyframe(WALK_BEHIND, glm::vec2(0.0f, 0.0f)); // TODO

		sprite->setAnimationSpeed(LAND_RIGHT, 1);
		sprite->addKeyframe(LAND_RIGHT, glm::vec2(0.0f, 0.0f)); // TODO

		sprite->setAnimationSpeed(LAND_LEFT, 1);
		sprite->addKeyframe(LAND_LEFT, glm::vec2(0.0f, 0.1f)); // TODO

		sprite->setAnimationSpeed(DISAPPEAR, 6);
		sprite->addKeyframe(DISAPPEAR, glm::vec2(0.0f, 0.8f)); // TODO
		sprite->addKeyframe(DISAPPEAR, glm::vec2(0.1f, 0.8f));
		sprite->addKeyframe(DISAPPEAR, glm::vec2(0.2f, 0.8f));
		sprite->addKeyframe(DISAPPEAR, glm::vec2(0.3f, 0.8f));
		sprite->addKeyframe(DISAPPEAR, glm::vec2(0.4f, 0.8f));

		sprite->setAnimationSpeed(TORNADO, 8);
		sprite->addKeyframe(TORNADO, glm::vec2(0.0f, 0.0f)); // TODO
		sprite->addKeyframe(TORNADO, glm::vec2(0.1f, 0.0f));
		sprite->addKeyframe(TORNADO, glm::vec2(0.2f, 0.0f));
		sprite->addKeyframe(TORNADO, glm::vec2(0.3f, 0.0f));
		sprite->addKeyframe(TORNADO, glm::vec2(0.4f, 0.0f));
	}
	else if (type == LUCAS)
	{
		sprite->setAnimationSpeed(WALK_RIGHT, 6);
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.0f, 0.2f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.1f, 0.2f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.2f, 0.2f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.3f, 0.2f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.4f, 0.2f));

		sprite->setAnimationSpeed(WALK_LEFT, 6);
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.0f, 0.3f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.1f, 0.3f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.2f, 0.3f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.3f, 0.3f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.4f, 0.3f));

		sprite->setAnimationSpeed(STAND_RIGHT, 1);
		sprite->addKeyframe(STAND_RIGHT, glm::vec2(0.0f, 0.2f));

		sprite->setAnimationSpeed(STAND_LEFT, 1);
		sprite->addKeyframe(STAND_LEFT, glm::vec2(0.0f, 0.3f));

		sprite->setAnimationSpeed(WALK_FRONT, 1);
		sprite->addKeyframe(WALK_FRONT, glm::vec2(0.0f, 0.2f)); // TODO

		sprite->setAnimationSpeed(WALK_BEHIND, 1);
		sprite->addKeyframe(WALK_BEHIND, glm::vec2(0.0f, 0.2f)); // TODO

		sprite->setAnimationSpeed(LAND_RIGHT, 1);
		sprite->addKeyframe(LAND_RIGHT, glm::vec2(0.0f, 0.2f)); // TODO

		sprite->setAnimationSpeed(LAND_LEFT, 1);
		sprite->addKeyframe(LAND_LEFT, glm::vec2(0.0f, 0.3f)); // TODO

		sprite->setAnimationSpeed(DISAPPEAR, 6);
		sprite->addKeyframe(DISAPPEAR, glm::vec2(0.0f, 0.8f)); // TODO
		sprite->addKeyframe(DISAPPEAR, glm::vec2(0.1f, 0.8f));
		sprite->addKeyframe(DISAPPEAR, glm::vec2(0.2f, 0.8f));
		sprite->addKeyframe(DISAPPEAR, glm::vec2(0.3f, 0.8f));
		sprite->addKeyframe(DISAPPEAR, glm::vec2(0.4f, 0.8f));

		sprite->setAnimationSpeed(TORNADO, 1);
		sprite->addKeyframe(TORNADO, glm::vec2(0.0f, 0.2f)); // N/A for Lucas
	}
	else if (type == SILVESTRE)
	{
		sprite->setAnimationSpeed(WALK_RIGHT, 6);
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.0f, 0.4f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.1f, 0.4f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.2f, 0.4f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.3f, 0.4f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.4f, 0.4f));

		sprite->setAnimationSpeed(WALK_LEFT, 6);
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.0f, 0.5f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.1f, 0.5f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.2f, 0.5f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.3f, 0.5f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.4f, 0.5f));

		sprite->setAnimationSpeed(STAND_RIGHT, 1);
		sprite->addKeyframe(STAND_RIGHT, glm::vec2(0.0f, 0.4f));

		sprite->setAnimationSpeed(STAND_LEFT, 1);
		sprite->addKeyframe(STAND_LEFT, glm::vec2(0.0f, 0.5f));

		sprite->setAnimationSpeed(WALK_FRONT, 1);
		sprite->addKeyframe(WALK_FRONT, glm::vec2(0.0f, 0.4f)); // TODO

		sprite->setAnimationSpeed(WALK_BEHIND, 1);
		sprite->addKeyframe(WALK_BEHIND, glm::vec2(0.0f, 0.4f)); // TODO

		sprite->setAnimationSpeed(LAND_RIGHT, 1);
		sprite->addKeyframe(LAND_RIGHT, glm::vec2(0.0f, 0.4f)); // TODO

		sprite->setAnimationSpeed(LAND_LEFT, 1);
		sprite->addKeyframe(LAND_LEFT, glm::vec2(0.0f, 0.5f)); // TODO

		sprite->setAnimationSpeed(DISAPPEAR, 6);
		sprite->addKeyframe(DISAPPEAR, glm::vec2(0.0f, 0.8f)); // TODO
		sprite->addKeyframe(DISAPPEAR, glm::vec2(0.1f, 0.8f));
		sprite->addKeyframe(DISAPPEAR, glm::vec2(0.2f, 0.8f));
		sprite->addKeyframe(DISAPPEAR, glm::vec2(0.3f, 0.8f));
		sprite->addKeyframe(DISAPPEAR, glm::vec2(0.4f, 0.8f));

		sprite->setAnimationSpeed(TORNADO, 1);
		sprite->addKeyframe(TORNADO, glm::vec2(0.0f, 0.4f)); // N/A for Silvestre
	}
	else if (type == PIOLIN)
	{
		sprite->setAnimationSpeed(WALK_RIGHT, 5);
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.0f, 0.6f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.1f, 0.6f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.2f, 0.6f));

		sprite->setAnimationSpeed(WALK_LEFT, 5);
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.3f, 0.6f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.4f, 0.6f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.5f, 0.6f));

		sprite->setAnimationSpeed(STAND_RIGHT, 3);
		sprite->addKeyframe(STAND_RIGHT, glm::vec2(0.1f, 0.6f));

		sprite->setAnimationSpeed(STAND_LEFT, 3);
		sprite->addKeyframe(STAND_LEFT, glm::vec2(0.4f, 0.6f));
	}
	sprite->changeAnimation(WALK_RIGHT);

	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x),
	                            float(tileMapDispl.y + posEnemy.y)));

	patrolMin = posEnemy.x - 300.f;
	if (patrolMin < 0.f) patrolMin = 0.f;
	patrolMax = posEnemy.x + 300.f;
}

void Enemy::update(int deltaTime)
{
	if (!alive) return;

	sprite->update(deltaTime);

	switch (type)
	{
	case PIOLIN:    patrolMovement(deltaTime);          break;
	case LUCAS:     chasingPlayer_Lucas(deltaTime);     break;
	case SILVESTRE: chasingPlayer_Ghost(deltaTime);  break;
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
	patrolMin = posEnemy.x - 300.f;
	if (patrolMin < 0.f) patrolMin = 0.f;
	patrolMax = posEnemy.x + 300.f;
}

void Enemy::setPatrolRange(float halfRangePixels)
{
	patrolMin = posEnemy.x - halfRangePixels;
	if (patrolMin < 0.f) patrolMin = 0.f;
	patrolMax = posEnemy.x + halfRangePixels;
}

static void applyGravity(glm::ivec2 &pos, TileMap *map, int sz)
{
	if (!map) return;
	glm::ivec2 size(sz, sz);
	pos.y += ENEMY_FALL;
	if (map->collisionMoveDown(pos, size, &pos.y)) return;

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

	applyGravity(posEnemy, map, spriteSize);

	posXfrac += dir * speed;
	int dx = (int)posXfrac;
	posXfrac -= (float)dx;
	posEnemy.x += dx;

	if (posEnemy.x < 0) {
		posEnemy.x = 0;
		patrolMin = 0.f;
		posXfrac = 0.f;
		dir = 1.f;
		sprite->changeAnimation(WALK_RIGHT);
	}
	else if (posEnemy.x > mapW - spriteSize) {
		posEnemy.x = mapW - spriteSize;
		patrolMax = (float)(mapW - spriteSize);
		posXfrac = 0.f;
		dir = -1.f;
		sprite->changeAnimation(WALK_LEFT);
	}

	//Si el enemigo se choca con alguna pared, dar la vuelta y actualizar el límite
	if (dir > 0 && map->collisionMoveRight(posEnemy, size, true))
	{
		posEnemy.x -= dx;
		posXfrac = 0.f;
		patrolMax = (float)posEnemy.x;  // la pared pasa a ser el nuevo límite derecho
		posEnemy.x = (int)patrolMax - 1;
		dir = -1.f;
		sprite->changeAnimation(WALK_LEFT);
	}
	else if (dir < 0 && map->collisionMoveLeft(posEnemy, size, true))
	{
		posEnemy.x -= dx;
		posXfrac = 0.f;
		patrolMin = (float)posEnemy.x;  // la pared pasa a ser el nuevo límite izquierdo
		posEnemy.x = (int)patrolMin + 1;
		dir = 1.f;
		sprite->changeAnimation(WALK_RIGHT);
	}
	//Si llega a toda su ruta completa, cambiar de dirección y recorrerla de nuevo
	if (posEnemy.x >= (int)patrolMax) {
		posEnemy.x = (int)patrolMax - 1;  // snap inside so condition won't re-trigger
		dir = -1.f; posXfrac = 0.f;
		sprite->changeAnimation(WALK_LEFT);
	}
	else if (posEnemy.x <= (int)patrolMin) {
		posEnemy.x = (int)patrolMin + 1;  // snap inside
		dir =  1.f; posXfrac = 0.f;
		sprite->changeAnimation(WALK_RIGHT);
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

void Enemy::chasingPlayer_Ghost(int deltaTime)
{
	// if (!map) return;
	// const glm::ivec2 size(spriteSize, spriteSize);
	// const int mapW = map->getMapWidth() * map->getTileSize();
	// const int mapH = map->getMapHeight() * map->getTileSize();

	// onLadder = map->isOnLadder(posEnemy, size);

	// if (onLadder)
	// {
	// 	int dy = targetPos.y - posEnemy.y;
	// 	if (abs(dy) > 4)
	// 	{
	// 		climbVy = (dy > 0) ? speed : -speed;
	// 		posEnemy.y += int(climbVy);
	// 		if (posEnemy.y < 0) posEnemy.y = 0;
	// 		if (posEnemy.y > mapH - spriteSize) posEnemy.y = mapH - spriteSize;
	// 		int dx = targetPos.x - posEnemy.x;
	// 		dir = (dx > 0) ? 1.f : -1.f;
	// 		sprite->changeAnimation((dir > 0) ? WALK_RIGHT : WALK_LEFT);
	// 	}
	// 	else
	// 	{
	// 		int dx = targetPos.x - posEnemy.x;
	// 		dir = (dx > 0) ? 1.f : -1.f;
	// 		sprite->changeAnimation((dir > 0) ? WALK_RIGHT : WALK_LEFT);
	// 		posEnemy.x += int(dir * speed);
	// 		if (posEnemy.x < 0)              posEnemy.x = 0;
	// 		if (posEnemy.x > mapW - spriteSize) posEnemy.x = mapW - spriteSize;
	// 		// On ladder rungs: do not block on ladder cells horizontally
	// 		if (dir > 0 && map->collisionMoveRight(posEnemy, size, false))
	// 			posEnemy.x -= int(speed);
	// 		else if (dir < 0 && map->collisionMoveLeft(posEnemy, size, false))
	// 			posEnemy.x += int(speed);
	// 	}
	// }
	// else
	// {
	// 	applyGravity(posEnemy, map, spriteSize);

	// 	int dx = targetPos.x - posEnemy.x;
	// 	dir = (dx > 0) ? 1.f : -1.f;
	// 	sprite->changeAnimation((dir > 0) ? WALK_RIGHT : WALK_LEFT);

	// 	posEnemy.x += int(dir * speed);
	// 	if (posEnemy.x < 0)              posEnemy.x = 0;
	// 	if (posEnemy.x > mapW - spriteSize) posEnemy.x = mapW - spriteSize;

	// 	if (dir > 0 && map->collisionMoveRight(posEnemy, size, true))
	// 		posEnemy.x -= int(speed);
	// 	else if (dir < 0 && map->collisionMoveLeft(posEnemy, size, true))
	// 		posEnemy.x += int(speed);
	// }
}

void Enemy::chasingPlayer_Tasmania(int deltaTime)
{
	// if (!map) return;
	// const glm::ivec2 size(spriteSize, spriteSize);
	// const int mapW = map->getMapWidth() * map->getTileSize();

	// applyGravity(posEnemy, map, spriteSize);

	// posEnemy.x += int(dir * speed);
	// if (posEnemy.x < 0)              posEnemy.x = 0;
	// if (posEnemy.x > mapW - spriteSize) posEnemy.x = mapW - spriteSize;

	// bool hitWall = false;
	// if (dir > 0 && map->collisionMoveRight(posEnemy, size, true))
	// {
	// 	posEnemy.x -= int(speed);
	// 	hitWall = true;
	// }
	// else if (dir < 0 && map->collisionMoveLeft(posEnemy, size, true))
	// {
	// 	posEnemy.x += int(speed);
	// 	hitWall = true;
	// }

	// if (hitWall || posEnemy.x <= 0 || posEnemy.x >= mapW - spriteSize)
	// {
	// 	dir = -dir;
	// 	if (rand() % 3 == 0)
	// 		dir = (targetPos.x > posEnemy.x) ? 1.f : -1.f;
	// }

	// sprite->changeAnimation((dir > 0) ? WALK_RIGHT : WALK_LEFT);
}
