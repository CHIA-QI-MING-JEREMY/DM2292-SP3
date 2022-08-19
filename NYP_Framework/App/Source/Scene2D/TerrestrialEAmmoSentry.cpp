/**
 EnemyAmmo2D
 @brief A class representing the player object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "TerrestrialEAmmoSentry.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

// Include ImageLoader
#include "System\ImageLoader.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "Map2D.h"
#include "Primitives/MeshBuilder.h"

/**
 @brief Constructor This constructor has public access modifier to be used in player class
 */
CTEAmmoSentry::CTEAmmoSentry()
	: cMap2D(NULL)
	, cKeyboardController(NULL)
	, animatedSprites(NULL)
	, camera2D(NULL)
	, runtimeColour(glm::vec4(1.0f))
{
	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first

	// Initialise vecIndex
	vec2Index = glm::i32vec2(-1, -1);
	// By default, microsteps should be zero
	vec2NumMicroSteps = glm::i32vec2(0, 0);

	// Initialise vec2UVCoordinate
	vec2UVCoordinate = glm::vec2(0.0f);

	hit = false; //ammo hasn't hit anything yet

	active = false; //hm, maybe might not need it

	Init();
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CTEAmmoSentry::~CTEAmmoSentry(void)
{
	// We won't delete this since it was created elsewhere
	cKeyboardController = NULL;

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
}

/**
  @brief Initialise this instance
  */
bool CTEAmmoSentry::Init(void)
{
	// Store the keyboard controller singleton instance here
	cKeyboardController = CKeyboardController::GetInstance();
	// Reset all keys since we are starting a new game
	cKeyboardController->Reset();

	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	// Get the handler to the Camera2D instance
	camera2D = Camera2D::GetInstance();

	// Get the handler to the CSoundController instance
	cSoundController = CSoundController::GetInstance();

	// Get the handler to the CMap2D instance
	cMap2D = CMap2D::GetInstance();

	// Initialise the cInventoryManager
	cInventoryManager = CInventoryManager::GetInstance();

	// Create and initialise the CPlayer2D
	cPlayer2D = CPlayer2D::GetInstance();

	// By default, microsteps should be zero
	vec2NumMicroSteps = glm::i32vec2(0, 0);

	//glGenVertexArrays(1, &VAO);
	//glBindVertexArray(VAO);

	// Create the quad mesh for the ammo
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

	// Load the ammo texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/scene2d_flame.png", true);
	if (iTextureID == 0)
	{
		std::cout << "Failed to load ammo texture" << std::endl;
		return false;
	}

	//CS: Create the animated spirte and setup the animation
	animatedSprites = CMeshBuilder::GenerateSpriteAnimation(1, 7,
		cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	//^ loads a spirte sheet with 3 by 3 diff images, all of equal size and positioning
	animatedSprites->AddAnimation("idle", 0, 6); //7 images for animation, index 0 to 7
	//CS: Play the "idle" animation as default
	animatedSprites->PlayAnimation("idle", -1, 1.0f);
		//-1 --> repeats forever
		//		settng it to say 1 will cause it to only repeat 1 time
		//1.0f --> set time between frames as 1.0f
		//		increasing this number will cause the animation to slowdown

	//Set Physics to fall status by default
	cPhysics2D.Init();
	cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);

	//CS: Init the color to white
	runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);

	return true;
}

/**
 @brief Update this instance
 */
