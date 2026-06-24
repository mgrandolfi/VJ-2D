#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Player.h"
#include "Game.h"

#ifndef M_PI
#endif

#define PLAYER_SPEED 2
#define BOOST_SPEED 4
#define JUMP_ANGLE_STEP 5
#define JUMP_TILES 5    
#define FALL_STEP 4
#define HURT_INVINCIBLE_MS 1500


enum PlayerAnims
{
	STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT,
	STAND_FRONT, CLIMB, ENTER, EXIT, DISAPPEAR, APPEAR,
	START_JUMP_LEFT, START_JUMP_RIGHT,
	JUMP_LEFT, JUMP_RIGHT, LAND_LEFT, LAND_RIGHT,
	DIE_LEFT, DIE_RIGHT, BOMB_LEFT, BOMB_RIGHT,
	HURT_LEFT, HURT_RIGHT, OPEN_CHEST_LEFT, OPEN_CHEST_RIGHT
};


Player::Player()
{
	sprite = NULL;
	map = NULL;
	spriteSize = 32;
	livesPlayer = 3;
	isJumping = false;
	jumpAngle= 0;
	startY = 0;
	onGround = false;
	onLadder = false;
	facing = 1;
	godMode = false;
	bootTimer = 0;
	hurtTimer = 0;
	landAnimTimer = 0;
	warpDisappearing = false;
	warpAppearing = false;
	warpTimer = 0.f;
	openingChest = false;
	chestTimer = 0.f;
}

Player::~Player()
{
	if (sprite != NULL) delete sprite;
}

