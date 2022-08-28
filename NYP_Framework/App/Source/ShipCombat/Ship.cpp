/**
 Player2D
 @brief A class representing the player object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Ship.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

// Include ImageLoader
#include "System\ImageLoader.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "../App/Source/Scene2D/Map2D.h"
#include "Primitives/MeshBuilder.h"
#include "../App/Source/GameStateManagement/GameInfo.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CShip::CShip(void)
	: cKeyboardController(NULL)
	, runtimeColour(glm::vec4(1.0f))
	, cSoundController(NULL)
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
CShip::~CShip(void)
{

	cInventoryManager = NULL;

	// We won't delete this since it was created elsewhere
	cKeyboardController = NULL;

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
}

/**
  @brief Initialise this instance
  */
bool CShip::Init(void)
{
	// Store the keyboard controller singleton instance here
	cKeyboardController = CKeyboardController::GetInstance();
	// Reset all keys since we are starting a new game
	cKeyboardController->Reset();

	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	
	// Create the quad mesh for the player
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Load the sounds into CSoundController
	cSoundController = CSoundController::GetInstance();

	cInventoryManager = CInventoryManager::GetInstance();

	TimeElapsed = 0;
	ventilationTiming = 0.0f;
	ventilationInterval = 10.0f;

	return true;
}

/**
 @brief Reset this instance
 */
bool CShip::Reset()
{
	// reset health and stuff

	return true;
}

/**
 @brief Update this instance
 */
void CShip::Update(const double dElapsedTime)
{
	TimeElapsed += 0.0167f;
	std::cout << ventilationTiming << " " << TimeElapsed << "\n";

	CInventoryItem* cItem1 = cInventoryManager->GetItem("Ventilation");
	if ((TimeElapsed - ventilationTiming) >= ventilationInterval) {
		// set the ship on fire bitches

		cItem1->Remove(1);
	}
	else if (cItem1->GetCount() != cItem1->GetMaxCount()) {
		cItem1->Add(1);
	}

	if (cItem1->GetCount() < 50) {
		cSettings->NUM_STEPS_PER_TILE_XAXIS = 16.0f;
		cSettings->NUM_STEPS_PER_TILE_YAXIS = 16.0f;

		cSettings->MICRO_STEP_XAXIS = cSettings->TILE_WIDTH / cSettings->NUM_STEPS_PER_TILE_XAXIS;
		cSettings->MICRO_STEP_YAXIS = cSettings->TILE_HEIGHT / cSettings->NUM_STEPS_PER_TILE_YAXIS;
	}
	else {
		cSettings->NUM_STEPS_PER_TILE_XAXIS = 8.0f;
		cSettings->NUM_STEPS_PER_TILE_YAXIS = 8.0f;

		cSettings->MICRO_STEP_XAXIS = cSettings->TILE_WIDTH / cSettings->NUM_STEPS_PER_TILE_XAXIS;
		cSettings->MICRO_STEP_YAXIS = cSettings->TILE_HEIGHT / cSettings->NUM_STEPS_PER_TILE_YAXIS;
	}
}

void CShip::ResetVentilationTimer(void)
{
	TimeElapsed = 0;
	ventilationTiming = 0;
}

