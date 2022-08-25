// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

#include "InventoryState.h"

// Include CGameStateManager
#include "GameStateManager.h"

// Include Mesh Builder
#include "Primitives/MeshBuilder.h"
// Include ImageLoader
#include "System\ImageLoader.h"
// Include Shader Manager
#include "RenderControl\ShaderManager.h"

 // Include shader
#include "RenderControl\shader.h"

// Include CSettings
#include "GameControl/Settings.h"

// Include CKeyboardController
#include "Inputs/KeyboardController.h"

#include "../SoundController/SoundController.h"

#include <iostream>
using namespace std;

/**
 @brief Constructor
 */
CInventoryState::CInventoryState(void)
	//: background(NULL)
{

}

/**
 @brief Destructor
 */
CInventoryState::~CInventoryState(void)
{
}

/**
 @brief Init this class instance
 */
bool CInventoryState::Init(void)
{

	CShaderManager::GetInstance()->Use("Shader2D");
	//CShaderManager::GetInstance()->activeShader->setInt("texture1", 0);

	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();
	// Initialise the CInventoryManagerPlanet
	cInventoryManagerPlanet = CInventoryManagerPlanet::GetInstance();
	cGUI_Scene2D = CGUI_Scene2D::GetInstance();
	//Create Background Entity
	background = new CBackgroundEntity("Image/InventoryBag.png");
	background->SetShader("Shader2D");
	background->Init();
	cout << "CInventoryState::Init()\n" << endl;


	// Load the images for buttons
	CImageLoader* il = CImageLoader::GetInstance();
	resourceSnow.fileName = "Image\\GUI\\icecrystal.tga";
	resourceSnow.textureID = il->LoadTextureGetID(resourceSnow.fileName.c_str(), false);

	resourceBattery.fileName = "Image\\GUI\\Battery.tga";
	resourceBattery.textureID = il->LoadTextureGetID(resourceBattery.fileName.c_str(), false);

	resourceScrapMetal.fileName = "Image\\GUI\\ScrapMetal.tga";
	resourceScrapMetal.textureID = il->LoadTextureGetID(resourceScrapMetal.fileName.c_str(), false);

	resourceJungle.fileName = "Image\\GUI\\Ironwood.tga";
	resourceJungle.textureID = il->LoadTextureGetID(resourceJungle.fileName.c_str(), false);

	resourceTerrestrial.fileName = "Image\\GUI\\EnergyQuartz.tga";
	resourceTerrestrial.textureID = il->LoadTextureGetID(resourceTerrestrial.fileName.c_str(), false);


	//VolumeDecreaseButtonData.fileName = "Image\\GUI\\Button_VolDown.png";
	//VolumeDecreaseButtonData.textureID = il->LoadTextureGetID(VolumeDecreaseButtonData.fileName.c_str(), false);

	return true;
}

/**
 @brief Update this class instance
 */
