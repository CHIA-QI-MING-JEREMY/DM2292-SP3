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
#include "Map2D.h"

// Include CPlayer2D
#include "Player2D.h"

// Include CEnemy2D
#include "Enemy2D.h"

// Include CPhysics2D
#include "Physics2D.h"

// Include GUI_Scene2D
#include "GUI_Scene2D.h"

// Include vector
#include <vector>

// Include Keyboard controller
#include "Inputs\KeyboardController.h"

// Game Manager
#include "GameManager.h"

// Camera
#include "Primitives/Camera2D.h"

// Include SoundController
#include "..\SoundController\SoundController.h"

// Add your include files here

class CScene2D : public CSingletonTemplate<CScene2D>
{
	friend CSingletonTemplate<CScene2D>;

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
	CPlayer2D* cPlayer2D;
	
	// The handler containing the instance of the camera
	Camera2D* camera2D;

	// A vector containing the instance of CEnemy2Ds
	vector<CEntity2D*> enemyVector;

	// Vector containing vectors of the enemies for each map
	vector<vector<CEntity2D*>> enemyVectors;

	// Physics
	CPhysics2D cPhysics2D;

	// The handler containing the instance of CGUI_Scene2D
	CGUI_Scene2D* cGUI_Scene2D;
	
	// Keyboard Controller singleton instance
	CKeyboardController* cKeyboardController;

	// Game Manager
	CGameManager* cGameManager;

	// Sound Controller
	CSoundController* cSoundController;

	// A transformation matrix for controlling where to render the entities
	glm::mat4 transform;

	// Add your variables and methods here.
	vector<glm::vec2> alarmBoxVector;
	bool isAlarmActive;
	double maxAlarmTimer;
	double alarmTimer;

	int maxNumOfMaps; //to set the number of maps to laod in for 1 scene and the number of maps to check for enemies to push in

	// zoom (just for demo)
	// TODO: [SP3] Remove code
	bool isZoomedIn = false;
	
	// Constructor
	CScene2D(void);
	// Destructor
	virtual ~CScene2D(void);
};

