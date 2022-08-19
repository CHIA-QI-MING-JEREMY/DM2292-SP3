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
	if (cInventoryManager)
	{
		cInventoryManager->Destroy();
		cInventoryManager = NULL;
	}
	if (cInventoryManagerPlanet)
	{
		cInventoryManagerPlanet->Destroy();
		cInventoryManagerPlanet = NULL;
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
bool CPlayer2D::Init(void)
{

	modeOfPlayer = NORMAL;
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
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

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
	shootingDirection = RIGHT; //by default

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

	// Get the handler to the CInventoryManager instance
	cInventoryManagerPlanet = CInventoryManagerPlanet::GetInstance();
	// Add a lives icon as one of the inventory items
	cInventoryItemPlanet = cInventoryManagerPlanet->Add("Lives", "Image/Scene2D_Lives.tga", 3, 3);
	cInventoryItemPlanet->vec2Size = glm::vec2(25, 25);

	// Add a health icon as one of the inventory items
	cInventoryItemPlanet = cInventoryManagerPlanet->Add("Health", "Image/Scene2D_Health.tga", 100, 100);
	cInventoryItemPlanet->vec2Size = glm::vec2(25, 25);

	// Add a resources as one of the inventory items --> check a max of 5 to bring back
	cInventoryItemPlanet = cInventoryManagerPlanet->Add("Resources", "Image/Scene2D_Health.tga", 5, 0);
	cInventoryItemPlanet->vec2Size = glm::vec2(25, 25);
	// Add a scrap metal as one of the inventory items
	cInventoryItemPlanet = cInventoryManagerPlanet->Add("ScrapMetal", "Image/Scene2D_Health.tga", 5, 0);
	cInventoryItemPlanet->vec2Size = glm::vec2(25, 25);
	// Add a battery as one of the inventory items
	cInventoryItemPlanet = cInventoryManagerPlanet->Add("Battery", "Image/Scene2D_Health.tga", 5, 0);
	cInventoryItemPlanet->vec2Size = glm::vec2(25, 25);
	// Add a ironwood as one of the inventory items
	cInventoryItemPlanet = cInventoryManagerPlanet->Add("Ironwood", "Image/Scene2D_Health.tga", 5, 0);
	cInventoryItemPlanet->vec2Size = glm::vec2(25, 25);
	// Add a energy quartz as one of the inventory items
	cInventoryItemPlanet = cInventoryManagerPlanet->Add("EnergyQuartz", "Image/Scene2D_Health.tga", 5, 0);
	cInventoryItemPlanet->vec2Size = glm::vec2(25, 25);
	// Add a ice crystal as one of the inventory items
	cInventoryItemPlanet = cInventoryManagerPlanet->Add("IceCrystal", "Image/Scene2D_Health.tga", 5, 0);
	cInventoryItemPlanet->vec2Size = glm::vec2(25, 25);

	//Jungle Planet
	// Add a green orb as one of the inventory items
	cInventoryItemPlanet = cInventoryManagerPlanet->Add("PoisonLevel", "Image/Scene2D_BlueOrb.tga", 3, 0);
	cInventoryItemPlanet->vec2Size = glm::vec2(25, 25);
	cInventoryItemPlanet = cInventoryManagerPlanet->Add("RiverWater", "Image/Scene2D_BlueOrb.tga", 5, 0);
	cInventoryItemPlanet->vec2Size = glm::vec2(25, 25);
	cInventoryItemPlanet = cInventoryManagerPlanet->Add("BurnableBlocks", "Image/Scene2D_BlueOrb.tga", 20, 0);
	cInventoryItemPlanet->vec2Size = glm::vec2(25, 25);
	cInventoryItemPlanet = cInventoryManagerPlanet->Add("Vine", "Image/Scene2D_BlueOrb.tga", 3, 0);
	cInventoryItemPlanet->vec2Size = glm::vec2(25, 25);

	// Terrestrial Planet
	// Add a yellow orb as one of the inventory items
	cInventoryItemPlanet = cInventoryManagerPlanet->Add("YellowOrb", "Image/Scene2D_YellowOrb.tga", 1, 0);
	cInventoryItemPlanet->vec2Size = glm::vec2(25, 25);
	// Add a red orb as one of the inventory items
	cInventoryItemPlanet = cInventoryManagerPlanet->Add("RedOrb", "Image/Scene2D_RedOrb.tga", 1, 0);
	cInventoryItemPlanet->vec2Size = glm::vec2(25, 25);
	// Add a blue orb as one of the inventory items
	cInventoryItemPlanet = cInventoryManagerPlanet->Add("GreenOrb", "Image/Scene2D_GreenOrb.tga", 1, 0);
	cInventoryItemPlanet->vec2Size = glm::vec2(25, 25);
	// Add a green orb as one of the inventory items
	cInventoryItemPlanet = cInventoryManagerPlanet->Add("BlueOrb", "Image/Scene2D_BlueOrb.tga", 1, 0);
	cInventoryItemPlanet->vec2Size = glm::vec2(25, 25);
	// Add a toxicity level as one of the inventory items
	cInventoryItemPlanet = cInventoryManagerPlanet->Add("ToxicityLevel", "Image/Scene2D_YellowOrb.tga", 100, 0);
	cInventoryItemPlanet->vec2Size = glm::vec2(25, 25);

	// Load the sounds into CSoundController
	cSoundController = CSoundController::GetInstance();

	// variables
	onRope = false;

	vec2CPIndex = vec2Index;

	isAttacking = false;
	attackDirection = RIGHT;
	maxAttackTimer = 1.25;
	attackTimer = 0.0;

	// sounds
	hasLanded = false;

	return true;
}

/**
 @brief Reset this instance, including player's location
 */
bool CPlayer2D::ResetRespawn()
{
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

	//CS: Reset jump
	iJumpCount = 0;

	//CS: Init the color to white
	runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);

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
	// Store the old position
	vec2OldIndex = vec2Index;

	// Get keyboard updates	
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

		// Attack Direction
		attackDirection = LEFT;
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
		shootingDirection = RIGHT; //setting direction for ammo shooting

		// Attack Direction
		attackDirection = RIGHT;
	}
	else if (cKeyboardController->IsKeyReleased(GLFW_KEY_D))
	{
		// Play the "idleL" animation
		animatedSprites->PlayAnimation("idleR", -1, 1.0f);
	}

	if (cKeyboardController->IsKeyDown(GLFW_KEY_W))
	{
		if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == CMap2D::TILE_INDEX::ROPE_LENGTH_LEFT || 
			cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == CMap2D::TILE_INDEX::ROPE_LENGTH_RIGHT ||
			cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == CMap2D::TILE_INDEX::VINE_LEFT ||
			cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == CMap2D::TILE_INDEX::VINE_RIGHT)

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

		shootingDirection = UP; //setting direction for ammo shooting
	}
	else if (cKeyboardController->IsKeyDown(GLFW_KEY_S))
	{
		if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == CMap2D::TILE_INDEX::ROPE_CORNER_LEFT || 
			cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == CMap2D::TILE_INDEX::ROPE_CORNER_RIGHT) ||
			(cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == CMap2D::TILE_INDEX::ROPE_LENGTH_LEFT || 
				cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == CMap2D::TILE_INDEX::ROPE_LENGTH_RIGHT) ||
			(cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == CMap2D::TILE_INDEX::VINE_CORNER_LEFT ||
				cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == CMap2D::TILE_INDEX::VINE_CORNER_RIGHT) ||
			(cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == CMap2D::TILE_INDEX::VINE_LEFT ||
				cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == CMap2D::TILE_INDEX::VINE_RIGHT))
		{
			if (vec2NumMicroSteps.y != 0 || vec2Index.y - 1 != cMap2D->FindGround(vec2Index.y, vec2Index.x))
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
		}

		shootingDirection = DOWN; //setting direction for ammo shooting
	}
	
	if (cKeyboardController->IsKeyPressed(GLFW_KEY_E) && attackTimer <= 0.0)
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

	if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != CMap2D::TILE_INDEX::ROPE_CORNER_LEFT && 
		cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != CMap2D::TILE_INDEX::ROPE_LENGTH_LEFT && 
		cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != CMap2D::TILE_INDEX::ROPE_CORNER_RIGHT && 
		cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != CMap2D::TILE_INDEX::ROPE_LENGTH_RIGHT &&
		cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != CMap2D::TILE_INDEX::VINE_CORNER_LEFT &&
		cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != CMap2D::TILE_INDEX::VINE_LEFT &&
		cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != CMap2D::TILE_INDEX::VINE_CORNER_RIGHT &&
		cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != CMap2D::TILE_INDEX::VINE_RIGHT)
	{
		onRope = false;
	}

	if (cKeyboardController->IsKeyPressed(GLFW_KEY_SPACE) && onRope == false)
	{
		if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::IDLE)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::JUMP);

			cPhysics2D.SetInitialVelocity(glm::vec2(0.0f, 3.5f)); // jump height: 2.5f -> 4 tiles for 64/48, 3.5 -> 4 tiles for 32/24
			iJumpCount += 1;

			// Play a sound for jump
			cSoundController->PlaySoundByID(2);
		}
	}
	//if (cKeyboardController->IsKeyPressed(GLFW_KEY_8)) {
	//	cout << "KNOCKBACK" << endl;
	//	if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::JUMP) {
	//		cPhysics2D.SetStatus(CPhysics2D::STATUS::KNOCKBACK);
	//		cPhysics2D.SetInitialVelocity(glm::vec2(-3.f, 0.f));
	//	}
	//	UpdateKnockback(dElapsedTime);
	//}


	// resets player location at last visited checkpoint
	//if (cKeyboardController->IsKeyPressed(GLFW_KEY_R))
	//{
	//	vec2Index = vec2CPIndex;
	//	vec2NumMicroSteps.x = 0;

	//	// reduce the lives by 1
	//	cInventoryItem = cInventoryManager->GetItem("Lives");
	//	cInventoryItem->Remove(1);
	//}

	// create ammo
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
	if (shootingDirection == LEFT) {
		if (modeOfPlayer == SHIELD) {
			animatedSprites->PlayAnimation("idleshieldL", -1, 1.0f);
		}
		else if (modeOfPlayer == BERSERKSHIELD) {
			animatedSprites->PlayAnimation("idleshieldL", -1, 1.0f);
		}
	}
	else if (shootingDirection == RIGHT) {
		if (modeOfPlayer == SHIELD) {
			animatedSprites->PlayAnimation("idleshieldR", -1, 1.0f);
		}
		else if (modeOfPlayer == BERSERKSHIELD) {
			animatedSprites->PlayAnimation("idleshieldR", -1, 1.0f);
		}
	}

	// Check if player is in mid-air, such as walking off a platform
	if (IsMidAir() == true && !onRope)
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

	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	
	// TO REMOVE LATER
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

