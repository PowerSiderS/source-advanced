//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: CS:GO VGUI2 Weapons Inventory Dialog (AK-47 Skins & Stickers)
//
//=============================================================================//

#ifndef CSTRIKEINVENTORY_H
#define CSTRIKEINVENTORY_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include "cs_inventory.h"

void CSInventory_Init();
void CSInventory_Shutdown();
void CSInventory_Open( bool bToggle = false );
void CSInventory_Close();

#endif // CSTRIKEINVENTORY_H
