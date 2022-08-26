/**
 CStnEnemy2D
 @brief A class which represents the enemy object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "JungleEnemy2DPatrolT.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"
// Include Mesh Builder
#include "Primitives/MeshBuilder.h"

// Include GLEW
#include <GL/glew.h>

// Include ImageLoader
#include "System\ImageLoader.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "Map2D.h"
// Include math.h
#include <math.h>

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
JEnemy2DPatrolT::JEnemy2DPatrolT(void)
	: bIsActive(false)
	, cMap2D(NULL)
	, cSettings(NULL)
	, cPlayer2D(NULL)
	, sCurrentFSM(FSM::PATROL)
	, iFSMCounter(0)
	, quadMesh(NULL)
	//, camera2D(NULL)
	, animatedSprites(NULL)
{
	transform = glm::mat4(1.0f);	// make sure to initialize matrix to identity matrix first

	// Initialise vecIndex
	vec2Index = glm::i32vec2(0);

	// Initialise vecNumMicroSteps
	i32vec2NumMicroSteps = glm::i32vec2(0);

	// Initialise vec2UVCoordinate
	vec2UVCoordinate = glm::vec2(0.0f);

	vec2Destination = glm::i32vec2(0, 0);	// Initialise the iDestination
	vec2Direction = glm::i32vec2(0, 0);		// Initialise the iDirection
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
JEnemy2DPatrolT::~JEnemy2DPatrolT(void)
{
	// Delete the quadMesh
	if (quadMesh)
	{
		delete quadMesh;
		quadMesh = NULL;
	}

	// We won't delete this since it was created elsewhere
	cPlayer2D = NULL;

	// We won't delete this since it was created elsewhere
	cMap2D = NULL;

	// Delete the CAnimationSprites
	if (animatedSprites)
	{
		delete animatedSprites;
		animatedSprites = NULL;
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

/**
  @brief Initialise this instance
  */
bool JEnemy2DPatrolT::Init(void)
{
	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();
	// Get the handler to the Camera2D instance
	camera2D = Camera2D::GetInstance();

	// Get the handler to the CSoundController instance
	cSoundController = CSoundController::GetInstance();

	// Get the handler to the CMap2D instance
	cMap2D = CMap2D::GetInstance();
	// Find the indices for the player in arrMapInfo, and assign it to CStnEnemy2D
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	if (cMap2D->FindValue(1620, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the enemy, so quit this game

	// Erase the value of the player in the arrMapInfo
	cMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	i32vec2NumMicroSteps = glm::i32vec2(0, 0);

	// Create and initialise the CPlayer2D
	cPlayer2D = CPlayer2D::GetInstance();

	// Get the handler to the CInventoryManager instance
	cInventoryManagerPlanet = CInventoryManagerPlanet::GetInstance();

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//CS: Create the Quad Mesh using the mesh builder
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

	// Load the enemy2D texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/JunglePlanet/PatrolTeamSpriteSheet.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/JunglePlanet/PatrolTeamSpriteSheet.png" << endl;
		return false;
	}

	//CS: Create the animated spirte and setup the animation
	animatedSprites = CMeshBuilder::GenerateSpriteAnimation(7, 4,
		cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	//^ loads a spirte sheet with 3 by 3 diff images, all of equal size and positioning
	animatedSprites->AddAnimation("idle", 0, 0); //idle
	animatedSprites->AddAnimation("movingL", 1, 4); //4 images for animation, index 1 to 4, walking left, 0.5f speed
	animatedSprites->AddAnimation("movingR", 5, 8); //walking right, 0.5f speed
	animatedSprites->AddAnimation("noisyL", 9, 12); //noisy left, 0.75f speed
	animatedSprites->AddAnimation("noisyR", 13, 16); //noisy right, 0.75f speed
	animatedSprites->AddAnimation("enrouteL", 17, 20); //en_route left, 0.5f speed
	animatedSprites->AddAnimation("enrouteR", 21, 24); //en_route right, 0.5f speed

	//CS: Play the "idle" animation as default
	animatedSprites->PlayAnimation("idle", -1, 0.5f);
	//-1 --> repeats forever
	//		settng it to say 1 will cause it to only repeat 1 time
	//1.0f --> set time between frames as 1.0f
	//		increasing this number will cause the animation to slowdown

	//CS: Init the color to white
	runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);

	// Set the Physics to fall status by default
	cPhysics2D.Init();
	cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);

	// If this class is initialised properly, then set the bIsActive to true
	bIsActive = true;

	//Construct 100 inactive ammo and add into ammoList
	for (int i = 0; i < 100; ++i)
	{
		CJEAmmo* cEnemyAmmo2D = new CJEAmmo();
		cEnemyAmmo2D->SetShader("Shader2D");
		ammoList.push_back(cEnemyAmmo2D);
	}

	//no patrol team enemies in tutorial lvl
	//for lvl 1
	if (cMap2D->GetCurrentLevel() == 1)
	{
		//if it's the enemy at this position
		if (vec2Index == glm::vec2(1, 4))
		{
			waypoints = ConstructWaypointVector(waypoints, 130, 4); //130, 131, 132, 133
		}
		else if (vec2Index == glm::vec2(14, 12))
		{
			waypoints = ConstructWaypointVector(waypoints, 140, 2); //140, 141
		}
		else if (vec2Index == glm::vec2(20, 17))
		{
			waypoints = ConstructWaypointVector(waypoints, 150, 3); //150, 151, 152
		}
	}
	//for lvl 3
	if (cMap2D->GetCurrentLevel() == 2)
	{
		//if it's the enemy at this position
		if (vec2Index == glm::vec2(28, 17))
		{
			waypoints = ConstructWaypointVector(waypoints, 130, 4); //130, 131, 132, 133
		}
		else if (vec2Index == glm::vec2(5, 19))
		{
			waypoints = ConstructWaypointVector(waypoints, 140, 4); //140, 141, 142, 143
		}
		else if (vec2Index == glm::vec2(26, 22))
		{
			waypoints = ConstructWaypointVector(waypoints, 150, 4); //150, 151, 152, 153
		}
	}
	

	// sets waypoint counter value
	currentWaypointCounter = 0;
	maxWaypointCounter = waypoints.size();

	type = COMMUNITY; //effects more of its own kind
	shootingDirection = LEFT; //setting direction for ammo shooting
	maxHealth = health = 50; //takes 10 hits to kill

	flickerTimer = 0.5; //used to progress the flicker counter
	flickerTimerMax = 0.5; //used to reset flicker counter
	flickerCounter = 0; //decides colour of enemy and when to explode

	attackCooldownCurrent = attackCooldownMax; //the cooldown that gets dt-ed away
	healingCooldown = 0.0; //timer between when the enemy heals when in new location

	noisy = false;
	alerted = false;

	return true;
}

