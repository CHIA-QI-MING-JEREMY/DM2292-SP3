/**
 Ammo2D
 @brief A class representing the player object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Resource.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

// Include ImageLoader
#include "System\ImageLoader.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "Map2D.h"
#include "Primitives/MeshBuilder.h"


#include <stdlib.h>

/**
 @brief Constructor This constructor has public access modifier to be used in player class
 */
CResource::CResource()
	: cMap2D(NULL)
	, cKeyboardController(NULL)
	//, animatedSprites(NULL)
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

	collected = false; //resource hasn't been collected yet

	type = NUM_RESOURCES; //set to max. in init, if at max, means check map to load resource

	Init();
}

CResource::CResource(int type)
	: cMap2D(NULL)
	, cKeyboardController(NULL)
	//, animatedSprites(NULL)
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

	collected = false; //ammo hasn't hit anything yet

	this->type = type; //set which resource is spawned

	Init();
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CResource::~CResource(void)
{
	// We won't delete this since it was created elsewhere
	cKeyboardController = NULL;

	// We won't delete this since it was created elsewhere
	cMap2D = NULL;

	//// Delete the CAnimationSprites
	//if (animatedSprites)
	//{
	//	delete animatedSprites;
	//	animatedSprites = NULL;
	//}

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
}

/**
  @brief Initialise this instance
  */
