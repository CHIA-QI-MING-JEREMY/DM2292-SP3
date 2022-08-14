/**
 CEnemy2D
 @brief A class which represents the enemy object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Enemy2D.h"

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
CEnemy2D::CEnemy2D(void)
	: bIsActive(false)
	, cMap2D(NULL)
	, cSettings(NULL)
	, cPlayer2D(NULL)
	, sCurrentFSM(FSM::IDLE)
	, iFSMCounter(0)
	, quadMesh(NULL)
	, animatedSprites(NULL)
	, cSoundController(NULL)
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
CEnemy2D::~CEnemy2D(void)
{
	// Delete the CAnimationSprites
	if (animatedSprites)
	{
		delete animatedSprites;
		animatedSprites = NULL;
	}

	// We won't delete this since it was created elsewhere
	cPlayer2D = NULL;

	// We won't delete this since it was created elsewhere
	cMap2D = NULL;

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

/**
  @brief Initialise this instance
  */
bool CEnemy2D::Init(void)
{
	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	// Get the handler to the CMap2D instance
	cMap2D = CMap2D::GetInstance();
	// Find the indices for the player in arrMapInfo, and assign it to cPlayer2D
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	if (cMap2D->FindValue(300, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the player in the arrMapInfo
	cMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	i32vec2NumMicroSteps = glm::i32vec2(0, 0);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Create the quad mesh for the player
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

	// Load the enemy2D texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_Ninja.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_Ninja.png" << endl;
		return false;
	}

	// Create the animated sprite and setup the animation
	animatedSprites = CMeshBuilder::GenerateSpriteAnimation(6, 10, cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	animatedSprites->AddAnimation("idleR", 0, 9);
	animatedSprites->AddAnimation("idleL", 10, 19);
	animatedSprites->AddAnimation("attackR", 20, 29);
	animatedSprites->AddAnimation("attackL", 30, 39);
	animatedSprites->AddAnimation("runR", 40, 49);
	animatedSprites->AddAnimation("runL", 50, 59);
	// Play idle animation as default
	animatedSprites->PlayAnimation("idleR", -1, 1.0f);

	// Get the handler to the CInventoryManager instance
	cInventoryManager = CInventoryManager::GetInstance();

	//CS: Init the color to white
	runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);

	// Set the Physics to fall status by default
	cPhysics2D.Init();
	cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);

	// If this class is initialised properly, then set the bIsActive to true
	bIsActive = true;

	// Update the UV Coordinates
	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false, i32vec2NumMicroSteps.x * cSettings->ENEMY_MICRO_STEP_XAXIS);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false, i32vec2NumMicroSteps.y * cSettings->ENEMY_MICRO_STEP_YAXIS);

	// Load the sounds into CSoundController
	cSoundController = CSoundController::GetInstance();

	// set waypoint patrol path based on enemy spawn location
	if (vec2Index == glm::vec2(14, 5))
	{
		waypoints.push_back(glm::vec2(14, 5));
		waypoints.push_back(glm::vec2(23, 5));
		waypoints.push_back(glm::vec2(28, 5));
		waypoints.push_back(glm::vec2(20, 5));
	}
	else if (vec2Index == glm::vec2(10, 14))
	{
		waypoints.push_back(glm::vec2(10, 14));
		waypoints.push_back(glm::vec2(7, 14));
		waypoints.push_back(glm::vec2(0, 20));
	}

	// sets waypoint counter value
	currentWaypointCounter = 0;
	maxWaypointCounter = waypoints.size();
	
	// sets timer values
	maxAttackTimer = 1.0;
	attackTimer = 0.0;
	maxWarnTimer = 2.5;
	warnTimer = maxWarnTimer;

	// sets health
	health = 50;

	return true;
}

/**
 @brief Update this instance
 */