/**
 @brief Update this instance
 */
void JEnemy2DPatrolT::Update(const double dElapsedTime)
{
	if (!bIsActive)
		return;

	//prevent health from going over maximum
	if (health > maxHealth)
	{
		health = maxHealth;
	}

	//attack cooldown
	if (attackCooldownCurrent > 0) //if not at 0
	{
		attackCooldownCurrent -= dElapsedTime; //minus
	}
	else
	{
		attackCooldownCurrent = 0;
	}

	// just for switching between states --> keep simple
	//action done under interaction with player, update position, update direction, etc
	switch (sCurrentFSM)
	{
	case IDLE:
		//Play the "idle" animation
		animatedSprites->PlayAnimation("idle", -1, 1.f);
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 4.0f) //near player
		{
			sCurrentFSM = NOISY;
			iFSMCounter = 0;
			cout << "Switching to Noisy State" << endl;
			break;
		}
		if (alerted) //if near a noisy patrol team enemy
		{
			sCurrentFSM = EN_ROUTE;
			iFSMCounter = 0;
			cout << "Switching to En Route State" << endl;
			break;
		}
		if (iFSMCounter > iMaxFSMCounter) //been in idle for too long
		{
			sCurrentFSM = PATROL;
			iFSMCounter = 0;
			cout << "Switching to Patrol State" << endl;
			++currentWaypointCounter; //increase the enemy's current waypoint count
			break;
		}
		iFSMCounter++;
		break;
	case PATROL:
		//set way point to get to
			//if pointing at the spot beyond the last waypoint index, set current back to first
		if (currentWaypointCounter == maxWaypointCounter)
		{
			currentWaypointCounter = 0;
		}
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 4.0f) //near player
		{
			sCurrentFSM = NOISY;
			iFSMCounter = 0;
			cout << "Switching to Noisy State" << endl;
			break;
		}
		if (alerted) //if near a noisy patrol team enemy
		{
			sCurrentFSM = EN_ROUTE;
			iFSMCounter = 0;
			cout << "Switching to En Route State" << endl;
			break;
		}
		if (cPhysics2D.CalculateDistance(vec2Index, waypoints[currentWaypointCounter]) < 0.5f) //close to current waypoint
		{
			sCurrentFSM = IDLE;
			iFSMCounter = 0;
			cout << "Switching to Idle State" << endl;
		}
		//pathfind to waypoint
		else 
		{
			glm::vec2 startIndices;
			if (vec2NumMicroSteps.x == 0)
			{
				startIndices = glm::vec2(vec2Index.x, vec2Index.y);
			}
			else
			{
				startIndices = glm::vec2(vec2Index.x + 1, vec2Index.y);
			}

			auto path = cMap2D->PathFind(startIndices,// start pos
				waypoints[currentWaypointCounter],	// target pos
				heuristic::manhattan,				// heuristic
				10);								// weight

			// Calculate new destination
			bool bFirstPosition = true;
			for (const auto& coord : path)
			{
				//std::cout << coord.x << ", " << coord.y << "\n";
				if (bFirstPosition == true)
				{
					// Set a destination
					vec2Destination = coord;

					// Calculate the direction between enemy2D and this destination
					vec2Direction = vec2Destination - vec2Index;
					bFirstPosition = false;
				}
				else
				{
					if ((coord - vec2Destination) == vec2Direction)
					{
						// Set a destination
						vec2Destination = coord;
					}
					else
					{
						break;
					}
				}
			}

			UpdatePosition();
		}
		iFSMCounter++;
		break;
	case NOISY:
		noisy = true; //used to let other enemies know this enemy is in noisy mode
			//follow player and attack
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::PATROL_TEAM_NOISY); //play noisy siren sound
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 4.0f) //near player
		{
			glm::vec2 startIndices;
			if (vec2NumMicroSteps.x == 0)
			{
				startIndices = glm::vec2(vec2Index.x, vec2Index.y);
			}
			else
			{
				startIndices = glm::vec2(vec2Index.x + 1, vec2Index.y);
			}

			auto path = cMap2D->PathFind(startIndices,
				cPlayer2D->vec2Index,
				heuristic::manhattan,
				10);

			// Calculate new destination
			bool bFirstPosition = true;
			for (const auto& coord : path)
			{
				//std::cout << coord.x << "," << coord.y << "\n";
				if (bFirstPosition == true)
				{
					// Set a destination
					vec2Destination = coord;
					// Calculate the direction between enemy2D and this destination
					vec2Direction = vec2Destination - vec2Index;
					bFirstPosition = false;
				}
				else
				{
					if ((coord - vec2Destination) == vec2Direction)
					{
						// Set a destination
						vec2Destination = coord;
					}
					else
					{
						break;
					}
				}
			}

			// Update the Enemy2D's position for attack
			UpdatePosition();

			//say the enemy is unable to move but is still in attack mode, trying to move to the player
			if (shootingDirection == LEFT && attackCooldownCurrent == 0)
				//if player is to the left of the enemy
				//and enemy is able to attack again
			{
				//if between them and the player is a burnable bush
				if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x - 1) == CMap2D::TILE_INDEX::BURNABLE_BUSH)
				{
					cMap2D->SetMapInfo(vec2Index.y, vec2Index.x - 1, CMap2D::TILE_INDEX::DISSOLVING_BUSH); //dissolve the bush
					attackCooldownCurrent = attackCooldownMax; //reset cooldown
					cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::ENEMY_MELEE); //play punch sound
				}
				//if between them and the player is a burning or dissolving bush
				else if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x - 1) == CMap2D::TILE_INDEX::BURNING_BUSH ||
					cMap2D->GetMapInfo(vec2Index.y, vec2Index.x - 1) == CMap2D::TILE_INDEX::DISSOLVING_BUSH)
				{
					cMap2D->SetMapInfo(vec2Index.y, vec2Index.x - 1, 0); //destroy bush and turn it to empty space
					attackCooldownCurrent = attackCooldownMax; //reset cooldown
					cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::ENEMY_MELEE); //play punch sound
				}
			}
			else if (shootingDirection == RIGHT && attackCooldownCurrent == 0)
				//if player is to the right of the enemy
				//and enemy is able to attack again
			{
				//if between them and the player is a burnable bush
				if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) == CMap2D::TILE_INDEX::BURNABLE_BUSH)
				{
					cMap2D->SetMapInfo(vec2Index.y, vec2Index.x + 1, CMap2D::TILE_INDEX::DISSOLVING_BUSH); //dissolve the bush
					attackCooldownCurrent = attackCooldownMax; //reset cooldown
					cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::ENEMY_MELEE); //play punch sound
				}
				//if between them and the player is a burning or dissolving bush
				else if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) == CMap2D::TILE_INDEX::BURNING_BUSH ||
					cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) == CMap2D::TILE_INDEX::DISSOLVING_BUSH)
				{
					cMap2D->SetMapInfo(vec2Index.y, vec2Index.x + 1, 0); //destroy bush and turn it to empty space
					attackCooldownCurrent = attackCooldownMax; //reset cooldown
					cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::ENEMY_MELEE); //play punch sound
				}
			}
		}
		else
		{
			sCurrentFSM = RETURN;
			iFSMCounter = 0;
			cout << "Switching to Return State" << endl;
			noisy = false;
		}
		iFSMCounter++;
		break;
	case EN_ROUTE:
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 4.0f) //near player
		{
			sCurrentFSM = NOISY;
			iFSMCounter = 0;
			cout << "Switching to Noisy State" << endl;
			alerted = false;
		}
		else if (iFSMCounter > iEnRouteMaxFSMCounter) //been in en_route for too long with no changes
		{
			sCurrentFSM = RETURN;
			iFSMCounter = 0;
			cout << "Switching to Return State" << endl;
			alerted = false;
		}
		else
		{
			glm::vec2 startIndices;
			if (vec2NumMicroSteps.x == 0)
			{
				startIndices = glm::vec2(vec2Index.x, vec2Index.y);
			}
			else
			{
				startIndices = glm::vec2(vec2Index.x + 1, vec2Index.y);
			}

			auto path = cMap2D->PathFind(startIndices,
				cPlayer2D->vec2Index,
				heuristic::manhattan,
				10);

			// Calculate new destination
			bool bFirstPosition = true;
			for (const auto& coord : path)
			{
				//std::cout << coord.x << "," << coord.y << "\n";
				if (bFirstPosition == true)
				{
					// Set a destination
					vec2Destination = coord;
					// Calculate the direction between enemy2D and this destination
					vec2Direction = vec2Destination - vec2Index;
					bFirstPosition = false;
				}
				else
				{
					if ((coord - vec2Destination) == vec2Direction)
					{
						// Set a destination
						vec2Destination = coord;
					}
					else
					{
						break;
					}
				}
			}

			// Update the Enemy2D's position for attack
			UpdatePosition();
		}
		iFSMCounter++;
		break;
	case RETURN:
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 4.0f) //near player
		{
			sCurrentFSM = NOISY;
			iFSMCounter = 0;
			cout << "Switching to Noisy State" << endl;
			break;
		}
		if (alerted) //if near a noisy patrol team enemy
		{
			sCurrentFSM = EN_ROUTE;
			iFSMCounter = 0;
			cout << "Switching to En Route State" << endl;
			break;
		}
		//if too close to the current waypoint
		if (cPhysics2D.CalculateDistance(vec2Index, waypoints[currentWaypointCounter]) < 0.5f)
		{
			sCurrentFSM = IDLE;
			iFSMCounter = 0;
			cout << "Switching to Idle State" << endl;
		}
		else //move back to waypoint
		{
			glm::vec2 startIndices;
			if (vec2NumMicroSteps.x == 0)
			{
				startIndices = glm::vec2(vec2Index.x, vec2Index.y);
			}
			else
			{
				startIndices = glm::vec2(vec2Index.x + 1, vec2Index.y);
			}

			auto path = cMap2D->PathFind(startIndices,	// start pos
				waypoints[currentWaypointCounter],		// target pos
				heuristic::manhattan,					// heuristic
				10);									// weight

			// Calculate new destination
			bool bFirstPosition = true;
			for (const auto& coord : path)
			{
				//std::cout << coord.x << ", " << coord.y << "\n";
				if (bFirstPosition == true)
				{
					// Set a destination
					vec2Destination = coord;

					// Calculate the direction between enemy2D and this destination
					vec2Direction = vec2Destination - vec2Index;
					bFirstPosition = false;
				}
				else
				{
					if ((coord - vec2Destination) == vec2Direction)
					{
						// Set a destination
						vec2Destination = coord;
					}
					else
					{
						break;
					}
				}
			}

			UpdatePosition();
		}
		iFSMCounter++;
		break;
	default:
		break;
	}

	UpdateDirection();

	// Check if enemy is in mid-air, such as walking off a platform
	if (IsMidAir())
	{
		if (cPhysics2D.GetStatus() != CPhysics2D::STATUS::JUMP)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}
	}
	// Update Jump or Fall
	UpdateJumpFall(dElapsedTime);

	// Interact with the Map
	InteractWithMap();

	//CS: Update the animated sprite
	//CS: Play the "left" animation
	animatedSprites->Update(dElapsedTime);

	// Update the UV Coordinates
	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false, i32vec2NumMicroSteps.x*cSettings->ENEMY_MICRO_STEP_XAXIS);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false, i32vec2NumMicroSteps.y*cSettings->ENEMY_MICRO_STEP_YAXIS);
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void JEnemy2DPatrolT::PreRender(void)
{
	if (!bIsActive)
		return;

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);

	// Activate blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate the shader
	CShaderManager::GetInstance()->Use(sShaderName);
}

