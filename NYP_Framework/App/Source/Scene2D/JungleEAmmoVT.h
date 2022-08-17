/**
 CEnemyAmmo2D
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

//Include inventory related classes
#include "InventoryManager.h"


// Include InventoryManager
#include "InventoryManagerPlanet.h"

// Include Player2D
#include "Player2D.h"

//Include SoundController
#include "..\SoundController\SoundController.h"

// Include Camera
#include "Primitives/Camera2D.h"

// Include enemy ammo
#include "EnemyAmmo2D.h"

class CJEAmmoVT : public CEnemyAmmo2D
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
	CJEAmmoVT();

	// Destructor
	virtual ~CJEAmmoVT(void);

	//setting the ammo's information needed for its travel path:
		//player location aka ammo OG location
		//player direction aka direction for amo to move in
	void setPath(const int spawnX, const int spawnY, const int eDirection);

	enum DIRECTION
	{
		LEFT = 0,
		RIGHT = 1,
		UP = 2,
		DOWN = 3,
		NUM_DIRECTIONS
	};

	//let enemy emmo interact with the map
	void InteractWithMap(void);
	bool CheckPosition(void);

	// Let enemy ammo interact with the player
	bool InteractWithPlayer(void);

	// Shoot ammo, keep it moving after it is already created, used in player class
	void ShootAmmo(void);

	// return true if ammo hits window boundaries, used to delete
	//uses ammo specific direction alrdy set in via constructor, used in player class
	bool LimitReached(void);

	//used to set active to render and check collision of ammo
	void setActive(bool active);

	//used to check if ammo is active before checking collision and rendering, etc
	bool getActive(void);

protected:
	glm::vec2 vec2OldIndex;

	// Handler to the CMap2D instance
	CMap2D* cMap2D;

	// Handler to the camera instance
	Camera2D* camera2D;

	// Handle to the CPlayer2D
	CPlayer2D* cPlayer2D;

	//For inventory
	CInventoryManager* cInventoryManager;
	CInventoryItem* cInventoryItem;

	// Keyboard Controller singleton instance
	CKeyboardController* cKeyboardController;

	//CS: The quadMesh for drawing the tiles
	CMesh* quadMesh;

	//CS: Animated Sprite
	CSpriteAnimation* animatedSprites;

	// Physics
	CPhysics2D cPhysics2D;

	// Handler to the CSoundController
	CSoundController* cSoundController;

	// ammo's colour
	glm::vec4 runtimeColour;

	//direction determines how it will travel after shot
	int direction;

	//if hit == true, aka ammo hit something, destroy ammo in player class
	bool hit;

	// Determines whether or not to render it
	bool active;

	enum TILE_INDEX
	{
		BURNABLE_BUSH = 600,
		BURNING_BUSH = 601,
		DISSOLVING_BUSH = 602
	};
};

