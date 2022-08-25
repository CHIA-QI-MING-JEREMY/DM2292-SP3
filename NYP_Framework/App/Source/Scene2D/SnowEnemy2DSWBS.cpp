/**
 CStnEnemy2D
 @brief A class which represents the enemy object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "SnowEnemy2DSWBS.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"
// Include Mesh Builder
#include "Primitives/MeshBuilder.h"

// Include GLEW
#include <GL/glew.h>

// Include ImageLoader
#include "System\ImageLoader.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "Map2D.h"
// Include math.h
#include <math.h>

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
SnowEnemy2DSWBS::SnowEnemy2DSWBS(void)
	: bIsActive(false)
	, cMap2D(NULL)
	, cSettings(NULL)
	, cPlayer2D(NULL)
	, sCurrentFSM(FSM::IDLE)
	, iFSMCounter(0)
	, quadMesh(NULL)
	, camera2D(NULL)
	, animatedSprites(NULL)
{
	transform = glm::mat4(1.0f);	// make sure to initialize matrix to identity matrix first

	// Initialise vecIndex
	vec2Index = glm::i32vec2(0);

	// Initialise vecNumMicroSteps
	i32vec2NumMicroSteps = glm::i32vec2(0);

	// Initialise vec2UVCoordinate
	vec2UVCoordinate = glm::vec2(0.0f);

	vec2Destination = glm::vec2(0, 0);	// Initialise the iDestination
	vec2Direction = glm::vec2(0, 0);		// Initialise the iDirection
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
SnowEnemy2DSWBS::~SnowEnemy2DSWBS(void)
{
	// Delete the quadMesh
	if (quadMesh)
	{
		delete quadMesh;
		quadMesh = NULL;
	}

	// We won't delete this since it was created elsewhere
	cPlayer2D = NULL;

	// We won't delete this since it was created elsewhere
	cMap2D = NULL;

	// Delete the CAnimationSprites
	if (animatedSprites)
	{
		delete animatedSprites;
		animatedSprites = NULL;
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

/**
  @brief Initialise this instance
  */
bool SnowEnemy2DSWBS::Init(void)
{
	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();
	// Get the handler to the Camera2D instance
	camera2D = Camera2D::GetInstance();

	// Get the handler to the CSoundController instance
	cSoundController = CSoundController::GetInstance();

	// Get the handler to the CMap2D instance
	cMap2D = CMap2D::GetInstance();
	// Find the indices for the player in arrMapInfo, and assign it to CStnEnemy2D
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	if (cMap2D->FindValue(2002, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the enemy, so quit this game

	// Erase the value of the player in the arrMapInfo
	cMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	i32vec2NumMicroSteps = glm::i32vec2(0, 0);

	// Create and initialise the CPlayer2D
	cPlayer2D = CPlayer2D::GetInstance();

	cInventoryManagerPlanet = CInventoryManagerPlanet::GetInstance();

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//CS: Create the Quad Mesh using the mesh builder
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

	// Load the enemy2D texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/SnowPlanet/wolfboss.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/SnowPlanet/wolfboss.png" << endl;
		return false;
	}

	//CS: Create the animated spirte and setup the animation
	animatedSprites = CMeshBuilder::GenerateSpriteAnimation(10, 4,
		cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	//^ loads a spirte sheet with 3 by 3 diff images, all of equal size and positioning
	animatedSprites->AddAnimation("idleR", 0, 3); //3 images for animation, index 0 to 2
	animatedSprites->AddAnimation("idleL", 4, 7);
	animatedSprites->AddAnimation("biteR", 8,11);
	animatedSprites->AddAnimation("biteL", 12, 15);
	animatedSprites->AddAnimation("walkR", 16, 19);
	animatedSprites->AddAnimation("walkL", 20, 23);
	animatedSprites->AddAnimation("shieldR", 24, 27);
	animatedSprites->AddAnimation("shieldL", 28, 31);
	animatedSprites->AddAnimation("shootR", 32, 35);
	animatedSprites->AddAnimation("shootL", 36, 39);
	//CS: Play the "idle" animation as default
	animatedSprites->PlayAnimation("idleL", -1, 1.0f);
	//-1 --> repeats forever
	//		settng it to say 1 will cause it to only repeat 1 time
	//1.0f --> set time between frames as 1.0f
	//		increasing this number will cause the animation to slowdown

	//CS: Init the color to white
	runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);
	shieldActivated = false;

	// Set the Physics to fall status by default
	cPhysics2D.Init();
	cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);

	// If this class is initialised properly, then set the bIsActive to true
	bIsActive = true;

	//Construct 100 inactive ammo and add into ammoList
	for (int i = 0; i < 100; ++i)
	{
		CSEAmmo* cEnemyAmmo2D = new CSEAmmo();
		cEnemyAmmo2D->SetShader("Shader2D");
		ammoList.push_back(cEnemyAmmo2D);
	}

	type = BOSS;
	shootingDirection = LEFT; //setting direction for ammo shooting
	maxHealth = health = 50; //takes 10 hits to kill
	shieldTimer = 2.0f;
	shieldCount = 1;
	healCount = 1;
	healFearCount = 1;
	boolTimer = 1.f;
	attackHit = false;


	//if (vec2Index.x == 11 && vec2Index.y == 1) {
	//	pathway.push_back(glm::vec2(11, 1));
	//	pathway.push_back(glm::vec2(18, 2));
	//	pathway.push_back(glm::vec2(14, 5));
	//	fearpathway = (glm::vec2(20, 1));
	//}

		// sets waypoints based on the level
	//TUTORIAL
	if (cMap2D->GetCurrentLevel() == 0)
	{
		//// sets waypoints based on the enemy spawn location
		//if (vec2Index == glm::vec2(3, 1))
		//{
		//	pathway.push_back(glm::vec2(3, 1));
		//	//pathway = ConstructWaypointVector(pathway, 400, 1);
		//	fearpathway = glm::vec2(8,1);
		//}
		//else if (vec2Index == glm::vec2(17, 15))
		//{
		//	pathway.push_back(glm::vec2(17, 15));
		//	pathway = ConstructWaypointVector(pathway, 401, 1);
		//	fearpathway = glm::vec2(20, 11);
		//}
		//else if (vec2Index == glm::vec2(17, 6))
		//{
		//	pathway.push_back(glm::vec2(17, 6));
		//	pathway = ConstructWaypointVector(pathway, 402, 1);
		//	fearpathway = glm::vec2(20, 1);
		//}
	}
	currentPathwayCounter = 0;
	maxPathwayCounter = pathway.size();

	return true;
}

