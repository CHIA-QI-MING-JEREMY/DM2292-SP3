/**
 CScene2D
 @brief A class which manages the 2D game scene
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "SceneCombat.h"
#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

#include "System\filesystem.h"
#include "../App/Source/GameStateManagement/GameInfo.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CSceneCombat::CSceneCombat(void)
	: cMap2D(NULL)
	, cPlayer2D(NULL)
	, cKeyboardController(NULL)	
	, cGUI_SceneCombat(NULL)
	, cGameManager(NULL)
	, camera2D(NULL)
	, cShip(NULL)
	, cSoundController(NULL)
{
}

/**
 @brief Destructor
 */
CSceneCombat::~CSceneCombat(void)
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

	if (cShip)
	{
		cShip->Destroy();
		cShip = NULL;
	}
	
	if (camera2D)
	{
		camera2D->Destroy();
		camera2D = NULL;
	}

	if (cGUI_SceneCombat)
	{
		cGUI_SceneCombat->Destroy();
		cGUI_SceneCombat = NULL;
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
bool CSceneCombat::Init(void)
{
	// Include Shader Manager
	CShaderManager::GetInstance()->Use("Shader2D");
	
	// Create and initialise the cMap2D
	cMap2D = CMap2D::GetInstance();
	// Set a shader to this class
	cMap2D->SetShader("Shader2D");
	// Initialise the instance
	if (cMap2D->Init(1, CSettings::GetInstance()->NUM_TILES_YAXIS, CSettings::GetInstance()->NUM_TILES_XAXIS) == false)
	{
		cout << "Failed to load CMap2D" << endl;
		return false;
	}

	if (CGameInfo::GetInstance()->loadedMap == false) {
		CGameInfo::GetInstance()->loadedMap = true;
		// Load the map into an array
		if (cMap2D->LoadMap("Maps/DM2213_Map_Ship.csv", 0) == false)
		{
			// The loading of a map has failed. Return false
			cout << "Failed to load Ship Layout" << endl;
			return false;
		}
	}
	else {
		CGameInfo::GetInstance()->loadedMap = true;
		// Load the map into an array
		if (cMap2D->LoadMap("Maps/DM2213_Map_Ship_SAVEGAME.csv", 0) == false)
		{
			// The loading of a map has failed. Return false
			cout << "Failed to load Ship Layout" << endl;
			return false;
		}
	}

	//Create Background Entity
	background = new CBackgroundEntity("Image/space_bg.png");
	background->SetShader("Shader2D");
	background->Init();

	// Create and initialise the CPlayer2D
	cPlayer2D = CShipPlayer::GetInstance();
	// Pass shader to cPlayer2D
	cPlayer2D->SetShader("Shader2D_Colour");
	// Initialise the instance
	if (cPlayer2D->Init() == false)
	{
		cout << "Failed to load CPlayer2D" << endl;
		return false;
	}

	cMap2D->SetCurrentLevel(0); //reset level
	cMap2D->SetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x, 598);

	// Initialise the Physics
	cPhysics2D.Init();
	
	cShip = CShip::GetInstance();
	cShip->Init();

	// Initalise the enemy
	cShipEnemy = CShipEnemy::GetInstance();
	cShipEnemy->Init();

	// Create and initialise the CGUI_Scene2D
	cGUI_SceneCombat = CGUI_SceneCombat::GetInstance();
	// Initialise the instance
	if (cGUI_SceneCombat->Init() == false)
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
	camera2D->setTargetPos(glm::vec2(cPlayer2D->vec2Index.x * cPlayer2D->vec2NumMicroSteps.x, cPlayer2D->vec2Index.y * cPlayer2D->vec2NumMicroSteps.y));
	camera2D->setTargetZoom(2.0f);

	lState = false;
	blockSelected = glm::vec2(0, 0);
	// max 2 encounters x
	numOfEncounters = rand() % 2 + 1;
	goToNextScene = false;
	state = CURRENT_STATE::SHIP_NOSTATE;
	fightTimeElapsed = 0.0f;
	prevFightTime = 0.0f;
	isDead = false;

	// Audio Stuff
	cSoundController = CSoundController::GetInstance();
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\planetSelection.ogg"), CSoundController::SOUND_LIST::BGM_PLANET, true, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\ShipCombat.ogg"), CSoundController::SOUND_LIST::BGM_FIGHT, true, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\GUI\\Window.ogg"), CSoundController::SOUND_LIST::WINODWOPEN, true, false);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\GUI\\Click.ogg"), CSoundController::SOUND_LIST::BUTTONCLICK, true, false);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\GUI\\ShipHit.ogg"), CSoundController::SOUND_LIST::SHIPHIT, true, false);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\GUI\\ShipGun.ogg"), CSoundController::SOUND_LIST::SHIPGUN, true, false);
	//cSoundController->StopSoundByID(CSoundController::SOUND_LIST::BGM_PLANET);
	cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BGM_PLANET);
	return true;
}

