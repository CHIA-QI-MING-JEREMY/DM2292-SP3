/**
 CScene2D
 @brief A class which manages the 2D game scene
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Scene2D.h"
#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

#include "System\filesystem.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CScene2D::CScene2D(void)
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
CScene2D::~CScene2D(void)
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
bool CScene2D::Init(void)
{
	// Include Shader Manager
	CShaderManager::GetInstance()->Use("Shader2D");
	
	maxNumOfMaps = 2;
	// Create and initialise the cMap2D
	cMap2D = CMap2D::GetInstance();
	// Set a shader to this class
	cMap2D->SetShader("Shader2D");
	// Initialise the instance
	if (cMap2D->Init(maxNumOfMaps, CSettings::GetInstance()->NUM_TILES_YAXIS, CSettings::GetInstance()->NUM_TILES_XAXIS) == false)
	{
		cout << "Failed to load CMap2D" << endl;
		return false;
	}
	// Load the map into an array
	if (cMap2D->LoadMap("Maps/DM2213_Map_Jungle_01.csv", 0) == false)
	{
		// The loading of a map has failed. Return false
		cout << "Failed to load Jungle Map Level 01" << endl;
		return false;
	}
	// Load the map into an array
	if (cMap2D->LoadMap("Maps/DM2213_Map_Jungle_02.csv", 1) == false)
	{
		// The loading of a map has failed. Return false
		cout << "Failed to load Jungle Map Level 02" << endl;
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
	for (int i = 0; i < maxNumOfMaps; ++i)
	{
		//current lvele to check for enemies
		cMap2D->SetCurrentLevel(i);

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

		enemyVectors.push_back(enemies); //push the vector of enemies into enemyVectors
	}

	cMap2D->SetCurrentLevel(0); //reset level

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



	// variables
	isAlarmActive = false;
	maxAlarmTimer = 10.0;
	alarmTimer = 0.0;

	return true;
}

/**
@brief Update Update this instance
*/
bool CScene2D::Update(const double dElapsedTime)
{
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
			delete enemyVectors[cMap2D->GetCurrentLevel()][i];
			enemyVectors[cMap2D->GetCurrentLevel()][i] = NULL;
			enemyVectors[cMap2D->GetCurrentLevel()].erase(enemyVectors[cMap2D->GetCurrentLevel()].begin() + i);
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

	// Player Attacks (TO DO)
	if (cPlayer2D->getPlayerAttackStatus())
	{
		for (int i = 0; i < enemyVectors[cMap2D->GetCurrentLevel()].size(); i++)
		{
			if (cPhysics2D.CalculateDistance(enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index, cPlayer2D->vec2Index) <= 1.5f)
			{
				int remainingHealth = enemyVectors[cMap2D->GetCurrentLevel()][i]->getHealth() - 25;
				cout << remainingHealth << endl;
				enemyVectors[cMap2D->GetCurrentLevel()][i]->setHealth(remainingHealth);
			}
		}
		cPlayer2D->setPlayerAttackStatus(false);
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
		for (int i = 0; i < maxNumOfMaps; ++i)
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
		for (int i = 0; i < maxNumOfMaps; ++i)
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
void CScene2D::PreRender(void)
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
void CScene2D::Render(void)
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

		//if enemy has ammo
		if (enemyVectors[cMap2D->GetCurrentLevel()][i]->getType() == CEnemy2D::ENEMYTYPE::LONG_RANGE ||
			enemyVectors[cMap2D->GetCurrentLevel()][i]->getType() == CEnemy2D::ENEMYTYPE::SPECIAL)
		{
			//render enemy ammo
			std::vector<CEnemyAmmo2D*> ammoList = enemyVectors[cMap2D->GetCurrentLevel()][i]->getAmmoList();
			for (std::vector<CEnemyAmmo2D*>::iterator it = ammoList.begin(); it != ammoList.end(); ++it)
			{
				CEnemyAmmo2D* ammo = (CEnemyAmmo2D*)*it;
				if (ammo->getActive())
				{
					ammo->PreRender();
					ammo->Render();
					ammo->PostRender();
				}
			}
		}
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
void CScene2D::PostRender(void)
{
}
