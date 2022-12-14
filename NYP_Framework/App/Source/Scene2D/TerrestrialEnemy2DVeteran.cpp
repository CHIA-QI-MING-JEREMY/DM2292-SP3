/**
 CStnEnemy2D
 @brief A class which represents the enemy object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "TerrestrialEnemy2DVeteran.h"

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
TEnemy2DVeteran::TEnemy2DVeteran(void)
	: bIsActive(false)
	, cMap2D(NULL)
	, cSettings(NULL)
	, cPlayer2D(NULL)
	, sCurrentFSM(FSM::IDLE)
	, iFSMCounter(0)
	, quadMesh(NULL)
	, camera2D(NULL)
	, animatedSprites(NULL)
{
	transform = glm::mat4(1.0f);	// make sure to initialize matrix to identity matrix first

	// Initialise vecIndex
	vec2Index = glm::vec2(0);

	// Initialise vecNumMicroSteps
	vec2NumMicroSteps = glm::vec2(0);

	// Initialise vec2UVCoordinate
	vec2UVCoordinate = glm::vec2(0.0f);

	vec2Destination = glm::vec2(0, 0);	// Initialise the iDestination
	vec2Direction = glm::vec2(0, 0);	// Initialise the iDirection
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
TEnemy2DVeteran::~TEnemy2DVeteran(void)
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

	if (cSoundController)
	{
		cSoundController->StopSoundByID(CSoundController::SOUND_LIST::VETERAN_RADIO); // stpp sound effect for the veteran radio
		cSoundController = NULL;
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

/**
  @brief Initialise this instance
  */
bool TEnemy2DVeteran::Init(void)
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
	if (cMap2D->FindValue(1801, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the enemy, so quit this game

	// Erase the value of the player in the arrMapInfo
	cMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	vec2NumMicroSteps = glm::vec2(0, 0);

	// Create and initialise the CPlayer2D
	cPlayer2D = CPlayer2D::GetInstance();

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//CS: Create the Quad Mesh using the mesh builder
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

	// Load the enemy texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/TerrestrialPlanet/VeteranSpriteSheet.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/TerrestrialPlanet/VeteranSpriteSheet.png" << endl;
		return false;
	}

	// Create the animated sprite and setup the animation
	animatedSprites = CMeshBuilder::GenerateSpriteAnimation(5, 6, cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	animatedSprites->AddAnimation("idleR", 0, 3);
	animatedSprites->AddAnimation("idleL", 6, 9);
	animatedSprites->AddAnimation("runR", 12, 17);
	animatedSprites->AddAnimation("runL", 18, 23);
	animatedSprites->AddAnimation("shootR", 24, 26);
	animatedSprites->AddAnimation("shootL", 27, 29);

	// Play idle animation as default
	animatedSprites->PlayAnimation("idleR", -1, 1.0f);

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
		CTEAmmoVeteran* cEnemyAmmo2D = new CTEAmmoVeteran();
		cEnemyAmmo2D->SetShader("Shader2D");
		ammoList.push_back(cEnemyAmmo2D);
	}

	// sets waypoints based on the level
	if (cMap2D->GetCurrentLevel() == 1)
	{
		if (vec2Index == glm::vec2(2, 4))
		{
			waypoints = ConstructWaypointVector(waypoints, 306, 5);
			repositionWaypoints = ConstructWaypointVector(repositionWaypoints, 311, 2);
			type = KEYHOLDER_PURPLE; // drops a purple key when killed
		}
		else if (vec2Index == glm::vec2(21, 4))
		{
			waypoints = ConstructWaypointVector(waypoints, 315, 1);
			repositionWaypoints = ConstructWaypointVector(repositionWaypoints, 316, 2);
			type = LONG_RANGE; // has ammo
		}
		else if (vec2Index == glm::vec2(26, 15))
		{
			waypoints = ConstructWaypointVector(waypoints, 318, 2);
			repositionWaypoints = ConstructWaypointVector(repositionWaypoints, 320, 1);
			type = KEYHOLDER_CYAN; // drops a cyan key when killed
		}
		else if (vec2Index == glm::vec2(24, 10))
		{
			waypoints = ConstructWaypointVector(waypoints, 321, 2);
			repositionWaypoints = ConstructWaypointVector(repositionWaypoints, 323, 2);
			type = LONG_RANGE; // has ammo
		}
	}
	else if (cMap2D->GetCurrentLevel() == 2)
	{
		if (vec2Index == glm::vec2(22, 17))
		{
			waypoints = ConstructWaypointVector(waypoints, 300, 4);
			repositionWaypoints = ConstructWaypointVector(repositionWaypoints, 304, 2);
			type = LONG_RANGE; // has ammo
		}
		else if (vec2Index == glm::vec2(20, 8))
		{
			waypoints = ConstructWaypointVector(waypoints, 308, 2);
			repositionWaypoints = ConstructWaypointVector(repositionWaypoints, 310, 2);
			type = KEYHOLDER_PURPLE; // drops a purple key when killed
		}
	}

	shootingDirection = RIGHT; //setting direction for ammo shooting
	maxHealth = health = 1000; // 100 damage per hit from player

	// sets waypoint counter value
	currentWaypointCounter = 0;
	maxWaypointCounter = waypoints.size();

	// alarm variables
	isAlarmerActive = false;
	isAlarmOn = false;

	// radio variables
	radioTimer = maxRadioTimer;

	// attack variables
	attackTimer = attackInterval;
	numFired = 0;

	// reposition variables
	currentRepositionCounter = 0;
	repositionWaypointDistance = -1.f;
	targetRepositionWaypoint = glm::vec2(NULL, NULL);

	return true;
}

