/**
 Map2D
 @brief A class which manages the map in the game
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Map2D.h"

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

// Include Filesystem
#include "System\filesystem.h"
// Include ImageLoader
#include "System\ImageLoader.h"
#include "Primitives/MeshBuilder.h"

#include <iostream>
#include <vector>
using namespace std;

// For AStar PathFinding
using namespace std::placeholders;

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CMap2D::CMap2D(void)
	: uiCurLevel(0)
	, quadMesh(NULL)
	, camera2D(NULL)
{
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CMap2D::~CMap2D(void)
{
	// Dynamically deallocate the 3D array used to store the map information
	for (unsigned int uiLevel = 0; uiLevel < uiNumLevels; uiLevel++)
	{
		for (unsigned int iRow = 0; iRow < cSettings->NUM_TILES_YAXIS; iRow++)
		{
			delete[] arrMapInfo[uiLevel][iRow];
		}
		delete [] arrMapInfo[uiLevel];
	}
	delete[] arrMapInfo;

	if (quadMesh)
	{
		delete quadMesh;
		quadMesh = NULL;
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	
	// nullify the camera since it was created elsewhere
	camera2D = NULL;

	// Set this to NULL since it was created elsewhere, so we let it be deleted there.
	cSettings = NULL;

	// Delete AStar lists
	DeleteAStarLists();
}

/**
@brief Init Initialise this instance
*/ 
bool CMap2D::Init(	const unsigned int uiNumLevels,
					const unsigned int uiNumRows,
					const unsigned int uiNumCols)
{
	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();
	// get the camera main
	camera2D = Camera2D::GetInstance();

	// Create the arrMapInfo and initialise to 0
	// Start by initialising the number of levels
	arrMapInfo = new Grid** [uiNumLevels];
	for (unsigned uiLevel = 0; uiLevel < uiNumLevels; uiLevel++)
	{
		arrMapInfo[uiLevel] = new Grid* [uiNumRows];
		for (unsigned uiRow = 0; uiRow < uiNumRows; uiRow++)
		{
			arrMapInfo[uiLevel][uiRow] = new Grid[uiNumCols];
			for (unsigned uiCol = 0; uiCol < uiNumCols; uiCol++)
			{
				arrMapInfo[uiLevel][uiRow][uiCol].value = 0;
			}
		}
	}

	// Store the map sizes in cSettings
	uiCurLevel = 0;
	this->uiNumLevels = uiNumLevels;
	cSettings->NUM_TILES_XAXIS = uiNumCols;
	cSettings->NUM_TILES_YAXIS = uiNumRows;
	cSettings->UpdateSpecifications();

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//CS: Create the Quad Mesh using the mesh builder
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

	// Load and create textures
	// Load the ground textures
	// Load the brown tile texture

	// TO REMOVE LATER
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_BrownTile.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_BrownTile.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(100, iTextureID));
	}

	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_BrownTile.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_BrownTile.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(1000, iTextureID));
	}

	/// <summary>
	/// JUNGLE PLANET SOLID BLOCKS
	/// </summary>
	{
		//poison sprout tile
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/JunglePlanet/PoisonSprout.tga", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/JunglePlanet/PoisonSprout.tga" << endl;
			return false;
		}
		else
		{
			// Store the texture ID into MapOfTextureIDs
			MapOfTextureIDs.insert(pair<int, int>(POISON_SPROUT, iTextureID));
		}
		//poison fog tile
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/JunglePlanet/PoisonFog.tga", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/JunglePlanet/PoisonFog.tga" << endl;
			return false;
		}
		else
		{
			// Store the texture ID into MapOfTextureIDs
			MapOfTextureIDs.insert(pair<int, int>(POISON_FOG, iTextureID));
		}
	}

	/// <summary>
	/// JUNGLE PLANET SOLID BLOCKS
	/// </summary>
	{
		//burnable bush tile
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/JunglePlanet/BurnableBushBlock.tga", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/JunglePlanet/BurnableBushBlock.tga" << endl;
			return false;
		}
		else
		{
			// Store the texture ID into MapOfTextureIDs
			MapOfTextureIDs.insert(pair<int, int>(BURNABLE_BUSH, iTextureID));
		}
		//burning burnable bush tile
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/JunglePlanet/BurnableBushBlock_Burning.tga", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/JunglePlanet/BurnableBushBlock_Burning.tga" << endl;
			return false;
		}
		else
		{
			// Store the texture ID into MapOfTextureIDs
			MapOfTextureIDs.insert(pair<int, int>(BURNING_BUSH, iTextureID));
		}
		//dissolving burnable bush tile
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/JunglePlanet/BurnableBushBlock_Dissolving.tga", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/JunglePlanet/BurnableBushBlock_Dissolving.tga" << endl;
			return false;
		}
		else
		{
			// Store the texture ID into MapOfTextureIDs
			MapOfTextureIDs.insert(pair<int, int>(DISSOLVING_BUSH, iTextureID));
		}


		//grass ground tile
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/JunglePlanet/GrassBlock.tga", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/JunglePlanet/GrassBlock.tga" << endl;
			return false;
		}
		else
		{
			// Store the texture ID into MapOfTextureIDs
			MapOfTextureIDs.insert(pair<int, int>(GRASS_GROUND, iTextureID));
		}
		//grass ground, top left corner tile
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/JunglePlanet/GrassBlock_TopLeftCorner.tga", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/JunglePlanet/GrassBlock_TopLeftCorner.tga" << endl;
			return false;
		}
		else
		{
			// Store the texture ID into MapOfTextureIDs
			MapOfTextureIDs.insert(pair<int, int>(GRASS_GROUND_L, iTextureID));
		}
		//grass ground, top right corner tile
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/JunglePlanet/GrassBlock_TopRightCorner.tga", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/JunglePlanet/GrassBlock_TopRightCorner.tga" << endl;
			return false;
		}
		else
		{
			// Store the texture ID into MapOfTextureIDs
			MapOfTextureIDs.insert(pair<int, int>(GRASS_GROUND_R, iTextureID));
		}
	}


	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_BrownTile.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_BrownTile.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(800, iTextureID));
	}
	// Load the yellow tile texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_YellowTile.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_YellowTile.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(801, iTextureID));
	}
	// Load the red tile texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_RedTile.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_RedTile.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(802, iTextureID));
	}
	// Load the green tile texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_GreenTile.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_GreenTile.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(803, iTextureID));
	}
	// Load the blue tile texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_BlueTile.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_BlueTile.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(804, iTextureID));
	}
	// Load the walkthrough textures
	// Load the yellow tile texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_YellowTile.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_YellowTile.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(201, iTextureID));
	}
	// Load the red tile texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_RedTile.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_RedTile.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(202, iTextureID));
	}
	// Load the green tile texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_GreenTile.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_GreenTile.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(203, iTextureID));
	}
	// Load the blue tile texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_BlueTile.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_BlueTile.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(204, iTextureID));
	}
	// Load rope textures
	// Load post with rope coiled
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_PostWithRope_Coiled.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_PostWithRope_Coiled.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(210, iTextureID));
	}
	// Load post with rope uncoiled left
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_PostWithRope_Uncoiled_Left.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_PostWithRope_Uncoiled_Left.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(211, iTextureID));
	}
	// Load rope corner left
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_Rope_Corner_Left.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_Rope_Corner_Left.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(212, iTextureID));
	}
	// Load rope length left
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_Rope_Length_Left.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_Rope_Length_Left.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(213, iTextureID));
	}
	// Load post with rope uncoiled right
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_PostWithRope_Uncoiled_Right.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_PostWithRope_Uncoiled_Right.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(214, iTextureID));
	}
	// Load rope corner right
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_Rope_Corner_Right.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_Rope_Corner_Right.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(215, iTextureID));
	}
	// Load rope length right
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_Rope_Length_Right.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_Rope_Length_Right.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(216, iTextureID));
	}
	// Load black flag
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_BlackFlag.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_BlackFlag.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(230, iTextureID));
	}
	// Load red flag
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_RedFlag.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_RedFlag.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(231, iTextureID));
	}
	// Load spikes up
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_SpikesUp.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_SpikesUp.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(240, iTextureID));
	}
	// Load spikes left
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_SpikesLeft.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_SpikesLeft.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(241, iTextureID));
	}
	// Load spikes down
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_SpikesDown.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_SpikesDown.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(242, iTextureID));
	}
	// Load spikes right
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_SpikesRight.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_SpikesRight.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(243, iTextureID));
	}
	// Load alarm box
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_AlarmBox.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_AlarmBox.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(250, iTextureID));
	}
	// Load alarm light off
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_AlarmLight_Off.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_AlarmLight_Off.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(251, iTextureID));
	}
	// Load alarm light on
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_AlarmLight_On.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_AlarmLight_On.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(252, iTextureID));
	}
	// Load the exit texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_Exit.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_Exit.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(299, iTextureID));
	}
	
	// Initialise the variables for AStar
	m_weight = 1;
	m_startPos = glm::vec2(0, 0);
	m_targetPos = glm::vec2(0, 0);
	//m_size = cSettings->NUM_TILES_YAXIS* cSettings->NUM_TILES_XAXIS;

	m_nrOfDirections = 4;
	m_directions = { { -1, 0 }, { 1, 0 }, { 0, 1 }, { 0, -1 },
						{ -1, -1 }, { 1, 1 }, { -1, 1 }, { 1, -1 } };

	// Resize these 2 lists
	m_cameFromList.resize(cSettings->NUM_TILES_YAXIS* cSettings->NUM_TILES_XAXIS);
	m_closedList.resize(cSettings->NUM_TILES_YAXIS* cSettings->NUM_TILES_XAXIS, false);

	return true;
}

