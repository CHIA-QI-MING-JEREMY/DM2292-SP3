/**
 CGUI_Scene2D
 @brief A class which manages the GUI for Scene2D
 By: Toh Da Jun
 Date: May 2021
 */
#include "GUI_Scene2D.h"
#include "System\ImageLoader.h"

#include <iostream>
using namespace std;

#include "../GameStateManagement/GameStateManager.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CGUI_Scene2D::CGUI_Scene2D(void)
	: cSettings(NULL)
	, window_flags(0)
	, m_fProgressBar(0.0f)
	, cInventoryManager(NULL)
	, cInventoryItem(NULL)
	, cInventoryManagerPlanet(NULL)
	, cInventoryItemPlanet(NULL)
{
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CGUI_Scene2D::~CGUI_Scene2D(void)
{
	if (cInventoryManager)
	{
		cInventoryManager->Destroy();
		cInventoryManager = NULL;
	}
	if (cInventoryManagerPlanet)
	{
		cInventoryManagerPlanet->Destroy();
		cInventoryManagerPlanet = NULL;
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// We won't delete this since it was created elsewhere
	cSettings = NULL;
}

/**
  @brief Initialise this instance
  */
bool CGUI_Scene2D::Init(void)
{
	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	// Store the CFPSCounter singleton instance here
	cFPSCounter = CFPSCounter::GetInstance();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(CSettings::GetInstance()->pWindow, true);
	const char* glsl_version = "#version 330";
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Define the window flags
	window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoBackground;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;

	//// Show the mouse pointer
	//glfwSetInputMode(CSettings::GetInstance()->pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	CImageLoader* il = CImageLoader::GetInstance();
	AcceptButtonData.fileName = "Image\\GUI\\tick.png";
	AcceptButtonData.textureID = il->LoadTextureGetID(AcceptButtonData.fileName.c_str(), false);

	// Initialise the CInventoryManagerPlanet
	cInventoryManagerPlanet = CInventoryManagerPlanet::GetInstance();
	lState = false;

	m_fProgressBar = 0.0f;
	goOnShip = false;
	showExitPanel = false;

	//tutorialPopupJungle = NONE;
	tutorialPopupJungle = 1;

	return true;
}

/**
 @brief Update this instance
 */
void CGUI_Scene2D::Update(const double dElapsedTime)
{
	// Calculate the relative scale to our default windows width
	const float relativeScale_x = cSettings->iWindowWidth / 800.0f;
	const float relativeScale_y = cSettings->iWindowHeight / 600.0f;

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// prevents crash when game is minimised
	if ((relativeScale_x == 0.0f) || (relativeScale_y == 0.0f))
	{
		return;
	}

	float buttonWidth = 25;
	float buttonHeight = 25;

	// Create an invisible window which covers the entire OpenGL window
	ImGui::Begin("Invisible window", NULL, window_flags);
	ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetWindowSize(ImVec2((float)cSettings->iWindowWidth, (float)cSettings->iWindowHeight));
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);

	if (CGameStateManager::GetInstance()->hasPauseGameState() == false) {
		// Display the FPS
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %d", cFPSCounter->GetFrameRate());

		// Render the Health
		ImGuiWindowFlags healthWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar;
		ImGui::Begin("Health", NULL, healthWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.01f,
			cSettings->iWindowHeight * 0.05f));
		ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Health");
		ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
			ImVec2(cInventoryItemPlanet->vec2Size.x * relativeScale_x,
				cInventoryItemPlanet->vec2Size.y * relativeScale_y),
			ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		ImGui::ProgressBar(cInventoryItemPlanet->GetCount() /
			(float)cInventoryItemPlanet->GetMaxCount(), ImVec2(100.0f *
				relativeScale_x, 20.0f * relativeScale_y));
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::End();

		// Render the Lives
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));  // Set a background color
		ImGuiWindowFlags livesWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar;
		ImGui::Begin("Lives", NULL, livesWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.85f,
			cSettings->iWindowHeight * 0.02f));
		ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Lives");
		ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
			ImVec2(cInventoryItemPlanet->vec2Size.x * relativeScale_x,
				cInventoryItemPlanet->vec2Size.y * relativeScale_y),
			ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d / %d",
			cInventoryItemPlanet->GetCount(), cInventoryItemPlanet->GetMaxCount());
		ImGui::End();
		ImGui::PopStyleColor();


		if (showExitPanel) {
			ImGuiWindowFlags ExitWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoScrollbar;
			ImGui::Begin("Exit Panel", NULL, ExitWindowFlags);
			ImGui::SetWindowPos(ImVec2(blockPosition.x + cSettings->iWindowWidth * 0.05f,
				blockPosition.y - cSettings->iWindowHeight * 0.1f));
			ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));

			// planet information
			ImGui::SetWindowFontScale(1.5f * relativeScale_y);
			ImGui::TextColored(ImVec4(1, 1, 1, 1), "Go Back to Ship?");
			ImGui::SetWindowFontScale(1.2f * relativeScale_y);
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "This action cannot be undone.");
			ImGui::NewLine();
			// Add codes for Start button here
			if (ImGui::ImageButton((ImTextureID)AcceptButtonData.textureID,
				ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0)))
			{
				goOnShip = true;
				showExitPanel = false;
			}

			ImGui::End();
		}
	}

	if (planetNum == 1 && CGameStateManager::GetInstance()->hasPauseGameState() == false)
	{
		//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));  // Set a background color
		//ImGuiWindowFlags textPopupFlags = ImGuiWindowFlags_AlwaysAutoResize |
		//	ImGuiWindowFlags_NoTitleBar |
		//	ImGuiWindowFlags_NoMove |
		//	ImGuiWindowFlags_NoResize |
		//	ImGuiWindowFlags_NoCollapse |
		//	ImGuiWindowFlags_NoScrollbar;
		//ImGui::Begin("textPopup", NULL, textPopupFlags);
		//ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.5f,
		//	cSettings->iWindowHeight * 0.5f));
		//ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 100.0f * relativeScale_y));
		//ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		//ImGui::TextColored(ImVec4(1, 1, 0, 1), "Hello");
		//ImGui::End();
		//ImGui::PopStyleColor();

		// Render the Poison Level
		ImGuiWindowFlags poisonWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar;
		ImGui::Begin("PoisonLevel", NULL, poisonWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.01f,
			cSettings->iWindowHeight * 0.125f));
		ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("PoisonLevel");
		ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
			ImVec2(cInventoryItemPlanet->vec2Size.x * relativeScale_x,
				cInventoryItemPlanet->vec2Size.y * relativeScale_y),
			ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.0f, 0.9f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
		ImGui::ProgressBar(cInventoryItemPlanet->GetCount() /
			(float)cInventoryItemPlanet->GetMaxCount(), ImVec2(100.0f *
				relativeScale_x, 20.0f * relativeScale_y));
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::End();

		// Render the inventory items
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));  // Set a background color
		ImGuiWindowFlags riverWaterWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar;
		ImGui::Begin("RiverWater", NULL, riverWaterWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.03f, cSettings->iWindowHeight * 0.9f));
		ImGui::SetWindowSize(ImVec2(200.0f * relativeScale_x, 25.0f * relativeScale_y));
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("RiverWater");
		ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
			ImVec2(cInventoryItemPlanet->vec2Size.x * relativeScale_x, cInventoryItemPlanet->vec2Size.y * relativeScale_y),
			ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d / %d",
			cInventoryItemPlanet->GetCount(), cInventoryItemPlanet->GetMaxCount());
		ImGui::End();
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));  // Set a background color
		ImGuiWindowFlags vineWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar;
		ImGui::Begin("Vine", NULL, vineWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.155f, cSettings->iWindowHeight * 0.9f));
		ImGui::SetWindowSize(ImVec2(200.0f * relativeScale_x, 25.0f * relativeScale_y));
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Vine");
		ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
			ImVec2(cInventoryItemPlanet->vec2Size.x * relativeScale_x, cInventoryItemPlanet->vec2Size.y * relativeScale_y),
			ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d / %d",
			cInventoryItemPlanet->GetCount(), cInventoryItemPlanet->GetMaxCount());
		ImGui::End();
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));  // Set a background color
		ImGuiWindowFlags bBlocksWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar;
		ImGui::Begin("BurnableBlocks", NULL, bBlocksWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.28f, cSettings->iWindowHeight * 0.9f));
		ImGui::SetWindowSize(ImVec2(200.0f * relativeScale_x, 25.0f * relativeScale_y));
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("BurnableBlocks");
		ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
			ImVec2(cInventoryItemPlanet->vec2Size.x * relativeScale_x, cInventoryItemPlanet->vec2Size.y * relativeScale_y),
			ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d / %d",
			cInventoryItemPlanet->GetCount(), cInventoryItemPlanet->GetMaxCount());
		ImGui::End();
		ImGui::PopStyleColor();

		//tutorial popups
		if (tutorialPopupJungle > 0) //there is a pop up
		{
			//popup window
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));  // Set a background color
			ImGuiWindowFlags textPopupFlags = ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoScrollbar;
			ImGui::Begin("textPopup", NULL, textPopupFlags);
			ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.01f,
				cSettings->iWindowHeight * 0.5f));
			ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 100.0f * relativeScale_y));

			//different popup text for different popups
			switch (tutorialPopupJungle)
			{
			case CHECKPOINT:
				//information
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Checkpoints");
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Activate checkpoints to set \nrespawn points");
				ImGui::NewLine();
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Press R to give 1 life to respawn \nHealth will be reset to 100%");
				break;
			default:
				break;
			}
			
			ImGui::End();
			ImGui::PopStyleColor();
		}
	}
	else if (planetNum == 2 && CGameStateManager::GetInstance()->hasPauseGameState() == false)
	{
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));  // Set a background color
		ImGuiWindowFlags textPopupFlags = ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar;
		ImGui::Begin("textPopup", NULL, textPopupFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.5f,
			cSettings->iWindowHeight * 0.5f));
		ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 100.0f * relativeScale_y));
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Hello");
		ImGui::End();
		ImGui::PopStyleColor();

		// Render the Toxicity Level
		ImGuiWindowFlags toxicityWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar;
		ImGui::Begin("ToxicityLevel", NULL, toxicityWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.01f,
			cSettings->iWindowHeight * 0.125f));
		ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("ToxicityLevel");
		ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
			ImVec2(cInventoryItemPlanet->vec2Size.x * relativeScale_x,
				cInventoryItemPlanet->vec2Size.y * relativeScale_y),
			ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.66f, 0.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
		ImGui::ProgressBar(cInventoryItemPlanet->GetCount() /
			(float)cInventoryItemPlanet->GetMaxCount(), ImVec2(100.0f *
				relativeScale_x, 20.0f * relativeScale_y));
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::End();
		
		// Render the inventory items
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));  // Set a background color
		ImGuiWindowFlags yellowOrbWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar;
		ImGui::Begin("YellowOrb", NULL, yellowOrbWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.03f, cSettings->iWindowHeight * 0.9f));
		ImGui::SetWindowSize(ImVec2(200.0f * relativeScale_x, 25.0f * relativeScale_y));
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("YellowOrb");
		ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
			ImVec2(cInventoryItemPlanet->vec2Size.x * relativeScale_x, cInventoryItemPlanet->vec2Size.y * relativeScale_y),
			ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d / %d",
			cInventoryItemPlanet->GetCount(), cInventoryItemPlanet->GetMaxCount());
		ImGui::End();
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));  // Set a background color
		ImGuiWindowFlags redOrbWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar;
		ImGui::Begin("RedOrb", NULL, redOrbWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.155f, cSettings->iWindowHeight * 0.9f));
		ImGui::SetWindowSize(ImVec2(200.0f * relativeScale_x, 25.0f * relativeScale_y));
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("RedOrb");
		ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
			ImVec2(cInventoryItemPlanet->vec2Size.x * relativeScale_x, cInventoryItemPlanet->vec2Size.y * relativeScale_y),
			ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d / %d",
			cInventoryItemPlanet->GetCount(), cInventoryItemPlanet->GetMaxCount());
		ImGui::End();
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));  // Set a background color
		ImGuiWindowFlags greenOrbWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar;
		ImGui::Begin("GreenOrb", NULL, greenOrbWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.28f, cSettings->iWindowHeight * 0.9f));
		ImGui::SetWindowSize(ImVec2(200.0f * relativeScale_x, 25.0f * relativeScale_y));
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("GreenOrb");
		ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
			ImVec2(cInventoryItemPlanet->vec2Size.x * relativeScale_x, cInventoryItemPlanet->vec2Size.y * relativeScale_y),
			ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d / %d",
			cInventoryItemPlanet->GetCount(), cInventoryItemPlanet->GetMaxCount());
		ImGui::End();
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));  // Set a background color
		ImGuiWindowFlags blueOrbWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar;
		ImGui::Begin("BlueOrb", NULL, blueOrbWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.405f, cSettings->iWindowHeight * 0.9f));
		ImGui::SetWindowSize(ImVec2(200.0f * relativeScale_x, 25.0f * relativeScale_y));
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("BlueOrb");
		ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
			ImVec2(cInventoryItemPlanet->vec2Size.x * relativeScale_x, cInventoryItemPlanet->vec2Size.y * relativeScale_y),
			ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d / %d",
			cInventoryItemPlanet->GetCount(), cInventoryItemPlanet->GetMaxCount());
		ImGui::End();
		ImGui::PopStyleColor();
	}
	else if (planetNum == 3 && CGameStateManager::GetInstance()->hasPauseGameState() == false) {
	// Render the temp Level
	ImGuiWindowFlags tempWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("temperature", NULL, tempWindowFlags);
	ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.01f,
		cSettings->iWindowHeight * 0.125f));
	ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);
	cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Temperature");
	ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
		ImVec2(cInventoryItemPlanet->vec2Size.x* relativeScale_x,
			cInventoryItemPlanet->vec2Size.y* relativeScale_y),
		ImVec2(0, 1), ImVec2(1, 0));
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.53f, 0.81f, 0.92f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
	ImGui::ProgressBar(cInventoryItemPlanet->GetCount()/
			(float)cInventoryItemPlanet->GetMaxCount(), ImVec2(100.0f *
		relativeScale_x, 20.0f * relativeScale_y)," ");
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::End();
	// Render the inventory items
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));  // Set a background color
	ImGuiWindowFlags berserkWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("berserk", NULL, berserkWindowFlags);
	ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.03f, cSettings->iWindowHeight * 0.9f));
	ImGui::SetWindowSize(ImVec2(200.0f * relativeScale_x, 25.0f * relativeScale_y));
	cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("berserk");
	ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
		ImVec2(cInventoryItemPlanet->vec2Size.x* relativeScale_x, cInventoryItemPlanet->vec2Size.y* relativeScale_y),
		ImVec2(0, 1), ImVec2(1, 0));
	ImGui::SameLine();
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d",
		cInventoryItemPlanet->GetCount());
	ImGui::End();
	ImGui::PopStyleColor();

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));  // Set a background color
	ImGuiWindowFlags freezeWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("freeze", NULL, freezeWindowFlags);
	ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.155f, cSettings->iWindowHeight * 0.9f));
	ImGui::SetWindowSize(ImVec2(200.0f * relativeScale_x, 25.0f * relativeScale_y));
	cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("freeze");
	ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
		ImVec2(cInventoryItemPlanet->vec2Size.x* relativeScale_x, cInventoryItemPlanet->vec2Size.y* relativeScale_y),
		ImVec2(0, 1), ImVec2(1, 0));
	ImGui::SameLine();
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d",
		cInventoryItemPlanet->GetCount());
	ImGui::End();
	ImGui::PopStyleColor();

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));  // Set a background color
	ImGuiWindowFlags shieldWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("shield", NULL, shieldWindowFlags);
	ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.28f, cSettings->iWindowHeight * 0.9f));
	ImGui::SetWindowSize(ImVec2(200.0f * relativeScale_x, 25.0f * relativeScale_y));
	cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("shield");
	ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
		ImVec2(cInventoryItemPlanet->vec2Size.x* relativeScale_x, cInventoryItemPlanet->vec2Size.y* relativeScale_y),
		ImVec2(0, 1), ImVec2(1, 0));
	ImGui::SameLine();
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d",
		cInventoryItemPlanet->GetCount());
	ImGui::End();
	ImGui::PopStyleColor();
}
	ImGui::End();
	if (CGameStateManager::GetInstance()->hasPauseGameState() == false)
		ImGui::EndFrame();
}


/**
 @brief Set up the OpenGL display environment before rendering
 */
void CGUI_Scene2D::PreRender(void)
{
}

/**
 @brief Render this instance
 */
void CGUI_Scene2D::Render(void)
{
	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CGUI_Scene2D::PostRender(void)
{
}

void CGUI_Scene2D::setPlanetNum(int num)
{
	planetNum = num;
}

int CGUI_Scene2D::getPlanetNum(void)
{
	return planetNum;
}

bool CGUI_Scene2D::getGoOnShip(void)
{
	return goOnShip;
}

bool CGUI_Scene2D::getShowExitPanel(void)
{
	return showExitPanel;
}

void CGUI_Scene2D::setShowExitPanel(bool newSet)
{
	showExitPanel = newSet;
}

void CGUI_Scene2D::setBlockPosition(glm::vec2 blockPos)
{
	blockPosition = blockPos;
}

int CGUI_Scene2D::getTutorialPopupJungle(void)
{
	return tutorialPopupJungle;
}

void CGUI_Scene2D::setTutorialPopupJungle(int index)
{
	tutorialPopupJungle = index;
}