void Player::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram, int tileSize)
{
	spriteSize = tileSize;
	livesPlayer = 3;
	isJumping = false;
	jumpAngle = 0;
	startY = 0;
	onGround= false;
	onLadder = false;
	facing = 1;
	godMode = false;
	bootTimer= 0;
	hurtTimer = 0;
	landAnimTimer = 0;
	openingChest = false;
	chestTimer= 0.f;

	spritesheet.loadFromFile("images/characters/bugs.png", TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheetFast.loadFromFile("images/characters/bugs_fast.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(glm::ivec2(spriteSize, spriteSize), glm::vec2(0.09f, 0.09f), &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(24);

	sprite->setAnimationSpeed(STAND_FRONT, 1);
	sprite->addKeyframe(STAND_FRONT, glm::vec2(0.4f, 0.0f));

	sprite->setAnimationSpeed(STAND_LEFT, 1);
	sprite->addKeyframe(STAND_LEFT, glm::vec2(0.0f, 0.1f));

	sprite->setAnimationSpeed(STAND_RIGHT, 1);
	sprite->addKeyframe(STAND_RIGHT, glm::vec2(0.9f, 0.1f));

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

	sprite->setAnimationSpeed(APPEAR, 3);
	sprite->addKeyframe(APPEAR, glm::vec2(0.7f, 0.0f));
	sprite->addKeyframe(APPEAR, glm::vec2(0.6f, 0.0f));
	sprite->addKeyframe(APPEAR, glm::vec2(0.5f, 0.0f));

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

	sprite->setAnimationSpeed(DIE_RIGHT, 2);
	sprite->addKeyframe(DIE_RIGHT, glm::vec2(0.5f, 0.3f));

	sprite->setAnimationSpeed(HURT_LEFT, 2);
	sprite->addKeyframe(HURT_LEFT, glm::vec2(0.3f, 0.3f));

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
	sprite->addKeyframe(OPEN_CHEST_RIGHT, glm::vec2(0.4f, 0.2f));
	sprite->addKeyframe(OPEN_CHEST_RIGHT, glm::vec2(0.5f, 0.2f));
	sprite->addKeyframe(OPEN_CHEST_RIGHT, glm::vec2(0.6f, 0.2f));
	sprite->addKeyframe(OPEN_CHEST_RIGHT, glm::vec2(0.7f, 0.2f));

	sprite->changeAnimation(STAND_FRONT);
	tileMapDispl = tileMapPos;
}

void Player::startWarpDisappear() {
	warpDisappearing = true;
	warpAppearing= false;
	warpTimer = 500.f;
	isJumping = false;
	sprite->changeAnimation(DISAPPEAR);
}

void Player::startWarpAppear(const glm::ivec2 &destPos) {
	posPlayer = destPos;
	warpDisappearing = false;
	warpAppearing = true;
	warpTimer = 500.f;
	sprite->changeAnimation(APPEAR);
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

void Player::startOpenChest() {
	openingChest = true;
	chestTimer = 1200.f; //duracion animacion
	isJumping = false;
	facing = 1;
	sprite->changeAnimation(22); //animacion open chest left
}

void Player::update(int deltaTime) {
	sprite->update(deltaTime);

	if (openingChest) {
		chestTimer -= deltaTime;
		if (chestTimer <= 0.f)
			openingChest = false;
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
		return;
	}

	//Teletransportacion
	if (warpDisappearing || warpAppearing) {
		warpTimer -= deltaTime;
		if (warpTimer <= 0.f) {
			if (warpDisappearing) warpDisappearing = false;
			else warpAppearing    = false; 
		}
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
		return;
	}

	if (bootTimer > 0) bootTimer -= deltaTime;
	if (hurtTimer > 0) hurtTimer -= deltaTime;
	sprite->setTexture(bootTimer > 0 ? &spritesheetFast : &spritesheet);

	//Caer animacion
	if (landAnimTimer > 0) {
		landAnimTimer -= deltaTime;
		if (landAnimTimer <= 0) {
			landAnimTimer = 0;
			if (sprite->animation() == LAND_LEFT || sprite->animation() == LAND_RIGHT) sprite->changeAnimation((facing >= 0) ? STAND_RIGHT : STAND_LEFT);
		}
	}

	const glm::ivec2 size(spriteSize, spriteSize);
	const int speed = (bootTimer > 0) ? BOOST_SPEED : PLAYER_SPEED;
	const int mapW = map->getMapWidth()  * map->getTileSize();
	const int mapH = map->getMapHeight() * map->getTileSize();

	onLadder = map->isOnLadder(posPlayer, size);
	bool onCliff = map->isOnCliff(posPlayer, size);

	//Moverse derecha a izquierda
	if (Game::instance().getKey(GLFW_KEY_LEFT)) {
		facing = -1;
		if (!isJumping && sprite->animation() != MOVE_LEFT) sprite->changeAnimation(MOVE_LEFT);
		posPlayer.x -= speed;
		if (posPlayer.x < 0) posPlayer.x = 0;
		if (map->collisionMoveLeft(posPlayer, size)) {
			posPlayer.x += speed;
			if (!isJumping) sprite->changeAnimation(STAND_LEFT);
		}
		if (onCliff) posPlayer.y -= speed;
	}
	else if (Game::instance().getKey(GLFW_KEY_RIGHT)) {
		facing = 1;
		if (!isJumping && sprite->animation() != MOVE_RIGHT)
			sprite->changeAnimation(MOVE_RIGHT);
		posPlayer.x += speed;
		if (posPlayer.x > mapW - spriteSize) posPlayer.x = mapW - spriteSize;
		if (map->collisionMoveRight(posPlayer, size)) {
			posPlayer.x -= speed;
			if (!isJumping) sprite->changeAnimation(STAND_RIGHT);
		}
		if (onCliff) posPlayer.y -= speed;
	}
	else if (!isJumping && !onLadder) {
		if (sprite->animation() == MOVE_LEFT) sprite->changeAnimation(STAND_LEFT);
		else if (sprite->animation() == MOVE_RIGHT) sprite->changeAnimation(STAND_RIGHT);
	}

	//Escalar / cliff
	if (onCliff && !isJumping) onGround = true; 
	else if (onLadder && !isJumping) {
		onGround = false;
		if (Game::instance().getKey(GLFW_KEY_UP)) {
			if (sprite->animation() != CLIMB) sprite->changeAnimation(CLIMB);
			posPlayer.y -= speed;
			if (posPlayer.y < 0) posPlayer.y = 0;
			int dummy = posPlayer.y;
			if (map->collisionMoveUp(posPlayer, size, &dummy)) posPlayer.y = dummy;
		}
		else if (Game::instance().getKey(GLFW_KEY_DOWN)) {
			if (sprite->animation() != CLIMB) sprite->changeAnimation(CLIMB);
			posPlayer.y += speed;
			if (map->collisionMoveDown(posPlayer, size, &posPlayer.y)) {
				onGround = true;
				onLadder = false;
			}
		}
		else {
			if (sprite->animation() != CLIMB) sprite->changeAnimation(CLIMB);
		}
	}
	//Salto del personaje y caída
	else {
		if (isJumping) {
			jumpAngle += JUMP_ANGLE_STEP;
			if (jumpAngle >= 180) {
				isJumping = false;
				posPlayer.y = startY;
				if (map->collisionMoveDown(posPlayer, size, &posPlayer.y)) onGround = true;
				sprite->changeAnimation((facing >= 0) ? LAND_RIGHT : LAND_LEFT);
				landAnimTimer = 220;
			}
			else {
				int jumpHeight = spriteSize * JUMP_TILES;
				posPlayer.y = int(startY - jumpHeight * sin(jumpAngle * M_PI / 180.0));

				if (jumpAngle < 90) {
					int dummy = posPlayer.y;
					if (map->collisionMoveUp(posPlayer, size, &dummy)) {
						posPlayer.y = dummy;
						isJumping = false;
						landAnimTimer = 0;
						sprite->changeAnimation((facing >= 0) ? STAND_RIGHT : STAND_LEFT);
					}
				}
				else if (jumpAngle > 90) {
					if (sprite->animation() != LAND_LEFT && sprite->animation() != LAND_RIGHT) sprite->changeAnimation((facing >= 0) ? LAND_RIGHT : LAND_LEFT);
					if (map->collisionMoveDown(posPlayer, size, &posPlayer.y)) {
						isJumping = false;
						onGround = true;
						landAnimTimer = 220;
					}
				}

				if (isJumping) {
					int anim = (facing >= 0) ? JUMP_RIGHT : JUMP_LEFT;
					if (sprite->animation() != anim && sprite->animation() != START_JUMP_LEFT && sprite->animation() != START_JUMP_RIGHT) sprite->changeAnimation(anim);
				}
			}
		}
		else {
			posPlayer.y += FALL_STEP;
			if (map->collisionMoveDown(posPlayer, size, &posPlayer.y)) {
				onGround = true;
				if (map->isOnJump(posPlayer, size)) {
					isJumping = true;
					jumpAngle = 0;
					startY    = posPlayer.y;
					onGround  = false;
					sprite->changeAnimation((facing >= 0) ? START_JUMP_RIGHT : START_JUMP_LEFT);
				}
			}
			else onGround = false;
			if (posPlayer.y >= mapH - spriteSize) {
				posPlayer.y = mapH - spriteSize;
				onGround = true;
			}
		}
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

void Player::playDoorEnterAnim()
{
	sprite->changeAnimation(ENTER);
}

void Player::playDoorExitAnim()
{
	sprite->changeAnimation(EXIT);
}

void Player::dies()
{
	if (hurtTimer > 0 || godMode || livesPlayer <= 0) return;

	livesPlayer--;
	hurtTimer = HURT_INVINCIBLE_MS;
	sprite->changeAnimation((facing >= 0) ? HURT_RIGHT : HURT_LEFT);
}