/**
 @brief Render this instance
 */
void JEnemy2DPatrolT::Render(void)
{
	if (!bIsActive)
		return;

	glBindVertexArray(VAO);
	// get matrix's uniform location and set matrix
	unsigned int transformLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "transform");
	unsigned int colorLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "runtimeColour");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	glm::vec2 offset = glm::i32vec2(float(cSettings->NUM_TILES_XAXIS / 2.0f), float(cSettings->NUM_TILES_YAXIS / 2.0f));
	glm::vec2 cameraPos = camera2D->getPos();

	glm::vec2 IndexPos = vec2Index;

	glm::vec2 actualPos = IndexPos - cameraPos + offset;
	actualPos = cSettings->ConvertIndexToUVSpace(actualPos) * camera2D->getZoom();
	actualPos.x += i32vec2NumMicroSteps.x * cSettings->MICRO_STEP_XAXIS;
	actualPos.y += i32vec2NumMicroSteps.y * cSettings->MICRO_STEP_YAXIS;

	transform = glm::translate(transform, glm::vec3(actualPos.x, actualPos.y, 0.f));
	transform = glm::scale(transform, glm::vec3(camera2D->getZoom()));


	// Update the shaders with the latest transform
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glUniform4fv(colorLoc, 1, glm::value_ptr(runtimeColour));

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	// Get the texture to be rendered
	glBindTexture(GL_TEXTURE_2D, iTextureID);

	//CS: Render the animated Sprite
	animatedSprites->Render();

	glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void JEnemy2DPatrolT::PostRender(void)
{
	if (!bIsActive)
		return;

	// Disable blending
	glDisable(GL_BLEND);
}

