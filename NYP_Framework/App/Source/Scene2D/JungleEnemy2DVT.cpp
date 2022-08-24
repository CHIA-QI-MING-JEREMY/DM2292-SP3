/**
 CStnEnemy2D
 @brief A class which represents the enemy object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "JungleEnemy2DVT.h"

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
JEnemy2DVT::JEnemy2DVT(void)
	: bIsActive(false)
	, cMap2D(NULL)
	, cSettings(NULL)
	, cPlayer2D(NULL)
	, sCurrentFSM(FSM::TELEPORT)
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
JEnemy2DVT::~JEnemy2DVT(void)
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
bool JEnemy2DVT::Init(void)
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
	if (cMap2D->FindValue(1600, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the enemy, so quit this game

	// Erase the value of the player in the arrMapInfo
	cMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	i32vec2NumMicroSteps = glm::i32vec2(0, 0);

	// Create and initialise the CPlayer2D
	cPlayer2D = CPlayer2D::GetInstance();

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//CS: Create the Quad Mesh using the mesh builder
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

	// Load the enemy2D texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/JunglePlanet/VigilantTeleporterSpriteSheet.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/JunglePlanet/VigilantTeleporterSpriteSheet.png" << endl;
		return false;
	}

	//CS: Create the animated spirte and setup the animation
	animatedSprites = CMeshBuilder::GenerateSpriteAnimation(5, 4,
		cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	//^ loads a spirte sheet with 3 by 3 diff images, all of equal size and positioning
	animatedSprites->AddAnimation("idle", 16, 17); //recovering (idle), 1.f speed
	animatedSprites->AddAnimation("movingR", 0, 4); //5 images for animation, index 0 to 4, walk right, 0.2f speed
	animatedSprites->AddAnimation("movingL", 5, 9); //walk left, 0.2f speed
	animatedSprites->AddAnimation("attackingR", 10, 12); //attack right, 0.75f speed
	animatedSprites->AddAnimation("attackingL", 13, 15); //attack left, 0.75f speed

	//CS: Play the "idle" animation as default
	animatedSprites->PlayAnimation("idle", -1, 1.f);
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
		CJEAmmoVT* cEnemyAmmo2D = new CJEAmmoVT();
		cEnemyAmmo2D->SetShader("Shader2D");
		ammoList.push_back(cEnemyAmmo2D);
	}

	//for tutorial lvl
	if (cMap2D->GetCurrentLevel() == 0)
	{
		//if it's the enemy at this position
		if (vec2Index == glm::vec2(30, 8))
		{
			waypoints = ConstructWaypointVector(waypoints, 100, 2);
		}
	}
	//for lvl 1
	if (cMap2D->GetCurrentLevel() == 1)
	{
		//if it's the enemy at this position
		if (vec2Index == glm::vec2(4, 4))
		{
			waypoints = ConstructWaypointVector(waypoints, 100, 4);
		}
	}
	//for lvl 2
	if (cMap2D->GetCurrentLevel() == 2)
	{
		//if it's the enemy at this position
		if (vec2Index == glm::vec2(4, 4))
		{
			waypoints = ConstructWaypointVector(waypoints, 100, 4);
		}
	}
	
	// sets waypoint counter value
	currentWaypointCounter = 0;
	maxWaypointCounter = waypoints.size();

	type = TELEPORTABLE; //has teleportation residue
	shootingDirection = LEFT; //setting direction for ammo shooting
	maxHealth = health = 50; //takes 10 hits to kill

	flickerTimer = 0.5; //used to progress the flicker counter
	flickerTimerMax = 0.5; //used to reset flicker counter
	flickerCounter = 0; //decides colour of enemy and when to explode

	//make sure both vectors start off empty
	enemysTeleportationResidue.clear(); //a vector of locations where this enemy left behind teleportation residue
	enemysTResidueCooldown.clear(); //timer for how long the residue will last

	healingCooldown = 0.0; //timer between when the enemy heals when in new location

	return true;
}

/**
 @brief Update this instance
 */