bool CInventoryState::Update(const double dElapsedTime)
{

	// Start the Dear ImGui frame
	//ImGui_ImplOpenGL3_NewFrame();
	//ImGui_ImplGlfw_NewFrame();
	//ImGui::NewFrame();
	const float relativeScale_x = cSettings->iWindowWidth / 800.0f;
	const float relativeScale_y = cSettings->iWindowHeight / 600.0f;
	float buttonWidth = cSettings->iWindowWidth / 8.f;
	float buttonHeight = cSettings->iWindowHeight / 6.f;

	ImGuiWindowFlags Textwindow_flags = 0;
	Textwindow_flags |= ImGuiWindowFlags_NoTitleBar;
	Textwindow_flags |= ImGuiWindowFlags_NoScrollbar;
	//window_flags |= ImGuiWindowFlags_MenuBar;
	Textwindow_flags |= ImGuiWindowFlags_NoBackground;
	Textwindow_flags |= ImGuiWindowFlags_NoMove;
	Textwindow_flags |= ImGuiWindowFlags_NoCollapse;
	Textwindow_flags |= ImGuiWindowFlags_NoNav;
	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		// Create a window called "Hello, world!" and append into it.
		ImGui::Begin("Max Bag", NULL, Textwindow_flags);
		ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 1.45,
			CSettings::GetInstance()->iWindowHeight / 3.3));	// Set the top-left of the window at (10,10)
		ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight));
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Resources");
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGui::SameLine;
		ImGui::TextColored(ImVec4(0, 0, 0, 1), "Bag Space:%d/%d",
			cInventoryItemPlanet->GetCount(), cInventoryItemPlanet->GetMaxCount());
		ImGui::End();
	}
	ImGuiWindowFlags TextBattwindow_flags = 0;
	TextBattwindow_flags |= ImGuiWindowFlags_NoTitleBar;
	TextBattwindow_flags |= ImGuiWindowFlags_NoScrollbar;
	//window_flags |= ImGuiWindowFlags_MenuBar;
	TextBattwindow_flags |= ImGuiWindowFlags_NoBackground;
	TextBattwindow_flags |= ImGuiWindowFlags_NoMove;
	TextBattwindow_flags |= ImGuiWindowFlags_NoCollapse;
	TextBattwindow_flags |= ImGuiWindowFlags_NoNav;
	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		// Create a window called "Hello, world!" and append into it.
		ImGui::Begin("BatteryText", NULL, TextBattwindow_flags);
		ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 1.45,
			CSettings::GetInstance()->iWindowHeight / 3.8));	// Set the top-left of the window at (10,10)
		ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight));
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Battery");
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGui::SameLine;
		ImGui::TextColored(ImVec4(0, 0, 0, 1), "Battery:%d",
			cInventoryItemPlanet->GetCount());
		ImGui::End();
	}
	ImGuiWindowFlags TextScrapMetalwindow_flags = 0;
	TextScrapMetalwindow_flags |= ImGuiWindowFlags_NoTitleBar;
	TextScrapMetalwindow_flags |= ImGuiWindowFlags_NoScrollbar;
	//window_flags |= ImGuiWindowFlags_MenuBar;
	TextScrapMetalwindow_flags |= ImGuiWindowFlags_NoBackground;
	TextScrapMetalwindow_flags |= ImGuiWindowFlags_NoMove;
	TextScrapMetalwindow_flags |= ImGuiWindowFlags_NoCollapse;
	TextScrapMetalwindow_flags |= ImGuiWindowFlags_NoNav;
	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		// Create a window called "Hello, world!" and append into it.
		ImGui::Begin("ScrapText", NULL, TextScrapMetalwindow_flags);
		ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 1.45,
			CSettings::GetInstance()->iWindowHeight / 4.5));	// Set the top-left of the window at (10,10)
		ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight));
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("ScrapMetal");
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGui::SameLine;
		ImGui::TextColored(ImVec4(0, 0, 0, 1), "Scrap Metal:%d",
			cInventoryItemPlanet->GetCount());
		ImGui::End();
	}

	ImGuiWindowFlags Batterywindow_flags = 0;
	Batterywindow_flags |= ImGuiWindowFlags_NoTitleBar;
	Batterywindow_flags |= ImGuiWindowFlags_NoScrollbar;
	//window_flags |= ImGuiWindowFlags_MenuBar;
	Batterywindow_flags |= ImGuiWindowFlags_NoBackground;
	Batterywindow_flags |= ImGuiWindowFlags_NoMove;
	Batterywindow_flags |= ImGuiWindowFlags_NoCollapse;
	Batterywindow_flags |= ImGuiWindowFlags_NoNav;
	Batterywindow_flags |= ImGuiWindowFlags_NoResize;

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		// Create a window called "Hello, world!" and append into it.
		ImGui::Begin("Battery", NULL, Batterywindow_flags);
		ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 3.0 * 2 - buttonWidth / 2.0 - 100,
			CSettings::GetInstance()->iWindowHeight / 3.0 - 60));	// Set the top-left of the window at (10,10)
		ImGui::SetWindowSize(ImVec2(buttonWidth, buttonHeight));
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Battery");
		ImGui::SetWindowFontScale(3.f * relativeScale_y);

		//Added rounding for nBatteryr effect
		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameRounding = 200.0f;

		// Display the FPS
		//ImGui::TextColored(ImVec4(1, 1, 1, 1), "In-Game Menu");

		// Add codes for Start button here
		if (ImGui::ImageButton((ImTextureID)resourceBattery.textureID,
			ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0)))
		{
			// Reset the CKeyboardController
			CKeyboardController::GetInstance()->Reset();

			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Battery");
			if (cInventoryItemPlanet->GetCount() > 0) {
				cInventoryItemPlanet->Remove(1);
				cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Resources");
				if (cInventoryItemPlanet->GetCount() > 0) {
					cInventoryItemPlanet->Remove(1);
				}
			}
		}
		ImGui::End();
	}
	ImGuiWindowFlags ScrapMetalwindow_flags = 0;
	ScrapMetalwindow_flags |= ImGuiWindowFlags_NoTitleBar;
	ScrapMetalwindow_flags |= ImGuiWindowFlags_NoScrollbar;
	//window_flags |= ImGuiWindowFlags_MenuBar;
	ScrapMetalwindow_flags |= ImGuiWindowFlags_NoBackground;
	ScrapMetalwindow_flags |= ImGuiWindowFlags_NoMove;
	ScrapMetalwindow_flags |= ImGuiWindowFlags_NoCollapse;
	ScrapMetalwindow_flags |= ImGuiWindowFlags_NoNav;
	ScrapMetalwindow_flags |= ImGuiWindowFlags_NoResize;


	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		// Create a window called "Hello, world!" and append into it.
		ImGui::Begin("ScrapMetal", NULL, ScrapMetalwindow_flags);
		ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 2.0 - buttonWidth / 2.0 - 100,
			CSettings::GetInstance()->iWindowHeight / 3.0 - 60));	// Set the top-left of the window at (10,10)
		ImGui::SetWindowSize(ImVec2(buttonWidth, buttonHeight));
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("ScrapMetal");
		ImGui::SetWindowFontScale(3.f * relativeScale_y);

		//Added rounding for nScrapMetalr effect
		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameRounding = 200.0f;

		// Display the FPS
		//ImGui::TextColored(ImVec4(1, 1, 1, 1), "In-Game Menu");

		// Add codes for Start button here
		if (ImGui::ImageButton((ImTextureID)resourceScrapMetal.textureID,
			ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0)))
		{
			// Reset the CKeyboardController
			CKeyboardController::GetInstance()->Reset();

			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("ScrapMetal");
			if (cInventoryItemPlanet->GetCount() > 0) {
				cInventoryItemPlanet->Remove(1);
				cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Resources");
				if (cInventoryItemPlanet->GetCount() > 0) {
					cInventoryItemPlanet->Remove(1);
				}
			}
		}
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0, 0, 0, 1), "%d",
			cInventoryItemPlanet->GetCount());
		ImGui::End();
	}
	if (cGUI_Scene2D->getPlanetNum() == 3) {
		//planet collectibles
		ImGuiWindowFlags BerserkTextwindow_flags = 0;
		BerserkTextwindow_flags |= ImGuiWindowFlags_NoTitleBar;
		BerserkTextwindow_flags |= ImGuiWindowFlags_NoScrollbar;
		//window_flags |= ImGuiWindowFlags_MenuBar;
		BerserkTextwindow_flags |= ImGuiWindowFlags_NoBackground;
		BerserkTextwindow_flags |= ImGuiWindowFlags_NoMove;
		BerserkTextwindow_flags |= ImGuiWindowFlags_NoCollapse;
		BerserkTextwindow_flags |= ImGuiWindowFlags_NoNav;
		{
			static float f = 0.0f;
			static int counter = 0;

			// Create a window called "Hello, world!" and append into it.
			ImGui::Begin("Berserktext", NULL, BerserkTextwindow_flags);
			ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 1.45,
				CSettings::GetInstance()->iWindowHeight / 2.0));
			ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight));
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("berserk");
			ImGui::SetWindowFontScale(1.5f * relativeScale_y);
			ImGui::SameLine;
			ImGui::TextColored(ImVec4(0, 0, 0, 1), "Berserk:%d",
				cInventoryItemPlanet->GetCount());
			ImGui::End();
		}
		ImGuiWindowFlags Berserkwindow_flags = 0;
		Berserkwindow_flags |= ImGuiWindowFlags_NoTitleBar;
		Berserkwindow_flags |= ImGuiWindowFlags_NoScrollbar;
		//window_flags |= ImGuiWindowFlags_MenuBar;
		Berserkwindow_flags |= ImGuiWindowFlags_NoBackground;
		Berserkwindow_flags |= ImGuiWindowFlags_NoMove;
		Berserkwindow_flags |= ImGuiWindowFlags_NoCollapse;
		Berserkwindow_flags |= ImGuiWindowFlags_NoNav;
		Berserkwindow_flags |= ImGuiWindowFlags_NoResize;
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Berserk", NULL, Berserkwindow_flags);
			ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 3.0 - buttonWidth / 2.0 - 100,
				CSettings::GetInstance()->iWindowHeight / 1.57 - 60));
			ImGui::SetWindowSize(ImVec2(buttonWidth, buttonHeight));
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("berserk");
			ImGui::SetWindowFontScale(3.f * relativeScale_y);

			//Added rounding for nIronr effect
			ImGuiStyle& style = ImGui::GetStyle();
			style.FrameRounding = 200.0f;

			// Add codes for Start button here
			ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
				ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0));
			//ImGui::SameLine();
			//ImGui::TextColored(ImVec4(0, 0, 0, 1), "%d",
			//	cInventoryItemPlanet->GetCount());
			ImGui::End();
		}

		ImGuiWindowFlags shieldTextwindow_flags = 0;
		shieldTextwindow_flags |= ImGuiWindowFlags_NoTitleBar;
		shieldTextwindow_flags |= ImGuiWindowFlags_NoScrollbar;
		//window_flags |= ImGuiWindowFlags_MenuBar;
		shieldTextwindow_flags |= ImGuiWindowFlags_NoBackground;
		shieldTextwindow_flags |= ImGuiWindowFlags_NoMove;
		shieldTextwindow_flags |= ImGuiWindowFlags_NoCollapse;
		shieldTextwindow_flags |= ImGuiWindowFlags_NoNav;
		{
			static float f = 0.0f;
			static int counter = 0;

			// Create a window called "Hello, world!" and append into it.
			ImGui::Begin("shieldtext", NULL, shieldTextwindow_flags);
			ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 1.45,
				CSettings::GetInstance()->iWindowHeight / 1.85));
			ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight));
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("shield");
			ImGui::SetWindowFontScale(1.5f * relativeScale_y);
			ImGui::SameLine;
			ImGui::TextColored(ImVec4(0, 0, 0, 1), "Shield:%d",
				cInventoryItemPlanet->GetCount());
			ImGui::End();
		}
		ImGuiWindowFlags shieldwindow_flags = 0;
		shieldwindow_flags |= ImGuiWindowFlags_NoTitleBar;
		shieldwindow_flags |= ImGuiWindowFlags_NoScrollbar;
		//window_flags |= ImGuiWindowFlags_MenuBar;
		shieldwindow_flags |= ImGuiWindowFlags_NoBackground;
		shieldwindow_flags |= ImGuiWindowFlags_NoMove;
		shieldwindow_flags |= ImGuiWindowFlags_NoCollapse;
		shieldwindow_flags |= ImGuiWindowFlags_NoNav;
		shieldwindow_flags |= ImGuiWindowFlags_NoResize;
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("shield", NULL, shieldwindow_flags);
			ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 2.0 - buttonWidth / 2.0 - 100,
				CSettings::GetInstance()->iWindowHeight / 1.57 - 60));
			ImGui::SetWindowSize(ImVec2(buttonWidth, buttonHeight));
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("shield");
			ImGui::SetWindowFontScale(3.f * relativeScale_y);

			//Added rounding for nIronr effect
			ImGuiStyle& style = ImGui::GetStyle();
			style.FrameRounding = 200.0f;

			// Add codes for Start button here
			ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
				ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0));
			//ImGui::SameLine();
			//ImGui::TextColored(ImVec4(0, 0, 0, 1), "%d",
			//	cInventoryItemPlanet->GetCount());
			ImGui::End();
		}

		ImGuiWindowFlags freezeTextwindow_flags = 0;
		freezeTextwindow_flags |= ImGuiWindowFlags_NoTitleBar;
		freezeTextwindow_flags |= ImGuiWindowFlags_NoScrollbar;
		//window_flags |= ImGuiWindowFlags_MenuBar;
		freezeTextwindow_flags |= ImGuiWindowFlags_NoBackground;
		freezeTextwindow_flags |= ImGuiWindowFlags_NoMove;
		freezeTextwindow_flags |= ImGuiWindowFlags_NoCollapse;
		freezeTextwindow_flags |= ImGuiWindowFlags_NoNav;
		{
			static float f = 0.0f;
			static int counter = 0;

			// Create a window called "Hello, world!" and append into it.
			ImGui::Begin("freezetext", NULL, freezeTextwindow_flags);
			ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 1.45,
				CSettings::GetInstance()->iWindowHeight / 1.73));
			ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight));
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("freeze");
			ImGui::SetWindowFontScale(1.5f * relativeScale_y);
			ImGui::SameLine;
			ImGui::TextColored(ImVec4(0, 0, 0, 1), "Ice Shard:%d",
				cInventoryItemPlanet->GetCount());
			ImGui::End();
		}
		ImGuiWindowFlags freezewindow_flags = 0;
		freezewindow_flags |= ImGuiWindowFlags_NoTitleBar;
		freezewindow_flags |= ImGuiWindowFlags_NoScrollbar;
		//window_flags |= ImGuiWindowFlags_MenuBar;
		freezewindow_flags |= ImGuiWindowFlags_NoBackground;
		freezewindow_flags |= ImGuiWindowFlags_NoMove;
		freezewindow_flags |= ImGuiWindowFlags_NoCollapse;
		freezewindow_flags |= ImGuiWindowFlags_NoNav;
		freezewindow_flags |= ImGuiWindowFlags_NoResize;
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("freeze", NULL, freezewindow_flags);
			ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 3.0 * 2 - buttonWidth / 2.0 - 100,
				CSettings::GetInstance()->iWindowHeight / 1.57 - 60));
			ImGui::SetWindowSize(ImVec2(buttonWidth, buttonHeight));
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("freeze");
			ImGui::SetWindowFontScale(3.f * relativeScale_y);

			//Added rounding for nIronr effect
			ImGuiStyle& style = ImGui::GetStyle();
			style.FrameRounding = 200.0f;

			// Add codes for Start button here
			ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
				ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0));
			//ImGui::SameLine();
			//ImGui::TextColored(ImVec4(0, 0, 0, 1), "%d",
			//	cInventoryItemPlanet->GetCount());
			ImGui::End();
		}


		// resources
		ImGuiWindowFlags IceTextwindow_flags = 0;
		IceTextwindow_flags |= ImGuiWindowFlags_NoTitleBar;
		IceTextwindow_flags |= ImGuiWindowFlags_NoScrollbar;
		//window_flags |= ImGuiWindowFlags_MenuBar;
		IceTextwindow_flags |= ImGuiWindowFlags_NoBackground;
		IceTextwindow_flags |= ImGuiWindowFlags_NoMove;
		IceTextwindow_flags |= ImGuiWindowFlags_NoCollapse;
		IceTextwindow_flags |= ImGuiWindowFlags_NoNav;
		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			// Create a window called "Hello, world!" and append into it.
			ImGui::Begin("Icetext", NULL, IceTextwindow_flags);
			ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 1.45,
				CSettings::GetInstance()->iWindowHeight / 5.5));	// Set the top-left of the window at (10,10)
			ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight));
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("IceCrystal");
			ImGui::SetWindowFontScale(1.5f * relativeScale_y);
			ImGui::SameLine;
			ImGui::TextColored(ImVec4(0, 0, 0, 1), "Ice Crystal:%d",
				cInventoryItemPlanet->GetCount());
			ImGui::End();
		}
		ImGuiWindowFlags Icewindow_flags = 0;
		Icewindow_flags |= ImGuiWindowFlags_NoTitleBar;
		Icewindow_flags |= ImGuiWindowFlags_NoScrollbar;
		//window_flags |= ImGuiWindowFlags_MenuBar;
		Icewindow_flags |= ImGuiWindowFlags_NoBackground;
		Icewindow_flags |= ImGuiWindowFlags_NoMove;
		Icewindow_flags |= ImGuiWindowFlags_NoCollapse;
		Icewindow_flags |= ImGuiWindowFlags_NoNav;
		Icewindow_flags |= ImGuiWindowFlags_NoResize;

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			// Create a window called "Hello, world!" and append into it.
			ImGui::Begin("Ice Crystal", NULL, Icewindow_flags);
			ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 3.0 - buttonWidth / 2.0 - 100,
				CSettings::GetInstance()->iWindowHeight / 3.0-60));	// Set the top-left of the window at (10,10)
			ImGui::SetWindowSize(ImVec2(buttonWidth, buttonHeight));
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("IceCrystal");
			ImGui::SetWindowFontScale(3.f * relativeScale_y);

			//Added rounding for nicer effect
			ImGuiStyle& style = ImGui::GetStyle();
			style.FrameRounding = 200.0f;

			// Display the FPS
			//ImGui::TextColored(ImVec4(1, 1, 1, 1), "In-Game Menu");

			// Add codes for Start button here
			if (ImGui::ImageButton((ImTextureID)resourceSnow.textureID,
				ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0)))
			{
				// Reset the CKeyboardController
				CKeyboardController::GetInstance()->Reset();

				cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("IceCrystal");
				if (cInventoryItemPlanet->GetCount() > 0) {
					cInventoryItemPlanet->Remove(1);
					cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Resources");
					if (cInventoryItemPlanet->GetCount() > 0) {
						cInventoryItemPlanet->Remove(1);
					}
				}
			}
			//ImGui::SameLine();
			//ImGui::TextColored(ImVec4(0, 0, 0, 1), "%d",
			//	cInventoryItemPlanet->GetCount());
			ImGui::End();
		}
		
	}
	else if (cGUI_Scene2D->getPlanetNum() == 1) 
	{
		//PLANET SPECIFIC COLLECTABLES
		//river water
		ImGuiWindowFlags RiverWaterTextwindow_flags = 0;
		RiverWaterTextwindow_flags |= ImGuiWindowFlags_NoTitleBar;
		RiverWaterTextwindow_flags |= ImGuiWindowFlags_NoScrollbar;
		RiverWaterTextwindow_flags |= ImGuiWindowFlags_NoBackground;
		RiverWaterTextwindow_flags |= ImGuiWindowFlags_NoMove;
		RiverWaterTextwindow_flags |= ImGuiWindowFlags_NoCollapse;
		RiverWaterTextwindow_flags |= ImGuiWindowFlags_NoNav;
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("RiverWaterText", NULL, RiverWaterTextwindow_flags);
			ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 1.45,
				CSettings::GetInstance()->iWindowHeight / 2.0));
			ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight));
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("RiverWater");
			ImGui::SetWindowFontScale(1.5f * relativeScale_y);
			ImGui::SameLine;
			ImGui::TextColored(ImVec4(0, 0, 0, 1), "River Water:%d",
				cInventoryItemPlanet->GetCount());
			ImGui::End();
		}
		ImGuiWindowFlags RiverWaterwindow_flags = 0;
		RiverWaterwindow_flags |= ImGuiWindowFlags_NoTitleBar;
		RiverWaterwindow_flags |= ImGuiWindowFlags_NoScrollbar;
		RiverWaterwindow_flags |= ImGuiWindowFlags_NoBackground;
		RiverWaterwindow_flags |= ImGuiWindowFlags_NoMove;
		RiverWaterwindow_flags |= ImGuiWindowFlags_NoCollapse;
		RiverWaterwindow_flags |= ImGuiWindowFlags_NoNav;
		RiverWaterwindow_flags |= ImGuiWindowFlags_NoResize;
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("RiverWater", NULL, RiverWaterwindow_flags);
			ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 3.0 - buttonWidth / 2.0 - 100,
				CSettings::GetInstance()->iWindowHeight / 1.57 - 60));
			ImGui::SetWindowSize(ImVec2(buttonWidth, buttonHeight));
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("RiverWater");
			ImGui::SetWindowFontScale(3.f * relativeScale_y);

			//Added rounding for nIronr effect
			ImGuiStyle& style = ImGui::GetStyle();
			style.FrameRounding = 200.0f;

			// Add codes for Start button here
			ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
				ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0));
			ImGui::End();
		}

		//vines
		ImGuiWindowFlags VineTextwindow_flags = 0;
		VineTextwindow_flags |= ImGuiWindowFlags_NoTitleBar;
		VineTextwindow_flags |= ImGuiWindowFlags_NoScrollbar;
		VineTextwindow_flags |= ImGuiWindowFlags_NoBackground;
		VineTextwindow_flags |= ImGuiWindowFlags_NoMove;
		VineTextwindow_flags |= ImGuiWindowFlags_NoCollapse;
		VineTextwindow_flags |= ImGuiWindowFlags_NoNav;
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("VineText", NULL, VineTextwindow_flags);
			ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 1.45,
				CSettings::GetInstance()->iWindowHeight / 1.85));
			ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight));
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Vine");
			ImGui::SetWindowFontScale(1.5f * relativeScale_y);
			ImGui::SameLine;
			ImGui::TextColored(ImVec4(0, 0, 0, 1), "Vine:%d",
				cInventoryItemPlanet->GetCount());
			ImGui::End();
		}
		ImGuiWindowFlags Vinewindow_flags = 0;
		Vinewindow_flags |= ImGuiWindowFlags_NoTitleBar;
		Vinewindow_flags |= ImGuiWindowFlags_NoScrollbar;
		Vinewindow_flags |= ImGuiWindowFlags_NoBackground;
		Vinewindow_flags |= ImGuiWindowFlags_NoMove;
		Vinewindow_flags |= ImGuiWindowFlags_NoCollapse;
		Vinewindow_flags |= ImGuiWindowFlags_NoNav;
		Vinewindow_flags |= ImGuiWindowFlags_NoResize;
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Vine", NULL, Vinewindow_flags);
			ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 2.0 - buttonWidth / 2.0 - 100,
				CSettings::GetInstance()->iWindowHeight / 1.57 - 60));
			ImGui::SetWindowSize(ImVec2(buttonWidth, buttonHeight));
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Vine");
			ImGui::SetWindowFontScale(3.f * relativeScale_y);

			//Added rounding for nIronr effect
			ImGuiStyle& style = ImGui::GetStyle();
			style.FrameRounding = 200.0f;

			// Add codes for Start button here
			ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
				ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0));
			ImGui::End();
		}

		//burnable blocks
		ImGuiWindowFlags BBlocksTextwindow_flags = 0;
		BBlocksTextwindow_flags |= ImGuiWindowFlags_NoTitleBar;
		BBlocksTextwindow_flags |= ImGuiWindowFlags_NoScrollbar;
		BBlocksTextwindow_flags |= ImGuiWindowFlags_NoBackground;
		BBlocksTextwindow_flags |= ImGuiWindowFlags_NoMove;
		BBlocksTextwindow_flags |= ImGuiWindowFlags_NoCollapse;
		BBlocksTextwindow_flags |= ImGuiWindowFlags_NoNav;
		{
			static float f = 0.0f;
			static int counter = 0;

			// Create a window called "Hello, world!" and append into it.
			ImGui::Begin("BBlocksText", NULL, BBlocksTextwindow_flags);
			ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 1.45,
				CSettings::GetInstance()->iWindowHeight / 1.73));
			ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight));
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("BurnableBlocks");
			ImGui::SetWindowFontScale(1.5f * relativeScale_y);
			ImGui::SameLine;
			ImGui::TextColored(ImVec4(0, 0, 0, 1), "Bushes:%d",
				cInventoryItemPlanet->GetCount());
			ImGui::End();
		}
		ImGuiWindowFlags BBlockswindow_flags = 0;
		BBlockswindow_flags |= ImGuiWindowFlags_NoTitleBar;
		BBlockswindow_flags |= ImGuiWindowFlags_NoScrollbar;
		BBlockswindow_flags |= ImGuiWindowFlags_NoBackground;
		BBlockswindow_flags |= ImGuiWindowFlags_NoMove;
		BBlockswindow_flags |= ImGuiWindowFlags_NoCollapse;
		BBlockswindow_flags |= ImGuiWindowFlags_NoNav;
		BBlockswindow_flags |= ImGuiWindowFlags_NoResize;
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("BBlocks", NULL, BBlockswindow_flags);
			ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 3.0 * 2 - buttonWidth / 2.0 - 100,
				CSettings::GetInstance()->iWindowHeight / 1.57 - 60));
			ImGui::SetWindowSize(ImVec2(buttonWidth, buttonHeight));
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("BurnableBlocks");
			ImGui::SetWindowFontScale(3.f * relativeScale_y);

			//Added rounding for nIronr effect
			ImGuiStyle& style = ImGui::GetStyle();
			style.FrameRounding = 200.0f;

			// Add codes for Start button here
			ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
				ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0));
			ImGui::End();
		}

		//RESOURCE
		ImGuiWindowFlags IronTextwindow_flags = 0;
		IronTextwindow_flags |= ImGuiWindowFlags_NoTitleBar;
		IronTextwindow_flags |= ImGuiWindowFlags_NoScrollbar;
		//window_flags |= ImGuiWindowFlags_MenuBar;
		IronTextwindow_flags |= ImGuiWindowFlags_NoBackground;
		IronTextwindow_flags |= ImGuiWindowFlags_NoMove;
		IronTextwindow_flags |= ImGuiWindowFlags_NoCollapse;
		IronTextwindow_flags |= ImGuiWindowFlags_NoNav;
		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			// Create a window called "Hello, world!" and append into it.
			ImGui::Begin("Irontext", NULL, IronTextwindow_flags);
			ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 1.45,
				CSettings::GetInstance()->iWindowHeight / 5.5));	// Set the top-left of the window at (10,10)
			ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight));
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Ironwood");
			ImGui::SetWindowFontScale(1.5f * relativeScale_y);
			ImGui::SameLine;
			ImGui::TextColored(ImVec4(0, 0, 0, 1), "Ironwood:%d",
				cInventoryItemPlanet->GetCount());
			ImGui::End();
		}
		ImGuiWindowFlags Ironwindow_flags = 0;
		Ironwindow_flags |= ImGuiWindowFlags_NoTitleBar;
		Ironwindow_flags |= ImGuiWindowFlags_NoScrollbar;
		//window_flags |= ImGuiWindowFlags_MenuBar;
		Ironwindow_flags |= ImGuiWindowFlags_NoBackground;
		Ironwindow_flags |= ImGuiWindowFlags_NoMove;
		Ironwindow_flags |= ImGuiWindowFlags_NoCollapse;
		Ironwindow_flags |= ImGuiWindowFlags_NoNav;
		Ironwindow_flags |= ImGuiWindowFlags_NoResize;

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			// Create a window called "Hello, world!" and append into it.
			ImGui::Begin("Ironwood", NULL, Ironwindow_flags);
			ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 3.0 - buttonWidth / 2.0 - 100,
				CSettings::GetInstance()->iWindowHeight / 3.0 - 60));	// Set the top-left of the window at (10,10)
			ImGui::SetWindowSize(ImVec2(buttonWidth, buttonHeight));
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Ironwood");
			ImGui::SetWindowFontScale(3.f * relativeScale_y);

			//Added rounding for nIronr effect
			ImGuiStyle& style = ImGui::GetStyle();
			style.FrameRounding = 200.0f;

			// Display the FPS
			//ImGui::TextColored(ImVec4(1, 1, 1, 1), "In-Game Menu");

			// Add codes for Start button here
			if (ImGui::ImageButton((ImTextureID)resourceJungle.textureID,
				ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0)))
			{
				// Reset the CKeyboardController
				CKeyboardController::GetInstance()->Reset();
				cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Ironwood");
				if (cInventoryItemPlanet->GetCount() > 0) {
					cInventoryItemPlanet->Remove(1);
					cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Resources");
					if (cInventoryItemPlanet->GetCount() > 0) {
						cInventoryItemPlanet->Remove(1);
					}
				}
			}
			//ImGui::SameLine();
			//ImGui::TextColored(ImVec4(0, 0, 0, 1), "%d",
			//	cInventoryItemPlanet->GetCount());
			ImGui::End();
		}
	}
	else if (cGUI_Scene2D->getPlanetNum() == 2) {
		ImGuiWindowFlags EnergyTextwindow_flags = 0;
		EnergyTextwindow_flags |= ImGuiWindowFlags_NoTitleBar;
		EnergyTextwindow_flags |= ImGuiWindowFlags_NoScrollbar;
		//window_flags |= ImGuiWindowFlags_MenuBar;
		EnergyTextwindow_flags |= ImGuiWindowFlags_NoBackground;
		EnergyTextwindow_flags |= ImGuiWindowFlags_NoMove;
		EnergyTextwindow_flags |= ImGuiWindowFlags_NoCollapse;
		EnergyTextwindow_flags |= ImGuiWindowFlags_NoNav;
		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			// Create a window called "Hello, world!" and append into it.
			ImGui::Begin("energytext", NULL, EnergyTextwindow_flags);
			ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 1.45,
				CSettings::GetInstance()->iWindowHeight / 5.5));	// Set the top-left of the window at (10,10)
			ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight));
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("EnergyQuartz");
			ImGui::SetWindowFontScale(1.5f * relativeScale_y);
			ImGui::SameLine;
			ImGui::TextColored(ImVec4(0, 0, 0, 1), "Energy Quartz:%d",
				cInventoryItemPlanet->GetCount());
			ImGui::End();
		}
		ImGuiWindowFlags Energywindow_flags = 0;
		Energywindow_flags |= ImGuiWindowFlags_NoTitleBar;
		Energywindow_flags |= ImGuiWindowFlags_NoScrollbar;
		//window_flags |= ImGuiWindowFlags_MenuBar;
		Energywindow_flags |= ImGuiWindowFlags_NoBackground;
		Energywindow_flags |= ImGuiWindowFlags_NoMove;
		Energywindow_flags |= ImGuiWindowFlags_NoCollapse;
		Energywindow_flags |= ImGuiWindowFlags_NoNav;
		Energywindow_flags |= ImGuiWindowFlags_NoResize;

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			// Create a window called "Hello, world!" and append into it.
			ImGui::Begin("Energy Crystal", NULL, Energywindow_flags);
			ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 3.0 - buttonWidth / 2.0 - 100,
				CSettings::GetInstance()->iWindowHeight / 3.0 - 60));	// Set the top-left of the window at (10,10)
			ImGui::SetWindowSize(ImVec2(buttonWidth, buttonHeight));
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("EnergyQuartz");
			ImGui::SetWindowFontScale(3.f * relativeScale_y);

			//Added rounding for nEnergyr effect
			ImGuiStyle& style = ImGui::GetStyle();
			style.FrameRounding = 200.0f;

			// Display the FPS
			//ImGui::TextColored(ImVec4(1, 1, 1, 1), "In-Game Menu");

			// Add codes for Start button here
			if (ImGui::ImageButton((ImTextureID)resourceTerrestrial.textureID,
				ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0)))
			{
				// Reset the CKeyboardController
				CKeyboardController::GetInstance()->Reset();

				cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("EnergyQuartz");
				if (cInventoryItemPlanet->GetCount() > 0) {
					cInventoryItemPlanet->Remove(1);
					cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Resources");
					if (cInventoryItemPlanet->GetCount() > 0) {
						cInventoryItemPlanet->Remove(1);
					}
				}
			}
			//ImGui::SameLine();
			//ImGui::TextColored(ImVec4(0, 0, 0, 1), "%d",
			//	cInventoryItemPlanet->GetCount());
			ImGui::End();
		}
	}
//	ImGui::EndFrame();
	return true;
}

/**
 @brief Render this class instance
 */
void CInventoryState::Render(void)
{
	// Clear the screen and buffer
	glClearColor(0.0f, 0.55f, 1.00f, 1.00f);

	//Render Background
	background->Render();

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	//cout << "CInventoryState::Render()\n" << endl;
}

/**
 @brief Destroy this class instance
 */
void CInventoryState::Destroy(void)
{
	// cout << "CInventoryState::Destroy()\n" << endl;

	// Delete the background
	if (background)
	{
		delete background;
		background = NULL;
	}

}
