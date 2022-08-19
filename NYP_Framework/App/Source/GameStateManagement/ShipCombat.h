#pragma once

/**
 CPlayGameState
 @brief This class is derived from CGameState. It will introduce the game to the player.
 By: Toh Da Jun
 Date: July 2021
 */

#include "GameStateBase.h"

#include "../App/Source/ShipCombat/SceneCombat.h"

class CShipCombatState : public CGameStateBase
{
public:
	// Constructor
	CShipCombatState(void);
	// Destructor
	~CShipCombatState(void);

	// Init this class instance
	virtual bool Init(void);
	// Update this class instance
	virtual bool Update(const double dElapsedTime);
	// Render this class instance
	virtual void Render(void);
	// Destroy this class instance
	virtual void Destroy(void);

protected:
	// The handler to the CScene2D instance
	CSceneCombat* cSceneCombat;
};
