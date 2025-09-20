#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"


Scene::Scene()
{
	quad = NULL;
}

Scene::~Scene()
{
	if(quad != NULL)
		delete quad;
	for(int i=0; i<3; i++)
		if(texQuad[i] != NULL)
			delete texQuad[i];
}


void Scene::init()
{
	float sueloY = CAMERA_HEIGHT - 128.f;  // parte superior del suelo
	xBolet = 320.f;   // posición inicial
	yBolet = sueloY - 128.f;               // bolet justo encima
	boletSpeed = 0.1f; // píxeles por milisegundo

	glm::vec2 geom[2] = { glm::vec2(0.f, 0.f), glm::vec2(128.f, 128.f) }; //Ocupa de la esquina superior izquierda a 128x128
	glm::vec2 texCoords[2] = { glm::vec2(0.f, 0.f), glm::vec2(1.f, 1.f) }; //Mapea las texturas a la ventana (0,0) a (1,1)

	initShaders();

	// Quad simple para el cielo (Escalo en render)

	quad = Quad::createQuad(0.f, 0.f, 128.f, 128.f, simpleProgram);

	// --- BOLET ---
	texCoords[0] = glm::vec2(0.f, 0.5f); texCoords[1] = glm::vec2(0.5f, 1.0f); //Defino el recuadro en donde se cargara la imagen en donde las texturas
	texQuad[0] = TexturedQuad::createTexturedQuad(geom, texCoords, texProgram);

	// --- SUELO (brick) ---
	texCoords[0] = glm::vec2(0.f, 0.f); texCoords[1] = glm::vec2(5.f, 1.f);
	texQuad[1] = TexturedQuad::createTexturedQuad(geom, texCoords, texProgram);

	/*texCoords[0] = glm::vec2(0.f, 0.f); texCoords[1] = glm::vec2(1.f, 1.f);
	texQuad[2] = TexturedQuad::createTexturedQuad(geom, texCoords, texProgram); */

	// Load textures
	texs[0].loadFromFile("images/varied.png", TEXTURE_PIXEL_FORMAT_RGBA); //Se carga el mario y todo 
	texs[1].loadFromFile("images/brick.png", TEXTURE_PIXEL_FORMAT_RGB); //Se carga el suelo
	projection = glm::ortho(0.f, float(CAMERA_WIDTH), float(CAMERA_HEIGHT), 0.f);
	currentTime = 0.0f;
}

void Scene::update(int deltaTime)
{
	xBolet += boletSpeed * deltaTime;

	// Rebote en los límites de la pantalla (ten en cuenta el ancho del quad = 128 px)
	if (xBolet <= 0.f) {
		xBolet = 0.f;
		boletSpeed = -boletSpeed;
	}
	else if (xBolet >= CAMERA_WIDTH - 128.f) {
		xBolet = CAMERA_WIDTH - 128.f;
		boletSpeed = -boletSpeed;
	}

	else if (yBolet >= CAMERA_HEIGHT - 128.f) {
		yBolet = CAMERA_HEIGHT - 128.f;
	}
	currentTime += deltaTime;
}

void Scene::render()
{
	glm::mat4 modelview;

	// === CIELO ===

	simpleProgram.use();
	simpleProgram.setUniformMatrix4f("projection", projection);
	simpleProgram.setUniform4f("color", 0.2f, 0.2f, 0.8f, 1.0f);

	modelview = glm::scale(glm::mat4(1.0f), glm::vec3(CAMERA_WIDTH / 128.f, CAMERA_HEIGHT / 128.f, 1.f));

	simpleProgram.setUniformMatrix4f("modelview", modelview);
	quad->render();
	//== = SUELO == =
	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

	modelview = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, CAMERA_HEIGHT - 128.f, 0.f));
	modelview = glm::scale(modelview, glm::vec3(CAMERA_WIDTH / 128.f, 1.f, 1.f));
	texProgram.setUniformMatrix4f("modelview", modelview);
	texQuad[1]->render(texs[1]);
	// Bolet
	float yOffset = 20.f * sin(currentTime / 1000.f); // amplitud = 20px, velocidad de oscilación
	modelview = glm::translate(glm::mat4(1.0f), glm::vec3(xBolet, yBolet+yOffset, 0.f));
	texProgram.setUniformMatrix4f("modelview", modelview);
	texQuad[0]->render(texs[0]);
}

void Scene::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "shaders/simple.vert");
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/simple.frag");
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	simpleProgram.init();
	simpleProgram.addShader(vShader);
	simpleProgram.addShader(fShader);
	simpleProgram.link();
	if(!simpleProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << simpleProgram.log() << endl << endl;
	}
	simpleProgram.bindFragmentOutput("outColor");

	vShader.free();
	fShader.free();
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
}

