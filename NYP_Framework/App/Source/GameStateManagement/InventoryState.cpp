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
	else if (cGUI_Scene2D->getPlanetNum() == 1) {
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
