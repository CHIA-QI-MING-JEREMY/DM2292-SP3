/**
 CGUI_Scene2D
 @brief A class which manages the GUI for Scene2D
 By: Toh Da Jun
 Date: May 2021
 */
#include "GUI_Scene2D.h"
#include "System\ImageLoader.h"
#include "../App/Source/GameStateManagement/GameInfo.h"

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
	, cInventoryManagerPlanet(NULL)
	, cInventoryItemPlanet(NULL)
{
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CGUI_Scene2D::~CGUI_Scene2D(void)
{
	if (cInventoryManagerPlanet)
	{
		cInventoryManagerPlanet->Destroy();
		cInventoryManagerPlanet = NULL;
	}

	cInventoryManager = NULL;

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
	cInventoryManager = CGameInfo::GetInstance()->ImportIM();
	lState = false;

	m_fProgressBar = 0.0f;
	goOnShip = false;
	showExitPanel = false;

	PlayerBarTextureID = il->LoadTextureGetID("Image/GUI/PlayerBar.png", false);
	BitTextureID = il->LoadTextureGetID("Image/GUI/ProgressBit.png", false);
	ProgressBarTextureID = il->LoadTextureGetID("Image/GUI/ShipProgressBar.png", false);

	tutorialPopupJungle = NONE;

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
		// Render the Health
		ImGuiWindowFlags healthWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar;

		ImGuiWindowFlags overlayWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoScrollbar;


		ImGui::Begin("Health", NULL, overlayWindowFlags);
		ImGui::SetWindowPos(ImVec2(8 * relativeScale_y,
			cSettings->iWindowHeight * (0.01f)));
		ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_y, 10.0f * relativeScale_y));
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);

		ImGui::Image((void*)(intptr_t)PlayerBarTextureID, ImVec2(300 * relativeScale_y, 100 * relativeScale_y));
		ImGui::End();

		// how. do i. get rid. of the border.
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Health");
		int healthCount = cInventoryItemPlanet->GetCount();
		for (int i = 0; i < int(healthCount / int(cInventoryItemPlanet->GetMaxCount() / 10)); i++) {
			ImGui::Begin("Health Bit", NULL, healthWindowFlags);
			ImGui::SetWindowPos(ImVec2(104.f * relativeScale_y, cSettings->iWindowHeight * (0.11f)));
			ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_y, 10.0f * relativeScale_y));

			ImGui::Image((void*)(intptr_t)BitTextureID, ImVec2(9 * relativeScale_x, 14 * relativeScale_y));
			ImGui::SameLine();
			ImGui::GetOverlayDrawList();
			ImGui::End();
		}

		/*ImGui::Begin("Health", NULL, healthWindowFlags);
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
		ImGui::End();*/

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
		ImGui::TextColored(ImVec4(0, 1, 1, 1), "%d / %d",
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
			ImGui::SetWindowFocus();

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
			cSettings->iWindowHeight * 0.2f));
		ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameRounding = 200.0f;
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
		ImGui::TextColored(ImVec4(0, 1, 1, 1), "%d / %d",
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
		ImGui::TextColored(ImVec4(0, 1, 1, 1), "%d / %d",
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
		ImGui::TextColored(ImVec4(0, 1, 1, 1), "%d / %d",
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
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Checkpoints");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Activate checkpoint to set respawn\npoint \nTouching checkpoints restores health\nto full \n\nOnly 1 checkpoint can be active at 1\ntime \nActivating a new checkpoint \ndeactivates all others");
				ImGui::NewLine();
				//controls instructions
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Press R to lose 1 heart to respawn");
				break;
			case SHOOT:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Shooting");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Shoot fireballs up, down, left or \nright \nFace the direction you want to shoot\nin");
				ImGui::NewLine();
				//controls instructions
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "WASD to change which direction to \nface \nPress E to shoot");
				break;
			case BURNABLE:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Bushes");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Hit a bush with fireballs \n\n1st hit sets the bush on fire \n2nd destroys the bush \n\nEnemies can dissolve and destroy \nbushes too");
				break;
			case RESOURCE:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Resources");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "You can only collect 5 resources \nto bring back to the ship \n\nYou can destroy resources you don't \nwant from your inventory");
				ImGui::NewLine();
				//controls instructions
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "I to toggle inventory"); 
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Left click a resource to destroy it");
				break;
			case POISON:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Poison Levels (PL)");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Purple objects are poisonous \n\nPoisonous objects increase your PL \nThere is a short cooldown between \nchanges in PL by static objects\n\nYou take damage periodically when \npoisoned\n\nPL 1: -3 health/5 sec \nPL 2: -5 health/4 sec \nPL 3: -10 health/3 sec");
				break;
			case SWITCHES:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Buttons");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Hit a button with fireballs to \nactivate it \n\nActivated buttons can destroy metal \nbarriers");
				break;
			case RIVER_WATER:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "River Water");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "You regain health standing in river \nwater \n\nYou can collect river water \nCollected river water can be used to\nheal and decrease poison level by 1\n\nUsing river water ignores the \ncooldown in poison level changes");
				ImGui::NewLine();
				//controls instructions
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Q to collect and use river water"); 
				break;
			case VINE:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Vines");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Collect vines to tie around rocks \n\nMakes it easier to climb back up");
				break;
			case BUSHES:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Placing Bushes");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Collect bushes to place down \n\nPlace bushes up, down, left or right\nFace the spot where you want to \nplace a bush \n\nPlaced bushes CANNOT be retrieved");
				ImGui::NewLine();
				//controls instructions
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "WASD to change what spot you're \nfacing \nPress G to place a bush");
				break;
			case BOUNCY_BLOOM:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Bounce Flowers");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Use river water to make these \nflowers bloom \n\nThese flowers act as springs when \nbloomed");
				ImGui::NewLine();
				//controls instructions
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Q to use river water on flower");
				break;
			case ROCK:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Deploying Vines");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Collected vines can be tied around \nrocks \n\nTied vines extend to the ground \nVines can be climbed up and down \n\nDeployed vines CANNOT be retrieved");
				ImGui::NewLine();
				//controls instructions
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "F to tie vine around rock \nWS to climb up and down vines"); 
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
			cSettings->iWindowHeight * 0.2f));
		ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameRounding = 200.0f;
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

		//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));  // Set a background color
		//ImGuiWindowFlags blueOrbWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
		//	ImGuiWindowFlags_NoTitleBar |
		//	ImGuiWindowFlags_NoMove |
		//	ImGuiWindowFlags_NoResize |
		//	ImGuiWindowFlags_NoCollapse |
		//	ImGuiWindowFlags_NoScrollbar;
		//ImGui::Begin("BlueOrb", NULL, blueOrbWindowFlags);
		//ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.405f, cSettings->iWindowHeight * 0.9f));
		//ImGui::SetWindowSize(ImVec2(200.0f * relativeScale_x, 25.0f * relativeScale_y));
		//cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("BlueOrb");
		//ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
		//	ImVec2(cInventoryItemPlanet->vec2Size.x * relativeScale_x, cInventoryItemPlanet->vec2Size.y * relativeScale_y),
		//	ImVec2(0, 1), ImVec2(1, 0));
		//ImGui::SameLine();
		//ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		//ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d / %d",
		//	cInventoryItemPlanet->GetCount(), cInventoryItemPlanet->GetMaxCount());
		//ImGui::End();
		//ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));  // Set a background color
		ImGuiWindowFlags purpleKeyWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar;
		ImGui::Begin("PurpleKey", NULL, purpleKeyWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.405f, cSettings->iWindowHeight * 0.9f));
		ImGui::SetWindowSize(ImVec2(200.0f * relativeScale_x, 25.0f * relativeScale_y));
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("PurpleKey");
		ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
			ImVec2(cInventoryItemPlanet->vec2Size.x * relativeScale_x, cInventoryItemPlanet->vec2Size.y * relativeScale_y),
			ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d", cInventoryItemPlanet->GetCount());
		ImGui::End();
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));  // Set a background color
		ImGuiWindowFlags cyanKeyWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar;
		ImGui::Begin("CyanKey", NULL, cyanKeyWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.478f, cSettings->iWindowHeight * 0.9f));
		ImGui::SetWindowSize(ImVec2(200.0f * relativeScale_x, 25.0f * relativeScale_y));
		cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("CyanKey");
		ImGui::Image((void*)(intptr_t)cInventoryItemPlanet->GetTextureID(),
			ImVec2(cInventoryItemPlanet->vec2Size.x* relativeScale_x, cInventoryItemPlanet->vec2Size.y* relativeScale_y),
			ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d", cInventoryItemPlanet->GetCount());
		ImGui::End();
		ImGui::PopStyleColor();

		//tutorial popups
		if (tutorialPopupTerrestrial > 0) //there is a pop up
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
			switch (tutorialPopupTerrestrial)
			{
			case T_CHECKPOINT:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Checkpoints");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Activate checkpoint to set respawn\npoint \nTouching checkpoints restores health\nto full \n\nOnly 1 checkpoint can be active at 1\ntime \nActivating a new checkpoint \ndeactivates all others");
				ImGui::NewLine();
				//controls instructions
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Press R to lose 1 heart to respawn");
				break;
			case T_WALL_YELLOW:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Rare Earth Walls");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "This planet is rich with rare earths\n\nCollecting samples of these rare\nearths (coloured orbs) will allow\nyou to coat your suit with them,\nenabling you to pass through walls\nof the same rare earth colour.");
				ImGui::NewLine();
				break;
			case T_ORB_YELLOW:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Orbs");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Once a rare earth sample has been\ncollected, you can coat your suit\nwith that rare earth\n\nYour suit colour will change\naccordingly with the rare earth\ncolour");
				ImGui::NewLine();
				//controls instructions
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Press 1 to coat your suit yellow");
				break;
			case T_ROPE:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Rope");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Walking over a rope post will\nuncoil the rope and throw it down.\n\nAllows you to climb up and down more\neasily");
				ImGui::NewLine();
				//controls instructions
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Press W to climb up the rope\nPress S to climb down the rope");
				break;
			case T_TOXICITY:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Toxicity Level");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Coating your suit with rare earths\nfor too long will cause your\ntoxicity meter to go up\n\nIf it completely fills up, you will\nbegin to take damage");
				ImGui::NewLine();
				break;
			case T_ANTIDOTE:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Antidote Pills");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Collect Antidote Pills to reduce\nyour toxicity level and continue to\nstay alive");
				ImGui::NewLine();
				break;
			case T_ORB_RED:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Orbs");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Once a rare earth sample has been\ncollected, you can coat your suit\nwith that rare earth\n\nYour suit colour will change\naccordingly with the rare earth\ncolour");
				ImGui::NewLine();
				//controls instructions
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Press 1 to coat your suit yellow\nPress 2 to coat your suit red\nPress 3 to coat your suit green\nPress 5 to remove coating");
				break;
			case T_COLOUR_CHANGE_DANGER:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Changing Coatings Pt I");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Changing the rare earth coating of\nthe suit while you are in a rare\nearth wall will kill you");
				ImGui::NewLine();
				break;
			case T_RESOURCE:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Resources");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "You can only collect 5 resources to\nbring back to the ship\n\nYou can destroy resources you don't\nwant from your inventory");
				ImGui::NewLine();
				//controls instructions
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "I to toggle Inventory open and close");
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Left click a resource to destroy it");
				break;
			case T_SHOOTING:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Shooting");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Shoot projectiles up, down, left or\nright\n\nFace the direction you want to shoot\nin");
				ImGui::NewLine();
				//controls instructions
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "WASD to change which direction to \nface \nPress E to shoot");
				break;
			case T_LOCK:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Locks and Keys");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Certain enemies will drop keys when\nthey are killed\n\nWalk over a key to pick it up\n\nWalk near a coloured door to unlock\nit if you possess a key of the same\ncolour");
				ImGui::NewLine();
				break;
			case T_SWITCH:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Switches");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Toggle the tiles on and off by\nshooting linked switches\n\nThe switch(es) linked to the tiles\nwill have blue wires joining them");
				ImGui::NewLine();
				break;
			case T_SPIKES:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Spikes");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Standing in spikes will damage you");
				ImGui::NewLine();
				break;
			case T_COLOUR_CHANGE_COOLDOWN:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Changing Coatings Pt II");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "After changing your rare earth\ncoating, you cannot change to\nanother rare earth coating for a\nshort period of time");
				ImGui::NewLine();
				break;
			case T_ALARM:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Alarm");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Some enemies are able to trigger the\nalarm\n\nIf the alarm is activated, all\nenemies will become more dangerous\n\nKill the enemy activating the alarm\nto prevent other enemies from being\nalerted");
				ImGui::NewLine();
				break;
			default:
				break;
			}

			ImGui::End();
			ImGui::PopStyleColor();
		}
	}
	else if (planetNum == 3 && CGameStateManager::GetInstance()->hasPauseGameState() == false) {

	//tutorial popups
		if (tutorialPopupSnow > 0) //there is a pop up
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
			switch (tutorialPopupSnow)
			{
			case SIGNINTRO1:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Introduction");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Welcome to the Snow Planet's tutorial!");
				ImGui::NewLine();
				////controls instructions
				//ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				//ImGui::TextColored(ImVec4(1, 1, 0, 1), "Press R to lose 1 heart to respawn");
				break;
			case SIGNINTRO2:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Introduction");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "There are a few things you need to know\nbefore you start your adventure\non this planet.");
				ImGui::NewLine();
				////controls instructions
				//ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				//ImGui::TextColored(ImVec4(1, 1, 0, 1), "Press R to lose 1 heart to respawn");
				break;
			case SIGNINTRO3:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Introduction");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "First, you have a temperature bar\nat the top left of your screen\nthat you have to maintain\nin order to prevent yourself from freezing to death.\nThere are two ways to gain back your temperature,\neither by picking up fur coats around the map\nor killing brown wolves and collecting their fur.");
				ImGui::NewLine();
				////controls instructions
				//ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				//ImGui::TextColored(ImVec4(1, 1, 0, 1), "Press R to lose 1 heart to respawn");
				break;
			case SIGNINTRO4:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Introduction");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "There will be power-ups which you can collect\nthroughout the level which will\nhelp with your progression\nin the level itself.\n(Berserk, Shield, Freeze Water)");
				ImGui::NewLine();
				////controls instructions
				//ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				//ImGui::TextColored(ImVec4(1, 1, 0, 1), "Press R to lose 1 heart to respawn");
				break;
			case SIGNINTRO5:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Introduction");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "You will have a limit of 5\ninventory space items to\ncollect your resources\n(Common:Battery,Scrap Metal) & (Level Specific:Ice Crystal).\nNote that powerups do not count as\npart of the inventory space\nso feel free to collect as many\nas you want!");
				ImGui::NewLine();
				//controls instructions
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Pressing 'I' allows you to access your inventory space\nand left clicking on a resource removes 1\nof the resource from your bag space.");
				break;
			case SIGNTUT1:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Ropes");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Climb ropes to move up and down.");
				ImGui::NewLine();
				//controls instructions
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Pressing 'W' and 'S' allows you to move up and down.");
				break;
			case SIGNTUT2:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Checkpoints");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Activate checkpoint to set respawn\npoint \nTouching checkpoints restores health\nto full \n\nOnly 1 checkpoint can be active at 1\ntime \nActivating a new checkpoint \ndeactivates all others");
				ImGui::NewLine();
				//controls instructions
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Press R to lose 1 heart to respawn");
				break;
			case SIGNTUT3:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Ice Shard");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Water damages you but ice does not.\nIce allows you to walk faster!");
				ImGui::NewLine();
				//controls instructions
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Press F to freeze all water in map");
				break;
			case SIGNTUT4:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Fur & Fur Coat");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Both furcoats and fur allow you\nto increase your body temperature.\nKilling brown wolves will drop their fur\nwhile furcoats will be laid out across\nthe level.");
				ImGui::NewLine();
				//controls instructions
				//ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				//ImGui::TextColored(ImVec4(1, 1, 0, 1), "Press F to freeze all water in map");
				break;
			case SIGNTUT5:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Berserk Mode");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "It allows you to do twice the damage to enemies,\nheals you for 15 health\nand allows you to walk around with a shield\nif you activate your shield while in berserk mode.");
				ImGui::NewLine();
				//controls instructions
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Press G to activate berserk mode");
				break;
			case SIGNTUT6:
				//header
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Shield Mode");
				//information
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "There is no time limit for shield\nif you are in normal mode.\nBut if you walk, the shield will stop.\nIf you are in berserk mode, you can activate your shield\nand walk around with it\nas long as the berserk\nmode is active.");
				ImGui::NewLine();
				//controls instructions
				ImGui::SetWindowFontScale(1.2f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Press Q to to activate shield mode");
				break;
			default:
				break;
			}
			ImGui::End();
			ImGui::PopStyleColor();
		}
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
		cSettings->iWindowHeight * 0.2f));
	ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);
	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameRounding = 200.0f;
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
	ImGui::TextColored(ImVec4(0, 1, 1, 1), "%d",
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
	ImGui::TextColored(ImVec4(0, 1, 1, 1), "%d",
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
	ImGui::TextColored(ImVec4(0, 1, 1, 1), "%d",
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

int CGUI_Scene2D::getTutorialPopupTerrestrial(void)
{
	return tutorialPopupTerrestrial;
}

void CGUI_Scene2D::setTutorialPopupTerrestrial(int index)
{
	tutorialPopupTerrestrial = index;
}

int CGUI_Scene2D::getTutorialPopupSnow(void)
{
	return tutorialPopupSnow;
}

void CGUI_Scene2D::setTutorialPopupSnow(int index)
{
	tutorialPopupSnow = index;
}
