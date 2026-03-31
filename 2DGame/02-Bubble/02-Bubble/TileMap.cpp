#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "TileMap.h"


using namespace std;

// Solid for movement (jump pads must be walkable, not only decorative)
static bool tileBlocksMovement(TileType t)
{
	return t == TILE_BLOCK || t == TILE_JUMP || t == TILE_ELEVATOR || t == TILE_WARP;
}


TileMap *TileMap::createTileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program)
{
	TileMap *map = new TileMap(levelFile, minCoords, program);
	
	return map;
}


TileMap::TileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program)
{
	map         = NULL;
	tileTypeMap = NULL;
	nTiles = 0;
	vao = 0;
	vbo = 0;
	if(loadLevel(levelFile))
		prepareArrays(minCoords, program);
}

TileMap::~TileMap()
{
	if(map != NULL)
		delete [] map;
	if(tileTypeMap != NULL)
		delete [] tileTypeMap;
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}


void TileMap::render() const
{
	if(vao == 0 || nTiles == 0)
		return;

	glEnable(GL_TEXTURE_2D);
	tilesheet.use();
	glBindVertexArray(vao);
	glEnableVertexAttribArray(posLocation);
	glEnableVertexAttribArray(texCoordLocation);
	glDrawArrays(GL_TRIANGLES, 0, 6 * nTiles);
	glDisable(GL_TEXTURE_2D);
}

void TileMap::free()
{
	glDeleteBuffers(1, &vbo);
}

bool TileMap::loadLevel(const string &levelFile)
{
	ifstream fin;
	string line, tilesheetFile;
	stringstream sstream;
	int tile;
	
	fin.open(levelFile.c_str());
	if(!fin.is_open())
		return false;
	getline(fin, line);
	if(line.compare(0, 7, "TILEMAP") != 0)
		return false;
	getline(fin, line);
	sstream.clear();
	sstream.str(line);
	sstream >> mapSize.x >> mapSize.y;
	getline(fin, line);
	sstream.clear();
	sstream.str(line);
	sstream >> tileSize >> blockSize;
	getline(fin, line);
	sstream.clear();
	sstream.str(line);
	sstream >> tilesheetFile;
	tilesheet.loadFromFile(tilesheetFile, TEXTURE_PIXEL_FORMAT_RGBA);
	tilesheet.setWrapS(GL_CLAMP_TO_EDGE);
	tilesheet.setWrapT(GL_CLAMP_TO_EDGE);
	tilesheet.setMinFilter(GL_NEAREST);
	tilesheet.setMagFilter(GL_NEAREST);
	getline(fin, line);
	sstream.clear();
	sstream.str(line);
	sstream >> tilesheetSize.x >> tilesheetSize.y;

	totalTilesMap = tilesheetSize.x * tilesheetSize.y;
	tileTypeMap = new TileType[totalTilesMap];
	for (int i = 0; i < totalTilesMap; ++i)
		tileTypeMap[i] = (i == 0) ? TILE_BLOCK : TILE_EMPTY;

	tileTexSize = glm::vec2(1.f / tilesheetSize.x, 1.f / tilesheetSize.y);

	map = new int[mapSize.x * mapSize.y];

	for(int j=0; j<mapSize.y; j++)
	{
		for(int i=0; i<mapSize.x; i++)
		{
			fin >> tile;
			map[j*mapSize.x+i] = tile;
		}
	}
	fin.close();
	
	return true;
}

