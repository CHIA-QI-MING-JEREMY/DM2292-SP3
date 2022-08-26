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

class CShip : public CSingletonTemplate<CShip>, public CEntity2D
{
	friend CSingletonTemplate<CShip>;
public:

	// Init
	bool Init(void);

	// Reset
	bool Reset(void);

	// Update
	void Update(const double dElapsedTime);

	void ResetVentilationTimer(void);

	// ventilation
	float ventilationInterval;
	float TimeElapsed;
	float ventilationTiming;

protected:
	// Keyboard Controller singleton instance
	CKeyboardController* cKeyboardController;

	std::string ImageFileName;

	// Player's colour
	glm::vec4 runtimeColour;

	// Inventory Manager
	CInventoryManager* cInventoryManager;

	// Inventory Item
	CInventoryItem* cInventoryItem;

	// Sound Controller
	CSoundController* cSoundController;

	// Small damage
	int smallWeaponDamage;

	// large damage
	int largeWeaponDamage;

	// Constructor
	CShip(void);

	// Destructor
	virtual ~CShip(void);
};

