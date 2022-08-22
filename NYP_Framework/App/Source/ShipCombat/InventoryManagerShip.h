/**
 CInventoryManager
 @brief A class which manages all the inventory items the game
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include SingletonTemplate
#include "DesignPatterns\SingletonTemplate.h"

#include <map>
#include <string>
#include "InventoryItemShip.h"

class CInventoryManagerShip : public CSingletonTemplate<CInventoryManagerShip>
{
	friend CSingletonTemplate<CInventoryManagerShip>;
public:
	void Exit(void);

	// Add a new item
	CInventoryItemShip* Add(	const std::string& _name,
							const char* imagePath, 
							const int iItemMaxCount, 
							const int iItemCount = 0);
	// Remove an item
	bool Remove(const std::string& _name);
	// Check if an item exists in inventoryMap
	bool Check(const std::string& _name);

	// Get an item by its name
	CInventoryItemShip* GetItem(const std::string& _name);
	// Get the number of items
	int GetNumItems(void) const;

protected:
	// Constructor
	CInventoryManagerShip(void);

	// Destructor
	virtual ~CInventoryManagerShip(void);

	// The map containing all the items
	std::map<std::string, CInventoryItemShip*> inventoryMap;
};
