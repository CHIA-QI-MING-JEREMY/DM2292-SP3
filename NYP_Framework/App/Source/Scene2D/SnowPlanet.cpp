/**
 CScene2D
 @brief A class which manages the 2D game scene
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "SnowPlanet.h"
#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

#include "System\filesystem.h"


/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
SnowPlanet::SnowPlanet(void)
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
SnowPlanet::~SnowPlanet(void)
{
	cSoundController->StopSoundByID(CSoundController::SOUND_LIST::BACKGROUNDSNOW);
	
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
bool SnowPlanet::Init(void)
{
	//Create Background Entity
	background = new CBackgroundEntity("Image/SnowPlanet/SnowBackground.png");
	background->SetShader("Shader2D");
	background->Init();
	healthDropTimer = 3.f;
	healthTimer = 0.f;
	tempTimer = 0.f;
	tempDropTimer = 5.0f;
	turnBerserkOffTimer = 5.f;
	turnBerserkTimer = 0.f;
	// Include Shader Manager
	CShaderManager::GetInstance()->Use("Shader2D");

	cInventoryManagerPlanet = CInventoryManagerPlanet::GetInstance();

	// Create and initialise the CGUI_Scene2D
	cGUI_Scene2D = CGUI_Scene2D::GetInstance();
	// Initialise the instance
	if (cGUI_Scene2D->Init() == false)
	{
		cout << "Failed to load CGUI_Scene2D" << endl;
		return false;
	}
	cGUI_Scene2D->setPlanetNum(3);


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
	if (cMap2D->LoadMap("Maps/DM2292_Map_Snow_Tutorial.csv", TUTORIAL) == false)
	{
		// The loading of a map has failed. Return false
		cout << "Failed to load Snow Map Tutorial Level" << endl;
		return false;
	}
	// Load the map into an array
	if (cMap2D->LoadMap("Maps/DM2292_Map_Snow_01.csv", LEVEL1) == false)
	{
		// The loading of a map has failed. Return false
		cout << "Failed to load Snow Map Level 01" << endl;
		return false;
	}
	//// Load the map into an array
	if (cMap2D->LoadMap("Maps/DM2292_Map_Snow_02.csv", LEVEL2) == false)
	{
		// The loading of a map has failed. Return false
		cout << "Failed to load Snow Map Level 02" << endl;
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
			SnowEnemy2DSWB* snowEnemy2DSWB = new SnowEnemy2DSWB();
			// Pass shader to cEnemy2D
			snowEnemy2DSWB->SetShader("Shader2D_Colour");
			// Initialise the instance
			if (snowEnemy2DSWB->Init() == true)
			{
				snowEnemy2DSWB->SetPlayer2D(cPlayer2D);
				enemies.push_back(snowEnemy2DSWB); //push each enemy into the individual enemy vector
			}
			else
			{
				// Break out of this loop if all enemies have been loaded
				break;
			}
		}
		while (true)
		{
			SnowEnemy2DSWW* snowEnemy2DSWW = new SnowEnemy2DSWW();
			// Pass shader to cEnemy2D
			snowEnemy2DSWW->SetShader("Shader2D_Colour");
			// Initialise the instance
			if (snowEnemy2DSWW->Init() == true)
			{
				snowEnemy2DSWW->SetPlayer2D(cPlayer2D);
				enemies.push_back(snowEnemy2DSWW); //push each enemy into the individual enemy vector
			}
			else
			{
				// Break out of this loop if all enemies have been loaded
				break;
			}
		}
		while (true)
		{
			SnowEnemy2DSWBS* snowEnemy2DSWBS = new SnowEnemy2DSWBS();
			// Pass shader to cEnemy2D
			snowEnemy2DSWBS->SetShader("Shader2D_Colour");
			// Initialise the instance
			if (snowEnemy2DSWBS->Init() == true)
			{
				snowEnemy2DSWBS->SetPlayer2D(cPlayer2D);
				enemies.push_back(snowEnemy2DSWBS); //push each enemy into the individual enemy vector
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
	cPlayer2D->ResetRespawn();

	// Get the handler for the cInventoryManager
	cInventoryManager = CGameInfo::GetInstance()->ImportIM();


	// Initialise the Physics
	cPhysics2D.Init();

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
	//common sounds
	cSoundController = CSoundController::GetInstance();
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\jumpland.ogg"), CSoundController::SOUND_LIST::LAND, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\jump.ogg"), CSoundController::SOUND_LIST::JUMP, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\enemyjump.ogg"), CSoundController::SOUND_LIST::ENEMY_JUMP, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\enemyjumpland.ogg"), CSoundController::SOUND_LIST::ENEMY_LAND, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\enemywalk.ogg"), CSoundController::SOUND_LIST::ENEMY_FOOTSTEPS, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\StowItemInPocket.ogg"), CSoundController::SOUND_LIST::COLLECT_ITEM, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\damage.ogg"), CSoundController::SOUND_LIST::TAKE_DAMAGE, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\climb.ogg"), CSoundController::SOUND_LIST::CLIMB, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\walk.ogg"), CSoundController::SOUND_LIST::FOOTSTEPS, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\checkpoint.ogg"), CSoundController::SOUND_LIST::HIT_CHECKPOINT, true);

	//Planet Sounds
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\SnowPlanet\\berserk.ogg"), CSoundController::SOUND_LIST::BERSERK, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\SnowPlanet\\shieldSnow.ogg"), CSoundController::SOUND_LIST::SHIELD, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\SnowPlanet\\icefreeze.ogg"), CSoundController::SOUND_LIST::FREEZE, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\SnowPlanet\\wolfbite.ogg"), CSoundController::SOUND_LIST::WOLFBITE, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\SnowPlanet\\heal.ogg"), CSoundController::SOUND_LIST::WOLFHEAL, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\SnowPlanet\\wolfpain.ogg"), CSoundController::SOUND_LIST::WOLFPAIN, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\SnowPlanet\\shieldEnemy.ogg"), CSoundController::SOUND_LIST::WOLFSHIELD, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\SnowPlanet\\snowShoot.ogg"), CSoundController::SOUND_LIST::PLAYERSNOWSHOOT, true);

	cSoundController->LoadSound(FileSystem::getPath("Sounds\\SnowPlanet\\backgroundSnow.ogg"), CSoundController::SOUND_LIST::BACKGROUNDSNOW, true, true);
	cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BACKGROUNDSNOW); // plays BGM on repeat


	return true;
}

/**
@brief Update Update this instance
*/
bool SnowPlanet::Update(const double dElapsedTime)
{
	cGUI_Scene2D->setTutorialPopupSnow(CGUI_Scene2D::SNOW_TUTORIAL_POPUP::NOTHING);
	if (cMap2D->GetCurrentLevel() == TUTORIAL)
	{
		if (cPhysics2D.CalculateDistance(cPlayer2D->vec2Index, cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNINTRO1)) < 2.f &&
			cPlayer2D->vec2Index.y - cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNINTRO1).y < 2.f) //player isn't much higher than the invisible pop up trigger point
		{
			cGUI_Scene2D->setTutorialPopupSnow(CGUI_Scene2D::SNOW_TUTORIAL_POPUP::SIGNINTRO1);
		}
		if (cPhysics2D.CalculateDistance(cPlayer2D->vec2Index, cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNINTRO2)) < 2.f &&
			cPlayer2D->vec2Index.y - cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNINTRO2).y < 2.f) //player isn't much higher than the invisible pop up trigger point
		{
			cGUI_Scene2D->setTutorialPopupSnow(CGUI_Scene2D::SNOW_TUTORIAL_POPUP::SIGNINTRO2);
		}
		if (cPhysics2D.CalculateDistance(cPlayer2D->vec2Index, cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNINTRO3)) < 2.f &&
			cPlayer2D->vec2Index.y - cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNINTRO3).y < 2.f) //player isn't much higher than the invisible pop up trigger point
		{
			cGUI_Scene2D->setTutorialPopupSnow(CGUI_Scene2D::SNOW_TUTORIAL_POPUP::SIGNINTRO3);
		}
		if (cPhysics2D.CalculateDistance(cPlayer2D->vec2Index, cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNINTRO4)) < 2.f &&
			cPlayer2D->vec2Index.y - cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNINTRO4).y < 2.f) //player isn't much higher than the invisible pop up trigger point
		{
			cGUI_Scene2D->setTutorialPopupSnow(CGUI_Scene2D::SNOW_TUTORIAL_POPUP::SIGNINTRO4);
		}
		if (cPhysics2D.CalculateDistance(cPlayer2D->vec2Index, cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNINTRO5)) < 2.f &&
			cPlayer2D->vec2Index.y - cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNINTRO5).y < 2.f) //player isn't much higher than the invisible pop up trigger point
		{
			cGUI_Scene2D->setTutorialPopupSnow(CGUI_Scene2D::SNOW_TUTORIAL_POPUP::SIGNINTRO5);
		}
		if (cPhysics2D.CalculateDistance(cPlayer2D->vec2Index, cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNTUT1)) < 2.f &&
			cPlayer2D->vec2Index.y - cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNTUT1).y < 2.f) //player isn't much higher than the invisible pop up trigger point
		{
			cGUI_Scene2D->setTutorialPopupSnow(CGUI_Scene2D::SNOW_TUTORIAL_POPUP::SIGNTUT1);
		}
		if (cPhysics2D.CalculateDistance(cPlayer2D->vec2Index, cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNTUT2)) < 2.f &&
			cPlayer2D->vec2Index.y - cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNTUT2).y < 2.f) //player isn't much higher than the invisible pop up trigger point
		{
			cGUI_Scene2D->setTutorialPopupSnow(CGUI_Scene2D::SNOW_TUTORIAL_POPUP::SIGNTUT2);
		}
		if (cPhysics2D.CalculateDistance(cPlayer2D->vec2Index, cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNTUT3)) < 2.f &&
			cPlayer2D->vec2Index.y - cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNTUT3).y < 2.f) //player isn't much higher than the invisible pop up trigger point
		{
			cGUI_Scene2D->setTutorialPopupSnow(CGUI_Scene2D::SNOW_TUTORIAL_POPUP::SIGNTUT3);
		}
		if (cPhysics2D.CalculateDistance(cPlayer2D->vec2Index, cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNTUT4)) < 2.f &&
			cPlayer2D->vec2Index.y - cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNTUT4).y < 2.f) //player isn't much higher than the invisible pop up trigger point
		{
			cGUI_Scene2D->setTutorialPopupSnow(CGUI_Scene2D::SNOW_TUTORIAL_POPUP::SIGNTUT4);
		}
		if (cPhysics2D.CalculateDistance(cPlayer2D->vec2Index, cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNTUT5)) < 2.f &&
			cPlayer2D->vec2Index.y - cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNTUT5).y < 2.f) //player isn't much higher than the invisible pop up trigger point
		{
			cGUI_Scene2D->setTutorialPopupSnow(CGUI_Scene2D::SNOW_TUTORIAL_POPUP::SIGNTUT5);
		}
		if (cPhysics2D.CalculateDistance(cPlayer2D->vec2Index, cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNTUT6)) < 2.f &&
			cPlayer2D->vec2Index.y - cMap2D->GetTilePosition(CMap2D::TILE_INDEX::SIGNTUT6).y < 2.f) //player isn't much higher than the invisible pop up trigger point
		{
			cGUI_Scene2D->setTutorialPopupSnow(CGUI_Scene2D::SNOW_TUTORIAL_POPUP::SIGNTUT6);
		}
	}

	if (cPlayer2D->getModeOfPlayer() !=CPlayer2D::MODE::NORMAL && turnBerserkOffTimer==5.0f) {
		if (cKeyboardController->IsKeyPressed(GLFW_KEY_A) || cKeyboardController->IsKeyPressed(GLFW_KEY_W) || cKeyboardController->IsKeyPressed(GLFW_KEY_S) || cKeyboardController->IsKeyPressed(GLFW_KEY_D) || cKeyboardController->IsKeyPressed(GLFW_KEY_SPACE)) {
			cout << "Switching back to NORMAL player mode" << endl;
			cPlayer2D->setModeOfPlayer(CPlayer2D::MODE::NORMAL);
			cPlayer2D->SetColour(CPlayer2D::COLOUR::WHITE);
		}
	}
	cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("freeze");
	if (cInventoryItemPlanet->GetCount() > 0 && cKeyboardController->IsKeyReleased(GLFW_KEY_F)) {
		cInventoryItemPlanet->Remove(1);
		cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::WATER, CMap2D::TILE_INDEX::ICE);
		cMap2D->ReplaceTiles(CMap2D::TILE_INDEX::WATER_TOP, CMap2D::TILE_INDEX::ICE);
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::FREEZE);
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
	if (cPlayer2D->getModeOfPlayer() !=CPlayer2D::MODE::SHIELD && cPlayer2D->getModeOfPlayer() != CPlayer2D::MODE::BERSERKSHIELD) {
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("shield");
		if (cKeyboardController->IsKeyReleased(GLFW_KEY_Q) && cPlayer2D->getModeOfPlayer() != CPlayer2D::MODE::BERSERK && cInventoryItemPlanet->GetCount()>0) {
			cInventoryItemPlanet->Remove(1);
			cout << "Shield Mode Activated" << endl;
			cPlayer2D->setModeOfPlayer(CPlayer2D::MODE::SHIELD);
			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::SHIELD);
		}
		else if (cKeyboardController->IsKeyReleased(GLFW_KEY_Q) && cPlayer2D->getModeOfPlayer() == CPlayer2D::MODE::BERSERK && cInventoryItemPlanet->GetCount() > 0) {
			cInventoryItemPlanet->Remove(1);
			cout << "Berserk Shield Mode Activated" << endl;
			cPlayer2D->setModeOfPlayer(CPlayer2D::MODE::BERSERKSHIELD);
			cPlayer2D->SetColour(CPlayer2D::COLOUR::PINK);
			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::SHIELD);
		}
	}
	if (cPlayer2D->getModeOfPlayer() != CPlayer2D::MODE::BERSERK && cPlayer2D->getModeOfPlayer() != CPlayer2D::MODE::BERSERKSHIELD) {
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("berserk");
		if (cKeyboardController->IsKeyReleased(GLFW_KEY_G)) {
			if (cInventoryItemPlanet->GetCount() > 0) {
				cInventoryItemPlanet->Remove(1);
				cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BERSERK);
				cout << "Turning to Berserk Mode" << endl;
				cPlayer2D->setModeOfPlayer(CPlayer2D::MODE::BERSERK);
				cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Health");
				cInventoryItemPlanet->Add(15);
				if (cInventoryItemPlanet->GetCount() > cInventoryItemPlanet->GetMaxCount()) {
					cInventoryItemPlanet->setCount(cInventoryItemPlanet->GetMaxCount());
				}
			}
		}
	}
	if (cPlayer2D->getModeOfPlayer() == CPlayer2D::MODE::BERSERK || cPlayer2D->getModeOfPlayer()==CPlayer2D::MODE::BERSERKSHIELD) {
		turnBerserkOffTimer -= dElapsedTime;
		cPlayer2D->SetColour(CPlayer2D::COLOUR::PINK);
		cout << "TurnBerserkOffTimer:" << turnBerserkOffTimer << endl;
	}
	if (turnBerserkOffTimer <= turnBerserkTimer) {
		cout << "Turning to Normal Mode" << endl;
		cPlayer2D->setModeOfPlayer(CPlayer2D::MODE::NORMAL);
	}
	if (cPlayer2D->getModeOfPlayer() == CPlayer2D::MODE::NORMAL) {
		turnBerserkOffTimer = 5.0f;
		cPlayer2D->SetColour(CPlayer2D::COLOUR::WHITE);
	}
	cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Temperature");
	if (tempDropTimer >= tempTimer && cInventoryItemPlanet->GetCount() > 0) {
		tempDropTimer -= dElapsedTime;
	}
	if (tempDropTimer < tempTimer && cPlayer2D->getModeOfPlayer() != CPlayer2D::MODE::BERSERK && cPlayer2D->getModeOfPlayer() != CPlayer2D::MODE::BERSERKSHIELD) {
		if (cInventoryItemPlanet->GetCount() > 0) {
			cInventoryItemPlanet->Remove(3);
		}
		else if (cInventoryItemPlanet->GetCount() <= 0) {
			cInventoryItemPlanet->setCount(0);
		}
		tempDropTimer = 5.0f;
	}
	if (cInventoryItemPlanet->GetCount() <= 0) {
		healthDropTimer -= dElapsedTime;
		cPlayer2D->SetColour(CPlayer2D::COLOUR::SKYBLUE);
	}
	else if (cPlayer2D->getModeOfPlayer() != CPlayer2D::MODE::BERSERK && cPlayer2D->getModeOfPlayer() != CPlayer2D::MODE::BERSERKSHIELD) {
		cPlayer2D->SetColour(CPlayer2D::COLOUR::WHITE);
	}
	if (healthDropTimer < healthTimer && cPlayer2D->getModeOfPlayer() != CPlayer2D::MODE::BERSERK && cPlayer2D->getModeOfPlayer() != CPlayer2D::MODE::BERSERKSHIELD) {
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Health");
		cInventoryItemPlanet->Remove(5);
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::TAKE_DAMAGE);
		healthDropTimer = 3.f;
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
	if (CMap2D::TILE_INDEX::ICE == cMap2D->GetMapInfo(cPlayer2D->vec2Index.y-1,cPlayer2D->vec2Index.x)) {
		CSettings::GetInstance()->NUM_STEPS_PER_TILE_XAXIS = 6.0;
		CSettings::GetInstance()->MICRO_STEP_XAXIS = 0.01041667f;
	}
	else {
		CSettings::GetInstance()->NUM_STEPS_PER_TILE_XAXIS = 8.0;
		CSettings::GetInstance()->MICRO_STEP_XAXIS = 0.0078125f;
	}

	// Call all of the cEnemy2D's update methods before Map2D
	// as we want to capture the updates before Map2D update
	for (unsigned int i = 0; i < enemyVectors[cMap2D->GetCurrentLevel()].size(); i++)
	{

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
					if (enemyVectors[cMap2D->GetCurrentLevel()][i]->getShieldActivated() && enemyVectors[cMap2D->GetCurrentLevel()][i]->getType()!=CEnemy2D::ENEMYTYPE::BROWN) {
						enemyVectors[cMap2D->GetCurrentLevel()][i]->setHealth(enemyVectors[cMap2D->GetCurrentLevel()][i]->getHealth() - 0); //every hit takes off 0 HP
						cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::WOLFPAIN);
						ammo->setActive(false);
					}
					else if (cPlayer2D->getModeOfPlayer() != CPlayer2D::MODE::BERSERK && cPlayer2D->getModeOfPlayer() != CPlayer2D::MODE::BERSERKSHIELD) {
						enemyVectors[cMap2D->GetCurrentLevel()][i]->setHealth(enemyVectors[cMap2D->GetCurrentLevel()][i]->getHealth() - 5); //every hit takes off 5 HP
						cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::WOLFPAIN);
						ammo->setActive(false);
					}
					else {
						enemyVectors[cMap2D->GetCurrentLevel()][i]->setHealth(enemyVectors[cMap2D->GetCurrentLevel()][i]->getHealth() - 10); //every hit takes off 10 HP
						cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::WOLFPAIN);
						ammo->setActive(false);
					}
				}
			}
		}

		// deletes enemies if they die
		if (enemyVectors[cMap2D->GetCurrentLevel()][i]->getHealth() <= 0)
		{
			//if this isn't the last enemy in this level
			if (enemyVectors[cMap2D->GetCurrentLevel()].size() > 1)
			{
				if (enemyVectors[cMap2D->GetCurrentLevel()][i]->getType() != CEnemy2D::ENEMYTYPE::BROWN) {
					if (enemyVectors[cMap2D->GetCurrentLevel()][i]->getType() != CEnemy2D::ENEMYTYPE::BOSS) {
						//20% chance to drop scrap metal, 20% chance to drop battery, 10% chance to drop ice crystal
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
							CResource* res = new CResource(CResource::RESOURCE_TYPE::ICE_CRYSTAL); //create new ironwood resource
							res->setPosition(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index, enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2NumMicroSteps);
							//set resource's position where enemy's position is
							res->SetShader("Shader2D_Colour"); //set shader
							resourceVectors[cMap2D->GetCurrentLevel()].push_back(res); //push this new resource into the resource vector for this level
						}
					}
					else {
						//20% chance to drop scrap metal, 20% chance to drop battery, 10% chance to drop ice crystal
						int resourceType = rand() % 20;
						std::cout << resourceType << std::endl;
						if (resourceType < 4) //0 1 2 3
						{
							CResource* res = new CResource(CResource::RESOURCE_TYPE::SCRAP_METAL); //create new scrap metal resource
							res->setPosition(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index, enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2NumMicroSteps);
							//set resource's position where enemy's position is
							res->SetShader("Shader2D_Colour"); //set shader
							resourceVectors[cMap2D->GetCurrentLevel()].push_back(res); //push this new resource into the resource vector for this level
													//confirm drop an ice crystal
							CResource* res2 = new CResource(CResource::RESOURCE_TYPE::SCRAP_METAL); //create new ice crystal resource
							res2->setPosition(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index + glm::vec2(0.5, 0.5), enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2NumMicroSteps);
							//set resource's position where enemy's position is
							res2->SetShader("Shader2D_Colour"); //set shader
							resourceVectors[cMap2D->GetCurrentLevel()].push_back(res2); //push this new resource into the resource vector for this level
						}
						else if (resourceType > 7 && resourceType < 12) //8 9 10 11
						{
							CResource* res = new CResource(CResource::RESOURCE_TYPE::BATTERY); //create new battery resource
							res->setPosition(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index, enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2NumMicroSteps);
							//set resource's position where enemy's position is
							res->SetShader("Shader2D_Colour"); //set shader
							resourceVectors[cMap2D->GetCurrentLevel()].push_back(res); //push this new resource into the resource vector for this level

													//confirm drop an ice crystal
							CResource* res2 = new CResource(CResource::RESOURCE_TYPE::BATTERY); //create new ice crystal resource
							res2->setPosition(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index + glm::vec2(0.5, 0.5), enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2NumMicroSteps);
							//set resource's position where enemy's position is
							res2->SetShader("Shader2D_Colour"); //set shader
							resourceVectors[cMap2D->GetCurrentLevel()].push_back(res2); //push this new resource into the resource vector for this level
						}
						else if (resourceType > 16 && resourceType < 19) //17 18
						{
							CResource* res = new CResource(CResource::RESOURCE_TYPE::ICE_CRYSTAL); //create new ironwood resource
							res->setPosition(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index, enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2NumMicroSteps);
							//set resource's position where enemy's position is
							res->SetShader("Shader2D_Colour"); //set shader
							resourceVectors[cMap2D->GetCurrentLevel()].push_back(res); //push this new resource into the resource vector for this level
													//confirm drop an ice crystal
							CResource* res2 = new CResource(CResource::RESOURCE_TYPE::ICE_CRYSTAL); //create new ice crystal resource
							res2->setPosition(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index + glm::vec2(0.5, 0.5), enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2NumMicroSteps);
							//set resource's position where enemy's position is
							res2->SetShader("Shader2D_Colour"); //set shader
							resourceVectors[cMap2D->GetCurrentLevel()].push_back(res2); //push this new resource into the resource vector for this level
						}
					}
				}
				else {
					CResource* res = new CResource(CResource::RESOURCE_TYPE::FUR); //create new fur resource
					res->setPosition(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index, enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2NumMicroSteps);
					//set resource's position where enemy's position is
					res->SetShader("Shader2D_Colour"); //set shader
					resourceVectors[cMap2D->GetCurrentLevel()].push_back(res); //push this new resource into the resource vector for this level
				}
			}
			//if this is the last enemy
			else if (enemyVectors[cMap2D->GetCurrentLevel()].size() == 1)
			{
				if (enemyVectors[cMap2D->GetCurrentLevel()][i]->getType() != CEnemy2D::ENEMYTYPE::BOSS) {
					//confirm drop an ice crystal
					CResource* res = new CResource(CResource::RESOURCE_TYPE::ICE_CRYSTAL); //create new ice crystal resource
					res->setPosition(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index, enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2NumMicroSteps);
					//set resource's position where enemy's position is
					res->SetShader("Shader2D_Colour"); //set shader
					resourceVectors[cMap2D->GetCurrentLevel()].push_back(res); //push this new resource into the resource vector for this level
				}
				else {
					//confirm drop an ice crystal
					CResource* res = new CResource(CResource::RESOURCE_TYPE::ICE_CRYSTAL); //create new ice crystal resource
					res->setPosition(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index, enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2NumMicroSteps);
					//set resource's position where enemy's position is
					res->SetShader("Shader2D_Colour"); //set shader
					resourceVectors[cMap2D->GetCurrentLevel()].push_back(res); //push this new resource into the resource vector for this level
										//confirm drop an ice crystal
					CResource* res2 = new CResource(CResource::RESOURCE_TYPE::ICE_CRYSTAL); //create new ice crystal resource
					res2->setPosition(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index+glm::vec2(0.5,0.5), enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2NumMicroSteps);
					//set resource's position where enemy's position is
					res2->SetShader("Shader2D_Colour"); //set shader
					resourceVectors[cMap2D->GetCurrentLevel()].push_back(res2); //push this new resource into the resource vector for this level
				}
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

	PlayerInteractWithMap();
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
void SnowPlanet::PreRender(void)
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
void SnowPlanet::Render(void)
{
	background->PreRender();
	background->Render();
	background->PostRender();
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
void SnowPlanet::PostRender(void)
{
}

void SnowPlanet::PlayerInteractWithMap(void)
{
	switch (cMap2D->GetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x))
	{
	case CMap2D::TILE_INDEX::WATER_TOP:
	case CMap2D::TILE_INDEX::WATER:
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Health");
		cInventoryItemPlanet->Remove(1);
		break;
	default:
		break;
	}
}

//to decide which map, aka which level to render
void SnowPlanet::DecideLevel(bool tutorial)
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
	cPlayer2D->ResetRespawn();
}

