/**
 CGUI_Scene2D
 @brief A class which manages the GUI for Scene2D
 By: Toh Da Jun
 Date: May 2021
 */
#include "GUI_SceneCombat.h"

// Include Mesh Builder
#include "Primitives/MeshBuilder.h"
// Include ImageLoader
#include "System\ImageLoader.h"
// Include Shader Manager
#include "RenderControl\ShaderManager.h"

#include <iostream>
using namespace std;

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CGUI_SceneCombat::CGUI_SceneCombat(void)
	: cSettings(NULL)
	, window_flags(0)
	, cInventoryManager(NULL)
	, cInventoryItem(NULL)
{
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CGUI_SceneCombat::~CGUI_SceneCombat(void)
{
	if (cInventoryManager)
	{
		cInventoryManager->Destroy();
		cInventoryManager = NULL;
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
bool CGUI_SceneCombat::Init(void)
{
	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	// Store the CFPSCounter singleton instance here
	cFPSCounter = CFPSCounter::GetInstance();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->AddFontFromFileTTF("Image/GUI/quaver.ttf", 10);
	io.Fonts->Build();

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

	cInventoryManager = CInventoryManager::GetInstance();

	CImageLoader* il = CImageLoader::GetInstance();
	AcceptButtonData.fileName = "Image\\GUI\\tick.png";
	AcceptButtonData.textureID = il->LoadTextureGetID(AcceptButtonData.fileName.c_str(), false);
	RejectButtonData.fileName = "Image\\GUI\\cross.png";
	RejectButtonData.textureID = il->LoadTextureGetID(RejectButtonData.fileName.c_str(), false);

	// Decoration
	PlayerBarTextureID = il->LoadTextureGetID("Image/GUI/PlayerBar.png", false);
	BitTextureID = il->LoadTextureGetID("Image/GUI/ProgressBit.png", false);

	// Show the mouse pointer
	glfwSetInputMode(CSettings::GetInstance()->pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	isShowPanel = false;
	GuiState = GUI_STATE::noShow;
	makeChanges = false;

	return true;
}

/**
 @brief Update this instance
 */
void CGUI_SceneCombat::Update(const double dElapsedTime)
{
	// Calculate the relative scale to our default windows width
	const float relativeScale_x = cSettings->iWindowWidth / 800.0f;
	const float relativeScale_y = cSettings->iWindowHeight / 600.0f;
	CImageLoader* il = CImageLoader::GetInstance();

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
	ImGui::SetWindowFontScale(2.f * relativeScale_y);

	// Render the Health
	/*ImGuiWindowFlags healthWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
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
	cInventoryItem = cInventoryManager->GetItem("Health");
	ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
		ImVec2(cInventoryItem->vec2Size.x * relativeScale_x,
			cInventoryItem->vec2Size.y * relativeScale_y),
		ImVec2(0, 1), ImVec2(1, 0));
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
	ImGui::ProgressBar(cInventoryItem->GetCount() /
		(float)cInventoryItem->GetMaxCount(), ImVec2(100.0f *
			relativeScale_x, 20.0f * relativeScale_y));
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::End();*/

	ImGuiWindowFlags healthWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar;

	ImGui::Begin("Health", NULL, healthWindowFlags);
	ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.01f,
		cSettings->iWindowHeight * 0.01f));
	ImGui::SetWindowSize(ImVec2(2.0f * relativeScale_x, 2.0f * relativeScale_y));
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);

	ImGui::Image((void*)(intptr_t)PlayerBarTextureID, ImVec2(300, 100));
	ImGui::End();

	// how. do i. get rid. of the border.
	cInventoryItem = cInventoryManager->GetItem("Health");
	int healthCount = cInventoryItem->GetCount();
	for (int i = 0; i < int(healthCount / int(cInventoryItem->GetMaxCount() / 10)); i++) {
		ImGui::Begin("Health Bit", NULL, healthWindowFlags);
		ImGui::SetWindowPos(ImVec2((cSettings->iWindowWidth) * 0.13f,
			cSettings->iWindowHeight * 0.11f));
		ImGui::SetWindowSize(ImVec2(1.0f * relativeScale_x, 1.0f * relativeScale_y));

		ImGui::Image((void*)(intptr_t)BitTextureID, ImVec2(10, 14));
		ImGui::SameLine();
		ImGui::GetOverlayDrawList();
		ImGui::End();
	}

	ImGuiWindowFlags livesWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar;

	// Render panels for click click
	switch (GuiState)
	{
	case CGUI_SceneCombat::showRepair:
		ImGui::Begin("Lives", NULL, livesWindowFlags);
		ImGui::SetWindowPos(ImVec2(blockPosition.x + cSettings->iWindowWidth * 0.05f ,
			blockPosition.y - cSettings->iWindowHeight * 0.1f));
		ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));

		// planet information
		ImGui::SetWindowFontScale(1.8f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 1, 1), "Repair Ship Tile?");
		ImGui::SetWindowFontScale(1.6f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Scrap x 1");
		ImGui::NewLine();
		// Add codes for Start button here
		if (ImGui::ImageButton((ImTextureID)AcceptButtonData.textureID,
			ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0)))
		{
			makeChanges = true;
		}
		ImGui::SameLine();
		// Add codes for Start button here
		if (ImGui::ImageButton((ImTextureID)RejectButtonData.textureID,
			ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0)))
		{
			GuiState = GUI_STATE::noShow;
			makeChanges = false;
		}

		ImGui::End();
		break;
	case CGUI_SceneCombat::showWeapons:
		break;
	case CGUI_SceneCombat::showStorage:
		break;
	case CGUI_SceneCombat::showExit:
		ImGui::Begin("Exit", NULL, livesWindowFlags);
		ImGui::SetWindowPos(ImVec2(blockPosition.x + cSettings->iWindowWidth * 0.05f,
			blockPosition.y - cSettings->iWindowHeight * 0.1f));
		ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));

		// planet information
		ImGui::SetWindowFontScale(1.8f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 1, 1), "Start Exploring?");
		ImGui::NewLine();
		// Add codes for Start button here
		if (ImGui::ImageButton((ImTextureID)AcceptButtonData.textureID,
			ImVec2(buttonWidth * relativeScale_x, buttonHeight * relativeScale_y), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0)))
		{
			makeChanges = true;
		}
		ImGui::SameLine();
		// Add codes for Start button here
		if (ImGui::ImageButton((ImTextureID)RejectButtonData.textureID,
			ImVec2(buttonWidth * relativeScale_x, buttonHeight * relativeScale_y), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0)))
		{
			GuiState = GUI_STATE::noShow;
			makeChanges = false;
		}

		ImGui::End();
		break;
	default:
		break;
	}

	ImGui::End();
}


/**
 @brief Set up the OpenGL display environment before rendering
 */
void CGUI_SceneCombat::PreRender(void)
{
}

/**
 @brief Render this instance
 */
void CGUI_SceneCombat::Render(void)
{
	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CGUI_SceneCombat::PostRender(void)
{
}