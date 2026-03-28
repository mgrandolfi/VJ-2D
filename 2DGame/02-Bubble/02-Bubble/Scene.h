#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "TileMap.h"
#include "Player.h"
#include "Enemy.h"


// Scene contains all the entities of our game.
// It is responsible for updating and render them.


class Scene
{

public:
	Scene();
	~Scene();

	void init();
	void update(int deltaTime);
	void render();

private:
	void initShaders();
	bool checkCollision(const glm::vec2 &posA, const glm::vec2 &posB, const glm::vec2 &sizeA, const glm::vec2 &sizeB);
	void initMap(int mapActive);

private:
	TileMap *map;
	Player *player;
	Enemy *enemies[4];
	ShaderProgram texProgram;
	float currentTime;
	glm::mat4 projection;

};


#endif // _SCENE_INCLUDE

