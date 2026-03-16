#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"


#define SCREEN_X 32
#define SCREEN_Y 16

#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 25


Scene::Scene()
{
	map = NULL;
	player = NULL;
}

Scene::~Scene()
{
	texProgram.free();
	if(map != NULL)
		delete map;
	if(player != NULL)
		delete player;
}

/*
	LEVEL 1: piolin
	LEVEL 2: lucas + piolin
	LEVEL 3: piolin + silvestre
	LEVEL 4: lucas + tasmania
	LEVEL 5: silvestre + tasmania + lucas
*/

bool Scene::checkCollision(const glm::vec2 &posA, const glm::vec2 &posB, const glm::vec2 &sizeA, const glm::vec2 &sizeB) {
	return (posA.x < posB.x + sizeB.x &&
            posA.x + sizeA.x > posB.x &&
            posA.y < posB.y + sizeB.y &&
            posA.y + sizeA.y > posB.y);
}

void Scene::init()
{
	initShaders();
	map = TileMap::createTileMap("levels/level01.txt", glm::vec2(SCREEN_X, SCREEN_Y), texProgram);
	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	player->setPosition(glm::vec2(INIT_PLAYER_X_TILES * map->getTileSize(), INIT_PLAYER_Y_TILES * map->getTileSize()));
	player->setTileMap(map);

	for (int i = 0; i < 4; ++i) {
		enemies[i] = new Enemy();
		enemies[i]->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram, static_cast<EnemyTypes>(i)); //REVIEW - Setear el tipo de enemigo según el nivel
		enemies[i]->setTileMap(map);
	}
	
	projection = glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f);
	currentTime = 0.0f;
}

void Scene::update(int deltaTime)
{
	currentTime += deltaTime;
	player->update(deltaTime);

	for (int i = 0; i < 3; ++i) { //REVIEW - son 3 enemigos max por level, adaptar a escena
		enemies[i]->update(deltaTime);

		//FIXME - acabar la funcion de colisiones
		if (checkCollision()) {
			if (player->getLives() == 0) return;//FIXME - llamar a funcion acabar juego
			else player->dies();
		}
	}
}

void Scene::render()
{
	glm::mat4 modelview;

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	modelview = glm::mat4(1.0f);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
	map->render();
	player->render();
}

void Scene::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "shaders/texture.vert");
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/texture.frag");
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	texProgram.init();
	texProgram.addShader(vShader);
	texProgram.addShader(fShader);
	texProgram.link();
	if(!texProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << texProgram.log() << endl << endl;
	}
	texProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}