//return shooting direction so jungle planet knows where to place the burnable blocks
int CPlayer2D::getShootingDirection()
{
	return shootingDirection;
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
	case CMap2D::TILE_INDEX::BLOOMED_BOUNCY_BLOOM: //player gets launched into the air
		cPhysics2D.SetStatus(CPhysics2D::STATUS::JUMP);
		cPhysics2D.SetInitialVelocity(glm::vec2(0.0f, 4.f));
		iJumpCount = 1;
		break;
	case CMap2D::TILE_INDEX::ROPE_POST_COILED:
		// runs if there is empty space on the left of the post
		if (cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x - 1) == 0)
		{
			// changes post with coiled rope to post with uncoiled rope left
			cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::ROPE_POST_COILED, CMap2D::TILE_INDEX::ROPE_POST_UNCOILED_LEFT, vec2Index.y, vec2Index.y + 1, vec2Index.x, vec2Index.x + 1);
			// adds rope for player to climb up
			// checks if there is any ground below the rope length
			unsigned int groundHeight = cMap2D->FindGround(vec2Index.y, vec2Index.x - 1);
			if (groundHeight < 0)
			{
				cout << "There is a hole in the ground at column " << vec2Index.x - 1 << endl;
			}
			else
			{
				cMap2D->ReplaceTiles(0, CMap2D::TILE_INDEX::ROPE_CORNER_LEFT, vec2Index.y, vec2Index.y + 1, vec2Index.x - 1, vec2Index.x);
				cMap2D->ReplaceTiles(0, CMap2D::TILE_INDEX::ROPE_LENGTH_LEFT, groundHeight, vec2Index.y, vec2Index.x - 1, vec2Index.x);
			}
		}
		// runs if there is empty space on the right of the post
		else if (cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x + 1) == 0)
		{
			// changes post with coiled rope to post with uncoiled rope right
			cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::ROPE_POST_COILED, CMap2D::TILE_INDEX::ROPE_POST_UNCOILED_RIGHT, vec2Index.y, vec2Index.y + 1, vec2Index.x, vec2Index.x + 1);
			// adds rope for player to climb up
			// checks if there is any ground below the rope length
			unsigned int groundHeight = cMap2D->FindGround(vec2Index.y, vec2Index.x + 1);
			if (groundHeight < 0)
			{
				cout << "There is a hole in the ground at column " << vec2Index.x + 1 << endl;
			}
			else
			{
				cMap2D->ReplaceTiles(0, CMap2D::TILE_INDEX::ROPE_CORNER_RIGHT, vec2Index.y, vec2Index.y + 1, vec2Index.x + 1, vec2Index.x + 2);
				cMap2D->ReplaceTiles(0, CMap2D::TILE_INDEX::ROPE_LENGTH_RIGHT, groundHeight, vec2Index.y, vec2Index.x + 1, vec2Index.x + 2);
			}
		}
		break;
	case CMap2D::TILE_INDEX::BLACK_FLAG:
		// change red flags (if any) to black flags
		cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::RED_FLAG, CMap2D::TILE_INDEX::BLACK_FLAG);
		// change current flag to red flag
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, CMap2D::TILE_INDEX::RED_FLAG);
		
		// sets CPIndex to checkpoint player just visited
		vec2CPIndex = vec2Index;

		// restores player health
		cInventoryItem = cInventoryManager->GetItem("Health");
		cInventoryItem->iItemCount = cInventoryItem->GetMaxCount();
		break;
	case CMap2D::TILE_INDEX::RED_FLAG:
		// sets CPIndex to checkpoint player just visited
		vec2CPIndex = vec2Index;

		// restores player health
		cInventoryItem = cInventoryManager->GetItem("Health");
		cInventoryItem->iItemCount = cInventoryItem->GetMaxCount();
		break;
	case CMap2D::TILE_INDEX::SPIKES_UP:
		// decrease health by 1
		cInventoryItem = cInventoryManager->GetItem("Health");
		cInventoryItem->Remove(1);
		break;
	case CMap2D::TILE_INDEX::SPIKES_LEFT:
		// decrease health by 1
		cInventoryItem = cInventoryManager->GetItem("Health");
		cInventoryItem->Remove(1);
		break;
	case CMap2D::TILE_INDEX::SPIKES_DOWN:
		// decrease health by 1
		cInventoryItem = cInventoryManager->GetItem("Health");
		cInventoryItem->Remove(1);
		break;
	case CMap2D::TILE_INDEX::SPIKES_RIGHT:
		// decrease health by 1
		cInventoryItem = cInventoryManager->GetItem("Health");
		cInventoryItem->Remove(1);
		break;
	case CMap2D::TILE_INDEX::EXIT_DOOR:
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
	cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Health");
	// Check if a life is lost
	if (cInventoryItemPlanet->GetCount() <= 0)
	{
		// Reset the Health to max value
		cInventoryItemPlanet->iItemCount = cInventoryItemPlanet->GetMaxCount();
		// But we reduce the lives by 1
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Lives");
		cInventoryItemPlanet->Remove(1);
		// Respawn player back at last visited checkpoint
		vec2Index = vec2CPIndex;
		vec2NumMicroSteps.x = 0;
	}
	// Check if there are any lives left
	cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Lives");
	if (cInventoryItemPlanet->GetCount() <= 0)
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