/**
 @brief Update this instance
 */
void SnowEnemy2DSWBS::Update(const double dElapsedTime)
{
	if (!bIsActive)
		return;

	//prevent health from going over maximum
	if (health > maxHealth)
	{
		health = maxHealth;
	}
	if (attackHit) {
		boolTimer -= dElapsedTime;
	}
	if (boolTimer <= 0.f) {
		attackHit = false;
		boolTimer = 1.f;
	}
	// just for switching between states --> keep simple
	//action done under interaction with player, update position, update direction, etc
	switch (sCurrentFSM)
	{
	case IDLE:
		if (attackHit) {
			if (vec2Direction.x > 0) {
				runtimeColour = glm::vec4(0.0, 0.0, 0.0, 1.0);
				animatedSprites->PlayAnimation("biteR", -1, 1.0f);
			}
			else {
				runtimeColour = glm::vec4(0.0, 0.0, 0.0, 1.0);
				animatedSprites->PlayAnimation("biteL", -1, 1.0f);
			}
		}
		else {
			if (vec2Direction.x > 0) {
				runtimeColour = glm::vec4(0.0, 0.0, 0.0, 1.0);
				animatedSprites->PlayAnimation("idleR", -1, 1.0f);
			}
			else if (vec2Direction.x < 0) {
				runtimeColour = glm::vec4(0.0, 0.0, 0.0, 1.0);
				animatedSprites->PlayAnimation("idleL", -1, 1.0f);
			}
		}
		if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = PATROL;
			iFSMCounter = 0;
			cout << "Switching to Patrol State" << endl;
			break;
		}
		if (health <= 40 && health >35 && shieldCount > 0) {
			sCurrentFSM = SHIELD;
			iFSMCounter = 0;
			cout << "Switching to Shield State" << endl;
			break;
		}
		if (health <= 10 && health >5) {
			sCurrentFSM = FEAR;
			iFSMCounter = 0;
			cout << "Switching to Fear State" << endl;
			break;
		}
		if (health <= 30 && health > 25 && healCount > 0) {
			sCurrentFSM = HEAL;
			iFSMCounter = 0;
			cout << "Switching to Heal State" << endl;
			break;
		}
		iFSMCounter++;
		break;
	case PATROL:
		if (vec2Direction.x > 0) {
			runtimeColour = glm::vec4(0.0, 0.0, 0.0, 1.0);
			animatedSprites->PlayAnimation("walkR", -1, 1.0f);
		}
		else if (vec2Direction.x < 0) {
			runtimeColour = glm::vec4(0.0, 0.0, 0.0, 1.0);
			animatedSprites->PlayAnimation("walkL", -1, 1.0f);
		}
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 5.0f)
		{
			sCurrentFSM = ATTACK;
			iFSMCounter = 0;
			cout << "Switching to Attack State" << endl;
			break;
		}
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 8.0f && cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) > 5.0f)
		{
			sCurrentFSM = SHOOT;
			iFSMCounter = 0;
			cout << "Switching to Shoot State" << endl;
			break;
		}
		if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = IDLE;
			iFSMCounter = 0;
			cout << "Switching to Idle State" << endl;
			break;
		}
		if (health <= 40 && health > 35 && shieldCount > 0) {
			sCurrentFSM = SHIELD;
			iFSMCounter = 0;
			cout << "Switching to Shield State" << endl;
			break;
		}
		if (health <= 10 && health > 5) {
			sCurrentFSM = FEAR;
			iFSMCounter = 0;
			cout << "Switching to Fear State" << endl;
			break;
		}
		if (health <= 30 && health > 25 && healCount > 0) {
			sCurrentFSM = HEAL;
			iFSMCounter = 0;
			cout << "Switching to Heal State" << endl;
			break;
		}
		else {
			if (vec2Index == pathway[currentPathwayCounter]) {
				currentPathwayCounter++;
			}
			if (currentPathwayCounter < maxPathwayCounter) {
				auto path = cMap2D->PathFind(vec2Index,
					pathway[currentPathwayCounter],
					heuristic::manhattan,
					10);
				//Calculate new destination
				bool bFirstPosition = true;
				for (const auto& coord : path) {
					//std::cout << coord.x << "," << coord.y << "\n";
					if (bFirstPosition == true) {
						//Set a destination
						vec2Destination = coord;
						//Calc the direction between enemy2D and this destination
						vec2Direction = vec2Destination - vec2Index;
						bFirstPosition = false;
					}
					else {
						if ((coord - vec2Destination) == vec2Direction) {
							//Set a destination
							vec2Destination = coord;
						}
						else
							break;
					}
				}
			}
			else {
				currentPathwayCounter = 0;
			}
			// Patrol around
			// Update the Enemy2D's position for patrol
			UpdatePosition();
		}
		iFSMCounter++;
		break;
	case ATTACK:
		if (vec2Direction.x > 0) {
			runtimeColour = glm::vec4(0.0, 0.0, 0.0, 1.0);
			animatedSprites->PlayAnimation("walkR", -1, 1.0f);
		}
		else if (vec2Direction.x < 0) {
			runtimeColour = glm::vec4(0.0, 0.0, 0.0, 1.0);
			animatedSprites->PlayAnimation("walkL", -1, 1.0f);
		}
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 5.0f)
		{
			auto path = cMap2D->PathFind(vec2Index,
				cPlayer2D->vec2Index,
				heuristic::manhattan,
				10);
			//	cout << "=== Printing out the path ===" << endl;
				//Calculate new destination
			bool bFirstPosition = true;
			for (const auto& coord : path) {
				//std::cout << coord.x << "," << coord.y << "\n";
				if (bFirstPosition == true) {
					//Set a destination
					vec2Destination = coord;
					//Calc the direction between enemy2D and this destination
					vec2Direction = vec2Destination - vec2Index;
					bFirstPosition = false;
				}
				else {
					if ((coord - vec2Destination) == vec2Direction) {
						//Set a destination
						vec2Destination = coord;
					}
					else
						break;
				}
			}
			UpdatePosition();
		}
		else {
			if (iFSMCounter > iMaxFSMCounter)
			{
				sCurrentFSM = PATROL;
				iFSMCounter = 0;
				cout << "Switching to Patrol State" << endl;
				break;
			}
			iFSMCounter++;
		}
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 8.0f && cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) > 5.0f)
		{
			sCurrentFSM = SHOOT;
			iFSMCounter = 0;
			cout << "Switching to Shoot State" << endl;
			break;
		}
		if (health <= 40 && health > 35 && shieldCount > 0) {
			sCurrentFSM = SHIELD;
			iFSMCounter = 0;
			cout << "Switching to Shield State" << endl;
			break;
		}
		if (health <= 10 && health > 5) {
			sCurrentFSM = FEAR;
			iFSMCounter = 0;
			cout << "Switching to Fear State" << endl;
			break;
		}
		if (health <= 30 && health > 25 && healCount > 0) {
			sCurrentFSM = HEAL;
			iFSMCounter = 0;
			cout << "Switching to Heal State" << endl;
			break;
		}
		break;
	case SHIELD:
		if (vec2Direction.x > 0) {
			runtimeColour = glm::vec4(0.0, 0.0, 0.0, 1.0);
			animatedSprites->PlayAnimation("shieldR", -1, 1.0f);
		}
		else if (vec2Direction.x < 0) {
			runtimeColour = glm::vec4(0.0, 0.0, 0.0, 1.0);
			animatedSprites->PlayAnimation("shieldL", -1, 1.0f);
		}
		shieldActivated=true;
		shieldTimer -= dElapsedTime;
		if (shieldTimer <= 0.f) {
			shieldCount -= 1;
			sCurrentFSM = PATROL;
			iFSMCounter = 0;
			shieldTimer = 0.f;
			shieldActivated=false;
			cout << "Switching to Patrol State" << endl;
			break;
		}
		iFSMCounter++;
		break;

	case FEAR:
		if (vec2Direction.x > 0) {
			runtimeColour = glm::vec4(0.0, 0.0, 0.0, 1.0);
			animatedSprites->PlayAnimation("walkR", -1, 1.0f);
		}
		else if (vec2Direction.x < 0) {
			runtimeColour = glm::vec4(0.0, 0.0, 0.0, 1.0);
			animatedSprites->PlayAnimation("walkL", -1, 1.0f);
		}
		if (health<=10 && health>5) {
			auto path = cMap2D->PathFind(vec2Index,
				fearpathway,
				heuristic::manhattan,
				10);
			//Calculate new destination
			bool bFirstPosition = true;
			for (const auto& coord : path) {
				//std::cout << coord.x << "," << coord.y << "\n";
				if (bFirstPosition == true) {
					//Set a destination
					vec2Destination = coord;
					//Calc the direction between enemy2D and this destination
					vec2Direction = vec2Destination - vec2Index;
					bFirstPosition = false;
				}
				else {
					if ((coord - vec2Destination) == vec2Direction) {
						//Set a destination
						vec2Destination = coord;
					}
					else
						break;
				}
			}
			UpdatePosition();
		}
		if (vec2Index == fearpathway) {
			sCurrentFSM = FEARIDLE;
			iFSMCounter = 0;
			cout << "Switching to Fear Idle State" << endl;
			break;
		}
		iFSMCounter++;
		break;
	case FEARIDLE:
		if (vec2Direction.x > 0) {
			runtimeColour = glm::vec4(0.0, 0.0, 0.0, 1.0);
			animatedSprites->PlayAnimation("idleR", -1, 1.0f);
		}
		else if (vec2Direction.x < 0) {
			runtimeColour = glm::vec4(0.0, 0.0, 0.0, 1.0);
			animatedSprites->PlayAnimation("idleL", -1, 1.0f);
		}
		if (health <= 5 && healFearCount > 0) {
			sCurrentFSM = FEARHEAL;
			iFSMCounter = 0;
			cout << "Switching to Heal Fear State" << endl;
			break;
		}
		iFSMCounter++;
		break;
	case HEAL:
		if (vec2Direction.x > 0) {
			if (healCount > 0) {
				runtimeColour = glm::vec4(0.f, 1.0f, 0.f, 1.f);
			}
			animatedSprites->PlayAnimation("idleR", -1, 1.0f);
		}
		else if (vec2Direction.x < 0) {
			if (healCount > 0) {
				runtimeColour = glm::vec4(0.f, 1.0f, 0.f, 1.f);
			}
			animatedSprites->PlayAnimation("idleL", -1, 1.0f);
		}
		if (healCount >0) {
			health += 10;
			healCount -= 1;
		}
		if (healCount <=0 && iFSMCounter > iMaxFSMCounter) {
			sCurrentFSM = PATROL;
			iFSMCounter = 0;
			cout << "ATTACK : Reset counter: " << iFSMCounter << endl;
		}
		iFSMCounter++;
		break;
	case FEARHEAL:
		if (vec2Direction.x > 0) {
			if (healFearCount > 0) {
				runtimeColour = glm::vec4(1.f, 0.5f, 0.5f, 1.f);
			}
			animatedSprites->PlayAnimation("idleR", -1, 1.0f);
		}
		else if (vec2Direction.x < 0) {
			if (healFearCount > 0) {
				runtimeColour = glm::vec4(1.f, 0.5f, 0.5f, 1.f);
			}
			animatedSprites->PlayAnimation("idleL", -1, 1.0f);
		}
		if (healFearCount > 0) {
			health =maxHealth;
			healFearCount -= 1;
			healCount += 1;
			shieldCount += 1;
			shieldTimer = 2.0f;
		}
		if (healFearCount <= 0 && iFSMCounter > iMaxFSMCounter) {
			sCurrentFSM = PATROL;
			iFSMCounter = 0;
			cout << "ATTACK : Reset counter: " << iFSMCounter << endl;
		}
		iFSMCounter++;
		break;
	case SHOOT:
		if (vec2Direction.x > 0)
		{
			runtimeColour = glm::vec4(0.f, 0.f, 0.f, 1.f);
			animatedSprites->PlayAnimation("shootR", 0, 1.f);

			shootingDirection = RIGHT;
		}
		else if (vec2Direction.x < 0)
		{
			runtimeColour = glm::vec4(0.f, 0.f, 0.f, 1.f);
			animatedSprites->PlayAnimation("shootL", 0, 1.f);

			shootingDirection = LEFT;
		}
		if (shootTimer <= 0)
		{
			// Shoot enemy ammo!
			//shoot ammo in accordance to the direction enemy is facing
			CSEAmmo* ammo = FetchAmmo();
			ammo->setActive(true);
			ammo->setPath(vec2Index.x, vec2Index.y, shootingDirection);
			cout << "Bam!" << shootingDirection << endl;

			shootTimer = shootInterval;
		}
		else
		{
			shootTimer -= dElapsedTime;
		}
		UpdateDirection();
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 5.0f)
		{
			sCurrentFSM = ATTACK;
			iFSMCounter = 0;
			cout << "Switching to Attack State" << endl;
			break;
		}
		if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = IDLE;
			iFSMCounter = 0;
			cout << "Switching to Idle State" << endl;
			break;
		}
		if (health <= 40 && health > 35 && shieldCount > 0) {
			sCurrentFSM = SHIELD;
			iFSMCounter = 0;
			cout << "Switching to Shield State" << endl;
			break;
		}
		if (health <= 10 && health > 5) {
			sCurrentFSM = FEAR;
			iFSMCounter = 0;
			cout << "Switching to Fear State" << endl;
			break;
		}
		if (health <= 30 && health > 25 && healCount > 0) {
			sCurrentFSM = HEAL;
			iFSMCounter = 0;
			cout << "Switching to Heal State" << endl;
			break;
		}
		iFSMCounter++;
		break;
	default:
		break;
	}

	//ammo behaviour
	for (std::vector<CSEAmmo*>::iterator it = ammoList.begin(); it != ammoList.end(); ++it)
	{
		CSEAmmo* ammo = (CSEAmmo*)*it;
		if (ammo->getActive())
		{
			ammo->Update(dElapsedTime);
			if (ammo->LimitReached())
			{
				ammo->setActive(false);
			}
		}
	}

	// Update Jump or Fall
	UpdateJumpFall(dElapsedTime);

	// Interact with the Map
	InteractWithMap();


	//CS: Update the animated sprite
	//CS: Play the "left" animation
	animatedSprites->Update(dElapsedTime);

	// Update the UV Coordinates
	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false, i32vec2NumMicroSteps.x*cSettings->ENEMY_MICRO_STEP_XAXIS);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false, i32vec2NumMicroSteps.y*cSettings->ENEMY_MICRO_STEP_YAXIS);
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void SnowEnemy2DSWBS::PreRender(void)
{
	if (!bIsActive)
		return;

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);

	// Activate blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate the shader
	CShaderManager::GetInstance()->Use(sShaderName);
}

