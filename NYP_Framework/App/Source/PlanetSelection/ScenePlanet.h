/**
 CScene2D
 @brief A class which manages the 2D game scene
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include SingletonTemplate
#include "DesignPatterns/SingletonTemplate.h"

// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "../App/Source/Scene2D/Map2D.h"

// Include CPlayer2D
#include "../App/Source/Scene2D/Player2D.h"

// Include GUI_Scene2D
#include "../App/Source/Scene2D/GUI_Scene2D.h"

// Include vector
#include <vector>

// Include Keyboard controller
#include "Inputs\KeyboardController.h"

// Game Manager
#include "../App/Source/Scene2D/GameManager.h"

// Camera
#include "Primitives/Camera2D.h"

// Include SoundController
#include "..\SoundController\SoundController.h"

// Add your include files here

class CScenePlanet : public CSingletonTemplate<CScenePlanet>
{
	friend CSingletonTemplate<CScenePlanet>;

public:
	// Init
	bool Init(void);

	// Update
	bool Update(const double dElapsedTime);

	// PreRender
	void PreRender(void);

	// Render
	void Render(void);

	// PostRender
	void PostRender(void);

protected:
	// The handler containing the instance of the 2D Map
	CMap2D* cMap2D;

	// The handler containing the instance of CPlayer2D
	// IDK if i need this so i'm going to keep it in first
	CPlayer2D* cPlayer2D;
	
	// The handler containing the instance of the camera
	Camera2D* camera2D;

	// A vector containing the instance of CEnemy2Ds
	//vector<CEntity2D*> enemyVector;

	// The handler containing the instance of CGUI_Scene2D
	// TODO: [SP3] Create new GUI
	CGUI_Scene2D* cGUI_Scene2D;
	
	// Keyboard Controller singleton instance
	CKeyboardController* cKeyboardController;

	// Game Manager
	CGameManager* cGameManager;

	// Sound Controller
	CSoundController* cSoundController;

	// A transformation matrix for controlling where to render the entities
	glm::mat4 transform;

	// Scene Specific Variables
	int PlanetSelected;
	
	// Constructor
	CScenePlanet(void);
	// Destructor
	virtual ~CScenePlanet(void);
};