void CEnemy2D::Update(const double dElapsedTime)
{
	// if this enemy is not active, then return now
	if (!bIsActive)
		return;

	// SUPERHOT movement
	//if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::IDLE)
	//{
		// SUPERHOT movement
		//if (cPlayer2D->getPlayerMoveStatus())
		//{

	switch (sCurrentFSM)
	{
	case IDLE:
		if (isAlarmOn)
		{
			sCurrentFSM = ALERT_IDLE;
			iFSMCounter = 0;
			cout << "Switching to Alert_Idle State" << endl;
			break;
		}

		if (vec2Direction.x > 0)
		{
			// Play the "idleR" animation
			animatedSprites->PlayAnimation("idleR", -1, 1.f);
		}
		else if(vec2Direction.x < 0)
		{
			// Play the "idleL" animation
			animatedSprites->PlayAnimation("idleL", -1, 1.f);
		}

		if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = PATROL;
			iFSMCounter = 0;
			cout << "Switching to Patrol State" << endl;
		}
		iFSMCounter++;
		break;
	case PATROL:
		if (isAlarmOn)
		{
			sCurrentFSM = ALERT_IDLE;
			iFSMCounter = 0;
			cout << "Switching to Alert_Idle State" << endl;
			break;
		}
		
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 5.0f)
		{
			srand(static_cast<unsigned> (time(0)));
			int randomState = rand() % 100;
			cout << randomState << endl;
			if (randomState < 50)
			{
				sCurrentFSM = TRACK;
				cout << "Switching to Track State" << endl;
			}
			else
			{
				if (!isAlarmerActive && !isAlarmOn)
				{
					sCurrentFSM = WARN;
					isAlarmerActive = true;
					cout << "Switching to Warn State" << endl;
				}
				else
				{
					sCurrentFSM = TRACK;
					cout << "Switching to Track State" << endl;
				}
			}
			iFSMCounter = 0;
		}
		else
		{
			if (vec2Index == waypoints[currentWaypointCounter])
			{
				currentWaypointCounter++;
				sCurrentFSM = IDLE;
				cout << "Switching to Idle State" << endl;
			}

			if (currentWaypointCounter < maxWaypointCounter)
			{
				auto path = cMap2D->PathFind(vec2Index,							// start pos
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
						// Calculate the direction between enemy2D and this detination
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
			}
			else
			{
				currentWaypointCounter = 0;
			}

			UpdatePosition();
		}
		break;
	case TRACK:
		if (isAlarmOn)
		{
			sCurrentFSM = ALERT_IDLE;
			iFSMCounter = 0;
			cout << "Switching to Alert_Idle State" << endl;
			break;
		}
		
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 5.0f)
		{
			//cout << "i32vec2Destination : " << i32vec2Destination.x 
			//		<< ", " << i32vec2Destination.y << endl;
			//cout << "i32vec2Direction : " << i32vec2Direction.x 
			//		<< ", " << i32vec2Direction.y << endl;
			//system("pause");

			// Attack
			// Update direction to move towards for attack
			//UpdateDirection();

			// Calculate a path to the player
			/*cMap2D->PrintSelf();
			cout << "StartPos: " << vec2Index.x << ", " << vec2Index.y << endl;
			cout << "TargetPos: " << cPlayer2D->vec2Index.x << ", " << cPlayer2D->vec2Index.y << endl;*/
					
			auto path = cMap2D->PathFind(vec2Index,				// start pos
										cPlayer2D->vec2Index,	// target pos
										heuristic::euclidean,	// heuristic
										10);					// weight

			/*cout << "=== Printing out the path ===" << endl;*/

			// Calculate new destination
			bool bFirstPosition = true;
			for (const auto& coord : path)
			{
				//std::cout << coord.x << ", " << coord.y << "\n";
				if (bFirstPosition == true)
				{
					// Set a destination
					vec2Destination = coord;
					// Calculate the direction between enemy2D and this detination
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

				// debug code
				/*cout << "vec2Destination: " << vec2Destination.x << ", " << vec2Destination.y << endl;
				cout << "vec2Direction: " << vec2Direction.x << ", " << vec2Direction.y << endl;
				system("pause");*/
			}
					
			// Update the Enemy2D's position
			UpdatePosition();

			if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) <= 0.5f)
			{
				sCurrentFSM = ATTACK;
				cout << "Switching to Attack State" << endl;
				iFSMCounter = 0;
			}
		}
		else
		{
			sCurrentFSM = PATROL;
			iFSMCounter = 0;
			cout << "Switching to Patrol state" << endl;
		}
		break;
	case ATTACK:
	{
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) <= 0.5f)
		{
			InteractWithPlayer();
		}
		else
		{
			sCurrentFSM = TRACK;
			cout << "Switching to Track state" << endl;
		}
		break;
	}
	case WARN:
	{
		auto path = cMap2D->PathFind(vec2Index,							// start pos
									assignedAlarmBox,					// target pos
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
				// Calculate the direction between enemy2D and this detination
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

		// Update the Enemy2D's position
		UpdatePosition();
		
		if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == 50)
		{
			sCurrentFSM = ALARM_TRIGGER;
			cout << "Switching to ALARM_TRIGGER state" << endl;

			warnTimer = maxWarnTimer;
		}

		break;
	}
	case ALARM_TRIGGER:
	{
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
		
		if (warnTimer <= 0.0 && !isAlarmOn)
		{
			InteractWithMap();
		}
		else if (warnTimer > 0.0 && !isAlarmOn)
		{
			warnTimer -= dElapsedTime;
		}
		
		if (isAlarmOn)
		{
			isAlarmerActive = false;

			sCurrentFSM = ALERT_IDLE;
			cout << "Switching to Alert_Idle state" << endl;
		}
		break;
	}
	case ALERT_IDLE:
	{
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
		
		if (!isAlarmOn)
		{
			sCurrentFSM = IDLE;
			cout << "Switching to Idle State" << endl;
			iFSMCounter = 0;
			break;
		}
		
		if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = ALERT_PATROL;
			iFSMCounter = 0;
			cout << "Switching to Alert Patrol State" << endl;
		}
		iFSMCounter += 3;
		break;
	}
	case ALERT_PATROL:
	{
		if (!isAlarmOn)
		{
			sCurrentFSM = IDLE;
			cout << "Switching to Idle State" << endl;
			iFSMCounter = 0;
			break;
		}
		
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 7.5f)
		{
			sCurrentFSM = ALERT_TRACK;
			cout << "Switching to Alert Track State" << endl;
		}
		else
		{
			if (vec2Index == waypoints[currentWaypointCounter])
			{
				currentWaypointCounter++;
				sCurrentFSM = ALERT_IDLE;
				cout << "Switching to Alert Idle State" << endl;
			}

			if (currentWaypointCounter < maxWaypointCounter)
			{
				auto path = cMap2D->PathFind(vec2Index,							// start pos
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
						// Calculate the direction between enemy2D and this detination
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
			}
			else
			{
				currentWaypointCounter = 0;
			}

			UpdatePosition();
		}
		break;
	}
	case ALERT_TRACK:
	{
		if (!isAlarmOn)
		{
			sCurrentFSM = IDLE;
			cout << "Switching to Idle State" << endl;
			iFSMCounter = 0;
			break;
		}
		
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 7.5f)
		{
			//cout << "i32vec2Destination : " << i32vec2Destination.x 
			//		<< ", " << i32vec2Destination.y << endl;
			//cout << "i32vec2Direction : " << i32vec2Direction.x 
			//		<< ", " << i32vec2Direction.y << endl;
			//system("pause");

			// Attack
			// Update direction to move towards for attack
			//UpdateDirection();

			// Calculate a path to the player
			/*cMap2D->PrintSelf();
			cout << "StartPos: " << vec2Index.x << ", " << vec2Index.y << endl;
			cout << "TargetPos: " << cPlayer2D->vec2Index.x << ", " << cPlayer2D->vec2Index.y << endl;*/

			auto path = cMap2D->PathFind(vec2Index,				// start pos
										cPlayer2D->vec2Index,	// target pos
										heuristic::euclidean,	// heuristic
										10);					// weight

			/*cout << "=== Printing out the path ===" << endl;*/

			// Calculate new destination
			bool bFirstPosition = true;
			for (const auto& coord : path)
			{
				//std::cout << coord.x << ", " << coord.y << "\n";
				if (bFirstPosition == true)
				{
					// Set a destination
					vec2Destination = coord;
					// Calculate the direction between enemy2D and this detination
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

				// debug code
				/*cout << "vec2Destination: " << vec2Destination.x << ", " << vec2Destination.y << endl;
				cout << "vec2Direction: " << vec2Direction.x << ", " << vec2Direction.y << endl;
				system("pause");*/
			}

			// Update the Enemy2D's position
			UpdatePosition();

			if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) <= 0.5f)
			{
				sCurrentFSM = ALERT_ATTACK;
				cout << "Switching to Alert Attack State" << endl;
				iFSMCounter = 0;
			}
		}
		else
		{
			sCurrentFSM = ALERT_PATROL;
			iFSMCounter = 0;
			cout << "Switching to Alert Patrol state" << endl;
		}
		break;
	}
	case ALERT_ATTACK:
	{
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) <= 0.5f)
		{
			InteractWithPlayer();
		}
		else
		{
			sCurrentFSM = ALERT_TRACK;
			cout << "Switching to Alert Track state" << endl;
		}
		break;
	}
	default:
		break;
	}


		//}
	//}
	// SUPERHOT movement
	//else
	//{
	
	// Update Jump or Fall
	UpdateJumpFall(dElapsedTime);

	//}

	// Update the animated sprite
	animatedSprites->Update(dElapsedTime);

	// Update the UV Coordinates
	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false, i32vec2NumMicroSteps.x*cSettings->ENEMY_MICRO_STEP_XAXIS);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false, i32vec2NumMicroSteps.y*cSettings->ENEMY_MICRO_STEP_YAXIS);

	// reduce attack timer cooldown
	attackTimer -= dElapsedTime;

	InteractWithPlayer();
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CEnemy2D::PreRender(void)
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
void CEnemy2D::Render(void)
{
	if (!bIsActive)
		return;

	glBindVertexArray(VAO);
	// get matrix's uniform location and set matrix
	unsigned int transformLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "transform");
	unsigned int colorLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "runtimeColour");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	transform = glm::translate(transform, glm::vec3(vec2UVCoordinate.x,
		vec2UVCoordinate.y,
		0.0f));
	// Update the shaders with the latest transform
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glUniform4fv(colorLoc, 1, glm::value_ptr(runtimeColour));

	// Get the texture to be rendered
	glBindTexture(GL_TEXTURE_2D, iTextureID);

	// Render the tile
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	//quadMesh->Render();

	// Render the animated sprite
	glBindVertexArray(VAO);
	animatedSprites->Render();
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);


	glBindVertexArray(0);

}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CEnemy2D::PostRender(void)
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
void CEnemy2D::Seti32vec2Index(const int iIndex_XAxis, const int iIndex_YAxis)
{
	this->vec2Index.x = iIndex_XAxis;
	this->vec2Index.y = iIndex_YAxis;
}