/**
 @brief Render this instance
 */
void SnowEnemy2DSWBS::Render(void)
{
	if (!bIsActive)
		return;

	glBindVertexArray(VAO);
	// get matrix's uniform location and set matrix
	unsigned int transformLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "transform");
	unsigned int colorLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "runtimeColour");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));


	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	glm::vec2 offset = glm::i32vec2(float(cSettings->NUM_TILES_XAXIS / 2.0f), float(cSettings->NUM_TILES_YAXIS / 2.0f));
	glm::vec2 cameraPos = camera2D->getPos();

	glm::vec2 IndexPos = vec2Index;

	glm::vec2 actualPos = IndexPos - cameraPos + offset;
	actualPos = cSettings->ConvertIndexToUVSpace(actualPos) * camera2D->getZoom();
	actualPos.x += i32vec2NumMicroSteps.x * cSettings->MICRO_STEP_XAXIS;
	actualPos.y += i32vec2NumMicroSteps.y * cSettings->MICRO_STEP_YAXIS;

	transform = glm::translate(transform, glm::vec3(actualPos.x, actualPos.y, 0.f));
	transform = glm::scale(transform, glm::vec3(camera2D->getZoom()));


	// Update the shaders with the latest transform
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glUniform4fv(colorLoc, 1, glm::value_ptr(runtimeColour));

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	// Get the texture to be rendered
	glBindTexture(GL_TEXTURE_2D, iTextureID);

	//CS: Render the animated Sprite
	animatedSprites->Render();

	glBindTexture(GL_TEXTURE_2D, 0);
	////render enemy ammo
	for (std::vector<CSEAmmo*>::iterator it = ammoList.begin(); it != ammoList.end(); ++it)
	{
		CSEAmmo* ammo = (CSEAmmo*)*it;
		if (ammo->getActive())
		{
			ammo->PreRender();
			ammo->Render();
			ammo->PostRender();
		}
	}

}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void SnowEnemy2DSWBS::PostRender(void)
{
	if (!bIsActive)
		return;

	// Disable blending
	glDisable(GL_BLEND);
}

