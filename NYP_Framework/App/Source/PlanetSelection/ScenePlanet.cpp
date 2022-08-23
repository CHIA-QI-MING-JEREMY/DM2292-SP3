/**
 CScene2D
 @brief A class which manages the 2D game scene
 By: Toh Da Jun
 Date: Mar 2020
 */
#include <iostream>
#include "math.h"
	using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

#include "System\filesystem.h"
#include "ScenePlanet.h"
#include "../App/Source/GameStateManagement/GameInfo.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CScenePlanet::CScenePlanet(void)
	: cMap2D(NULL)
	, cKeyboardController(NULL)
	, cGUI_ScenePlanet(NULL)
	, cGameManager(NULL)
	, camera2D(NULL)
	, cSoundController(NULL)
{
}

/**
 @brief Destructor
 */
CScenePlanet::~CScenePlanet(void)
{
	if (cKeyboardController)
	{
		// We won't delete this since it was created elsewhere
		cKeyboardController = NULL;
	}

	if (cMap2D)
	{
		cMap2D->Destroy();
		cMap2D = NULL;
	}

	if (camera2D)
	{
		camera2D->Destroy();
		camera2D = NULL;
	}

	if (cGUI_ScenePlanet)
	{
		cGUI_ScenePlanet->Destroy();
		cGUI_ScenePlanet = NULL;
	}

	if (cGameManager)
	{
		cGameManager->Destroy();
		cGameManager = NULL;
	}

	if (cSoundController)
	{
		cSoundController = NULL;
	}
}

