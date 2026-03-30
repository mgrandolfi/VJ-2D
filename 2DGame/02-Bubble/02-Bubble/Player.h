#ifndef _PLAYER_INCLUDE
#define _PLAYER_INCLUDE


#include "Sprite.h"
#include "TileMap.h"


class Player
{
public:
	Player();
	~Player();

public:
	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram, int tileSize);
	void update(int deltaTime);
	void render();

	void setTileMap(TileMap *tileMap);
	void setPosition(const glm::vec2 &pos);

	void dies();
	int        getLives()     const { return livesPlayer; }
	glm::ivec2 getPosition()  const { return posPlayer; }
	glm::ivec2 getSpriteSize()const { return glm::ivec2(spriteSize, spriteSize); }
	bool       isGodMode()    const { return godMode; }
	bool       isHurt()       const { return hurtTimer > 0; }

	void setGodMode(bool g)  { godMode = g; }
	void applyBoots(int ms)  { bootTimer = ms; }

private:
	glm::ivec2  tileMapDispl, posPlayer;
	int         spriteSize;      // world-unit size (= tileSize)

	// Jump state (sine-arc approach)
	bool        isJumping;
	int         jumpAngle;       // 0 → 180 (degrees)
	int         startY;          // Y position when jump began
	int         landAnimTimer;     // ms left before leaving LAND_* (keyframes loop otherwise)

	// Physics flags
	bool        onGround;
	bool        onLadder;
	int         facing;          // 1=right, -1=left

	// Status
	bool        godMode;
	int         bootTimer;       // ms remaining for speed boost
	int         hurtTimer;       // ms of post-hit invincibility
	int         livesPlayer;

	// Graphics
	Texture     spritesheet;
	Texture     spritesheetFast;   // boots-active sprite
	Sprite     *sprite;
	TileMap    *map;
};


#endif // _PLAYER_INCLUDE
