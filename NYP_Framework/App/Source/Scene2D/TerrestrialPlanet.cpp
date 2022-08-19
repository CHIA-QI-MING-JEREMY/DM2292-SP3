/**
 CScene2D
 @brief A class which manages the 2D game scene
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "TerrestrialPlanet.h"
#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

#include "System\filesystem.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
TerrestrialPlanet::TerrestrialPlanet(void)
	: cMap2D(NULL)
	, cPlayer2D(NULL)
	, cKeyboardController(NULL)
	, cGUI_Scene2D(NULL)
	, cGameManager(NULL)
	, camera2D(NULL)
	, cSoundController(NULL)
{
}

/**
 @brief Destructor
 */
TerrestrialPlanet::~TerrestrialPlanet(void)
{
	if (cKeyboardController)
	{
		// We won't delete this since it was created elsewhere
		cKeyboardController = NULL;
	}

	if (cMap2D)
	{
		cMap2D->Destroy();
		cMap2D = NULL;
	}

	if (cPlayer2D)
	{
		cPlayer2D->Destroy();
		cPlayer2D = NULL;
	}

	if (camera2D)
	{
		camera2D->Destroy();
		camera2D = NULL;
	}

	for (unsigned int i = 0; i < enemyVectors.size(); i++)
	{
		for (unsigned int j = 0; j < enemyVectors[i].size(); ++j)
		{
			delete enemyVectors[i][j];
			enemyVectors[i][j] = NULL;
		}
		enemyVectors[i].clear();
	}
	enemyVectors.clear();

	for (unsigned int i = 0; i < resourceVectors.size(); i++)
	{
		for (unsigned int j = 0; j < resourceVectors[i].size(); ++j)
		{
			delete resourceVectors[i][j];
			resourceVectors[i][j] = NULL;
		}
		resourceVectors[i].clear();
	}
	resourceVectors.clear();

	if (cGUI_Scene2D)
	{
		cGUI_Scene2D->Destroy();
		cGUI_Scene2D = NULL;
	}

	if (cGameManager)
	{
		cGameManager->Destroy();
		cGameManager = NULL;
	}

	if (cSoundController)
	{
		cSoundController = NULL;
	}
}