/**
@brief Set the number of microsteps of the enemy2D
@param iNumMicroSteps_XAxis A const int variable storing the current microsteps in the X-axis
@param iNumMicroSteps_YAxis A const int variable storing the current microsteps in the Y-axis
*/
void CEnemy2D::Seti32vec2NumMicroSteps(const int iNumMicroSteps_XAxis, const int iNumMicroSteps_YAxis)
{
	this->i32vec2NumMicroSteps.x = iNumMicroSteps_XAxis;
	this->i32vec2NumMicroSteps.y = iNumMicroSteps_YAxis;
}

/**
 @brief Set the handle to cPlayer to this class instance
 @param cPlayer2D A CPlayer2D* variable which contains the pointer to the CPlayer2D instance
 */
void CEnemy2D::SetPlayer2D(CPlayer2D* cPlayer2D)
{
	this->cPlayer2D = cPlayer2D;

	// Update the enemy's direction
	UpdateDirection();
}

/**
 @brief Constraint the enemy2D's position within a boundary
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
void CEnemy2D::Constraint(DIRECTION eDirection)
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
		cout << "CEnemy2D::Constraint: Unknown direction." << endl;
	}
}

/**
 @brief Check if a position is possible to move into
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
bool CEnemy2D::CheckPosition(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		// If the new position is fully within a row, then check this row only
		if (i32vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100))
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
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 100))
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
			if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (i32vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 100))
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
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (i32vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100))
			{
				return false;
			}
		}
	}
	else
	{
		cout << "CEnemy2D::CheckPosition: Unknown direction." << endl;
	}

	return true;
}

// Check if the enemy2D is in mid-air
bool CEnemy2D::IsMidAir(void)
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
void CEnemy2D::UpdateJumpFall(const double dElapsedTime)
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
				cSoundController->PlaySoundByID(4); // plays soft thump sound when player hits the floor
				break;
			}
		}
	}
}

void CEnemy2D::InteractWithMap(void)
{
	switch (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x))
	{
	case 50:
	{
		cMap2D->ReplaceTiles(51, 52);
		isAlarmOn = true;
	}
	default:
		break;
	}
}

/**
 @brief Let enemy2D interact with the player.
 */
