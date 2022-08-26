/**
 Ammo2D
 @brief A class representing the player object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Ammo2D.h"

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
CAmmo2D::CAmmo2D()
	: cMap2D(NULL)
	, animatedSprites(NULL)
	//, camera2D(NULL)
	, runtimeColour(glm::vec4(1.0f))
{
	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first

	// Initialise vecIndex
	vec2Index = glm::i32vec2(-1, -1);
	// Initialise vecNumMicroSteps
	vec2NumMicroSteps = glm::i32vec2(0);

	// Initialise vec2UVCoordinate
	vec2UVCoordinate = glm::vec2(0.0f);

	hit = false; //ammo hasn't hit anything yet

	active = false; //hm, maybe might not need it

	Init();
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CAmmo2D::~CAmmo2D(void)
{
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
bool CAmmo2D::Init(void)
{
	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	camera2D = Camera2D::GetInstance();

	// Get the handler to the CSoundController instance
	cSoundController = CSoundController::GetInstance();

	// Get the handler to the CMap2D instance
	cMap2D = CMap2D::GetInstance();

	// Create and initialise the CGUI_Scene2D
	cGUI_Scene2D = CGUI_Scene2D::GetInstance();

	// By default, microsteps should be zero
	vec2NumMicroSteps = glm::i32vec2(0, 0);

	//glGenVertexArrays(1, &VAO);
	//glBindVertexArray(VAO);

	// Create the quad mesh for the ammo
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

	switch (cGUI_Scene2D->getPlanetNum())
	{
	case 1:
	{
		// Load the ammo texture
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/scene2d_blue_flame.png", true);
		if (iTextureID == 0)
		{
			std::cout << "Failed to load jungle ammo texture" << std::endl;
			return false;
		}

		//CS: Create the animated spirte and setup the animation
		animatedSprites = CMeshBuilder::GenerateSpriteAnimation(1, 7,
			cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
		//^ loads a spirte sheet with 3 by 3 diff images, all of equal size and positioning
		animatedSprites->AddAnimation("idle", 0, 6); //7 images for animation, index 0 to 7
		break;
	}
	case 2:
	{
		// Load the ammo texture
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/scene2d_blue_flame.png", true);
		if (iTextureID == 0)
		{
			std::cout << "Failed to load the ammo texture" << std::endl;
			return false;
		}

		//CS: Create the animated spirte and setup the animation
		animatedSprites = CMeshBuilder::GenerateSpriteAnimation(1, 7,
			cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
		//^ loads a spirte sheet with 3 by 3 diff images, all of equal size and positioning
		animatedSprites->AddAnimation("idle", 0, 6); //7 images for animation, index 0 to 7
		break;
	}
	case 3:
	{
		// Load the ammo texture
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/SnowPlanet/Icedart.png", true);
		if (iTextureID == 0)
		{
			std::cout << "Failed to load snow ammo texture" << std::endl;
			return false;
		}

		//CS: Create the animated spirte and setup the animation
		animatedSprites = CMeshBuilder::GenerateSpriteAnimation(4, 4,
			cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
		//^ loads a spirte sheet with 3 by 3 diff images, all of equal size and positioning
		animatedSprites->AddAnimation("idleUp", 0, 3); //4 images for animation
		animatedSprites->AddAnimation("idleDown", 4, 7); //4 images for animation
		animatedSprites->AddAnimation("idleRight", 8, 11); //4 images for animation
		animatedSprites->AddAnimation("idleLeft", 12, 15); //4 images for animation
		break;
	}
	default:
	{
		// Load the ammo texture
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/scene2d_blue_flame.png", true);
		if (iTextureID == 0)
		{
			std::cout << "Failed to load jungle ammo texture" << std::endl;
			return false;
		}

		//CS: Create the animated spirte and setup the animation
		animatedSprites = CMeshBuilder::GenerateSpriteAnimation(1, 7,
			cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
		//^ loads a spirte sheet with 3 by 3 diff images, all of equal size and positioning
		animatedSprites->AddAnimation("idle", 0, 6); //7 images for animation, index 0 to 7
		break;
	}
	}

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
void CAmmo2D::Update(const double dElapsedTime)
{
	// Store the old position
	vec2OldIndex = vec2Index;

	//move
	ShootAmmo();

	hit = false; //reset hit to false

	// Interact with the Map
	InteractWithMap();

	if (!CheckPosition()) //if hit a wall type obj
	{
		hit = true; //destroy ammo
	}
	if (cGUI_Scene2D->getPlanetNum() == 3) {
		//update sprite animation to play depending on the direction enemy is facing
		if (direction == LEFT)
		{
			//CS: Play the "left" animation
			animatedSprites->PlayAnimation("idleLeft", -1, 1.0f);
		}
		else if (direction == RIGHT)
		{
			//CS: Play the "right" animation
			animatedSprites->PlayAnimation("idleRight", -1, 1.0f);
		}
		else if (direction == UP)
		{
			//CS: Play the "up" animation
			animatedSprites->PlayAnimation("idleUp", -1, 1.0f);
		}
		else if (direction == DOWN)
		{
			//CS: Play the "down" animation
			animatedSprites->PlayAnimation("idleDown", -1, 1.0f);
		}
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
void CAmmo2D::PreRender(void)
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
void CAmmo2D::Render(void)
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
	actualPos.x += vec2NumMicroSteps.x * cSettings->MICRO_STEP_XAXIS;
	actualPos.y += vec2NumMicroSteps.y * cSettings->MICRO_STEP_YAXIS;

	transform = glm::translate(transform, glm::vec3(actualPos.x, actualPos.y, 0.f));
	transform = glm::scale(transform, glm::vec3(camera2D->getZoom()));

	/*transform = glm::translate(transform, glm::vec3(vec2UVCoordinate.x,
		vec2UVCoordinate.y,
		0.0f));*/

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
void CAmmo2D::PostRender(void)
{
	// Disable blending
	glDisable(GL_BLEND);
}

