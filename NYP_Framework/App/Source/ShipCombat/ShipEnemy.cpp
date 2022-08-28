/**
 Player2D
 @brief A class representing the player object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "ShipEnemy.h"

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
#include "Primitives/Camera2D.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CShipEnemy::CShipEnemy(void)
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
CShipEnemy::~CShipEnemy(void)
{	

	cInventoryManager = NULL;

	// We won't delete this since it was created elsewhere
	cKeyboardController = NULL;
	cPlayer = NULL;

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
}

/**
  @brief Initialise this instance
  */
bool CShipEnemy::Init(void)
{
	// Store the keyboard controller singleton instance here
	cKeyboardController = CKeyboardController::GetInstance();
	// Reset all keys since we are starting a new game
	cKeyboardController->Reset();

	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	// By default, microsteps should be zero
	vec2NumMicroSteps = glm::i32vec2(0, 0);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	
	// Create the quad mesh for the player
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	cPlayer = CShipPlayer::GetInstance();

	// Load the sounds into CSoundController
	cSoundController = CSoundController::GetInstance();

	NoiseStartTime = 0.0f;
	TimeElapsed = 0.0f;
	attackTimer = 0.0f;
	attackCounter = 0;
	enemyHealth = 0;

	return true;
}

/**
 @brief Reset this instance
 */
bool CShipEnemy::Reset()
{
	// reset health and stuff

	return true;
}

/**
 @brief Update this instance
 */
void CShipEnemy::Update(const double dElapsedTime)
{
	TimeElapsed += 0.0167;

	switch (enemType)
	{
	case CShipEnemy::EASY:
		attackCounter = 1;
		break;
	case CShipEnemy::MEDIUM:
		attackCounter = 1;
		break;
	case CShipEnemy::HARD:
		attackCounter = 2;
		break;
	default:
		break;
	}

	if (TimeElapsed - attackTimer > enemyTimer) {
		for (int i = 0; i < attackCounter; i++) {
			Attack();
		}

		attackTimer = TimeElapsed;
	}

	for (int i = 0; i < tileVector.size(); i++) {
		tileVector[i].second += 0.0167;
		std::cout << tileVector[i].second << "\n";

		if (tileVector[i].second >= kWarningLength) {
			SetDamage(tileVector[i].first);
			tileVector.erase(tileVector.begin() + i);
			cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::SHIPHIT);
		}
	}

	if (cKeyboardController->IsKeyDown(GLFW_KEY_L)) {
		enemyTimer = 0.5f;
	}
	else {
		enemyTimer = 10.0f;
	}

	if ((TimeElapsed - NoiseStartTime) > kScreenShakeLength) {
		Camera2D::GetInstance()->noiseOn = false;
		NoiseStartTime = 0;
	}

}

