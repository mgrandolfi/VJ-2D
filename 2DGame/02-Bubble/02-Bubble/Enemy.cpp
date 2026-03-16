#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Enemy.h"
#include "Game.h"

enum EnemyAnims
{
	STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT, //4
	STAND_FRONT, CLIMB, ENTER, EXIT, DISAPPEAR, //5
	START_JUMP_LEFT, START_JUMP_RIGHT, //2
	JUMP_LEFT, JUMP_RIGHT, LAND_LEFT, LAND_RIGHT, //4
};

enum EnemyTypes
{
	PIOLIN, LUCAS, SILVESTRE, TASMANIA
};

Enemy::Enemy()
{
	sprite = NULL;
	map = NULL;
}

Enemy::~Enemy()
{
	if (sprite != NULL)
		delete sprite;
}

void Enemy::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram, EnemyTypes enemyType)
{
	//Variables necesarias
	type = enemyType;
	dir = 1.f;
	speed = 2.f;
	isStopped = false;
	timeWait = 0;

	spritesheet.loadFromFile("images/enemies.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(0.09, 0.09), &spritesheet, &shaderProgram); // donde 1/10 = 0.1, porque son 10 filas x 10 columnas de sprites en la imagen
	sprite->setNumberAnimations(11); //REVIEW - son la cantidad de animaciones que hay en el enum

	if (type == PIOLIN) {
		//cargar sprite de piolin
	}
	else if (type == LUCAS) {
		//cargar sprite de lucas
	}
	else if (type == SILVESTRE) {
		//cargar sprite de silvestre
	}
	else if (type == TASMANIA) {
		//cargar sprite de tasmania
	}
	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
	
	//REVIEW - Setear las variables de patrulla (caso PIOLIN)
	patrolMin = posEnemy.x - 100.f;
	patrolMax = posEnemy.x + 100.f;
}

void Enemy::update(int deltaTime)
{
	sprite->update(deltaTime);

	switch (type) {
		case PIOLIN:
			patrolMovement(deltaTime);
			break;
		case LUCAS:
			chasingPlayer_Lucas();
			break;
		case SILVESTRE:
			chasingPlayer_Silvestre();
			break;
		case TASMANIA:
			chasingPlayer_Tasmania();
			break;
	}
}

void Enemy::render()
{
	sprite->render();
}

void Enemy::setTileMap(TileMap *tileMap)
{
	map = tileMap;
}

void Enemy::setPosition(const glm::vec2 &pos)
{
	posEnemy = pos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
}

//movimiento para PIOLIN
void Enemy::patrolMovement(int deltaTime) {
	if (isStopped) {
		timeWait -= deltaTime; 
		if (timeWait <= 0) {
			steps = 0;
			isStopped = false;
		}
		return;
	}
	posEnemy.x += dir * speed; 
	++steps;

	if (posEnemy.x >= patrolMax) {
		posEnemy.x = patrolMax;
		dir = -1.0f;
	}
	else if (posEnemy.x <= patrolMin) {
		posEnemy.x = patrolMin;
		dir = 1.0f;
	}

	if (steps >= maxSteps) {
		timeWait = 1000;
		isStopped = true;
	}
}

void Enemy::chasingPlayer_Lucas() {

}

void Enemy::chasingPlayer_Silvestre() {

}

void Enemy::chasingPlayer_Tasmania() {

}



