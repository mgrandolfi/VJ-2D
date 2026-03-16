#ifndef _ENEMY_INCLUDE
#define _ENEMY_INCLUDE


#include "Sprite.h"
#include "TileMap.h"

enum EnemyTypes
{
	PIOLIN, LUCAS, SILVESTRE, TASMANIA
};


class Enemy
{

public:
	Enemy();
	~Enemy();

public:
	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram, EnemyTypes enemyType);
	void update(int deltaTime);
	void render();
	
	void setTileMap(TileMap *tileMap);
	void setPosition(const glm::vec2 &pos);

    //funciones agregadas de los enemigos
    void patrolMovement(int deltaTime);
	void chasingPlayer_Lucas();
    void chasingPlayer_Silvestre();
    void chasingPlayer_Tasmania();

private:
	glm::ivec2 tileMapDispl, posEnemy;
	int startY;
	Texture spritesheet;
	Sprite *sprite;
	TileMap *map;
    EnemyTypes type;

    float dir, speed, patrolMin, patrolMax, timeWait;
    int steps, maxSteps;
    bool isStopped;
};


#endif 


