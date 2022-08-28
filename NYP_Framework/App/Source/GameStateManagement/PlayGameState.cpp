// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLFW
#include <GLFW/glfw3.h>

#include "PlayGameState.h"

// Include CGameStateManager
#include "GameStateManager.h"
#include "../App/Source/Scene2D/InventoryManager.h"

// Include CKeyboardController
#include "Inputs/KeyboardController.h"
#include "GameInfo.h"

#include <iostream>
using namespace std;

/**
 @brief Constructor
 */
CPlayGameState::CPlayGameState(void)
	: cSnowPlanet(NULL)
	, cJunglePlanet(NULL)
	, cTerrestrialPlanet(NULL)
{

}

/**
 @brief Destructor
 */
CPlayGameState::~CPlayGameState(void)
{

}

/**
 @brief Init this class instance
 */
bool CPlayGameState::Init(void)
{
	cout << "CPlayGameState::Init()\n" << endl;

	type = CGameInfo::GetInstance()->selectedPlanet->getType();

	std::cout << (CPlanet::TYPE(type)) << "\n";

	// Initialise the cScene2D instance
	switch (type)
	{
	case CPlanet::TYPE::SNOW:
	case CPlanet::TYPE::SNOW_TUTORIAL:
		cSnowPlanet = SnowPlanet::GetInstance();
		if (cSnowPlanet->Init() == false) {
			return false;
		}
		break;
	case CPlanet::TYPE::JUNGLE:
	case CPlanet::TYPE::JUNGLE_TUTORIAL:
		cJunglePlanet = JunglePlanet::GetInstance();
		if (cJunglePlanet->Init() == false) {
			return false;
		}
		break;
	case CPlanet::TYPE::TERRESTRIAL:
	case CPlanet::TYPE::TERRESTRIAL_TUTORIAL:
		cTerrestrialPlanet = TerrestrialPlanet::GetInstance();
		if (cTerrestrialPlanet->Init() == false) {
			return false;
		}
		break;
	default:
		return false;
		break;
	}

	switch (type)
	{
	case CPlanet::TYPE::SNOW:
		cSnowPlanet->DecideLevel(false);
		break;
	case CPlanet::TYPE::SNOW_TUTORIAL:
		cSnowPlanet->DecideLevel(true);
		break;
	case CPlanet::TYPE::JUNGLE:
		cJunglePlanet->DecideLevel(false);
		break;
	case CPlanet::TYPE::JUNGLE_TUTORIAL:
		cJunglePlanet->DecideLevel(true);
		break;
	case CPlanet::TYPE::TERRESTRIAL:
		cTerrestrialPlanet->DecideLevel(false);
		break;
	case CPlanet::TYPE::TERRESTRIAL_TUTORIAL:
		cTerrestrialPlanet->DecideLevel(true);
		break;
	default:
		return false;
		break;
	}
	return true;
}

/**
 @brief Update this class instance
 */
