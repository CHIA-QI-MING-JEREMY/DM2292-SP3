/**
 Player2D
 @brief A class representing the player object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Player2D.h"

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
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CPlayer2D::CPlayer2D(void)
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
CPlayer2D::~CPlayer2D(void)
{
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
bool CPlayer2D::Init(void)
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
	if (cMap2D->FindValue(200, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the player in the arrMapInfo
	cMap2D->SetMapInfo(uiRow, uiCol, 0);

	if (cMap2D->FindValue(200, uiRow, uiCol) == true)
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
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

	// Load the player texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_Skeleton.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_Skeleton.png" << endl;
		return false;
	}

	// Create the animated sprite and setup the animation
	animatedSprites = CMeshBuilder::GenerateSpriteAnimation(6, 18, cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	animatedSprites->AddAnimation("idleR", 0, 10);
	animatedSprites->AddAnimation("idleL", 18, 28);
	animatedSprites->AddAnimation("runR", 36, 48);
	animatedSprites->AddAnimation("runL", 54, 66);
	animatedSprites->AddAnimation("attackR", 72, 89);
	animatedSprites->AddAnimation("attackL", 90, 107);

	// Play idle animation as default
	animatedSprites->PlayAnimation("idleR", -1, 1.0f);

	// Set the Physics to fall status by default
	cPhysics2D.Init();
	cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
	iJumpCount = 0;

	//Construct 100 inactive ammo and add into ammoList
	for (int i = 0; i < 100; ++i)
	{
		CAmmo2D* cAmmo2D = new CAmmo2D();
		cAmmo2D->SetShader("Shader2D");
		ammoList.push_back(cAmmo2D);
	}
	shootingDirection = LEFT; //by default

	//CS: Init the color to white
	SetColour(WHITE);

	// Get the handler to the CInventoryManager instance
	cInventoryManager = CInventoryManager::GetInstance();
	// Add a lives icon as one of the inventory items
	cInventoryItem = cInventoryManager->Add("Lives", "Image/Scene2D_Lives.tga", 3, 3);
	cInventoryItem->vec2Size = glm::vec2(25, 25);

	// Add a health icon as one of the inventory items
	cInventoryItem = cInventoryManager->Add("Health", "Image/Scene2D_Health.tga", 100, 100);
	cInventoryItem->vec2Size = glm::vec2(25, 25);

	// Load the sounds into CSoundController
	cSoundController = CSoundController::GetInstance();

	// variables
	onRope = false;
	
	isYellowObtained = false;
	isYellowUsed = false;
	isRedObtained = false;
	isRedUsed = false;
	isGreenObtained = false;
	isGreenUsed = false;
	isBlueObtained = false;
	isBlueUsed = false;

	vec2CPIndex = vec2Index;

	isPlayerMoving = false;

	isAttacking = false;
	attackDirection = RIGHT;
	maxAttackTimer = 1.25;
	attackTimer = 0.0;

	// sounds
	hasLanded = false;

	return true;
}

/**
 @brief Reset this instance
 */
bool CPlayer2D::Reset()
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

	//CS: Reset double jump
	iJumpCount = 0;

	return true;
}

/**
 @brief Update this instance
 */
