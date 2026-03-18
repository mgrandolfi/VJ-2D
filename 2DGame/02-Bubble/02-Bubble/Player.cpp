#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Player.h"
#include "Game.h"


#define JUMP_ANGLE_STEP 4
#define JUMP_HEIGHT 96
#define FALL_STEP 4



enum PlayerAnims
{
	STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT, //4
	STAND_FRONT, CLIMB, ENTER, EXIT, DISAPPEAR, //5
	START_JUMP_LEFT, START_JUMP_RIGHT, //2
	JUMP_LEFT, JUMP_RIGHT, LAND_LEFT, LAND_RIGHT, //4
	DIE_LEFT, DIE_RIGHT, BOMB_LEFT, BOMB_RIGHT, //4
	HURT_LEFT, HURT_RIGHT, OPEN_CHEST_LEFT, OPEN_CHEST_RIGHT //4
};
//falta acciones para las animaciones de velocidad y obtencion de poderes

Player::Player()
{
	sprite = NULL;
	map = NULL;
}

Player::~Player()
{
	if (sprite != NULL)
		delete sprite;
}

void Player::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	spritesheet.loadFromFile("images/bugs.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(0.09, 0.09), &spritesheet, &shaderProgram); // donde 1/10 = 0.1, porque son 10 filas x 10 columnas de sprites en la imagen
	sprite->setNumberAnimations(23); //son la cantidad de animaciones que hay en el enum

	sprite ->setAnimationSpeed(STAND_FRONT, 1);
	sprite -> addKeyframe(STAND_FRONT, glm::vec2(0.4, 0.0f));

	sprite ->setAnimationSpeed(STAND_LEFT, 1);
	sprite -> addKeyframe(STAND_LEFT, glm::vec2(0.0, 0.1f));

	sprite ->setAnimationSpeed(STAND_RIGHT, 1);
	sprite -> addKeyframe(STAND_RIGHT, glm::vec2(0.9, 0.1f));

	sprite->setAnimationSpeed(MOVE_LEFT, 8);
	sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.0f, 0.1f));
	sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.1f, 0.1f));
	sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.2f, 0.1f));
	sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.3f, 0.1f));
	sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.4f, 0.1f));
	
	sprite->setAnimationSpeed(MOVE_RIGHT, 8);
	sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.5f, 0.1f));
	sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.6f, 0.1f));
	sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.7f, 0.1f));
	sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.8f, 0.1f));
	sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.9f, 0.1f));

	sprite->setAnimationSpeed(ENTER, 4);
	sprite->addKeyframe(ENTER, glm::vec2(0.0f, 0.0f));
	sprite->addKeyframe(ENTER, glm::vec2(0.1f, 0.0f));

	sprite->setAnimationSpeed(CLIMB, 4);
	sprite->addKeyframe(CLIMB, glm::vec2(0.8f, 0.0f));
	sprite->addKeyframe(CLIMB, glm::vec2(0.9f, 0.0f));

	sprite->setAnimationSpeed(EXIT, 4);
	sprite->addKeyframe(EXIT, glm::vec2(0.2f, 0.0f));
	sprite->addKeyframe(EXIT, glm::vec2(0.3f, 0.0f));

	sprite->setAnimationSpeed(DISAPPEAR, 3);
	sprite->addKeyframe(DISAPPEAR, glm::vec2(0.5f, 0.0f));
	sprite->addKeyframe(DISAPPEAR, glm::vec2(0.6f, 0.0f));
	sprite->addKeyframe(DISAPPEAR, glm::vec2(0.7f, 0.0f));

	sprite->setAnimationSpeed(START_JUMP_LEFT, 2);
	sprite->addKeyframe(START_JUMP_LEFT, glm::vec2(0.7f, 0.4f));

	sprite->setAnimationSpeed(JUMP_LEFT, 4);
	sprite->addKeyframe(JUMP_LEFT, glm::vec2(0.6f, 0.4f));

	sprite->setAnimationSpeed(START_JUMP_RIGHT, 2);
	sprite->addKeyframe(START_JUMP_RIGHT, glm::vec2(0.0f, 0.4f));

	sprite->setAnimationSpeed(JUMP_RIGHT, 4);
	sprite->addKeyframe(JUMP_RIGHT, glm::vec2(0.1f, 0.4f));

	sprite->setAnimationSpeed(LAND_LEFT, 2);
	sprite->addKeyframe(LAND_LEFT, glm::vec2(0.5f, 0.4f));
	sprite->addKeyframe(LAND_LEFT, glm::vec2(0.4f, 0.4f));

	sprite->setAnimationSpeed(LAND_RIGHT, 2);
	sprite->addKeyframe(LAND_RIGHT, glm::vec2(0.2f, 0.4f));
	sprite->addKeyframe(LAND_RIGHT, glm::vec2(0.3f, 0.4f));

	sprite->setAnimationSpeed(DIE_LEFT, 2);
	sprite->addKeyframe(DIE_LEFT, glm::vec2(0.4f, 0.3f));
	
	sprite->setAnimationSpeed(HURT_LEFT, 2);
	sprite->addKeyframe(HURT_LEFT, glm::vec2(0.3f, 0.3f));

	sprite->setAnimationSpeed(DIE_RIGHT, 2);
	sprite->addKeyframe(DIE_RIGHT, glm::vec2(0.5f, 0.3f));

	sprite->setAnimationSpeed(HURT_RIGHT, 2);
	sprite->addKeyframe(HURT_RIGHT, glm::vec2(0.6f, 0.3f));

	sprite->setAnimationSpeed(BOMB_LEFT, 4);
	sprite->addKeyframe(BOMB_LEFT, glm::vec2(0.2f, 0.3f));
	sprite->addKeyframe(BOMB_LEFT, glm::vec2(0.1f, 0.3f));
	sprite->addKeyframe(BOMB_LEFT, glm::vec2(0.0f, 0.3f));

	sprite->setAnimationSpeed(BOMB_RIGHT, 4);
	sprite->addKeyframe(BOMB_RIGHT, glm::vec2(0.7f, 0.3f));
	sprite->addKeyframe(BOMB_RIGHT, glm::vec2(0.8f, 0.3f));
	sprite->addKeyframe(BOMB_RIGHT, glm::vec2(0.9f, 0.3f));

	sprite->setAnimationSpeed(OPEN_CHEST_LEFT, 5);
	sprite->addKeyframe(OPEN_CHEST_LEFT, glm::vec2(0.0f, 0.2f));
	sprite->addKeyframe(OPEN_CHEST_LEFT, glm::vec2(0.1f, 0.2f));
	sprite->addKeyframe(OPEN_CHEST_LEFT, glm::vec2(0.2f, 0.2f));
	sprite->addKeyframe(OPEN_CHEST_LEFT, glm::vec2(0.3f, 0.2f));
	
	sprite->setAnimationSpeed(OPEN_CHEST_RIGHT, 5);
	sprite->addKeyframe(OPEN_CHEST_RIGHT, glm::vec2(0.7f, 0.2f));
	sprite->addKeyframe(OPEN_CHEST_RIGHT, glm::vec2(0.6f, 0.2f));
	sprite->addKeyframe(OPEN_CHEST_RIGHT, glm::vec2(0.5f, 0.2f));
	sprite->addKeyframe(OPEN_CHEST_RIGHT, glm::vec2(0.4f, 0.2f));

	sprite->changeAnimation(STAND_FRONT);
	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
	
}