bool CEnemy2D::InteractWithPlayer(void)
{
	glm::i32vec2 i32vec2PlayerPos = cPlayer2D->vec2Index;
	
	// if attack cooldown timer has expired
	if (attackTimer <= 0.0)
	{
		// Check if the enemy2D is within 1.5 indices of the player2D
		if (((vec2Index.x >= i32vec2PlayerPos.x - 0.5) &&
			(vec2Index.x <= i32vec2PlayerPos.x + 0.5))
			&&
			((vec2Index.y >= i32vec2PlayerPos.y - 0.5) &&
				(vec2Index.y <= i32vec2PlayerPos.y + 0.5)))
		{
			// TO DO check for player colour then stun if attack when same colour
			
			if (vec2Direction.x > 0)
			{
				// Play the "attackR" animation
				animatedSprites->PlayAnimation("attackR", -1, 1.f);
			}
			else if (vec2Direction.x < 0)
			{
				// Play the "attackL" animation
				animatedSprites->PlayAnimation("attackL", -1, 1.f);
			}

			cInventoryItem = cInventoryManager->GetItem("Health");
			if (sCurrentFSM == ATTACK)
			{
				// decrease health by 20
				cInventoryItem->Remove(20);

				// Since the player has been caught, then reset the FSM
				sCurrentFSM = IDLE;
				iFSMCounter = 0;
			}
			else if (sCurrentFSM == ALERT_ATTACK)
			{
				// decrease health by 35
				cInventoryItem->Remove(35);

				// Since the player has been caught, then reset the FSM
				sCurrentFSM = ALERT_IDLE;
				iFSMCounter = 0;
			}
			
			// Calculate nearest waypoint to return to (TO DO)

			// reset attack cooldown timer
			attackTimer = maxAttackTimer;
			return true;
		}
	}
	
	return false;
}