bool CResource::Init(void)
{
	// Store the keyboard controller singleton instance here
	cKeyboardController = CKeyboardController::GetInstance();
	// Reset all keys since we are starting a new game
	cKeyboardController->Reset();

	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	//camera2D = Camera2D::GetInstance();

	// Get the handler to the CSoundController instance
	cSoundController = CSoundController::GetInstance();

	// Get the handler to the CMap2D instance
	cMap2D = CMap2D::GetInstance();

	// Create and initialise the CPlayer2D
	cPlayer2D = CPlayer2D::GetInstance();

	// Get the handler to the CInventoryManager instance
	cInventoryManagerPlanet = CInventoryManagerPlanet::GetInstance();

	// By default, microsteps should be zero
	vec2NumMicroSteps = glm::i32vec2(0, 0);

	//glGenVertexArrays(1, &VAO);
	//glBindVertexArray(VAO);

	// Create the quad mesh for the resource
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

	//if type is at default, aka type = NUM_RESOURCES, then check map for num resources
	if (type == NUM_RESOURCES)
	{
		// Find the indices for the player in arrMapInfo, and assign it to CStnEnemy2D
		unsigned int uiRow = -1;
		unsigned int uiCol = -1;

		//if found index for default resource
		if (cMap2D->FindValue(1, uiRow, uiCol))
		{
			//random between 2 numbers to set us Scrap metal or battery
			//according to which number type is set to, load which texture
			srand(static_cast<unsigned> (time(0)));
			int randomState = rand() % 100;
			if (randomState < 50)
			{
				type = SCRAP_METAL;
			}
			else
			{
				type = BATTERY;
			}
		}
		//index for ironwood
		else if (cMap2D->FindValue(199, uiRow, uiCol))
		{
			type = IRONWOOD;
		}
		//index for energy quartz
		else if (cMap2D->FindValue(399, uiRow, uiCol))
		{
			type = ENERGY_QUARTZ;
		}
		//index for ice crystal
		else if (cMap2D->FindValue(599, uiRow, uiCol))
		{
			type = ICE_CRYSTAL;
		}

		//Jungle Planet
		//index for bunrable blocks
		else if (cMap2D->FindValue(197, uiRow, uiCol))
		{
			type = BURNABLE_BLOCKS;
		}
		//index for vine
		else if (cMap2D->FindValue(198, uiRow, uiCol))
		{
			type = VINE;
		}

		// index for yellow orb
		else if (cMap2D->FindValue(221, uiRow, uiCol))
		{
			type = YELLOW_ORB;
		}
		// index for red orb
		else if (cMap2D->FindValue(222, uiRow, uiCol))
		{
			type = RED_ORB;
		}
		// index for green orb
		else if (cMap2D->FindValue(223, uiRow, uiCol))
		{
			type = GREEN_ORB;
		}
		// index for blue orb
		else if (cMap2D->FindValue(224, uiRow, uiCol))
		{
			type = BLUE_ORB;
		}
		else
		{
			return false;	// Unable to find the start position of the enemy, so quit this game
		}

		// Erase the value of the player in the arrMapInfo
		cMap2D->SetMapInfo(uiRow, uiCol, 0);

		// Set the start position of the Player to iRow and iCol
		vec2Index = glm::vec2(uiCol, uiRow);
		// By default, microsteps should be zero
		vec2NumMicroSteps = glm::vec2(0, 0);
	}
	//after that use switch case to load in image
		//if it is created with a type, aka dropped by enemy, just immedately use switch case to decide
	switch (type)
	{
	case SCRAP_METAL:
	{
		// Load the scrap metal texture
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/ScrapMetal.tga", true);
		if (iTextureID == 0)
		{
			std::cout << "Failed to load scrap metal texture" << std::endl;
			return false;
		}
		break;
	}
	case BATTERY:
	{
		// Load the battery texture
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Battery.tga", true);
		if (iTextureID == 0)
		{
			std::cout << "Failed to load battery texture" << std::endl;
			return false;
		}
		break;
	}
	case IRONWOOD:
	{
		// Load the ironwood texture
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/JunglePlanet/Ironwood.tga", true);
		if (iTextureID == 0)
		{
			std::cout << "Failed to load ironwood texture" << std::endl;
			return false;
		}
		break;
	}
	case ENERGY_QUARTZ:
	{
		// Load the energy quartz texture
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/TerrestrialPlanet/EnergyQuartz.tga", true);
		if (iTextureID == 0)
		{
			std::cout << "Failed to load energy quartz texture" << std::endl;
			return false;
		}
		break;
	}
	case ICE_CRYSTAL:
	{
		// Load the ice crystal texture
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/SnowPlanet/icecrystal.tga", true);
		if (iTextureID == 0)
		{
			std::cout << "Failed to load ice crystal texture" << std::endl;
			return false;
		}
		break;
	}

	//Jungle Planet
	case BURNABLE_BLOCKS:
	{
		// Load the ironwood texture
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/JunglePlanet/Ironwood.tga", true);
		if (iTextureID == 0)
		{
			std::cout << "Failed to load burnable blocks texture" << std::endl;
			return false;
		}
		break;
	}
	case VINE:
	{
		// Load the ironwood texture
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/JunglePlanet/Ironwood.tga", true);
		if (iTextureID == 0)
		{
			std::cout << "Failed to load vines texture" << std::endl;
			return false;
		}
		break;
	}

	case YELLOW_ORB:
	{
		// Load the yellow orb texture
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_YellowOrb.tga", true);
		if (iTextureID == 0)
		{
			std::cout << "Failed to load yellow orb texture" << std::endl;
			return false;
		}
		break;
	}
	case RED_ORB:
	{
		// Load the red orb texture
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_RedOrb.tga", true);
		if (iTextureID == 0)
		{
			std::cout << "Failed to load red orb texture" << std::endl;
			return false;
		}
		break;
	}
	case GREEN_ORB:
	{
		// Load the green orb texture
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_GreenOrb.tga", true);
		if (iTextureID == 0)
		{
			std::cout << "Failed to load green orb texture" << std::endl;
			return false;
		}
		break;
	}
	case BLUE_ORB:
	{
		// Load the blue orb texture
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_BlueOrb.tga", true);
		if (iTextureID == 0)
		{
			std::cout << "Failed to load blue orb texture" << std::endl;
			return false;
		}
		break;
	}
	default:
	{
		std::cout << "Failed to load resource texture" << std::endl;
		return false;
		break;
	}
	}

	////CS: Create the animated spirte and setup the animation
	//animatedSprites = CMeshBuilder::GenerateSpriteAnimation(1, 7,
	//	cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	////^ loads a spirte sheet with 3 by 3 diff images, all of equal size and positioning
	//animatedSprites->AddAnimation("idle", 0, 6); //7 images for animation, index 0 to 7
	////CS: Play the "idle" animation as default
	//animatedSprites->PlayAnimation("idle", -1, 1.0f);
	//	//-1 --> repeats forever
	//	//		settng it to say 1 will cause it to only repeat 1 time
	//	//1.0f --> set time between frames as 1.0f
	//	//		increasing this number will cause the animation to slowdown

	//Set Physics to fall status by default
	cPhysics2D.Init();
	cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);

	//CS: Init the color to white
	runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);

	direction = DOWN;

	return true;
}