/**
@brief Init Initialise this instance
*/
bool TerrestrialPlanet::Init(void)
{
	// Include Shader Manager
	CShaderManager::GetInstance()->Use("Shader2D");

	// Create and initialise the cMap2D
	cMap2D = CMap2D::GetInstance();
	// Set a shader to this class
	cMap2D->SetShader("Shader2D");
	// Initialise the instance
	if (cMap2D->Init(NUM_LEVELS, CSettings::GetInstance()->NUM_TILES_YAXIS, CSettings::GetInstance()->NUM_TILES_XAXIS) == false)
	{
		cout << "Failed to load CMap2D" << endl;
		return false;
	}
	// Load the map into an array
	if (cMap2D->LoadMap("Maps/DM2292_Map_Terrestrial_Tutorial.csv", TUTORIAL) == false)
	{
		// The loading of a map has failed. Return false
		cout << "Failed to load Terrestrial Map Tutorial Level" << endl;
		return false;
	}
	// Load the map into an array
	if (cMap2D->LoadMap("Maps/DM2292_Map_Terrestrial_01.csv", LEVEL1) == false)
	{
		// The loading of a map has failed. Return false
		cout << "Failed to load Terrestrial Map Level 01" << endl;
		return false;
	}
	// Load the map into an array
	if (cMap2D->LoadMap("Maps/DM2292_Map_Terrestrial_02A.csv", LEVEL2A) == false)
	{
		// The loading of a map has failed. Return false
		cout << "Failed to load Terrestrial Map Level 02A" << endl;
		return false;
	}
	// Load the map into an array
	if (cMap2D->LoadMap("Maps/DM2292_Map_Terrestrial_02B.csv", LEVEL2B) == false)
	{
		// The loading of a map has failed. Return false
		cout << "Failed to load Terrestrial Map Level 02B" << endl;
		return false;
	}

	// Create and initialise the CPlayer2D
	cPlayer2D = CPlayer2D::GetInstance();
	// Pass shader to cPlayer2D
	cPlayer2D->SetShader("Shader2D_Colour");
	// Initialise the instance
	if (cPlayer2D->Init() == false)
	{
		cout << "Failed to load CPlayer2D" << endl;
		return false;
	}

	//clear the individual enemy vectors inside enemyVectors
	for (int i = 0; i < enemyVectors.size(); ++i)
	{
		enemyVectors[i].clear();
	}

	//clear enemyVectors
	enemyVectors.clear();

	//cycle through the maps and find the enemies
		//and push them into the 2d enemy vector
	for (int i = 0; i < NUM_LEVELS; ++i)
	{
		//current level to check for enemies and resources
		cMap2D->SetCurrentLevel(i);

		/// <summary>
		/// ENEMIES
		/// </summary>

		vector<CEnemy2D*> enemies; //temporary vector to contain all the enemies in this 1 map
			//gets pushed into the enemyVectors vector once filled up

		while (true)
		{
			TEnemy2DSentry* cTEnemy2DSentry = new TEnemy2DSentry();
			// Pass shader to cEnemy2D
			cTEnemy2DSentry->SetShader("Shader2D_Colour");
			// Initialise the instance
			if (cTEnemy2DSentry->Init() == true)
			{
				cTEnemy2DSentry->SetPlayer2D(cPlayer2D);
				enemies.push_back(cTEnemy2DSentry); //push each enemy into the individual enemy vector
			}
			else
			{
				// Break out of this loop if all enemies have been loaded
				break;
			}
		}

		enemyVectors.push_back(enemies); //push the vector of enemies into enemyVectors

		/// <summary>
		/// RESOURCES
		/// </summary>

		vector<CResource*> resources; //temporary vector to contain all the resources in this 1 map
			//gets pushed into the resourceVectors vector once filled 
		while (true)
		{
			CResource* resource = new CResource();
			// Pass shader to cEnemy2D
			resource->SetShader("Shader2D_Colour");
			// Initialise the instance
			if (resource->Init() == true)
			{
				resources.push_back(resource); //push each resource into the individual resource vector
			}
			else
			{
				// Break out of this loop if all resources have been loaded
				break;
			}
		}

		resourceVectors.push_back(resources); //push the vector of enemies into enemyVectors

	}

	cMap2D->SetCurrentLevel(2); //reset level

	cPlayer2D->ResetRespawn();

	//// create the alarm box vector
	//alarmBoxVector.clear();
	//alarmBoxVector = cMap2D->FindAllTiles(50);

	//// assign alarm boxes to enemies (if applicable)
	//for (unsigned int i = 0; i < enemyVectors[cMap2D->GetCurrentLevel()].size(); i++)
	//{
	//	for (unsigned int j = 0; j < alarmBoxVector.size(); j++)
	//	{
	//		if (enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index.y == alarmBoxVector[j].y)
	//		{
	//			enemyVectors[cMap2D->GetCurrentLevel()][i]->setAssignedAlarmBox(alarmBoxVector[j]);
	//		}
	//	}
	//}

	// Initialise the Physics
	cPhysics2D.Init();

	// Create and initialise the CGUI_Scene2D
	cGUI_Scene2D = CGUI_Scene2D::GetInstance();
	// Initialise the instance
	if (cGUI_Scene2D->Init() == false)
	{
		cout << "Failed to load CGUI_Scene2D" << endl;
		return false;
	}

	// Store the keyboard controller singleton instance here
	cKeyboardController = CKeyboardController::GetInstance();

	// Create and initialise the cGameManager
	cGameManager = CGameManager::GetInstance();
	cGameManager->Init();

	// Init the camera
	camera2D = Camera2D::GetInstance();
	camera2D->Reset();
	camera2D->setTargetPos(cPlayer2D->vec2Index);

	// Load the sounds into CSoundController
	cSoundController = CSoundController::GetInstance();
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Sound_Thump.ogg"), 1, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Sound_JumpEffort.ogg"), 2, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Sound_JumpEffort_Female.ogg"), 3, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Sound_Thump_Female.ogg"), 4, true);

	//temp: index to be changed
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Burning.ogg"), CSoundController::SOUND_LIST::BURNING, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Fireball.ogg"), CSoundController::SOUND_LIST::FIREBALL, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\FlickSwitch.ogg"), CSoundController::SOUND_LIST::FLICK_SWITCH, true);

	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Sound_BGM.ogg"), 5, true, true);
	cSoundController->PlaySoundByID(5); // plays BGM on repeat

	// Get the handler to the CInventoryManager instance
	cInventoryManager = CInventoryManager::GetInstance();

	// Get the handler to the CInventoryManager instance
	cInventoryManagerPlanet = CInventoryManagerPlanet::GetInstance();

	// variables
	isAlarmActive = false;
	maxAlarmTimer = 10.0;
	alarmTimer = 0.0;

	isYellowObtained = false;
	isRedObtained = false;
	isGreenObtained = false;
	isBlueObtained = false;

	isWhite = true;

	return true;
}

