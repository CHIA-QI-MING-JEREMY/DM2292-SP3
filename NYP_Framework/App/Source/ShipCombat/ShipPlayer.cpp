/**
 Player2D
 @brief A class representing the player object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "ShipPlayer.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

// Include ImageLoader
#include "System\ImageLoader.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "../App/Source/Scene2D/Map2D.h"
#include "Primitives/MeshBuilder.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CShipPlayer::CShipPlayer(void)
	: cMap2D(NULL)
	, cKeyboardController(NULL)
	, animatedSprites(NULL)
	, runtimeColour(glm::vec4(1.0f))
	, cSoundController(NULL)
	, camera2D()
{
	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first

	// Initialise vecIndex
	vec2Index = glm::i32vec2(0);

	// Initialise vecNumMicroSteps
	vec2NumMicroSteps = glm::i32vec2(0);

	// Initialise vec2UVCoordinate
	vec2UVCoordinate = glm::vec2(0.0f);
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CShipPlayer::~CShipPlayer(void)
{
	if (cInventoryManager)
	{
		cInventoryManager->Destroy();
		cInventoryManager = NULL;
	}

	// Delete the CAnimationSprites
	if (animatedSprites)
	{
		delete animatedSprites;
		animatedSprites = NULL;
	}
	
	// We won't delete this since it was created elsewhere
	cKeyboardController = NULL;

	// We won't delete this since it was created elsewhere
	cMap2D = NULL;

	// nullify the camera since it was created elsewhere
	camera2D = NULL;
	
	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
}

/**
  @brief Initialise this instance
  */
