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
	bool isLevelBack()     const { return levelBack; }

	void setGodMode(bool g);
	void killAllEnemies();
	void collectAllKeys();

private:
	void initMap(int level);
	void applyTileTypes();
	void spawnEntities(int level);
	void recreateWorldPickupSprites(int tileSize);
	void configureItemsAtlas();
	void renderHUD();
	bool checkCollision(const glm::ivec2 &posA, const glm::ivec2 &posB,
	                    const glm::ivec2 &sizeA, const glm::ivec2 &sizeB) const;

	bool playerOnSecretDoor(const glm::ivec2 &playerPos, const glm::ivec2 &playerSize) const;
	void beginEnterSecretRoom();
	void exitSecretRoom();

private:
	// Tile type classification for the current level (set in initMap per level)
	std::vector<int> tileBlocks;
	std::vector<int> tileCliffs;
	std::vector<int> tileLadders;
	std::vector<int> tileDoors;
	std::vector<int> tileJumps;
	std::vector<int> tileWarps;
	std::vector<glm::ivec2>   warpTiles; //para posicionar los tiles de warp

	TileMap       *map;
	Player        *player;
	Enemy         *enemies[MAX_ENEMIES];
	int            activeEnemies;

	ShaderProgram  texProgram;
	float          currentTime;
	glm::mat4      projection;

	// camara
	float          camZoom;
	float          camX, camY;    // top-left of viewport en coordenadas de mundo

	// HUD assets (rendered in screen-space projection)
	Texture        heartTex;
	Sprite        *heartSprite;

	Texture        itemTex;
	Sprite        *itemSprite;      
	Sprite        *itemHudSprite;    
	Sprite        *keyWorldSprite;   
	Sprite        *keyHudSprite;    
	Sprite        *godHudSprite;     
	Sprite        *godAuraSprites[3];
	int            keyWorldPixelSize;
	int            itemAtlasCols;
	int            itemAtlasRows;
	glm::vec2      itemAtlasCellUv; 

	// Doors 
	std::vector<LevelDoor> doors;
	Texture                doorTex;
	Sprite                *doorSprite;

	TileMap       *mainMap;
	TileMap       *secretMap;
	int            levelIndex;
	bool           inSecretRoom;
	glm::ivec2     secretReturnPos;
	int            secretAnimTimer;   
	bool           secretEnterPending;
	LevelItem      secretLoot;
	bool           secretLootTaken;
	bool           secretIsChest;      
	Sprite        *chestSprite;       
	bool           chestOpening;      
	float          chestOpenTimer;     
	int            secretDoorIndex;    
	int            secretExitCooldown;

	// Spawn position
	glm::ivec2     spawnPos;

	// Game state flags
	bool           gameOver;
	bool           levelComplete;
	bool           levelBack;

	// Warp state
	bool           playerWarpingOut;
	glm::ivec2     warpDestPos;
	bool           enemiesFrozen;
	float          freezeTimer;
	float          respawnTimer;   // countdown para respwan despues de morir

	// Keys 
	LevelKey       keys[MAX_KEYS];
	int            keysRequired;
	int            keysCollected;

	// Items 
	LevelItem      items[MAX_ITEMS];
	int            itemCount;

	// Carried item
	bool           hasItem;
	ItemType       carriedItem;

	// Pesos
	struct WorldWeight {
		glm::ivec2 pos;
		bool       active;   
		bool       falling;
		float      fallSpeed;
	};
	static const int MAX_WEIGHTS = 4;
	WorldWeight    weights[MAX_WEIGHTS];
	int            weightCount;

	// Bomba
	bool           bombActive;
	glm::ivec2     bombPos;
	float          bombTimer;     //contador para explosion   
	bool           bombExploding;   
	float          bombSmokeTimer;  //contador para duracion de humo
	Sprite        *bombLitSprite;   
	Sprite        *bombSmokeSprite; 

	// Explosion efecto
	struct Explosion {
		bool active;
		glm::ivec2 pos;
		float timer;
	};
	static const int MAX_EXPLOSIONS = 4;
	Explosion      explosions[MAX_EXPLOSIONS];
};


#endif // _SCENE_INCLUDE
