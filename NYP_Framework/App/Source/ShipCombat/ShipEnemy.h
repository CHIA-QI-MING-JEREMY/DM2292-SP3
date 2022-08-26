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

// Additional includes
#include <string>

// Include CEntity2D
#include "Primitives/Entity2D.h"

// Include Keyboard controller
#include "Inputs\KeyboardController.h"

// Include GameManager
#include "../App/Source/Scene2D/GameManager.h"

// Include InventoryManager
#include "../App/Source/Scene2D/InventoryManager.h"

// Include Player ??
#include "ShipPlayer.h"

// Include ShipInventoryManager
#include "InventoryManagerShip.h"

// Include SoundController
#include "..\SoundController\SoundController.h"

// Include Camera2D
#include "Primitives/Camera2D.h"

class CShipEnemy : public CSingletonTemplate<CShipEnemy>, public CEntity2D
{
	friend CSingletonTemplate<CShipEnemy>;
public:

	// Init
	bool Init(void);

	// Reset
	bool Reset(void);

	// Update
	void Update(const double dElapsedTime);

	void Attack(void);
	void Randomise(int position);
	int getHealth(void);
	void setHealth(int h);

	enum ENEMY_TYPE
	{
		EASY = 0,
		MEDIUM,
		HARD,
		NUM_ENEMYTYPE
	};

	// display information
	std::string enemyName;
	ENEMY_TYPE enemType;
	float enemyTimer = 10.0f;
	float TimeElapsed;
	float attackTimer;
	int maxHealth = 0;

protected:
	// constants
	float kScreenShakeLength = 0.5f;
	float kWarningLength = 0.5f;

	// Keyboard Controller singleton instance
	CKeyboardController* cKeyboardController;

	int attackCounter;

	// Player's colour
	glm::vec4 runtimeColour;

	// Inventory Manager
	CInventoryManager* cInventoryManager;

	// Player character
	CShipPlayer* cPlayer;

	// Inventory Item
	CInventoryItem* cInventoryItem;

	// Sound Controller
	CSoundController* cSoundController;

	// Constructor
	CShipEnemy(void);

	// Screen shake
	
	float NoiseStartTime;
	int enemyHealth;
	void SetDamage(glm::vec2 position);
	void SetDanger(glm::vec2 position);
	std::vector<std::pair<glm::vec2, float>> tileVector;

	// Destructor
	virtual ~CShipEnemy(void);
};