/**
@brief Set the indices of the enemy2D
@param iIndex_XAxis A const int variable which stores the index in the x-axis
@param iIndex_YAxis A const int variable which stores the index in the y-axis
*/
void SnowEnemy2DSWBS::Seti32vec2Index(const int iIndex_XAxis, const int iIndex_YAxis)
{
	this->vec2Index.x = iIndex_XAxis;
	this->vec2Index.y = iIndex_YAxis;
}

/**
@brief Set the number of microsteps of the enemy2D
@param iNumMicroSteps_XAxis A const int variable storing the current microsteps in the X-axis
@param iNumMicroSteps_YAxis A const int variable storing the current microsteps in the Y-axis
*/
void SnowEnemy2DSWBS::Seti32vec2NumMicroSteps(const int iNumMicroSteps_XAxis, const int iNumMicroSteps_YAxis)
{
	this->i32vec2NumMicroSteps.x = iNumMicroSteps_XAxis;
	this->i32vec2NumMicroSteps.y = iNumMicroSteps_YAxis;
}

/**
 @brief Set the handle to cPlayer to this class instance
 @param cPlayer2D A CPlayer2D* variable which contains the pointer to the CPlayer2D instance
 */
void SnowEnemy2DSWBS::SetPlayer2D(CPlayer2D* cPlayer2D)
{
	this->cPlayer2D = cPlayer2D;

	// Update the enemy's direction
	UpdateDirection();
}