/**
@brief Init Initialise this instance
*/
bool CScenePlanet::Init(void)
{
	// Include Shader Manager
	CShaderManager::GetInstance()->Use("Shader2D");

	//Create Background Entity
	background = new CBackgroundEntity("Image/space_bg.png");
	background->SetShader("Shader2D");
	background->Init();

	// Create and initialise the cMap2D
	cMap2D = CMap2D::GetInstance();
	// Set a shader to this class
	cMap2D->SetShader("Shader2D");
	// Initialise the instance
	if (cMap2D->Init(1, CSettings::GetInstance()->NUM_TILES_YAXIS, CSettings::GetInstance()->NUM_TILES_XAXIS) == false)
	{
		cout << "Failed to load CMap2D" << endl;
		return false;
	}
	// Load the map into an array
	if (cMap2D->LoadMap("Maps/DM2213_Map_Planet.csv", 0) == false)
	{
		// The loading of a map has failed. Return false
		cout << "Failed to load planet selection map" << endl;
		return false;
	}

	// Create and initialise the CGUI_Scene2D
	cGUI_ScenePlanet = CGUI_ScenePlanet::GetInstance();
	// Initialise the instance
	if (cGUI_ScenePlanet->Init() == false)
	{
		cout << "Failed to load cGUI_ScenePlanet" << endl;
		return false;
	}

	//cycle through the maps and find the enemies
	//and push them into the planet vector

	cMap2D->SetCurrentLevel(0); //reset level
	nebula = NULL;

	camera2D = Camera2D::GetInstance();
	camera2D->Reset();

	int snowCount = 0;
	int terraCount = 0;
	int jungleCount = 0;
	int counter = 0;

	srand(time(NULL));

	planetVector.clear();
	while (true) {
		CPlanet* cPlanet = new CPlanet();
		// Pass shader to cEnemy2D
		cPlanet->SetShader("Shader2D_Colour");
		// Initialise the instance
		if (cPlanet->Init() == true) {
			if (cPlanet->vec2Index.x == 0) {
				cPlanet->UpdatePlanetIcon("Image/Planet/nebula.png");
				cPlanet->SetScale(6);
				cPlanet->SetVisibility(true);
				nebula = cPlanet;
			}
			else if (cPlanet->vec2Index.x == 8) {
				// first time running the planet so we init with this x
				if (CGameInfo::GetInstance()->initPlanets == false) {
					camera2D->setTargetPos(glm::vec2(cPlanet->vec2Index.x, cPlanet->vec2Index.y));
					CGameInfo::GetInstance()->currentPlanetPos = cPlanet->vec2Index;
				}
				cPlanet->SetType(CPlanet::TYPE::JUNGLE_TUTORIAL);
				PlanetSelected = cPlanet;
				cPlanet->SetVisibility(true);
				cPlanet->planetName = "Stan Loona";
				cGUI_ScenePlanet->setPlanetInfo(cPlanet);
			}
			else if (cPlanet->vec2Index.x == 11) {
				cPlanet->SetType(CPlanet::TYPE::SNOW_TUTORIAL);
			}
			else if (cPlanet->vec2Index.x == 14) {
				cPlanet->SetType(CPlanet::TYPE::TERRESTRIAL_TUTORIAL);
			}
			else if (cPlanet->vec2Index.x == 31) {
				cPlanet->SetType(CPlanet::TYPE::FINAL);
				cPlanet->planetName = "Stan Kep1er";
			}
			else {
				if (CGameInfo::GetInstance()->initPlanets == false) {
					// randomiser function
					bool isSet = false;
					while (!isSet) {
						int randomType = rand() % 6;
						switch (randomType)
						{
						case 0:
						case 3:
							if (jungleCount < 2) {
								cPlanet->SetType(CPlanet::TYPE::JUNGLE);
								jungleCount++;
								isSet = true;
								CGameInfo::GetInstance()->planetList.push_back(0);
							}
							else {
								continue;
							}
							break;
						case 1:
						case 4:
							if (snowCount < 2) {
								cPlanet->SetType(CPlanet::TYPE::SNOW);
								snowCount++;
								isSet = true;
								CGameInfo::GetInstance()->planetList.push_back(1);
							}
							else {
								continue;
							}
							break;
						case 2:
						case 5:
							if (terraCount < 2) {
								cPlanet->SetType(CPlanet::TYPE::TERRESTRIAL);
								terraCount++;
								isSet = true;
								CGameInfo::GetInstance()->planetList.push_back(2);
							}
							else {
								continue;
							}
							break;
						default:
							break;
						}
					}
				}
				else {
					switch (CGameInfo::GetInstance()->planetList[counter])
					{
					case 0:
						cPlanet->SetType(CPlanet::TYPE::JUNGLE);
						break;
					case 1:
						cPlanet->SetType(CPlanet::TYPE::SNOW);
						break;
					case 2:
						cPlanet->SetType(CPlanet::TYPE::TERRESTRIAL);
						break;
					default:
						break;
					}
					counter++;
				}
			}

			planetVector.insert(std::make_pair(std::make_pair(cPlanet->vec2Index.x, cPlanet->vec2Index.y), cPlanet));
		}
		else {
			//break up of loop if all the enemies have been loaded
			break;
		}
	}

	if (CGameInfo::GetInstance()->initPlanets == false) {
		CGameInfo::GetInstance()->initPlanets = true;
		realSize = nebulaSize = 5;
		nebula->SetScale(nebulaSize);
		CGameInfo::GetInstance()->nebulaSize = nebulaSize;
	}
	else {
		std::map<std::pair<int, int>, CPlanet*>::iterator otherPlanets = planetVector.begin();

		realSize = nebulaSize = CGameInfo::GetInstance()->nebulaSize;
		nebulaSize += rand() % 10 + 1;
		std::cout << nebulaSize << "\n";
		CGameInfo::GetInstance()->nebulaSize = nebulaSize;

		while (otherPlanets != planetVector.end()) {
			if (otherPlanets->second->vec2Index.x == 0) {
				otherPlanets++;
				continue;
			}
			// TODO: [SP3] Set it so that u cannot visit other planets during tutorial
			glm::vec2 s = CGameInfo::GetInstance()->selectedPlanet->getPos();
			if (s == otherPlanets->second->vec2Index) {
				camera2D->setTargetPos(glm::vec2(otherPlanets->second->vec2Index.x, otherPlanets->second->vec2Index.y));
				PlanetSelected = otherPlanets->second;
			}

			if (glm::abs(otherPlanets->second->vec2Index.x - PlanetSelected->vec2Index.x) < 6) {
				otherPlanets->second->SetVisibility(true);
			}
			else {
				otherPlanets->second->SetVisibility(false);
			}

			if (otherPlanets->second->vec2Index.x <= int((nebulaSize - 3) / 2)) {
				// Add lose condition

				otherPlanets->second->SetVisibility(false);
				cMap2D->SetMapInfo(otherPlanets->second->vec2Index.y, otherPlanets->second->vec2Index.x, 598, true);
			}
			otherPlanets++;
		}
	}

	// Store the keyboard controller singleton instance here
	cKeyboardController = CKeyboardController::GetInstance();

	// Create and initialise the cGameManager
	cGameManager = CGameManager::GetInstance();
	cGameManager->Init();

	StartCombat = false;
	lState = false;

	return true;
}

