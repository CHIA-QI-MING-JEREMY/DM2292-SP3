// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLFW
#include <GLFW/glfw3.h>

#include "../PlanetSelection/ScenePlanet.h"
#include "ShipCombat.h"

// Include CGameStateManager
#include "GameStateManager.h"
#include "../App/Source/Scene2D/InventoryManager.h"
#include "GameInfo.h"

// Include CKeyboardController
#include "Inputs/KeyboardController.h"

#include <iostream>
using namespace std;

/**
 @brief Constructor
 */
CShipCombatState::CShipCombatState(void)
	: cSceneCombat(NULL)
{

}

/**
 @brief Destructor
 */
CShipCombatState::~CShipCombatState(void)
{

}

/**
 @brief Init this class instance
 */
bool CShipCombatState::Init(void)
{
	cout << "CShipCombatState::Init()\n" << endl;

	// Initialise the cScene2D instance
	cSceneCombat = CSceneCombat::GetInstance();
	if (cSceneCombat->Init() == false)
	{
		cout << "Failed to load cSceneCombat" << endl;
		return false;
	}

	if (CGameInfo::GetInstance()->PrevState == 1) {
		cSceneCombat->SetNewState(CSceneCombat::CURRENT_STATE::SHIPREST);
	}
	else if (CGameInfo::GetInstance()->PrevState == 2) {
		cSceneCombat->SetNewState(CSceneCombat::CURRENT_STATE::SHIPUPGRADE_NP);
		CGUI_SceneCombat::GetInstance()->GuiState = CGUI_SceneCombat::GUI_STATE::showUpgrade;
	}

	return true;
}

/**
 @brief Update this class instance
 */
bool CShipCombatState::Update(const double dElapsedTime)
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

		// Load the menu state
		cout << "Loading MenuState" << endl;
		CGameStateManager::GetInstance()->SetActiveGameState("MenuState");
	}

	if (cSceneCombat->goToNextScene && cSceneCombat->isDead) {
		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();

		// Load the menu state
		cout << "Loading MenuState" << endl;
		CGameStateManager::GetInstance()->SetActiveGameState("LoseState");
	}

	if (cSceneCombat->goToNextScene) {
		switch (cSceneCombat->getCurrentState())
		{
		case CSceneCombat::CURRENT_STATE::SHIPLANDED:

			if (CGameInfo::GetInstance()->selectedPlanet->getType() == CPlanet::TYPE::FINAL) {
				// Reset the CKeyboardController
				CKeyboardController::GetInstance()->Reset();

				// Load the menu state
				cout << "Loading WinState" << endl;
				CGameStateManager::GetInstance()->SetActiveGameState("WinState");
			}
			else {
				// Reset the CKeyboardController
				CKeyboardController::GetInstance()->Reset();

				// Load the menu state
				cout << "Loading PlayGameState" << endl;
				CGameStateManager::GetInstance()->SetActiveGameState("PlayGameState");
			}
			break;
		case CSceneCombat::CURRENT_STATE::SHIPUPGRADE_NP:
			// Reset the CKeyboardController
			CKeyboardController::GetInstance()->Reset();

			// Load the menu state
			cout << "Loading PlayGameState" << endl;
			CGameStateManager::GetInstance()->SetActiveGameState("PlanetState");
			break;
		default:
			break;
		}
	}

	// Call the cScene2D's Update method
	cSceneCombat->Update(dElapsedTime);

	return true;
}

/**
 @brief Render this class instance
 */
void CShipCombatState::Render(void)
{
	//cout << "CPlayGameState::Render()\n" << endl;

	// Call the cScene2D's Pre-Render method
	cSceneCombat->PreRender();

	// Call the cScene2D's Render method
	cSceneCombat->Render();

	// Call the cScene2D's PostRender method
	cSceneCombat->PostRender();
}

/**
 @brief Destroy this class instance
 */
void CShipCombatState::Destroy(void)
{
	cout << "CPlayGameState::Destroy()\n" << endl;

	// Destroy the cScene2D instance
	if (cSceneCombat)
	{
		cSceneCombat->Destroy();
		cSceneCombat = NULL;
	}
}