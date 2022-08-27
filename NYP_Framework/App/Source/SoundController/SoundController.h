/**
 CSoundController
 @brief A class which manages the sound objects
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include SingletonTemplate
#include <DesignPatterns\SingletonTemplate.h>

// Include GLEW
#include <includes/irrKlang.h>
using namespace irrklang;
#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll

// Include string
#include <string>
// Include map storage
#include <map>
using namespace std;

// Include SoundInfo class; it stores the sound and other information
#include "SoundInfo.h"

class CSoundController : public CSingletonTemplate<CSoundController>
{
	friend CSingletonTemplate<CSoundController>;
public:
	// Initialise this class instance
	bool Init(void);

	enum SOUND_LIST
	{
		//Background Music
		BGM_NORMAL = 0,
		BGM_PLANET,
		BGM_FIGHT,

		//Common for Planets
		FOOTSTEPS,
		ENEMY_FOOTSTEPS,
		JUMP,
		ENEMY_JUMP,
		LAND,
		ENEMY_LAND,
		HIT_CHECKPOINT,
		COLLECT_ITEM, 
		TAKE_DAMAGE,
		CLIMB,

		//Jungle Planet
		SPLASH,
		BURNING,
		TICKING,
		EXPLOSION,
		FIREBALL,
		FLICK_SWITCH,
		POISON_DAMAGE,
		POISONBALL,
		ENEMY_MELEE,
		WATER_FLOWER,
		USING_WATER, //to heal
		PLACE_BUSH,
		TIE_VINE,
		PATROL_TEAM_NOISY,
		VT_TELEPORT_POOF,

		// Terrestrial Planet
		TURRET_SHOOTING,

		//Snow Planet
		BACKGROUNDSNOW,
		BERSERK,
		FREEZE,
		SHIELD,
		PLAYERSNOWSHOOT,
		WOLFBITE,
		WOLFPAIN,
		WOLFSHIELD,
		WOLFHEAL,

		// UI Sounds
		BUTTONCLICK,
		WINODWOPEN,

		NUM_SOUNDS
	};

	// Load a sound
	bool LoadSound(	string filename, 
					const int ID,
					const bool bPreload = true,
					const bool bIsLooped = false,
					CSoundInfo::SOUNDTYPE eSoundType = CSoundInfo::SOUNDTYPE::_2D,
					vec3df vec3dfSoundPos = vec3df(0.0f, 0.0f, 0.0f));

	// Play a sound by its ID
	void PlaySoundByID(const int ID);
	// Stop a sound by its ID
	void StopSoundByID(const int ID);

	// Increase Master volume
	bool MasterVolumeIncrease(void);
	// Decrease Master volume
	bool MasterVolumeDecrease(void);

	// Increase volume of a ISoundSource
	bool VolumeIncrease(const int ID);
	// Decrease volume of a ISoundSource
	bool VolumeDecrease(const int ID);

	// For 3D sounds only
	// Set Listener position
	void SetListenerPosition(const float x, const float y, const float z);
	// Set Listener direction
	void SetListenerDirection(const float x, const float y, const float z);

protected:
	// Constructor
	CSoundController(void);

	// Destructor
	virtual ~CSoundController(void);

	// Get an sound from this map
	CSoundInfo* GetSound(const int ID);
	// Remove an sound from this map
	bool RemoveSound(const int ID);
	// Get the number of sounds in this map
	int GetNumOfSounds(void) const;

	// The handler to the irrklang Sound Engine
	ISoundEngine* cSoundEngine;

	// The map of all the entity created
	std::map<int, CSoundInfo*> soundMap;

	// For 3D sound only: Listener position
	vec3df vec3dfListenerPos;
	// For 3D sound only: Listender view direction
	vec3df vec3dfListenerDir;
};