void CTEAmmoSentry::Update(const double dElapsedTime)
{
	// Store the old position
	vec2OldIndex = vec2Index;

	//move
	ShootAmmo();

	hit = false; //reset hit to false

	// Interact with the Map
	InteractWithMap();

	// Interact with the Player
	InteractWithPlayer();

	if (!CheckPosition()) //if hit a wall type obj
	{
		hit = true; //destroy ammo
	}
	
	//CS: Update the animated sprite
	//CS: Play the animation
	animatedSprites->Update(dElapsedTime);

	// Update the UV Coordinates
	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false, vec2NumMicroSteps.x*cSettings->MICRO_STEP_XAXIS);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false, vec2NumMicroSteps.y*cSettings->MICRO_STEP_YAXIS);
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CTEAmmoSentry::PreRender(void)
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
void CTEAmmoSentry::Render(void)
{
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
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CTEAmmoSentry::PostRender(void)
{
	// Disable blending
	glDisable(GL_BLEND);
}

//setting the ammo's information needed for its travel path:
		//player location aka ammo OG location
		//player direction aka direction for amo to move in
void CTEAmmoSentry::setPath(const int spawnX, const int spawnY, const int eDirection)
{
	vec2Index.x = spawnX;
	vec2Index.y = spawnY;
	direction = eDirection;
}

void CTEAmmoSentry::InteractWithMap(void)
{
	switch (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x))
	{
	//full sheild to half shield
	case 150: //player's full shield, down, normal
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 160);
			//change sheild to half damaged sheild
		hit = true; //destory ammo upon collision
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning sound
		break;
	case 151: //player's full shield, up, normal
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 161);
		//change sheild to half damaged sheild
		hit = true; //destory ammo upon collision
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning sound
		break;
	case 152: //player's full shield, left, normal
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 162);
		//change sheild to half damaged sheild
		hit = true; //destory ammo upon collision
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning sound
		break;
	case 153: //player's full shield, right, normal
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 163);
		//change sheild to half damaged sheild
		hit = true; //destory ammo upon collision
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning sound
		break;
	case 154: //player's full shield, down, BnW
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 164);
		//change sheild to half damaged sheild
		hit = true; //destory ammo upon collision
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning sound
		break;
	case 155: //player's full shield, up, BnW
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 165);
		//change sheild to half damaged sheild
		hit = true; //destory ammo upon collision
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning sound
		break;
	case 156: //player's full shield, left, BnW
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 166);
		//change sheild to half damaged sheild
		hit = true; //destory ammo upon collision
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning sound
		break;
	case 157: //player's full shield, right, BnW
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 167);
		//change sheild to half damaged sheild
		hit = true; //destory ammo upon collision
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning sound
		break;
	//half shield to nothing
	case 160: //player's half shield, down, normal
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0);
			//change half damaged sheild to empty space
		hit = true; //destory ammo upon collision
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning sound
		break;
	case 161: //player's half shield, up, normal
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0);
			//change half damaged sheild to empty space
		hit = true; //destory ammo upon collision
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning sound
		break;
	case 162: //player's half shield, left, normal
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0);
			//change half damaged sheild to empty space
		hit = true; //destory ammo upon collision
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning sound
		break;
	case 163: //player's half shield, right, normal
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0);
			//change half damaged sheild to empty space
		hit = true; //destory ammo upon collision
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning sound
		break;
	case 164: //player's half shield, down, BnW
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0);
			//change half damaged sheild to empty space
		hit = true; //destory ammo upon collision
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning sound
		break;
	case 165: //player's half shield, up, BnW
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0);
			//change half damaged sheild to empty space
		hit = true; //destory ammo upon collision
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning sound
		break;
	case 166: //player's half shield, left, BnW
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0);
			//change half damaged sheild to empty space
		hit = true; //destory ammo upon collision
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning sound
		break;
	case 167: //player's half shield, right, BnW
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0);
			//change half damaged sheild to empty space
		hit = true; //destory ammo upon collision
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning sound
		break;
	default:
		break;
	}
}

/**
 @brief Let enemy ammo interact with the player.
 */
bool CTEAmmoSentry::InteractWithPlayer(void)
{
	glm::i32vec2 i32vec2PlayerPos = cPlayer2D->vec2Index;

	// Check if the enemy2D is within 1.5 indices of the player2D
	if (((vec2Index.x >= i32vec2PlayerPos.x - 0.5) &&
		(vec2Index.x <= i32vec2PlayerPos.x + 0.5))
		&&
		((vec2Index.y >= i32vec2PlayerPos.y - 0.5) &&
			(vec2Index.y <= i32vec2PlayerPos.y + 0.5)))
	{
		// Decrease the health by 1
		cInventoryItem = cInventoryManager->GetItem("Health");
		cInventoryItem->Remove(5);
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning noise
		//cout << "Take that!" << endl;
		hit = true; //destory ammo --> only hits player once

		return true;
	}
	return false;
}