/**
 @brief Update the enemy's direction.
 */
void CEnemy2D::UpdateDirection(void)
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
void CEnemy2D::FlipHorizontalDirection(void)
{
	vec2Direction.x *= -1;
}

/**
@brief Update position.
*/
void CEnemy2D::UpdatePosition(void)
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

		// Constraint the enemy2D's position within the screen boundary
		Constraint(LEFT);

		if (CheckPosition(LEFT) == false)
		{
			FlipHorizontalDirection();
			vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.x = 0;
		}

		// Play the "runR" animation
		animatedSprites->PlayAnimation("runL", -1, 1.f);
		
		// Check if enemy2D is in mid-air, such as walking off a platform
		if (IsMidAir() == true)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}
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

		// Constraint the enemy2D's position within the screen boundary
		Constraint(RIGHT);

		if (CheckPosition(RIGHT) == false)
		{
			FlipHorizontalDirection();
			//vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.x = 0;
		}

		// Play the "runL" animation
		animatedSprites->PlayAnimation("runR", -1, 1.f);

		// Check if enemy2D is in mid-air, such as walking off a platform
		if (IsMidAir() == true)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}
	}

	// if the player is above the enemy2D, then jump to attack
	if (vec2Direction.y > 0)
	{
		if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::IDLE)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::JUMP);
			cPhysics2D.SetInitialVelocity(glm::vec2(0.0f, 2.7f)); // jump height: 1.8f -> 3 tiles for 64/48, 2.6f -> 3 tiles for 32/24
		}

		// Play a sound for jump
		cSoundController->PlaySoundByID(3);
	}
}