/**
@brief Update Update this instance
*/
bool TerrestrialPlanet::Update(const double dElapsedTime)
{
	cGUI_Scene2D->setPlanetNum(2);
	// mouse Position demo
	glm::vec2 camPos = glm::vec2(camera2D->getMousePosition().x - cPlayer2D->vec2Index.x, camera2D->getMousePosition().y - cPlayer2D->vec2Index.y);
	camPos = glm::normalize(camPos);
	camPos = glm::vec2(cPlayer2D->vec2Index.x + camPos.x * 2, cPlayer2D->vec2Index.y + camPos.y * 2);

	camera2D->setTargetPos(camPos);
	camera2D->Update(dElapsedTime);

	// zoom demo
	if (!isZoomedIn && cKeyboardController->IsKeyPressed('X')) {
		camera2D->setTargetZoom(3.0f);
		isZoomedIn = true;
	}
	else if (isZoomedIn && cKeyboardController->IsKeyPressed('X')) {
		camera2D->setTargetZoom(1.0f);
		isZoomedIn = false;
	}

	// click test
	if (CMouseController::GetInstance()->IsButtonDown(CMouseController::BUTTON_TYPE::LMB)) {
		std::cout << camera2D->getBlockSelected().x << " " << camera2D->getBlockSelected().y << "\n";
	}

	// Checks if the coloured orbs have been obtained
	cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("YellowOrb");
	if (cInventoryItemPlanet->GetCount() == 1)
	{
		isYellowObtained = true;
	}
	cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("RedOrb");
	if (cInventoryItemPlanet->GetCount() == 1)
	{
		isRedObtained = true;
	}
	cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("GreenOrb");
	if (cInventoryItemPlanet->GetCount() == 1)
	{
		isGreenObtained = true;
	}
	cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("BlueOrb");
	if (cInventoryItemPlanet->GetCount() == 1)
	{
		isBlueObtained = true;
	}

	// allows player to change the colour of the character
	// coloured tiles will be replaced accordingly
	// coloured orb count decreases
	if (cKeyboardController->IsKeyPressed(GLFW_KEY_1))
	{
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("YellowOrb");
		if (cInventoryItemPlanet->GetCount() == 1)
		{
			cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::YELLOW_TILE_SOLID, CMap2D::TILE_INDEX::YELLOW_TILE_HOLLOW); // allow player to walk through yellow tiles
			cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::RED_TILE_HOLLOW, CMap2D::TILE_INDEX::RED_TILE_SOLID); // dont allow player to walk through red tiles
			cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::GREEN_TILE_HOLLOW, CMap2D::TILE_INDEX::GREEN_TILE_SOLID); // dont allow player to walk through green tiles
			cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::BLUE_TILE_HOLLOW, CMap2D::TILE_INDEX::BLUE_TILE_SOLID); // dont allow player to walk through blue tiles

			cPlayer2D->SetColour(CPlayer2D::COLOUR::YELLOW); // change player colour to yellow

			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("YellowOrb");
			cInventoryItemPlanet->Remove(1); // subtract 1 from yellow orb counter

			isWhite = false;
		}
	}
	else if (cKeyboardController->IsKeyPressed(GLFW_KEY_2))
	{
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("RedOrb");
		if (cInventoryItemPlanet->GetCount() == 1)
		{
			cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::YELLOW_TILE_HOLLOW, CMap2D::TILE_INDEX::YELLOW_TILE_SOLID); // dont allow player to walk through yellow tiles
			cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::RED_TILE_SOLID, CMap2D::TILE_INDEX::RED_TILE_HOLLOW); // allow player to walk through red tiles
			cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::GREEN_TILE_HOLLOW, CMap2D::TILE_INDEX::GREEN_TILE_SOLID); // dont allow player to walk through green tiles
			cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::BLUE_TILE_HOLLOW, CMap2D::TILE_INDEX::BLUE_TILE_SOLID); // dont allow player to walk through blue tiles

			cPlayer2D->SetColour(CPlayer2D::COLOUR::RED); // change player colour to red

			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("RedOrb");
			cInventoryItemPlanet->Remove(1); // subtract 1 from red orb counter

			isWhite = false;
		}
	}
	else if (cKeyboardController->IsKeyPressed(GLFW_KEY_3))
	{
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("GreenOrb");
		if (cInventoryItemPlanet->GetCount() == 1)
		{
			cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::YELLOW_TILE_HOLLOW, CMap2D::TILE_INDEX::YELLOW_TILE_SOLID); // dont allow player to walk through yellow tiles
			cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::RED_TILE_HOLLOW, CMap2D::TILE_INDEX::RED_TILE_SOLID); // dont allow player to walk through red tiles
			cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::GREEN_TILE_SOLID, CMap2D::TILE_INDEX::GREEN_TILE_HOLLOW); // allow player to walk through green tiles
			cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::BLUE_TILE_HOLLOW, CMap2D::TILE_INDEX::BLUE_TILE_SOLID); // dont allow player to walk through blue tiles

			cPlayer2D->SetColour(CPlayer2D::COLOUR::GREEN); // change player colour to green

			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("GreenOrb");
			cInventoryItemPlanet->Remove(1); // subtract 1 from green orb counter

			isWhite = false;
		}
	}
	else if (cKeyboardController->IsKeyPressed(GLFW_KEY_4))
	{
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("BlueOrb");
		if (cInventoryItemPlanet->GetCount() == 1)
		{
			cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::YELLOW_TILE_HOLLOW, CMap2D::TILE_INDEX::YELLOW_TILE_SOLID); // dont allow player to walk through yellow tiles
			cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::RED_TILE_HOLLOW, CMap2D::TILE_INDEX::RED_TILE_SOLID); // dont allow player to walk through red tiles
			cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::GREEN_TILE_HOLLOW, CMap2D::TILE_INDEX::GREEN_TILE_SOLID); // dont allow player to walk through green tiles
			cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::BLUE_TILE_SOLID, CMap2D::TILE_INDEX::BLUE_TILE_HOLLOW); // allow player to walk through blue tiles

			cPlayer2D->SetColour(CPlayer2D::COLOUR::BLUE); // change player colour to blue

			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("BlueOrb");
			cInventoryItemPlanet->Remove(1); // subtract 1 from blue orb counter

			isWhite = false;
		}
	}
	else if (cKeyboardController->IsKeyPressed(GLFW_KEY_5))
	{
		cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::YELLOW_TILE_HOLLOW, CMap2D::TILE_INDEX::YELLOW_TILE_SOLID); // dont allow player to walk through yellow tiles
		cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::RED_TILE_HOLLOW, CMap2D::TILE_INDEX::RED_TILE_SOLID); // dont allow player to walk through red tiles
		cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::GREEN_TILE_HOLLOW, CMap2D::TILE_INDEX::GREEN_TILE_SOLID); // dont allow player to walk through green tiles
		cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::BLUE_TILE_HOLLOW, CMap2D::TILE_INDEX::BLUE_TILE_SOLID); // dont allow player to walk through blue tiles

		cPlayer2D->SetColour(CPlayer2D::COLOUR::WHITE); // change player colour to white

		isWhite = true;
	}

	// restores coloured orb count when the player walks over a checkpoint
	if (cMap2D->GetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x) == CMap2D::TILE_INDEX::RED_FLAG)
	{
		// if yellow orb has been obtained and yellow charge has been consumed
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("YellowOrb");
		if (isYellowObtained && cInventoryItemPlanet->GetCount() == 0)
		{
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("YellowOrb");
			cInventoryItemPlanet->Add(1); // adds 1 to yellow orb counter
		}
		// if red orb has been obtained and red charge has been consumed
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("RedOrb");
		if (isRedObtained && cInventoryItemPlanet->GetCount() == 0)
		{
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("RedOrb");
			cInventoryItemPlanet->Add(1); // adds 1 to red orb counter
		}
		// if green orb has been obtained and green charge has been consumed
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("GreenOrb");
		if (isGreenObtained && cInventoryItemPlanet->GetCount() == 0)
		{
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("GreenOrb");
			cInventoryItemPlanet->Add(1); // adds 1 to green orb counter
		}
		// if blue orb has been obtained and blue charge has been consumed
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("BlueOrb");
		if (isBlueObtained && cInventoryItemPlanet->GetCount() == 0)
		{
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("BlueOrb");
			cInventoryItemPlanet->Add(1); // adds 1 to blue orb counter
		}
	}

	// Call the cPlayer2D's update method before Map2D
	// as we want to capture the inputs before Map2D update
	cPlayer2D->Update(dElapsedTime);

	if (cKeyboardController->IsKeyReleased(GLFW_KEY_F7))
	{
		cMap2D->SetCurrentLevel(0);
	}
	if (cKeyboardController->IsKeyReleased(GLFW_KEY_F8))
	{
		cMap2D->SetCurrentLevel(1);
	}

	//// Checks if alarm is active
	//if (!isAlarmActive)
	//{
	//	unsigned int uiRow = -1;
	//	unsigned int uiCol = -1;
	//	if (cMap2D->FindValue(52, uiRow, uiCol))
	//	{
	//		isAlarmActive = true;
	//		alarmTimer = maxAlarmTimer;
	//	}
	//}

	//if (alarmTimer <= 0.0)
	//{
	//	alarmTimer = maxAlarmTimer;
	//	isAlarmActive = false;
	//	cMap2D->ReplaceTiles(52, 51);
	//}
	//else
	//{
	//	alarmTimer -= dElapsedTime;
	//}

	// Call all of the cEnemy2D's update methods before Map2D
	// as we want to capture the updates before Map2D update
	for (unsigned int i = 0; i < enemyVectors[cMap2D->GetCurrentLevel()].size(); i++)
	{
		//// informs all enemies that the alarm has been activated
		//if (isAlarmActive && !enemyVectors[cMap2D->GetCurrentLevel()][i]->getAlarmState() && alarmTimer > 0.0)
		//{
		//	enemyVectors[cMap2D->GetCurrentLevel()][i]->setAlarmState(true);
		//}
		//else if (!isAlarmActive && enemyVectors[cMap2D->GetCurrentLevel()][i]->getAlarmState() && alarmTimer == maxAlarmTimer)
		//{
		//	enemyVectors[cMap2D->GetCurrentLevel()][i]->setAlarmState(false);
		//}

		enemyVectors[cMap2D->GetCurrentLevel()][i]->Update(dElapsedTime);

		//player ammo collision check with enemy
		std::vector<CAmmo2D*> ammoList = cPlayer2D->getAmmoList();
		for (std::vector<CAmmo2D*>::iterator it = ammoList.begin(); it != ammoList.end(); ++it)
		{
			CAmmo2D* ammo = (CAmmo2D*)*it;
			if (ammo->getActive())
			{
				//check if ammo hits enemy
					//if it does, minus away the enemy's health & destory the ammo
				if (ammo->InteractWithEnemy(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index))
				{
					enemyVectors[cMap2D->GetCurrentLevel()][i]->setHealth(enemyVectors[cMap2D->GetCurrentLevel()][i]->getHealth() - 25); //every hit takes off 25 HP
					ammo->setActive(false);
				}
			}
		}

		// deletes enemies if they die
		if (enemyVectors[cMap2D->GetCurrentLevel()][i]->getHealth() <= 0)
		{
			//if this isn't the last enemy in this level
			if (enemyVectors[cMap2D->GetCurrentLevel()].size() > 1)
			{
				//20% chance to drop scrap metal, 20% chance to drop battery, 10% chance to drop energy quartz
				srand(static_cast<unsigned> (time(0)));
				int resourceType = rand() % 20;
				std::cout << resourceType << std::endl;
				if (resourceType < 4) //0 1 2 3
				{
					CResource* res = new CResource(CResource::RESOURCE_TYPE::SCRAP_METAL); //create new scrap metal resource
					res->setPosition(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index, enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2NumMicroSteps);
					//set resource's position where enemy's position is
					res->SetShader("Shader2D_Colour"); //set shader
					resourceVectors[cMap2D->GetCurrentLevel()].push_back(res); //push this new resource into the resource vector for this level
				}
				else if (resourceType > 7 && resourceType < 12) //8 9 10 11
				{
					CResource* res = new CResource(CResource::RESOURCE_TYPE::BATTERY); //create new battery resource
					res->setPosition(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index, enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2NumMicroSteps);
					//set resource's position where enemy's position is
					res->SetShader("Shader2D_Colour"); //set shader
					resourceVectors[cMap2D->GetCurrentLevel()].push_back(res); //push this new resource into the resource vector for this level
				}
				else if (resourceType > 16 && resourceType < 19) //17 18
				{
					CResource* res = new CResource(CResource::RESOURCE_TYPE::ENERGY_QUARTZ); //create new energy quartz resource
					res->setPosition(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index, enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2NumMicroSteps);
					//set resource's position where enemy's position is
					res->SetShader("Shader2D_Colour"); //set shader
					resourceVectors[cMap2D->GetCurrentLevel()].push_back(res); //push this new resource into the resource vector for this level
				}
			}
			//if this is the last enemy
			else if (enemyVectors[cMap2D->GetCurrentLevel()].size() == 1)
			{
				//confirm drop an energy quartz
				CResource* res = new CResource(CResource::RESOURCE_TYPE::ENERGY_QUARTZ); //create new energy quartz resource
				res->setPosition(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index, enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2NumMicroSteps);
				//set resource's position where enemy's position is
				res->SetShader("Shader2D_Colour"); //set shader
				resourceVectors[cMap2D->GetCurrentLevel()].push_back(res); //push this new resource into the resource vector for this level
			}

			delete enemyVectors[cMap2D->GetCurrentLevel()][i];
			enemyVectors[cMap2D->GetCurrentLevel()][i] = NULL;
			enemyVectors[cMap2D->GetCurrentLevel()].erase(enemyVectors[cMap2D->GetCurrentLevel()].begin() + i);
		}
	}

	//update all resources
	for (unsigned int i = 0; i < resourceVectors[cMap2D->GetCurrentLevel()].size(); i++)
	{
		resourceVectors[cMap2D->GetCurrentLevel()][i]->Update(dElapsedTime);

		//if resource is collected
		if (resourceVectors[cMap2D->GetCurrentLevel()][i]->getCollected())
		{
			delete resourceVectors[cMap2D->GetCurrentLevel()][i];
			resourceVectors[cMap2D->GetCurrentLevel()][i] = NULL;
			resourceVectors[cMap2D->GetCurrentLevel()].erase(resourceVectors[cMap2D->GetCurrentLevel()].begin() + i);
		}
	}

	// Call the Map2D's update method
	cMap2D->Update(dElapsedTime);

	// Get keyboard updates
	if (cKeyboardController->IsKeyReleased(GLFW_KEY_F1))
	{
		cSoundController->MasterVolumeIncrease();
	}
	else if (cKeyboardController->IsKeyReleased(GLFW_KEY_F2))
	{
		cSoundController->MasterVolumeDecrease();
	}

	if (cKeyboardController->IsKeyReleased(GLFW_KEY_F6))
	{
		// Save the current game to a save file
		// Make sure the file is open
		try {
			if (cMap2D->SaveMap("Maps/DM2292_SAVEGAME.csv", cMap2D->GetCurrentLevel()) == false)
			{
				throw runtime_error("Unable to save the current game to a file");
			}
		}
		catch (runtime_error e)
		{
			cout << "Runtime error: " << e.what();
			return false;
		}
	}

	// Call the cGUI_Scene2D's update method
	cGUI_Scene2D->Update(dElapsedTime);

	// Check if the game should go to the next level
	if (cGameManager->bLevelCompleted == true)
	{
		//cMap2D->SetCurrentLevel(cMap2D->GetCurrentLevel() + 1);
		cGameManager->bLevelCompleted = false;
	}

	// Check if the game has been won by the player
	if (cGameManager->bPlayerWon == true)
	{
		// Deletes all enemies
		for (int i = 0; i < NUM_LEVELS; ++i)
		{
			enemyVectors[i].erase(enemyVectors[i].begin(), enemyVectors[i].end());
		}

		// End the game and switch to win screen
		//cMap2D->SetCurrentLevel(cMap2D->GetCurrentLevel() + 1);
		cGameManager->bPlayerWon = false;
		return false;
	}
	// Check if the game should be ended
	else if (cGameManager->bPlayerLost == true)
	{
		// Deletes all enemies
		for (int i = 0; i < NUM_LEVELS; ++i)
		{
			enemyVectors[i].erase(enemyVectors[i].begin(), enemyVectors[i].end());
		}

		// End the game and switch to lose screen
		//cMap2D->SetCurrentLevel(cMap2D->GetCurrentLevel() + 2);
		cGameManager->bPlayerLost = false;
		return false;
	}

	return true;
}

