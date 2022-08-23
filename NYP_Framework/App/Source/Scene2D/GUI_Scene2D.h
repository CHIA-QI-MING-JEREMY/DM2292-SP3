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
#include "InventoryManager.h"


// Include InventoryManager
#include "InventoryManagerPlanet.h"

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

#include <string>
using namespace std;

class CGUI_Scene2D : public CSingletonTemplate<CGUI_Scene2D>, public CEntity2D
{
	friend CSingletonTemplate<CGUI_Scene2D>;
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

	bool getGoOnShip(void);
	bool getShowExitPanel(void);
	void setShowExitPanel(bool newSet);
	void setBlockPosition(glm::vec2 blockPos);

	int getTutorialPopupJungle(void);
	void setTutorialPopupJungle(int index);

	enum JUNGLE_TUTORIAL_POPUP
	{
		NONE = 0,
		CHECKPOINT, //triggered onceplayer hits first checkpoint
		SHOOT, //triggered at the first set of bushes to destory with shooting
		BUNRABLE, //triggered whenever player burns a bush
		RESOURCE, //triggered every time player collects an item
		POISON, //triggered by the first poison sprout hit (hardcoded)
		SWITCHES, //triggered the first time the player hits a switch
		RIVER_WATER, //triggered when in river water
		VINE, //triggered when player collects vine --> OVERRIDEN BY OTHERS THAT TRIGGER AFTER
		BUSHES, //triggered when player collects burnable blocks --> OVERRIDEN BY OTHERS THAT TRIGGER AFTER
		BOUNCY_BLOOM, //triggered everytime player collides with unbloomed bouncy bloom
		ROCK, //triggered when player collides with a rock with on vine on it
		NUM_JT_POPUP
	};

protected:
	// Constructor
	CGUI_Scene2D(void);

	// Destructor
	virtual ~CGUI_Scene2D(void);

	// FPS Control
	CFPSCounter* cFPSCounter;

	// Flags for IMGUI
	ImGuiWindowFlags window_flags;
	// For progress bar demo only
	float m_fProgressBar;

	CSettings* cSettings;

	// The handler containing the instance of CInventoryManager
	CInventoryManager* cInventoryManager;
	// The handler containing the instance of CInventoryItem
	CInventoryItem* cInventoryItem;

	// The handler containing the instance of CInventoryManager
	CInventoryManagerPlanet* cInventoryManagerPlanet;
	// The handler containing the instance of CInventoryItem
	CInventoryItemPlanet* cInventoryItemPlanet;

	struct ButtonData
	{
		std::string fileName;
		unsigned textureID;
	};

	ButtonData AcceptButtonData;
	ButtonData RejectButtonData;

	bool lState;
	bool goOnShip;
	bool showExitPanel;
	glm::vec2 blockPosition;

	int tutorialPopupJungle; //index for which pop up to show in jungle tutorial
private:
	int planetNum;
};