//check if need to check to put other enemies in alert state
bool JEnemy2DPatrolT::getNoisy(void)
{
	return noisy;
}

//set alert to true to get specific enemy to go into en_route state
void JEnemy2DPatrolT::setAlert(bool alert)
{
	alerted = alert;
}

/**
@brief Set the indices of the enemy2D
@param iIndex_XAxis A const int variable which stores the index in the x-axis
@param iIndex_YAxis A const int variable which stores the index in the y-axis
*/
void JEnemy2DPatrolT::Seti32vec2Index(const int iIndex_XAxis, const int iIndex_YAxis)
{
	this->vec2Index.x = iIndex_XAxis;
	this->vec2Index.y = iIndex_YAxis;
}

/**
@brief Set the number of microsteps of the enemy2D
@param iNumMicroSteps_XAxis A const int variable storing the current microsteps in the X-axis
@param iNumMicroSteps_YAxis A const int variable storing the current microsteps in the Y-axis
*/
void JEnemy2DPatrolT::Seti32vec2NumMicroSteps(const int iNumMicroSteps_XAxis, const int iNumMicroSteps_YAxis)
{
	this->i32vec2NumMicroSteps.x = iNumMicroSteps_XAxis;
	this->i32vec2NumMicroSteps.y = iNumMicroSteps_YAxis;
}

