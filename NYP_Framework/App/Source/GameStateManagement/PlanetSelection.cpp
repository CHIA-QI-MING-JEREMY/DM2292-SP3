// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLFW
#include <GLFW/glfw3.h>

#include "../PlanetSelection/ScenePlanet.h"
#include "PlanetSelection.h"

// Include CGameStateManager
#include "GameStateManager.h"
// Include ImageLoader
#include "System\ImageLoader.h"

// Include CKeyboardController
#include "Inputs/KeyboardController.h"

#include <iostream>
using namespace std;

/**
 @brief Constructor
 */
CPlanetSelectionState::CPlanetSelectionState(void)
	: cScenePlanet(NULL)
{

}

/**
 @brief Destructor
 */
CPlanetSelectionState::~CPlanetSelectionState(void)
{

}

/**
 @brief Init this class instance
 */
bool CPlanetSelectionState::Init(void)
{
	cout << "CPlanetSelectionState::Init()\n" << endl;

	// Initialise the cScene2D instance
	cScenePlanet = CScenePlanet::GetInstance();
	if (cScenePlanet->Init() == false)
	{
		cout << "Failed to load cScenePlanet" << endl;
		return false;
	}

	return true;
}

/**
 @brief Update this class instance
 */
bool CPlanetSelectionState::Update(const double dElapsedTime)
{

	if (cScenePlanet->StartCombat) {
		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();

		// Load the menu state
		cout << "Loading ShipCombatState" << endl;
		CGameStateManager::GetInstance()->SetActiveGameState("ShipCombatState");
	}
	
	if (CKeyboardController::GetInstance()->IsKeyReleased(GLFW_KEY_ESCAPE))
	{
		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();

		// Load the menu state
		cout << "Loading MenuState" << endl;
		CGameStateManager::GetInstance()->SetActiveGameState("MenuState");
	}

	// Call the cScene2D's Update method
	cScenePlanet->Update(dElapsedTime);

	return true;
}

/**
 @brief Render this class instance
 */
void CPlanetSelectionState::Render(void)
{
	//cout << "CPlayGameState::Render()\n" << endl;

	// Call the cScene2D's Pre-Render method
	cScenePlanet->PreRender();

	// Call the cScene2D's Render method
	cScenePlanet->Render();

	// Call the cScene2D's PostRender method
	cScenePlanet->PostRender();
}

/**
 @brief Destroy this class instance
 */
void CPlanetSelectionState::Destroy(void)
{
	cout << "CPlayGameState::Destroy()\n" << endl;

	// Destroy the cScene2D instance
	if (cScenePlanet)
	{
		cScenePlanet->Destroy();
		cScenePlanet = NULL;
	}
}