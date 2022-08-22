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

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "Map2D.h"

// Include CPlayer2D
#include "Player2D.h"

// Include CEnemy2D
#include "Enemy2D.h"
#include "JungleEnemy2DVT.h"
#include "JungleEnemy2DShyC.h"
#include "JungleEnemy2DPatrolT.h"

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

//include ammo
#include "Ammo2D.h"
#include "EnemyAmmo2D.h"
#include "JungleEAmmoVT.h"

//include resources
#include "Resource.h"

// Add your include files here

class JunglePlanet : public CSingletonTemplate<JunglePlanet>
{
	friend CSingletonTemplate<JunglePlanet>;

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

	//to decide which map, aka which level to render
		//pass in whether or not to load tutorial level
		//if true, level is set to tutorial level
		//if false, level is randomly set between 1 and 2
	void DecideLevel(bool tutorial);

	bool getIsOnShip(void);

protected:
	// The handler containing the instance of the 2D Map
	CMap2D* cMap2D;

	// The handler containing the instance of CPlayer2D
	CPlayer2D* cPlayer2D;
	
	// The handler containing the instance of the camera
	Camera2D* camera2D;

	// A vector containing the instance of CEnemy2Ds
	//vector<CEntity2D*> enemyVector;

	// Vector containing vectors of the enemies for each map
	vector<vector<CEnemy2D*>> enemyVectors;

	// Vector containing vectors containig the resources in the level
	vector<vector<CResource*>> resourceVectors;

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

	// Inventory Manager
	CInventoryManagerPlanet* cInventoryManagerPlanet;

	// Inventory Item
	CInventoryItemPlanet* cInventoryItemPlanet;


	// A transformation matrix for controlling where to render the entities
	glm::mat4 transform;

	// Add your variables and methods here.
	vector<glm::vec2> alarmBoxVector;
	bool isAlarmActive;
	double maxAlarmTimer;
	double alarmTimer;

	enum LEVELS
	{
		TUTORIAL = 0,
		LEVEL1,
		LEVEL2A,
		LEVEL2B,
		NUM_LEVELS //to set the number of maps to load in for 1 scene and the number of maps to check for enemies to push in
	};

	enum TILE_INDEX
	{
		BURNABLE_BUSH = 600,
		BURNING_BUSH = 601,
		DISSOLVING_BUSH = 602
	};

	int playerPoisonLevel; 
		//dictates how badly affected the player is by poison
		//poison level 0 means the player has not been poisoned
		//from 0 to 3, caps at 3, increases by 1 everytime the player is hit by something poisonous with a 2 second cooldwon inbetween
	double poisonLevelIncreaseCooldown;
		//IMPORTANT NOTE: COOLDOWN ONLY APPLIES FOR POISON FROM POISON SPROUT AND POISON FOG, NOT ENEMY POISON AMMO
	const double poisonLevelIncreaseMaxCooldown = 2.0; //used to reset cooldown

	double poisonDamageHitCooldown; //starts off at max cooldown whenever the poison lvl is set/changes before depleting
		//once it hits 0, player takes damage
	vector<int> poisonDamage; //contains the int of how much of the health is removed from player per damage hit
		//0 means that the player takes 0% damage
		//1 means that the player takes 3% damage
		//2 means that the player takes 5% damage
		//3 means that the player takes 10% damage
	vector<double> poisonDamageHitMaxCooldown; //contains the max cooldown to be used for each poison level
		//0 would be 0.0s
		//1 would be 5.0s
		//2 would be 4.0s
		//3 would be 3.0s

	double poisonPurpleCooldown; //cooldown to flicker between the purple colour and white colour to represent being poisoned
	const double poisonPurpleMaxCooldown = 0.5; //flicker every 0.5 seconds
	bool poisonPurple; //decide if colour should be switched to purple or white

	double swayingLeavesCooldown; //cooldown to switch leaves with its alt positions
	const double swayingLeavesMaxCooldown = 1; //flicker every 1 second


	// Let player interact with the map
	void PlayerInteractWithMap(void);


	// zoom (just for demo)
	// TODO: [SP3] Remove code
	bool isZoomedIn = false;
	
	// Constructor
	JunglePlanet(void);
	// Destructor
	virtual ~JunglePlanet(void);
};

