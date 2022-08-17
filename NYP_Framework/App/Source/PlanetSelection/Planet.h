/**
 CEnemy2D
 @brief A class which represents the enemy object
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include shader
#include "RenderControl\shader.h"

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include CEntity2D
#include "Primitives/Entity2D.h"

// Include the Map2D as we will use it to check the player's movements and actions
class CMap2D;

// Include Settings
#include "GameControl\Settings.h"

// Include AnimatedSprites
#include "Primitives/SpriteAnimation.h"

// Include InventoryManager
#include "../App/Source/Scene2D/InventoryManager.h"

// Include SoundController
#include "..\SoundController\SoundController.h"

// Include Camera
#include "Primitives/Camera2D.h"

class CPlanet : public CEntity2D
{
public:
	// Constructor
	CPlanet(void);

	// Destructor
	virtual ~CPlanet(void);

	// Init
	bool Init(void);

	// Update
	void Update(const double dElapsedTime);

	// PreRender
	void PreRender(void);

	// Render
	void Render(void);

	// PostRender
	void PostRender(void);

	// Set the indices of the enemy2D
	void Seti32vec2Index(const int iIndex_XAxis, const int iIndex_YAxis);

	// Set the UV coordinates of the enemy2D
	void Setvec2UVCoordinates(const float fUVCoordinate_XAxis, const float fUVCoordinate_YAxis);

	// Get the indices of the enemy2D
	glm::vec2 Geti32vec2Index(void) const;

	// Set the UV coordinates of the planet
	glm::vec2 Getvec2UVCoordinates(void) const;

	// boolean flag to indicate if this enemy is active
	bool bIsActive;

	//// colour functions
	//glm::vec4 getColour();
	//void setColour(glm::vec4 colour);

	void SetVisibility(bool isVisible);
	bool getVisibility(void);

	bool hasExplored;

	void SetScale(float newScale);
	bool UpdatePlanetIcon(const char* newPlanetIcon);

protected:

	glm::vec2 i32vec2OldIndex;

	// CS: Animated Sprite
	CSpriteAnimation* animatedSprites;

	//CS: The quadMesh for drawing the tiles
	CMesh* quadMesh;

	// Handler to the camera instance
	Camera2D* camera2D;

	// Handler to the CMap2D instance
	CMap2D* cMap2D;

	// A transformation matrix for controlling where to render the entities
	glm::mat4 transform;

	// The vec2 which stores the indices of the enemy2D in the Map2D
	glm::vec2 i32vec2Index;

	// The vec2 variable which stores the UV coordinates to render the enemy2D
	glm::vec2 vec2UVCoordinate;

	// Settings
	CSettings* cSettings;

	// Current color
	glm::vec4 runtimeColour;
	float scale;

	// Sound Controller
	CSoundController* cSoundController;

	// planet Customisation
	const char* planetName;
	const char* planetIcon = "Image/Planet/PlanetDefault.png";

	bool isVisible;
	// TODO: [SP3] Find way to link to platformer level

	// colour variables
	glm::vec4 colour;
};