void JEnemy2DVT::Update(const double dElapsedTime)
{
	if (!bIsActive)
		return;

	//prevent health from going over maximum
	if (health > maxHealth)
	{
		health = maxHealth;
	}

	//if dead, remove all teleportation residue
	if (health <= 0)
	{
		for (int i = 0; i < enemysTeleportationResidue.size(); ++i)
		{
			//remove poof effect
			cMap2D->SetMapInfo(enemysTeleportationResidue[i].y, enemysTeleportationResidue[i].x, 0);
		}

		enemysTeleportationResidue.clear();
		enemysTResidueCooldown.clear();
		return;
	}

	//check if there are any existing teleporattion residue left behind by this enemy
		//deplete their cooldowns if so
	for (int i = 0; i < enemysTeleportationResidue.size(); ++i)
	{
		enemysTResidueCooldown[i] -= dElapsedTime; //deplete specific cooldown

		if (enemysTResidueCooldown[i] <= 0.0) //if cooldown is up
		{
			//remove poof effect
			cMap2D->SetMapInfo(enemysTeleportationResidue[i].y, enemysTeleportationResidue[i].x, 0);

			//remove the removed teleportation residue's location from the vector
			enemysTeleportationResidue.erase(enemysTeleportationResidue.begin() + i); 
			//as well as its cooldown timer
			enemysTResidueCooldown.erase(enemysTResidueCooldown.begin() + i);
		}
	}

	// just for switching between states --> keep simple
	//action done under interaction with player, update position, update direction, etc
	switch (sCurrentFSM)
	{
	case TELEPORT:
		++currentWaypointCounter; //increase the enemy's current waypoint count
			//if pointing at the spot beyond the last waypoint index, set current back to first
		if (currentWaypointCounter == maxWaypointCounter)
		{
			currentWaypointCounter = 0;
		}

		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, CMap2D::TILE_INDEX::TELEPORTATION_RESIDUE); //leave behind a "poof" effect before teleporting
		enemysTeleportationResidue.push_back(vec2Index); //push the poof's location into vector of poof locations
		enemysTResidueCooldown.push_back(enemysTResidueMaxCooldown); //starts its own cooldown

		vec2Index = waypoints[currentWaypointCounter]; //teleport the enemy to the way point
		if (health <= 15) //if health is low, switch to recover
		{
			sCurrentFSM = RECOVER;
			iFSMCounter = 0;
			cout << "Switching to Recover State" << endl;
		}
		else
		{
			sCurrentFSM = WANDER; //switch to wander
			iFSMCounter = 0;
			cout << "Switching to Wander State" << endl;
		}
		iFSMCounter++;
		break;
	case WANDER:
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 8.0f &&
			(vec2Index.y == cPlayer2D->vec2Index.y || // player is left or right of the enemy
				vec2Index.x == cPlayer2D->vec2Index.x)) // player is above or below the enemy
		{
			bool pathClear = true; //only set to false if there is an impassable tile
			if (vec2Index.y == cPlayer2D->vec2Index.y) // player is left or right of the enemy
				//if player is on same position as enemy, uses this checking instead of the one below
			{
				// check if player's x is larger than or smaller than enemy's x
				// if is larger, direction is right
				// if is smaller, direction is left
				if (cPlayer2D->vec2Index.x > vec2Index.x)
				{
					//check if the path to the right is clear
					for (int i = 0; i <= (cPlayer2D->vec2Index.x - vec2Index.x); ++i)
					{
						//if the tile is impassable : 610 on
						if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + i) >= 610) //tile isnt a destoryable block
						{
							pathClear = false;
						}
					}
				}
				else
				{
					//check if the path to the left is clear
					for (int i = 0; i <= (vec2Index.x - cPlayer2D->vec2Index.x); ++i)
					{
						//if the tile is impassable : 610 on
						if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x - i) >= 610) //tile isnt a destoryable block
						{
							pathClear = false;
						}
					}
				}
			}
			else if (vec2Index.x == cPlayer2D->vec2Index.x ||
				vec2Index.x - 1 == cPlayer2D->vec2Index.x ||
				vec2Index.x + 1 == cPlayer2D->vec2Index.x) // player is above or below the enemy, with a small margin of error x wise
			{
				// check if player's y is larger than or smaller than enemy's y
				// if is larger, direction is up
				// if is smaller, direction is down
				if (cPlayer2D->vec2Index.y > vec2Index.y)
				{
					//check if the path upward is clear
					for (int i = 0; i <= (cPlayer2D->vec2Index.y - vec2Index.y); ++i)
					{
						//if the tile is impassable : 610 on
						if (cMap2D->GetMapInfo(vec2Index.y + i, vec2Index.x) >= 610) //tile isnt a destoryable block
						{
							pathClear = false;
						}
					}
				}
				else
				{
					//check if the path down is clear
					for (int i = 0; i <= (vec2Index.y - cPlayer2D->vec2Index.y); ++i)
					{
						//if the tile is impassable : 610 on
						if (cMap2D->GetMapInfo(vec2Index.y - i, vec2Index.x) >= 610) //tile isnt a destoryable block
						{
							pathClear = false;
						}
					}
				}
			}

			//only change into SHOOT state if path is clear
			if (pathClear)
			{
				sCurrentFSM = SHOOT;
				iFSMCounter = 0;
				cout << "Switching to Shooting State" << endl;
				break;
			}

		}
		//if too far from the current waypoint
		if (cPhysics2D.CalculateDistance(vec2Index, waypoints[currentWaypointCounter]) > 1.5f)
		{
			sCurrentFSM = RETURN; //switch to return
			cout << "Switching to Return State" << endl;
			break;
		}
		UpdatePosition(); //move around a bit aimlessly
		iFSMCounter++;
		break;
	case SHOOT:
		if (health <= 5)
		{
			sCurrentFSM = RECOVER;
			iFSMCounter = 0;
			cout << "Switching to Recover State" << endl;
			break;
		}
		//if within shooting range
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 8.0f)
		{
			// Update direction to face and shoot in
			if (vec2Index.y == cPlayer2D->vec2Index.y) // player is left or right of the enemy
				//if player is on same position as enemy, uses this checking instead of the one below
			{
				// check if player's x is larger than or smaller than enemy's x
				// if is larger, direction is right
				// if is smaller, direction is left
				if (cPlayer2D->vec2Index.x > vec2Index.x)
				{
					//check if the path to the right is clear
					bool pathClear = true; //only set to false if there is an impassable tile
					for (int i = 0; i <= (cPlayer2D->vec2Index.x - vec2Index.x); ++i)
					{
						//if the tile is impassable : 610 on
						if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + i) >= 610) //tile isnt a destoryable block
						{
							pathClear = false;
						}
					}

					//only set to shoot if path is clear
					if (pathClear)
					{
						shootingDirection = RIGHT; //setting direction for ammo shooting
						//Play the "attacking right" animation
						animatedSprites->PlayAnimation("attackingR", -1, 0.75f);
					}
					else
					{
						sCurrentFSM = WANDER;
						cout << "Switching to Wander State" << endl;
						break;
					}

				}
				else
				{
					//check if the path to the left is clear
					bool pathClear = true; //only set to false if there is an impassable tile
					for (int i = 0; i <= (vec2Index.x - cPlayer2D->vec2Index.x); ++i)
					{
						//if the tile is impassable : 610 on
						if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x - i) >= 610) //tile isnt a destoryable block
						{
							pathClear = false;
						}
					}

					//only set to shoot if path is clear
					if (pathClear)
					{
						shootingDirection = LEFT; //setting direction for ammo shooting
						//Play the "attacking left" animation
						animatedSprites->PlayAnimation("attackingL", -1, 0.75f);
					}
					else
					{
						sCurrentFSM = WANDER;
						cout << "Switching to Wander State" << endl;
						break;
					}
				}

			}
			else if (vec2Index.x == cPlayer2D->vec2Index.x ||
				vec2Index.x - 1 == cPlayer2D->vec2Index.x ||
				vec2Index.x + 1 == cPlayer2D->vec2Index.x) // player is above or below the enemy, with a small margin of error x wise
			{
				// check if player's y is larger than or smaller than enemy's y
				// if is larger, direction is up
				// if is smaller, direction is down
				if (cPlayer2D->vec2Index.y > vec2Index.y)
				{
					//check if the path upward is clear
					bool pathClear = true; //only set to false if there is an impassable tile
					for (int i = 0; i <= (cPlayer2D->vec2Index.y - vec2Index.y); ++i)
					{
						//if the tile is impassable : 610 on
						if (cMap2D->GetMapInfo(vec2Index.y + i, vec2Index.x) >= 610) //tile isnt a destoryable block
						{
							pathClear = false;
						}
					}

					//only set to shoot if path is clear
					if (pathClear)
					{
						shootingDirection = UP; //setting direction for ammo shooting
						//Play the "attacking right" animation
						animatedSprites->PlayAnimation("attackingR", -1, 0.75f);
					}
					else
					{
						sCurrentFSM = WANDER;
						cout << "Switching to Wander State" << endl;
						break;
					}
				}
				else
				{
					//check if the path down is clear
					bool pathClear = true; //only set to false if there is an impassable tile
					for (int i = 0; i <= (vec2Index.y - cPlayer2D->vec2Index.y); ++i)
					{
						//if the tile is impassable : 610 on
						if (cMap2D->GetMapInfo(vec2Index.y - i, vec2Index.x) >= 610) //tile isnt a destoryable block
						{
							pathClear = false;
						}
					}

					//only set to shoot if path is clear
					if (pathClear)
					{
						shootingDirection = DOWN; //setting direction for ammo shooting
						//Play the "attacking right" animation
						animatedSprites->PlayAnimation("attackingR", -1, 0.75f);
					}
					else
					{
						sCurrentFSM = WANDER;
						cout << "Switching to Wander State" << endl;
						break;
					}
				}
			}

			// Shoot enemy ammo!
			//shoot ammo in accordance to the direction enemy is facing
			CJEAmmoVT* ammo = FetchAmmo();
			ammo->setActive(true);
			ammo->setPath(vec2Index.x, vec2Index.y, shootingDirection);
			cout << "Bam!" << shootingDirection << endl;

			sCurrentFSM = RELOAD;
			cout << "Switching to Reload State" << endl;
		}
		else
		{
			sCurrentFSM = WANDER;
			iFSMCounter = 0;
			cout << "Switching to Wander State" << endl;
		}
		break;
	case RELOAD:
		if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = SHOOT;
			iFSMCounter = 0;
			cout << "Switching to Attack State" << endl;
		}
		if (health <= 15)
		{
			sCurrentFSM = TELEPORT;
			iFSMCounter = 0;
			cout << "Switching to Teleport State" << endl;
		}
		iFSMCounter++;
		break;
	case RETURN:
		if (health <= 15) //if health is low, switch to teleport (teleport then recover)
		{
			sCurrentFSM = TELEPORT;
			iFSMCounter = 0;
			cout << "Switching to Teleport State" << endl;
		}
		//if too close to the current waypoint
		else if (cPhysics2D.CalculateDistance(vec2Index, waypoints[currentWaypointCounter]) < 0.5f)
		{
			if (iFSMCounter > iWanderReturnMaxFSMCounter) //after a while in wander/return mode
			{
				sCurrentFSM = TELEPORT; //switch to teleport
				iFSMCounter = 0;
				cout << "Switching to Teleport State" << endl;
				break;
			}
			else
			{
				sCurrentFSM = WANDER; //switch to wander
				cout << "Switching to Wander State" << endl;
			}
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
	case RECOVER:
		//Play the "idle" animation
		animatedSprites->PlayAnimation("idle", -1, 1.f);
		if (health >= maxHealth) //at full health
		{
			health = maxHealth; //max health

			sCurrentFSM = WANDER;
			iFSMCounter = 0;
			cout << "Switching to Wander State" << endl;
			runtimeColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); //reset to white
			break;
		}
		else if (health >= maxHealth / 2) //if at least 50% health, check if player is in shooting range
		{
			if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 8.0f &&
				(vec2Index.y == cPlayer2D->vec2Index.y || // player is left or right of the enemy
					vec2Index.x == cPlayer2D->vec2Index.x)) // player is above or below the enemy
			{
				bool pathClear = true; //only set to false if there is an impassable tile
				if (vec2Index.y == cPlayer2D->vec2Index.y) // player is left or right of the enemy
					//if player is on same position as enemy, uses this checking instead of the one below
				{
					// check if player's x is larger than or smaller than enemy's x
					// if is larger, direction is right
					// if is smaller, direction is left
					if (cPlayer2D->vec2Index.x > vec2Index.x)
					{
						//check if the path to the right is clear
						for (int i = 0; i <= (cPlayer2D->vec2Index.x - vec2Index.x); ++i)
						{
							//if the tile is impassable : 610 on
							if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + i) >= 610) //tile isnt a destoryable block
							{
								pathClear = false;
							}
						}
					}
					else
					{
						//check if the path to the left is clear
						for (int i = 0; i <= (vec2Index.x - cPlayer2D->vec2Index.x); ++i)
						{
							//if the tile is impassable : 610 on
							if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x - i) >= 610) //tile isnt a destoryable block
							{
								pathClear = false;
							}
						}
					}
				}
				else if (vec2Index.x == cPlayer2D->vec2Index.x ||
					vec2Index.x - 1 == cPlayer2D->vec2Index.x ||
					vec2Index.x + 1 == cPlayer2D->vec2Index.x) // player is above or below the enemy, with a small margin of error x wise
				{
					// check if player's y is larger than or smaller than enemy's y
					// if is larger, direction is up
					// if is smaller, direction is down
					if (cPlayer2D->vec2Index.y > vec2Index.y)
					{
						//check if the path upward is clear
						for (int i = 0; i <= (cPlayer2D->vec2Index.y - vec2Index.y); ++i)
						{
							//if the tile is impassable : 610 on
							if (cMap2D->GetMapInfo(vec2Index.y + i, vec2Index.x) >= 610) //tile isnt a destoryable block
							{
								pathClear = false;
							}
						}
					}
					else
					{
						//check if the path down is clear
						for (int i = 0; i <= (vec2Index.y - cPlayer2D->vec2Index.y); ++i)
						{
							//if the tile is impassable : 610 on
							if (cMap2D->GetMapInfo(vec2Index.y - i, vec2Index.x) >= 610) //tile isnt a destoryable block
							{
								pathClear = false;
							}
						}
					}
				}

				//only change into SHOOT state if path is clear
				if (pathClear)
				{
					sCurrentFSM = SHOOT;
					iFSMCounter = 0;
					cout << "Switching to Shooting State" << endl;
					runtimeColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); //reset to white
					break;
				}

			}
		}
		runtimeColour = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); //green colour to show healing
		if (healingCooldown <= 0.0) //cooldown up
		{
			++health; //heal
			healingCooldown = healingMaxCooldown; //reset healing cooldown
		}
		healingCooldown -= dElapsedTime; //deplete healing cooldown
		iFSMCounter++;
		break;
	default:
		break;
	}

	//ammo beahviour
	for (std::vector<CJEAmmoVT*>::iterator it = ammoList.begin(); it != ammoList.end(); ++it)
	{
		CJEAmmoVT* ammo = (CJEAmmoVT*)*it;
		if (ammo->getActive())
		{
			ammo->Update(dElapsedTime);
			if (ammo->LimitReached())
			{
				ammo->setActive(false);
			}
		}
	}

	UpdateDirection();

	// Update Jump or Fall
	UpdateJumpFall(dElapsedTime);

	// Interact with the Map
	InteractWithMap();

	////update sprite animation to play depending on the direction enemy is facing
	//if (shootingDirection == LEFT)
	//{
	//	//CS: Play the "left" animation
	//	animatedSprites->PlayAnimation("left", -1, 1.0f);
	//}
	//else if (shootingDirection == RIGHT)
	//{
	//	//CS: Play the "right" animation
	//	animatedSprites->PlayAnimation("right", -1, 1.0f);
	//}
	//else if (shootingDirection == UP)
	//{
	//	//CS: Play the "up" animation
	//	animatedSprites->PlayAnimation("up", -1, 1.0f);
	//}
	//else if (shootingDirection == DOWN)
	//{
	//	//CS: Play the "idle" animation
	//	animatedSprites->PlayAnimation("idle", -1, 1.0f);
	//}

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
void JEnemy2DVT::PreRender(void)
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
void JEnemy2DVT::Render(void)
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

	//render enemy ammo
	for (std::vector<CJEAmmoVT*>::iterator it = ammoList.begin(); it != ammoList.end(); ++it)
	{
		CJEAmmoVT* ammo = (CJEAmmoVT*)*it;
		if (ammo->getActive())
		{
			ammo->PreRender();
			ammo->Render();
			ammo->PostRender();
		}
	}

}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void JEnemy2DVT::PostRender(void)
{
	if (!bIsActive)
		return;

	// Disable blending
	glDisable(GL_BLEND);
}