void CPlayer2D::Update(const double dElapsedTime)
{
	// SUPERHOT movement
	if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::IDLE)
	{
		isPlayerMoving = false;
	}
	
	// Store the old position
	vec2OldIndex = vec2Index;

	// Get keyboard updates
	if (cKeyboardController->IsKeyPressed(GLFW_KEY_1))
	{
		if (isYellowObtained && !isYellowUsed)
		{
			cMap2D->ReplaceTiles(101, 1); // allow player to walk through yellow tiles
			cMap2D->ReplaceTiles(2, 102); // dont allow player to walk through red tiles
			cMap2D->ReplaceTiles(3, 103); // dont allow player to walk through green tiles
			cMap2D->ReplaceTiles(4, 104); // dont allow player to walk through blue tiles

			SetColour(YELLOW); // change player colour to yellow

			isYellowUsed = true; // consumes yellow charge
			cInventoryItem = cInventoryManager->GetItem("YellowOrb");
			cInventoryItem->Remove(1); // subtract 1 from yellow orb counter
		}
	}
	if (cKeyboardController->IsKeyPressed(GLFW_KEY_2))
	{
		if (isRedObtained && !isRedUsed)
		{
			cMap2D->ReplaceTiles(1, 101); // dont allow player to walk through yellow tiles
			cMap2D->ReplaceTiles(102, 2); // allow player to walk through red tiles
			cMap2D->ReplaceTiles(3, 103); // dont allow player to walk through green tiles
			cMap2D->ReplaceTiles(4, 104); // dont allow player to walk through blue tiles

			SetColour(RED); // change player colour to red

			isRedUsed = true; // consumes red charge
			cInventoryItem = cInventoryManager->GetItem("RedOrb");
			cInventoryItem->Remove(1); // subtract 1 from red orb counter
		}
	}
	if (cKeyboardController->IsKeyPressed(GLFW_KEY_3))
	{
		if (isGreenObtained && !isGreenUsed)
		{
			cMap2D->ReplaceTiles(1, 101); // dont allow player to walk through yellow tiles
			cMap2D->ReplaceTiles(2, 102); // dont allow player to walk through red tiles
			cMap2D->ReplaceTiles(103, 3); // allow player to walk through green tiles
			cMap2D->ReplaceTiles(4, 104); // dont allow player to walk through blue tiles

			SetColour(GREEN); // change player colour to green

			isGreenUsed = true; // consumes green charge
			cInventoryItem = cInventoryManager->GetItem("GreenOrb");
			cInventoryItem->Remove(1); // subtract 1 from green orb counter
		}
	}
	if (cKeyboardController->IsKeyPressed(GLFW_KEY_4))
	{
		if (isBlueObtained && !isBlueUsed)
		{
			cMap2D->ReplaceTiles(1, 101); // dont allow player to walk through yellow tiles
			cMap2D->ReplaceTiles(2, 102); // dont allow player to walk through red tiles
			cMap2D->ReplaceTiles(3, 103); // dont allow player to walk through green tiles
			cMap2D->ReplaceTiles(104, 4); // allow player to walk through blue tiles

			SetColour(BLUE); // change player colour to blue

			isBlueUsed = true; // consumes blue charge
			cInventoryItem = cInventoryManager->GetItem("BlueOrb");
			cInventoryItem->Remove(1); // subtract 1 from blue orb counter
		}
	}
	
	if (cKeyboardController->IsKeyDown(GLFW_KEY_A))
	{
		// Calculate the new position to the left
		if (vec2Index.x >= 0)
		{
			vec2NumMicroSteps.x--;
			if (vec2NumMicroSteps.x < 0)
			{
				vec2NumMicroSteps.x = ((int)cSettings->NUM_STEPS_PER_TILE_XAXIS) - 1;
				vec2Index.x--;
			}
		}

		// Constraint the player's position within the screen 
		Constraint(LEFT);

		if (CheckPosition(LEFT) == false)
		{
			vec2Index = vec2OldIndex;
			vec2NumMicroSteps.x = 0;
		}
		
		// Play the "runL" animation
		animatedSprites->PlayAnimation("runL", -1, 1.0f);
		shootingDirection = LEFT; //set direction for shooting ammo

		// SUPERHOT movement
		isPlayerMoving = true;

		// Attack Direction
		attackDirection = LEFT;
	}
	else if (cKeyboardController->IsKeyReleased(GLFW_KEY_A))
	{
		// Play the "idleL" animation
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
		shootingDirection = RIGHT; //setting direction for ammo shooting

		// SUPERHOT movement
		isPlayerMoving = true;

		// Attack Direction
		attackDirection = RIGHT;
	}
	else if (cKeyboardController->IsKeyReleased(GLFW_KEY_D))
	{
		// Play the "idleR" animation
		animatedSprites->PlayAnimation("idleR", -1, 1.0f);
	}

	if (cKeyboardController->IsKeyDown(GLFW_KEY_W))
	{
		if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == 13 || cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == 16)
		{
			// Calculate the new position to the up
			if (vec2Index.y < ((int)cSettings->NUM_TILES_YAXIS))
			{
				vec2NumMicroSteps.y++;
				if (vec2NumMicroSteps.y > cSettings->NUM_STEPS_PER_TILE_YAXIS)
				{
					vec2NumMicroSteps.y = 0;
					vec2Index.y++;
				}
			}
			onRope = true;

			// Constraint the player's position within the screen 
			Constraint(UP);

			if (CheckPosition(UP) == false)
			{
				vec2NumMicroSteps.y = 0;
			}
		}

		// SUPERHOT movement
		isPlayerMoving = true;
		shootingDirection = UP; //setting direction for ammo shooting
	}
	else if (cKeyboardController->IsKeyDown(GLFW_KEY_S))
	{
		if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == 12 || cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == 13
			|| cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == 15 || cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == 16)
		{
			// Calculate the new position to the down
			if (vec2Index.y >= 0)
			{
				vec2NumMicroSteps.y--;
				if (vec2NumMicroSteps.y < 0)
				{
					vec2NumMicroSteps.y = ((int)cSettings->NUM_STEPS_PER_TILE_YAXIS) - 1;
					vec2Index.y--;
				}
			}
			onRope = true;

			// Constraint the player's position within the screen 
			Constraint(DOWN);

			if (CheckPosition(DOWN) == false)
			{
				vec2Index = vec2OldIndex;
				vec2NumMicroSteps.y = 0;
			}

			
		}

		// SUPERHOT movement
		isPlayerMoving = true;
		shootingDirection = DOWN; //setting direction for ammo shooting
	}
	
	// TO DO: Solve animation issue
	if (cKeyboardController->IsKeyPressed(GLFW_KEY_Q) && attackTimer <= 0.0)
	{
		if (attackDirection == RIGHT)
		{
			// Play the "attackR" animation
			animatedSprites->PlayAnimation("attackR", -1, 1.5f);
		}
		else if (attackDirection == LEFT)
		{
			// Play the "attackL" animation
			animatedSprites->PlayAnimation("attackL", -1, 1.5f);
		}
		isAttacking = true;
		attackTimer = maxAttackTimer;
	}
	if (attackTimer > 0.0)
	{
		attackTimer -= dElapsedTime;
	}

	if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 12 && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 13
		&& cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 15 && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 16)
	{
		onRope = false;
	}

	if (cKeyboardController->IsKeyPressed(GLFW_KEY_SPACE) && onRope == false)
	{
		if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::IDLE)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::JUMP);

			if (!isAlarmOn)
			{
				cPhysics2D.SetInitialVelocity(glm::vec2(0.0f, 3.5f)); // jump height: 2.5f -> 4 tiles for 64/48, 3.5 -> 4 tiles for 32/24
			}
			iJumpCount += 1;

			// Play a sound for jump
			cSoundController->PlaySoundByID(2);
		}
	}

	// resets player location at last visited checkpoint
	if (cKeyboardController->IsKeyPressed(GLFW_KEY_R))
	{
		vec2Index = vec2CPIndex;
		vec2NumMicroSteps.x = 0;

		// reduce the lives by 1
		cInventoryItem = cInventoryManager->GetItem("Lives");
		cInventoryItem->Remove(1);
	}

	//create ammo
	if (cKeyboardController->IsKeyReleased(GLFW_KEY_E))
	{
		CAmmo2D* ammo = FetchAmmo();
		ammo->setActive(true);
		ammo->setPath(vec2Index.x, vec2Index.y, shootingDirection);
	}

	//ammo beahviour
	for (std::vector<CAmmo2D*>::iterator it = ammoList.begin(); it != ammoList.end(); ++it)
	{
		CAmmo2D* ammo = (CAmmo2D*)*it;
		if (ammo->getActive())
		{
			ammo->Update(dElapsedTime);
			if (ammo->LimitReached())
			{
				ammo->setActive(false);
			}
		}
	}

	// Check if player is in mid-air, such as walking off a platform
	if (IsMidAir() == true)
	{
		if (cPhysics2D.GetStatus() != CPhysics2D::STATUS::JUMP)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}
	}

	UpdateJumpFall(dElapsedTime);

	// Sounds
	

	// Update health and lives
	UpdateHealthLives();

	// Interact with the Map
	InteractWithMap();

	// Update the animated sprite
	animatedSprites->Update(dElapsedTime);

	// Update the UV Coordinates
	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false, vec2NumMicroSteps.x*cSettings->MICRO_STEP_XAXIS);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false, vec2NumMicroSteps.y*cSettings->MICRO_STEP_YAXIS);
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CPlayer2D::PreRender(void)
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
void CPlayer2D::Render(void)
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
void CPlayer2D::PostRender(void)
{
	// Disable blending
	glDisable(GL_BLEND);
}