bool SnowEnemy2DSWBS::getShieldActivated()
{
	return shieldActivated;
}

void SnowEnemy2DSWBS::setShieldActivated(bool s)
{
	shieldActivated = s;
}



/**
 @brief Constraint the enemy2D's position within a boundary
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
void SnowEnemy2DSWBS::Constraint(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		if (vec2Index.x < 0)
		{
			vec2Index.x = 0;
			i32vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == RIGHT)
	{
		if (vec2Index.x >= (int)cSettings->NUM_TILES_XAXIS - 1)
		{
			vec2Index.x = ((int)cSettings->NUM_TILES_XAXIS) - 1;
			i32vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == UP)
	{
		if (vec2Index.y >= (int)cSettings->NUM_TILES_YAXIS - 1)
		{
			vec2Index.y = ((int)cSettings->NUM_TILES_YAXIS) - 1;
			i32vec2NumMicroSteps.y = 0;
		}
	}
	else if (eDirection == DOWN)
	{
		if (vec2Index.y < 0)
		{
			vec2Index.y = 0;
			i32vec2NumMicroSteps.y = 0;
		}
	}
	else
	{
		cout << "CStnEnemy2D::Constraint: Unknown direction." << endl;
	}
}

/**
 @brief Check if a position is possible to move into
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
bool SnowEnemy2DSWBS::CheckPosition(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		// If the new position is fully within a row, then check this row only
		if (i32vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 600)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 600) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 600))
			{
				return false;
			}
		}
	}
	else if (eDirection == RIGHT)
	{
		// If the new position is at the top row, then return true
		if (vec2Index.x >= cSettings->NUM_TILES_XAXIS - 1)
		{
			i32vec2NumMicroSteps.x = 0;
			return true;
		}

		// If the new position is fully within a row, then check this row only
		if (i32vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 600)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 600) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 600))
			{
				return false;
			}
		}
	}
	else if (eDirection == UP)
	{
		// If the new position is at the top row, then return true
		if (vec2Index.y >= cSettings->NUM_TILES_YAXIS - 1)
		{
			i32vec2NumMicroSteps.y = 0;
			return true;
		}

		// If the new position is fully within a column, then check this column only
		if (i32vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 600)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (i32vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 600) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 600))
			{
				return false;
			}
		}
	}
	else if (eDirection == DOWN)
	{
		// If the new position is fully within a column, then check this column only
		if (i32vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 600)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (i32vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 600) ||
				(cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 600))
			{
				return false;
			}
		}
	}
	else
	{
		cout << "CStnEnemy2D::CheckPosition: Unknown direction." << endl;
	}

	return true;
}

// Check if the enemy2D is in mid-air
bool SnowEnemy2DSWBS::IsMidAir(void)
{
	// if the player is at the bottom row, then he is not in mid-air for sure
	if (vec2Index.y == 0)
		return false;

	// Check if the tile below the player's current position is empty
	if ((i32vec2NumMicroSteps.x == 0) &&
		(cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) == 0))
	{
		return true;
	}

	return false;
}

// Update Jump or Fall
void SnowEnemy2DSWBS::UpdateJumpFall(const double dElapsedTime)
{
	if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::JUMP)
	{
		// Update the elapsed time to the physics engine
		cPhysics2D.SetTime((float)dElapsedTime);
		// Call the physics engine update method to calculate the final velocity and displacement
		cPhysics2D.Update();
		// Get the displacement from the physics engine
		glm::vec2 v2Displacement = cPhysics2D.GetDisplacement();

		// Store the current vec2Index.y
		int iIndex_YAxis_OLD = vec2Index.y;

		int iDisplacement_MicroSteps = (int)(v2Displacement.y / cSettings->ENEMY_MICRO_STEP_YAXIS); //DIsplacement divide by distance for 1 microstep
		if (vec2Index.y < (int)cSettings->NUM_TILES_YAXIS)
		{
			i32vec2NumMicroSteps.y += iDisplacement_MicroSteps;
			if (i32vec2NumMicroSteps.y > cSettings->ENEMY_NUM_STEPS_PER_TILE_YAXIS)
			{
				i32vec2NumMicroSteps.y -= cSettings->ENEMY_NUM_STEPS_PER_TILE_YAXIS;
				if (i32vec2NumMicroSteps.y < 0)
					i32vec2NumMicroSteps.y = 0;
				vec2Index.y++;
			}
		}

		// Constraint the player's position within the screen boundary
		Constraint(UP);

		// Iterate through all rows until the proposed row
		// Check if the player will hit a tile; stop jump if so.
		int iIndex_YAxis_Proposed = vec2Index.y;
		for (int i = iIndex_YAxis_OLD; i <= iIndex_YAxis_Proposed; i++)
		{
			// Change the player's index to the current i value
			vec2Index.y = i;
			// If the new position is not feasible, then revert to old position
			if (CheckPosition(UP) == false)
			{
				// Align with the row
				i32vec2NumMicroSteps.y = 0;
				// Set the Physics to fall status
				cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
				break;
			}
		}

		// If the player is still jumping and the initial velocity has reached zero or below zero, 
		// then it has reach the peak of its jump
		if ((cPhysics2D.GetStatus() == CPhysics2D::STATUS::JUMP) && (cPhysics2D.GetInitialVelocity().y <= 0.0f))
		{
			// Set status to fall
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}
	}
	else if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::FALL)
	{
		// Update the elapsed time to the physics engine
		cPhysics2D.SetTime((float)dElapsedTime);
		// Call the physics engine update method to calculate the final velocity and displacement
		cPhysics2D.Update();
		// Get the displacement from the physics engine
		glm::vec2 v2Displacement = cPhysics2D.GetDisplacement();

		// Store the current vec2Index.y
		int iIndex_YAxis_OLD = vec2Index.y;

		// Translate the displacement from pixels to indices
		int iDisplacement_MicroSteps = (int)(v2Displacement.y / cSettings->ENEMY_MICRO_STEP_YAXIS);

		if (vec2Index.y >= 0)
		{
			i32vec2NumMicroSteps.y -= fabs(iDisplacement_MicroSteps);
			if (i32vec2NumMicroSteps.y < 0)
			{
				i32vec2NumMicroSteps.y = ((int)cSettings->ENEMY_NUM_STEPS_PER_TILE_YAXIS) - 1;
				vec2Index.y--;
			}
		}

		// Constraint the player's position within the screen boundary
		Constraint(DOWN);

		// Iterate through all rows until the proposed row
		// Check if the player will hit a tile; stop fall if so.
		int iIndex_YAxis_Proposed = vec2Index.y;
		for (int i = iIndex_YAxis_OLD; i >= iIndex_YAxis_Proposed; i--)
		{
			// Change the player's index to the current i value
			vec2Index.y = i;
			// If the new position is not feasible, then revert to old position
			if (CheckPosition(DOWN) == false)
			{
				// Revert to the previous position
				if (i != iIndex_YAxis_OLD)
					vec2Index.y = i + 1;
				// Set the Physics to idle status
				cPhysics2D.SetStatus(CPhysics2D::STATUS::IDLE);
				i32vec2NumMicroSteps.y = 0;
				break;
			}
		}
	}
}

/**
 @brief Let enemy2D interact with the player.
 */
