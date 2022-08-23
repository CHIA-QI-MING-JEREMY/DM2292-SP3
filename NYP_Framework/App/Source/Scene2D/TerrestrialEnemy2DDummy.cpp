/**
 CStnEnemy2D
 @brief A class which represents the enemy object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "TerrestrialEnemy2DDummy.h"

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
TEnemy2DDummy::TEnemy2DDummy(void)
	: bIsActive(false)
	, cMap2D(NULL)
	, cSettings(NULL)
	, cPlayer2D(NULL)
	, sCurrentFSM(FSM::HIDDEN)
	, iFSMCounter(0)
	, quadMesh(NULL)
	, camera2D(NULL)
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
TEnemy2DDummy::~TEnemy2DDummy(void)
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
bool TEnemy2DDummy::Init(void)
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
	vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	i32vec2NumMicroSteps = glm::i32vec2(0, 0);

	// Create and initialise the CPlayer2D
	cPlayer2D = CPlayer2D::GetInstance();

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//CS: Create the Quad Mesh using the mesh builder
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

	// Load the enemy texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/TerrestrialPlanet/TurretSpriteSheet_TileFade.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/TerrestrialPlanet/TurretSpriteSheet_TileFade.png" << endl;
		return false;
	}

	// Create the animated sprite and setup the animation
	animatedSprites = CMeshBuilder::GenerateSpriteAnimation(6, 8, cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	animatedSprites->AddAnimation("hidden", 0, 0);
	animatedSprites->AddAnimation("decloakR", 0, 7);
	animatedSprites->AddAnimation("decloakL", 8, 15);
	animatedSprites->AddAnimation("targetR", 16, 18);
	animatedSprites->AddAnimation("targetL", 24, 26);
	animatedSprites->AddAnimation("cloakR", 16, 23);
	animatedSprites->AddAnimation("cloakL", 24, 31);
	animatedSprites->AddAnimation("attackR", 34, 34);
	animatedSprites->AddAnimation("attackL", 42, 42);

	// Play hidden animation as default
	animatedSprites->PlayAnimation("hidden", -1, 1.0f);

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
		CTEAmmoDummy* cEnemyAmmo2D = new CTEAmmoDummy();
		cEnemyAmmo2D->SetShader("Shader2D");
		ammoList.push_back(cEnemyAmmo2D);
	}

	type = LONG_RANGE; //has ammo
	shootingDirection = RIGHT; //setting direction for ammo shooting
	maxHealth = health = 25; //takes 1 hit to kill

	isAlarmerActive = false;
	isAlarmOn = false;

	maxAttackTimer = 0.21;
	attackTimer = 0;

	return true;
}

/**
 @brief Update this instance
 */