void TileMap::prepareArrays(const glm::vec2 &minCoords, ShaderProgram &program)
{
	int tile;
	glm::vec2 posTile, texCoordTile[2], halfTexel;
	vector<float> vertices;
	
	nTiles = 0;
	halfTexel = glm::vec2(0.5f / tilesheet.width(), 0.5f / tilesheet.height());
	for(int j=0; j<mapSize.y; j++)
	{
		for(int i=0; i<mapSize.x; i++)
		{
			tile = map[j * mapSize.x + i];
			if(tile >= 0)
			{
				// Non-empty tile
				nTiles++;
				posTile = glm::vec2(minCoords.x + i * tileSize, minCoords.y + j * tileSize);
				texCoordTile[0] = glm::vec2(float((tile)%tilesheetSize.x) / tilesheetSize.x, float((tile)/tilesheetSize.x) / tilesheetSize.y);
				texCoordTile[1] = texCoordTile[0] + tileTexSize;
				//texCoordTile[0] += halfTexel;
				texCoordTile[1] -= halfTexel;
				// First triangle
				vertices.push_back(posTile.x); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[1].y);
				// Second triangle
				vertices.push_back(posTile.x); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[1].y);
				vertices.push_back(posTile.x); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[1].y);
			}
		}
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 24 * nTiles * sizeof(float), &vertices[0], GL_STATIC_DRAW);
	posLocation = program.bindVertexAttribute("position", 2, 4*sizeof(float), 0);
	texCoordLocation = program.bindVertexAttribute("texCoord", 2, 4*sizeof(float), (void *)(2*sizeof(float)));
}

bool TileMap::collisionMoveLeft(const glm::ivec2 &pos, const glm::ivec2 &size,
                                bool blockLadders) const
{
	int x  = pos.x / tileSize;
	int y0 = pos.y / tileSize;
	int y1 = (pos.y + size.y - 1) / tileSize;

	if (x < 0 || x >= mapSize.x) return false;
	if (y0 < 0) y0 = 0;
	if (y1 >= mapSize.y) y1 = mapSize.y - 1;

	for (int y = y0; y <= y1; y++)
	{
		int tile = map[y * mapSize.x + x];
		if (tile < 0 || tile >= totalTilesMap) continue;
		TileType tt = tileTypeMap[tile];
		if (tileBlocksMovement(tt) || (blockLadders && tt == TILE_LADDER))
			return true;
	}
	return false;
}

bool TileMap::collisionMoveRight(const glm::ivec2 &pos, const glm::ivec2 &size,
                                 bool blockLadders) const
{
	int x  = (pos.x + size.x - 1) / tileSize;
	int y0 = pos.y / tileSize;
	int y1 = (pos.y + size.y - 1) / tileSize;

	if (x < 0 || x >= mapSize.x) return false;
	if (y0 < 0) y0 = 0;
	if (y1 >= mapSize.y) y1 = mapSize.y - 1;

	for (int y = y0; y <= y1; y++)
	{
		int tile = map[y * mapSize.x + x];
		if (tile < 0 || tile >= totalTilesMap) continue;
		TileType tt = tileTypeMap[tile];
		if (tileBlocksMovement(tt) || (blockLadders && tt == TILE_LADDER))
			return true;
	}
	return false;
}

bool TileMap::collisionMoveDown(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY) const
{
	int x0 = pos.x / tileSize;
	int x1 = (pos.x + size.x - 1) / tileSize;
	int y  = (pos.y + size.y - 1) / tileSize;

	if (y < 0 || y >= mapSize.y) return false;
	if (x0 < 0) x0 = 0;
	if (x1 >= mapSize.x) x1 = mapSize.x - 1;

	for (int x = x0; x <= x1; x++)
	{
		int tile = map[y * mapSize.x + x];
		if (tile >= 0 && tile < totalTilesMap && tileBlocksMovement(tileTypeMap[tile]))
		{
			if (*posY - tileSize * y + size.y <= 4)
			{
				*posY = tileSize * y - size.y;
				return true;
			}
		}
	}
	return false;
}

bool TileMap::collisionMoveUp(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY) const
{
	int x0 = pos.x / tileSize;
	int x1 = (pos.x + size.x - 1) / tileSize;
	int y  = pos.y / tileSize;

	if (y < 0 || y >= mapSize.y) return false;
	if (x0 < 0) x0 = 0;
	if (x1 >= mapSize.x) x1 = mapSize.x - 1;

	for (int x = x0; x <= x1; x++)
	{
		int tile = map[y * mapSize.x + x];
		if (tile >= 0 && tile < totalTilesMap && tileBlocksMovement(tileTypeMap[tile]))
		{
			*posY = tileSize * (y + 1);
			return true;
		}
	}
	return false;
}


void TileMap::setTileType(int tile, TileType type)
{
	if(tile >= 0 && tile < totalTilesMap)
		tileTypeMap[tile] = type;
}

