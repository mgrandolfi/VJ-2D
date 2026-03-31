#ifndef _ENEMY_INCLUDE
#define _ENEMY_INCLUDE


#include "Sprite.h"
#include "TileMap.h"


enum EnemyTypes { PIOLIN, LUCAS, GHOST, TASMANIA };


class Enemy
{

public:
	Enemy();
	~Enemy();

public:
	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram, EnemyTypes enemyType, int tileSize);
	void update(int deltaTime);
	void render();

	void setTileMap(TileMap *tileMap);
	void setPosition(const glm::vec2 &pos);
	void setPatrolRange(float halfRangePixels);  // how far left/right from spawn
	void setTarget(const glm::ivec2 &pos) { targetPos = pos; }

	glm::ivec2 getPosition() const { return posEnemy; }
	bool       isAlive()     const { return alive; }
	void       kill()              { alive = false; }

private:
	void patrolMovement(int deltaTime);
	void chasingPlayer_Lucas(int deltaTime);
	void chasingPlayer_Tasmania(int deltaTime);

private:
	glm::ivec2 tileMapDispl, posEnemy;
	glm::ivec2 targetPos;

	Texture    spritesheet;
	Sprite    *sprite;
	TileMap   *map;

	EnemyTypes type;
	bool       alive;

	float      dir;         //  1 = right, -1 = left
	float      speed;
	float      patrolMin, patrolMax;
	float      timeWait;
	int        steps, maxSteps;
	bool       isStopped;

	// Used by Silvestre for ladder climbing
	int        spriteSize;

	float      climbVy;
	bool       onLadder;
	float      posXfrac;     // sub-pixel x accumulator
	float      tornadoYfrac; // sub-pixel y accumulator (Tasmania tornado)

	// Tasmania state machine
	enum TasmaniaState { TAS_IDLE, TAS_TORNADO, TAS_WALK };
	TasmaniaState tasState;
};


#endif // _ENEMY_INCLUDE