/**
 @brief Set the handle to cPlayer to this class instance
 @param cPlayer2D A CPlayer2D* variable which contains the pointer to the CPlayer2D instance
 */
void JEnemy2DPatrolT::SetPlayer2D(CPlayer2D* cPlayer2D)
{
	this->cPlayer2D = cPlayer2D;

	// Update the enemy's direction
	UpdateDirection();
}

/**
 @brief Constraint the enemy2D's position within a boundary
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
void JEnemy2DPatrolT::Constraint(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		if (vec2Index.x < 0)
		{
			vec2Index.x = 0;
			i32vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == RIGHT)
	{
		if (vec2Index.x >= (int)cSettings->NUM_TILES_XAXIS - 1)
		{
			vec2Index.x = ((int)cSettings->NUM_TILES_XAXIS) - 1;
			i32vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == UP)
	{
		if (vec2Index.y >= (int)cSettings->NUM_TILES_YAXIS - 1)
		{
			vec2Index.y = ((int)cSettings->NUM_TILES_YAXIS) - 1;
			i32vec2NumMicroSteps.y = 0;
		}
	}
	else if (eDirection == DOWN)
	{
		if (vec2Index.y < 0)
		{
			vec2Index.y = 0;
			i32vec2NumMicroSteps.y = 0;
		}
	}
	else
	{
		cout << "CStnEnemy2D::Constraint: Unknown direction." << endl;
	}
}

/**
 @brief Check if a position is possible to move into
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
bool JEnemy2DPatrolT::CheckPosition(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		// If the new position is fully within a row, then check this row only
		if (i32vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 600)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 600) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 600))
			{
				return false;
			}
		}
	}
	else if (eDirection == RIGHT)
	{
		// If the new position is at the top row, then return true
		if (vec2Index.x >= cSettings->NUM_TILES_XAXIS - 1)
		{
			i32vec2NumMicroSteps.x = 0;
			return true;
		}

		// If the new position is fully within a row, then check this row only
		if (i32vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 600)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 600) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 600))
			{
				return false;
			}
		}

	}
	else if (eDirection == UP)
	{
		// If the new position is at the top row, then return true
		if (vec2Index.y >= cSettings->NUM_TILES_YAXIS - 1)
		{
			i32vec2NumMicroSteps.y = 0;
			return true;
		}

		// If the new position is fully within a column, then check this column only
		if (i32vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 600)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (i32vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 600) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 600))
			{
				return false;
			}
		}
	}
	else if (eDirection == DOWN)
	{
		// If the new position is fully within a column, then check this column only
		if (i32vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 600)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (i32vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 600) ||
				(cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 600))
			{
				return false;
			}
		}
	}
	else
	{
		cout << "CStnEnemy2D::CheckPosition: Unknown direction." << endl;
	}

	return true;
}

// Check if the enemy2D is in mid-air
bool JEnemy2DPatrolT::IsMidAir(void)
{
	// if the enemy is at the bottom row, then he is not in mid-air for sure
	if (vec2Index.y == 0)
	{
		return false;
	}

	// Check if the tile below the enemy's current position is empty
	if (vec2NumMicroSteps.x == 0 &&
		(cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) < 600))
	{
		return true;
	}

	//if enemy is standing between 2 tiles which are both not obstruction blocks
	if ((cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) < 600) && (cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x + 1) < 600))
	{
		return true;
	}

	return false;
}

// Update Jump or Fall
void JEnemy2DPatrolT::UpdateJumpFall(const double dElapsedTime)
{
	if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::JUMP)
	{
		// Update the elapsed time to the physics engine
		cPhysics2D.SetTime((float)dElapsedTime);
		// Call the physics engine update method to calculate the final velocity and displacement
		cPhysics2D.Update();
		// Get the displacement from the physics engine
		glm::vec2 v2Displacement = cPhysics2D.GetDisplacement();

		// Store the current vec2Index.y
		int iIndex_YAxis_OLD = vec2Index.y;

		int iDisplacement_MicroSteps = (int)(v2Displacement.y / cSettings->ENEMY_MICRO_STEP_YAXIS); //DIsplacement divide by distance for 1 microstep
		if (vec2Index.y < (int)cSettings->NUM_TILES_YAXIS)
		{
			i32vec2NumMicroSteps.y += iDisplacement_MicroSteps;
			if (i32vec2NumMicroSteps.y > cSettings->ENEMY_NUM_STEPS_PER_TILE_YAXIS)
			{
				i32vec2NumMicroSteps.y -= cSettings->ENEMY_NUM_STEPS_PER_TILE_YAXIS;
				if (i32vec2NumMicroSteps.y < 0)
					i32vec2NumMicroSteps.y = 0;
				vec2Index.y++;
			}
		}

		// Constraint the player's position within the screen boundary
		Constraint(UP);

		// Iterate through all rows until the proposed row
		// Check if the player will hit a tile; stop jump if so.
		int iIndex_YAxis_Proposed = vec2Index.y;
		for (int i = iIndex_YAxis_OLD; i <= iIndex_YAxis_Proposed; i++)
		{
			// Change the player's index to the current i value
			vec2Index.y = i;
			// If the new position is not feasible, then revert to old position
			if (CheckPosition(UP) == false)
			{
				// Align with the row
				i32vec2NumMicroSteps.y = 0;
				// Set the Physics to fall status
				cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
				break;
			}
		}

		// If the player is still jumping and the initial velocity has reached zero or below zero, 
		// then it has reach the peak of its jump
		if ((cPhysics2D.GetStatus() == CPhysics2D::STATUS::JUMP) && (cPhysics2D.GetInitialVelocity().y <= 0.0f))
		{
			// Set status to fall
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}
	}
	else if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::FALL)
	{
		// Update the elapsed time to the physics engine
		cPhysics2D.SetTime((float)dElapsedTime);
		// Call the physics engine update method to calculate the final velocity and displacement
		cPhysics2D.Update();
		// Get the displacement from the physics engine
		glm::vec2 v2Displacement = cPhysics2D.GetDisplacement();

		// Store the current vec2Index.y
		int iIndex_YAxis_OLD = vec2Index.y;

		// Translate the displacement from pixels to indices
		int iDisplacement_MicroSteps = (int)(v2Displacement.y / cSettings->ENEMY_MICRO_STEP_YAXIS);

		if (vec2Index.y >= 0)
		{
			i32vec2NumMicroSteps.y -= fabs(iDisplacement_MicroSteps);
			if (i32vec2NumMicroSteps.y < 0)
			{
				i32vec2NumMicroSteps.y = ((int)cSettings->ENEMY_NUM_STEPS_PER_TILE_YAXIS) - 1;
				vec2Index.y--;
			}
		}

		// Constraint the player's position within the screen boundary
		Constraint(DOWN);

		// Iterate through all rows until the proposed row
		// Check if the player will hit a tile; stop fall if so.
		int iIndex_YAxis_Proposed = vec2Index.y;
		for (int i = iIndex_YAxis_OLD; i >= iIndex_YAxis_Proposed; i--)
		{
			// Change the player's index to the current i value
			vec2Index.y = i;
			// If the new position is not feasible, then revert to old position
			if (CheckPosition(DOWN) == false)
			{
				// Revert to the previous position
				if (i != iIndex_YAxis_OLD)
					vec2Index.y = i + 1;
				// Set the Physics to idle status
				cPhysics2D.SetStatus(CPhysics2D::STATUS::IDLE);
				cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::ENEMY_LAND); //play enemy landing sound
				i32vec2NumMicroSteps.y = 0;
				break;
			}
		}
	}
}

/**
 @brief Let enemy2D interact with the player.
 */
