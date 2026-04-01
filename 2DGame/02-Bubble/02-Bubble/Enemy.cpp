#include <cmath>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <GL/glew.h>
#include "Enemy.h"
#include "Game.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define ENEMY_FALL 4

#define TAS_DETECT_TILES 12   // tiles away before Tasmania notices the player
#define TAS_STOP_TILES    5   // tiles away to stop tornado and switch to walk
#define TAS_WALK_SPEED   2.0f // px/frame when walking

#define LUC_DETECT_TILES  5   // detection radius in tiles
#define LUC_JUMP_TILES    5   // jump height in tiles (matches player)
#define LUC_JUMP_STEP     5   // angle step per frame


enum EnemyAnims
{
	WALK_RIGHT, WALK_LEFT, STAND_RIGHT, STAND_LEFT,
	WALK_FRONT, WALK_BEHIND, LAND_RIGHT, LAND_LEFT,
	STAND_FRONT, STAND_BEHIND, DISAPPEAR, TORNADO      
};

Enemy::Enemy()
{
	sprite        = NULL;
	map           = NULL;
	alive         = true;
	spriteSize    = 32;
	posXfrac      = 0.f;
	tornadoYfrac = 0.f;
	tasState     = TAS_IDLE;
	lucIsJumping = false;
	lucJumpAngle = 0;
	lucStartY    = 0;
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
	climbVy       = 0.f;
	onLadder      = false;
	posXfrac      = 0.f;
	tornadoYfrac  = 0.f;
	tasState     = TAS_IDLE;
	lucIsJumping = false;
	lucJumpAngle = 0;
	lucStartY    = 0;

	switch (type)
	{
	case PIOLIN:    
		speed = 0.7f; 
		break;
	case LUCAS:
		speed = 0.8f;
		break;
	case GHOST: 
		speed = 1.0f; 
		break;
	case TASMANIA:  
		speed = 3.5f; 
		break;
	}

	spritesheet.loadFromFile("images/characters/enemies.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(glm::ivec2(spriteSize, spriteSize),
	                            glm::vec2(0.1f, 0.1f),
	                            &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(12);

	if (type == TASMANIA)
	{
		sprite->setAnimationSpeed(WALK_RIGHT, 6);
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.7f, 0.0f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.8f, 0.0f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.2f, 0.0f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.0f, 0.0f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.2f, 0.0f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.8f, 0.0f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.7f, 0.0f));

		sprite->setAnimationSpeed(WALK_LEFT, 6);
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.2f, 0.1f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.1f, 0.1f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.7f, 0.1f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.9f, 0.1f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.7f, 0.1f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.1f, 0.1f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.2f, 0.1f));

		sprite->setAnimationSpeed(STAND_RIGHT, 1);
		sprite->addKeyframe(STAND_RIGHT, glm::vec2(0.1f, 0.0f));

		sprite->setAnimationSpeed(STAND_LEFT, 1);
		sprite->addKeyframe(STAND_LEFT, glm::vec2(0.8f, 0.1f));

		sprite->setAnimationSpeed(STAND_FRONT, 1);
		sprite->addKeyframe(STAND_FRONT, glm::vec2(0.0f, 0.1f)); 

		sprite->setAnimationSpeed(STAND_BEHIND, 1);
		sprite->addKeyframe(STAND_BEHIND, glm::vec2(0.4f, 0.1f));

		sprite->setAnimationSpeed(TORNADO, 8);
		sprite->addKeyframe(TORNADO, glm::vec2(0.0f, 0.3f)); 
		sprite->addKeyframe(TORNADO, glm::vec2(0.1f, 0.3f));
		sprite->addKeyframe(TORNADO, glm::vec2(0.2f, 0.3f));
		sprite->addKeyframe(TORNADO, glm::vec2(0.3f, 0.3f));
		sprite->addKeyframe(TORNADO, glm::vec2(0.4f, 0.3f));
		sprite->addKeyframe(TORNADO, glm::vec2(0.5f, 0.3f));
		sprite->addKeyframe(TORNADO, glm::vec2(0.6f, 0.3f));
	}
	else if (type == LUCAS)
	{
		sprite->setAnimationSpeed(WALK_RIGHT, 6);
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.0f, 0.3f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.1f, 0.3f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.2f, 0.3f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.3f, 0.3f));

		sprite->setAnimationSpeed(WALK_LEFT, 6);
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.9f, 0.4f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.8f, 0.4f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.6f, 0.4f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.5f, 0.4f));

		sprite->setAnimationSpeed(STAND_RIGHT, 1);
		sprite->addKeyframe(STAND_RIGHT, glm::vec2(0.1f, 0.3f));

		sprite->setAnimationSpeed(STAND_LEFT, 1);
		sprite->addKeyframe(STAND_LEFT, glm::vec2(0.8f, 0.4f));

		sprite->setAnimationSpeed(WALK_FRONT, 3);
		sprite->addKeyframe(WALK_FRONT, glm::vec2(0.2f, 0.5f));
		sprite->addKeyframe(WALK_FRONT, glm::vec2(0.3f, 0.5f)); 

		sprite->setAnimationSpeed(WALK_BEHIND, 1);
		sprite->addKeyframe(WALK_BEHIND, glm::vec2(0.0f, 0.5f));
		sprite->addKeyframe(WALK_BEHIND, glm::vec2(0.1f, 0.5f));

		sprite->setAnimationSpeed(LAND_RIGHT, 1);
		sprite->addKeyframe(LAND_RIGHT, glm::vec2(0.5f, 0.3f)); 
		sprite->addKeyframe(LAND_RIGHT, glm::vec2(0.6f, 0.3f));
		sprite->addKeyframe(LAND_RIGHT, glm::vec2(0.7f, 0.3f));
		sprite->addKeyframe(LAND_RIGHT, glm::vec2(0.8f, 0.3f));   

		sprite->setAnimationSpeed(LAND_LEFT, 1);
		sprite->addKeyframe(LAND_LEFT, glm::vec2(0.4f, 0.4f)); 
		sprite->addKeyframe(LAND_LEFT, glm::vec2(0.3f, 0.4f));
		sprite->addKeyframe(LAND_LEFT, glm::vec2(0.2f, 0.4f));
		sprite->addKeyframe(LAND_LEFT, glm::vec2(0.1f, 0.4f));   

		sprite->setAnimationSpeed(DISAPPEAR, 6);
		sprite->addKeyframe(DISAPPEAR, glm::vec2(0.4f, 0.5f)); 
		sprite->addKeyframe(DISAPPEAR, glm::vec2(0.5f, 0.5f));
		sprite->addKeyframe(DISAPPEAR, glm::vec2(0.6f, 0.5f));
		sprite->addKeyframe(DISAPPEAR, glm::vec2(0.7f, 0.5f));
	}
	else if (type == GHOST)
	{
		sprite->setAnimationSpeed(WALK_RIGHT, 5);
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.0f, 0.7f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.1f, 0.7f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.2f, 0.7f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.3f, 0.7f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.4f, 0.7f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.5f, 0.7f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.6f, 0.7f));

		sprite->setAnimationSpeed(WALK_LEFT, 5);
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.9f, 0.8f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.8f, 0.8f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.7f, 0.8f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.6f, 0.8f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.5f, 0.8f));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.4f, 0.8f));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.3f, 0.8f));

		sprite->setAnimationSpeed(STAND_RIGHT, 1);
		sprite->addKeyframe(STAND_RIGHT, glm::vec2(0.0f, 0.7f));

		sprite->setAnimationSpeed(STAND_LEFT, 1);
		sprite->addKeyframe(STAND_LEFT, glm::vec2(0.0f, 0.8f));
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
	case PIOLIN:
	case GHOST:
		patrolMovement(deltaTime);
		break;
	case LUCAS:
		chasingPlayer_Lucas(deltaTime);
		break;
	case TASMANIA:
		chasingPlayer_Tasmania(deltaTime);
		break;
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
	const int ts   = map->getTileSize();
	const int half = spriteSize / 2;
	const glm::ivec2 size(spriteSize, spriteSize);

	// Distancia al jugador en tiles (Chebyshev)
	int distX = abs((posEnemy.x + half) - (targetPos.x + half)) / ts;
	int distY = abs((posEnemy.y + half) - (targetPos.y + half)) / ts;

	if (std::max(distX, distY) > LUC_DETECT_TILES)
	{
		patrolMovement(deltaTime);
		return;
	}

	// --- Chase mode ---
	int dx = targetPos.x - posEnemy.x;  // + = jugador a la derecha
	int dy = targetPos.y - posEnemy.y;  // + = jugador abajo

	bool onJump   = map->isOnJump(posEnemy, size);

	// --- Arco de salto activo ---
	if (lucIsJumping)
	{
		lucJumpAngle += LUC_JUMP_STEP;
		if (lucJumpAngle >= 180)
		{
			lucIsJumping = false;
			posEnemy.y   = lucStartY;
			map->collisionMoveDown(posEnemy, size, &posEnemy.y);
		}
		else
		{
			int jumpH  = spriteSize * LUC_JUMP_TILES;
			posEnemy.y = int(lucStartY - jumpH * sin(lucJumpAngle * M_PI / 180.0));
			if (lucJumpAngle < 90)
			{
				int dummy = posEnemy.y;
				if (map->collisionMoveUp(posEnemy, size, &dummy))
				{
					posEnemy.y   = dummy;
					lucIsJumping = false;
				}
			}
			else if (map->collisionMoveDown(posEnemy, size, &posEnemy.y))
				lucIsJumping = false;
		}
		// Deriva horizontal mientras salta
		posXfrac += (dx > 0 ? 1.f : -1.f) * speed;
		int step  = (int)posXfrac;
		posXfrac -= (float)step;
		posEnemy.x += step;
		if (dx > 0 && map->collisionMoveRight(posEnemy, size)) posEnemy.x -= step;
		if (dx < 0 && map->collisionMoveLeft (posEnemy, size)) posEnemy.x -= step;
		int anim = (dx >= 0) ? WALK_RIGHT : WALK_LEFT;
		if (sprite->animation() != anim) sprite->changeAnimation(anim);
		return;
	}

	// --- Gravedad ---
	applyGravity(posEnemy, map, spriteSize);

	// --- Jugador encima: usar escalera o trampolín ---
	if (dy < -ts)
	{
		if (onJump)
		{
			lucIsJumping = true;
			lucJumpAngle = 0;
			lucStartY    = posEnemy.y;
			return;
		}
	}

	// --- Movimiento horizontal hacia el jugador ---
	if (dx != 0)
	{
		posXfrac += (dx > 0 ? 1.f : -1.f) * speed;
		int step  = (int)posXfrac;
		posXfrac -= (float)step;
		posEnemy.x += step;

		bool blocked = false;
		if (dx > 0)
		{
			if (map->collisionMoveRight(posEnemy, size)) { posEnemy.x -= step; posXfrac = 0.f; blocked = true; }
			if (sprite->animation() != WALK_RIGHT) sprite->changeAnimation(WALK_RIGHT);
		}
		else
		{
			if (map->collisionMoveLeft(posEnemy, size)) { posEnemy.x -= step; posXfrac = 0.f; blocked = true; }
			if (sprite->animation() != WALK_LEFT) sprite->changeAnimation(WALK_LEFT);
		}

		// Rampa: subir pendiente como el jugador
		if (!blocked && map->isOnCliff(posEnemy, size))
			posEnemy.y -= (int)speed;

		// Trampolín al moverse horizontalmente
		if (onJump && !lucIsJumping)
		{
			lucIsJumping = true;
			lucJumpAngle = 0;
			lucStartY    = posEnemy.y;
		}
	}
	else
	{
		// Parado pero detectando al jugador: mirar hacia él
		int standAnim = (targetPos.x >= posEnemy.x) ? STAND_RIGHT : STAND_LEFT;
		if (sprite->animation() != STAND_RIGHT && sprite->animation() != STAND_LEFT)
			sprite->changeAnimation(standAnim);
		else if (sprite->animation() != standAnim)
			sprite->changeAnimation(standAnim);
	}
}