/**
@brief Update Update this instance
*/
bool CScenePlanet::Update(const double dElapsedTime)
{
 	if (cKeyboardController->IsKeyReleased(GLFW_KEY_ENTER)) {
		if (PlanetSelected->getVisibility() == true) {
			gotoPlanet = PlanetSelected;
			CGameInfo::GetInstance()->selectedPlanet = PlanetSelected;
			StartCombat = true;
		}
	}

	if (nebulaSize != realSize) {
		camera2D->lerp(realSize, nebulaSize, 0.01f);
		nebula->SetScale(realSize);
	}

	// mouse Position demo
	camera2D->setTargetZoom(1.0f);
	if (CMouseController::GetInstance()->IsButtonUp(CMouseController::BUTTON_TYPE::LMB)) {
		lState = false;
	}

	// mouse Click
	auto& io = ImGui::GetIO();
	if (CMouseController::GetInstance()->IsButtonDown(CMouseController::BUTTON_TYPE::LMB) && !lState && !cGUI_ScenePlanet->isButtonHover) {
		lState = true;
		glm::vec2 blockSelected = camera2D->getBlockSelected();
		std::map<std::pair<int, int>, CPlanet*>::iterator x = planetVector.begin();
		while (x != planetVector.end()) {
			glm::vec2 playerPos = glm::vec2(x->first.first, x->first.second);
			if (glm::abs(glm::length(blockSelected - playerPos)) < 3.f) {

				// skip miss nebularr
				if (x->second->vec2Index.x == 0) {
					x++;
					continue;
				}

				PlanetSelected = x->second;
				cGUI_ScenePlanet->setPlanetInfo(x->second);

				// TODO: remove the panel uh-
				if (glm::abs(x->second->vec2Index.x - camera2D->getPos().x) < 1) {
					if (cGUI_ScenePlanet->isShowPanel) {
						cGUI_ScenePlanet->isShowPanel = false;
					}
					else {
						cGUI_ScenePlanet->isShowPanel = true;
					}
				}
				else {
					cGUI_ScenePlanet->isShowPanel = true;
				}

				camera2D->setTargetPos(x->second->vec2Index);
			}
			x++;
		}
	}
	

	camera2D->Update(dElapsedTime);

	// Call the Map2D's update method
	cMap2D->Update(dElapsedTime);

	if (cKeyboardController->IsKeyReleased(GLFW_KEY_F6))
	{
		// Save the current game to a save file
		// Make sure the file is open
		try {
			if (cMap2D->SaveMap("Maps/DM2213_SAVEGAME.csv", cMap2D->GetCurrentLevel()) == false)
			{
				throw runtime_error("Unable to save the current game to a file");
			}
		}
		catch (runtime_error e)
		{
			cout << "Runtime error: " << e.what();
			return false;
		}
	}

	// Call the cGUI_Scene2D's update method
	cGUI_ScenePlanet->Update(dElapsedTime);

	//TODO: [SP3] Add transition to next stage (ship combat)
	// unslay

	return true;
}

/**
 @brief PreRender Set up the OpenGL display environment before rendering
 */
void CScenePlanet::PreRender(void)
{
	// Reset the OpenGL rendering environment
	glLoadIdentity();

	// Clear the screen and buffer
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Enable 2D texture rendering
	glEnable(GL_TEXTURE_2D);
}

/**
 @brief Render Render this instance
 */
void CScenePlanet::Render(void)
{
	//Render Background
	background->Render();

	// Calls the Map2D's PreRender()
	cMap2D->PreRender();
	// Calls the Map2D's Render()
	cMap2D->Render();
	// Calls the Map2D's PostRender()
	cMap2D->PostRender();

	for (auto planet : planetVector) {
		planet.second->PreRender();
		planet.second->Render();
		planet.second->PostRender();
	}

	// Calls the CGUI_Scene2D's PreRender()
	cGUI_ScenePlanet->PreRender();
	// Calls the CGUI_Scene2D's Render()
	cGUI_ScenePlanet->Render();
	// Calls the CGUI_Scene2D's PostRender()
	cGUI_ScenePlanet->PostRender();
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CScenePlanet::PostRender(void)
{
}