bool CShipPlayer::Init(void)
{
	// Store the keyboard controller singleton instance here
	cKeyboardController = CKeyboardController::GetInstance();
	// Reset all keys since we are starting a new game
	cKeyboardController->Reset();

	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	camera2D = Camera2D::GetInstance();

	// Get the handler to the CMap2D instance
	cMap2D = CMap2D::GetInstance();
	// Find the indices for the player in arrMapInfo, and assign it to cPlayer2D
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	if (cMap2D->FindValue(1400, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the player in the arrMapInfo
	cMap2D->SetMapInfo(uiRow, uiCol, 0);

	if (cMap2D->FindValue(1400, uiRow, uiCol) == true)
	{
		cout << "Another position of the player has been found" << endl;
		return false;	// Another position of the player has been found, so quit this game
	}

	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	vec2NumMicroSteps = glm::i32vec2(0, 0);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	
	// Create the quad mesh for the player
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH * 0.75, cSettings->TILE_HEIGHT * 0.75 );

	// Load the player texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_PlayerSpriteSheet.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/playerspritesheet.png" << endl;
		return false;
	}

	// Create the animated sprite and setup the animation
	animatedSprites = CMeshBuilder::GenerateSpriteAnimation(4, 4, cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	animatedSprites->AddAnimation("idleR", 0, 0);
	animatedSprites->AddAnimation("idleL", 1, 1);
	animatedSprites->AddAnimation("runR", 4, 7);
	animatedSprites->AddAnimation("runL", 8, 11);
	animatedSprites->AddAnimation("attackR", 2, 2);
	animatedSprites->AddAnimation("attackL", 3, 3);
	animatedSprites->AddAnimation("idleshieldR", 12, 12);
	animatedSprites->AddAnimation("idleshieldL", 13, 13);

	// Play idle animation as default
	animatedSprites->PlayAnimation("idleR", -1, 1.0f);

	// Load the sounds into CSoundController
	cSoundController = CSoundController::GetInstance();

	isPlayerMoving = false;

	return true;
}

/**
 @brief Reset this instance
 */
bool CShipPlayer::Reset()
{
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	if (cMap2D->FindValue(200, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the player in the arrMapInfo
	cMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	vec2NumMicroSteps = glm::i32vec2(0, 0);

	//CS: Init the color to white
	runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);

	isPlayerMoving = false;

	return true;
}

/**
 @brief Update this instance
 */
void CShipPlayer::Update(const double dElapsedTime)
{
	// SUPERHOT movement
	if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::IDLE)
	{
		isPlayerMoving = false;
	}
	
	// Store the old position
	vec2OldIndex = vec2Index;

	// Get keyboard updates	
	if (cKeyboardController->IsKeyDown(GLFW_KEY_A))
	{
		// Calculate the new position to the left
		if (vec2Index.x >= 0)
		{
			vec2NumMicroSteps.x--; //speed_multiplier;
			if (vec2NumMicroSteps.x < 0)
			{
				vec2NumMicroSteps.x = ((int)cSettings->NUM_STEPS_PER_TILE_XAXIS) - 1;
				vec2Index.x--;
			}
		}

		if (!CheckPosition(LEFT))
		{
			vec2Index.x = vec2OldIndex.x;
			vec2NumMicroSteps.x = 0;
		}

		runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);
		Constraint(LEFT);

		// Play the "runL" animation
		animatedSprites->PlayAnimation("runL", -1, 1.0f);

		// SUPERHOT movement
		isPlayerMoving = true;

	}
	else if (cKeyboardController->IsKeyReleased(GLFW_KEY_A))
	{
		// Play the "idleR" animation
		animatedSprites->PlayAnimation("idleL", -1, 1.0f);
	}
	else if (cKeyboardController->IsKeyDown(GLFW_KEY_D))
	{
		// Calculate the new position to the right
		if (vec2Index.x < ((int)cSettings->NUM_TILES_XAXIS))
		{
			vec2NumMicroSteps.x++;
			if (vec2NumMicroSteps.x >= cSettings->NUM_STEPS_PER_TILE_XAXIS)
			{
				vec2NumMicroSteps.x = 0;
				vec2Index.x++;
			}
		}

		// Constraint the player's position within the screen 
		Constraint(RIGHT);

		if (CheckPosition(RIGHT) == false)
		{
			vec2NumMicroSteps.x = 0;
		}

		// Play the "runR" animation
		animatedSprites->PlayAnimation("runR", -1, 1.0f);

		// SUPERHOT movement
		isPlayerMoving = true;
	}
	else if (cKeyboardController->IsKeyReleased(GLFW_KEY_D))
	{
		// Play the "idleL" animation
		animatedSprites->PlayAnimation("idleR", -1, 1.0f);
	}

	if (cKeyboardController->IsKeyDown(GLFW_KEY_W))
	{
		// Calculate the new position up
		if (vec2Index.y < (int)cSettings->NUM_TILES_YAXIS)
		{
			vec2NumMicroSteps.y++;
			if (vec2NumMicroSteps.y > cSettings->NUM_STEPS_PER_TILE_YAXIS)
			{
				vec2NumMicroSteps.y = 0;
				vec2Index.y++;
			}
		}

		// Constraint the player's position within the screen boundary
		Constraint(UP);

		// If the new position is not feasible, then revert to old position
		if (CheckPosition(UP) == false)
		{
			vec2NumMicroSteps.y = 0;
		}

		// SUPERHOT movement
		isPlayerMoving = true;
	}
	else if (cKeyboardController->IsKeyDown(GLFW_KEY_S))
	{
		// Calculate the new position down
		if (vec2Index.y >= 0)
		{
			vec2NumMicroSteps.y--;
			if (vec2NumMicroSteps.y < 0)
			{
				vec2NumMicroSteps.y = ((int)cSettings->NUM_STEPS_PER_TILE_YAXIS) - 1;
				vec2Index.y--;
			}
		}

		// Constraint the player's position within the screen boundary
		Constraint(DOWN);

		// If the new position is not feasible, then revert to old position
		if (CheckPosition(DOWN) == false)
		{
			vec2Index = vec2OldIndex;
			vec2NumMicroSteps.y = 0;
		}

		// SUPERHOT movement
		isPlayerMoving = true;
	}

	// Update the animated sprite
	animatedSprites->Update(dElapsedTime);

	// Update the UV Coordinates
	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false, vec2NumMicroSteps.x*cSettings->MICRO_STEP_XAXIS);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false, vec2NumMicroSteps.y*cSettings->MICRO_STEP_YAXIS);
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CShipPlayer::PreRender(void)
{
	// Activate blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate the shader
	CShaderManager::GetInstance()->Use(sShaderName);
}