void Player::update(int deltaTime)
{
	sprite->update(deltaTime);
	if(Game::instance().getKey(GLFW_KEY_LEFT))
	{
		if(sprite->animation() != MOVE_LEFT)
			sprite->changeAnimation(MOVE_LEFT);
		posPlayer.x -= 2;
		if(map->collisionMoveLeft(posPlayer, glm::ivec2(32, 32)))
		{
			posPlayer.x += 2;
			sprite->changeAnimation(STAND_LEFT);
		}
	}
	else if(Game::instance().getKey(GLFW_KEY_RIGHT))
	{
		if(sprite->animation() != MOVE_RIGHT)
			sprite->changeAnimation(MOVE_RIGHT);
		posPlayer.x += 2;
		if(map->collisionMoveRight(posPlayer, glm::ivec2(32, 32)))
		{
			posPlayer.x -= 2;
			sprite->changeAnimation(STAND_RIGHT);
		}
	}
	else if (Game::instance().getKey(GLFW_KEY_UP)) {
		sprite->changeAnimation(STAND_FRONT);
	}
	else
	{
		if(sprite->animation() == MOVE_LEFT)
			sprite->changeAnimation(STAND_LEFT);
		else if(sprite->animation() == MOVE_RIGHT)
			sprite->changeAnimation(STAND_RIGHT);
	}
	if (door && (Game::instance().getKey(GLFW_KEY_UP) || Game::instance().getKey(GLFW_KEY_DOWN))) {
		sprite->changeAnimation(ENTER);
	}
	if (climb && (Game::instance().getKey(GLFW_KEY_UP) || Game::instance().getKey(GLFW_KEY_DOWN))) {
		sprite->changeAnimation(CLIMB);
	}
	if (final_door && (Game::instance().getKey(GLFW_KEY_UP) || Game::instance().getKey(GLFW_KEY_DOWN))) {
		sprite->changeAnimation(DISAPPEAR);
	}
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

void Player::render()
{
	sprite->render();
}

void Player::setTileMap(TileMap *tileMap)
{
	map = tileMap;
}

void Player::setPosition(const glm::vec2 &pos)
{
	posPlayer = pos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

void Player::dies() {

}

int Player::getLives() {
	return livesPlayer;
}




