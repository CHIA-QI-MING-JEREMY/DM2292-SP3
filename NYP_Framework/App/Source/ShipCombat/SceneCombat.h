/**
 JunglePlanet
 @brief A class which manages the Jungle Planets/Levels
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

// Include background bitch
#include "../App/Source/Scene2D/BackgroundEntity.h"

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "../App/Source/Scene2D/Map2D.h"

// Include CPlayer2D
#include "ShipPlayer.h"

// Include CShip
#include "Ship.h"

// Include CPhysics2D
#include "../App/Source/Scene2D/Physics2D.h"

// Include vector
#include <vector>

// Include Keyboard controller
#include "Inputs\KeyboardController.h"

// Game Manager
#include "../App/Source/Scene2D/GameManager.h"

// Camera
#include "Primitives/Camera2D.h"

// Include GUI
#include "GUI_SceneCombat.h"

// Include SoundController
#include "..\SoundController\SoundController.h"

// Include Enemy
#include "ShipEnemy.h"


// Add your include files here

class CSceneCombat : public CSingletonTemplate<CSceneCombat>
{
	friend CSingletonTemplate<CSceneCombat>;

public:
	// State
	enum CURRENT_STATE
	{
		SHIP_NOSTATE = 0,
		SHIPUPGRADE_NP,
		SHIPUPGRADE,
		SHIPREST,
		SHIPCOMBAT,
		SHIPLANDED,
		NUM_STATES
	};

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

	int getNumEncounters(void);
	bool goToNextScene;
	void SetNewState(int newState);
	int getCurrentState();

	float prevFightTime;
	float fightTimeElapsed;
	bool isDead;

protected:
	// constants
	float kBreakTime = 3.0f;

	CBackgroundEntity* background;

	CURRENT_STATE state;

	// The handler containing the instance of the 2D Map
	CMap2D* cMap2D;

	// THe handler containing the instance of a ship
	CShip* cShip;

	// The handler containing the instance of the enemy
	CShipEnemy* cShipEnemy;

	// The handler containing the instance of CPlayer2D
	CShipPlayer* cPlayer2D;
	
	// The handler containing the instance of the camera
	Camera2D* camera2D;

	// Physics
	CPhysics2D cPhysics2D;

	// The handler containing the instance of CGUI_Scene2D
	CGUI_SceneCombat* cGUI_SceneCombat;
	
	// Keyboard Controller singleton instance
	CKeyboardController* cKeyboardController;

	// Game Manager
	CGameManager* cGameManager;

	// Sound Controller
	CSoundController* cSoundController;

	// A transformation matrix for controlling where to render the entities
	glm::mat4 transform;

	// Let player interact with the map
	// why is this here lol
	void PlayerInteractWithMap(glm::vec2 position);

	// zoom (just for demo)
	// TODO: [SP3] Remove code
	bool isZoomedIn = false;
	bool lState;
	glm::vec2 blockSelected;

	// Ship Combat !!
	int numOfEncounters;
	bool isEncounter;
	
	// Constructor
	CSceneCombat(void);
	// Destructor
	virtual ~CSceneCombat(void);
};