/**
 @brief Render this instance
 */
void CShipPlayer::Render(void)
{
	glBindVertexArray(VAO);
	// get matrix's uniform location and set matrix
	unsigned int transformLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "transform");
	unsigned int colorLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "runtimeColour");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

	//transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	//transform = glm::translate(transform, glm::vec3(vec2UVCoordinate.x,
	//												vec2UVCoordinate.y,
	//												0.0f));


	//// Update the shaders with the latest transform
	//glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	//glUniform4fv(colorLoc, 1, glm::value_ptr(runtimeColour));

	//// bind textures on corresponding texture units
	//glActiveTexture(GL_TEXTURE0);
	//// Get the texture to be rendered
	//glBindTexture(GL_TEXTURE_2D, iTextureID);

	////CS: render the tile
	////quadMesh->Render();
	////CS: Render the animated Sprite
	//animatedSprites->Render();

	//glBindTexture(GL_TEXTURE_2D, 0);

	//Get camera transforms and use them instead

	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	glm::vec2 offset = glm::i32vec2(float(cSettings->NUM_TILES_XAXIS / 2.0f), float(cSettings->NUM_TILES_YAXIS / 2.0f));
	glm::vec2 cameraPos = camera2D->getPos();

	glm::vec2 IndexPos = vec2Index;

	glm::vec2 actualPos = IndexPos - cameraPos + offset;
	actualPos = cSettings->ConvertIndexToUVSpace(actualPos) * camera2D->getZoom();
	actualPos.x += vec2NumMicroSteps.x * cSettings->MICRO_STEP_XAXIS;
	actualPos.y += vec2NumMicroSteps.y * cSettings->MICRO_STEP_YAXIS;

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
void CShipPlayer::PostRender(void)
{
	// Disable blending
	glDisable(GL_BLEND);
}

/**
 @brief Constraint the player's position within a boundary
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
void CShipPlayer::Constraint(DIRECTION eDirection)
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
			vec2Index.x = (int)cSettings->NUM_TILES_XAXIS - 1;
			vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == UP)
	{
		if (vec2Index.y >= (int)cSettings->NUM_TILES_YAXIS - 1)
		{
			vec2Index.y = (int)cSettings->NUM_TILES_YAXIS - 1;
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
		cout << "CPlayer2D::Constraint: Unknown Directon." << endl;
	}
}

void CShipPlayer::UpdateHealthLives(void)
{
	// Update health and lives
	cInventoryItem = cInventoryManager->GetItem("Health");
	// Check if a life is lost
	if (cInventoryItem->GetCount() <= 0)
	{
		// Reset the Health to max value
		cInventoryItem->iItemCount = cInventoryItem->GetMaxCount();
		// But we reduce the lives by 1
		cInventoryItem = cInventoryManager->GetItem("Lives");
		cInventoryItem->Remove(1);
		// Respawn player back at last visited checkpoint
		vec2NumMicroSteps.x = 0;
	}
	// Check if there are any lives left
	cInventoryItem = cInventoryManager->GetItem("Lives");
	if (cInventoryItem->GetCount() <= 0)
	{
		// Player loses the game
		CGameManager::GetInstance()->bPlayerLost = true;
	}
}

bool CShipPlayer::CheckPosition(DIRECTION eDirection)
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
				if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 600) ||
					(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 600))
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
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 600) ||
				(cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 600))
			{
				return false;
			}
		}
	}
	else
	{
		cout << "CPlayer2D::CheckPosition: Unknown direction." << endl;
	}

	return true;
}