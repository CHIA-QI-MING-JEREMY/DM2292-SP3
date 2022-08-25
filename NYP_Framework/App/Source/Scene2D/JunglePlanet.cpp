/**
 CScene2D
 @brief A class which manages the 2D game scene
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "JunglePlanet.h"
#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"
#include "../App/Source/GameStateManagement/GameInfo.h"

#include "System\filesystem.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
JunglePlanet::JunglePlanet(void)
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
JunglePlanet::~JunglePlanet(void)
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
bool JunglePlanet::Init(void)
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
	if (cMap2D->LoadMap("Maps/DM2292_Map_Jungle_Tutorial.csv", TUTORIAL) == false)
	{
		// The loading of a map has failed. Return false
		cout << "Failed to load Jungle Map Tutorial Level" << endl;
		return false;
	}
	// Load the map into an array
	if (cMap2D->LoadMap("Maps/DM2292_Map_Jungle_01.csv", LEVEL1) == false)
	{
		// The loading of a map has failed. Return false
		cout << "Failed to load Jungle Map Level 01" << endl;
		return false;
	}
	// Load the map into an array
	if (cMap2D->LoadMap("Maps/DM2292_Map_Jungle_02.csv", LEVEL2) == false)
	{
		// The loading of a map has failed. Return false
		cout << "Failed to load Jungle Map Level 02" << endl;
		return false;
	}

	// Create and initialise the CGUI_Scene2D
		//done before player as ammo needs to know the planet number and ammo is init in player
	cGUI_Scene2D = CGUI_Scene2D::GetInstance();
	// Initialise the instance
	if (cGUI_Scene2D->Init() == false)
	{
		cout << "Failed to load CGUI_Scene2D" << endl;
		return false;
	}
	cGUI_Scene2D->setPlanetNum(1);

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
			JEnemy2DVT* cJEnemy2DVT = new JEnemy2DVT();
			// Pass shader to cEnemy2D
			cJEnemy2DVT->SetShader("Shader2D_Colour");
			// Initialise the instance
			if (cJEnemy2DVT->Init() == true)
			{
				cJEnemy2DVT->SetPlayer2D(cPlayer2D);
				enemies.push_back(cJEnemy2DVT); //push each enemy into the individual enemy vector
			}
			else
			{
				// Break out of this loop if all enemies have been loaded
				break;
			}
		}

		while (true)
		{
			JEnemy2DShyC* cJEnemy2DShyC = new JEnemy2DShyC();
			// Pass shader to cEnemy2D
			cJEnemy2DShyC->SetShader("Shader2D_Colour");
			// Initialise the instance
			if (cJEnemy2DShyC->Init() == true)
			{
				cJEnemy2DShyC->SetPlayer2D(cPlayer2D);
				enemies.push_back(cJEnemy2DShyC); //push each enemy into the individual enemy vector
			}
			else
			{
				// Break out of this loop if all enemies have been loaded
				break;
			}
		}

		while (true)
		{
			JEnemy2DPatrolT* cJEnemy2DPatrolT = new JEnemy2DPatrolT();
			// Pass shader to cEnemy2D
			cJEnemy2DPatrolT->SetShader("Shader2D_Colour");
			// Initialise the instance
			if (cJEnemy2DPatrolT->Init() == true)
			{
				cJEnemy2DPatrolT->SetPlayer2D(cPlayer2D);
				enemies.push_back(cJEnemy2DPatrolT); //push each enemy into the individual enemy vector
			}
			else
			{
				// Break out of this loop if all enemies have been loaded
				break;
			}
		}

		while (true)
		{
			JEnemy2DITracker* cJEnemy2DITracker = new JEnemy2DITracker();
			// Pass shader to cEnemy2D
			cJEnemy2DITracker->SetShader("Shader2D_Colour");
			// Initialise the instance
			if (cJEnemy2DITracker->Init() == true)
			{
				cJEnemy2DITracker->SetPlayer2D(cPlayer2D);
				enemies.push_back(cJEnemy2DITracker); //push each enemy into the individual enemy vector
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

	cMap2D->SetCurrentLevel(0); //reset level

	//find new spawn location of player according to which map is loaded in
	cPlayer2D->ResetRespawn(); //used if set current lvl used in init doesnt reset to first map

	// Initialise the Physics
	cPhysics2D.Init();

	// Get the handler to the CInventoryManager instance
	cInventoryManagerPlanet = CInventoryManagerPlanet::GetInstance();
	cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("PoisonLevel");
		//dictates how badly affected the player is by poison
		//poison level 0 means the player has not been poisoned
	cInventoryItemPlanet->Remove(cInventoryItemPlanet->GetCount()); //set poison level to 0

	// Get the handler for the cInventoryManager
	cInventoryManager = CGameInfo::GetInstance()->ImportIM();

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
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Sound_Thump.ogg"), CSoundController::SOUND_LIST::LAND, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Sound_JumpEffort.ogg"), CSoundController::SOUND_LIST::JUMP, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Sound_JumpEffort_Female.ogg"), CSoundController::SOUND_LIST::ENEMY_JUMP, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Sound_Thump_Female.ogg"), CSoundController::SOUND_LIST::ENEMY_LAND, true);

	//common sounds
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\StowItemInPocket.ogg"), CSoundController::SOUND_LIST::COLLECT_ITEM, true);

	//planet specific sounds
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\WalkInWater.ogg"), CSoundController::SOUND_LIST::SPLASH, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Burning.ogg"), CSoundController::SOUND_LIST::BURNING, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Fireball.ogg"), CSoundController::SOUND_LIST::FIREBALL, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\FlickSwitch.ogg"), CSoundController::SOUND_LIST::FLICK_SWITCH, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Ticking.ogg"), CSoundController::SOUND_LIST::TICKING, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Exploding.ogg"), CSoundController::SOUND_LIST::EXPLOSION, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\PoisonDamage.ogg"), CSoundController::SOUND_LIST::POISON_DAMAGE, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\WaterDroplet.ogg"), CSoundController::SOUND_LIST::WATER_FLOWER, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Magic.ogg"), CSoundController::SOUND_LIST::USING_WATER, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\PlaceBush.ogg"), CSoundController::SOUND_LIST::PLACE_BUSH, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\TyingVine.ogg"), CSoundController::SOUND_LIST::TIE_VINE, true);

	//enemy related
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\AcidBurn.ogg"), CSoundController::SOUND_LIST::POISONBALL, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\SlashAttack.ogg"), CSoundController::SOUND_LIST::ENEMY_MELEE, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\WarningSiren.ogg"), CSoundController::SOUND_LIST::PATROL_TEAM_NOISY, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\TeleportPop.ogg"), CSoundController::SOUND_LIST::VT_TELEPORT_POOF, true);

	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Sound_BGM.ogg"), CSoundController::SOUND_LIST::BGM_NORMAL, true, true);
	cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BGM_NORMAL); // plays BGM on repeat

	// variables
	isAlarmActive = false;
	maxAlarmTimer = 10.0;
	alarmTimer = 0.0;
	
	poisonLevelIncreaseCooldown = 0.0; //poison level can increase every 2 seconds if hit by something poisonous

	//contains the int of how much of the health is removed from player per damage hit
	poisonDamage.clear(); //make sure it is clear before pushing in
	poisonDamage.push_back(0); //0 means that the player takes 0% damage
	poisonDamage.push_back(3); //1 means that the player takes 3% damage
	poisonDamage.push_back(5); //2 means that the player takes 5% damage
	poisonDamage.push_back(10); //3 means that the player takes 10% damage

	//contains the max cooldown to be used for each poison level
	poisonDamageHitMaxCooldown.clear(); //make sure it is clear before pushing in
	poisonDamageHitMaxCooldown.push_back(0.0); //0 would be 0.0s
	poisonDamageHitMaxCooldown.push_back(5.0); //1 would be 5.0s
	poisonDamageHitMaxCooldown.push_back(4.0); //2 would be 4.0s
	poisonDamageHitMaxCooldown.push_back(3.0); //3 would be 3.0s

	cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("PoisonLevel");
	poisonDamageHitCooldown = poisonDamageHitMaxCooldown[cInventoryItemPlanet->GetCount()]; //starts off at max cooldown whenever the poison lvl is set/changes before depleting
		//once it hits 0, player takes damage

	poisonPurpleCooldown = 0.0;
	poisonPurple = false;

	swayingLeavesCooldown = swayingLeavesMaxCooldown; //cooldown to switch leaves with its alt positions

	triggeredPlaceableBushPopUp = false; //so placeable bush pop up only gets triggered once before it is overridden
	triggeredVinePopUp = false; //so placeable bush pop up only gets triggered once before it is overridden

	return true;
}

/**
@brief Update Update this instance
*/
bool JunglePlanet::Update(const double dElapsedTime)
{
	//if not showing vine or bushes pop up (since those 2 are only set once and meant to disappear only after being overriden
	if (cGUI_Scene2D->getTutorialPopupJungle() != CGUI_Scene2D::JUNGLE_TUTORIAL_POPUP::BUSHES &&
		cGUI_Scene2D->getTutorialPopupJungle() != CGUI_Scene2D::JUNGLE_TUTORIAL_POPUP::VINE)
	{
		cGUI_Scene2D->setTutorialPopupJungle(CGUI_Scene2D::JUNGLE_TUTORIAL_POPUP::NONE); //remove pop ups by default
	}

	// mouse Position demo
	// zoom demo
	if (!isZoomedIn && cKeyboardController->IsKeyPressed('X')) {
		camera2D->setTargetZoom(3.0f);
		isZoomedIn = true;
	}
	else if (isZoomedIn && cKeyboardController->IsKeyPressed('X')) {
		camera2D->setTargetZoom(1.0f);
		isZoomedIn = false;
	}

	float mouseDist;

	if (cGUI_Scene2D->getShowExitPanel() == false) {
		mouseDist = 2.0f;
	}
	else {
		mouseDist = 0.3f;
	}

	// mouse Position demo
	/*glm::vec2 camPos = glm::vec2(camera2D->getMousePosition().x - cPlayer2D->vec2Index.x, camera2D->getMousePosition().y - cPlayer2D->vec2Index.y);
	camPos = glm::normalize(camPos);
	camPos = glm::vec2(cPlayer2D->vec2Index.x + camPos.x * mouseDist, cPlayer2D->vec2Index.y + camPos.y * mouseDist);

	camera2D->setTargetPos(camPos);*/
	camera2D->setTargetPos(cPlayer2D->vec2Index);
	camera2D->Update(dElapsedTime);
 
	// click test
	if (CMouseController::GetInstance()->IsButtonDown(CMouseController::BUTTON_TYPE::LMB)) {
		std::cout << camera2D->getBlockSelected().x << " " << camera2D->getBlockSelected().y << "\n";
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

	PlayerInteractWithMap(); //called before tutorial lvl pop ups so they can override what is decided in playerinteract with map
	
	//Tutorial lvl pop ups
	if (cMap2D->GetCurrentLevel() == TUTORIAL)
	{
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("BurnableBlocks");
		//if player has picked up burnable blocks
		if (!triggeredPlaceableBushPopUp && cInventoryItemPlanet->GetCount() > 0) //only triggered once then gets overridden
		{
			cGUI_Scene2D->setTutorialPopupJungle(CGUI_Scene2D::JUNGLE_TUTORIAL_POPUP::BUSHES); //trigger bush pop up
			triggeredPlaceableBushPopUp = true;
		}

		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Vine");
		//if player has picked up vines
		if (!triggeredVinePopUp && cInventoryItemPlanet->GetCount() > 0) //only triggered once then gets overridden
		{
			cGUI_Scene2D->setTutorialPopupJungle(CGUI_Scene2D::JUNGLE_TUTORIAL_POPUP::VINE); //trigger vines pop up
			triggeredVinePopUp = true;
		}

		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("PoisonLevel");
		//if player poisoned by first poison sprout
		if (cInventoryItemPlanet->GetCount() > 0 &&
			cPhysics2D.CalculateDistance(cPlayer2D->vec2Index, cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SHOOTING_POPUP)) < 8.f &&
			cPlayer2D->vec2Index.y - cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SHOOTING_POPUP).y < 2) //player isn't much higher than the invisible pop up trigger point
		{
			cGUI_Scene2D->setTutorialPopupJungle(CGUI_Scene2D::JUNGLE_TUTORIAL_POPUP::POISON); //trigger poison pop up
		}

		//if hit the first button (button now pressed) and near the shoot invisible pop up trigger point
		if (cMap2D->FindAllTiles(CMap2D::TILE_INDEX::DOWN_BUTTON_PRESSED).size() > 0 &&
			cPhysics2D.CalculateDistance(cPlayer2D->vec2Index, cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SHOOTING_POPUP)) < 9.f &&
			cPlayer2D->vec2Index.y - cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SHOOTING_POPUP).y < 2) //player isn't much higher than the invisible pop up trigger point
		{
			cGUI_Scene2D->setTutorialPopupJungle(CGUI_Scene2D::JUNGLE_TUTORIAL_POPUP::SWITCHES); //trigger button pop up
		}

		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Resources");
		//if collected 1 resource and near the shoot invisible pop up trigger point
		if (cInventoryItemPlanet->GetCount() > 0 &&
			cPhysics2D.CalculateDistance(cPlayer2D->vec2Index, cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SHOOTING_POPUP)) < 6.f &&
			cPlayer2D->vec2Index.y - cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SHOOTING_POPUP).y < 2) //player isn't much higher than the invisible pop up trigger point
		{
			cGUI_Scene2D->setTutorialPopupJungle(CGUI_Scene2D::JUNGLE_TUTORIAL_POPUP::RESOURCE); //trigger resource pop up
		}

		//if hit a bush wth ammo (set the bush on fire) and near the shoot invisible pop up trigger point
		if (cMap2D->FindAllTiles(CMap2D::TILE_INDEX::BURNING_BUSH).size() > 0 &&
			cPhysics2D.CalculateDistance(cPlayer2D->vec2Index, cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SHOOTING_POPUP)) < 3.f)
		{
			cGUI_Scene2D->setTutorialPopupJungle(CGUI_Scene2D::JUNGLE_TUTORIAL_POPUP::BURNABLE); //trigger burnable bush pop up
		}

		//if player at the first checkpoint
		if (cMap2D->GetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x) == CMap2D::TILE_INDEX::RED_FLAG &&
			cPhysics2D.CalculateDistance(cPlayer2D->vec2Index, cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SHOOTING_POPUP)) < 3.f)
		{
			cGUI_Scene2D->setTutorialPopupJungle(CGUI_Scene2D::JUNGLE_TUTORIAL_POPUP::CHECKPOINT); //start with checkpoint popup
		}
	}

	// Call all of the cEnemy2D's update methods before Map2D
	// as we want to capture the updates before Map2D update
	for (unsigned int i = 0; i < enemyVectors[cMap2D->GetCurrentLevel()].size(); i++)
	{
		enemyVectors[cMap2D->GetCurrentLevel()][i]->Update(dElapsedTime);

		//for patrol team, aka community based enemies
			//if it is in noisy mode (aka getNoisy is true), check all other community enemies to see if they are near this noisy enemy
			//if they are near and not in noisy mode, set alert to be true
		if (enemyVectors[cMap2D->GetCurrentLevel()][i]->getType() == CEnemy2D::ENEMYTYPE::COMMUNITY &&
			enemyVectors[cMap2D->GetCurrentLevel()][i]->getNoisy())
		{
			//check all other enemies
			for (unsigned int j = 0; j < enemyVectors[cMap2D->GetCurrentLevel()].size(); j++)
			{
				//if is a community enemy and not the current enemy 
				if (enemyVectors[cMap2D->GetCurrentLevel()][j]->getType() == CEnemy2D::ENEMYTYPE::COMMUNITY &&
					enemyVectors[cMap2D->GetCurrentLevel()][j] != enemyVectors[cMap2D->GetCurrentLevel()][i])
				{
					//if this other enemy is near the current enemy
					if (cPhysics2D.CalculateDistance(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index, 
						enemyVectors[cMap2D->GetCurrentLevel()][j]->vec2Index) < 6.0f)
					{
						enemyVectors[cMap2D->GetCurrentLevel()][j]->setAlert(); //send it into en_route state
					}
				}
			}
		}

		//player ammo collision check with enemy
		std::vector<CAmmo2D*> ammoList = cPlayer2D->getAmmoList();
		for (std::vector<CAmmo2D*>::iterator it = ammoList.begin(); it != ammoList.end(); ++it)
		{
			CAmmo2D* ammo = (CAmmo2D*)*it;
			if (ammo->getActive())
			{
				//check if ammo hits enemy
					//if it does, minus away the enemy's health & destroy the ammo
				if (ammo->InteractWithEnemy(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index))
				{
					//if enemy can hunker, aka increase their own defence
					if (enemyVectors[cMap2D->GetCurrentLevel()][i]->getType() == CEnemy2D::ENEMYTYPE::DEFENCE)
					{
						//if enemy can only take a max of 1 full power hit left, aka time to switch to explode mode, 
							//enemy not allowed to take damage anymore
							//so enemy can only take damage if enemy's health is above 5
						if (enemyVectors[cMap2D->GetCurrentLevel()][i]->getHealth() > 5)
						{
							//if enemy is hunkering, aka defence is up
							if (enemyVectors[cMap2D->GetCurrentLevel()][i]->getHunkering())
							{
								enemyVectors[cMap2D->GetCurrentLevel()][i]->setHealth(enemyVectors[cMap2D->GetCurrentLevel()][i]->getHealth() - 2); //every hit takes off 2 HP

							}
							else
							{
								enemyVectors[cMap2D->GetCurrentLevel()][i]->setHealth(enemyVectors[cMap2D->GetCurrentLevel()][i]->getHealth() - 5); //every hit takes off 5 HP
							}
						}
					}
					else //normal enemy in terms of damage taking
					{
						enemyVectors[cMap2D->GetCurrentLevel()][i]->setHealth(enemyVectors[cMap2D->GetCurrentLevel()][i]->getHealth() - 5); //every hit takes off 5 HP
					}
					ammo->setActive(false);
				}
			}
		}

		// deletes enemies if they die
		if (enemyVectors[cMap2D->GetCurrentLevel()][i]->getHealth() <= 0)
		{
			//in the tutorial level, all enemies are guaranteed to drop resources
			if (cMap2D->GetCurrentLevel() == TUTORIAL)
			{
				//if this isn't the last enemy in this level
				if (enemyVectors[cMap2D->GetCurrentLevel()].size() > 1)
				{
					//40% chance to drop scrap metal, 40% chance to drop battery, 20% chance to drop ironwood
					int resourceType = rand() % 10;
					std::cout << resourceType << std::endl;
					if (resourceType < 4) //0 1 2 3
					{
						CResource* res = new CResource(CResource::RESOURCE_TYPE::SCRAP_METAL); //create new scrap metal resource
						res->setPosition(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index, enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2NumMicroSteps);
						//set resource's position where enemy's position is
						res->SetShader("Shader2D_Colour"); //set shader
						resourceVectors[cMap2D->GetCurrentLevel()].push_back(res); //push this new resource into the resource vector for this level
					}
					else if (resourceType < 8) //4 5 6 7
					{
						CResource* res = new CResource(CResource::RESOURCE_TYPE::BATTERY); //create new battery resource
						res->setPosition(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index, enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2NumMicroSteps);
						//set resource's position where enemy's position is
						res->SetShader("Shader2D_Colour"); //set shader
						resourceVectors[cMap2D->GetCurrentLevel()].push_back(res); //push this new resource into the resource vector for this level
					}
					else //8 9 
					{
						CResource* res = new CResource(CResource::RESOURCE_TYPE::IRONWOOD); //create new ironwood resource
						res->setPosition(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index, enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2NumMicroSteps);
						//set resource's position where enemy's position is
						res->SetShader("Shader2D_Colour"); //set shader
						resourceVectors[cMap2D->GetCurrentLevel()].push_back(res); //push this new resource into the resource vector for this level
					}
				}
				//if this is the last enemy
				else if (enemyVectors[cMap2D->GetCurrentLevel()].size() == 1)
				{
					//confirm drop an ironwood
					CResource* res = new CResource(CResource::RESOURCE_TYPE::IRONWOOD); //create new ironwood resource
					res->setPosition(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index, enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2NumMicroSteps);
					//set resource's position where enemy's position is
					res->SetShader("Shader2D_Colour"); //set shader
					resourceVectors[cMap2D->GetCurrentLevel()].push_back(res); //push this new resource into the resource vector for this level
				}
			}
			else
			{
				//if this isn't the last enemy in this level
				if (enemyVectors[cMap2D->GetCurrentLevel()].size() > 1)
				{
					//20% chance to drop scrap metal, 20% chance to drop battery, 10% chance to drop ironwood
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
						CResource* res = new CResource(CResource::RESOURCE_TYPE::IRONWOOD); //create new ironwood resource
						res->setPosition(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index, enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2NumMicroSteps);
						//set resource's position where enemy's position is
						res->SetShader("Shader2D_Colour"); //set shader
						resourceVectors[cMap2D->GetCurrentLevel()].push_back(res); //push this new resource into the resource vector for this level
					}
				}
				//if this is the last enemy
				else if (enemyVectors[cMap2D->GetCurrentLevel()].size() == 1)
				{
					//confirm drop an ironwood
					CResource* res = new CResource(CResource::RESOURCE_TYPE::IRONWOOD); //create new ironwood resource
					res->setPosition(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index, enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2NumMicroSteps);
					//set resource's position where enemy's position is
					res->SetShader("Shader2D_Colour"); //set shader
					resourceVectors[cMap2D->GetCurrentLevel()].push_back(res); //push this new resource into the resource vector for this level
				}
			}
			

			//if enemy has teleportation residue, call its update to delete the residue before deleting it
			if (enemyVectors[cMap2D->GetCurrentLevel()][i]->getType() == CEnemy2D::ENEMYTYPE::TELEPORTABLE)
			{
 				enemyVectors[cMap2D->GetCurrentLevel()][i]->Update(dElapsedTime);
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

	//if player is not in river water, F will be used to use the river water instead of collect it
		//and instead of using it on an unbloomed bouncy bloom
	if (cMap2D->GetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x) != CMap2D::TILE_INDEX::RIVER_WATER &&
		cMap2D->GetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x) != CMap2D::TILE_INDEX::ENEMY_WAYPOINT_RIVER_WATER &&
		cMap2D->GetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x) != CMap2D::TILE_INDEX::UNBLOOMED_BOUNCY_BLOOM &&
		cMap2D->GetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x) != CMap2D::TILE_INDEX::BLOOMED_BOUNCY_BLOOM)
	{
		//if the player tries to use the river water
		if (cKeyboardController->IsKeyPressed(GLFW_KEY_Q))
		{
			//if the player has river water
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("RiverWater");
			//if at least 1 river water
			if (cInventoryItemPlanet->GetCount() > 0)
			{
				cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::USING_WATER); //play sound to indicate plyer using water to heal
				//else water is used to heal the player and cure their poison
				cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Health");
				cInventoryItemPlanet->Add(5); //increase health by 5 for every river water used

				//cure poison, no cooldown consideration needed here
				cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("PoisonLevel");
				//if poison lvl not at 0
				if (cInventoryItemPlanet->GetCount() > 0)
				{
					cInventoryItemPlanet->Remove(1); //decrease poison level by 1

					//if removed poison entirely
					if (cInventoryItemPlanet->GetCount() == 0)
					{
						cPlayer2D->SetColour(CPlayer2D::COLOUR::WHITE); //remove purple poison colour defintely
					}
				}

				cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("RiverWater");
				cInventoryItemPlanet->Remove(1); //use up 1 river water
			}
		}
	}
	
	//leaves sawying --> moving platforms
	swayingLeavesCooldown -= dElapsedTime; //depelte cooldown
	if (swayingLeavesCooldown <= 0.0) //cooldown up
	{
		//switch alt with solid and solid with alt
		cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::MOVING_LEAF_ALT, CMap2D::TILE_INDEX::MOVING_LEAF_TRANSITIONER); //all alt still bank
		cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::MOVING_LEAF_SOLID, CMap2D::TILE_INDEX::MOVING_LEAF_ALT); //all solid now blank
		cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::MOVING_LEAF_TRANSITIONER, CMap2D::TILE_INDEX::MOVING_LEAF_SOLID); //all transiiton blank now solid

		swayingLeavesCooldown = swayingLeavesMaxCooldown; //reset cooldown
	}

	// resets player location at last visited checkpoint
	if (cKeyboardController->IsKeyPressed(GLFW_KEY_R))
	{
		cPlayer2D->vec2Index = cPlayer2D->getCPIndex();
		cPlayer2D->vec2NumMicroSteps.x = 0;

		// reduce the lives by 1
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Lives");
		if (cMap2D->GetCurrentLevel() != TUTORIAL || cInventoryItemPlanet->GetCount() > 1)
		{
			cInventoryItemPlanet->Remove(1);
		}
	}

	//if player has burnable blocks to put down, put down in the direction the player is facing
	if (cKeyboardController->IsKeyPressed(GLFW_KEY_G))
	{
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("BurnableBlocks");
		//if player has burnable blocks
		if (cInventoryItemPlanet->GetCount() > 0)
		{
			//player facing down, trying to put a burnable block
			if (cPlayer2D->getShootingDirection() == CPlayer2D::DIRECTION::DOWN)
			{
				//if the spot is empty
				if (cMap2D->GetMapInfo(cPlayer2D->vec2Index.y - 1, cPlayer2D->vec2Index.x) == 0)
				{
					//set tile to burnable bush
					cMap2D->SetMapInfo(cPlayer2D->vec2Index.y - 1, cPlayer2D->vec2Index.x, CMap2D::TILE_INDEX::BURNABLE_BUSH);
					cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::PLACE_BUSH); //play bush placing sound
					cInventoryItemPlanet->Remove(1);
				}
			}
			//player facing up, trying to put a burnable block
			else if (cPlayer2D->getShootingDirection() == CPlayer2D::DIRECTION::UP)
			{
				//if the spot is empty
				if (cMap2D->GetMapInfo(cPlayer2D->vec2Index.y + 1, cPlayer2D->vec2Index.x) == 0)
				{
					//set tile to burnable bush
					cMap2D->SetMapInfo(cPlayer2D->vec2Index.y + 1, cPlayer2D->vec2Index.x, CMap2D::TILE_INDEX::BURNABLE_BUSH);
					cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::PLACE_BUSH); //play bush placing sound
					cInventoryItemPlanet->Remove(1);
				}
			}
			//player facing left, trying to put a burnable block
			else if (cPlayer2D->getShootingDirection() == CPlayer2D::DIRECTION::LEFT)
			{
				//if the spot is empty
				if (cMap2D->GetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x - 1) == 0)
				{
					//set tile to burnable bush
					cMap2D->SetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x - 1, CMap2D::TILE_INDEX::BURNABLE_BUSH);
					cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::PLACE_BUSH); //play bush placing sound
					cInventoryItemPlanet->Remove(1);
				}
			}
			//player facing right, trying to put a burnable block
			else if (cPlayer2D->getShootingDirection() == CPlayer2D::DIRECTION::RIGHT)
			{
				//if the spot is empty
				if (cMap2D->GetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x + 1) == 0)
				{
					//set tile to burnable bush
					cMap2D->SetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x + 1, CMap2D::TILE_INDEX::BURNABLE_BUSH);
					cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::PLACE_BUSH); //play bush placing sound
					cInventoryItemPlanet->Remove(1);
				}
			}
		}
	}

	//if time to take damage from poison,
	if (poisonDamageHitCooldown <= 0.0)
	{
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("PoisonLevel");
		int poisonLvl = cInventoryItemPlanet->GetCount(); //find poison lvl
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Health"); //find player's health to deplete
		cInventoryItemPlanet->Remove(poisonDamage[poisonLvl]); //player take damage according to their poison level

		if (poisonLvl > 0) //if player is poisoned
		{
			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::POISON_DAMAGE); //play sound to indicate taking poison damage
		}

		poisonDamageHitCooldown = poisonDamageHitMaxCooldown[poisonLvl]; //reset damage hit cooldown
	}
	else //time to dElapsedTime away from cooldown
	{
		poisonDamageHitCooldown -= dElapsedTime; //deplete cooldown
	}

	//deplete poison lvl increase cooldown if it isnt at 0
	if (poisonLevelIncreaseCooldown > 0.0)
	{
		poisonLevelIncreaseCooldown -= dElapsedTime; //deplete cooldown
	}
	else if (poisonLevelIncreaseCooldown < 0.0)
	{
		poisonLevelIncreaseCooldown = 0.0;
	}

	//if player is poisoned, then set player colour to purple
	cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("PoisonLevel");
	//if poison lvl at least lvl 1
	if (cInventoryItemPlanet->GetCount() > 0)
	{
		//deplete poison colour flickering
		if (poisonPurpleCooldown < 0.0) //if below zero
		{
			poisonPurpleCooldown = poisonPurpleMaxCooldown; //reset cooldown

			//if supposed to change to purple
			if (poisonPurple)
			{
				cPlayer2D->SetColour(CPlayer2D::COLOUR::PURPLE);
				poisonPurple = false;
			}
			else //set to white
			{
				cPlayer2D->SetColour(CPlayer2D::COLOUR::WHITE);
				poisonPurple = true;
			}
		}
		else //above 0
		{
			poisonPurpleCooldown -= dElapsedTime; //deplete cooldown
		}
	}
	else //nto poisoned
	{
		poisonPurple = false;
	}

	//play ammo sound if there is an ammo on screen
	std::vector<CAmmo2D*> ammoList = cPlayer2D->getAmmoList();
	for (std::vector<CAmmo2D*>::iterator it = ammoList.begin(); it != ammoList.end(); ++it)
	{
		CAmmo2D* ammo = (CAmmo2D*)*it;
		if (ammo->getActive()) //if got active ammo, set to true
		{
			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::FIREBALL); //play fireball sound
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
			if (cMap2D->SaveMap("Maps/DM2213_SAVEGAME.csv", cMap2D->GetCurrentLevel()) == false)
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
void JunglePlanet::PreRender(void)
{
	// Reset the OpenGL rendering environment
	glLoadIdentity();

	// Clear the screen and buffer
	glClearColor(0.2f, 0.4f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Enable 2D texture rendering
	glEnable(GL_TEXTURE_2D);
}

/**
 @brief Render Render this instance
 */
void JunglePlanet::Render(void)
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
void JunglePlanet::PostRender(void)
{
}

void JunglePlanet::PlayerInteractWithMap(void)
{
	switch (cMap2D->GetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x))
	{
	case CMap2D::TILE_INDEX::POISON_SPROUT:
	case CMap2D::TILE_INDEX::POISON_FOG:
		if (poisonLevelIncreaseCooldown <= 0) //poison lvl increase cooldown up
		{
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("PoisonLevel"); 
			//if not at max poison lvl yet
			if (cInventoryItemPlanet->GetCount() != cInventoryItemPlanet->GetMaxCount())
			{
				cInventoryItemPlanet->Add(1); //increase poison level by 1
				poisonLevelIncreaseCooldown = poisonLevelIncreaseMaxCooldown; //reset poison level increase cooldown
			}
		}
		break;
	case CMap2D::TILE_INDEX::POISON_EXPLOSION: //same as poison sproute and fog but deals constant damage as well
		if (poisonLevelIncreaseCooldown <= 0) //poison lvl increase cooldown up
		{
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("PoisonLevel");
			//if not at max poison lvl yet
			if (cInventoryItemPlanet->GetCount() != cInventoryItemPlanet->GetMaxCount())
			{
				cInventoryItemPlanet->Add(1); //increase poison level by 1
				poisonLevelIncreaseCooldown = poisonLevelIncreaseMaxCooldown; //reset poison level increase cooldown
			}
		}
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Health");
		cInventoryItemPlanet->Remove(1); //deplete player's health
		break;
	case CMap2D::TILE_INDEX::RIVER_WATER:
	case CMap2D::TILE_INDEX::ENEMY_WAYPOINT_RIVER_WATER:
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::SPLASH); //play sound of player walking in water

		//pop up only appears in tutorial lvl
		if (cMap2D->GetCurrentLevel() == TUTORIAL)
		{
			cGUI_Scene2D->setTutorialPopupJungle(CGUI_Scene2D::JUNGLE_TUTORIAL_POPUP::RIVER_WATER); //trigger river water pop up
		}

		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Health");
		cInventoryItemPlanet->Add(1); //increase health by 1 for every frame in river water

		//cure poison
		if (poisonLevelIncreaseCooldown <= 0) //poison lvl increase cooldown up
		{
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("PoisonLevel");
			//if poison lvl not at 0
			if (cInventoryItemPlanet->GetCount() > 0)
			{
				cInventoryItemPlanet->Remove(1); //decrease poison level by 1
				poisonLevelIncreaseCooldown = poisonLevelIncreaseMaxCooldown; //reset poison level increase/decrease cooldown

				//if removed poison entirely
				if (cInventoryItemPlanet->GetCount() == 0)
				{
					cPlayer2D->SetColour(CPlayer2D::COLOUR::WHITE); //remove purple poison colour defintely
				}
			}
		}

		//can pick up river water
		if (cKeyboardController->IsKeyPressed(GLFW_KEY_Q))
		{
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("RiverWater");
			//if river water inventory not full yet
			if (cInventoryItemPlanet->GetCount() != cInventoryItemPlanet->GetMaxCount())
			{
				cInventoryItemPlanet->Add(1); //collect 1 cup of river water
				std::cout << "COLLECTED RIVER WATER: " << cInventoryItemPlanet->GetCount() << std::endl;
			}
		}
		break;
	case CMap2D::TILE_INDEX::UNBLOOMED_BOUNCY_BLOOM: 
		//pop up only appears in tutorial lvl
		if (cMap2D->GetCurrentLevel() == TUTORIAL)
		{
			cGUI_Scene2D->setTutorialPopupJungle(CGUI_Scene2D::JUNGLE_TUTORIAL_POPUP::BOUNCY_BLOOM); //trigger bouncy bloom pop up
		}

		//can make bouncy bloom bloom if using river water on it while standing on it
		if (cKeyboardController->IsKeyPressed(GLFW_KEY_Q))
		{
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("RiverWater");
			//if player has river water
			if (cInventoryItemPlanet->GetCount() > 0)
			{
				cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::WATER_FLOWER); //play sound of watering flower
				cMap2D->SetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x, CMap2D::TILE_INDEX::BLOOMED_BOUNCY_BLOOM); //make flower bloom
				cInventoryItemPlanet->Remove(1); //use 1 cup of river water
			}
		}
		break;
	case CMap2D::TILE_INDEX::ROCK:
		//pop up only appears in tutorial lvl
		if (cMap2D->GetCurrentLevel() == TUTORIAL)
		{
			cGUI_Scene2D->setTutorialPopupJungle(CGUI_Scene2D::JUNGLE_TUTORIAL_POPUP::ROCK); //trigger rock pop up
		}

		//if player wants to tie a vine to the rock
		if (cKeyboardController->IsKeyPressed(GLFW_KEY_F))
		{
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Vine");
			//if player has a vine
			if (cInventoryItemPlanet->GetCount() > 0)
			{
				// runs if there is empty space on the left of the rock
				if (cMap2D->GetMapInfo(cPlayer2D->vec2Index.y - 1, cPlayer2D->vec2Index.x - 1) == 0)
				{
					// changes rock to rock with vine tied toward the left
					cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::ROCK, CMap2D::TILE_INDEX::ROCK_VINE_LEFT,
						cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.y + 1, cPlayer2D->vec2Index.x, cPlayer2D->vec2Index.x + 1);
					// adds vine for player to climb up
					// checks if there is any ground below the vine length
					unsigned int groundHeight = cMap2D->FindGround(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x - 1);
					if (groundHeight < 0)
					{
						cout << "There is a hole in the ground at column " << cPlayer2D->vec2Index.x - 1 << endl;
					}
					else
					{
						cMap2D->ReplaceTiles(0, CMap2D::TILE_INDEX::VINE_CORNER_LEFT,
							cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.y + 1, cPlayer2D->vec2Index.x - 1, cPlayer2D->vec2Index.x);
						cMap2D->ReplaceTiles(0, CMap2D::TILE_INDEX::VINE_LEFT, groundHeight, 
							cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x - 1, cPlayer2D->vec2Index.x);
					}
				}
				// runs if there is empty space on the right of the rock
				else if (cMap2D->GetMapInfo(cPlayer2D->vec2Index.y - 1, cPlayer2D->vec2Index.x + 1) == 0)
				{
					// changes rock to rock with vine tied toward the  right
					cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::ROCK, CMap2D::TILE_INDEX::ROCK_VINE_RIGHT,
						cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.y + 1, cPlayer2D->vec2Index.x, cPlayer2D->vec2Index.x + 1);
					// adds vine for player to climb up
					// checks if there is any ground below the vine length
					unsigned int groundHeight = cMap2D->FindGround(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x + 1);
					if (groundHeight < 0)
					{
						cout << "There is a hole in the ground at column " << cPlayer2D->vec2Index.x + 1 << endl;
					}
					else
					{
						cMap2D->ReplaceTiles(0, CMap2D::TILE_INDEX::VINE_CORNER_RIGHT,
							cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.y + 1, cPlayer2D->vec2Index.x + 1, cPlayer2D->vec2Index.x + 2);
						cMap2D->ReplaceTiles(0, CMap2D::TILE_INDEX::VINE_RIGHT, groundHeight,
							cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x + 1, cPlayer2D->vec2Index.x + 2);
					}
				}

				cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::TIE_VINE); //play sound of tying vine around rock
				cInventoryItemPlanet->Remove(1); //use 1 vine
			}
		}
		break;
	case CMap2D::TILE_INDEX::SHOOTING_POPUP:
		//pop up only appears in tutorial lvl
		if (cMap2D->GetCurrentLevel() == TUTORIAL)
		{
			cGUI_Scene2D->setTutorialPopupJungle(CGUI_Scene2D::JUNGLE_TUTORIAL_POPUP::SHOOT); //shooting pop up
		}
		break;
	default:
		break;
	}
}

