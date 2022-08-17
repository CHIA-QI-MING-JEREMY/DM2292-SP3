/**
 CAmmo2D
 @brief A class representing the player object
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include Singleton template
#include "DesignPatterns\SingletonTemplate.h"

// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include CEntity2D
#include "Primitives/Entity2D.h"

// Include the Map2D as we will use it to check the player's movements and actions
class CMap2D;

// Include Keyboard controller
#include "Inputs\KeyboardController.h"

// Include AnimatedSprites
#include "Primitives/SpriteAnimation.h"

// Include Physics
#include "Physics2D.h"

// Include Player2D
#include "Player2D.h"

//Include SoundController
#include "..\SoundController\SoundController.h"

// Include Camera2D
#include "Primitives/Camera2D.h"

// Include math.h
#include <math.h>

// Include InventoryManager
#include "InventoryManagerPlanet.h"

//// Include enemies
//#include "Enemy2D.h"
//#include "MblEnemy2D.h"
//#include "StationaryEnemy2D.h"

class CResource : public CEntity2D
{
public:

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

	// Constructor
	CResource();
	CResource(int type); //used to create a specific resource

	// Destructor
	virtual ~CResource(void);

	//enum to decide what resource is dropped
	enum RESOURCE_TYPE 
	{
		SCRAP_METAL = 0,
		BATTERY,
		IRONWOOD,
		ENERGY_QUARTZ,
		ICE_CRYSTAL,
		NUM_RESOURCES
	};

	//sets its spawn location, including microsteps
		//will fall to land on a platform in update where updatefall is called
	void setPosition(glm::vec2 indexPos, glm::vec2 microStep);

	enum DIRECTION
	{
		LEFT = 0,
		RIGHT = 1,
		UP = 2,
		DOWN = 3,
		NUM_DIRECTIONS
	};

	//let ammo interact with the map
	bool CheckPosition(void);



	// return true if ammo hits window boundaries, used to delete
	//uses ammo specific direction alrdy set in via constructor, used in player class
	bool LimitReached(void);

	bool getCollected(void); //return collected
		//if true means delete resource


protected:
	glm::vec2 vec2OldIndex;

	// Handler to the CMap2D instance
	CMap2D* cMap2D;

	// Handle to the CPlayer2D
	CPlayer2D* cPlayer2D;

	// Keyboard Controller singleton instance
	CKeyboardController* cKeyboardController;

	//CS: The quadMesh for drawing the tiles
	CMesh* quadMesh;

	//CS: Animated Sprite
	//CSpriteAnimation* animatedSprites;

	// Handler to the CSoundController
	CSoundController* cSoundController;

	// The handler containing the instance of the camera
	Camera2D* camera2D;

	// Physics
	CPhysics2D cPhysics2D;
	void UpdateFall(const double dElapsedTime);

	// Inventory Manager
	CInventoryManagerPlanet* cInventoryManagerPlanet;

	// Inventory Item
	CInventoryItemPlanet* cInventoryItemPlanet;

	// Let resource interact with the player
	bool InteractWithPlayer(void);

	// resource's colour
	glm::vec4 runtimeColour;

	//direction determines how it will travel after shot
	int direction;

	//if collected == true, aka resource is collected, delete resource in scene
	bool collected;

	//determine which resource it is
	int type;
};