//TO DO AFTER WE FINISH THE REST
//void CPlayer2D::UpdateKnockback(const double dElapsedTime)
//{
//	if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::KNOCKBACK)
//	{
//		// Update the elapsed time to the physics engine
//		cPhysics2D.SetTime((float)dElapsedTime);
//		// Call the physics engine update method to calculate the final velocity and displacement
//		cPhysics2D.Update();
//		// Get the displacement from the physics engine
//		glm::vec2 v2Displacement = cPhysics2D.GetDisplacement();
//
//		// Store the current vec2Index.y
//		int iIndex_XAxis_OLD = vec2Index.x;
//
//		// Translate the displacement from pixels to indices
//		int iDisplacement_Microsteps = (int)(v2Displacement.x / cSettings->MICRO_STEP_XAXIS);
//		if (vec2Index.x < (int)cSettings->NUM_TILES_XAXIS)
//		{
//			vec2NumMicroSteps.x += iDisplacement_Microsteps;
//			if (vec2NumMicroSteps.x > cSettings->NUM_STEPS_PER_TILE_XAXIS)
//			{
//				vec2NumMicroSteps.x -= cSettings->NUM_STEPS_PER_TILE_XAXIS;
//				if (vec2NumMicroSteps.x < 0)
//				{
//					vec2NumMicroSteps.x = 0;
//				}
//				vec2Index.x--;
//			}
//		}
//		// Constraint the player's position within the screen boundary
//		if (vec2Index.x >= 0)
//		{
//			vec2NumMicroSteps.x--;
//			if (vec2NumMicroSteps.x < 0)
//			{
//				vec2NumMicroSteps.x = ((int)cSettings->NUM_STEPS_PER_TILE_XAXIS) - 1;
//				vec2Index.x--;
//			}
//		}
//		Constraint(LEFT);
//		if (CheckPosition(LEFT) == false)
//		{
//			vec2Index = vec2OldIndex;
//			vec2NumMicroSteps.x = 0;
//			cPhysics2D.SetStatus(CPhysics2D::STATUS::IDLE);
//		}
//		// Calculate the new position to the down
//		if (vec2Index.y >= 0)
//		{
//			vec2NumMicroSteps.y--;
//			if (vec2NumMicroSteps.y < 0)
//			{
//				vec2NumMicroSteps.y = ((int)cSettings->NUM_STEPS_PER_TILE_YAXIS) - 1;
//				vec2Index.y--;
//			}
//		}
//		// Constraint the player's position within the screen 
//		Constraint(DOWN);
//
//		if (CheckPosition(DOWN) == false)
//		{
//			vec2Index = vec2OldIndex;
//			vec2NumMicroSteps.y = 0;
//			cPhysics2D.SetStatus(CPhysics2D::STATUS::IDLE);
//		}
//
//	}
//
//}