/**
@brief Update Update this instance
*/
void CMap2D::Update(const double dElapsedTime)
{
}

/**
 @brief PreRender Set up the OpenGL display environment before rendering
 */
void CMap2D::PreRender(void)
{
	// Activate blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);

	// Activate the shader
	CShaderManager::GetInstance()->Use(sShaderName);
}

/**
 @brief Render Render this instance
 */
void CMap2D::Render(void)
{
	// get matrix's uniform location and set matrix
	unsigned int transformLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

	glm::vec2 offset = glm::vec2(float(cSettings->NUM_TILES_XAXIS / 2.f), float(cSettings->NUM_TILES_YAXIS / 2.f) - 1);
	//glm::vec2 offset = glm::vec2(0);
	glm::vec2 cameraPos = camera2D->getPos();

	// Render
	for (unsigned int uiRow = 0; uiRow < cSettings->NUM_TILES_YAXIS; uiRow++)
	{
		for (unsigned int uiCol = 0; uiCol < cSettings->NUM_TILES_XAXIS; uiCol++)
		{
			glm::vec2 objTransform = glm::vec2(uiCol, cSettings->NUM_TILES_YAXIS - uiRow);
			glm::vec2 objCamPos = objTransform - cameraPos + offset;

			glm::vec2 actualPos;
			actualPos = cSettings->ConvertIndexToUVSpace(objCamPos) * camera2D->getZoom();

			transform = glm::mat4(1.f);
			transform = glm::translate(transform, glm::vec3(actualPos.x, actualPos.y, 0.f));
			transform = glm::scale(transform, glm::vec3(camera2D->getZoom()));

			// Update the shaders with the latest transform
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

			RenderTile(uiRow, uiCol);
		}
	}
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CMap2D::PostRender(void)
{
	// Disable blending
	glDisable(GL_BLEND);
}

// Set the specifications of the map
void CMap2D::SetNumTiles(const CSettings::AXIS sAxis, const unsigned int uiValue)
{
	// Check if the value is valid
	if (uiValue <= 0)
	{
		cout << "CMap2D::SetNumTiles() : value must be more than 0" << endl;
		return;
	}

	if (sAxis == CSettings::x)
	{
		cSettings->NUM_TILES_XAXIS = uiValue;
		cSettings->UpdateSpecifications();
	}
	else if (sAxis == CSettings::y)
	{
		cSettings->NUM_TILES_YAXIS = uiValue;
		cSettings->UpdateSpecifications();
	}
	else if (sAxis == CSettings::z)
	{
		// Not used in here
	}
	else
	{
		cout << "Unknown axis" << endl;
	}
}

// Set the specifications of the map
void CMap2D::SetNumSteps(const CSettings::AXIS sAxis, const unsigned int uiValue)
{
	// Check if the value is valid
	if (uiValue <= 0)
	{
		cout << "CMap2D::SetNumSteps() : value must be more than 0" << endl;
		return;
	}

	if (sAxis == CSettings::x)
	{
		cSettings->NUM_STEPS_PER_TILE_XAXIS = (float)uiValue;
		cSettings->UpdateSpecifications();
	}
	else if (sAxis == CSettings::y)
	{
		cSettings->NUM_STEPS_PER_TILE_YAXIS = (float)uiValue;
		cSettings->UpdateSpecifications();
	}
	else if (sAxis == CSettings::z)
	{
		// Not used in here
	}
	else
	{
		cout << "Unknown axis" << endl;
	}
}

/**
 @brief Set the value at certain indices in the arrMapInfo
 @param iRow A const int variable containing the row index of the element to set to
 @param iCol A const int variable containing the column index of the element to set to
 @param iValue A const int variable containing the value to assign to this arrMapInfo
 */
void CMap2D::SetMapInfo(const unsigned int uiRow, const unsigned int uiCol, const int iValue, const bool bInvert)
{
	if (bInvert)
		arrMapInfo[uiCurLevel][cSettings->NUM_TILES_YAXIS - uiRow - 1][uiCol].value = iValue;
	else
		arrMapInfo[uiCurLevel][uiRow][uiCol].value = iValue;
}

/**
 @brief Get the value at certain indices in the arrMapInfo
 @param iRow A const int variable containing the row index of the element to get from
 @param iCol A const int variable containing the column index of the element to get from
 @param bInvert A const bool variable which indicates if the row information is inverted
 */
int CMap2D::GetMapInfo(const unsigned int uiRow, const int unsigned uiCol, const bool bInvert) const
{
	if (bInvert)
		return arrMapInfo[uiCurLevel][cSettings->NUM_TILES_YAXIS - uiRow - 1][uiCol].value;
	else
		return arrMapInfo[uiCurLevel][uiRow][uiCol].value;
}

/**
 @brief Load a map
 */ 
bool CMap2D::LoadMap(string filename, const unsigned int uiCurLevel)
{
	doc = rapidcsv::Document(FileSystem::getPath(filename).c_str());

	// Check if the sizes of CSV data matches the declared arrMapInfo sizes
	if ((cSettings->NUM_TILES_XAXIS != (unsigned int)doc.GetColumnCount()) ||
		(cSettings->NUM_TILES_YAXIS != (unsigned int)doc.GetRowCount()))
	{
		cout << "Sizes of CSV map does not match declared arrMapInfo sizes." << endl;
		return false;
	}

	// Read the rows and columns of CSV data into arrMapInfo
	for (unsigned int uiRow = 0; uiRow < cSettings->NUM_TILES_YAXIS; uiRow++)
	{
		// Read a row from the CSV file
		std::vector<std::string> row = doc.GetRow<std::string>(uiRow);
		
		// Load a particular CSV value into the arrMapInfo
		for (unsigned int uiCol = 0; uiCol < cSettings->NUM_TILES_XAXIS; ++uiCol)
		{
			arrMapInfo[uiCurLevel][uiRow][uiCol].value = (int)stoi(row[uiCol]);
		}
	}

	return true;
}

int CMap2D::GetNumRows()
{
	return cSettings->NUM_TILES_YAXIS;
}

int CMap2D::GetNumCols()
{
	return cSettings->NUM_TILES_XAXIS;
}

vector<glm::vec2> CMap2D::FindAllTiles(unsigned int textureID, int lowerMapRow, int upperMapRow, int leftMapCol, int rightMapCol)
{
	vector<glm::vec2> tileVector;
	tileVector.clear();
	
	if (lowerMapRow == -1)
	{
		lowerMapRow = cSettings->NUM_TILES_YAXIS;
	}
	else
	{
		lowerMapRow = cSettings->NUM_TILES_YAXIS - lowerMapRow;
	}
	if (upperMapRow == -1)
	{
		upperMapRow = 0;
	}
	else
	{
		upperMapRow = cSettings->NUM_TILES_YAXIS - upperMapRow;
	}

	if (rightMapCol == -1)
	{
		rightMapCol = cSettings->NUM_TILES_XAXIS;
	}

	// checks through entire map of all levels
	for (unsigned int uiRow = upperMapRow; uiRow < lowerMapRow; uiRow++)
	{
		for (unsigned int uiCol = leftMapCol; uiCol < rightMapCol; uiCol++)
		{
			// if texture ID is found, change to new texture ID
			if (arrMapInfo[uiCurLevel][uiRow][uiCol].value == textureID)
			{
				tileVector.push_back(glm::vec2(uiCol, cSettings->NUM_TILES_YAXIS - 1 - uiRow));
			}
		}
	}
	
	return tileVector;
}

bool CMap2D::ReplaceTiles(unsigned int currTextureID, unsigned int newTextureID, int lowerMapRow, int upperMapRow, int leftMapCol, int rightMapCol)
{
	if (lowerMapRow == -1)
	{
		lowerMapRow = cSettings->NUM_TILES_YAXIS;
	}
	else
	{
		lowerMapRow = cSettings->NUM_TILES_YAXIS - lowerMapRow;
	}
	if (upperMapRow == -1)
	{
		upperMapRow = 0;
	}
	else
	{
		upperMapRow = cSettings->NUM_TILES_YAXIS - upperMapRow;
	}

	if (rightMapCol == -1)
	{
		rightMapCol = cSettings->NUM_TILES_XAXIS;
	}

	// checks through entire map of all levels
	for (unsigned int uiRow = upperMapRow; uiRow < lowerMapRow; uiRow++)
	{
		for (unsigned int uiCol = leftMapCol; uiCol < rightMapCol; uiCol++)
		{
			// if texture ID is found, change to new texture ID
			if (arrMapInfo[uiCurLevel][uiRow][uiCol].value == currTextureID)
			{
				arrMapInfo[uiCurLevel][uiRow][uiCol].value = newTextureID;
			}
		}
	}
	return true;
}

/**
 @brief Save the tilemap to a text file
 @param filename A string variable containing the name of the text file to save the map to
 */
bool CMap2D::SaveMap(string filename, const unsigned int uiCurLevel)
{
	// Update the rapidcsv::Document from arrMapInfo
	for (unsigned int uiRow = 0; uiRow < cSettings->NUM_TILES_YAXIS; uiRow++)
	{
		for (unsigned int uiCol = 0; uiCol < cSettings->NUM_TILES_XAXIS; uiCol++)
		{
			doc.SetCell(uiCol, uiRow, arrMapInfo[uiCurLevel][uiRow][uiCol].value);
		}
		cout << endl;
	}

	// Save the rapidcsv::Document to a file
	doc.Save(FileSystem::getPath(filename).c_str());

	return true;
}

/**
@brief Find the indices of a certain value in arrMapInfo
@param iValue A const int variable containing the row index of the found element
@param iRow A const int variable containing the row index of the found element
@param iCol A const int variable containing the column index of the found element
@param bInvert A const bool variable which indicates if the row information is inverted
*/
bool CMap2D::FindValue(const int iValue, unsigned int& uirRow, unsigned int& uirCol, const bool bInvert)
{
	for (unsigned int uiRow = 0; uiRow < cSettings->NUM_TILES_YAXIS; uiRow++)
	{
		for (unsigned int uiCol = 0; uiCol < cSettings->NUM_TILES_XAXIS; uiCol++)
		{
			if (arrMapInfo[uiCurLevel][uiRow][uiCol].value == iValue)
			{
				if (bInvert)
					uirRow = cSettings->NUM_TILES_YAXIS - uiRow - 1;
				else
					uirRow = uiRow;
				uirCol = uiCol;
				return true;	// Return true immediately if the value has been found
			}
		}
	}
	return false;
}

unsigned int CMap2D::FindGround(unsigned int startRow, unsigned int mapCol, const bool bInvert)
{
	if (bInvert)
	{
		startRow = cSettings->NUM_TILES_YAXIS - startRow - 1;
	}

	for (unsigned int uiRow = startRow; uiRow < cSettings->NUM_TILES_YAXIS; uiRow++)
	{
		if (arrMapInfo[uiCurLevel][uiRow][mapCol].value >= 600 && arrMapInfo[uiCurLevel][uiRow][mapCol].value < 1400)
		{
			if (bInvert)
			{
				return cSettings->NUM_TILES_YAXIS - uiRow - 1;
			}
			else
			{
				return uiRow;
			}
		}
	}
	return -1;
}

/**
 @brief Set current level
 */
void CMap2D::SetCurrentLevel(unsigned int uiCurLevel)
{
	if (uiCurLevel < uiNumLevels)
	{
		this->uiCurLevel = uiCurLevel;
	}
}
/**
 @brief Get current level
 */
unsigned int CMap2D::GetCurrentLevel(void) const
{
	return uiCurLevel;
}

std::vector<glm::vec2> CMap2D::PathFind(const glm::vec2& startPos, const glm::vec2& targetPos, HeuristicFunction heuristicFunc, int weight)
{
	// Check if the startPos and targetPost are blocked
	if (isBlocked(startPos.y, startPos.x) ||
		(isBlocked(targetPos.y, targetPos.x)))
	{
		cout << "Invalid start or target position." << endl;
		// Return an empty path
		std::vector<glm::vec2> path;
		return path;
	}

	// Set up the variables and lists
	m_startPos = startPos;
	m_targetPos = targetPos;
	m_weight = weight;
	m_heuristic = std::bind(heuristicFunc, _1, _2, _3);

	// Reset AStar lists
	ResetAStarLists();

	// Add the start pos to 2 lists
	m_cameFromList[ConvertTo1D(m_startPos)].parent = m_startPos;
	m_openList.push(Grid(m_startPos, 0));

	unsigned int fNew, gNew, hNew;
	glm::vec2 currentPos;

	// Start the path finding...
	while (!m_openList.empty())
	{
		// Get the node with the least f value
		currentPos = m_openList.top().pos;
		//cout << endl << "*** New position to check: " << currentPos.x << ", " << currentPos.y << endl;
		//cout << "*** targetPos: " << m_targetPos.x << ", " << m_targetPos.y << endl;

		// If the targetPos was reached, then quit this loop
		if (currentPos == m_targetPos)
		{
			//cout << "=== Found the targetPos: " << m_targetPos.x << ", " << m_targetPos.y << endl;
			while (m_openList.size() != 0)
				m_openList.pop();
			break;
		}

		m_openList.pop();
		m_closedList[ConvertTo1D(currentPos)] = true;

		// Check the neighbors of the current node
		for (unsigned int i = 0; i < m_nrOfDirections; ++i)
		{
			const auto neighborPos = currentPos + m_directions[i];
			const auto neighborIndex = ConvertTo1D(neighborPos);

			//cout << "\t#" << i << ": Check this: " << neighborPos.x << ", " << neighborPos.y << ":\t";
			if (!isValid(neighborPos) ||
				isBlocked(neighborPos.y, neighborPos.x) ||
				m_closedList[neighborIndex] == true)
			{
				//cout << "This position is not valid. Going to next neighbour." << endl;
				continue;
			}

			gNew = m_cameFromList[ConvertTo1D(currentPos)].g + 1;
			hNew = m_heuristic(neighborPos, m_targetPos, m_weight);
			fNew = gNew + hNew;

			if (m_cameFromList[neighborIndex].f == 0 || fNew < m_cameFromList[neighborIndex].f)
			{
				//cout << "Adding to Open List: " << neighborPos.x << ", " << neighborPos.y;
				//cout << ". [ f : " << fNew << ", g : " << gNew << ", h : " << hNew << "]" << endl;
				m_openList.push(Grid(neighborPos, fNew));
				m_cameFromList[neighborIndex] = { neighborPos, currentPos, fNew, gNew, hNew };
			}
			else
			{
				//cout << "Not adding this" << endl;
			}
		}
		//system("pause");
	}

	return BuildPath();
}

void CMap2D::SetDiagonalMovement(const bool bEnable)
{
	m_nrOfDirections = (bEnable) ? 8 : 4;
}

/**
 @brief Render a tile at a position based on its tile index
 @param iRow A const int variable containing the row index of the tile
 @param iCol A const int variable containing the column index of the tile
 */
void CMap2D::RenderTile(const unsigned int uiRow, const unsigned int uiCol)
{
	if ((arrMapInfo[uiCurLevel][uiRow][uiCol].value > 0) && 
		(arrMapInfo[uiCurLevel][uiRow][uiCol].value < 1400))
	{
		//if (arrMapInfo[uiCurLevel][uiRow][uiCol].value < 3)
		glBindTexture(GL_TEXTURE_2D, MapOfTextureIDs.at(arrMapInfo[uiCurLevel][uiRow][uiCol].value));

		glBindVertexArray(VAO);
		//CS: Render the tile
		quadMesh->Render();
		glBindVertexArray(0);
	}
}

std::vector<glm::vec2> CMap2D::BuildPath() const
{
	std::vector<glm::vec2> path;
	auto currentPos = m_targetPos;
	auto currentIndex = ConvertTo1D(currentPos);

	while (!(m_cameFromList[currentIndex].parent == currentPos))
	{
		path.push_back(currentPos);
		currentPos = m_cameFromList[currentIndex].parent;
		currentIndex = ConvertTo1D(currentPos);
	}

	// If the path has only 1 entry, then it is the the target position
	if (path.size() == 1)
	{
		// if m_startPos is next to m_targetPos, then having 1 path point is OK
		if (m_nrOfDirections == 4)
		{
			if (abs(m_targetPos.y - m_startPos.y) + abs(m_targetPos.x - m_startPos.x) > 1)
				path.clear();
		}
		else
		{
			if (abs(m_targetPos.y - m_startPos.y) + abs(m_targetPos.x - m_startPos.x) > 2)
				path.clear();
			else if (abs(m_targetPos.y - m_startPos.y) + abs(m_targetPos.x - m_startPos.x) > 1)
				path.clear();
		}
	}
	else
		std::reverse(path.begin(), path.end());

	return path;
}

bool CMap2D::isValid(const glm::vec2& pos) const
{
	//return (pos.x >= 0) && (pos.x < m_dimensions.x) &&
	//	(pos.y >= 0) && (pos.y < m_dimensions.y);
	return (pos.x >= 0) && (pos.x < cSettings->NUM_TILES_XAXIS) &&
		(pos.y >= 0) && (pos.y < cSettings->NUM_TILES_YAXIS);
}

bool CMap2D::isBlocked(const unsigned int uiRow, const unsigned int uiCol, const bool bInvert) const
{
	if (bInvert == true)
	{
		if (((arrMapInfo[uiCurLevel][cSettings->NUM_TILES_YAXIS - uiRow - 1][uiCol].value >= 600) &&
			(arrMapInfo[uiCurLevel][cSettings->NUM_TILES_YAXIS - uiRow - 1][uiCol].value < 1400)) &&
			((arrMapInfo[uiCurLevel][cSettings->NUM_TILES_YAXIS - uiRow - 1][uiCol + 1].value >= 600) &&
				(arrMapInfo[uiCurLevel][cSettings->NUM_TILES_YAXIS - uiRow - 1][uiCol + 1].value < 1400)))
			return true;
		else
			return false;
	}
	else
	{
		if (((arrMapInfo[uiCurLevel][uiRow][uiCol].value >= 600) &&
			(arrMapInfo[uiCurLevel][uiRow][uiCol].value < 1400)) &&
			((arrMapInfo[uiCurLevel][uiRow][uiCol + 1].value >= 600) &&
				(arrMapInfo[uiCurLevel][uiRow][uiCol + 1].value < 1400)))
			return true;
		else
			return false;
	}
}

int CMap2D::ConvertTo1D(const glm::vec2& pos) const
{
	//return (pos.y * m_dimensions.x) + pos.x;
	return (pos.y * cSettings->NUM_TILES_XAXIS) + pos.x;
}

bool CMap2D::DeleteAStarLists(void)
{
	// Delete m_openList
	while (m_openList.size() != 0)
		m_openList.pop();
	// Delete m_cameFromList
	m_cameFromList.clear();
	// Delete m_closedList
	m_closedList.clear();

	return true;
}

bool CMap2D::ResetAStarLists(void)
{
	// Delete m_openList
	while (m_openList.size() != 0)
		m_openList.pop();
	// Reset m_cameFromList
	for (int i = 0; i < m_cameFromList.size(); i++)
	{
		m_cameFromList[i].pos = glm::vec2(0, 0);
		m_cameFromList[i].parent = glm::vec2(0, 0);
		m_cameFromList[i].f = 0;
		m_cameFromList[i].g = 0;
		m_cameFromList[i].h = 0;
	}
	// Reset m_closedList
	for (int i = 0; i < m_closedList.size(); i++)
	{
		m_closedList[i] = false;
	}

	return true;
}

/**
 @brief Print out the details about this class instance in the console
 */
void CMap2D::PrintSelf(void) const
{
	cout << endl << "AStar::PrintSelf()" << endl;

	for (unsigned uiLevel = 0; uiLevel < uiNumLevels; uiLevel++)
	{
		cout << "Level: " << uiLevel << endl;
		for (unsigned uiRow = 0; uiRow < cSettings->NUM_TILES_YAXIS; uiRow++)
		{
			for (unsigned uiCol = 0; uiCol < cSettings->NUM_TILES_XAXIS; uiCol++)
			{
				cout.fill('0');
				cout.width(3);
				cout << arrMapInfo[uiLevel][uiRow][uiCol].value;
				if (uiCol != cSettings->NUM_TILES_XAXIS - 1)
					cout << ", ";
				else
					cout << endl;
			}
		}
	}

	cout << "m_openList: " << m_openList.size() << endl;
	cout << "m_cameFromList: " << m_cameFromList.size() << endl;
	cout << "m_closedList: " << m_closedList.size() << endl;

	cout << "===== AStar::PrintSelf() =====" << endl;
}

unsigned int heuristic::manhattan(const glm::vec2& v1, const glm::vec2& v2, int weight)
{
	glm::vec2 delta = v2 - v1;
	return static_cast<unsigned int>(weight * (delta.x + delta.y));
}

unsigned int heuristic::euclidean(const glm::vec2& v1, const glm::vec2& v2, int weight)
{
	glm::vec2 delta = v2 - v1;
	return static_cast<unsigned int>(weight * sqrt((delta.x * delta.x) + (delta.y * delta.y)));
}
