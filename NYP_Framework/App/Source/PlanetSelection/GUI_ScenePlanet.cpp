/**
 CGUI_Scene2D
 @brief A class which manages the GUI for Scene2D
 By: Toh Da Jun
 Date: May 2021
 */
#include "GUI_ScenePlanet.h"

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
CGUI_ScenePlanet::CGUI_ScenePlanet(void)
	: cSettings(NULL)
	, window_flags(0)
	, cPlanet(NULL)
	, cInventoryManager(NULL)
	, cInventoryItem(NULL)
{
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CGUI_ScenePlanet::~CGUI_ScenePlanet(void)
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
bool CGUI_ScenePlanet::Init(void)
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
	io.WantCaptureMouse;

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

	CImageLoader* il = CImageLoader::GetInstance();
	StartCombatButtonData.fileName = "Image\\GUI\\Button_VolDown.png";
	StartCombatButtonData.textureID = il->LoadTextureGetID(StartCombatButtonData.fileName.c_str(), false);

	// Show the mouse pointer
	glfwSetInputMode(CSettings::GetInstance()->pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	isShowPanel = false;
	StartCombat = false;

	return true;
}

/**
 @brief Update this instance
 */
void CGUI_ScenePlanet::Update(const double dElapsedTime)
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

	float buttonWidth = 128;
	float buttonHeight = 64;


	// Create an invisible window which covers the entire OpenGL window
	ImGui::Begin("Invisible window", NULL, window_flags);
	ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetWindowSize(ImVec2((float)cSettings->iWindowWidth, (float)cSettings->iWindowHeight));
	ImGui::SetWindowFontScale(2.f * relativeScale_y);

	// Display the FPS
	ImGui::TextColored(ImVec4(1, 1, 1, 1), "INTER-GALACTICAL MAP SYSTEM");
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);
	ImGui::TextColored(ImVec4(1, 1, 1, 1), "Press 'I' to view your inventory.");

	ImGui::End();

	// Render the panel
	if (cPlanet != NULL && isShowPanel) {
		ImGuiWindowFlags livesWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar;
		ImGui::Begin("Lives", NULL, livesWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.6f,
			cSettings->iWindowHeight * 0.45f));
		ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));

		// planet information
		ImGui::SetWindowFontScale(1.8f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 1, 1), "Planet Name: ");
		ImGui::SetWindowFontScale(1.6f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 0, 1), cPlanet->planetName);
		ImGui::NewLine();
		switch (cPlanet->getType())
		{
		case CPlanet::TYPE::JUNGLE:
		case CPlanet::TYPE::JUNGLE_TUTORIAL:
			ImGui::SetWindowFontScale(1.8f * relativeScale_y);
			ImGui::TextColored(ImVec4(1, 1, 1, 1), "Key Resources: ");
			ImGui::SetWindowFontScale(1.6f * relativeScale_y);
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "Ironwood\nScrap Metal\nBatteries");
			break;
		case CPlanet::TYPE::SNOW:
		case CPlanet::TYPE::SNOW_TUTORIAL:
			ImGui::SetWindowFontScale(1.8f * relativeScale_y);
			ImGui::TextColored(ImVec4(1, 1, 1, 1), "Key Resources: ");
			ImGui::SetWindowFontScale(1.6f * relativeScale_y);
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "Ice Crystals\nHound Teeth\nScrap Metal\nBatteries");
			break;
		case CPlanet::TYPE::TERRESTRIAL:
		case CPlanet::TYPE::TERRESTRIAL_TUTORIAL:
			ImGui::SetWindowFontScale(1.8f * relativeScale_y);
			ImGui::TextColored(ImVec4(1, 1, 1, 1), "Key Resources: ");
			ImGui::SetWindowFontScale(1.6f * relativeScale_y);
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "Energy Quartz\nScrap Metal\nBatteries");
			break;
		default:
			break;
		}
		ImGui::End();
	}

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		// Create a window called "Hello, world!" and append into it.
		ImGui::Begin("Main Menu", NULL, window_flags);
		ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth * 0.8,
			CSettings::GetInstance()->iWindowHeight * 0.85));				// Set the top-left of the window at (10,10)
		ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight));

		// Add codes for Start button here
		if (ImGui::ImageButton((ImTextureID)StartCombatButtonData.textureID,
			ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0)))
		{
			// To next scene
			StartCombat = true;
		}
		ImGui::End();
	}
}


/**
 @brief Set up the OpenGL display environment before rendering
 */
void CGUI_ScenePlanet::PreRender(void)
{
}

/**
 @brief Render this instance
 */
void CGUI_ScenePlanet::Render(void)
{
	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CGUI_ScenePlanet::PostRender(void)
{
}

void CGUI_ScenePlanet::setPlanetNum(int num)
{
	planetNum = num;
}

int CGUI_ScenePlanet::getPlanetNum(void)
{
	return planetNum;
}

void CGUI_ScenePlanet::setPlanetInfo(CPlanet* cPlanet)
{
	this->cPlanet = cPlanet;
}