//return ammolist to the scene for pre, post and normal rendering
std::vector<CAmmo2D*> CPlayer2D::getAmmoList(void)
{
	return ammoList;
}

/**
 @brief Constraint the player's position within a boundary
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
void CPlayer2D::Constraint(DIRECTION eDirection)
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

void CPlayer2D::InteractWithMap(void)
{
	switch (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x))
	{
	case 10:
		// runs if there is empty space on the left of the post
		if (cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x - 1) == 0)
		{
			// changes post with coiled rope to post with uncoiled rope left
			cMap2D->ReplaceTiles(10, 11, vec2Index.y, vec2Index.y + 1, vec2Index.x, vec2Index.x + 1);
			// adds rope for player to climb up
			// checks if there is any ground below the rope length
			unsigned int groundHeight = cMap2D->FindGround(vec2Index.y, vec2Index.x - 1);
			if (groundHeight < 0)
			{
				cout << "There is a hole in the ground at column " << vec2Index.x - 1 << endl;
			}
			else
			{
				cMap2D->ReplaceTiles(0, 12, vec2Index.y, vec2Index.y + 1, vec2Index.x - 1, vec2Index.x);
				cMap2D->ReplaceTiles(0, 13, groundHeight, vec2Index.y, vec2Index.x - 1, vec2Index.x);
			}
		}
		// runs if there is empty space on the right of the post
		else if (cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x + 1) == 0)
		{
			// changes post with coiled rope to post with uncoiled rope right
			cMap2D->ReplaceTiles(10, 14, vec2Index.y, vec2Index.y + 1, vec2Index.x, vec2Index.x + 1);
			// adds rope for player to climb up
			// checks if there is any ground below the rope length
			unsigned int groundHeight = cMap2D->FindGround(vec2Index.y, vec2Index.x + 1);
			if (groundHeight < 0)
			{
				cout << "There is a hole in the ground at column " << vec2Index.x + 1 << endl;
			}
			else
			{
				cMap2D->ReplaceTiles(0, 15, vec2Index.y, vec2Index.y + 1, vec2Index.x + 1, vec2Index.x + 2);
				cMap2D->ReplaceTiles(0, 16, groundHeight, vec2Index.y, vec2Index.x + 1, vec2Index.x + 2);
			}
		}
		break;
	case 21:
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0); // destroy yellow orb
		isYellowObtained = true; // allow player to switch to yellow
		cInventoryItem = cInventoryManager->GetItem("YellowOrb");
		cInventoryItem->Add(1); // adds 1 to yellow orb counter
		break;
	case 22:
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0); // destroy red orb
		isRedObtained = true; // allow player to switch to red
		cInventoryItem = cInventoryManager->GetItem("RedOrb");
		cInventoryItem->Add(1); // adds 1 to red orb counter
		break;
	case 23:
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0); // destroy green orb
		isGreenObtained = true; // allow player to switch to green
		cInventoryItem = cInventoryManager->GetItem("GreenOrb");
		cInventoryItem->Add(1); // adds 1 to green orb counter
		break;
	case 24:
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0); // destroy blue orb
		isBlueObtained = true; // allow player to switch to blue
		cInventoryItem = cInventoryManager->GetItem("BlueOrb");
		cInventoryItem->Add(1); // adds 1 to blue orb counter
		break;
	case 30:
		// change red flags (if any) to black flags
		cMap2D->ReplaceTiles(31, 30);
		// change current flag to red flag
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 31);
		
		// sets CPIndex to checkpoint player just visited
		vec2CPIndex = vec2Index;

		// if yellow orb has been obtained and yellow charge has been consumed
		if (isYellowObtained && isYellowUsed)
		{
			isYellowUsed = false; // restore yellow charge
			cInventoryItem = cInventoryManager->GetItem("YellowOrb");
			cInventoryItem->Add(1); // adds 1 to yellow orb counter
		}
		// if red orb has been obtained and red charge has been consumed
		if (isRedObtained && isRedUsed)
		{
			isRedUsed = false; // restore red charge
			cInventoryItem = cInventoryManager->GetItem("RedOrb");
			cInventoryItem->Add(1); // adds 1 to red orb counter
		}
		// if green orb has been obtained and green charge has been consumed
		if (isGreenObtained && isGreenUsed)
		{
			isGreenUsed = false; // restore green charge
			cInventoryItem = cInventoryManager->GetItem("GreenOrb");
			cInventoryItem->Add(1); // adds 1 to green orb counter
		}
		// if blue orb has been obtained and blue charge has been consumed
		if (isBlueObtained && isBlueUsed)
		{
			isBlueUsed = false; // restore blue charge
			cInventoryItem = cInventoryManager->GetItem("BlueOrb");
			cInventoryItem->Add(1); // adds 1 to blue orb counter
		}

		// restores player health
		cInventoryItem = cInventoryManager->GetItem("Health");
		cInventoryItem->iItemCount = cInventoryItem->GetMaxCount();
		break;
	case 31:
		// sets CPIndex to checkpoint player just visited
		vec2CPIndex = vec2Index;

		// if yellow orb has been obtained and yellow charge has been consumed
		if (isYellowObtained && isYellowUsed)
		{
			isYellowUsed = false; // restore yellow charge
			cInventoryItem = cInventoryManager->GetItem("YellowOrb");
			cInventoryItem->Add(1); // adds 1 to yellow orb counter
		}
		// if red orb has been obtained and red charge has been consumed
		if (isRedObtained && isRedUsed)
		{
			isRedUsed = false; // restore red charge
			cInventoryItem = cInventoryManager->GetItem("RedOrb");
			cInventoryItem->Add(1); // adds 1 to red orb counter
		}
		// if green orb has been obtained and green charge has been consumed
		if (isGreenObtained && isGreenUsed)
		{
			isGreenUsed = false; // restore green charge
			cInventoryItem = cInventoryManager->GetItem("GreenOrb");
			cInventoryItem->Add(1); // adds 1 to green orb counter
		}
		// if blue orb has been obtained and blue charge has been consumed
		if (isBlueObtained && isBlueUsed)
		{
			isBlueUsed = false; // restore blue charge
			cInventoryItem = cInventoryManager->GetItem("BlueOrb");
			cInventoryItem->Add(1); // adds 1 to blue orb counter
		}

		// restores player health
		cInventoryItem = cInventoryManager->GetItem("Health");
		cInventoryItem->iItemCount = cInventoryItem->GetMaxCount();
		break;
	case 40:
		// decrease health by 1
		cInventoryItem = cInventoryManager->GetItem("Health");
		cInventoryItem->Remove(1);
		break;
	case 41:
		// decrease health by 1
		cInventoryItem = cInventoryManager->GetItem("Health");
		cInventoryItem->Remove(1);
		break;
	case 42:
		// decrease health by 1
		cInventoryItem = cInventoryManager->GetItem("Health");
		cInventoryItem->Remove(1);
		break;
	case 43:
		// decrease health by 1
		cInventoryItem = cInventoryManager->GetItem("Health");
		cInventoryItem->Remove(1);
		break;
	case 99:
		// Game has been completed
		CGameManager::GetInstance()->bPlayerWon = true;
		break;
	default:
		break;
	}
}

void CPlayer2D::UpdateHealthLives(void)
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
		vec2Index = vec2CPIndex;
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

void CPlayer2D::UpdateJumpFall(const double dElapsedTime)
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

		// Translate the displacement from pixels to indices
		int iDisplacement_Microsteps = (int)(v2Displacement.y / cSettings->MICRO_STEP_YAXIS);
		if (vec2Index.y < (int)cSettings->NUM_TILES_YAXIS)
		{
			vec2NumMicroSteps.y += iDisplacement_Microsteps;
			if (vec2NumMicroSteps.y > cSettings->NUM_STEPS_PER_TILE_YAXIS)
			{
				vec2NumMicroSteps.y -= cSettings->NUM_STEPS_PER_TILE_YAXIS;
				if (vec2NumMicroSteps.y < 0)
				{
					vec2NumMicroSteps.y = 0;
				}
				vec2Index.y++;
			}
		}

		// Constraint the player's position within the screen boundary
		Constraint(UP);

		// Iterate through all rows until the proposed row
		// Check if the player will hit a tile, stop jump if so
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

		if ((cPhysics2D.GetStatus() == CPhysics2D::STATUS::JUMP) && (onRope == true))
		{
			// Set status to idle
			cPhysics2D.SetStatus(CPhysics2D::STATUS::IDLE);
		}

		// If the player is still jumping and the initial velocity has reached 0 or below 0
		// then it has reached the peak of its jump
		if ((cPhysics2D.GetStatus() == CPhysics2D::STATUS::JUMP) && (cPhysics2D.GetDisplacement().y <= 0.0f))
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

		// Constraint the player's position within the screen boundary
		Constraint(DOWN);

		// Iterate through all rows until the proposed row
		// Check if the player will hit a tile, stop jump if so
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
				{
					vec2Index.y = i + 1;
				}
				// Set the Physics to idle status
				cPhysics2D.SetStatus(CPhysics2D::STATUS::IDLE);
				iJumpCount = 0;
				vec2NumMicroSteps.y = 0;
				cSoundController->PlaySoundByID(1); // plays thump sound when player hits the floor
				hasLanded = true;
				break;
			}
		}
	}
}

bool CPlayer2D::CheckPosition(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		// If the new position is fully within a row, then check this row only
		if (vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (vec2NumMicroSteps.y != 0)
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
			vec2NumMicroSteps.x = 0;
			return true;
		}
		// If the new position is fully within a row, then check this row only
		if (vec2NumMicroSteps.y == 0)
		{
			if (vec2NumMicroSteps.x != 0)
			{
				// If the grid is not accessible, then return false
				if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100)
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
				if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100) ||
					(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 100))
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
			if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (vec2NumMicroSteps.x != 0)
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
		if (vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (vec2NumMicroSteps.x != 0)
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
		cout << "CPlayer2D::CheckPosition: Unknown direction." << endl;
	}

	return true;
}

bool CPlayer2D::IsMidAir(void)
{
	// if the player is at the bottom row, then he is not in mid-air for sure
	if (vec2Index.y == 0)
	{
		return false;
	}

	// Check if the tile below the player is a coloured collectible tile
	if (cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) == 1
		|| cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) == 2
		|| cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) == 3
		|| cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) == 4)
	{
		return true;
	}

	// Check if the tile below the player's current position is empty
	if (vec2NumMicroSteps.x == 0 &&
		(cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) == 0))
	{
		return true;
	}

	// Check if the player is floating between 2 tiles when not on rope
	if (vec2NumMicroSteps.y != 0 && (onRope == false))
	{
		return true;
	}

	return false;
}

void CPlayer2D::SetColour(COLOUR colour)
{
	switch (colour)
	{
	case 0:
		runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0); // WHITE
		break;
	case 1:
		runtimeColour = glm::vec4(1.0, 1.0, 0.0, 1.0); // YELLOW
		break;
	case 2:
		runtimeColour = glm::vec4(1.0, 0.0, 0.0, 1.0); // RED
		break;
	case 3:
		runtimeColour = glm::vec4(0.0, 1.0, 0.0, 1.0); // GREEN
		break;
	case 4:
		runtimeColour = glm::vec4(0.0, 0.0, 1.0, 1.0); // BLUE
		break;
	default:
		cout << "Unknown Colour." << endl;
		break;
	}
}

glm::vec4 CPlayer2D::GetColour()
{
	return runtimeColour;
}

bool CPlayer2D::getPlayerMoveStatus()
{
	return isPlayerMoving;
}

bool CPlayer2D::getPlayerAttackStatus()
{
	return isAttacking;
}

void CPlayer2D::setPlayerAttackStatus(bool isAttacking)
{
	this->isAttacking = isAttacking;
}

int CPlayer2D::getPlayerAttackDirection()
{
	return attackDirection;
}

//called whenever an ammo is needed ot be shot
CAmmo2D* CPlayer2D::FetchAmmo()
{
	//Exercise 3a: Fetch a game object from m_goList and return it
	for (std::vector<CAmmo2D*>::iterator it = ammoList.begin(); it != ammoList.end(); ++it)
	{
		CAmmo2D* ammo = (CAmmo2D*)*it;
		if (ammo->getActive()) {
			continue;
		}
		ammo->setActive(true);
		// microsteps set to player's microsteps
		ammo->vec2NumMicroSteps = vec2NumMicroSteps;
		return ammo;
	}

	//whenever ammoList runs out of ammo, create 10 ammo to use
	//Get Size before adding 10
	int prevSize = ammoList.size();
	for (int i = 0; i < 10; ++i) {
		ammoList.push_back(new CAmmo2D);
	}
	ammoList.at(prevSize)->setActive(true);
	return ammoList.at(prevSize);

}