bool JEnemy2DPatrolT::InteractWithPlayer(void)
{
	glm::i32vec2 i32vec2PlayerPos = cPlayer2D->vec2Index;
	
	// Check if the enemy2D is within 1.5 indices of the player2D
	if (((vec2Index.x >= i32vec2PlayerPos.x - 0.5) && 
		(vec2Index.x <= i32vec2PlayerPos.x + 0.5))
		&& 
		((vec2Index.y >= i32vec2PlayerPos.y - 0.5) &&
		(vec2Index.y <= i32vec2PlayerPos.y + 0.5)))
	{
		//cout << "Patrol Team Gotcha!" << endl;
		if (attackCooldownCurrent == 0)
		{
			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::ENEMY_MELEE); //play melee attack noise

			// Decrease the health by 1
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Health");
			cInventoryItemPlanet->Remove(5);
			//cout << "Take that!" << endl;
			attackCooldownCurrent = attackCooldownMax; //reset attack cooldown
		}
		return true;
	}
	return false;
}

//enemy interact with map
void JEnemy2DPatrolT::InteractWithMap(void)
{
	switch (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x))
	{
	case 99:
		break;
	default:
		break;
	}
}

/**
 @brief Update the enemy's direction.
 */
void JEnemy2DPatrolT::UpdateDirection(void)
{
	// Set the destination to the player
	vec2Destination = cPlayer2D->vec2Index;

	// Calculate the direction between enemy2D and player2D
	vec2Direction = vec2Destination - vec2Index;

	// Calculate the distance between enemy2D and player2D
	float fDistance = cPhysics2D.CalculateDistance(vec2Index, vec2Destination);
	if (fDistance >= 0.01f)
	{
		// Calculate direction vector.
		// We need to round the numbers as it is easier to work with whole numbers for movements
		vec2Direction.x = (int)round(vec2Direction.x / fDistance);
		vec2Direction.y = (int)round(vec2Direction.y / fDistance);
	}
	else
	{
		// Since we are not going anywhere, set this to 0.
		vec2Direction = glm::i32vec2(0);
	}
}

