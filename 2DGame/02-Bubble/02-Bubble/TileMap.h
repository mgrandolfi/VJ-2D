#ifndef _TILE_MAP_INCLUDE
#define _TILE_MAP_INCLUDE


#include <glm/glm.hpp>
#include "Texture.h"
#include "ShaderProgram.h"


enum TileType
{
	TILE_EMPTY,
	TILE_BLOCK,
	TILE_LADDER,
	TILE_DOOR,
	TILE_JUMP,
	TILE_WARP
};

class TileMap
{

private:
	TileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program);

public:
	static TileMap *createTileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program);

	~TileMap();

	void render() const;
	void free();

	int getTileSize() const { return tileSize; }
	int getMapWidth() const { return mapSize.x; }
	int getMapHeight() const { return mapSize.y; }

	// When blockLadders is true, TILE_LADDER blocks horizontal moves (ground enemies).
	bool collisionMoveLeft(const glm::ivec2 &pos, const glm::ivec2 &size,
	                       bool blockLadders = false) const;
	bool collisionMoveRight(const glm::ivec2 &pos, const glm::ivec2 &size,
	                        bool blockLadders = false) const;
	bool collisionMoveDown(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY) const;
	bool collisionMoveUp(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY) const;

	// Tile type queries at world pixel positions
	TileType tileTypeAt(int worldX, int worldY) const;
	bool isOnLadder(const glm::ivec2 &pos, const glm::ivec2 &size) const;
	bool isOnDoor(const glm::ivec2 &pos, const glm::ivec2 &size) const;
	bool isOnJump(const glm::ivec2 &pos, const glm::ivec2 &size) const;
	bool isOnWarp(const glm::ivec2 &pos, const glm::ivec2 &size) const;

	void setTileType(int tile, TileType type);
	void setTileTypeRange(int tileFrom, int tileTo, TileType type);

private:
	bool loadLevel(const string &levelFile);
	void prepareArrays(const glm::vec2 &minCoords, ShaderProgram &program);

private:
	GLuint vao;
	GLuint vbo;
	GLint posLocation, texCoordLocation;
	int nTiles;
	glm::ivec2 position, mapSize, tilesheetSize;
	int tileSize, blockSize;
	Texture tilesheet;
	glm::vec2 tileTexSize;
	int *map;
	TileType *tileTypeMap;
	int totalTilesMap;
};


#endif // _TILE_MAP_INCLUDE