/**
 @brief Update this instance
 */
void CResource::Update(const double dElapsedTime)
{
	// Store the old position
	vec2OldIndex = vec2Index;

	// Check if player is in mid-air, such as walking off a platform
	if (IsMidAir() == true)
	{
		cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
	}
	//fall to land onto a platform if in the air
	UpdateFall(dElapsedTime);

	InteractWithPlayer();


	//hit = false; //reset hit to false

	//

	//if (!CheckPosition()) //if hit a wall type obj
	//{
	//	hit = true; //destroy ammo
	//}
	
	//CS: Update the animated sprite
	//CS: Play the animation
	//animatedSprites->Update(dElapsedTime);

	// Update the UV Coordinates
	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false, vec2NumMicroSteps.x*cSettings->MICRO_STEP_XAXIS);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false, vec2NumMicroSteps.y*cSettings->MICRO_STEP_YAXIS);
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CResource::PreRender(void)
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
void CResource::Render(void)
{
	glBindVertexArray(VAO);
	// get matrix's uniform location and set matrix
	unsigned int transformLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "transform");
	unsigned int colorLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "runtimeColour");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	
	// TO REMOVE LATER
	/*glm::vec2 offset = glm::i32vec2(float(cSettings->NUM_TILES_XAXIS / 2.0f), float(cSettings->NUM_TILES_YAXIS / 2.0f));
	glm::vec2 cameraPos = camera2D->getPos();

	glm::vec2 IndexPos = vec2Index;

	glm::vec2 actualPos = IndexPos - cameraPos + offset;
	actualPos = cSettings->ConvertIndexToUVSpace(actualPos) * camera2D->getZoom();
	actualPos.x += vec2NumMicroSteps.x * cSettings->MICRO_STEP_XAXIS;
	actualPos.y += vec2NumMicroSteps.y * cSettings->MICRO_STEP_YAXIS;

	transform = glm::translate(transform, glm::vec3(actualPos.x, actualPos.y, 0.f));
	transform = glm::scale(transform, glm::vec3(camera2D->getZoom()));*/

	transform = glm::translate(transform, glm::vec3(vec2UVCoordinate.x,
		vec2UVCoordinate.y,
		0.0f));

	// Update the shaders with the latest transform
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glUniform4fv(colorLoc, 1, glm::value_ptr(runtimeColour));

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	// Get the texture to be rendered
	glBindTexture(GL_TEXTURE_2D, iTextureID);

	//CS: render the tile
	quadMesh->Render();

	glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CResource::PostRender(void)
{
	// Disable blending
	glDisable(GL_BLEND);
}

//sets its spawn location, including microsteps
		//will fall to land on a platform in update where updatefall is called
void CResource::setPosition(glm::vec2 indexPos, glm::vec2 microStep)
{
	vec2Index = indexPos;
	vec2NumMicroSteps = microStep;
	return;
}

bool CResource::IsMidAir(void)
{
	// if the resource is at the bottom row, then he is not in mid-air for sure
	if (vec2Index.y == 0)
	{
		return false;
	}

	// Check if the tile below the resource's current position is empty
	if ((vec2NumMicroSteps.x == 0) && 
		(cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) < 600)) //fall if is not a collision obstruction block
	{
		return true;
	}

	//if resource is standing between 2 tiles which are both not obsuctrtion blocks
	if ((cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) < 600) && (cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x + 1) < 600))
	{
		return true;
	}
	return false;
}