void CShipEnemy::Attack(void)
{
	int randX = rand() % 13 + 10;
	int randY = rand() % 7 + 9;

	switch (enemType)
	{
	case CShipEnemy::EASY:
		std::cout << randX << " " << randY << " " << CMap2D::GetInstance()->GetMapInfo(randY, randX) << "\n";

		if (CMap2D::GetInstance()->GetMapInfo(randY, randX) == 598) {
			SetDanger(glm::vec2(randX, randY));
		}
		break;
	case CShipEnemy::MEDIUM:
		if (CMap2D::GetInstance()->GetMapInfo(cPlayer->vec2Index.y, cPlayer->vec2Index.x) == 598) {
			SetDanger(glm::vec2(cPlayer->vec2Index));
		}
		else {
			switch (cPlayer->player_Dir)
			{
			case CPlayer2D::DIRECTION::LEFT:
				if (CMap2D::GetInstance()->GetMapInfo(cPlayer->vec2Index.y, cPlayer->vec2Index.x - 1) == 598) {
					SetDanger(glm::vec2(cPlayer->vec2Index.x - 1, cPlayer->vec2Index.y));
				}
				break;
			case CPlayer2D::DIRECTION::RIGHT:
				if (CMap2D::GetInstance()->GetMapInfo(cPlayer->vec2Index.y, cPlayer->vec2Index.x + 1) == 598) {
					SetDanger(glm::vec2(cPlayer->vec2Index.x + 1, cPlayer->vec2Index.y));
				}
				break;
			case CPlayer2D::DIRECTION::UP:
				if (CMap2D::GetInstance()->GetMapInfo(cPlayer->vec2Index.y + 1, cPlayer->vec2Index.x) == 598) {
					SetDanger(glm::vec2(cPlayer->vec2Index.x, cPlayer->vec2Index.y + 1));
				}
				break;
			case CPlayer2D::DIRECTION::DOWN:
				if (CMap2D::GetInstance()->GetMapInfo(cPlayer->vec2Index.y - 1, cPlayer->vec2Index.x) == 598) {
					SetDanger(glm::vec2(cPlayer->vec2Index.x, cPlayer->vec2Index.y - 1));
				}
				break;
			default:
				break;
			}
		}
		break;
	case CShipEnemy::HARD:
		if (CMap2D::GetInstance()->GetMapInfo(cPlayer->vec2Index.y, cPlayer->vec2Index.x) == 598) {
			SetDanger(glm::vec2(cPlayer->vec2Index));
		}
		switch (cPlayer->player_Dir)
		{
		case CPlayer2D::DIRECTION::LEFT:
			if (CMap2D::GetInstance()->GetMapInfo(cPlayer->vec2Index.y, cPlayer->vec2Index.x - 2) == 598) {
				SetDanger(glm::vec2(cPlayer->vec2Index.x - 2, cPlayer->vec2Index.y));
			}
			break;
		case CPlayer2D::DIRECTION::RIGHT:
			if (CMap2D::GetInstance()->GetMapInfo(cPlayer->vec2Index.y, cPlayer->vec2Index.x + 2) == 598) {
				SetDanger(glm::vec2(cPlayer->vec2Index.x + 2, cPlayer->vec2Index.y));
			}
			break;
		case CPlayer2D::DIRECTION::UP:
			if (CMap2D::GetInstance()->GetMapInfo(cPlayer->vec2Index.y + 2, cPlayer->vec2Index.x) == 598) {
				SetDanger(glm::vec2(cPlayer->vec2Index.x, cPlayer->vec2Index.y + 2));
			}
			break;
		case CPlayer2D::DIRECTION::DOWN:
			if (CMap2D::GetInstance()->GetMapInfo(cPlayer->vec2Index.y - 2, cPlayer->vec2Index.x) == 598) {
				SetDanger(glm::vec2(cPlayer->vec2Index.x, cPlayer->vec2Index.y - 2));
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void CShipEnemy::Randomise(int position)
{
	if (position > 20) {
		// difficulter
		int luckyDraw = rand() % 99 + 1;
		if (luckyDraw < 30) {
			enemType = CShipEnemy::EASY;
			maxHealth = enemyHealth = 50;
			enemyName = "Rookie Enforcer";
		}
		else if (luckyDraw < 70) {
			enemType = CShipEnemy::MEDIUM;
			maxHealth = enemyHealth = 80;
			enemyName = "Sargeant Enforcer";
		}
		else {
			enemType = CShipEnemy::HARD;
			maxHealth = enemyHealth = 100;
			enemyName = "Commander Enforcer";
		}
	}
	else {
		// easier
		int luckyDraw = rand() % 99 + 1;
		if (luckyDraw < 70) {
			enemType = CShipEnemy::EASY;
			maxHealth = enemyHealth = 50;
			enemyName = "Rookie Enforcer";
		}
		else if (luckyDraw < 100) {
			enemType = CShipEnemy::MEDIUM;
			maxHealth = enemyHealth = 80;
			enemyName = "Sargeant Enforcer";
		}
	}
}

int CShipEnemy::getHealth(void)
{
	return enemyHealth;
}

void CShipEnemy::setHealth(int h)
{
	glm::clamp(h, 0, maxHealth);
	enemyHealth = h;
}


void CShipEnemy::SetDamage(glm::vec2 position)
{
	CInventoryItem* item = CInventoryManager::GetInstance()->GetItem("Damage");
	item->Remove(10);

	Camera2D::GetInstance()->noiseOn = true;
	NoiseStartTime = TimeElapsed;
	CMap2D::GetInstance()->SetMapInfo(position.y, position.x, 597);
}

void CShipEnemy::SetDanger(glm::vec2 position)
{
	CMap2D::GetInstance()->SetMapInfo(position.y, position.x, 590);
	tileVector.push_back(std::make_pair(position, 0.0f));
}
