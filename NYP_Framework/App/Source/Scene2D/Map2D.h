/**
 Map2D
 @brief A class which manages the map in the game
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include SingletonTemplate
#include "DesignPatterns\SingletonTemplate.h"

// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include the RapidCSV
#include "System/rapidcsv.h"
// Include map storage
#include <map>

// Include Settings
#include "GameControl\Settings.h"

// Include Entity2D
#include "Primitives/Entity2D.h"

// Include files for AStar
#include <queue>
#include <functional>

// Include camera
#include "Primitives/Camera2D.h"

// A structure storing information about Map Sizes
struct MapSize {
	unsigned int uiRowSize;
	unsigned int uiColSize;
};

// A structure storing information about a map grid
// It includes data to be used for A* Path Finding
struct Grid {
	unsigned int value;

	Grid() 
		: value(0), pos(0, 0), parent(-1, -1), f(0), g(0), h(0) {}
	Grid(	const glm::vec2& pos, unsigned int f) 
		: value(0), pos(pos), parent(-1, 1), f(f), g(0), h(0) {}
	Grid(	const glm::vec2& pos, const glm::vec2& parent, 
			unsigned int f, unsigned int g, unsigned int h) 
		: value(0), pos(pos), parent(parent), f(f), g(g), h(h) {}

	glm::vec2 pos;
	glm::vec2 parent;
	unsigned int f;
	unsigned int g;
	unsigned int h;
};

using HeuristicFunction = std::function<unsigned int(const glm::vec2&, const glm::vec2&, int)>;
// Reverse std::priority_queue to get the smallest element on top
inline bool operator< (const Grid& a, const Grid& b) { return b.f < a.f; }

namespace heuristic
{
	unsigned int manhattan(const glm::vec2& v1, const glm::vec2& v2, int weight);
	unsigned int euclidean(const glm::vec2& v1, const glm::vec2& v2, int weight);
}

class CMap2D : public CSingletonTemplate<CMap2D>, public CEntity2D
{
	friend CSingletonTemplate<CMap2D>;
public:
	// Init
	bool Init(	const unsigned int uiNumLevels = 1,
				const unsigned int uiNumRows = 24,
				const unsigned int uiNumCols = 32);

	// Update
	void Update(const double dElapsedTime);

	// PreRender
	void PreRender(void);

	// Render
	void Render(void);

	// PostRender
	void PostRender(void);

	//for tile index
	enum TILE_INDEX
	{
		// jungle planet
		POISON_SPROUT = 2,
		POISON_FOG = 3,
		POISON_EXPLOSION = 4,
		RIVER_WATER = 10,
		ENEMY_WAYPOINT_RIVER_WATER = 11,
		UNBLOOMED_BOUNCY_BLOOM = 20,
		BLOOMED_BOUNCY_BLOOM = 21,
		ROCK = 30,
		ROCK_VINE_LEFT = 31,
		VINE_CORNER_LEFT = 32,
		VINE_LEFT = 33,
		ROCK_VINE_RIGHT = 34,
		VINE_CORNER_RIGHT = 35,
		VINE_RIGHT = 36,
		MOVING_LEAF_ALT = 40,
		MOVING_LEAF_TRANSITIONER = 41,
		TELEPORTATION_RESIDUE = 50,
		DOWN_BUTTON_PRESSED = 51,
		UP_BUTTON_PRESSED = 52,
		LEFT_BUTTON_PRESSED = 53,
		RIGHT_BUTTON_PRESSED = 54,
		SHOOTING_POPUP = 55,
		DOWN_BUTTON_1 = 60,
		RIGHT_BUTTON_1 = 61,
		LEFT_BUTTON_1 = 62,
		DOWN_BUTTON_2 = 64,
		DOWN_BUTTON_3 = 68,
		IRONWOOD = 199,
		BURNABLE_BUSH = 600,
		BURNING_BUSH = 601,
		DISSOLVING_BUSH = 602,
		GRASS_GROUND = 610,
		GRASS_GROUND_L = 611,
		GRASS_GROUND_R = 612,
		MOVING_LEAF_SOLID = 620,
		LEAF_SOLID = 621,
		IRONWOOD_TREE_TRUNK = 622,
		STONE_GROUND = 630,
		STONE_GROUND_GRASS = 631,
		STONE_GROUND_MOSS = 632,
		DOWN_BUTTON_1_BLOCKS = 640,
		RIGHT_BUTTON_1_BLOCKS = 641,
		LEFT_BUTTON_1_BLOCKS = 642,
		DOWN_BUTTON_2_BLOCKS = 644,
		DOWN_BUTTON_3_BLOCKS = 648,

		// terrestrial planet
		YELLOW_TILE_HOLLOW = 201,
		RED_TILE_HOLLOW = 202,
		GREEN_TILE_HOLLOW = 203,
		BLUE_TILE_HOLLOW = 204,
		BROWN_TILE_HOLLOW = 205,
		ROPE_POST_COILED = 210,
		ROPE_POST_UNCOILED_LEFT = 211,
		ROPE_CORNER_LEFT = 212,
		ROPE_LENGTH_LEFT = 213,
		ROPE_POST_UNCOILED_RIGHT = 214,
		ROPE_CORNER_RIGHT = 215,
		ROPE_LENGTH_RIGHT = 216,
		YELLOW_ORB = 221,
		RED_ORB = 222,
		GREEN_ORB = 223,
		BLUE_ORB = 224,
		BLACK_FLAG = 230,
		RED_FLAG = 231,
		SPIKES_UP = 240,
		SPIKES_LEFT = 241,
		SPIKES_DOWN = 242,
		SPIKES_RIGHT = 243,
		EXPLOSION = 244,
		ALARM_BOX = 250,
		ALARM_LIGHT_OFF = 251,
		ALARM_LIGHT_ON = 252,
		ANTIDOTE_PILL = 260,
		SWITCH_UP_OFF = 270,
		SWITCH_UP_ON = 271,
		SWITCH_LEFT_OFF = 272,
		SWITCH_LEFT_ON = 273,
		SWITCH_RIGHT_OFF = 276,
		SWITCH_RIGHT_ON = 277,
		KEY_PURPLE = 280,
		KEY_CYAN = 281,
		EXIT_DOOR = 299,
		TUT_START = 370,
		TUT_YELLOW_WALL = 371,
		TUT_ROPE = 372,
		TUT_TOXICITY_TOP_L = 373,
		TUT_TOXICITY_BOT_R = 374,
		TUT_COLOUR_CHANGE_DANGER = 375,
		TUT_RESOURCE = 376,
		TUT_SHOOTING = 377,
		TUT_LOCK = 378,
		TUT_SWITCH = 379,
		TUT_SPIKES = 380,
		TUT_COLOUR_CHANGE_COOLDOWN_TOP_L = 381,
		TUT_COLOUR_CHANGE_COOLDOWN_BOT_R = 382,
		TUT_ALARM_TOP_L = 383,
		TUT_ALARM_BOT_R = 384,
		ENERGY_QUARTZ = 399,
		GROUND_TILE_SOLID = 800,
		YELLOW_TILE_SOLID = 801,
		RED_TILE_SOLID = 802,
		GREEN_TILE_SOLID = 803,
		BLUE_TILE_SOLID = 804,
		BROWN_TILE_SOLID = 805,
		GROUND_WIRE_CORNER_BOT_L = 810,
		GROUND_WIRE_CORNER_BOT_R = 811,
		GROUND_WIRE_CORNER_TOP_L = 812,
		GROUND_WIRE_CORNER_TOP_R = 813,
		GROUND_WIRE_LENGTH_HOR = 814,
		GROUND_WIRE_LENGTH_VER = 815,
		DOOR_LOCKED_PURPLE = 820,
		DOOR_LOCKED_CYAN = 821,

		// snow planet
		SIGNINTRO1 = 499,
		SIGNINTRO2 = 498,
		SIGNINTRO3 = 497,
		SIGNINTRO4 = 496,
		SIGNINTRO5 = 495,
		SIGNTUT1 = 494,
		SIGNTUT2 = 493,
		SIGNTUT3 = 492,
		SIGNTUT4 = 491,
		SIGNTUT5 = 490,
		SIGNTUT6 = 489,
		FUR = 500,
		FUR_COAT = 501,
		SHIELD_POWERUP=502,
		BERSERK_POWERUP =503,
		FREEZE_POWERUP =504,
		ROPE_TOP_ICE=505,
		ROPE_MIDDLE_ICE=506,
		ROPE_BOTTOM_ICE=507,
		WATER_TOP = 595,
		WATER=596,
		ICE_CRYSTAL=599,
		TOP_SNOW=1000,
		LEFT_SNOW=1001,
		RIGHT_SNOW=1002,
		BOTTOM_SNOW=1003,
		NO_SNOW=1004,
		ICE=1100,

		NUM_TILES
	};

	// Set the specifications of the map
	void SetNumTiles(const CSettings::AXIS sAxis, const unsigned int uiValue);
	void SetNumSteps(const CSettings::AXIS sAxis, const unsigned int uiValue);

	// Set the value at certain indices in the arrMapInfo
	void SetMapInfo(const unsigned int uiRow, const unsigned int uiCol, const int iValue, const bool bInvert = true);

	// Get the value at certain indices in the arrMapInfo
	int GetMapInfo(const unsigned int uiRow, const unsigned int uiCol, const bool bInvert = true) const;

	//find position by tile index
	glm::vec2 CMap2D::GetTilePosition(const int tileIndex, const bool bInvert = true);

	// Load a map
	bool LoadMap(string filename, const unsigned int uiLevel = 0);

	// Get the number of rows and columns in the map
	int GetNumRows();
	int GetNumCols();

	// Find all tiles of a certain ID
	vector<glm::vec2> FindAllTiles(unsigned int textureID, int lowerMapRow = -1, int upperMapRow = -1, int leftMapCol = 0, int rightMapCol = -1);

	// Replace certain tiles
	bool ReplaceTiles(unsigned int currTextureID, unsigned int newTextureID, int lowerMapRow = -1, int upperMapRow = -1, int leftMapCol = 0, int rightMapCol = -1);

	// Save a tilemap
	bool SaveMap(string filename, const unsigned int uiLevel = 0);

	// Find the indices of a certain value in arrMapInfo
	bool FindValue(const int iValue, unsigned int& uirRow, unsigned int& uirCol, const bool bInvert = true);

	// Find the indices of the first ground tile in arrMapInfo
	unsigned int FindGround(unsigned int startRow, unsigned int mapCol, const bool bInvert = true);

	// Set current level
	void SetCurrentLevel(unsigned int uiCurLevel);
	// Get current level
	unsigned int GetCurrentLevel(void) const;

	// For AStar PathFinding
	std::vector<glm::vec2> PathFind(const glm::vec2& startPos, const glm::vec2& targetPos, HeuristicFunction heuristicFunc, int weight = 1);
	// Set if AStar PathFinding will consider diagonal movements
	void SetDiagonalMovement(const bool bEnable);
	// Print out details about this class instance in the console window
	void PrintSelf(void) const;

protected:
	// The variable containing the rapidcsv::Document
	// We will load the CSV file's content into this Document
	rapidcsv::Document doc;

	// A 3-D array which stores the values of the tile map
	Grid*** arrMapInfo;

	// The current level
	unsigned int uiCurLevel;
	// The number of levels
	unsigned int uiNumLevels;
	
	// The handler containing the instance of the camera
	Camera2D* camera2D;

	// A 1-D array which stores the map sizes for each level
	MapSize* arrMapSizes;

	// Map containing texture IDs
	map<int, int> MapOfTextureIDs;

	//CS: The quadMesh for drawing the tiles
	CMesh* quadMesh;

	// Constructor
	CMap2D(void);

	// Destructor
	virtual ~CMap2D(void);

	// Render a tile
	void RenderTile(const unsigned int uiRow, const unsigned int uiCol);

	// For A-Star PathFinding
	// Build a path from m_cameFromList after calling PathFind()
	std::vector<glm::vec2> BuildPath() const;
	// Check if a grid is valid
	bool isValid(const glm::vec2& pos) const;
	// Check if a grid is blocked
	bool isBlocked(const unsigned int uiRow,
		const unsigned int uiCol,
		const bool bInvert = true) const;
	// Convert a position to a 1D position in the array
	int ConvertTo1D(const glm::vec2& pos) const;

	// Delete AStar lists
	bool DeleteAStarLists(void);
	// Reset AStar lists
	bool ResetAStarLists(void);

	int m_weight;
	//int m_size;
	unsigned int m_nrOfDirections;
	//glm::vec2 m_dimensions;
	glm::vec2 m_startPos;
	glm::vec2 m_targetPos;

	std::priority_queue<Grid> m_openList;
	std::vector<bool> m_closedList;
	std::vector<Grid> m_cameFromList;
	//std::vector<int> m_grid;
	std::vector<glm::vec2> m_directions;
	HeuristicFunction m_heuristic;

	// my function disappeared what the fuck
	bool findTilesForShip(void);
};