bool CPlayer2D::IsMidAir(void)
{
	// if the player is at the bottom row, then he is not in mid-air for sure
	if (vec2Index.y == 0)
	{
		return false;
	}

	// Check if the tile below the player is a coloured collectible tile
	if (cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) == CMap2D::TILE_INDEX::YELLOW_TILE_HOLLOW
		|| cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) == CMap2D::TILE_INDEX::RED_TILE_HOLLOW
		|| cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) == CMap2D::TILE_INDEX::GREEN_TILE_HOLLOW
		|| cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) == CMap2D::TILE_INDEX::BLUE_TILE_HOLLOW)
	{
		return true;
	}

	// Check if the player is on the rope corners
	if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == CMap2D::TILE_INDEX::ROPE_CORNER_LEFT || cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == CMap2D::TILE_INDEX::ROPE_CORNER_RIGHT)
	{
		return false;
	}

	// Check if the tile below the player's current position is empty
	if (vec2NumMicroSteps.x == 0 &&
		(cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) < 600))
	{
		return true;
	}

	// Check if the player is floating between 2 tiles when not on rope
	if (vec2NumMicroSteps.y != 0 && (onRope == false))
	{
		return true;
	}

	//if player is standing between 2 tiles which are both not obstruction blocks
	if ((cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) < 600) && (cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x + 1) < 600))
	{
		return true;
	}

	return false;
}

void CPlayer2D::SetColour(COLOUR colour)
{
	switch (colour)
	{
	case WHITE:
		runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0); // WHITE
		break;
	case YELLOW:
		runtimeColour = glm::vec4(1.0, 1.0, 0.0, 1.0); // YELLOW
		break;
	case RED:
		runtimeColour = glm::vec4(1.0, 0.0, 0.0, 1.0); // RED
		break;
	case GREEN:
		runtimeColour = glm::vec4(0.0, 1.0, 0.0, 1.0); // GREEN
		break;
	case BLUE:
		runtimeColour = glm::vec4(0.0, 0.0, 1.0, 1.0); // BLUE
		break;
	case PURPLE:
		runtimeColour = glm::vec4(1.0, 0.0, 1.0, 1.0); // PURPLE
		break;
	case PINK:
		runtimeColour = glm::vec4(1.0, 0.5, 0.5, 1.0); // PINK
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

int CPlayer2D::getModeOfPlayer()
{
	return modeOfPlayer;
}

void CPlayer2D::setModeOfPlayer(int a)
{
	modeOfPlayer = a;
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
