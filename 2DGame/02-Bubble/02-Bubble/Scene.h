#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <vector>
#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "TileMap.h"
#include "Player.h"
#include "Enemy.h"
#include "Texture.h"


#define MAX_ENEMIES 3
#define MAX_KEYS    5
#define MAX_ITEMS   4


enum ItemType { ITEM_WEIGHT, ITEM_BOMB, ITEM_BOOTS, ITEM_CLOCK };

struct ElevatorPair
{
	glm::ivec2 entryTile;  // map tile coordinates (col, row) of the entry
	glm::ivec2 exitTile;   // map tile coordinates (col, row) of the exit
};

struct LevelItem
{
	ItemType type;
	glm::ivec2 pos;  // world pixel position
	bool collected;
};

struct LevelKey
{
	glm::ivec2 pos;
	bool collected;
};

enum DoorType { DOOR_ENTRY, DOOR_EXIT, DOOR_SECRET };

struct LevelDoor {
	glm::ivec2 tilePos;  // (col, row)
	DoorType   type;
	bool       open;
};

// Scene contains all the entities of our game.
// It is responsible for updating and rendering them.

class Scene
{

public:
	Scene();
	~Scene();

	void initShaders();     // called once by Game::init
	void loadLevel(int n);  // load/reload a level

	void update(int deltaTime);
	void render();

	bool isGameOver()      const { return gameOver; }
	bool isLevelComplete() const { return levelComplete; }

	void setGodMode(bool g);
	void killAllEnemies();
	void collectAllKeys();

private:
	void initMap(int level);
	void applyTileTypes();
	void applyElevatorPairs();
	void spawnEntities(int level);
	void recreateWorldPickupSprites(int tileSize);
	void configureItemsAtlas();
	void renderHUD();
	bool checkCollision(const glm::ivec2 &posA, const glm::ivec2 &posB,
	                    const glm::ivec2 &sizeA, const glm::ivec2 &sizeB) const;

	void markSecretDoorTiles(int level);
	bool playerOnSecretDoor(const glm::ivec2 &playerPos, const glm::ivec2 &playerSize) const;
	void beginEnterSecretRoom();
	void finishEnterSecretRoom();
	void exitSecretRoom();

private:
	// Tile type classification for the current level (set in initMap per level)
	std::vector<int> tileBlocks;
	std::vector<int> tileCliffs;
	std::vector<int> tileLadders;
	std::vector<int> tileDoors;
	std::vector<int> tileJumps;
	std::vector<int> tileWarps;
	std::vector<int>          tileElevators;   // tile IDs to mark as TILE_ELEVATOR (still used for applyTileTypes)
	std::vector<ElevatorPair> elevatorPairs;   // explicit entry/exit positions per level
	std::vector<glm::ivec2>   warpTiles;       // (col, row) positions of the 2 warp floors per level

	TileMap       *map;
	Player        *player;
	Enemy         *enemies[MAX_ENEMIES];
	int            activeEnemies;

	ShaderProgram  texProgram;
	float          currentTime;
	glm::mat4      projection;

	// Camera
	float          camZoom;
	float          camX, camY;    // top-left of viewport in world coords

	// HUD assets (rendered in screen-space projection)
	Texture        heartTex;
	Sprite        *heartSprite;

	// Item/key sprites — atlas images/items.png (row-major 1-based indices; default 32×32px cells on this project’s 320² sheet)
	Texture        itemTex;
	Sprite        *itemSprite;       // world pickups (quad = tileSize)
	Sprite        *itemHudSprite;    // carried-item icon in HUD (fixed size)
	Sprite        *keyWorldSprite;   // keys in the level (smaller than tile)
	Sprite        *keyHudSprite;     // one key icon + numeric counter in HUD
	Sprite        *godHudSprite;     // god-mode HUD icon (sprite #5)
	Sprite        *godAuraSprites[3];
	int            keyWorldPixelSize;
	int            itemAtlasCols;
	int            itemAtlasRows;
	glm::vec2      itemAtlasCellUv;  // normalized (du, dv) per cell

	// Doors (images/items.png row 2: col 0=closed, col 1=open)
	std::vector<LevelDoor> doors;
	Texture                doorTex;
	Sprite                *doorSprite;

	// Main level vs secret room
	TileMap       *mainMap;
	TileMap       *secretMap;
	int            levelIndex;
	bool           inSecretRoom;
	glm::ivec2     secretReturnPos;
	int            secretAnimTimer;   // ms: entering / exiting
	bool           secretEnterPending;
	LevelItem      secretLoot;
	bool           secretLootTaken;
	int            secretExitCooldown;

	// Spawn position (reset here on respawn)
	glm::ivec2     spawnPos;

	// Game state flags
	bool           gameOver;
	bool           levelComplete;

	// Elevator state
	bool           playerEnteringElevator;
	glm::ivec2     elevatorExitPos;

	// Warp state
	bool           playerWarpingOut;
	glm::ivec2     warpDestPos;
	bool           enemiesFrozen;
	float          freezeTimer;
	float          respawnTimer;   // countdown after death before respawning

	// Keys to collect
	LevelKey       keys[MAX_KEYS];
	int            keysRequired;
	int            keysCollected;

	// Items scattered in the level
	LevelItem      items[MAX_ITEMS];
	int            itemCount;

	// Carried item
	bool           hasItem;
	ItemType       carriedItem;

	// Pushable weights (not picked up — exist in world)
	struct WorldWeight {
		glm::ivec2 pos;
		bool       active;   // still in world
		bool       falling;
		float      fallSpeed;
	};
	static const int MAX_WEIGHTS = 4;
	WorldWeight    weights[MAX_WEIGHTS];
	int            weightCount;

	// Placed bomb state
	bool           bombActive;
	glm::ivec2     bombPos;
	float          bombTimer;       // countdown ms until explosion
	bool           bombExploding;   // smoke animation playing
	float          bombSmokeTimer;  // ms accumulated for smoke animation
	Sprite        *bombLitSprite;   // lit bomb on ground (atlas block 8)
	Sprite        *bombSmokeSprite; // smoke explosion (atlas blocks 11-18, 8 frames)

	// Explosion effect (reused by bomb and weight)
	struct Explosion {
		bool active;
		glm::ivec2 pos;
		float timer;
	};
	static const int MAX_EXPLOSIONS = 4;
	Explosion      explosions[MAX_EXPLOSIONS];
};


#endif // _SCENE_INCLUDE