bool CPlayGameState::Update(const double dElapsedTime)
{
	//if (CKeyboardController::GetInstance()->IsKeyReleased(GLFW_KEY_F10))
	//{
	//	// Reset the CKeyboardController
	//	CKeyboardController::GetInstance()->Reset();

	//	// Load the menu state
	//	cout << "Loading MenuState" << endl;
	//	CGameStateManager::GetInstance()->SetActiveGameState("MenuState");
	//	CGameStateManager::GetInstance()->OffPauseGameState();
	//	return true;
	//}

	if (CKeyboardController::GetInstance()->IsKeyReleased(GLFW_KEY_ESCAPE))
	{
		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();
		CInventoryManager::GetInstance()->Exit();
		CInventoryManagerPlanet::GetInstance()->Exit();

		// Load the menu state
		cout << "Loading MenuState" << endl;
		CGameStateManager::GetInstance()->SetActiveGameState("MenuState");
		CGameStateManager::GetInstance()->OffPauseGameState();
		return true;
	}
	else if (CKeyboardController::GetInstance()->IsKeyReleased(GLFW_KEY_I))
	{
		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();

		// Load the menu state
		cout << "Loading InventoryState" << endl;
		CGameStateManager::GetInstance()->SetPauseGameState("InventoryState");
	}


	// Call the cScene2D's Update method
	switch (type)
	{
	case CPlanet::TYPE::SNOW:
	case CPlanet::TYPE::SNOW_TUTORIAL:
		cSnowPlanet->Update(dElapsedTime);
		break;
	case CPlanet::TYPE::JUNGLE:
	case CPlanet::TYPE::JUNGLE_TUTORIAL:
		cJunglePlanet->Update(dElapsedTime);
		break;
	case CPlanet::TYPE::TERRESTRIAL:
	case CPlanet::TYPE::TERRESTRIAL_TUTORIAL:
		cTerrestrialPlanet->Update(dElapsedTime);
		break;
	default:
		return false;
		break;
	}

	// Call the cScene2D's Update method
	switch (type)
	{
	case CPlanet::TYPE::SNOW:
	case CPlanet::TYPE::SNOW_TUTORIAL:
		if (cSnowPlanet->getIsOnShip() || CInventoryManagerPlanet::GetInstance()->GetItem("Lives")->GetCount() <= 0) {
			// Reset the CKeyboardController
			CKeyboardController::GetInstance()->Reset();
			cSnowPlanet->SetResourcesBack();
			CSoundController::GetInstance()->StopSoundByID(CSoundController::SOUND_LIST::BGM_NORMAL);
			CSoundController::GetInstance()->PlaySoundByID(CSoundController::SOUND_LIST::WINODWOPEN);

			// Load the menu state
			cout << "Loading ShipCombatState" << endl;
			CGameStateManager::GetInstance()->SetActiveGameState("ShipCombatState");
			CGameInfo::GetInstance()->PrevState = 2;
		}
		break;
	case CPlanet::TYPE::JUNGLE:
	case CPlanet::TYPE::JUNGLE_TUTORIAL:
		if (cJunglePlanet->getIsOnShip() || CInventoryManagerPlanet::GetInstance()->GetItem("Lives")->GetCount() <= 0) {
			// Reset the CKeyboardController
			CKeyboardController::GetInstance()->Reset();
			cJunglePlanet->SetResourcesBack();
			CSoundController::GetInstance()->StopSoundByID(CSoundController::SOUND_LIST::BACKGROUNDSNOW);
			CSoundController::GetInstance()->PlaySoundByID(CSoundController::SOUND_LIST::WINODWOPEN);

			// Load the menu state
			cout << "Loading ShipCombatState" << endl;
			CGameStateManager::GetInstance()->SetActiveGameState("ShipCombatState");
			CGameInfo::GetInstance()->PrevState = 2;
		}
		break;
	case CPlanet::TYPE::TERRESTRIAL:
	case CPlanet::TYPE::TERRESTRIAL_TUTORIAL:
		if (cTerrestrialPlanet->getIsOnShip() || CInventoryManagerPlanet::GetInstance()->GetItem("Lives")->GetCount() <= 0) {
			// Reset the CKeyboardController
			CKeyboardController::GetInstance()->Reset();
			cTerrestrialPlanet->SetResourcesBack();
			CSoundController::GetInstance()->StopSoundByID(CSoundController::SOUND_LIST::BGM_NORMAL);
			CSoundController::GetInstance()->PlaySoundByID(CSoundController::SOUND_LIST::WINODWOPEN);

			// Load the menu state
			cout << "Loading ShipCombatState" << endl;
			CGameStateManager::GetInstance()->SetActiveGameState("ShipCombatState");
			CGameInfo::GetInstance()->PrevState = 2;
		}
		break;
	default:
		return false;
		break;
	}

	return true;
}

/**
 @brief Render this class instance
 */
void CPlayGameState::Render(void)
{
	// Call the cScene2D's Update method
	switch (type)
	{
	case CPlanet::TYPE::SNOW:
	case CPlanet::TYPE::SNOW_TUTORIAL:
		cSnowPlanet->PreRender();
		cSnowPlanet->Render();
		cSnowPlanet->PostRender();
		break;
	case CPlanet::TYPE::JUNGLE:
	case CPlanet::TYPE::JUNGLE_TUTORIAL:
		cJunglePlanet->PreRender();
		cJunglePlanet->Render();
		cJunglePlanet->PostRender();
		break;
	case CPlanet::TYPE::TERRESTRIAL:
	case CPlanet::TYPE::TERRESTRIAL_TUTORIAL:
		cTerrestrialPlanet->PreRender();
		cTerrestrialPlanet->Render();
		cTerrestrialPlanet->PostRender();
		break;
	default:
		break;
	}
}

/**
 @brief Destroy this class instance
 */
void CPlayGameState::Destroy(void)
{
	cout << "CPlayGameState::Destroy()\n" << endl;

	// Call the cScene2D's Update method
	switch (type)
	{
	case CPlanet::TYPE::SNOW:
	case CPlanet::TYPE::SNOW_TUTORIAL:
		if (cSnowPlanet) {
			cSnowPlanet->Destroy();
			cSnowPlanet = NULL;
		}
		break;
	case CPlanet::TYPE::JUNGLE:
	case CPlanet::TYPE::JUNGLE_TUTORIAL:
		if (cJunglePlanet) {
			cJunglePlanet->Destroy();
			cJunglePlanet = NULL;
		}
		break;
	case CPlanet::TYPE::TERRESTRIAL:
	case CPlanet::TYPE::TERRESTRIAL_TUTORIAL:
		if (cTerrestrialPlanet) {
			cTerrestrialPlanet->Destroy();
			cTerrestrialPlanet = NULL;
		}
		break;
	default:
		break;
	}
}