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
#include "../App/Source/GameStateManagement/GameInfo.h"
#include "ShipEnemy.h"
#include "Ship.h"

#include <iostream>
using namespace std;

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CGUI_SceneCombat::CGUI_SceneCombat(void)
	: cSettings(NULL)
	, window_flags(0)
{
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CGUI_SceneCombat::~CGUI_SceneCombat(void)
{

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

	cInventoryManager = CGameInfo::GetInstance()->ImportIM();

	CImageLoader* il = CImageLoader::GetInstance();
	AcceptButtonData.fileName = "Image\\GUI\\tick.png";
	AcceptButtonData.textureID = il->LoadTextureGetID(AcceptButtonData.fileName.c_str(), false);
	RejectButtonData.fileName = "Image\\GUI\\cross.png";
	RejectButtonData.textureID = il->LoadTextureGetID(RejectButtonData.fileName.c_str(), false);

	// Decoration
	PlayerBarTextureID = il->LoadTextureGetID("Image/GUI/PlayerBar.png", false);
	BitTextureID = il->LoadTextureGetID("Image/GUI/ProgressBit.png", false);
	ProgressBarTextureID = il->LoadTextureGetID("Image/GUI/ShipProgressBar.png", false);
	FocusBarBackground = il->LoadTextureGetID("Image/GUI/BqMain.png", false);
	boxBarBackground = il->LoadTextureGetID("Image/GUI/InfoBar.png", false);
	SquareInfoTextureID = il->LoadTextureGetID("Image/GUI/InfoBar2.png", false);

	ShipTextureID = il->LoadTextureGetID("Image/ShipCombat/shipBody.png", false);
	smallWeaponUpgrade1 = il->LoadTextureGetID("Image/ShipCombat/weaponSmall_1.png", false);
	smallWeaponUpgrade2 = il->LoadTextureGetID("Image/ShipCombat/weaponSmall_2.png", false);
	smallWeaponUpgrade3 = il->LoadTextureGetID("Image/ShipCombat/weaponSmall_3.png", false);

	largeWeaponUpgrade1 = il->LoadTextureGetID("Image/ShipCombat/weaponLarge_1.png", false);
	largeWeaponUpgrade2 = il->LoadTextureGetID("Image/ShipCombat/weaponLarge_2.png", false);
	largeWeaponUpgrade3 = il->LoadTextureGetID("Image/ShipCombat/weaponLarge_3.png", false);

	enemyShip1 = il->LoadTextureGetID("Image/ShipCombat/RookieShip.png", false);
	enemyShip2 = il->LoadTextureGetID("Image/ShipCombat/SargeantShip.png", false);
	enemyShip3 = il->LoadTextureGetID("Image/ShipCombat/CommanderShip.png", false);
	
	// Show the mouse pointer
	glfwSetInputMode(CSettings::GetInstance()->pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	isShowPanel = false;
	GuiState = GUI_STATE::noShow;
	isCombat = false;
	makeChanges = false;
	showRepairPanel = false;

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

	float buttonWidth = 20;
	float buttonHeight = 20;
	float buttonWidthSmall = 16;
	float buttonHeightSmall = 16;

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

	ImGuiWindowFlags overlayWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoScrollbar;

	ImGuiWindowFlags livesWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar;

	ImGuiWindowFlags livesWindowFlags_noFront = ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar;

	if (GuiState != GUI_STATE::showUpgrade) {
		ImGui::Begin("Health", NULL, overlayWindowFlags);
		ImGui::SetWindowPos(ImVec2(8 * relativeScale_y,
			cSettings->iWindowHeight * (0.01f)));
		ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_y, 10.0f * relativeScale_y));
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);

		ImGui::Image((void*)(intptr_t)PlayerBarTextureID, ImVec2(300 * relativeScale_y, 100 * relativeScale_y));
		ImGui::End();

		// how. do i. get rid. of the border.
		cInventoryItem1 = cInventoryManager->GetItem("Health");
		int healthCount = cInventoryItem1->GetCount();
		for (int i = 0; i < int(healthCount / int(cInventoryItem1->GetMaxCount() / 10)); i++) {
			ImGui::Begin("Health Bit", NULL, healthWindowFlags);
			ImGui::SetWindowPos(ImVec2(104.f * relativeScale_y, cSettings->iWindowHeight * (0.11f)));
			ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_y, 10.0f * relativeScale_y));

			ImGui::Image((void*)(intptr_t)BitTextureID, ImVec2(9 * relativeScale_x, 14 * relativeScale_y));
			ImGui::SameLine();
			ImGui::GetOverlayDrawList();
			ImGui::End();
		}

		ImGui::Begin("Damage", NULL, overlayWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth - 240 * relativeScale_y,
			cSettings->iWindowHeight * (0.01f)));
		ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_y, 10.0f * relativeScale_y));
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGui::Text("Damage");
		ImGui::Image((void*)(intptr_t)ProgressBarTextureID, ImVec2(205 * relativeScale_y, 25 * relativeScale_y));
		ImGui::End();

		if (cSettings->iWindowWidth < 1000) {
			// how. do i. get rid. of the border.
			cInventoryItem1 = cInventoryManager->GetItem("Damage");
			int damCount = cInventoryItem1->GetCount();
			for (int i = 0; i < int(damCount / floor(cInventoryItem1->GetMaxCount() / 15)); i++) {
				ImGui::Begin("Damage Bit", NULL, healthWindowFlags);
				ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth - 232 * relativeScale_y, cSettings->iWindowHeight * 0.05f));
				ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_x, 10.0f * relativeScale_y));

				ImGui::Image((void*)(intptr_t)BitTextureID, ImVec2(5 * relativeScale_x, 13 * relativeScale_y));
				ImGui::SameLine();
				ImGui::GetOverlayDrawList();
				ImGui::End();
			}
		}
		else {
			// how. do i. get rid. of the border.
			cInventoryItem1 = cInventoryManager->GetItem("Damage");
			int damCount = cInventoryItem1->GetCount();
			for (int i = 0; i < int(damCount / floor(cInventoryItem1->GetMaxCount() / 15)); i++) {
				ImGui::Begin("Damage Bit", NULL, healthWindowFlags);
				ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth - 232 * relativeScale_y, cSettings->iWindowHeight * 0.05f));
				ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_x, 10.0f * relativeScale_y));

				ImGui::Image((void*)(intptr_t)BitTextureID, ImVec2(5.6 * relativeScale_x, 12 * relativeScale_y));
				ImGui::SameLine();
				ImGui::GetOverlayDrawList();
				ImGui::End();
			}
		}

		ImGui::Begin("Oxygen by twice", NULL, overlayWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth - 240 * relativeScale_y,
			cSettings->iWindowHeight * (0.1f)));
		ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_y, 10.0f * relativeScale_y));
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGui::Text("Oxygen Level");
		ImGui::Image((void*)(intptr_t)ProgressBarTextureID, ImVec2(205 * relativeScale_y, 25 * relativeScale_y));
		ImGui::End();

		if (cSettings->iWindowWidth < 1000) {
			// how. do i. get rid. of the border.
			cInventoryItem1 = cInventoryManager->GetItem("Ventilation");
			int oxyCount = cInventoryItem1->GetCount();
			for (int i = 0; i < int(oxyCount / floor(cInventoryItem1->GetMaxCount() / 15)); i++) {
				ImGui::Begin("Oxygen Bit", NULL, healthWindowFlags);
				ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth - 232 * relativeScale_y, cSettings->iWindowHeight * 0.14f));
				ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_x, 10.0f * relativeScale_y));

				ImGui::Image((void*)(intptr_t)BitTextureID, ImVec2(5 * relativeScale_x, 13 * relativeScale_y));
				ImGui::SameLine();
				ImGui::GetOverlayDrawList();
				ImGui::End();
			}
		}
		else {
			// how. do i. get rid. of the border.
			cInventoryItem1 = cInventoryManager->GetItem("Ventilation");
			int damCount = cInventoryItem1->GetCount();
			for (int i = 0; i < int(damCount / floor(cInventoryItem1->GetMaxCount() / 15)); i++) {
				ImGui::Begin("Oxygen Bit", NULL, healthWindowFlags);
				ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth - 232 * relativeScale_y, cSettings->iWindowHeight * 0.14f));
				ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_x, 10.0f * relativeScale_y));

				ImGui::Image((void*)(intptr_t)BitTextureID, ImVec2(5.6 * relativeScale_x, 12 * relativeScale_y));
				ImGui::SameLine();
				ImGui::GetOverlayDrawList();
				ImGui::End();
			}
		}
	}

	if (!isCombat && GuiState == GUI_STATE::showUpgrade) {
		ImGui::Begin("FocusBar", NULL, healthWindowFlags);
		ImGui::SetWindowPos(ImVec2(0, cSettings->iWindowHeight * (0.01f)));
		ImGui::SetWindowSize(ImVec2(cSettings->iWindowWidth, 10.0f));
		ImGui::Image((void*)(intptr_t)FocusBarBackground, ImVec2(cSettings->iWindowWidth, 75));
		ImGui::GetOverlayDrawList();
		ImGui::End();

		ImGui::Begin("Lives", NULL, healthWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.35f, cSettings->iWindowHeight * 0.05f));
		ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));

		// planet information
		ImGui::SetWindowFontScale(2.5f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 1, 1), "Ship Upgrade Mode");
		ImGui::End();
	}

	if (isCombat &&	GuiState == GUI_STATE::showWeapons) {
		// fire weapons and stuff lol

		ImGui::Begin("Large weapon", NULL, livesWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth* (0.05f),
			cSettings->iWindowHeight - cSettings->iWindowHeight * (0.25f)));
		ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_x, 10.0f * relativeScale_y));
		ImGui::SetWindowFontScale(1.3f * relativeScale_y);
		ImGui::SetWindowFocus();

		ImGui::Text("Large weapon Fire");

		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.01f, 0.93f, 1.f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
		ImGui::ProgressBar((CShipPlayer::GetInstance()->TimeElapsed - CShipPlayer::GetInstance()->LargeWeaponTime) /
			(float)CShipPlayer::GetInstance()->LargeWeaponInterval, ImVec2(100.0f *
				relativeScale_x, 20.0f * relativeScale_y));
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::NewLine();
		ImGui::SetWindowFontScale(1.6f * relativeScale_y);
		ImGui::Text("FIRE WEAPON");
		if (ImGui::IsItemClicked() && (CShipPlayer::GetInstance()->TimeElapsed - CShipPlayer::GetInstance()->LargeWeaponTime) >= (float)CShipPlayer::GetInstance()->LargeWeaponInterval) {
			CShipEnemy::GetInstance()->setHealth(CShipEnemy::GetInstance()->getHealth() - 10);
			CShipPlayer::GetInstance()->LargeWeaponTime = CShipPlayer::GetInstance()->TimeElapsed;
		}
		ImGui::End();

		ImGui::Begin("Small weapon 1", NULL, livesWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth* (0.25f),
			cSettings->iWindowHeight - cSettings->iWindowHeight * (0.25f)));
		ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_x, 10.0f * relativeScale_y));
		ImGui::SetWindowFontScale(1.3f * relativeScale_y);
		ImGui::SetWindowFocus();

		ImGui::Text("Small weapon 1 Fire");

		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.01f, 0.93f, 1.f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
		ImGui::ProgressBar((CShipPlayer::GetInstance()->TimeElapsed - CShipPlayer::GetInstance()->Weapon1Time) /
			(float)CShipPlayer::GetInstance()->weapon1Interval, ImVec2(100.0f *
				relativeScale_x, 20.0f * relativeScale_y));
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::NewLine();
		ImGui::SetWindowFontScale(1.6f * relativeScale_y);
		ImGui::Text("FIRE WEAPON");
		if (ImGui::IsItemClicked() && (CShipPlayer::GetInstance()->TimeElapsed - CShipPlayer::GetInstance()->Weapon1Time) >= CShipPlayer::GetInstance()->weapon1Interval) {
			CShipEnemy::GetInstance()->setHealth(CShipEnemy::GetInstance()->getHealth() - 5);
			CShipPlayer::GetInstance()->Weapon1Time = CShipPlayer::GetInstance()->TimeElapsed;

		}
		ImGui::End();

		ImGui::Begin("Small weapon", NULL, livesWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth* (0.45f),
			cSettings->iWindowHeight - cSettings->iWindowHeight * (0.25f)));
		ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_x, 10.0f * relativeScale_y));
		ImGui::SetWindowFontScale(1.3f * relativeScale_y);
		ImGui::SetWindowFocus();

		ImGui::Text("Small weapon 2 Fire");

		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.01f, 0.93f, 1.f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
		ImGui::ProgressBar((CShipPlayer::GetInstance()->TimeElapsed - CShipPlayer::GetInstance()->Weapon2Time) /
			(float)CShipPlayer::GetInstance()->weapon2Interval, ImVec2(100.0f * relativeScale_x, 20.0f * relativeScale_y));
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::NewLine();
		ImGui::SetWindowFontScale(1.6f * relativeScale_y);
		ImGui::Text("FIRE WEAPON");
		if (ImGui::IsItemClicked() && (CShipPlayer::GetInstance()->TimeElapsed - CShipPlayer::GetInstance()->Weapon2Time) >= CShipPlayer::GetInstance()->weapon2Interval) {
			CShipEnemy::GetInstance()->setHealth(CShipEnemy::GetInstance()->getHealth() - 5);
			CShipPlayer::GetInstance()->Weapon2Time = CShipPlayer::GetInstance()->TimeElapsed;

		}
		ImGui::End();

	}

	if (GuiState != GUI_STATE::showWeapons && isCombat) {
		// show the ventilation panel

		ImGui::Begin("Ventilation", NULL, livesWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * (0.05f),
			cSettings->iWindowHeight - cSettings->iWindowHeight * (0.25f)));
		ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_x, 10.0f * relativeScale_y));
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGui::SetWindowFocus();

		ImGui::Text("Time until Ventilation Shutoff");

		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.01f, 0.93f, 1.f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
		ImGui::ProgressBar((CShip::GetInstance()->ventilationInterval - (CShip::GetInstance()->TimeElapsed - CShip::GetInstance()->ventilationTiming)) /
			(float)CShip::GetInstance()->ventilationInterval, ImVec2(160.0f *
				relativeScale_x, 20.0f * relativeScale_y));
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::NewLine();
		ImGui::SetWindowFontScale(1.8f * relativeScale_y);
		ImGui::Text("Restart Ventilation");
		if (ImGui::IsItemClicked()) {
			CShip::GetInstance()->ResetVentilationTimer();
		}
		ImGui::End();
	}

	if (showRepairPanel) {
		ImGui::Begin("Lives", NULL, livesWindowFlags);
		ImGui::SetWindowPos(ImVec2(blockPosition.x + cSettings->iWindowWidth * 0.05f,
			blockPosition.y - cSettings->iWindowHeight * 0.1f));
		ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));
		ImGui::SetWindowFocus();

		// planet information
		ImGui::SetWindowFontScale(1.8f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 1, 1), "Repair Ship Tile?");
		ImGui::SetWindowFontScale(1.6f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Scrap x 1");

		ImGui::NewLine();
		ImGui::Text("You have:");
		cInventoryItem1 = cInventoryManager->GetItem("ScrapMetal");
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Scrap: %d", cInventoryItem1->GetCount());
		ImGui::NewLine();
		// Add codes for Start button here
		if (cInventoryItem1->GetCount() >= 1)
		{
			ImGui::Image((ImTextureID)AcceptButtonData.textureID, ImVec2(buttonWidth * relativeScale_y, buttonHeight * relativeScale_y), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
			if (ImGui::IsItemClicked()) {
				makeChanges = true;
				cInventoryItem1->Remove(1);
			}
		}
		ImGui::SameLine();

		ImGui::Image((ImTextureID)RejectButtonData.textureID, ImVec2(buttonWidth* relativeScale_y, buttonHeight * relativeScale_y), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
		if (ImGui::IsItemClicked())
		{
			showRepairPanel = false;
			makeChanges = false;
		}

		ImGui::End();
	}


	int ShowItemCat = 1;
	int counter = 0;

	// Render panels for click click
	switch (GuiState)
	{	
	case CGUI_SceneCombat::showWeapons:
		glm::vec2 pos1 = Camera2D::GetInstance()->getBlockPositionWindow(glm::vec2(4, 18));
		blockPosition = ImVec2(pos1.x, pos1.y);

		ImGui::Begin("Storage", NULL, healthWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.01f,
			cSettings->iWindowHeight * 0.2f));
		ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));
		ImGui::SetWindowFocus();

		ImGui::Image((ImTextureID)RejectButtonData.textureID, ImVec2(buttonWidth * relativeScale_x, buttonWidth * relativeScale_x), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
		if (ImGui::IsItemClicked())
		{
			GuiState = CGUI_SceneCombat::noShow;
			Camera2D::GetInstance()->setTargetZoom(2.0f);
		}

		ImGui::GetOverlayDrawList();
		ImGui::End();

		ImGui::Begin("Weapon by Itzy", NULL, overlayWindowFlags);
		ImGui::SetWindowPos(ImVec2(pos1.x + cSettings->iWindowWidth * 0.05f,
			pos1.y - cSettings->iWindowHeight * 0.1f));
		ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));

		ImGui::Image((void*)(intptr_t)ShipTextureID, ImVec2(700 * relativeScale_x, 375 * relativeScale_y));

		ImGui::GetOverlayDrawList();

		// draw indiv weapons
		pos1 = Camera2D::GetInstance()->getBlockPositionWindow(glm::vec2(14, 13));
		cInventoryItem1 = CInventoryManager::GetInstance()->GetItem("Upgrade_large");
		ImGui::Begin("Weapon Large by Itzy", NULL, healthWindowFlags);
		ImGui::SetWindowPos(ImVec2(pos1.x + cSettings->iWindowWidth * 0.05f,
			pos1.y - cSettings->iWindowHeight * 0.1f));
		ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));

		switch (cInventoryItem1->GetCount())
		{
		case 1:
			ImGui::Image((void*)(intptr_t)largeWeaponUpgrade1, ImVec2(125 * relativeScale_x, 125 * relativeScale_y));
			break;
		case 2:
			ImGui::Image((void*)(intptr_t)largeWeaponUpgrade2, ImVec2(125 * relativeScale_x, 125 * relativeScale_y));
			break;
		case 3:
			ImGui::Image((void*)(intptr_t)largeWeaponUpgrade3, ImVec2(125 * relativeScale_x, 125 * relativeScale_y));
			break;
		default:
			break;
		}

		ImGui::GetOverlayDrawList();
		ImGui::End();

		pos1 = Camera2D::GetInstance()->getBlockPositionWindow(glm::vec2(20, 12));
		blockPosition = ImVec2(pos1.x, pos1.y);


		cInventoryItem1 = CInventoryManager::GetInstance()->GetItem("Upgrade_small1");
		ImGui::Begin("Weapon Small1 by Itzy", NULL, healthWindowFlags);
		ImGui::SetWindowPos(ImVec2(pos1.x + cSettings->iWindowWidth * 0.05f,
			pos1.y - cSettings->iWindowHeight * 0.1f));
		ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));

		switch (cInventoryItem1->GetCount())
		{
		case 1:
			ImGui::Image((void*)(intptr_t)smallWeaponUpgrade1, ImVec2(100 * relativeScale_x, 100 * relativeScale_y));
			break;
		case 2:
			ImGui::Image((void*)(intptr_t)smallWeaponUpgrade2, ImVec2(100 * relativeScale_x, 100 * relativeScale_y));
			break;
		case 3:
			ImGui::Image((void*)(intptr_t)smallWeaponUpgrade3, ImVec2(100 * relativeScale_x, 100 * relativeScale_y));
			break;
		default:
			break;
		}


		ImGui::GetOverlayDrawList();
		ImGui::End();

		pos1 = Camera2D::GetInstance()->getBlockPositionWindow(glm::vec2(23.5, 12));
		blockPosition = ImVec2(pos1.x, pos1.y);

		cInventoryItem1 = CInventoryManager::GetInstance()->GetItem("Upgrade_small2");
		ImGui::Begin("Weapon Small2 by Itzy", NULL, healthWindowFlags);
		ImGui::SetWindowPos(ImVec2(pos1.x + cSettings->iWindowWidth * 0.05f,
			pos1.y - cSettings->iWindowHeight * 0.1f));
		ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));

		switch (cInventoryItem1->GetCount())
		{
		case 1:
			ImGui::Image((void*)(intptr_t)smallWeaponUpgrade1, ImVec2(100 * relativeScale_x, 100 * relativeScale_y));
			break;
		case 2:
			ImGui::Image((void*)(intptr_t)smallWeaponUpgrade2, ImVec2(100 * relativeScale_x, 100 * relativeScale_y));
			break;
		case 3:
			ImGui::Image((void*)(intptr_t)smallWeaponUpgrade3, ImVec2(100 * relativeScale_x, 100 * relativeScale_y));
			break;
		default:
			break;
		}

		ImGui::GetOverlayDrawList();
		ImGui::End();
		ImGui::End();
		break;
	case CGUI_SceneCombat::showWeaponUpgrade:
		glm::vec2 pos = Camera2D::GetInstance()->getBlockPositionWindow(glm::vec2(4, 18));
		blockPosition = ImVec2(pos.x, pos.y);

		ImGui::Begin("Storage", NULL, healthWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.01f,
			cSettings->iWindowHeight * 0.2f));
		ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));
		ImGui::SetWindowFocus();

		ImGui::Image((ImTextureID)RejectButtonData.textureID, ImVec2(buttonWidth * relativeScale_x, buttonWidth * relativeScale_x), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
		if (ImGui::IsItemClicked())
		{
			GuiState = CGUI_SceneCombat::GUI_STATE::showUpgrade;

			Camera2D::GetInstance()->setTargetZoom(2.0f);
		}

		ImGui::GetOverlayDrawList();
		ImGui::End();

		ImGui::Begin("Weapon by Itzy", NULL, overlayWindowFlags);
		ImGui::SetWindowPos(ImVec2(pos.x + cSettings->iWindowWidth * 0.05f,
			pos.y - cSettings->iWindowHeight * 0.1f));
		ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));

		ImGui::Image((void*)(intptr_t)ShipTextureID, ImVec2(700 * relativeScale_x, 375 * relativeScale_y));

		ImGui::GetOverlayDrawList();

		// draw indiv weapons
		pos = Camera2D::GetInstance()->getBlockPositionWindow(glm::vec2(14, 13));
		cInventoryItem1 = CInventoryManager::GetInstance()->GetItem("Upgrade_large");
		ImGui::Begin("Weapon Large by Itzy", NULL, healthWindowFlags);
		ImGui::SetWindowPos(ImVec2(pos.x + cSettings->iWindowWidth * 0.05f,
			pos.y - cSettings->iWindowHeight * 0.1f));
		ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));

		switch (cInventoryItem1->GetCount())
		{
		case 1:
			ImGui::Image((void*)(intptr_t)largeWeaponUpgrade1, ImVec2(125 * relativeScale_x, 125 * relativeScale_y));
			break;
		case 2:
			ImGui::Image((void*)(intptr_t)largeWeaponUpgrade2, ImVec2(125 * relativeScale_x, 125 * relativeScale_y));
			break;
		case 3:
			ImGui::Image((void*)(intptr_t)largeWeaponUpgrade3, ImVec2(125 * relativeScale_x, 125 * relativeScale_y));
			break;
		default:
			break;
		}
		
		if (ImGui::IsItemClicked() && !isCombat) {
			UpgradeState = UPGRADE_STATE::LARGEWEAPON_UPGRADE;
		}

		ImGui::GetOverlayDrawList();
		ImGui::End();

		pos = Camera2D::GetInstance()->getBlockPositionWindow(glm::vec2(20, 12));
		blockPosition = ImVec2(pos.x, pos.y);


		cInventoryItem1 = CInventoryManager::GetInstance()->GetItem("Upgrade_small1");
		ImGui::Begin("Weapon Small1 by Itzy", NULL, healthWindowFlags);
		ImGui::SetWindowPos(ImVec2(pos.x + cSettings->iWindowWidth * 0.05f,
			pos.y - cSettings->iWindowHeight * 0.1f));
		ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));

		switch (cInventoryItem1->GetCount())
		{
		case 1:
			ImGui::Image((void*)(intptr_t)smallWeaponUpgrade1, ImVec2(100 * relativeScale_x, 100 * relativeScale_y));
			break;
		case 2:
			ImGui::Image((void*)(intptr_t)smallWeaponUpgrade2, ImVec2(100 * relativeScale_x, 100 * relativeScale_y));
			break;
		case 3:
			ImGui::Image((void*)(intptr_t)smallWeaponUpgrade3, ImVec2(100 * relativeScale_x, 100 * relativeScale_y));
			break;
		default:
			break;
		}

		if (ImGui::IsItemClicked() && !isCombat) {
			UpgradeState = UPGRADE_STATE::SMALLWEAPON1_UPGRADE;
		}

		ImGui::GetOverlayDrawList();
		ImGui::End();

		pos = Camera2D::GetInstance()->getBlockPositionWindow(glm::vec2(23.5, 12));
		blockPosition = ImVec2(pos.x, pos.y);

		cInventoryItem1 = CInventoryManager::GetInstance()->GetItem("Upgrade_small2");
		ImGui::Begin("Weapon Small2 by Itzy", NULL, healthWindowFlags);
		ImGui::SetWindowPos(ImVec2(pos.x + cSettings->iWindowWidth * 0.05f,
			pos.y - cSettings->iWindowHeight * 0.1f));
		ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));

		switch (cInventoryItem1->GetCount())
		{
		case 1:
			ImGui::Image((void*)(intptr_t)smallWeaponUpgrade1, ImVec2(100 * relativeScale_x, 100 * relativeScale_y));
			break;
		case 2:
			ImGui::Image((void*)(intptr_t)smallWeaponUpgrade2, ImVec2(100 * relativeScale_x, 100 * relativeScale_y));
			break;
		case 3:
			ImGui::Image((void*)(intptr_t)smallWeaponUpgrade3, ImVec2(100 * relativeScale_x, 100 * relativeScale_y));
			break;
		default:
			break;
		}

		if (ImGui::IsItemClicked() && !isCombat) {
			UpgradeState = UPGRADE_STATE::SMALLWEAPON2_UPGRADE;
		}

		ImGui::GetOverlayDrawList();
		ImGui::End();
		ImGui::End();



		switch (UpgradeState)
		{
		case CGUI_SceneCombat::UPGRADE_STATE::SMALLWEAPON1_UPGRADE:
			cInventoryItem1 = cInventoryManager->GetItem("Upgrade_small1");
			if (cInventoryItem1->GetCount() < cInventoryItem1->GetMaxCount()) {
				ImGui::Begin("Weapon", NULL, livesWindowFlags);
				ImGui::SetWindowPos(ImVec2(ImVec2(cSettings->iWindowWidth * 0.01f,
					cSettings->iWindowHeight * 0.6f)));
				ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));

				ImGui::SetWindowFontScale(1.8f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Upgrade Weapon?");
				ImGui::SetWindowFontScale(1.6f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Batteries x 2");
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Scrap x 1");

				ImGui::NewLine();
				ImGui::Text("You have:");
				cInventoryItem1 = cInventoryManager->GetItem("Battery");
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Battery: %d", cInventoryItem1->GetCount());
				cInventoryItem2 = cInventoryManager->GetItem("ScrapMetal");
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Scrap: %d", cInventoryItem2->GetCount());
				ImGui::NewLine();
				// Add codes for Start button here
				if (cInventoryItem1->GetCount() >= 2 && cInventoryItem2->GetCount() >= 1)
				{
					ImGui::Image((ImTextureID)AcceptButtonData.textureID, ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
					if (ImGui::IsItemClicked()) {
						makeChanges = true;
						cInventoryItem1->Remove(2);
						cInventoryItem2->Remove(1);
						cInventoryManager->GetItem("Upgrade_small2")->Add(1);
						CShipPlayer::GetInstance()->weapon2Interval = 7.0f - cInventoryManager->GetItem("Upgrade_small2")->GetCount() * 2;
						UpgradeState = UPGRADE_STATE::NOSTATE;
					}
				}
				ImGui::SameLine();

				ImGui::Image((ImTextureID)RejectButtonData.textureID, ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
				if (ImGui::IsItemClicked())
				{
					UpgradeState = UPGRADE_STATE::NOSTATE;
					makeChanges = false;
				}

				ImGui::GetOverlayDrawList();
				ImGui::End();
			}
			else {
				ImGui::Begin("Storage", NULL, livesWindowFlags);
				ImGui::SetWindowPos(ImVec2(ImVec2(cSettings->iWindowWidth * 0.01f,
					cSettings->iWindowHeight * 0.6f)));
				ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));

				ImGui::SetWindowFontScale(1.8f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Maximum Upgrades Reached.");

				ImGui::SameLine();
				ImGui::Image((ImTextureID)RejectButtonData.textureID, ImVec2(buttonWidthSmall, buttonHeightSmall), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
				if (ImGui::IsItemClicked())
				{
					UpgradeState = UPGRADE_STATE::NOSTATE;
					makeChanges = false;
				}

				ImGui::GetOverlayDrawList();
				ImGui::End();
			}
			break;
		case CGUI_SceneCombat::UPGRADE_STATE::SMALLWEAPON2_UPGRADE:
			cInventoryItem1 = cInventoryManager->GetItem("Upgrade_small2");
			if (cInventoryItem1->GetCount() < cInventoryItem1->GetMaxCount()) {
				ImGui::Begin("Weapon", NULL, livesWindowFlags);
				ImGui::SetWindowPos(ImVec2(ImVec2(cSettings->iWindowWidth * 0.01f,
					cSettings->iWindowHeight * 0.6f)));
				ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));

				ImGui::SetWindowFontScale(1.8f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Upgrade Weapon?");
				ImGui::SetWindowFontScale(1.6f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Batteries x 2");
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Scrap x 1");

				ImGui::NewLine();
				ImGui::Text("You have:");
				cInventoryItem1 = cInventoryManager->GetItem("Battery");
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Battery: %d", cInventoryItem1->GetCount());
				cInventoryItem2 = cInventoryManager->GetItem("ScrapMetal");
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Scrap: %d", cInventoryItem2->GetCount());
				ImGui::NewLine();
				// Add codes for Start button here
				if (cInventoryItem1->GetCount() >= 2 && cInventoryItem2->GetCount() >= 1)
				{
					ImGui::Image((ImTextureID)AcceptButtonData.textureID, ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
					if (ImGui::IsItemClicked()) {
						makeChanges = true;
						cInventoryItem1->Remove(2);
						cInventoryItem2->Remove(1);
						cInventoryManager->GetItem("Upgrade_small2")->Add(1);
						CShipPlayer::GetInstance()->weapon2Interval = 7.0f - cInventoryManager->GetItem("Upgrade_small2")->GetCount() * 2;
						UpgradeState = UPGRADE_STATE::NOSTATE;
					}
				}
				ImGui::SameLine();

				ImGui::Image((ImTextureID)RejectButtonData.textureID, ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
				if (ImGui::IsItemClicked())
				{
					UpgradeState = UPGRADE_STATE::NOSTATE;
					makeChanges = false;
				}

				ImGui::GetOverlayDrawList();
				ImGui::End();
			}
			else {
				ImGui::Begin("Storage", NULL, livesWindowFlags);
				ImGui::SetWindowPos(ImVec2(ImVec2(cSettings->iWindowWidth * 0.01f,
					cSettings->iWindowHeight * 0.6f)));
				ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));

				ImGui::SetWindowFontScale(1.8f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Maximum Upgrades Reached.");

				ImGui::SameLine();
				ImGui::Image((ImTextureID)RejectButtonData.textureID, ImVec2(buttonWidthSmall, buttonHeightSmall), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
				if (ImGui::IsItemClicked())
				{
					UpgradeState = UPGRADE_STATE::NOSTATE;
					makeChanges = false;
				}

				ImGui::GetOverlayDrawList();
				ImGui::End();
			}
			break;
		case CGUI_SceneCombat::UPGRADE_STATE::LARGEWEAPON_UPGRADE:
			cInventoryItem1 = cInventoryManager->GetItem("Upgrade_large");
			if (cInventoryItem1->GetCount() < cInventoryItem1->GetMaxCount()) {
				ImGui::Begin("Weapon", NULL, livesWindowFlags);
				ImGui::SetWindowPos(ImVec2(ImVec2(cSettings->iWindowWidth * 0.01f,
					cSettings->iWindowHeight * 0.6f)));
				ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));

				ImGui::SetWindowFontScale(1.8f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Upgrade Weapon?");
				ImGui::SetWindowFontScale(1.6f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Quartz x 3");
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Batteries x 2");

				ImGui::NewLine();
				ImGui::Text("You have:");
				cInventoryItem1 = cInventoryManager->GetItem("EnergyQuartz");
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Quartz: %d", cInventoryItem1->GetCount());
				cInventoryItem2 = cInventoryManager->GetItem("Battery");
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Battery: %d", cInventoryItem2->GetCount());

				ImGui::NewLine();
				// Add codes for Start button here
				if (cInventoryItem1->GetCount() >= 3 && cInventoryItem2->GetCount() >= 2)
				{
					ImGui::Image((ImTextureID)AcceptButtonData.textureID, ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
					if (ImGui::IsItemClicked()) {
						makeChanges = true;
						cInventoryItem1->Remove(3);
						cInventoryItem2->Remove(2);
						cInventoryManager->GetItem("Upgrade_large")->Add(1);
						CShipPlayer::GetInstance()->LargeWeaponInterval = 12.0f - cInventoryManager->GetItem("Upgrade_large")->GetCount() * 2;
						UpgradeState = UPGRADE_STATE::NOSTATE;
					}
				}
				ImGui::SameLine();

				ImGui::Image((ImTextureID)RejectButtonData.textureID, ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
				if (ImGui::IsItemClicked())
				{
					UpgradeState = UPGRADE_STATE::NOSTATE;
					makeChanges = false;
				}

				ImGui::GetOverlayDrawList();
				ImGui::End();
			}
			else {
				ImGui::Begin("Storage", NULL, livesWindowFlags);
				ImGui::SetWindowPos(ImVec2(ImVec2(cSettings->iWindowWidth * 0.01f,
					cSettings->iWindowHeight * 0.6f)));
				ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));

				ImGui::SetWindowFontScale(1.8f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Maximum Upgrades Reached.");

				ImGui::SameLine();
				ImGui::Image((ImTextureID)RejectButtonData.textureID, ImVec2(buttonWidthSmall, buttonHeightSmall), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
				if (ImGui::IsItemClicked())
				{
					UpgradeState = UPGRADE_STATE::NOSTATE;
					makeChanges = false;
				}

				ImGui::GetOverlayDrawList();
				ImGui::End();
			}
			break;
		default:
			break;
		}
		break;

	case CGUI_SceneCombat::showStorage:
		ImGui::Begin("Storage", NULL, livesWindowFlags);
		ImGui::SetWindowPos(ImVec2(blockPosition.x - cSettings->iWindowWidth * 0.10f,
			blockPosition.y - cSettings->iWindowHeight * (cInventoryManager->GetItem("Storage")->GetMaxCount() / 5) * 0.05f - 0.2f));
		ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));
		ImGui::SetWindowFocus();

		ImGui::SetWindowFontScale(1.8f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 1, 1), "Storage");

		if (cInventoryManager->GetItem("Storage")->GetCount() == 0) {
			ImGui::SetWindowFontScale(1.5f * relativeScale_y);
			ImGui::Text("Inventory is Empty.");
		}

		counter = 0;
		ShowItemCat = 1;

		for (int i = 0; i < int(cInventoryManager->GetItem("Storage")->GetMaxCount() / 5); i++) {
			for (int j = 0; j < 5; j++) {
				if (ShowItemCat == 1) {
					cInventoryItem1 = cInventoryManager->GetItem("ScrapMetal");
					if (cInventoryItem1->GetCount() > counter) {
						ImGui::Image((ImTextureID)cInventoryItem1->GetTextureID(), ImVec2(buttonWidth, buttonHeight), ImVec2(1.0, 1.0), ImVec2(0.0, 0.0));
						counter++;
					}
					else {
						ShowItemCat++;
						counter = 0;
					}
				}
				if (ShowItemCat == 2) {
					cInventoryItem1 = cInventoryManager->GetItem("Battery");

					if (cInventoryItem1->GetCount() > counter) {
						ImGui::Image((ImTextureID)cInventoryItem1->GetTextureID(), ImVec2(buttonWidth, buttonHeight), ImVec2(1.0, 1.0), ImVec2(0.0, 0.0));
						counter++;
					}
					else {
						ShowItemCat++;
						counter = 0;
					}
				}
				if (ShowItemCat == 3) {
					cInventoryItem1 = cInventoryManager->GetItem("Ironwood");

					if (cInventoryItem1->GetCount() > counter) {
						ImGui::Image((ImTextureID)cInventoryItem1->GetTextureID(), ImVec2(buttonWidth, buttonHeight), ImVec2(1.0, 1.0), ImVec2(0.0, 0.0));
						counter++;
					}
					else {
						ShowItemCat++;
						counter = 0;
					}
				}
				if (ShowItemCat == 4) {
					cInventoryItem1 = cInventoryManager->GetItem("EnergyQuartz");

					if (cInventoryItem1->GetCount() > counter) {
						ImGui::Image((ImTextureID)cInventoryItem1->GetTextureID(), ImVec2(buttonWidth, buttonHeight), ImVec2(1.0, 1.0), ImVec2(0.0, 0.0));
						counter++;
					}
					else {
						ShowItemCat++;
						counter = 0;
					}
				}
				if (ShowItemCat == 5) {
					cInventoryItem1 = cInventoryManager->GetItem("IceCrystal");

					if (cInventoryItem1->GetCount() > counter) {
						ImGui::Image((ImTextureID)cInventoryItem1->GetTextureID(), ImVec2(buttonWidth, buttonHeight), ImVec2(1.0, 1.0), ImVec2(0.0, 0.0));
						counter++;
					}
					else {
						ShowItemCat++;
						counter = 0;
					}
				}

				ImGui::SameLine();
				ImGui::GetOverlayDrawList();
			}

			ImGui::NewLine();
		}

		// TODO: add option to trash and bin it !!
		// TODO: add multipage scroll

		ImGui::NewLine();
		ImGui::Image((ImTextureID)RejectButtonData.textureID, ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
		if (ImGui::IsItemClicked())
		{
			GuiState = CGUI_SceneCombat::GUI_STATE::noShow;
			makeChanges = false;
		}

		ImGui::End();

		break;
	case CGUI_SceneCombat::showUpgrade:
		switch (UpgradeState)
		{
		case CGUI_SceneCombat::STORAGE_UPGRADE:
			// planet information
			cInventoryItem1 = cInventoryManager->GetItem("Upgrade_storage");
			if (cInventoryItem1->GetCount() < cInventoryItem1->GetMaxCount()) {
				ImGui::Begin("Storage", NULL, livesWindowFlags);
				ImGui::SetWindowPos(ImVec2(blockPosition.x - cSettings->iWindowWidth * 0.20f,
					blockPosition.y - cSettings->iWindowHeight * 0.2f));
				ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));

				ImGui::SetWindowFontScale(1.8f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Add additional Storage?");
				ImGui::SetWindowFontScale(1.6f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Ironwood x 2");
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Scrap x 2");

				ImGui::NewLine();
				ImGui::Text("You have:");
				cInventoryItem1 = cInventoryManager->GetItem("Ironwood");
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Ironwood: %d", cInventoryItem1->GetCount());
				cInventoryItem2 = cInventoryManager->GetItem("ScrapMetal");
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Scrap: %d", cInventoryItem2->GetCount());
				ImGui::NewLine();
				// Add codes for Start button here
				if (cInventoryItem1->GetCount() >= 0 && cInventoryItem2->GetCount() >= 0)
				{
					ImGui::Image((ImTextureID)AcceptButtonData.textureID, ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
					if (ImGui::IsItemClicked()) {
						makeChanges = true;
						/*cInventoryItem1->Remove(2);
						cInventoryItem2->Remove(2);*/
						cInventoryManager->GetItem("Upgrade_storage")->Add(1);
						cInventoryManager->GetItem("Storage")->SetMaxCount(cInventoryManager->GetItem("Upgrade_storage")->GetCount() * 15);
					}
				}
				ImGui::SameLine();

				ImGui::Image((ImTextureID)RejectButtonData.textureID, ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
				if (ImGui::IsItemClicked())
				{
					UpgradeState = UPGRADE_STATE::NOSTATE;
					makeChanges = false;
				}

				ImGui::End();
			}
			else {
				ImGui::Begin("Storage", NULL, livesWindowFlags);
				ImGui::SetWindowPos(ImVec2(blockPosition.x - cSettings->iWindowWidth * 0.30f,
					blockPosition.y - cSettings->iWindowHeight * 0.1f));
				ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));
				ImGui::SetWindowFocus();

				ImGui::SetWindowFontScale(1.8f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Maximum Upgrades Reached.");

				ImGui::SameLine();
				ImGui::Image((ImTextureID)RejectButtonData.textureID, ImVec2(buttonWidthSmall, buttonHeightSmall), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
				if (ImGui::IsItemClicked())
				{
					UpgradeState = UPGRADE_STATE::NOSTATE;
					makeChanges = false;
				}

				ImGui::End();
			}			
			break;
		case CGUI_SceneCombat::VENTILATION_UPGRADE:
			cInventoryItem1 = cInventoryManager->GetItem("Upgrade_ventilation");
			if (cInventoryItem1->GetCount() < cInventoryItem1->GetMaxCount()) {
				ImGui::Begin("Storage", NULL, livesWindowFlags);
				ImGui::SetWindowPos(ImVec2(blockPosition.x - cSettings->iWindowWidth * 0.20f,
					blockPosition.y - cSettings->iWindowHeight * 0.2f));
				ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));

				ImGui::SetWindowFontScale(1.8f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Upgrade Ventilation?");
				ImGui::SetWindowFontScale(1.6f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Quartz x 2");
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Ice x 1");

				ImGui::NewLine();
				ImGui::Text("You have:");
				cInventoryItem1 = cInventoryManager->GetItem("EnergyQuartz");
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Quartz: %d", cInventoryItem1->GetCount());
				cInventoryItem2 = cInventoryManager->GetItem("IceCrystal");
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Ice: %d", cInventoryItem2->GetCount());
				ImGui::NewLine();
				// Add codes for Start button here
				if (cInventoryItem1->GetCount() >= 0 && cInventoryItem2->GetCount() >= 0)
				{
					ImGui::Image((ImTextureID)AcceptButtonData.textureID, ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
					if (ImGui::IsItemClicked()) {
						makeChanges = true;
						//cInventoryItem1->Remove(2);
						//cInventoryItem2->Remove(1);
						cInventoryManager->GetItem("Upgrade_ventilation")->Add(1);
						CShip::GetInstance()->ventilationInterval = 12 - cInventoryManager->GetItem("Upgrade_ventilation")->GetCount() * 2;
						UpgradeState = UPGRADE_STATE::NOSTATE;
					}
				}
				ImGui::SameLine();

				ImGui::Image((ImTextureID)RejectButtonData.textureID, ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
				if (ImGui::IsItemClicked())
				{
					UpgradeState = UPGRADE_STATE::NOSTATE;
					makeChanges = false;
				}

				ImGui::End();
			}
			else {
				ImGui::Begin("Storage", NULL, livesWindowFlags);
				ImGui::SetWindowPos(ImVec2(blockPosition.x - cSettings->iWindowWidth * 0.30f,
					blockPosition.y - cSettings->iWindowHeight * 0.1f));
				ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));
				ImGui::SetWindowFocus();

				ImGui::SetWindowFontScale(1.8f * relativeScale_y);
				ImGui::TextColored(ImVec4(1, 1, 1, 1), "Maximum Upgrades Reached.");

				ImGui::SameLine();
				ImGui::Image((ImTextureID)RejectButtonData.textureID, ImVec2(buttonWidthSmall, buttonHeightSmall), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
				if (ImGui::IsItemClicked())
				{
					UpgradeState = UPGRADE_STATE::NOSTATE;
					makeChanges = false;
				}

				ImGui::End();
			}
			break;
		default:
			break;
		}
	
		break;
	case CGUI_SceneCombat::showExit:
		ImGui::Begin("Exit", NULL, livesWindowFlags);
		ImGui::SetWindowPos(ImVec2(blockPosition.x + cSettings->iWindowWidth * 0.05f,
			blockPosition.y - cSettings->iWindowHeight * 0.1f));
		ImGui::SetWindowSize(ImVec2(500.0f * relativeScale_x, 250.0f * relativeScale_y));
		ImGui::SetWindowFocus();

		// planet information
		ImGui::SetWindowFontScale(1.8f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 1, 1), "Start Exploring?");
		ImGui::NewLine();
		// Add codes for Start button here
		ImGui::Image((ImTextureID)AcceptButtonData.textureID, ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
		if (ImGui::IsItemClicked()) {
			makeChanges = true;
		}
		ImGui::SameLine();
		ImGui::Image((ImTextureID)RejectButtonData.textureID, ImVec2(buttonWidthSmall, buttonHeightSmall), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
		if (ImGui::IsItemClicked())
		{
			GuiState = GUI_STATE::noShow;
			makeChanges = false;
		}


		ImGui::End();
		break;
	default:
		break;
	}


	if (isCombat) {
		ImGui::Begin("EnemyInfo", NULL, healthWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * (0.65),
			cSettings->iWindowHeight * (0.2f)));
		ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_x, 10.0f * relativeScale_y));
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGui::SetWindowFocus();

		ImGui::Image((void*)(intptr_t)boxBarBackground, ImVec2(250 * relativeScale_x, 400 * relativeScale_y));
		ImGui::End();

		// information
		ImGui::Begin("EnemyInDepthInfo", NULL, healthWindowFlags);

		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * (0.73),
			cSettings->iWindowHeight * (0.27f)));
		ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_x, 10.0f * relativeScale_y));
		ImGui::SetWindowFocus();

		ImGui::SetWindowFontScale(1.8f * relativeScale_y);
		ImGui::Text("Enemy Type: ");
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGui::TextColored(ImVec4(1, 1, 0, 1), CShipEnemy::GetInstance()->enemyName.c_str());
		ImGui::NewLine();
		ImGui::GetOverlayDrawList();

		//ImGui::Begin("EnemDam", NULL, healthWindowFlags);
		//ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * (0.73f),
		//	cSettings->iWindowHeight * (0.36f)));
		//ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_x, 10.0f * relativeScale_y));
		//ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		//ImGui::SetWindowFocus();

		//ImGui::Text("Enemy Damage");
		//ImGui::Image((void*)(intptr_t)ProgressBarTextureID, ImVec2(160, 25));
		//ImGui::End();

		//// how. do i. get rid. of the border.
		//int damCount = CShipEnemy::GetInstance()->getHealth();
		//for (int i = 0; i < int(damCount / floor(CShipEnemy::GetInstance()->maxHealth / 10)); i++) {
		//	ImGui::Begin("Enem Damage Bit", NULL, healthWindowFlags);
		//	ImGui::SetWindowPos(ImVec2((cSettings->iWindowWidth) * 0.74f,
		//		cSettings->iWindowHeight * 0.40f));
		//	ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_x, 10.0f * relativeScale_y));
		//	ImGui::SetWindowFocus();

		//	ImGui::Image((void*)(intptr_t)BitTextureID, ImVec2(7 * relativeScale_x, 14 * relativeScale_y));
		//	ImGui::SameLine();
		//	ImGui::GetOverlayDrawList();
		//	ImGui::End();
		//}

		
		if (cSettings->iWindowWidth < 1000) {
			ImGui::Begin("EnemDam", NULL, healthWindowFlags);
			ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth - 216 * relativeScale_x,
				cSettings->iWindowHeight* (0.36f)));
			ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_y, 10.0f * relativeScale_y));
			ImGui::SetWindowFontScale(1.5f * relativeScale_y);
			ImGui::SetWindowFocus();

			ImGui::Text("Enemy Damage");
			ImGui::Image((void*)(intptr_t)ProgressBarTextureID, ImVec2(160 * relativeScale_y, 25 * relativeScale_y));
			ImGui::End();

			// how. do i. get rid. of the border.
			int damCount = CShipEnemy::GetInstance()->getHealth();
			for (int i = 0; i < int(damCount / floor(CShipEnemy::GetInstance()->maxHealth / 10)); i++) {
				ImGui::Begin("Enem Damage Bit", NULL, healthWindowFlags);
				ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth - 208 * relativeScale_y, cSettings->iWindowHeight * 0.40f));
				ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_x, 10.0f * relativeScale_y));
				ImGui::SetWindowFocus();

				ImGui::Image((void*)(intptr_t)BitTextureID, ImVec2(7 * relativeScale_x, 13 * relativeScale_y));
				ImGui::SameLine();
				ImGui::GetOverlayDrawList();
				ImGui::End();
			}
		}
		else {
			ImGui::Begin("EnemDam", NULL, healthWindowFlags);
			ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth - 216 * relativeScale_x,
				cSettings->iWindowHeight* (0.36f)));
			ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_y, 10.0f * relativeScale_y));
			ImGui::SetWindowFontScale(1.5f * relativeScale_y);
			ImGui::SetWindowFocus();

			ImGui::Text("Enemy Damage");
			ImGui::Image((void*)(intptr_t)ProgressBarTextureID, ImVec2(160 * relativeScale_x, 25 * relativeScale_y));
			ImGui::End();

			// how. do i. get rid. of the border.
			int damCount = CShipEnemy::GetInstance()->getHealth();
			for (int i = 0; i < int(damCount / floor(CShipEnemy::GetInstance()->maxHealth / 10)); i++) {
				ImGui::Begin("Enem Damage Bit", NULL, healthWindowFlags);
				ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth - 208 * relativeScale_x, cSettings->iWindowHeight * 0.398f));
				ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_x, 10.0f * relativeScale_y));
				ImGui::SetWindowFocus();

				ImGui::Image((void*)(intptr_t)BitTextureID, ImVec2(11.5 * relativeScale_x, 12 * relativeScale_y));
				ImGui::SameLine();
				ImGui::GetOverlayDrawList();
				ImGui::End();
			}
		}

		ImGui::Begin("EnemyPic", NULL, healthWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * (0.75f),
			cSettings->iWindowHeight * (0.45f)));
		ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_x, 10.0f * relativeScale_y));
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGui::SetWindowFocus();


		switch (CShipEnemy::GetInstance()->enemType)
		{
		case CShipEnemy::ENEMY_TYPE::EASY:
			ImGui::Image((void*)(intptr_t)enemyShip1, ImVec2(120 * relativeScale_y, 120 * relativeScale_y));
			break;
		case CShipEnemy::ENEMY_TYPE::MEDIUM:
			ImGui::Image((void*)(intptr_t)enemyShip2, ImVec2(120 * relativeScale_y, 120 * relativeScale_y));
			break;
		case CShipEnemy::ENEMY_TYPE::HARD:
			ImGui::Image((void*)(intptr_t)enemyShip3, ImVec2(120 * relativeScale_y, 120 * relativeScale_y));
			break;
		default:
			break;
		}

		ImGui::GetOverlayDrawList();
		ImGui::End();

		ImGui::Begin("EnemyGun", NULL, healthWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * (0.73f),
			cSettings->iWindowHeight * (0.70f)));
		ImGui::SetWindowSize(ImVec2(10.0f * relativeScale_x, 10.0f * relativeScale_y));
		ImGui::SetWindowFocus();

		ImGui::SetWindowFontScale(1.5f * relativeScale_y);
		ImGui::Text("Enemy Weapon:");
		// Time until next attack
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.0f, 0.9f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
		ImGui::ProgressBar((CShipEnemy::GetInstance()->TimeElapsed - CShipEnemy::GetInstance()->attackTimer) /
			(float)CShipEnemy::GetInstance()->enemyTimer, ImVec2(160.0f *
				relativeScale_x, 20.0f * relativeScale_y));
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::GetOverlayDrawList();
		ImGui::End();
		ImGui::End();

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