//let player collect resource
bool CResource::InteractWithPlayer(void)
{
	glm::i32vec2 i32vec2PlayerPos = cPlayer2D->vec2Index;

	// Check if the enemy2D is within 1.5 indices of the player2D
	if (((vec2Index.x >= i32vec2PlayerPos.x - 0.5) &&
		(vec2Index.x <= i32vec2PlayerPos.x + 0.5))
		&&
		((vec2Index.y >= i32vec2PlayerPos.y - 0.5) &&
			(vec2Index.y <= i32vec2PlayerPos.y + 0.5)))
	{
		switch (type)
		{
		case SCRAP_METAL:
		{
			std::cout << "COLLECTED SCRAP METAL" << std::endl;
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Resources");
			//check if player's planet inventory total count is max or not
				//if not at max, add in scrap metal and + 1 to resources
			if (cInventoryItemPlanet->GetCount() != cInventoryItemPlanet->GetMaxCount())
			{
				cInventoryItemPlanet->Add(1); //add 1 to the 5 limited space for resources

				cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("ScrapMetal");
				cInventoryItemPlanet->Add(1); //add 1 scrap metal
				collected = true; //delete in scene
			}
			std::cout << cInventoryItemPlanet->GetCount() << std::endl;
			break;
		}
		case BATTERY:
		{
			// Load the battery texture
			std::cout << "COLLECTED BATTERY" << std::endl;
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Resources");
			//check if player's planet inventory total count is max or not
				//if not at max, add in battery and + 1 to resources
			if (cInventoryItemPlanet->GetCount() != cInventoryItemPlanet->GetMaxCount())
			{
				cInventoryItemPlanet->Add(1); //add 1 to the 5 limited space for resources

				cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Battery");
				cInventoryItemPlanet->Add(1); //add 1 battery
				collected = true; //delete in scene
			}
			std::cout << cInventoryItemPlanet->GetCount() << std::endl;
			break;
		}
		case IRONWOOD:
		{
			std::cout << "COLLECTED IRONWOOD" << std::endl;
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Resources");
			//check if player's planet inventory total count is max or not
				//if not at max, add in ironwood and + 1 to resources
			if (cInventoryItemPlanet->GetCount() != cInventoryItemPlanet->GetMaxCount())
			{
				cInventoryItemPlanet->Add(1); //add 1 to the 5 limited space for resources

				cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Ironwood");
				cInventoryItemPlanet->Add(1); //add 1 ironwood
				collected = true; //delete in scene
			}
			std::cout << cInventoryItemPlanet->GetCount() << std::endl;
			break;
		}
		case ENERGY_QUARTZ:
		{
			std::cout << "COLLECTED ENERGY_QUARTZ" << std::endl;
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Resources");
			//check if player's planet inventory total count is max or not
				//if not at max, add in energy quartz and + 1 to resources
			if (cInventoryItemPlanet->GetCount() != cInventoryItemPlanet->GetMaxCount())
			{
				cInventoryItemPlanet->Add(1); //add 1 to the 5 limited space for resources

				cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("EnergyQuartz");
				cInventoryItemPlanet->Add(1); //add 1 energy quartz
				collected = true; //delete in scene
			}
			std::cout << cInventoryItemPlanet->GetCount() << std::endl;
			break;
		}
		case ICE_CRYSTAL:
		{
			std::cout << "COLLECTED ICE_CRYSTAL" << std::endl;
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Resources");
			//check if player's planet inventory total count is max or not
				//if not at max, add in ice crystal and + 1 to resources
			if (cInventoryItemPlanet->GetCount() != cInventoryItemPlanet->GetMaxCount())
			{
				cInventoryItemPlanet->Add(1); //add 1 to the 5 limited space for resources

				cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("IceCrystal");
				cInventoryItemPlanet->Add(1); //add 1 ice crystal
				collected = true; //delete in scene
			}
			std::cout << cInventoryItemPlanet->GetCount() << std::endl;
			break;
		}

		//Jungle Planet
		case BURNABLE_BLOCKS:
		{
			std::cout << "COLLECTED BURNABLE_BLOCKS" << std::endl;

			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("BurnableBlocks");
			cInventoryItemPlanet->Add(5); //add 5 burnable blocks --> comes in a bundle
			collected = true; //delete in scene

			std::cout << cInventoryItemPlanet->GetCount() << std::endl;
			break;
		}
		case VINE:
		{
			std::cout << "COLLECTED VINE" << std::endl;

			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Vine");
			cInventoryItemPlanet->Add(1); //add 1 vine
			collected = true; //delete in scene

			std::cout << cInventoryItemPlanet->GetCount() << std::endl;
			break;
		}

		//T Planet
		case YELLOW_ORB:
		{
			std::cout << "COLLECTED YELLOW_ORB" << std::endl;

			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("YellowOrb");
			cInventoryItemPlanet->Add(1); //add 1 yellow orb
			collected = true; //delete in scene
			
			std::cout << cInventoryItemPlanet->GetCount() << std::endl;
			break;
		}
		case RED_ORB:
		{
			std::cout << "COLLECTED RED_ORB" << std::endl;

			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("RedOrb");
			cInventoryItemPlanet->Add(1); //add 1 red orb
			collected = true; //delete in scene

			std::cout << cInventoryItemPlanet->GetCount() << std::endl;
			break;
		}
		case GREEN_ORB:
		{
			std::cout << "COLLECTED GREEN_ORB" << std::endl;

			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("GreenOrb");
			cInventoryItemPlanet->Add(1); //add 1 green orb
			collected = true; //delete in scene

			std::cout << cInventoryItemPlanet->GetCount() << std::endl;
			break;
		}
		case BLUE_ORB:
		{
			std::cout << "COLLECTED BLUE_ORB" << std::endl;

			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("BlueOrb");
			cInventoryItemPlanet->Add(1); //add 1 blue orb
			collected = true; //delete in scene

			std::cout << cInventoryItemPlanet->GetCount() << std::endl;
			break;
		}
		default:
			break;
		}

		return true;
	}
	return false;
}

