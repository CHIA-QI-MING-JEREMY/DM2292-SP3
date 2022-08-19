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

class CGameInfo : public CSingletonTemplate<CGameInfo>
{
	friend CSingletonTemplate<CGameInfo>;
public:
	CPlanet* selectedPlanet;

protected:
	// Constructor
	CGameInfo() : selectedPlanet(NULL) {

	}
	// Destructor
	~CGameInfo() {

	}

};