//setting the ammo's information needed for its travel path:
		//player location aka ammo OG location
		//player direction aka direction for amo to move in
void CAmmo2D::setPath(const int spawnX, const int spawnY, const int eDirection)
{
	vec2Index.x = spawnX;
	vec2Index.y = spawnY;
	direction = eDirection;
}

void CAmmo2D::InteractWithMap(void)
{
	switch (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x))
	{
	// Jungle Planet
	case CMap2D::TILE_INDEX::BURNABLE_BUSH: //burnable bush block
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, CMap2D::TILE_INDEX::BURNING_BUSH); //turn to burning bush
		hit = true;
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning sound
		break;
	case CMap2D::TILE_INDEX::BURNING_BUSH: //burning bush block
	case CMap2D::TILE_INDEX::DISSOLVING_BUSH: //dissolving bush block
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0); //turn to empty space
		hit = true;
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning sound
		break;
	case CMap2D::TILE_INDEX::DOWN_BUTTON_1:
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, CMap2D::TILE_INDEX::DOWN_BUTTON_PRESSED); //deactivate button
		cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::DOWN_BUTTON_1_BLOCKS, 0); //replace blocks with air
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::FLICK_SWITCH); //play button press sound
		break;
	case CMap2D::TILE_INDEX::RIGHT_BUTTON_1:
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, CMap2D::TILE_INDEX::RIGHT_BUTTON_PRESSED); //deactivate button
		cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::RIGHT_BUTTON_1_BLOCKS, 0); //replace blocks with air
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::FLICK_SWITCH); //play button press sound
		break;
	case CMap2D::TILE_INDEX::LEFT_BUTTON_1:
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, CMap2D::TILE_INDEX::LEFT_BUTTON_PRESSED); //deactivate button
		cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::LEFT_BUTTON_1_BLOCKS, 0); //replace blocks with air
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::FLICK_SWITCH); //play button press sound
		break;
	case CMap2D::TILE_INDEX::DOWN_BUTTON_2:
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, CMap2D::TILE_INDEX::DOWN_BUTTON_PRESSED); //deactivate button
		cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::DOWN_BUTTON_2_BLOCKS, 0); //replace blocks with air
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::FLICK_SWITCH); //play button press sound
		break;
	case CMap2D::TILE_INDEX::DOWN_BUTTON_3:
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, CMap2D::TILE_INDEX::DOWN_BUTTON_PRESSED); //deactivate button
		cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::DOWN_BUTTON_3_BLOCKS, 0); //replace blocks with air
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::FLICK_SWITCH); //play button press sound
		break;

	// Terrestrial Planet
	case CMap2D::TILE_INDEX::SWITCH_UP_OFF:
	{
		// change to left switch
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, CMap2D::TILE_INDEX::SWITCH_UP_ON);

		// destroys the bullet
		hit = true;

		// different behaviour for different levels
		if (cMap2D->GetCurrentLevel() == 0)
		{
			// different tiles replaced for different switches
			if (vec2Index == glm::vec2(17, 16))
			{
				cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::BROWN_TILE_HOLLOW, CMap2D::TILE_INDEX::BROWN_TILE_SOLID, 15, 16, 18, 29);
			}
			else if (vec2Index == glm::vec2(7, 3))
			{
				cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::BROWN_TILE_HOLLOW, CMap2D::TILE_INDEX::BROWN_TILE_SOLID, 3, 5, 19, 20);
			}
			else if (vec2Index == glm::vec2(23, 5))
			{
				cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::BROWN_TILE_HOLLOW, CMap2D::TILE_INDEX::BROWN_TILE_SOLID, 7, 8, 20, 22);
			}
		}
		else if (cMap2D->GetCurrentLevel() == 1)
		{
			// different tiles replaced for different switches
			if (vec2Index == glm::vec2(1, 13))
			{
				cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::BROWN_TILE_HOLLOW, CMap2D::TILE_INDEX::BROWN_TILE_SOLID, 12, 13, 5, 15);
			}
		}
		break;
	}
	case CMap2D::TILE_INDEX::SWITCH_UP_ON:
	{
		// change to right switch
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, CMap2D::TILE_INDEX::SWITCH_UP_OFF);

		// destroys the bullet
		hit = true;

		// different behaviour for different levels
		if (cMap2D->GetCurrentLevel() == 0)
		{
			// different tiles replaced for different switches
			if (vec2Index == glm::vec2(17, 16))
			{
				cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::BROWN_TILE_SOLID, CMap2D::TILE_INDEX::BROWN_TILE_HOLLOW, 15, 16, 18, 29);
			}
			else if (vec2Index == glm::vec2(7, 3))
			{
				cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::BROWN_TILE_SOLID, CMap2D::TILE_INDEX::BROWN_TILE_HOLLOW, 3, 5, 19, 20);
			}
			else if (vec2Index == glm::vec2(23, 5))
			{
				cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::BROWN_TILE_SOLID, CMap2D::TILE_INDEX::BROWN_TILE_HOLLOW, 7, 8, 20, 22);
			}
		}
		else if (cMap2D->GetCurrentLevel() == 1)
		{
			// different tiles replaced for different switches
			if (vec2Index == glm::vec2(1, 13))
			{
				cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::BROWN_TILE_SOLID, CMap2D::TILE_INDEX::BROWN_TILE_HOLLOW, 12, 13, 5, 15);
			}
		}
		break;
	}
	case CMap2D::TILE_INDEX::SWITCH_LEFT_OFF:
	{
		// change to left switch
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, CMap2D::TILE_INDEX::SWITCH_LEFT_ON);

		// destroys the bullet
		hit = true;

		// different behaviour for different levels
		if (cMap2D->GetCurrentLevel() == 1)
		{
			// different tiles replaced for different switches
			if (vec2Index == glm::vec2(1, 8))
			{
				cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::BROWN_TILE_HOLLOW, CMap2D::TILE_INDEX::BROWN_TILE_SOLID, 7, 9, 1, 5);
			}
		}
		break;
	}
	case CMap2D::TILE_INDEX::SWITCH_LEFT_ON:
	{
		// change to left switch
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, CMap2D::TILE_INDEX::SWITCH_LEFT_OFF);

		// destroys the bullet
		hit = true;

		// different behaviour for different levels
		if (cMap2D->GetCurrentLevel() == 1)
		{
			// different tiles replaced for different switches
			if (vec2Index == glm::vec2(1, 8))
			{
				cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::BROWN_TILE_SOLID, CMap2D::TILE_INDEX::BROWN_TILE_HOLLOW, 7, 9, 1, 5);
			}
		}
		break;
	}
	default:
		break;
	}
}