void TileMap::setTileTypeRange(int tileFrom, int tileTo, TileType type)
{
	for(int i = tileFrom; i <= tileTo; i++)
		setTileType(i, type);
}

TileType TileMap::tileTypeAt(int worldX, int worldY) const
{
	int tx = worldX / tileSize;
	int ty = worldY / tileSize;
	if (tx < 0 || tx >= mapSize.x || ty < 0 || ty >= mapSize.y)
		return TILE_EMPTY;
	int tile = map[ty * mapSize.x + tx];
	if (tile < 0 || tile >= totalTilesMap)
		return TILE_EMPTY;
	return tileTypeMap[tile];
}

bool TileMap::isOnLadder(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	int cx = pos.x + size.x / 2;
	int ty0 = pos.y / tileSize;
	int ty1 = (pos.y + size.y - 1) / tileSize;
	for(int ty = ty0; ty <= ty1; ++ty)
	{
		if(tileTypeAt(cx, ty * tileSize + tileSize / 2) == TILE_LADDER)
			return true;
	}
	return false;
}

bool TileMap::isOnCliff(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	int cx = pos.x + size.x / 2;
	int ty0 = pos.y / tileSize;
	int ty1 = (pos.y + size.y - 1) / tileSize;
	for (int ty = ty0; ty <= ty1; ++ty)
	{
		if (tileTypeAt(cx, ty * tileSize + tileSize / 2) == TILE_CLIFF)
			return true;
	}
	return false;
}

bool TileMap::isOnDoor(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	int cx = pos.x + size.x / 2;
	int cy = pos.y + size.y / 2;
	return tileTypeAt(cx, cy) == TILE_DOOR;
}

bool TileMap::isOnSecret(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	int cx = pos.x + size.x / 2;
	int cy = pos.y + size.y / 2;
	return tileTypeAt(cx, cy) == TILE_SECRET;
}

bool TileMap::isOnJump(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	int x0 = pos.x / tileSize;
	int x1 = (pos.x + size.x - 1) / tileSize;
	// First row strictly below sprite bottom — the tile surface we stand on
	int y = (pos.y + size.y) / tileSize;
	if(y >= mapSize.y) return false;
	for(int x = x0; x <= x1; ++x)
	{
		int tile = map[y * mapSize.x + x];
		if(tile >= 0 && tile < totalTilesMap && tileTypeMap[tile] == TILE_JUMP)
			return true;
	}
	return false;
}

bool TileMap::isOnWarp(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	int cx = pos.x + size.x / 2;
	int cy = pos.y + size.y / 2;
	return tileTypeAt(cx, cy) == TILE_WARP;
}

// Returns the tile ID of the first TILE_ELEVATOR tile directly below the sprite,
// or -1 if none found.
int TileMap::getTileIdBelow(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	int x0 = pos.x / tileSize;
	int x1 = (pos.x + size.x - 1) / tileSize;
	int y  = (pos.y + size.y) / tileSize;  // row just below sprite bottom
	if (y >= mapSize.y) return -1;
	if (x0 < 0) x0 = 0;
	if (x1 >= mapSize.x) x1 = mapSize.x - 1;
	for (int x = x0; x <= x1; ++x)
	{
		int tile = map[y * mapSize.x + x];
		if (tile >= 0 && tile < totalTilesMap && tileTypeMap[tile] == TILE_ELEVATOR)
			return tile;
	}
	return -1;
}

// Returns the world-pixel top-left position of the first map cell with the given tile ID.
// Returns (-1,-1) if not found.
glm::ivec2 TileMap::findTileId(int tileId) const
{
	for (int j = 0; j < mapSize.y; ++j)
		for (int i = 0; i < mapSize.x; ++i)
			if (map[j * mapSize.x + i] == tileId)
				return glm::ivec2(i * tileSize, j * tileSize);
	return glm::ivec2(-1, -1);
}

int TileMap::getTileIdAt(int tx, int ty) const
{
	if (tx < 0 || tx >= mapSize.x || ty < 0 || ty >= mapSize.y)
		return -1;
	return map[ty * mapSize.x + tx];
}

