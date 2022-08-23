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

//Include SoundController
#include "..\SoundController\SoundController.h"

// Include Camera2D
#include "Primitives/Camera2D.h"

class CAmmo2D : public CEntity2D
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
	CAmmo2D();

	// Destructor
	virtual ~CAmmo2D(void);

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

	//let ammo interact with the map
	void InteractWithMap(void);
	bool CheckPosition(void);

	//let ammo interact with enemies
	//bool InteractWithEnemy(CEnemy2D* enemy); //called in scene, 1 ammo in the vec to 1 enemy in the vec
		//if return is true, delete the enemy passed in from the enemy vector and set it to null
	bool InteractWithEnemy(glm::i32vec2 i32vec2EnemyPos);

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

	// Keyboard Controller singleton instance
	CKeyboardController* cKeyboardController;

	//CS: The quadMesh for drawing the tiles
	CMesh* quadMesh;

	//CS: Animated Sprite
	CSpriteAnimation* animatedSprites;

	// Handler to the CSoundController
	CSoundController* cSoundController;

	// The handler containing the instance of the camera
	Camera2D* camera2D;

	// Physics
	CPhysics2D cPhysics2D;

	// ammo's colour
	glm::vec4 runtimeColour;

	//direction determines how it will travel after shot
	int direction;

	//if hit == true, aka ammo hit something, destroy ammo in player class
	bool hit;

	// Determines whether or not to render it
	bool active;
};

