/**
 CScene2D
 @brief A class which manages the 2D game scene
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "ScenePlanet.h"
#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

#include "System\filesystem.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CScenePlanet::CScenePlanet(void)
	: cMap2D(NULL)
	, cPlayer2D(NULL)
	, cKeyboardController(NULL)	
	, cGUI_Scene2D(NULL)
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

	if (cPlayer2D)
	{
		cPlayer2D->Destroy();
		cPlayer2D = NULL;
	}
	
	if (camera2D)
	{
		camera2D->Destroy();
		camera2D = NULL;
	}

	if (cGUI_Scene2D)
	{
		cGUI_Scene2D->Destroy();
		cGUI_Scene2D = NULL;
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
	// TODO: [SP3] Load Empty Map
	if (cMap2D->LoadMap("Maps/DM2213_Map_Jungle_01.csv", 0) == false)
	{
		// The loading of a map has failed. Return false
		cout << "Failed to load Jungle Map Level 01" << endl;
		return false;
	}

	// Create and initialise the CPlayer2D
	cPlayer2D = CPlayer2D::GetInstance();
	// Pass shader to cPlayer2D
	cPlayer2D->SetShader("Shader2D_Colour");
	// Initialise the instance
	if (cPlayer2D->Init() == false)
	{
		cout << "Failed to load CPlayer2D" << endl;
		return false;
	}

	cMap2D->SetCurrentLevel(0); //reset level

	//// create the alarm box vector
	//alarmBoxVector.clear();
	//alarmBoxVector = cMap2D->FindAllTiles(50);

	//// assign alarm boxes to enemies (if applicable)
	//for (unsigned int i = 0; i < enemyVectors[cMap2D->GetCurrentLevel()].size(); i++)
	//{
	//	for (unsigned int j = 0; j < alarmBoxVector.size(); j++)
	//	{
	//		if (enemyVectors[cMap2D->GetCurrentLevel()][i]->vec2Index.y == alarmBoxVector[j].y)
	//		{
	//			enemyVectors[cMap2D->GetCurrentLevel()][i]->setAssignedAlarmBox(alarmBoxVector[j]);
	//		}
	//	}
	//}

	// Create and initialise the CGUI_Scene2D
	cGUI_Scene2D = CGUI_Scene2D::GetInstance();
	// Initialise the instance
	if (cGUI_Scene2D->Init() == false)
	{
		cout << "Failed to load CGUI_Scene2D" << endl;
		return false;
	}

	// Store the keyboard controller singleton instance here
	cKeyboardController = CKeyboardController::GetInstance();

	// Create and initialise the cGameManager
	cGameManager = CGameManager::GetInstance();
	cGameManager->Init();
	
	// Init the camera
	camera2D = Camera2D::GetInstance();
	camera2D->Reset();
	camera2D->setTargetPos(cPlayer2D->vec2Index);

	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Sound_BGM.ogg"), CSoundController::SOUND_LIST::BGM_NORMAL, true, true);
	cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BGM_NORMAL); // plays BGM on repeat

	return true;
}

/**
@brief Update Update this instance
*/
bool CScenePlanet::Update(const double dElapsedTime)
{
	// mouse Position demo
	camera2D->setTargetZoom(3.0f);
	
	// Call the cPlayer2D's update method before Map2D
	// as we want to capture the inputs before Map2D update
	cPlayer2D->Update(dElapsedTime);

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
	cGUI_Scene2D->Update(dElapsedTime);

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
	// Calls the Map2D's PreRender()
	cMap2D->PreRender();
	// Calls the Map2D's Render()
	cMap2D->Render();
	// Calls the Map2D's PostRender()
	cMap2D->PostRender();


}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CScenePlanet::PostRender(void)
{
}
