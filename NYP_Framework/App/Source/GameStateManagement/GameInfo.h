/*
This class is a high-level game state controller
Used for passing data in between states
*/
#pragma once

#include "DesignPatterns/SingletonTemplate.h"
#include <map>
#include <string>
#include "../App/Source/PlanetSelection/ScenePlanet.h"
#include "../App/Source/PlanetSelection/Planet.h"
#include "../App/Source/Scene2D/InventoryManager.h"

class CGameInfo : public CSingletonTemplate<CGameInfo>
{
	friend CSingletonTemplate<CGameInfo>;
public:
	CPlanet* selectedPlanet;
	int PrevState = 0;
	bool initInventory = false;
	bool initPlanets = false;
	bool loadedMap = false;
	int nebulaSize = 3;
	glm::vec2 currentPlanetPos = glm::vec2(-1, -1);
	std::vector<int> planetList;
	CInventoryManager* cInventoryManager;

	void ExportIM(CInventoryManager* newIM) {
		cInventoryManager = newIM;
	}

	CInventoryManager* ImportIM(void) {
		return cInventoryManager;
	}


protected:
	// Constructor
	CGameInfo() : selectedPlanet(NULL) {
		cInventoryManager = CInventoryManager::GetInstance();
	}
	// Destructor
	~CGameInfo() {
		if (cInventoryManager)
		{
			cInventoryManager->Destroy();
			cInventoryManager = NULL;
		}
	}

};
