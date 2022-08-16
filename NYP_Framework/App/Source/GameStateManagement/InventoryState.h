#pragma once

/**
 CInventoryState
 @brief This class is derived from CGameState. It will introduce the game to the player.
 By: Toh Da Jun
 Date: July 2021
 */

#include "GameStateBase.h"

#include "Primitives/Mesh.h"
#include "../Scene2D/BackgroundEntity.h"
#include "../Scene2D/GUI_Scene2D.h"
#include "../Scene2D/InventoryManagerPlanet.h"
#include "GameControl/Settings.h"

#include <string>

 // Include IMGUI
 // Important: GLEW and GLFW must be included before IMGUI
#ifndef IMGUI_ACTIVE
#include "GUI\imgui.h"
#include "GUI\backends\imgui_impl_glfw.h"
#include "GUI\backends\imgui_impl_opengl3.h"
#define IMGUI_ACTIVE
#endif

class CInventoryState : public CGameStateBase
{
public:
	// Constructor
	CInventoryState(void);
	// Destructor
	~CInventoryState(void);

	// Init this class instance
	virtual bool Init(void);
	// Update this class instance
	virtual bool Update(const double dElapsedTime);
	// Render this class instance
	virtual void Render(void);
	// Destroy this class instance
	virtual void Destroy(void);

protected:
	struct ButtonData
	{
		std::string fileName;
		unsigned textureID;
	};
	CGUI_Scene2D* cGUI_Scene2D;
	CBackgroundEntity* background;
	ButtonData resourceJungle;
	ButtonData resourceTerrestrial;
	ButtonData resourceSnow;
	ButtonData resourceBattery;
	ButtonData resourceScrapMetal;
	// The handler containing the instance of CInventoryManager
	CInventoryManagerPlanet* cInventoryManagerPlanet;
	// The handler containing the instance of CInventoryItem
	CInventoryItemPlanet* cInventoryItemPlanet;
	CSettings* cSettings;
};