bool SnowEnemy2DSWBS::InteractWithPlayer(void)
{
	glm::i32vec2 i32vec2PlayerPos = cPlayer2D->vec2Index;
	
	// Check if the enemy2D is within 1.5 indices of the player2D
	if (((vec2Index.x >= i32vec2PlayerPos.x - 0.5) && 
		(vec2Index.x <= i32vec2PlayerPos.x + 0.5))
		&& 
		((vec2Index.y >= i32vec2PlayerPos.y - 0.5) &&
		(vec2Index.y <= i32vec2PlayerPos.y + 0.5)))
	{
		attackHit = true;
		cout << "Snow Gotcha!" << endl;
		// Since the player has been caught, then reset the FSM
		sCurrentFSM = IDLE;
		iFSMCounter = 0;
		if (cPlayer2D->getModeOfPlayer() != CPlayer2D::MODE::BERSERKSHIELD && cPlayer2D->getModeOfPlayer() != CPlayer2D::MODE::SHIELD) {
			cInventoryItemPlanet = cInventoryManagerPlanet->GetItem("Health");
			cInventoryItemPlanet->Remove(3);
		}
		return true;
	}
	return false;
}

//enemy interact with map
void SnowEnemy2DSWBS::InteractWithMap(void)
{
	switch (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x))
	{
	case 50: //purple spring, same function as the black and white spring 
	case 51: //black and white spring, launch the player repeatedly up into the sky at the rate of a double jump
		cPhysics2D.SetStatus(CPhysics2D::STATUS::JUMP);
		cPhysics2D.SetInitialVelocity(glm::vec2(0.0f, 4.f));
		break;
	case 53: //lava, same function as black and white lava
	case 54: //black and white lava, depletes health
		// Decrease the health by 2
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::BURNING); //play burning noise
		health--;
		break;
	case 55: //ice water, same function as black and white ice water
	case 56: //black and white ice water, restores health
	case 57: //ice water + enemy waypoint
	case 58: //ice water + enemy waypoint, BnW
		// Increase the health by 2
		runtimeColour = glm::vec4(0.0, 1.0, 0.0, 1.0); //green
		cSoundController->PlaySoundByID(CSoundController::SOUND_LIST::SPLASH); //play watching in water sound
		health++;
		break;
	case 99:
		break;
	default:
		break;
	}
}

