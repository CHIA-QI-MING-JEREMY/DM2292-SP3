/**
 CInventoryManagerPlanet
 @brief A class which manages all the inventory items the game
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include SingletonTemplate
#include "DesignPatterns\SingletonTemplate.h"

#include <map>
#include <string>
#include "InventoryItemPlanet.h"

class CInventoryManagerPlanet : public CSingletonTemplate<CInventoryManagerPlanet>
{
	friend CSingletonTemplate<CInventoryManagerPlanet>;
public:
	void Exit(void);

	// Add a new item
	CInventoryItemPlanet* Add(	const std::string& _name,
							const char* imagePath, 
							const int iItemMaxCount, 
							const int iItemCount = 0);
	// Remove an item
	bool Remove(const std::string& _name);
	// Check if an item exists in inventoryMap
	bool Check(const std::string& _name);

	// Get an item by its name
	CInventoryItemPlanet* GetItem(const std::string& _name);
	// Get the number of items
	int GetNumItems(void) const;

protected:
	// Constructor
	CInventoryManagerPlanet(void);

	// Destructor
	virtual ~CInventoryManagerPlanet(void);

	// The map containing all the items
	std::map<std::string, CInventoryItemPlanet*> inventoryMapPlanet;
};
