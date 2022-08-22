/**
 CGUI_Scene2D
 @brief A class which manages the GUI for Scene2D
 By: Toh Da Jun
 Date: May 2021
 */
#pragma once

// Include SingletonTemplate
#include "DesignPatterns/SingletonTemplate.h"

// Include CEntity2D
#include "Primitives/Entity2D.h"

// FPS Counter
#include "TimeControl\FPSCounter.h"

// Include CInventoryManager
#include "../App/Source/Scene2D/InventoryManager.h"


// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include IMGUI
// Important: GLEW and GLFW must be included before IMGUI
#ifndef IMGUI_ACTIVE
#include "GUI\imgui.h"
#include "GUI\backends\imgui_impl_glfw.h"
#include "GUI\backends\imgui_impl_opengl3.h"
#define IMGUI_ACTIVE
#endif

#include "GameControl/Settings.h"

#include "Planet.h"

#include <string>
using namespace std;

class CGUI_ScenePlanet : public CSingletonTemplate<CGUI_ScenePlanet>, public CEntity2D
{
	friend CSingletonTemplate<CGUI_ScenePlanet>;
public:
	// Init
	bool Init(void);

	// Update
	void Update(const double dElapsedTime);

	// PreRender
	void PreRender(void);

	// Render
	void Render(void);

	// PostRender
	void PostRender(void);

	void setPlanetNum(int num);
	int getPlanetNum(void);

	void setPlanetInfo(CPlanet* cPlanet);
	bool isShowPanel;
	bool StartCombat;
	bool isButtonHover;

protected:
	// Constructor
	CGUI_ScenePlanet(void);

	// Destructor
	virtual ~CGUI_ScenePlanet(void);

	// FPS Control
	CFPSCounter* cFPSCounter;

	// Flags for IMGUI
	ImGuiWindowFlags window_flags;

	CSettings* cSettings;

	struct ButtonData
	{
		std::string fileName;
		unsigned textureID;
	};

	ButtonData StartCombatButtonData;

	// The handler containing the instance of CInventoryManager
	CInventoryManager* cInventoryManager;
	// The handler containing the instance of CInventoryItem
	CInventoryItem* cInventoryItem;

	CPlanet* cPlanet;

private:
	int planetNum;
};