/**
@brief Set the indices of the enemy2D
@param iIndex_XAxis A const int variable which stores the index in the x-axis
@param iIndex_YAxis A const int variable which stores the index in the y-axis
*/
void JEnemy2DVT::Seti32vec2Index(const int iIndex_XAxis, const int iIndex_YAxis)
{
	this->vec2Index.x = iIndex_XAxis;
	this->vec2Index.y = iIndex_YAxis;
}

/**
@brief Set the number of microsteps of the enemy2D
@param iNumMicroSteps_XAxis A const int variable storing the current microsteps in the X-axis
@param iNumMicroSteps_YAxis A const int variable storing the current microsteps in the Y-axis
*/
void JEnemy2DVT::Seti32vec2NumMicroSteps(const int iNumMicroSteps_XAxis, const int iNumMicroSteps_YAxis)
{
	this->i32vec2NumMicroSteps.x = iNumMicroSteps_XAxis;
	this->i32vec2NumMicroSteps.y = iNumMicroSteps_YAxis;
}

/**
 @brief Set the handle to cPlayer to this class instance
 @param cPlayer2D A CPlayer2D* variable which contains the pointer to the CPlayer2D instance
 */
void JEnemy2DVT::SetPlayer2D(CPlayer2D* cPlayer2D)
{
	this->cPlayer2D = cPlayer2D;

	// Update the enemy's direction
	UpdateDirection();
}

