#ifndef _ENEMY_INCLUDE
#define _ENEMY_INCLUDE

#include <vector>
#include "Sprite.h"
#include "TileMap.h"


enum EnemyTypes { PIOLIN, LUCAS, GHOST, TASMANIA };


class Enemy {

public:
	Enemy();
	~Enemy();

public:
	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram, EnemyTypes enemyType, int tileSize);
	void update(int deltaTime);
	void render();

	void setTileMap(TileMap *tileMap);
	void setPosition(const glm::vec2 &pos);
	void setPatrolRange(float halfRangePixels);  //rango del movimiento de patrulla (para ghost y piolin)
	void setTarget(const glm::ivec2 &pos) { 
		targetPos = pos; 
	}

	glm::ivec2 getPosition() const { 
		return posEnemy; 
	}
	bool isAlive() const { 
		return alive; 
	}
	void kill() { 
		alive = false; 
	}

private:
	//funciones de los diferentes comportamientos de los enemigos
	void patrolMovement(int deltaTime);
	void chasingPlayer_Lucas(int deltaTime);
	void chasingPlayer_Tasmania(int deltaTime);

private:
	glm::ivec2 tileMapDispl, posEnemy;
	glm::ivec2 targetPos;

	Texture spritesheet;
	Sprite *sprite;
	TileMap *map;

	EnemyTypes type;
	bool alive;

	float dir;         //  1 = right, -1 = left
	float speed, patrolMin, patrolMax, timeWait;
	int steps, maxSteps;
	bool isStopped;

	int spriteSize;

	float climbVy;
	bool onLadder;
	float posXfrac; //para acumular pixeles para el movimiento horizontal
	float tornadoYfrac; //para acumular pixeles para el tornado

	//estados del tasmania: si no detecta al jugador no hace nada, tornado para perseguir y persecucion normal
	enum TasmaniaState { TAS_IDLE, TAS_TORNADO, TAS_WALK };
	TasmaniaState tasState;

	bool lucIsJumping;
	int  lucJumpAngle;
	int  lucStartY;
};


#endif 