bool CResource::CheckPosition(void)
{
	if (direction == LEFT)
	{
		// If the new position is fully within a row, then check this row only
		if (vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 600) 
				//100 and above in the excel are obj that cannot be walked into
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
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 600)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 600) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 600))
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
	else if (direction == DOWN)
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
		cout << "CAmmo2D::CheckPosition: Unknown direction." << endl;
	}
	return true;
}

// return true if ammo hits window boundaries, used to delete
	//uses ammo specific direction alrdy set in via constructor, used in player class
bool CResource::LimitReached(void)
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
	return collected;
}

void CResource::UpdateFall(const double dElapsedTime)
{
	if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::FALL)
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
		int iDisplacement_Microsteps = (int)(v2Displacement.y / cSettings->MICRO_STEP_YAXIS);

		if (vec2Index.y >= 0)
		{
			vec2NumMicroSteps.y -= fabs(iDisplacement_Microsteps);
			if (vec2NumMicroSteps.y < 0)
			{
				vec2NumMicroSteps.y = ((int)cSettings->NUM_STEPS_PER_TILE_YAXIS) - 1;
				vec2Index.y--;
			}
		}

		// Iterate through all rows until the proposed row
		// Check if the player will hit a tile, stop jump if so
		int iIndex_YAxis_Proposed = vec2Index.y;
		for (int i = iIndex_YAxis_OLD; i >= iIndex_YAxis_Proposed; i--)
		{
			// Change the player's index to the current i value
			vec2Index.y = i;
			// If the new position is not feasible, then revert to old position
			if (CheckPosition() == false)
			{
				// Revert to the previous position
				if (i != iIndex_YAxis_OLD)
				{
					vec2Index.y = i + 1;
				}
				// Set the Physics to idle status
				cPhysics2D.SetStatus(CPhysics2D::STATUS::IDLE);
				vec2NumMicroSteps.y = 0;
				break;
			}
		}
	}
}

//return collected
		//if true means delete resource
bool CResource::getCollected(void)
{
	return collected;
}