/**
 @brief Update the enemy's direction.
 */
void SnowEnemy2DSWBS::UpdateDirection(void)
{
	// Set the destination to the player
	vec2Destination = cPlayer2D->vec2Index;

	// Calculate the direction between enemy2D and player2D
	vec2Direction = vec2Destination - vec2Index;

	// Calculate the distance between enemy2D and player2D
	float fDistance = cPhysics2D.CalculateDistance(vec2Index, vec2Destination);
	if (fDistance >= 0.01f)
	{
		// Calculate direction vector.
		// We need to round the numbers as it is easier to work with whole numbers for movements
		vec2Direction.x = (int)round(vec2Direction.x / fDistance);
		vec2Direction.y = (int)round(vec2Direction.y / fDistance);
	}
	else
	{
		// Since we are not going anywhere, set this to 0.
		vec2Direction = glm::vec2(0);
	}
}

/**
 @brief Flip horizontal direction. For patrol use only
 */
void SnowEnemy2DSWBS::FlipHorizontalDirection(void)
{
	vec2Direction.x *= -1;
}

/**
@brief Update position.
*/
void SnowEnemy2DSWBS::UpdatePosition(void)
{
	// Store the old position
	i32vec2OldIndex = vec2Index;

	// if the player is to the left or right of the enemy2D, then jump to attack
	if (vec2Direction.x < 0)
	{
		// Move left
		const int iOldIndex = vec2Index.x;
		if (vec2Index.x >= 0)
		{
			i32vec2NumMicroSteps.x--;
			if (i32vec2NumMicroSteps.x < 0)
			{
				i32vec2NumMicroSteps.x = ((int)cSettings->ENEMY_NUM_STEPS_PER_TILE_XAXIS) - 1;
				vec2Index.x--;
			}
		}

		// Constraint the enemy2D's position within the screen boundary
		Constraint(LEFT);

		// Find a feasible position for the enemy2D's current position
		if (CheckPosition(LEFT) == false)
		{
			FlipHorizontalDirection();
			vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.x = 0;
		}

		// Check if enemy2D is in mid-air, such as walking off a platform
		if (IsMidAir() == true)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}

		// Interact with the Player
		InteractWithPlayer();
	}
	else if (vec2Direction.x > 0)
	{
		// Move right
		const int iOldIndex = vec2Index.x;
		if (vec2Index.x < (int)cSettings->NUM_TILES_XAXIS)
		{
			i32vec2NumMicroSteps.x++;

			if (i32vec2NumMicroSteps.x >= cSettings->ENEMY_NUM_STEPS_PER_TILE_XAXIS)
			{
				i32vec2NumMicroSteps.x = 0;
				vec2Index.x++;
			}
		}

		// Constraint the enemy2D's position within the screen boundary
		Constraint(RIGHT);

		// Find a feasible position for the enemy2D's current position
		if (CheckPosition(RIGHT) == false)
		{
			FlipHorizontalDirection();
			//vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.x = 0;
		}

		// Check if enemy2D is in mid-air, such as walking off a platform
		if (IsMidAir() == true)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}

		// Interact with the Player
		InteractWithPlayer();
	}

	// if the player is above the enemy2D, then jump to attack
	if (vec2Direction.y > 0)
	{
		if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::IDLE)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::JUMP);
			cPhysics2D.SetInitialVelocity(glm::vec2(0.0f, 3.5f));
		}
	}
}