/**
 @brief PreRender Set up the OpenGL display environment before rendering
 */
void TerrestrialPlanet::PreRender(void)
{
	// Reset the OpenGL rendering environment
	glLoadIdentity();

	// Clear the screen and buffer
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Enable 2D texture rendering
	glEnable(GL_TEXTURE_2D);
}

/**
 @brief Render Render this instance
 */
void TerrestrialPlanet::Render(void)
{
	// Calls the Map2D's PreRender()
	cMap2D->PreRender();
	// Calls the Map2D's Render()
	cMap2D->Render();
	// Calls the Map2D's PostRender()
	cMap2D->PostRender();

	// Calls the CEnemy2D's PreRender()
	for (unsigned int i = 0; i < enemyVectors[cMap2D->GetCurrentLevel()].size(); i++)
	{
		// Calls the CEnemy2D's PreRender()
		enemyVectors[cMap2D->GetCurrentLevel()][i]->PreRender();
		// Calls the CEnemy2D's Render()
		enemyVectors[cMap2D->GetCurrentLevel()][i]->Render();
		// Calls the CPlayer2D's PostRender()
		enemyVectors[cMap2D->GetCurrentLevel()][i]->PostRender();
	}

	// Calls the CResource's PreRender()
	for (unsigned int i = 0; i < resourceVectors[cMap2D->GetCurrentLevel()].size(); i++)
	{
		// Calls the CResource's PreRender()
		resourceVectors[cMap2D->GetCurrentLevel()][i]->PreRender();
		// Calls the CResource's Render()
		resourceVectors[cMap2D->GetCurrentLevel()][i]->Render();
		// Calls the CResource's PostRender()
		resourceVectors[cMap2D->GetCurrentLevel()][i]->PostRender();
	}

	// Calls the CPlayer2D's PreRender()
	cPlayer2D->PreRender();
	// Calls the CPlayer2D's Render()
	cPlayer2D->Render();
	// Calls the CPlayer2D's PostRender()
	cPlayer2D->PostRender();

	// Calls the CGUI_Scene2D's PreRender()
	cGUI_Scene2D->PreRender();
	// Calls the CGUI_Scene2D's Render()
	cGUI_Scene2D->Render();
	// Calls the CGUI_Scene2D's PostRender()
	cGUI_Scene2D->PostRender();

	//render player ammo
	std::vector<CAmmo2D*> ammoList = cPlayer2D->getAmmoList();
	for (std::vector<CAmmo2D*>::iterator it = ammoList.begin(); it != ammoList.end(); ++it)
	{
		CAmmo2D* ammo = (CAmmo2D*)*it;
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
void TerrestrialPlanet::PostRender(void)
{
}

//to decide which map, aka which level to render
void TerrestrialPlanet::DecideLevel(bool tutorial)
{
	//if it is to load tutorial level
	if (tutorial)
	{
		cMap2D->SetCurrentLevel(TUTORIAL); //tutorial level
	}
	else //randomise between level 1 and 2
	{
		//random between 2 numbers to set us Scrap metal or battery
			//according to which number type is set to, load which texture
		srand(static_cast<unsigned> (time(0)));
		int randomState = rand() % 100;
		if (randomState < 50)
		{
			cMap2D->SetCurrentLevel(LEVEL1); //level 1
		}
		else
		{
			cMap2D->SetCurrentLevel(LEVEL2A); //level 2
		}
	}

	cPlayer2D->ResetRespawn(); // spawn player at the right starting location
}