/**
@brief Update Update this instance
*/
bool CSceneCombat::Update(const double dElapsedTime)
{
	//cGUI_Scene2D->setPlanetNum(1);
	// mouse Position demo
	cGUI_SceneCombat->blockPosition = ImVec2(camera2D->getBlockPositionWindow(blockSelected).x, camera2D->getBlockPositionWindow(blockSelected).y);

	float mouseDist;

	if (cGUI_SceneCombat->GuiState == CGUI_SceneCombat::GUI_STATE::noShow) {
		mouseDist = 1.f;
	}
	else {
		mouseDist = 0.3f;
	}

	// mouse Position demo

	int offSetX = 0;

	if (cGUI_SceneCombat->isCombat) {
		offSetX = 2;
	}
	else if (cGUI_SceneCombat->GuiState == CGUI_SceneCombat::GUI_STATE::showWeapons) {
		offSetX = 5;
	}
	else {
		offSetX = 0;
	}

	glm::vec2 camPos = glm::vec2(camera2D->getMousePosition().x - cPlayer2D->vec2Index.x, camera2D->getMousePosition().y - cPlayer2D->vec2Index.y);
	camPos = glm::normalize(camPos);
	camPos = glm::vec2(cPlayer2D->vec2Index.x + camPos.x * mouseDist + offSetX, cPlayer2D->vec2Index.y + camPos.y * mouseDist);
	camera2D->setTargetPos(camPos);
	camera2D->Update(dElapsedTime);

	if ((state == CURRENT_STATE::SHIPREST) && numOfEncounters > 0) {
		fightTimeElapsed += float(dElapsedTime);
		std::cout << fightTimeElapsed << " " << kBreakTime << "\n";

		if (fightTimeElapsed > kBreakTime) {
			fightTimeElapsed = 0.0f;
			cGUI_SceneCombat->isCombat = true;
			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BGM_FIGHT);
			cSoundController->StopSoundByID(CSoundController::SOUND_LIST::BGM_PLANET);
			cGUI_SceneCombat->GuiState = CGUI_SceneCombat::GUI_STATE::noShow;
			cGUI_SceneCombat->UpgradeState = CGUI_SceneCombat::UPGRADE_STATE::NOSTATE;
			state = CURRENT_STATE::SHIPCOMBAT;
			cShipEnemy->Randomise(cPlayer2D->vec2Index.x);
		}
	}

	

	if (cGUI_SceneCombat->isCombat) {
		cShipEnemy->Update(dElapsedTime);

		// enemy Death
		if (cShipEnemy->getHealth() <= 0) {
			// fights over its christmas lets go home
			if (camera2D->noiseOn) {
				camera2D->noiseOn = false;
			}
			cGUI_SceneCombat->isCombat = false;
			state = CURRENT_STATE::SHIPREST;

			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::SHIPHIT);
			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BGM_PLANET);
			cSoundController->StopSoundByID(CSoundController::SOUND_LIST::BGM_FIGHT);

			numOfEncounters--;
		}
	}
	else if (CInventoryManager::GetInstance()->GetItem("Health")->GetCount() != CInventoryManager::GetInstance()->GetItem("Health")->GetMaxCount()) {
		CInventoryManager::GetInstance()->GetItem("Health")->Add(1);
	}
	else if (CInventoryManager::GetInstance()->GetItem("Ventilation")->GetCount() != CInventoryManager::GetInstance()->GetItem("Ventilation")->GetMaxCount()) {
		CInventoryManager::GetInstance()->GetItem("Ventilation")->Add(1);

	}

	if (CMouseController::GetInstance()->IsButtonUp(CMouseController::BUTTON_TYPE::LMB) && lState) {
		lState = false;
	}

	// resolve if changes to gui are made
	if (cGUI_SceneCombat->makeChanges && cGUI_SceneCombat->showRepairPanel) {
		cMap2D->SetMapInfo(blockSelected.y, blockSelected.x, 598);
		CInventoryItem* item = CInventoryManager::GetInstance()->GetItem("Damage");
		item->Add(10);
		cGUI_SceneCombat->showRepairPanel = false;
		cGUI_SceneCombat->makeChanges = false;
	}

	if (CInventoryManager::GetInstance()->GetItem("Damage")->GetCount() == 0) {
		isDead = true;
		goToNextScene = true;
		cSoundController->StopSoundByID(CSoundController::SOUND_LIST::BGM_FIGHT);
		cSoundController->StopSoundByID(CSoundController::SOUND_LIST::BGM_PLANET);
	}

	if (cGUI_SceneCombat->makeChanges && cGUI_SceneCombat->GuiState == CGUI_SceneCombat::GUI_STATE::showExit) {
		// make the door closed x
		cMap2D->SetMapInfo(7, 16, 1216, false);

		try {
			if (cMap2D->SaveMap("Maps/DM2213_Map_Ship_SAVEGAME.csv", cMap2D->GetCurrentLevel()) == false)
			{
				throw runtime_error("Unable to save the current game to a file");
			}
		}
		catch (runtime_error e)
		{
			cout << "Runtime error: " << e.what();
			return false;
		}

		state = CURRENT_STATE::SHIPLANDED;
		goToNextScene = true;
		cSoundController->StopSoundByID(CSoundController::SOUND_LIST::BGM_FIGHT);
		cSoundController->StopSoundByID(CSoundController::SOUND_LIST::BGM_PLANET);
		cGUI_SceneCombat->GuiState = CGUI_SceneCombat::GUI_STATE::noShow;
		cGUI_SceneCombat->makeChanges = false;
	}
	if (cGUI_SceneCombat->makeChanges && cGUI_SceneCombat->GuiState == CGUI_SceneCombat::GUI_STATE::showUpgrade && cGUI_SceneCombat->UpgradeState == CGUI_SceneCombat::UPGRADE_STATE::STORAGE_UPGRADE) {
		int counter = 1;
		int xCount = 20;
		while (counter <= 3) {
			if (cMap2D->GetMapInfo(13, xCount, false) == 598) {
				// add item
				cMap2D->SetMapInfo(13, xCount, 1219, false);
				break;
			}
			else if (cMap2D->GetMapInfo(13, xCount, false) == 1219) {
				counter++;
				xCount--;
			}
		}
		cGUI_SceneCombat->UpgradeState = CGUI_SceneCombat::UPGRADE_STATE::NOSTATE;
		cGUI_SceneCombat->makeChanges = false;
	}


	if (CMouseController::GetInstance()->IsButtonDown(0) && !lState) {
		lState = true;
		PlayerInteractWithMap(camera2D->getBlockSelected());

	}

	if (cKeyboardController->IsKeyPressed(GLFW_KEY_V)) {
		numOfEncounters--;
	}

	// Call the cPlayer2D's update method before Map2D
	// as we want to capture the inputs before Map2D update
	cPlayer2D->Update(dElapsedTime);
	

	// Call all of the cEnemy2D's update methods before Map2D
	// as we want to capture the updates before Map2D update

	// Call the Map2D's update method
	cMap2D->Update(dElapsedTime);

	if (cGUI_SceneCombat->isCombat) {
		// Call the ship update
		cShip->Update(dElapsedTime);
	}

	// Get keyboard updates
	if (cKeyboardController->IsKeyReleased(GLFW_KEY_F1))
	{
		cSoundController->MasterVolumeIncrease();
	}
	else if (cKeyboardController->IsKeyReleased(GLFW_KEY_F2))
	{
		cSoundController->MasterVolumeDecrease();
	}

	// Call the cGUI_Scene2D's update method
	cGUI_SceneCombat->Update(dElapsedTime);

	switch (state)
	{
	case CURRENT_STATE::SHIPLANDED:
	case CURRENT_STATE::SHIPUPGRADE_NP:
		break;
	case CURRENT_STATE::SHIPUPGRADE:
		
		break;
	case CURRENT_STATE::SHIPREST:
		if (numOfEncounters <= 0) {
			cMap2D->SetMapInfo(7, 16, 1222, false);
		}
		break;
	default:
		break;
	}

	return true;
}