//to decide which map, aka which level to render
void JunglePlanet::DecideLevel(bool tutorial)
{
	//if it is to load tutorial level
	if (tutorial)
	{
		cMap2D->SetCurrentLevel(TUTORIAL); //tutorial level
		cGUI_Scene2D->setTutorialPopupJungle(CGUI_Scene2D::JUNGLE_TUTORIAL_POPUP::CHECKPOINT); //start with checkpoint pop up
	}
	else //randomise between level 1 and 2
	{
		//random between 2 numbers to set us Scrap metal or battery
			//according to which number type is set to, load which texture
		int randomState = rand() % 100;
		if (randomState < 50)
		{
			cMap2D->SetCurrentLevel(LEVEL1); //level 1
		}
		else
		{
			cMap2D->SetCurrentLevel(LEVEL2); //level 2
		}
	}

	cPlayer2D->ResetRespawn(); //spawn player at the right starting location
}

bool JunglePlanet::getIsOnShip(void)
{
	return cGUI_Scene2D->getGoOnShip();
}

void JunglePlanet::SetResourcesBack(void)
{

	if (cInventoryManagerPlanet->Check("ScrapMetal")) {
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("ScrapMetal");
		if (cInventoryManager->Check("ScrapMetal")) {
			cInventoryItem = cInventoryManager->GetItem("ScrapMetal");
			cInventoryItem->Set(cInventoryItemPlanet->GetCount());
		}
		else {
			std::cout << "Item does not exist in inventory manager\n";
		}
	}
	else {
		std::cout << "Item does not exist in inventory manager planet \n";
	}
	
	if (cInventoryManagerPlanet->Check("Battery")) {
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Battery");
		if (cInventoryManager->Check("Battery")) {
			cInventoryItem = cInventoryManager->GetItem("Battery");
			cInventoryItem->Set(cInventoryItemPlanet->GetCount());
		}
		else {
			std::cout << "Item does not exist in inventory manager\n";
		}
	}
	else {
		std::cout << "Item does not exist in inventory manager planet \n";
	}

	if (cInventoryManagerPlanet->Check("Ironwood")) {
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Ironwood");
		if (cInventoryManager->Check("Ironwood")) {
			cInventoryItem = cInventoryManager->GetItem("Ironwood");
			cInventoryItem->Set(cInventoryItemPlanet->GetCount());
		}
		else {
			std::cout << "Item does not exist in inventory manager\n";
		}
	}
	else {
		std::cout << "Item does not exist in inventory manager planet \n";
	}

	CGameInfo::GetInstance()->ExportIM(cInventoryManager);
}