void TEnemy2DDummy::Update(const double dElapsedTime)
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
	case HIDDEN:
	{
		if (vec2Direction.x > 0)
		{
			animatedSprites->PlayAnimation("hidden", -1, 1.0f);
			//cout << "Play hidden anim" << endl;
		}
		else if (vec2Direction.x < 0)
		{
			animatedSprites->PlayAnimation("hidden", -1, 1.0f);
			//cout << "Play hidden anim" << endl;
		}

		if (isAlarmOn)
		{
			sCurrentFSM = ALERT_DECLOAK;
			iFSMCounter = 0;
			animatedSprites->Reset();
			cout << "Switching to Alert_Decloak state" << endl;
			break;
		}

		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) <= 5.f)
		{
			sCurrentFSM = DECLOAK;
			iFSMCounter = 0;
			animatedSprites->Reset();
			cout << "Switching to Decloak State" << endl;
			break;
		}
		break;
	}
	case DECLOAK:
	{
		if (vec2Direction.x > 0)
		{
			animatedSprites->PlayAnimation("decloakR", 0, 1.f);
			//cout << "Play decloakR anim" << endl;
		}
		else if (vec2Direction.x < 0)
		{
			animatedSprites->PlayAnimation("decloakL", 0, 1.f);
			//cout << "Play decloakL anim" << endl;
		}

		if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = ATTACK;
			iFSMCounter = 0;
			maxAttackTimer = 0.21;
			attackTimer = 0;
			cout << "Switching to Attack State" << endl;
			break;
		}
		iFSMCounter++;
		break;
	}
	case ATTACK:
	{
		if (vec2Direction.x > 0)
		{
			animatedSprites->PlayAnimation("attackR", 0, 1.f);
			//cout << "Play attackR anim" << endl;

			shootingDirection = RIGHT;
		}
		else if (vec2Direction.x < 0)
		{
			animatedSprites->PlayAnimation("attackL", 0, 1.f);
			//cout << "Play attackL anim" << endl;

			shootingDirection = LEFT;
		}

		if (attackTimer <= 0)
		{
			// Shoot enemy ammo!
			//shoot ammo in accordance to the direction enemy is facing
			CTEAmmoDummy* ammo = FetchAmmo();
			ammo->setActive(true);
			ammo->setPath(vec2Index.x, vec2Index.y, shootingDirection);
			ammo->setIsAlerted(false);
			cout << "Bam!" << shootingDirection << endl;

			attackTimer = maxAttackTimer;
		}
		else
		{
			attackTimer -= dElapsedTime;
		}

		if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = TARGET;
			iFSMCounter = 0;
			attackTimer = 0;
			cout << "Switching to Target State" << endl;
			break;
		}
		iFSMCounter++;
		break;
	}
	case TARGET:
	{
		if (vec2Direction.x > 0)
		{
			animatedSprites->PlayAnimation("targetR", -1, 1.f);
			//cout << "Play targetR anim" << endl;
		}
		else if (vec2Direction.x < 0)
		{
			animatedSprites->PlayAnimation("targetL", -1, 1.f);
			//cout << "Play targetL anim" << endl;
		}

		if (isAlarmOn)
		{
			sCurrentFSM = ALERT_TARGET;
			iFSMCounter = 0;
			cout << "Switching to Alert_Target state" << endl;
			break;
		}

		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) > 7.f)
		{
			sCurrentFSM = CLOAK;
			iFSMCounter = 0;
			cout << "Switching to Cloak State" << endl;
			break;
		}

		if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = ATTACK;
			iFSMCounter = 0;
			maxAttackTimer = 0.21;
			attackTimer = 0;
			cout << "Switching to Attack State" << endl;
			break;
		}
		iFSMCounter++;
		break;
	}
	case CLOAK:
	{
		if (vec2Direction.x > 0)
		{
			animatedSprites->PlayAnimation("cloakR", 0, 1.f);
			//cout << "Play cloakR anim" << endl;
		}
		else if (vec2Direction.x < 0)
		{
			animatedSprites->PlayAnimation("cloakL", 0, 1.f);
			//cout << "Play cloakL anim" << endl;
		}

		if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = HIDDEN;
			iFSMCounter = 0;
			cout << "Switching to Hidden State" << endl;
		}
		iFSMCounter++;
		break;
	}
	case ALERT_DECLOAK:
	{
		if (vec2Direction.x > 0)
		{
			animatedSprites->PlayAnimation("decloakR", 0, 0.5f);
			//cout << "Play decloakR anim" << endl;
		}
		else if (vec2Direction.x < 0)
		{
			animatedSprites->PlayAnimation("decloakL", 0, 0.5f);
			//cout << "Play decloakL anim" << endl;
		}

		if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = ALERT_ATTACK;
			iFSMCounter = 0;
			maxAttackTimer = 0.11;
			attackTimer = 0;
			cout << "Switching to Alert_Attack State" << endl;
		}
		iFSMCounter += 2;
		break;
	}
	case ALERT_ATTACK:
		if (vec2Direction.x > 0)
		{
			animatedSprites->PlayAnimation("attackR", 0, 1.f);
			//cout << "Play attackR anim" << endl;

			shootingDirection = RIGHT;
		}
		else if (vec2Direction.x < 0)
		{
			animatedSprites->PlayAnimation("attackL", 0, 1.f);
			//cout << "Play attackL anim" << endl;

			shootingDirection = LEFT;
		}

		if (attackTimer <= 0)
		{
			// Shoot enemy ammo!
			//shoot ammo in accordance to the direction enemy is facing
			CTEAmmoDummy* ammo = FetchAmmo();
			ammo->setActive(true);
			ammo->setPath(vec2Index.x, vec2Index.y, shootingDirection);
			ammo->setIsAlerted(true);
			cout << "Bam!" << shootingDirection << endl;

			attackTimer = maxAttackTimer;
		}
		else
		{
			attackTimer -= dElapsedTime;
		}

		if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = ALERT_TARGET;
			iFSMCounter = 0;
			attackTimer = 0;
			cout << "Switching to Alert_Target State" << endl;
			break;
		}
		iFSMCounter++;
		break;
	case ALERT_TARGET:
	{
		if (vec2Direction.x > 0)
		{
			animatedSprites->PlayAnimation("targetR", -1, 0.5f);
			//cout << "Play targetR anim" << endl;
		}
		else if (vec2Direction.x < 0)
		{
			animatedSprites->PlayAnimation("targetL", -1, 0.5f);
			//cout << "Play targetL anim" << endl;
		}

		if (!isAlarmOn)
		{
			sCurrentFSM = TARGET;
			iFSMCounter = 0;
			cout << "Switching to Target state" << endl;
			break;
		}

		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) > 9.f)
		{
			sCurrentFSM = ALERT_STANDBY;
			iFSMCounter = 0;
			cout << "Switching to Alert_Standby State" << endl;
			break;
		}

		if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = ALERT_ATTACK;
			iFSMCounter = 0;
			maxAttackTimer = 0.11;
			attackTimer = 0;
			cout << "Switching to Alert_Attack State" << endl;
			break;
		}
		iFSMCounter += 2;
		break;
	}
	case ALERT_STANDBY:
	{
		if (vec2Direction.x > 0)
		{
			animatedSprites->PlayAnimation("targetR", -1, 0.5f);
			//cout << "Play targetR anim" << endl;
		}
		else if (vec2Direction.x < 0)
		{
			animatedSprites->PlayAnimation("targetL", -1, 0.5f);
			//cout << "Play targetL anim" << endl;
		}

		if (!isAlarmOn)
		{
			sCurrentFSM = TARGET;
			iFSMCounter = 0;
			cout << "Switching to Target State" << endl;
			break;
		}

		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) <= 9.f)
		{
			sCurrentFSM = ALERT_TARGET;
			iFSMCounter = 0;
			cout << "Switching to Alert_Target State" << endl;
			break;
		}
		break;
	}
	default:
		break;
	}

	//ammo beahviour
	for (std::vector<CTEAmmoDummy*>::iterator it = ammoList.begin(); it != ammoList.end(); ++it)
	{
		CTEAmmoDummy* ammo = (CTEAmmoDummy*)*it;
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

	//update sprite animation to play depending on the direction enemy is facing
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
void TEnemy2DDummy::PreRender(void)
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
void TEnemy2DDummy::Render(void)
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
	actualPos.x += i32vec2NumMicroSteps.x * cSettings->ENEMY_MICRO_STEP_XAXIS;
	actualPos.y += i32vec2NumMicroSteps.y * cSettings->ENEMY_MICRO_STEP_YAXIS;

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
	for (std::vector<CTEAmmoDummy*>::iterator it = ammoList.begin(); it != ammoList.end(); ++it)
	{
		CTEAmmoDummy* ammo = (CTEAmmoDummy*)*it;
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
void TEnemy2DDummy::PostRender(void)
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
void TEnemy2DDummy::Seti32vec2Index(const int iIndex_XAxis, const int iIndex_YAxis)
{
	this->vec2Index.x = iIndex_XAxis;
	this->vec2Index.y = iIndex_YAxis;
}

/**
@brief Set the number of microsteps of the enemy2D
@param iNumMicroSteps_XAxis A const int variable storing the current microsteps in the X-axis
@param iNumMicroSteps_YAxis A const int variable storing the current microsteps in the Y-axis
*/
void TEnemy2DDummy::Seti32vec2NumMicroSteps(const int iNumMicroSteps_XAxis, const int iNumMicroSteps_YAxis)
{
	this->i32vec2NumMicroSteps.x = iNumMicroSteps_XAxis;
	this->i32vec2NumMicroSteps.y = iNumMicroSteps_YAxis;
}

/**
 @brief Set the handle to cPlayer to this class instance
 @param cPlayer2D A CPlayer2D* variable which contains the pointer to the CPlayer2D instance
 */
void TEnemy2DDummy::SetPlayer2D(CPlayer2D* cPlayer2D)
{
	this->cPlayer2D = cPlayer2D;

	// Update the enemy's direction
	UpdateDirection();
}

/**
 @brief Constraint the enemy2D's position within a boundary
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
void TEnemy2DDummy::Constraint(DIRECTION eDirection)
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
bool TEnemy2DDummy::CheckPosition(DIRECTION eDirection)
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
bool TEnemy2DDummy::IsMidAir(void)
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
void TEnemy2DDummy::UpdateJumpFall(const double dElapsedTime)
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

// TO DO
/**
 @brief Let enemy2D interact with the player.
 */
bool TEnemy2DDummy::InteractWithPlayer(void)
{
	glm::i32vec2 i32vec2PlayerPos = cPlayer2D->vec2Index;
	
	// Check if the enemy2D is within 1.5 indices of the player2D
	if (((vec2Index.x >= i32vec2PlayerPos.x - 0.5) && 
		(vec2Index.x <= i32vec2PlayerPos.x + 0.5))
		&& 
		((vec2Index.y >= i32vec2PlayerPos.y - 0.5) &&
		(vec2Index.y <= i32vec2PlayerPos.y + 0.5)))
	{
		/*
		sCurrentFSM = IDLE;
		iFSMCounter = 0;*/
		return true;
	}
	return false;
}

// TO DO
//enemy interact with map
void TEnemy2DDummy::InteractWithMap(void)
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
void TEnemy2DDummy::UpdateDirection(void)
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
void TEnemy2DDummy::FlipHorizontalDirection(void)
{
	vec2Direction.x *= -1;
}

/**
@brief Update position.
*/
void TEnemy2DDummy::UpdatePosition(void)
{
	// Store the old position
	i32vec2OldIndex = vec2Index;

	// if the player is to the left or right of the enemy2D, then jump to attack
	if (vec2Direction.x < 0)
	{
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
		//InteractWithPlayer();
	}
	else if (vec2Direction.x > 0)
	{
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
		}

		// Interact with the Player
		//InteractWithPlayer();
	}
}

//called whenever an ammo is needed to be shot
CTEAmmoDummy* TEnemy2DDummy::FetchAmmo()
{
	//Exercise 3a: Fetch a game object from m_goList and return it
	for (std::vector<CTEAmmoDummy*>::iterator it = ammoList.begin(); it != ammoList.end(); ++it)
	{
		CTEAmmoDummy* ammo = (CTEAmmoDummy*)*it;
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
		ammoList.push_back(new CTEAmmoDummy);
	}
	ammoList.at(prevSize)->setActive(true);
	return ammoList.at(prevSize);

}