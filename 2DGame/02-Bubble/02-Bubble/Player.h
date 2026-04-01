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
	void playDoorEnterAnim();
	void playDoorExitAnim();

	void startWarpDisappear();
	void startWarpAppear(const glm::ivec2 &destPos);
	bool isWarpDisappearing() const { return warpDisappearing; }
	bool isWarping()          const { return warpDisappearing || warpAppearing; }

	void startOpenChest();
	bool isOpeningChest()     const { return openingChest; }

private:
	glm::ivec2  tileMapDispl, posPlayer;
	int         spriteSize;     

	// Jump state
	bool        isJumping;
	int         jumpAngle;       // 0 - 180 (degrees)
	int         startY;          // Y position cuando empieza salto
	int         landAnimTimer;     

	// Physics flags
	bool        onGround;
	bool        onLadder;
	int         facing;          // 1=right, -1=left

	// Status
	bool        godMode;
	int         bootTimer;       // ms que quedan de boost (0 = no boost)
	int         hurtTimer;      
	int         livesPlayer;

	// Warp state
	bool        warpDisappearing;
	bool        warpAppearing;
	float       warpTimer;

	// Chest open
	bool        openingChest;
	float       chestTimer;

	// Graphics
	Texture     spritesheet;
	Texture     spritesheetFast;   
	Sprite     *sprite;
	TileMap    *map;
};


#endif // _PLAYER_INCLUDE