bool SnowPlanet::getIsOnShip(void)
{
	return cGUI_Scene2D->getGoOnShip();
}

void SnowPlanet::SetResourcesBack(void)
{
	if (cInventoryManagerPlanet->GetItem("Lives")->GetCount() != 0) {
		if (cInventoryManagerPlanet->Check("ScrapMetal") && CInventoryManager::GetInstance()->GetItem("Storage")->GetCount() < CInventoryManager::GetInstance()->GetItem("Storage")->GetMaxCount()) {
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("ScrapMetal");
			if (cInventoryManager->Check("ScrapMetal")) {
				cInventoryItem = cInventoryManager->GetItem("ScrapMetal");
				cInventoryItem->Add(cInventoryItemPlanet->GetCount());
				CInventoryManager::GetInstance()->GetItem("Storage")->Add(cInventoryItem->GetCount());
			}
			else {
				std::cout << "Item does not exist in inventory manager\n";
			}
		}
		else {
			std::cout << "Item does not exist in inventory manager planet \n";
		}

		if (cInventoryManagerPlanet->Check("Battery") && CInventoryManager::GetInstance()->GetItem("Storage")->GetCount() < CInventoryManager::GetInstance()->GetItem("Storage")->GetMaxCount()) {
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Battery");
			if (cInventoryManager->Check("Battery")) {
				cInventoryItem = cInventoryManager->GetItem("Battery");
				cInventoryItem->Add(cInventoryItemPlanet->GetCount());
				CInventoryManager::GetInstance()->GetItem("Storage")->Add(cInventoryItem->GetCount());
			}
			else {
				std::cout << "Item does not exist in inventory manager\n";
			}
		}
		else {
			std::cout << "Item does not exist in inventory manager planet \n";
		}

		if (cInventoryManagerPlanet->Check("IceCrystal") && CInventoryManager::GetInstance()->GetItem("Storage")->GetCount() < CInventoryManager::GetInstance()->GetItem("Storage")->GetMaxCount()) {
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("IceCrystal");
			if (cInventoryManager->Check("IceCrystal")) {
				cInventoryItem = cInventoryManager->GetItem("IceCrystal");
				cInventoryItem->Add(cInventoryItemPlanet->GetCount());
				CInventoryManager::GetInstance()->GetItem("Storage")->Add(cInventoryItem->GetCount());
			}
			else {
				std::cout << "Item does not exist in inventory manager\n";
			}
		}
		else {
			std::cout << "Item does not exist in inventory manager planet \n";
		}
	}
	CGameInfo::GetInstance()->ExportIM(cInventoryManager);
}
