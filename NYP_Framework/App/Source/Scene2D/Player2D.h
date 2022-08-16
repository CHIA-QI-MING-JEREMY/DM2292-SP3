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
class CMap2D;

// Include Keyboard controller
#include "Inputs\KeyboardController.h"

// Include AnimatedSprites
#include "Primitives/SpriteAnimation.h"

// Include Physics2D
#include "Physics2D.h"

// Include GameManager
#include "GameManager.h"

// Include InventoryManager
#include "InventoryManager.h"


// Include InventoryManager
#include "InventoryManagerPlanet.h"

// Include SoundController
#include "..\SoundController\SoundController.h"

//include ammo for firing
#include "Ammo2D.h"

// Include Camera2D
#include "Primitives/Camera2D.h"

class CPlayer2D : public CSingletonTemplate<CPlayer2D>, public CEntity2D
{
	friend CSingletonTemplate<CPlayer2D>;
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

	//return ammolist to the scene for pre, post and normal rendering
	std::vector<CAmmo2D*> getAmmoList(void);

	// Player Movement
	bool getPlayerMoveStatus();

	// Attacks
	bool getPlayerAttackStatus();
	void setPlayerAttackStatus(bool isAttacking);
	int getPlayerAttackDirection();

protected:
	enum DIRECTION
	{
		LEFT = 0,
		RIGHT = 1,
		UP = 2,
		DOWN = 3,
		NUM_DIRECTIONS
	};

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
	CPlayer2D(void);

	// Destructor
	virtual ~CPlayer2D(void);

	// Constraint the player's position within a boundary
	void Constraint(DIRECTION eDirection = LEFT);

	// Let player interact with the map
	void InteractWithMap(void);

	// Updates the player's health and number of lives
	void UpdateHealthLives(void);

	// Physics
	CPhysics2D cPhysics2D;
	void UpdateJumpFall(const double dElapsedTime);
	int iJumpCount;

	// Checks for feasible position to move into
	bool CheckPosition(DIRECTION eDirection);

	// Checks if the player is in mid-air
	bool IsMidAir(void);
	bool onRope;

	// colours
	enum COLOUR
	{
		WHITE = 0,
		YELLOW,
		RED,
		GREEN,
		BLUE,
		NUM_COLOURS
	};

	// colour switcher
	void SetColour(COLOUR colour = WHITE);
	glm::vec4 GetColour();

	// Checks if player has obtained coloured orbs
	bool isYellowObtained;
	bool isYellowUsed;
	bool isRedObtained;
	bool isRedUsed;
	bool isGreenObtained;
	bool isGreenUsed;
	bool isBlueObtained;
	bool isBlueUsed;

	// Checkpoint coordinates
	glm::vec2 vec2CPIndex;

	// SUPERHOT movement
	// Checks if the player is moving
	bool isPlayerMoving;

	// Sounds
	bool hasLanded;

	// Attack
	bool isAttacking;
	int attackDirection;
	double attackTimer;
	double maxAttackTimer;
	//vector full of player's fired ammo
	std::vector<CAmmo2D*> ammoList;
	int shootingDirection; //shoots in the direction the player is facing

	//used to get a deactivated ammo to activate
	CAmmo2D* FetchAmmo(void);

};