/**
 @brief Flip horizontal direction. For patrol use only
 */
void JEnemy2DPatrolT::FlipHorizontalDirection(void)
{
	vec2Direction.x *= -1;
}

/**
@brief Update position.
*/
void JEnemy2DPatrolT::UpdatePosition(void)
{
	// Store the old position
	i32vec2OldIndex = vec2Index;

	// if the player is to the left or right of the enemy2D, then jump to attack
	if (vec2Direction.x < 0)
	{
		// Move left
		const int iOldIndex = vec2Index.x;
		if (vec2Index.x >= 0)
		{
			i32vec2NumMicroSteps.x--;
			if (i32vec2NumMicroSteps.x < 0)
			{
				i32vec2NumMicroSteps.x = ((int)cSettings->ENEMY_NUM_STEPS_PER_TILE_XAXIS) - 1;
				vec2Index.x--;
			}
		}
		shootingDirection = LEFT; //moving to the left
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::ENEMY_FOOTSTEPS); //play enemy footstep sound

		//set animation based on state
		if (sCurrentFSM == PATROL)
		{
			//Play the "moving left" animation
			animatedSprites->PlayAnimation("movingL", -1, 0.5f);
		}
		else if (sCurrentFSM == NOISY)
		{
			//Play the "noisy left" animation
			animatedSprites->PlayAnimation("noisyL", -1, 0.75f);
		}
		else if (sCurrentFSM == EN_ROUTE)
		{
			//Play the "en route left" animation
			animatedSprites->PlayAnimation("enrouteL", -1, 0.5f);
		}

		// Constraint the enemy2D's position within the screen boundary
		Constraint(LEFT);

		// Find a feasible position for the enemy2D's current position
		if (CheckPosition(LEFT) == false)
		{
			FlipHorizontalDirection();
			vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.x = 0;
		}

		// Check if enemy2D is in mid-air, such as walking off a platform
		if (IsMidAir() == true)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}

		// Interact with the Player
		InteractWithPlayer();
	}
	else if (vec2Direction.x > 0)
	{
		// Move right
		const int iOldIndex = vec2Index.x;
		if (vec2Index.x < (int)cSettings->NUM_TILES_XAXIS)
		{
			i32vec2NumMicroSteps.x++;

			if (i32vec2NumMicroSteps.x >= cSettings->ENEMY_NUM_STEPS_PER_TILE_XAXIS)
			{
				i32vec2NumMicroSteps.x = 0;
				vec2Index.x++;
			}
		}
		shootingDirection = RIGHT; //moving to the right
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::ENEMY_FOOTSTEPS); //play enemy footstep sound

		//set animation based on state
		if (sCurrentFSM == PATROL)
		{
			//Play the "moving right" animation
			animatedSprites->PlayAnimation("movingR", -1, 0.5f);
		}
		else if (sCurrentFSM == NOISY)
		{
			//Play the "noisy right" animation
			animatedSprites->PlayAnimation("noisyR", -1, 0.75f);
		}
		else if (sCurrentFSM == EN_ROUTE)
		{
			//Play the "en route right" animation
			animatedSprites->PlayAnimation("enrouteR", -1, 0.5f);
		}

		// Constraint the enemy2D's position within the screen boundary
		Constraint(RIGHT);

		// Find a feasible position for the enemy2D's current position
		if (CheckPosition(RIGHT) == false)
		{
			FlipHorizontalDirection();
			//vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.x = 0;
		}

		// Check if enemy2D is in mid-air, such as walking off a platform
		if (IsMidAir() == true)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
			shootingDirection = DOWN; //moving down
		}

		// Interact with the Player
		InteractWithPlayer();
	}

	// if the player is above the enemy2D, then jump to attack
	if (vec2Direction.y > 0)
	{
		if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::IDLE)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::JUMP);
			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::ENEMY_JUMP); //play enemy jump sound
			cPhysics2D.SetInitialVelocity(glm::vec2(0.0f, 3.5f));
		}
	}
}