/**
 @brief Update this instance
 */
void TEnemy2DVeteran::Update(const double dElapsedTime)
{
	if (!bIsActive)
		return;

	//prevent health from going over maximum
	if (health > maxHealth)
	{
		health = maxHealth;
	}

	// just for switching between states --> keep simple
	//action done under interaction with player, update position, update direction, etc
	switch (sCurrentFSM)
	{
	case IDLE:
	{
		if (isAlarmOn)
		{
			sCurrentFSM = ALERT_IDLE;
			iFSMCounter = 0;
			cout << "Switching to Alert_Idle State" << endl;
			break;
		}

		if (vec2Direction.x > 0)
		{
			animatedSprites->PlayAnimation("idleR", -1, 1.0f);
		}
		else if (vec2Direction.x < 0)
		{
			animatedSprites->PlayAnimation("idleL", -1, 1.0f);
		}

		if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = PATROL;
			iFSMCounter = 0;
			cout << "Switching to Patrol State" << endl;
			break;
		}
		iFSMCounter++;
		break;
	}
	case PATROL:
	{
		if (isAlarmOn)
		{
			sCurrentFSM = ALERT_IDLE;
			iFSMCounter = 0;
			cout << "Switching to Alert_Idle State" << endl;
			break;
		}

		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 5.f)
		{
			int randomState = rand() % 100;
			cout << randomState << endl;
			if (randomState < 50)
			{
				sCurrentFSM = ROCKET;
				iFSMCounter = 0;
				attackTimer = attackInterval;
				cout << "Switching to Rocket State" << endl;
				break;
			}
			else
			{			
				sCurrentFSM = REPOSITION;
				iFSMCounter = 0;
				cout << "Switching to Reposition State" << endl;
				break;
			}
		}
		else
		{
			if (vec2Index == waypoints[currentWaypointCounter])
			{
				currentWaypointCounter++;
				sCurrentFSM = IDLE;
				iFSMCounter = 0;
				cout << "Switching to Idle State" << endl;
			}

			if (vec2Direction.x > 0)
			{
				animatedSprites->PlayAnimation("runR", -1, 1.0f);
			}
			else if (vec2Direction.x < 0)
			{
				animatedSprites->PlayAnimation("runL", -1, 1.0f);
			}

			if (currentWaypointCounter < maxWaypointCounter)
			{
				glm::vec2 startPosition = vec2Index;
				/*if (vec2NumMicroSteps.x != 0)
				{
					startPosition.x += 1;
				}*/

				auto path = cMap2D->PathFind(startPosition,						// start pos
											waypoints[currentWaypointCounter],	// target pos
											heuristic::euclidean,				// heuristic
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
						vec2Direction = vec2Destination - startPosition;
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
			}
			else
			{
				currentWaypointCounter = 0;
			}
		}

		UpdatePosition();
		break;
	}
	case ROCKET:
	{
		if (vec2Direction.x > 0)
		{
			animatedSprites->PlayAnimation("shootR", 0, 1.f);

			shootingDirection = RIGHT;
		}
		else if (vec2Direction.x < 0)
		{
			animatedSprites->PlayAnimation("shootL", 0, 1.f);

			shootingDirection = LEFT;
		}

		if (attackTimer <= 0)
		{
			// Shoot enemy ammo!
			//shoot ammo in accordance to the direction enemy is facing
			CTEAmmoVeteran* ammo = FetchAmmo();
			ammo->setActive(true);
			ammo->setPath(vec2Index.x, vec2Index.y, shootingDirection);
			ammo->setIsAlerted(false);
			ammo->setStartingIndex(vec2Index);
			cout << "Bam!" << shootingDirection << endl;

			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::VETERAN_ROCKET_LAUNCH); // play sound effect for the veteran shooting

			attackTimer = attackInterval;
			numFired++;
		}
		else
		{
			attackTimer -= dElapsedTime;
		}

		if (numFired > attackMagSize)
		{
			sCurrentFSM = PATROL;
			iFSMCounter = 0;
			attackTimer = 0;
			numFired = 0;
			cout << "Switching to Patrol State" << endl;
			break;
		}

		UpdateDirection();

		break;
	}
	case REPOSITION:
	{		
		// cycles through each alarm box to find the nearest reposition waypoint
		for (int i = 0; i < repositionWaypoints.size(); ++i)
		{
			// sets target to first reposition waypoint
			if (repositionWaypointDistance < 0.f)
			{
				repositionWaypointDistance = cPhysics2D.CalculateDistance(vec2Index, repositionWaypoints[i]); // finds waypoint distance
				
				// checks if distance between enemy and player is less than distance between waypoint and player
				if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < cPhysics2D.CalculateDistance(repositionWaypoints[i], cPlayer2D->vec2Index))
				{
					targetRepositionWaypoint = repositionWaypoints[i]; // sets target if the enemy moves away from the player to get to the waypoint
				}
			}
			// checks if new waypoint is closer than targeted waypoint, and if distance between enemy and player is less than distance between waypoint and player
			else if (cPhysics2D.CalculateDistance(vec2Index, repositionWaypoints[i]) < repositionWaypointDistance &&
				cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < cPhysics2D.CalculateDistance(repositionWaypoints[i], cPlayer2D->vec2Index))
			{
				targetRepositionWaypoint = repositionWaypoints[i]; // sets target if the enemy moves away from the player to get to the waypoint
				repositionWaypointDistance = cPhysics2D.CalculateDistance(vec2Index, repositionWaypoints[i]); // updates waypoint distance
			}
		}

		// checks if waypoint distance is valid and there is a targeted waypoint
		if (repositionWaypointDistance >= 0.f && targetRepositionWaypoint != glm::vec2(NULL, NULL))
		{
			glm::vec2 startPosition = vec2Index;
			/*if (vec2NumMicroSteps.x != 0)
			{
				startPosition.x += 1;
			}*/

			auto path = cMap2D->PathFind(startPosition,				// start pos
										targetRepositionWaypoint,	// target pos
										heuristic::euclidean,		// heuristic
										10);						// weight

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
					vec2Direction = vec2Destination - startPosition;
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
		else
		{
			sCurrentFSM = ROCKET;
			cout << "Switching to Rocket State" << endl;
			attackTimer = attackInterval;

			repositionWaypointDistance = -1.f;
			targetRepositionWaypoint = glm::vec2(NULL, NULL);
			radioTimer = maxRadioTimer;
			break;
		}

		if (vec2Index == targetRepositionWaypoint)
		{
			sCurrentFSM = RADIO;
			cout << "Switching to Radio State" << endl;

			repositionWaypointDistance = -1.f;
			targetRepositionWaypoint = glm::vec2(NULL, NULL);
			radioTimer = maxRadioTimer;
			break;
		}
		break;
	}
	case RADIO:
	{
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::VETERAN_RADIO); // play sound effect for the veteran radio
		
		if (vec2Direction.x > 0)
		{
			// Play the "idleR" animation
			animatedSprites->PlayAnimation("idleR", -1, 1.f);
		}
		else if (vec2Direction.x < 0)
		{
			// Play the "idleL" animation
			animatedSprites->PlayAnimation("idleL", -1, 1.f);
		}
		
		if (radioTimer <= 0.0 && !isAlarmOn)
		{
			cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::ALARM_LIGHT_OFF, CMap2D::TILE_INDEX::ALARM_LIGHT_ON);
			isAlarmOn = true;
		}
		else if (radioTimer > 0.0 && !isAlarmOn)
		{
			radioTimer -= dElapsedTime;
		}

		if (isAlarmOn)
		{
			isAlarmerActive = false;

			sCurrentFSM = ALERT_IDLE;
			cout << "Switching to Alert_Idle state" << endl;

			cSoundController->StopSoundByID(CSoundController::SOUND_LIST::VETERAN_RADIO); // stpp sound effect for the veteran radio

			break;
		}
		break;
	}
	case ALERT_IDLE:
	{
		if (!isAlarmOn)
		{
			sCurrentFSM = IDLE;
			iFSMCounter = 0;
			cout << "Switching to Idle State" << endl;
			break;
		}

		if (vec2Direction.x > 0)
		{
			animatedSprites->PlayAnimation("idleR", -1, 1.0f);
		}
		else if (vec2Direction.x < 0)
		{
			animatedSprites->PlayAnimation("idleL", -1, 1.0f);
		}

		if (iFSMCounter > iMaxAlertFSMCounter)
		{
			sCurrentFSM = ALERT_PATROL;
			iFSMCounter = 0;
			cout << "Switching to Alert_Patrol State" << endl;
			break;
		}
		iFSMCounter++;
		break;
	}
	case ALERT_PATROL:
	{
		if (!isAlarmOn)
		{
			sCurrentFSM = IDLE;
			iFSMCounter = 0;
			cout << "Switching to Idle State" << endl;
			break;
		}

		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 7.f)
		{
			sCurrentFSM = ALERT_ROCKET;
			iFSMCounter = 0;
			attackTimer = alertAttackInterval;
			cout << "Switching to Alert_Rocket State" << endl;
			break;
		}
		else
		{
			if (vec2Index == waypoints[currentWaypointCounter])
			{
				currentWaypointCounter++;
				sCurrentFSM = ALERT_IDLE;
				iFSMCounter = 0;
				cout << "Switching to Alert_Idle State" << endl;
			}

			if (vec2Direction.x > 0)
			{
				animatedSprites->PlayAnimation("runR", -1, 1.0f);
			}
			else if (vec2Direction.x < 0)
			{
				animatedSprites->PlayAnimation("runL", -1, 1.0f);
			}

			if (currentWaypointCounter < maxWaypointCounter)
			{
				glm::vec2 startPosition = vec2Index;
				/*if (vec2NumMicroSteps.x != 0)
				{
					startPosition.x += 1;
				}*/

				auto path = cMap2D->PathFind(startPosition,						// start pos
											waypoints[currentWaypointCounter],	// target pos
											heuristic::euclidean,				// heuristic
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
						vec2Direction = vec2Destination - startPosition;
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
			}
			else
			{
				currentWaypointCounter = 0;
			}
		}

		UpdatePosition();
		break;
	}
	case ALERT_ROCKET:
	{
		if (vec2Direction.x > 0)
		{
			animatedSprites->PlayAnimation("shootR", 0, 1.f);

			shootingDirection = RIGHT;
		}
		else if (vec2Direction.x < 0)
		{
			animatedSprites->PlayAnimation("shootL", 0, 1.f);

			shootingDirection = LEFT;
		}

		if (attackTimer <= 0)
		{
			// Shoot enemy ammo!
			//shoot ammo in accordance to the direction enemy is facing
			CTEAmmoVeteran* ammo = FetchAmmo();
			ammo->setActive(true);
			ammo->setPath(vec2Index.x, vec2Index.y, shootingDirection);
			ammo->setIsAlerted(true);
			ammo->setStartingIndex(vec2Index);
			cout << "Bam!" << shootingDirection << endl;

			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::VETERAN_ROCKET_LAUNCH); // play sound effect for the veteran shooting

			attackTimer = alertAttackInterval;
			numFired++;
		}
		else
		{
			attackTimer -= dElapsedTime;
		}

		if (numFired > alertAttackMagSize)
		{
			sCurrentFSM = ALERT_PATROL;
			iFSMCounter = 0;
			attackTimer = 0;
			numFired = 0;
			cout << "Switching to Alert_Patrol State" << endl;
			break;
		}

		UpdateDirection();

		break;
	}
	default:
		break;
	}

	//ammo beahviour
	for (std::vector<CTEAmmoVeteran*>::iterator it = ammoList.begin(); it != ammoList.end(); ++it)
	{
		CTEAmmoVeteran* ammo = (CTEAmmoVeteran*)*it;
		if (ammo->getActive())
		{
			ammo->Update(dElapsedTime);
			if (ammo->LimitReached())
			{
				ammo->setActive(false);
			}
		}
	}

	// Update Jump or Fall
	UpdateJumpFall(dElapsedTime);

	//CS: Update the animated sprite
	//CS: Play the "left" animation
	animatedSprites->Update(dElapsedTime);

	// Update the UV Coordinates
	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false, vec2NumMicroSteps.x*cSettings->ENEMY_MICRO_STEP_XAXIS);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false, vec2NumMicroSteps.y*cSettings->ENEMY_MICRO_STEP_YAXIS);
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void TEnemy2DVeteran::PreRender(void)
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
void TEnemy2DVeteran::Render(void)
{
	if (!bIsActive)
		return;

	glBindVertexArray(VAO);
	// get matrix's uniform location and set matrix
	unsigned int transformLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "transform");
	unsigned int colorLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "runtimeColour");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	glm::vec2 offset = glm::vec2(float(cSettings->NUM_TILES_XAXIS / 2.0f), float(cSettings->NUM_TILES_YAXIS / 2.0f));
	glm::vec2 cameraPos = camera2D->getPos();

	glm::vec2 IndexPos = vec2Index;

	glm::vec2 actualPos = IndexPos - cameraPos + offset;
	actualPos = cSettings->ConvertIndexToUVSpace(actualPos) * camera2D->getZoom();
	actualPos.x += vec2NumMicroSteps.x * cSettings->ENEMY_MICRO_STEP_XAXIS;
	actualPos.y += vec2NumMicroSteps.y * cSettings->ENEMY_MICRO_STEP_YAXIS;

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
	for (std::vector<CTEAmmoVeteran*>::iterator it = ammoList.begin(); it != ammoList.end(); ++it)
	{
		CTEAmmoVeteran* ammo = (CTEAmmoVeteran*)*it;
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
void TEnemy2DVeteran::PostRender(void)
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
void TEnemy2DVeteran::Setvec2Index(const int iIndex_XAxis, const int iIndex_YAxis)
{
	this->vec2Index.x = iIndex_XAxis;
	this->vec2Index.y = iIndex_YAxis;
}

/**
@brief Set the number of microsteps of the enemy2D
@param iNumMicroSteps_XAxis A const int variable storing the current microsteps in the X-axis
@param iNumMicroSteps_YAxis A const int variable storing the current microsteps in the Y-axis
*/
void TEnemy2DVeteran::Setvec2NumMicroSteps(const int iNumMicroSteps_XAxis, const int iNumMicroSteps_YAxis)
{
	this->vec2NumMicroSteps.x = iNumMicroSteps_XAxis;
	this->vec2NumMicroSteps.y = iNumMicroSteps_YAxis;
}

/**
 @brief Set the handle to cPlayer to this class instance
 @param cPlayer2D A CPlayer2D* variable which contains the pointer to the CPlayer2D instance
 */
void TEnemy2DVeteran::SetPlayer2D(CPlayer2D* cPlayer2D)
{
	this->cPlayer2D = cPlayer2D;

	// Update the enemy's direction
	UpdateDirection();
}

/**
 @brief Constraint the enemy2D's position within a boundary
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
void TEnemy2DVeteran::Constraint(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		if (vec2Index.x < 0)
		{
			vec2Index.x = 0;
			vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == RIGHT)
	{
		if (vec2Index.x >= (int)cSettings->NUM_TILES_XAXIS - 1)
		{
			vec2Index.x = ((int)cSettings->NUM_TILES_XAXIS) - 1;
			vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == UP)
	{
		if (vec2Index.y >= (int)cSettings->NUM_TILES_YAXIS - 1)
		{
			vec2Index.y = ((int)cSettings->NUM_TILES_YAXIS) - 1;
			vec2NumMicroSteps.y = 0;
		}
	}
	else if (eDirection == DOWN)
	{
		if (vec2Index.y < 0)
		{
			vec2Index.y = 0;
			vec2NumMicroSteps.y = 0;
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
bool TEnemy2DVeteran::CheckPosition(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		// If the new position is fully within a row, then check this row only
		if (vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 600)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 600 ||
				cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 600)
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
			vec2NumMicroSteps.x = 0;
			return true;
		}
		// If the new position is fully within a row, then check this row only
		if (vec2NumMicroSteps.y == 0)
		{
			if (vec2NumMicroSteps.x != 0)
			{
				// If the grid is not accessible, then return false
				if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 600)
				{
					return false;
				}
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (vec2NumMicroSteps.y != 0)
		{
			if (vec2NumMicroSteps.x != 0)
			{
				// If the 2 grids are not accessible, then return false
				if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 600 ||
					cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 600)
				{
					return false;
				}
			}
		}
	}
	else if (eDirection == UP)
	{
		// If the new position is at the top row, then return true
		if (vec2Index.y >= cSettings->NUM_TILES_YAXIS - 1)
		{
			vec2NumMicroSteps.y = 0;
			return true;
		}
		// If the new position is fully within a column, then check this column only
		if (vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 600)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 600 ||
				cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 600)
			{
				return false;
			}
		}
	}
	else if (eDirection == DOWN)
	{
		// If the new position is fully within a column, then check this column only
		if (vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 600)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 600 ||
				cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 600)
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
bool TEnemy2DVeteran::IsMidAir(void)
{
	// if the player is at the bottom row, then he is not in mid-air for sure
	if (vec2Index.y == 0)
		return false;

	// Check if the tile below the player's current position is empty
	if ((vec2NumMicroSteps.x == 0) &&
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
void TEnemy2DVeteran::UpdateJumpFall(const double dElapsedTime)
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
			vec2NumMicroSteps.y += iDisplacement_MicroSteps;
			if (vec2NumMicroSteps.y > cSettings->ENEMY_NUM_STEPS_PER_TILE_YAXIS)
			{
				vec2NumMicroSteps.y -= cSettings->ENEMY_NUM_STEPS_PER_TILE_YAXIS;
				if (vec2NumMicroSteps.y < 0)
					vec2NumMicroSteps.y = 0;
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
				vec2NumMicroSteps.y = 0;
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
			vec2NumMicroSteps.y -= fabs(iDisplacement_MicroSteps);
			if (vec2NumMicroSteps.y < 0)
			{
				vec2NumMicroSteps.y = ((int)cSettings->ENEMY_NUM_STEPS_PER_TILE_YAXIS) - 1;
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
				vec2NumMicroSteps.y = 0;
				cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::ENEMY_LAND); // play enemy landing sound
				break;
			}
		}
	}
}

/**
 @brief Let enemy2D interact with the player.
 */
bool TEnemy2DVeteran::InteractWithPlayer(void)
{
	glm::vec2 vec2PlayerPos = cPlayer2D->vec2Index;
	
	// Check if the enemy2D is within 1.5 indices of the player2D
	if (((vec2Index.x >= vec2PlayerPos.x - 0.5) && 
		(vec2Index.x <= vec2PlayerPos.x + 0.5))
		&& 
		((vec2Index.y >= vec2PlayerPos.y - 0.5) &&
		(vec2Index.y <= vec2PlayerPos.y + 0.5)))
	{
		return true;
	}
	return false;
}

//enemy interact with map
void TEnemy2DVeteran::InteractWithMap(void)
{
	switch (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x))
	{
	default:
		break;
	}
}