//called whenever an ammo is needed to be shot
CSEAmmo* SnowEnemy2DSWBS::FetchAmmo()
{
	//Exercise 3a: Fetch a game object from m_goList and return it
	for (std::vector<CSEAmmo*>::iterator it = ammoList.begin(); it != ammoList.end(); ++it)
	{
		CSEAmmo* ammo = (CSEAmmo*)*it;
		if (ammo->getActive()) {
			continue;
		}
		ammo->setActive(true);
		// By default, microsteps should be zero --> reset in case a previously active ammo that was used then ste inactive was used again
		ammo->vec2NumMicroSteps = glm::i32vec2(0, 0);
		return ammo;
	}

	//whenever ammoList runs out of ammo, create 10 ammo to use
	//Get Size before adding 10
	int prevSize = ammoList.size();
	for (int i = 0; i < 10; ++i) {
		ammoList.push_back(new CSEAmmo);
	}
	ammoList.at(prevSize)->setActive(true);
	return ammoList.at(prevSize);

}
vector<glm::vec2> SnowEnemy2DSWBS::ConstructWaypointVector(vector<glm::vec2> waypointVector, int startIndex, int numOfWaypoints)
{
	for (int i = 0; i < numOfWaypoints; ++i)
	{
		waypointVector.push_back(cMap2D->GetTilePosition(startIndex + i));
		// Erase the value of the waypoint in the arrMapInfo
		cMap2D->SetMapInfo(waypointVector[i].y, waypointVector[i].x, 0);
		cout << waypointVector.size() << endl;
	}
	return waypointVector;
}