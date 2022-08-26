/**
 CPlayer2D
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
#include "../App/Source/Scene2D/Map2D.h"

// Include Keyboard controller
#include "Inputs\KeyboardController.h"

// Include AnimatedSprites
#include "Primitives/SpriteAnimation.h"

// Include Physics2D
#include "../App/Source/Scene2D/Physics2D.h"

// Include GameManager
#include "../App/Source/Scene2D/GameManager.h"

// Include InventoryManager
#include "../App/Source/Scene2D/InventoryManager.h"

// Include ShipInventoryManager
#include "InventoryManagerShip.h"

// Include SoundController
#include "..\SoundController\SoundController.h"

// Include Camera2D
#include "Primitives/Camera2D.h"

class CShipPlayer : public CSingletonTemplate<CShipPlayer>, public CEntity2D
{
	friend CSingletonTemplate<CShipPlayer>;
public:

	// Init
	bool Init(void);

	// Reset
	bool Reset(void);

	// Update
	void Update(const double dElapsedTime);

	// PreRender
	void PreRender(void);

	// Render
	void Render(void);

	// PostRender
	void PostRender(void);

	enum DIRECTION
	{
		LEFT = 0,
		RIGHT = 1,
		UP = 2,
		DOWN = 3,
		NUM_DIRECTIONS
	};

	DIRECTION player_Dir;

	// Weapon by itzy
	float TimeElapsed;
	float weapon1Interval;
	float Weapon1Time;
	float weapon2Interval;
	float Weapon2Time;
	float LargeWeaponInterval;
	float LargeWeaponTime;
	
protected:
	glm::vec2 vec2OldIndex;

	// Handler to the CMap2D instance
	CMap2D* cMap2D;

	// CS: Animated Sprite
	CSpriteAnimation* animatedSprites;

	// Keyboard Controller singleton instance
	CKeyboardController* cKeyboardController;

	// CS: The quadMesh for drawing the tiles
	CMesh* quadMesh;
	
	// The handler containing the instance of the camera
	Camera2D* camera2D;

	// Player's colour
	glm::vec4 runtimeColour;

	// Inventory Manager
	CInventoryManager* cInventoryManager;

	// Inventory Item
	CInventoryItem* cInventoryItem;

	// Sound Controller
	CSoundController* cSoundController;

	// Constructor
	CShipPlayer(void);

	// Destructor
	virtual ~CShipPlayer(void);

	// Constraint the player's position within a boundary
	void Constraint(DIRECTION eDirection = LEFT);

	// Updates the player's health and number of lives
	void UpdateHealthLives(void);

	// Let player interact with the map
	void InteractWithMap(void);

	// Physics
	CPhysics2D cPhysics2D;

	// Checks for feasible position to move into
	bool CheckPosition(DIRECTION eDirection);

	// SUPERHOT movement
	// Checks if the player is moving
	bool isPlayerMoving;
};

