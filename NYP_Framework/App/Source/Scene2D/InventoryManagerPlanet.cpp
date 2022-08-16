/**
 InventoryManager
 @brief A class which manages all the inventory items the game
 By: Toh Da Jun
 Date: Mar 2020
 */

#include "InventoryManagerPlanet.h"
#include <stdexcept>      // std::invalid_argument

/**
@brief Constructor
*/
CInventoryManagerPlanet::CInventoryManagerPlanet(void) 
{
}

/**
@brief Destructor
*/
CInventoryManagerPlanet::~CInventoryManagerPlanet(void)
{
	// Clear the memory
	Exit();
}

/**
@brief Exit by deleting the items
*/
void CInventoryManagerPlanet::Exit(void)
{
	// Delete all scenes stored and empty the entire map
	std::map<std::string, CInventoryItemPlanet*>::iterator it, end;
	end = inventoryMapPlanet.end();
	for (it = inventoryMapPlanet.begin(); it != end; ++it)
	{
		delete it->second;
		it->second = nullptr;
	}
	inventoryMapPlanet.clear();
}

/**
@brief Add a Scene to this Inventory Manager
*/
CInventoryItemPlanet* CInventoryManagerPlanet::Add(	const std::string& _name,
								const char* imagePath,
								const int iItemMaxCount,
								const int iItemCount)
{
	if (Check(_name))
	{
		// Item name already exist here, unable to proceed
		throw std::exception("Duplicate item name provided");
		return NULL;
	}

	CInventoryItemPlanet* cNewItem = new CInventoryItemPlanet(imagePath);
	cNewItem->iItemMaxCount = iItemMaxCount;
	cNewItem->iItemCount = iItemCount;

	// Nothing wrong, add the scene to our map
	inventoryMapPlanet[_name] = cNewItem;

	return cNewItem;
}

/**
@brief Remove an item from this Inventory Manager
*/
bool CInventoryManagerPlanet::Remove(const std::string& _name)
{
	// Does nothing if it does not exist
	if (Check(_name))
	{
		// Item is not available, unable to proceed
		throw std::exception("Unknown item name provided");
		return false;
	}

	CInventoryItemPlanet* target = inventoryMapPlanet[_name];

	// Delete and remove from our map
	delete target;
	inventoryMapPlanet.erase(_name);

	return true;
}

/**
@brief Check if a item exists in this Inventory Manager
*/
bool CInventoryManagerPlanet::Check(const std::string& _name)
{
	return inventoryMapPlanet.count(_name) != 0;
}

/**
@brief Get an item by its name
*/ 
CInventoryItemPlanet* CInventoryManagerPlanet::GetItem(const std::string& _name)
{
	// Does nothing if it does not exist
	if (!Check(_name))
		return NULL;

	// Find and return the item
	return inventoryMapPlanet[_name];
}

/**
@brief Get the number of items
*/
int CInventoryManagerPlanet::GetNumItems(void) const
{
	return inventoryMapPlanet.size();
}
