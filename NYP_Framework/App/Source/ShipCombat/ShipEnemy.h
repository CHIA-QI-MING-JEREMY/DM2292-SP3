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

// Include Keyboard controller
#include "Inputs\KeyboardController.h"

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

class CShipEnemy : public CEntity2D
{;
public:

	// Init
	bool Init(void);

	// Reset
	bool Reset(void);

	// Update
	void Update(const double dElapsedTime);

	enum ENEMY_TYPE
	{
		EASY = 0,
		MEDIUM,
		HARD,
		NUM_ENEMYTYPE
	};

protected:
	// Keyboard Controller singleton instance
	CKeyboardController* cKeyboardController;

	std::string ImageFileName;
	ENEMY_TYPE setType;

	// Player's colour
	glm::vec4 runtimeColour;

	// Inventory Manager
	CInventoryManager* cInventoryManager;

	// Inventory Item
	CInventoryItem* cInventoryItem;

	// Sound Controller
	CSoundController* cSoundController;

	// Constructor
	CShipEnemy(void);

	// Destructor
	virtual ~CShipEnemy(void);
};