/**
 @brief Update the enemy's direction.
 */
void TEnemy2DVeteran::UpdateDirection(void)
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
		vec2Direction = glm::vec2(0);
	}
}

/**
 @brief Flip horizontal direction. For patrol use only
 */
void TEnemy2DVeteran::FlipHorizontalDirection(void)
{
	vec2Direction.x *= -1;
}

/**
@brief Update position.
*/
void TEnemy2DVeteran::UpdatePosition(void)
{
	// Store the old position
	vec2OldIndex = vec2Index;

	// if the player is to the left or right of the enemy2D, then jump to attack
	if (vec2Direction.x < 0)
	{
		// Move left
		const int iOldIndex = vec2Index.x;
		if (vec2Index.x >= 0)
		{
			vec2NumMicroSteps.x--;
			if (vec2NumMicroSteps.x < 0)
			{
				vec2NumMicroSteps.x = ((int)cSettings->ENEMY_NUM_STEPS_PER_TILE_XAXIS) - 1;
				vec2Index.x--;
			}
		}

		// Constraint the enemy2D's position within the screen boundary
		Constraint(LEFT);

		// Find a feasible position for the enemy2D's current position
		if (CheckPosition(LEFT) == false)
		{
			FlipHorizontalDirection();
			vec2Index = vec2OldIndex;
			vec2NumMicroSteps.x = 0;
		}

		// Check if enemy2D is in mid-air, such as walking off a platform
		if (IsMidAir() == true)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}

		// Interact with the Player
		//InteractWithPlayer();
	}
	else if (vec2Direction.x > 0)
	{
		// Move right
		const int iOldIndex = vec2Index.x;
		if (vec2Index.x < (int)cSettings->NUM_TILES_XAXIS)
		{
			vec2NumMicroSteps.x++;

			if (vec2NumMicroSteps.x >= cSettings->ENEMY_NUM_STEPS_PER_TILE_XAXIS)
			{
				vec2NumMicroSteps.x = 0;
				vec2Index.x++;
			}
		}

		// Constraint the enemy2D's position within the screen boundary
		Constraint(RIGHT);

		// Find a feasible position for the enemy2D's current position
		if (CheckPosition(RIGHT) == false)
		{
			FlipHorizontalDirection();
			//vec2Index = vec2OldIndex;
			vec2NumMicroSteps.x = 0;
		}

		// Check if enemy2D is in mid-air, such as walking off a platform
		if (IsMidAir() == true)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}

		// Interact with the Player
		//InteractWithPlayer();
	}

	if (vec2Direction.x != 0 && cPhysics2D.GetStatus() == CPhysics2D::STATUS::IDLE)
	{
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::ENEMY_FOOTSTEPS); // play enemy footstep sound
	}

	// if the player is above the enemy2D, then jump to attack
	if (vec2Direction.y > 0)
	{
		if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::IDLE)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::JUMP);
			cPhysics2D.SetInitialVelocity(glm::vec2(0.0f, 3.5f));
			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::ENEMY_JUMP); // play enemy jump sound
		}
	}
}