/**
 @brief Constraint the enemy2D's position within a boundary
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
void JEnemy2DVT::Constraint(DIRECTION eDirection)
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
bool JEnemy2DVT::CheckPosition(DIRECTION eDirection)
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
bool JEnemy2DVT::IsMidAir(void)
{
	// if the player is at the bottom row, then he is not in mid-air for sure
	if (vec2Index.y == 0)
		return false;

	// Check if the tile below the player's current position is empty
	if ((i32vec2NumMicroSteps.x == 0) &&
		(cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) == 0))
	{
		return true;
	}

	return false;
}

// Update Jump or Fall
void JEnemy2DVT::UpdateJumpFall(const double dElapsedTime)
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
				i32vec2NumMicroSteps.y = 0;
				break;
			}
		}
	}
}

/**
 @brief Let enemy2D interact with the player.
 */
bool JEnemy2DVT::InteractWithPlayer(void)
{
	glm::i32vec2 i32vec2PlayerPos = cPlayer2D->vec2Index;
	
	// Check if the enemy2D is within 1.5 indices of the player2D
	if (((vec2Index.x >= i32vec2PlayerPos.x - 0.5) && 
		(vec2Index.x <= i32vec2PlayerPos.x + 0.5))
		&& 
		((vec2Index.y >= i32vec2PlayerPos.y - 0.5) &&
		(vec2Index.y <= i32vec2PlayerPos.y + 0.5)))
	{
		
		return true;
	}
	return false;
}