/**
 @brief PreRender Set up the OpenGL display environment before rendering
 */
void CSceneCombat::PreRender(void)
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
void CSceneCombat::Render(void)
{
	//Render Background
	background->Render();

	// Calls the Map2D's PreRender()
	cMap2D->PreRender();
	// Calls the Map2D's Render()
	cMap2D->Render();
	// Calls the Map2D's PostRender()
	cMap2D->PostRender();

	// Calls the CPlayer2D's PreRender()
	cPlayer2D->PreRender();
	// Calls the CPlayer2D's Render()
	cPlayer2D->Render();
	// Calls the CPlayer2D's PostRender()
	cPlayer2D->PostRender();

	// Calls the CGUI_Scene2D's PreRender()
	cGUI_SceneCombat->PreRender();
	// Calls the CGUI_Scene2D's Render()
	cGUI_SceneCombat->Render();
	// Calls the CGUI_Scene2D's PostRender()
	cGUI_SceneCombat->PostRender();
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CSceneCombat::PostRender(void)
{
}

int CSceneCombat::getNumEncounters(void)
{
	return numOfEncounters;
}

void CSceneCombat::SetNewState(int newState)
{
	state = (CURRENT_STATE)newState;
}

int CSceneCombat::getCurrentState()
{
	return state;
}


void CSceneCombat::PlayerInteractWithMap(glm::vec2 position)
{
	switch (cMap2D->GetMapInfo(position.y, position.x))
	{
	case 597:
		// TODO: add resource consumption
		if (cGUI_SceneCombat->UpgradeState != CGUI_SceneCombat::UPGRADE_STATE::NOSTATE) {
			cGUI_SceneCombat->UpgradeState = CGUI_SceneCombat::UPGRADE_STATE::NOSTATE;
		}

		cGUI_SceneCombat->showRepairPanel = true;
		blockSelected = position;
		cGUI_SceneCombat->blockPosition = ImVec2(camera2D->getBlockPositionWindow(position).x, camera2D->getBlockPositionWindow(position).y);
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::WINODWOPEN);
		break;

	case 1222:
		cGUI_SceneCombat->GuiState = CGUI_SceneCombat::GUI_STATE::showExit;
		blockSelected = position;
		cGUI_SceneCombat->blockPosition = ImVec2(camera2D->getBlockPositionWindow(position).x, camera2D->getBlockPositionWindow(position).y);
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::WINODWOPEN);

		break;
	case 1220:
		if (cGUI_SceneCombat->GuiState == CGUI_SceneCombat::GUI_STATE::noShow) {
			cGUI_SceneCombat->GuiState = CGUI_SceneCombat::GUI_STATE::showWeapons;
			camera2D->setTargetZoom(0.2f);
			//cGUI_SceneCombat->UpgradeState = CGUI_SceneCombat::UPGRADE_STATE::NOSTATE;
			blockSelected = position;
			cGUI_SceneCombat->blockPosition = ImVec2(camera2D->getBlockPositionWindow(position).x, camera2D->getBlockPositionWindow(position).y);
			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::WINODWOPEN);

		}
		else if (cGUI_SceneCombat->GuiState == CGUI_SceneCombat::GUI_STATE::showUpgrade) {
			cGUI_SceneCombat->GuiState = CGUI_SceneCombat::GUI_STATE::showWeaponUpgrade;
			camera2D->setTargetZoom(0.2f);
			//cGUI_SceneCombat->UpgradeState = CGUI_SceneCombat::UPGRADE_STATE::NOSTATE;
			blockSelected = position;
			cGUI_SceneCombat->blockPosition = ImVec2(camera2D->getBlockPositionWindow(position).x, camera2D->getBlockPositionWindow(position).y);
			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::WINODWOPEN);

		}
		break;
	case 1215:
	case 1218:
		if (cGUI_SceneCombat->showRepairPanel) {
			cGUI_SceneCombat->showRepairPanel = false;
			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::WINODWOPEN);

		}
		if (cGUI_SceneCombat->GuiState == CGUI_SceneCombat::GUI_STATE::showUpgrade) {
			cGUI_SceneCombat->UpgradeState = CGUI_SceneCombat::UPGRADE_STATE::VENTILATION_UPGRADE;
			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::WINODWOPEN);
			blockSelected = position;
			cGUI_SceneCombat->blockPosition = ImVec2(camera2D->getBlockPositionWindow(position).x, camera2D->getBlockPositionWindow(position).y);
		}
		else if (cGUI_SceneCombat->GuiState == CGUI_SceneCombat::GUI_STATE::showStorage) {
			cGUI_SceneCombat->GuiState = CGUI_SceneCombat::GUI_STATE::noShow;
			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::WINODWOPEN);

		}
		break;
	case 1219:
		if (cGUI_SceneCombat->GuiState == CGUI_SceneCombat::GUI_STATE::showUpgrade) {
			cGUI_SceneCombat->UpgradeState = CGUI_SceneCombat::UPGRADE_STATE::STORAGE_UPGRADE;
			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::WINODWOPEN);

			blockSelected = position;
			cGUI_SceneCombat->blockPosition = ImVec2(camera2D->getBlockPositionWindow(position).x, camera2D->getBlockPositionWindow(position).y);
		}
		else if (cGUI_SceneCombat->GuiState == CGUI_SceneCombat::GUI_STATE::showStorage) {
			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::WINODWOPEN);

			cGUI_SceneCombat->GuiState = CGUI_SceneCombat::GUI_STATE::noShow;

		}
		else {

			cGUI_SceneCombat->GuiState = CGUI_SceneCombat::GUI_STATE::showStorage;
			blockSelected = position;
			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::WINODWOPEN);

			cGUI_SceneCombat->blockPosition = ImVec2(camera2D->getBlockPositionWindow(position).x, camera2D->getBlockPositionWindow(position).y);
		}
		break;
	case 1221:
		if (state == CURRENT_STATE::SHIPUPGRADE_NP) {


			// save the planet
			try {
				if (cMap2D->SaveMap("Maps/DM2213_Map_Ship_SAVEGAME.csv", cMap2D->GetCurrentLevel()) == false)
				{
					throw runtime_error("Unable to save the current game to a file");
				}
			}
			catch (runtime_error e)
			{
				cout << "Runtime error: " << e.what();
			}

			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::WINODWOPEN);
			goToNextScene = true;

			cSoundController->StopSoundByID(CSoundController::SOUND_LIST::BGM_FIGHT);
			cSoundController->StopSoundByID(CSoundController::SOUND_LIST::BGM_PLANET);
		} 
		break;
	default:
		break;
	}
}