vector<glm::vec2> JEnemy2DPatrolT::ConstructWaypointVector(vector<glm::vec2> waypointVector, int startIndex, int numOfWaypoints)
{
	for (int i = 0; i < numOfWaypoints; ++i)
	{
		waypointVector.push_back(cMap2D->GetTilePosition(startIndex + i));
		// Erase the value of the waypoint in the arrMapInfo
		cMap2D->SetMapInfo(waypointVector[i].y, waypointVector[i].x, 0);
	}

	return waypointVector;
}

//called whenever an ammo is needed to be shot
CJEAmmo* JEnemy2DPatrolT::FetchAmmo()
{
	//Exercise 3a: Fetch a game object from m_goList and return it
	for (std::vector<CJEAmmo*>::iterator it = ammoList.begin(); it != ammoList.end(); ++it)
	{
		CJEAmmo* ammo = (CJEAmmo*)*it;
		if (ammo->getActive()) {
			continue;
		}
		ammo->setActive(true);
		// By default, microsteps should be zero --> reset in case a previously active ammo that was used then ste inactive was used again
		ammo->vec2NumMicroSteps = glm::i32vec2(0, 0);
		return ammo;
	}

	//whenever ammoList runs out of ammo, create 10 ammo to use
	//Get Size before adding 10
	int prevSize = ammoList.size();
	for (int i = 0; i < 10; ++i) {
		ammoList.push_back(new CJEAmmo);
	}
	ammoList.at(prevSize)->setActive(true);
	return ammoList.at(prevSize);

}