//enemy interact with map
void JEnemy2DVT::InteractWithMap(void)
{
	switch (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x))
	{
	case 50: //purple spring, same function as the black and white spring 
	case 51: //black and white spring, launch the player repeatedly up into the sky at the rate of a double jump
		cPhysics2D.SetStatus(CPhysics2D::STATUS::JUMP);
		cPhysics2D.SetInitialVelocity(glm::vec2(0.0f, 4.f));
		break;
	case 53: //lava, same function as black and white lava
	case 54: //black and white lava, depletes health
		// Decrease the health by 2
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning noise
		health--;
		break;
	case 55: //ice water, same function as black and white ice water
	case 56: //black and white ice water, restores health
	case 57: //ice water + enemy waypoint
	case 58: //ice water + enemy waypoint, BnW
		// Increase the health by 2
		runtimeColour = glm::vec4(0.0, 1.0, 0.0, 1.0); //green
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::SPLASH); //play watching in water sound
		health++;
		break;
	case 99:
		break;
	default:
		break;
	}
}

/**
 @brief Update the enemy's direction.
 */
void JEnemy2DVT::UpdateDirection(void)
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
void JEnemy2DVT::FlipHorizontalDirection(void)
{
	vec2Direction.x *= -1;
}

/**
@brief Update position.
*/
void JEnemy2DVT::UpdatePosition(void)
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

		//Play the "moving left" animation
		animatedSprites->PlayAnimation("movingL", -1, 0.2f);

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

		//Play the "moving right" animation
		animatedSprites->PlayAnimation("movingR", -1, 0.2f);

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
			cPhysics2D.SetInitialVelocity(glm::vec2(0.0f, 3.5f));
		}
	}
}

vector<glm::vec2> JEnemy2DVT::ConstructWaypointVector(vector<glm::vec2> waypointVector, int startIndex, int numOfWaypoints)
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
CJEAmmoVT* JEnemy2DVT::FetchAmmo()
{
	//Exercise 3a: Fetch a game object from m_goList and return it
	for (std::vector<CJEAmmoVT*>::iterator it = ammoList.begin(); it != ammoList.end(); ++it)
	{
		CJEAmmoVT* ammo = (CJEAmmoVT*)*it;
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
		ammoList.push_back(new CJEAmmoVT);
	}
	ammoList.at(prevSize)->setActive(true);
	return ammoList.at(prevSize);

}