vector<glm::vec2> TEnemy2DVeteran::ConstructWaypointVector(vector<glm::vec2> waypointVector, int startIndex, int numOfWaypoints)
{
	for (int i = 0; i < numOfWaypoints; ++i)
	{
		waypointVector.push_back(cMap2D->GetTilePosition(startIndex + i));
		// Erase the value of the waypoint in the arrMapInfo
		cMap2D->SetMapInfo(waypointVector[i].y, waypointVector[i].x, 0);
		cout << waypointVector.size() << endl;
	}

	return waypointVector;
}

//called whenever an ammo is needed to be shot
CTEAmmoVeteran* TEnemy2DVeteran::FetchAmmo()
{
	//Exercise 3a: Fetch a game object from m_goList and return it
	for (std::vector<CTEAmmoVeteran*>::iterator it = ammoList.begin(); it != ammoList.end(); ++it)
	{
		CTEAmmoVeteran* ammo = (CTEAmmoVeteran*)*it;
		if (ammo->getActive()) {
			continue;
		}
		ammo->setActive(true);
		// By default, microsteps should be zero --> reset in case a previously active ammo that was used then ste inactive was used again
		ammo->vec2NumMicroSteps = glm::vec2(0, 0);
		return ammo;
	}

	//whenever ammoList runs out of ammo, create 10 ammo to use
	//Get Size before adding 10
	int prevSize = ammoList.size();
	for (int i = 0; i < 10; ++i) {
		ammoList.push_back(new CTEAmmoVeteran);
	}
	ammoList.at(prevSize)->setActive(true);
	return ammoList.at(prevSize);

}