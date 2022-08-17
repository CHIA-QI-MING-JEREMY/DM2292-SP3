/**
 CEnemy2D
 @brief A class which represents the enemy object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Planet.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"
// Include Mesh Builder
#include "Primitives/MeshBuilder.h"

// Include GLEW
#include <GL/glew.h>

// Include ImageLoader
#include "System\ImageLoader.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "../App/Source/Scene2D/Map2D.h"
// Include math.h
#include <math.h>

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CPlanet::CPlanet(void)
	: bIsActive(false)
	, cMap2D(NULL)
	, cSettings(NULL)
	, quadMesh(NULL)
	, camera2D(NULL)
	, animatedSprites(NULL)
	, cSoundController(NULL)
{
	transform = glm::mat4(1.0f);	// make sure to initialize matrix to identity matrix first

	// Initialise vecIndex
	vec2Index = glm::i32vec2(0);

	// Initialise vec2UVCoordinate
	vec2UVCoordinate = glm::vec2(0.0f);
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CPlanet::~CPlanet(void)
{
	delete quadMesh;

	// We won't delete this since it was created elsewhere
	cMap2D = NULL;

	// We won't delete this since it was created elsewhere
	camera2D = NULL;

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

/**
  @brief Initialise this instance
  */
bool CPlanet::Init(void)
{
	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();
	// Get the handler to the Camera2D instance
	camera2D = Camera2D::GetInstance();
	
	// Get the handler to the CMap2D instance
	cMap2D = CMap2D::GetInstance();
	// Find the indices for the player in arrMapInfo, and assign it to cPlayer2D
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	if (cMap2D->FindValue(1200, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the player in the arrMapInfo
	cMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::i32vec2(uiCol, uiRow);
	
	scale = 3.0f;
	hasExplored = true;
	isVisible = false;

	planetName = "???";

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Create the quad mesh for the player
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH * scale, cSettings->TILE_HEIGHT * scale);

	// Load the enemy2D texture
	if (UpdatePlanetIcon("Image/Planet/PlanetDisable.png") == false) {
		return false;
	}

	//CS: Init the color to white
	runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);
	planetIcon = "Image/Planet/PlanetDefault.png";
	type = TYPE::NO_TYPE;

	// If this class is initialised properly, then set the bIsActive to true
	bIsActive = true;

	//// Update the UV Coordinates
	//vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false, i32vec2NumMicroSteps.x * cSettings->ENEMY_MICRO_STEP_XAXIS);
	//vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false, i32vec2NumMicroSteps.y * cSettings->ENEMY_MICRO_STEP_YAXIS);

	// Load the sounds into CSoundController
	cSoundController = CSoundController::GetInstance();

	return true;
}

/**
 @brief Update this instance
 */
void CPlanet::Update(const double dElapsedTime)
{
	// if this enemy is not active, then return now
	if (!bIsActive)
		return;


	// Update the UV Coordinates
	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false);
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CPlanet::PreRender(void)
{
	if (!bIsActive)
		return;

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);

	// Activate blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate the shader
	CShaderManager::GetInstance()->Use(sShaderName);
}

/**
 @brief Render this instance
 */
void CPlanet::Render(void)
{
	if (!bIsActive)
		return;

	glBindVertexArray(VAO);
	// get matrix's uniform location and set matrix
	unsigned int transformLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "transform");
	unsigned int colorLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "runtimeColour");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

	//transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	//transform = glm::translate(transform, glm::vec3(vec2UVCoordinate.x,
	//												vec2UVCoordinate.y,
	//												0.0f));


	//// Update the shaders with the latest transform
	//glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	//glUniform4fv(colorLoc, 1, glm::value_ptr(runtimeColour));

	//// bind textures on corresponding texture units
	//glActiveTexture(GL_TEXTURE0);
	//// Get the texture to be rendered
	//glBindTexture(GL_TEXTURE_2D, iTextureID);

	////CS: render the tile
	////quadMesh->Render();
	////CS: Render the animated Sprite
	//animatedSprites->Render();

	//glBindTexture(GL_TEXTURE_2D, 0);

	//Get camera transforms and use them instead

	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	glm::vec2 offset = glm::i32vec2(float(cSettings->NUM_TILES_XAXIS / 2.0f), float(cSettings->NUM_TILES_YAXIS / 2.0f));
	glm::vec2 cameraPos = camera2D->getPos();

	glm::vec2 IndexPos = vec2Index;

	glm::vec2 actualPos = IndexPos - cameraPos + offset;
	actualPos = cSettings->ConvertIndexToUVSpace(actualPos) * camera2D->getZoom();
	actualPos.x += vec2NumMicroSteps.x * cSettings->MICRO_STEP_XAXIS;
	actualPos.y += vec2NumMicroSteps.y * cSettings->MICRO_STEP_YAXIS;

	transform = glm::translate(transform, glm::vec3(actualPos.x, actualPos.y, 0.f));
	transform = glm::scale(transform, glm::vec3(camera2D->getZoom()));

	// Update the shaders with the latest transform
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glUniform4fv(colorLoc, 1, glm::value_ptr(runtimeColour));

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	// Get the texture to be rendered
	glBindTexture(GL_TEXTURE_2D, iTextureID);

	//CS: Render the animated Sprite
	quadMesh->Render();

	glBindTexture(GL_TEXTURE_2D, 0);

}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CPlanet::PostRender(void)
{
	if (!bIsActive)
		return;

	// Disable blending
	glDisable(GL_BLEND);
}

/**
@brief Set the indices of the enemy2D
@param iIndex_XAxis A const int variable which stores the index in the x-axis
@param iIndex_YAxis A const int variable which stores the index in the y-axis
*/
void CPlanet::Seti32vec2Index(const int iIndex_XAxis, const int iIndex_YAxis)
{
	this->vec2Index.x = iIndex_XAxis;
	this->vec2Index.y = iIndex_YAxis;
}

void CPlanet::SetVisibility(bool isVisible)
{
	this->isVisible = isVisible;
	UpdatePlanetIcon(planetIcon);
}

bool CPlanet::getVisibility(void)
{
	return isVisible;
}

void CPlanet::SetType(TYPE type)
{
	if (type == TYPE::JUNGLE || type == TYPE::JUNGLE_TUTORIAL || type == NO_TYPE) {
		this->type = type;
		UpdatePlanetIcon("Image/Planet/PlanetDefault.png");
	}
	else if (type == TYPE::SNOW || type == TYPE::SNOW_TUTORIAL) {
		this->type = type;
		UpdatePlanetIcon("Image/Planet/PlanetSnow.png");
	}
	else if (type == TYPE::TERRESTRIAL || type == TYPE::TERRESTRIAL_TUTORIAL) {
		this->type = type;
		UpdatePlanetIcon("Image/Planet/PlanetTerrestrial.png");
	}
	else {
		this->type = type;
		UpdatePlanetIcon("Image/Planet/PlanetFinal.png");
	}
}

int CPlanet::getType(void)
{
	return this->type;
}

void CPlanet::SetScale(float newScale)
{
	this->scale = newScale;
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH * scale, cSettings->TILE_HEIGHT * scale);
}

bool CPlanet::UpdatePlanetIcon(const char* newPlanetIcon)
{
	planetIcon = newPlanetIcon;
	if (isVisible) {
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID(planetIcon, true);
		if (iTextureID == 0)
		{
			cout << "Unable to load planet image " << planetIcon << ".png" << endl;
			return false;
		}
	}
	else {
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Planet/PlanetDisable.png", true);
	}
	return true;
}