void Enemy::chasingPlayer_Tasmania(int deltaTime)
{
	const float ts   = (float)spriteSize;
	const float dx   = (float)(targetPos.x - posEnemy.x);
	const float dy   = (float)(targetPos.y - posEnemy.y);
	const float dist = sqrtf(dx * dx + dy * dy);

	if (dist > TAS_DETECT_TILES * ts)
	{
		if (tasState != TAS_IDLE)
		{
			tasState = TAS_IDLE;
			sprite->changeAnimation(STAND_FRONT);
		}
		return;
	}

	if (dist > TAS_STOP_TILES * ts)
	{
		if (tasState != TAS_TORNADO)
		{
			tasState     = TAS_TORNADO;
			tornadoYfrac = 0.f;
			posXfrac     = 0.f;
			sprite->changeAnimation(TORNADO);
		}
		float nx = dx / dist;
		float ny = dy / dist;

		posXfrac     += nx * speed;
		tornadoYfrac += ny * speed;
		int stepX = (int)posXfrac;
		int stepY = (int)tornadoYfrac;
		posXfrac     -= (float)stepX;
		tornadoYfrac -= (float)stepY;
		posEnemy.x   += stepX;
		posEnemy.y   += stepY;
		return;
	}

	if (tasState != TAS_WALK)
	{
		tasState = TAS_WALK;
		posXfrac = 0.f;
	}

	const glm::ivec2 size(spriteSize, spriteSize);
	applyGravity(posEnemy, map, spriteSize);

	dir = (dx > 0.f) ? 1.f : -1.f;
	posXfrac += dir * TAS_WALK_SPEED;
	int step  = (int)posXfrac;
	posXfrac -= (float)step;
	posEnemy.x += step;

	if (dir > 0.f)
	{
		if (map->collisionMoveRight(posEnemy, size, true))
		{
			posEnemy.x -= step;
			posXfrac = 0.f;
		}
		if (sprite->animation() != WALK_RIGHT)
			sprite->changeAnimation(WALK_RIGHT);
	}
	else
	{
		if (map->collisionMoveLeft(posEnemy, size, true))
		{
			posEnemy.x -= step;
			posXfrac = 0.f;
		}
		if (sprite->animation() != WALK_LEFT)
			sprite->changeAnimation(WALK_LEFT);
	}
}
