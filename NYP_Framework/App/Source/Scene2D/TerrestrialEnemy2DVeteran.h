/**
 CEnemy2D
 @brief A class which represents the enemy object
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include shader
#include "RenderControl\shader.h"

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include CEntity2D
#include "Primitives/Entity2D.h"

//Include SoundController
#include "..\SoundController\SoundController.h"

// Include the Map2D as we will use it to check the player's movements and actions
class CMap2D;

// Include Settings
#include "GameControl\Settings.h"

// Include Physics2D
#include "Physics2D.h"

// Include Player2D
#include "Player2D.h"

//include ammo
#include "EnemyAmmo2D.h"
#include "TerrestrialEAmmoVeteran.h"

//include enemy base class
#include "Enemy2D.h"

// Include AnimatedSprites
#include "Primitives/SpriteAnimation.h"

// Include Camera
#include "Primitives/Camera2D.h"

class TEnemy2DVeteran : public CEnemy2D
{
public:
	// Constructor
	TEnemy2DVeteran(void);

	// Destructor
	virtual ~TEnemy2DVeteran(void);

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

	// Set the indices of the enemy2D
	void Seti32vec2Index(const int iIndex_XAxis, const int iIndex_YAxis);

	// Set the number of microsteps of the enemy2D
	void Seti32vec2NumMicroSteps(const int iNumMicroSteps_XAxis, const int iNumMicroSteps_YAxis);

	// Set the UV coordinates of the enemy2D
	void Setvec2UVCoordinates(const float fUVCoordinate_XAxis, const float fUVCoordinate_YAxis);

	// Get the indices of the enemy2D
	glm::vec2 Geti32vec2Index(void) const;

	// Get the number of microsteps of the enemy2D
	glm::vec2 Geti32vec2NumMicroSteps(void) const;

	// Set the UV coordinates of the enemy2D
	glm::vec2 Getvec2UVCoordinates(void) const;

	// Set the handle to cPlayer to this class instance
	void SetPlayer2D(CPlayer2D* cPlayer2D);

	// boolean flag to indicate if this enemy is active
	bool bIsActive;

protected:
	enum DIRECTION
	{
		LEFT = 0,
		RIGHT = 1,
		UP = 2,
		DOWN = 3,
		NUM_DIRECTIONS
	};

	vector<glm::vec2> ConstructWaypointVector(vector<glm::vec2> waypointVector, int startIndex, int numOfWaypoints);

	////vector full of enemy's fired ammo
	std::vector<CTEAmmoVeteran*> ammoList;
	int shootingDirection; //shoots in the direction the enemy is facing
	//used to get a deactivated ammo to activate
	CTEAmmoVeteran* FetchAmmo(void);

	//CS: Animated Sprite
	CSpriteAnimation* animatedSprites;

	// TO DO
	enum FSM
	{
		IDLE = 0,
		PATROL = 1,
		TRACK = 2,
		SHOOT = 3,
		WARN = 4,
		ALARM_TRIGGER = 5,
		ALERT_IDLE = 6,
		ALERT_PATROL = 7,
		ALERT_TRACK = 8,
		ALERT_SHOOT = 9,
		NUM_FSM
	};

	glm::vec2 i32vec2OldIndex;

	//CS: The quadMesh for drawing the tiles
	CMesh* quadMesh;

	// Handler to the CMap2D instance
	CMap2D* cMap2D;

	// Handler to the camera instance
	Camera2D* camera2D;

	// A transformation matrix for controlling where to render the entities
	glm::mat4 transform;

	// The vec2 variable which stores the UV coordinates to render the enemy2D
	glm::vec2 vec2UVCoordinate;

	// Settings
	CSettings* cSettings;

	// Physics
	CPhysics2D cPhysics2D;

	// waypoint path
	vector<glm::vec2> waypoints;
	// waypoint counter
	int currentWaypointCounter;
	int maxWaypointCounter;

	// Current color
	glm::vec4 runtimeColour;

	// Handle to the CPlayer2D
	CPlayer2D* cPlayer2D;

	// Handler to the CSoundController
	CSoundController* cSoundController;

	// Current FSM
	FSM sCurrentFSM;

	// FSM counter - count how many frames it has been in this FSM
	int iFSMCounter;

	// Max count in a state
	const int iMaxFSMCounter = 60;

	// Constraint the enemy2D's position within a boundary
	void Constraint(DIRECTION eDirection = LEFT);

	// Check if a position is possible to move into
	bool CheckPosition(DIRECTION eDirection);

	// Check if the enemy2D is in mid-air
	bool IsMidAir(void);

	// Update Jump or Fall
	void UpdateJumpFall(const double dElapsedTime = 0.0166666666666667);

	// Let enemy2D interact with the player
	bool InteractWithPlayer(void);

	//let enemy2D interact with the map
	void InteractWithMap(void);

	// Update direction
	void UpdateDirection(void);

	// Flip horizontal direction. For patrol use only
	void FlipHorizontalDirection(void);

	// Update position
	void UpdatePosition(void);

	// attack timer
	double attackTimer;
	const double attackInterval = 0.4;
	const double alertAttackInterval = 0.2;

	// checks how many bullets have been fired
	int numFired;
	const int attackMagSize = 3;
	const int alertAttackMagSize = 5;

	vector<glm::vec2>alarmBoxVector;
	bool isAlarmBoxAssigned;
	double warnTimer;
	const double maxWarnTimer = 2.0;
	float alarmBoxDistance;
};