bool CAmmo2D::CheckPosition(void)
{
	if (direction == LEFT)
	{
		// If the new position is fully within a row, then check this row only
		if (vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 610) 
				//100 and above in the excel are obj that cannot be walked into
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 610) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 610))
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
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 610 && vec2NumMicroSteps.x >= cSettings->NUM_STEPS_PER_TILE_XAXIS - 2.f)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 610) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 610))
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
			if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 610)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 610) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 610))
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
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 610)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 610) ||
				(cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 610))
			{
				return false;
			}
		}
	}
	else
	{
		cout << "CAmmo2D::CheckPosition: Unknown direction." << endl;
	}
	return true;
}

//let ammo interact with enemies
bool CAmmo2D::InteractWithEnemy(glm::i32vec2 i32vec2EnemyPos)
{
	// Check if the ammo is within 1.5 indices of the enemy
	if (((vec2Index.x >= i32vec2EnemyPos.x - 0.5) &&
		(vec2Index.x <= i32vec2EnemyPos.x + 0.5))
		&&
		((vec2Index.y >= i32vec2EnemyPos.y - 0.5) &&
			(vec2Index.y <= i32vec2EnemyPos.y + 0.5)))
	{
		// Decrease enemy health by 1 done in scene
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning noise
		hit = true; //destroy ammo --> only hits enemy once

		return true;
	}
	return false;
}

// Shoot ammo, keep it moving after it is already created, used in player class
void CAmmo2D::ShootAmmo(void)
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
bool CAmmo2D::LimitReached(void)
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

	//if (hit) //ammo hit something, aka a non-world-border limit is reached
	//{
	//	return true;
	//}
	return hit;
}

//used to set active to render and check collision of ammo
void CAmmo2D::setActive(bool active)
{
	this->active = active;
}

//used to check if ammo is active before checking collision and rendering, etc
bool CAmmo2D::getActive(void)
{
	return active;
}