bool CTEAmmoSentry::CheckPosition(void)
{
	if (direction == LEFT)
	{
		// If the new position is fully within a row, then check this row only
		if (vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
				//150 to 199 is considered accessable only because they are set to automatically get destoryed upon collision with such index elsewhere
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100 && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) < 150)
				//100 and above in the excel are obj that cannot be walked into
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100 && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) < 150) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100 && cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) < 150))
			{
				return false;
			}
		}
	}
	else if (direction == RIGHT)
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
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100 && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) < 150)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100 && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) < 150) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 100 && cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) < 150))
			{
				return false;
			}
		}
	}
	else if (direction == UP)
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
			if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100 && cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) < 150)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100 && cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) < 150) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 100 && cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) < 150))
			{
				return false;
			}
		}
	}
	else if (direction == DOWN)
	{
		// If the new position is fully within a column, then check this column only
		if (vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100 && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) < 150)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100 && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) < 150) ||
				(cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100 && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) < 150))
			{
				return false;
			}
		}
	}
	else
	{
		cout << "JungleEAmmo2D::CheckPosition: Unknown direction." << endl;
	}
	return true;
}

// Shoot ammo, keep it moving after it is already created, used in player class
void CTEAmmoSentry::ShootAmmo(void)
{
	//can only shoot left, right and up
	if (direction == LEFT)
	{
		vec2NumMicroSteps.x -= 2; //increased to 2 to make ammo faster than player
		if (vec2NumMicroSteps.x < 0)
		{
			vec2NumMicroSteps.x = ((int)cSettings->NUM_STEPS_PER_TILE_XAXIS) - 1;
			vec2Index.x--;
		}
	}
	else if (direction == RIGHT)
	{
		vec2NumMicroSteps.x += 2;
		if (vec2NumMicroSteps.x >= cSettings->NUM_STEPS_PER_TILE_XAXIS)
		{
			vec2NumMicroSteps.x = 0;
			vec2Index.x++;
		}
	}
	else if (direction == UP)
	{
		vec2NumMicroSteps.y += 2;
		if (vec2NumMicroSteps.y > cSettings->NUM_STEPS_PER_TILE_YAXIS)
		{
			vec2NumMicroSteps.y = 0;
			vec2Index.y++;
		}
	}
	else if (direction == DOWN)
	{
		vec2NumMicroSteps.y -= 2;
		if (vec2NumMicroSteps.y < 0)
		{
			vec2NumMicroSteps.y = ((int)cSettings->NUM_STEPS_PER_TILE_YAXIS) - 1;
			vec2Index.y--;
		}
	}
}

// return true if ammo hits window boundaries, used to delete
	//uses ammo specific direction alrdy set in via constructor, used in player class
bool CTEAmmoSentry::LimitReached(void)
{
	//ammo can only be shot left, right and up
	if (direction == LEFT)
	{
		if (vec2Index.x < 0)
		{
			return true;
		}
	}
	else if (direction == RIGHT)
	{
		if (vec2Index.x >= (int)cSettings->NUM_TILES_XAXIS - 1)
		{
			return true;
		}
	}
	else if (direction == UP)
	{
		if (vec2Index.y >= (int)cSettings->NUM_TILES_YAXIS - 1)
		{
			return true;
		}
	}
	else if (direction == DOWN)
	{
		if (vec2Index.y < 0)
		{
			return true;
		}
	}

	if (hit) //ammo hit something, aka a non-world-border limit is reached
	{
		return true;
	}
	return false;
}

//used to set active to render and check collision of ammo
void CTEAmmoSentry::setActive(bool active)
{
	this->active = active;
}

//used to check if ammo is active before checking collision and rendering, etc
bool CTEAmmoSentry::getActive(void)
{
	return active;
}