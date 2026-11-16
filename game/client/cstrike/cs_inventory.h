//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: CS:GO Weapons, Knives, Gloves, Sleeves & Stickers Inventory backend
//
//=============================================================================//

#ifndef CS_INVENTORY_H
#define CS_INVENTORY_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

class C_BaseViewModel;
class C_CSPlayer;

enum EInventoryTeam
{
	INV_TEAM_CT = 0,
	INV_TEAM_T  = 1,
	INV_TEAM_COUNT
};

enum EInventoryTeamMask
{
	INV_TEAMMASK_NONE = 0,
	INV_TEAMMASK_CT   = ( 1 << 0 ),
	INV_TEAMMASK_T    = ( 1 << 1 ),
	INV_TEAMMASK_BOTH = ( INV_TEAMMASK_CT | INV_TEAMMASK_T ),
};

enum EInventoryCategory
{
	INV_CAT_ALL = 0,
	INV_CAT_PISTOLS,
	INV_CAT_SMGS,
	INV_CAT_RIFLES,
	INV_CAT_HEAVY,
	INV_CAT_KNIVES,
	INV_CAT_GLOVES,
	INV_CAT_SLEEVES,
	INV_CAT_COUNT
};

enum EInventoryWeaponMode
{
	INV_MODE_UNSUPPORTED = 0,
	INV_MODE_AK47,
	INV_MODE_AWP,
	INV_MODE_DEAGLE,
	INV_MODE_M4A1,
	INV_MODE_GLOCK,
	INV_MODE_USP,
	INV_MODE_MAC10,
	INV_MODE_MP9,
	INV_MODE_P90,
	INV_MODE_NOVA,
	INV_MODE_XM1014,
	INV_MODE_P228,
	INV_MODE_ELITE,
	INV_MODE_FIVESEVEN,
	INV_MODE_MP5,
	INV_MODE_UMP45,
	INV_MODE_GALIL,
	INV_MODE_FAMAS,
	INV_MODE_SCOUT,
	INV_MODE_SG550,
	INV_MODE_SG552,
	INV_MODE_G3SG1,
	INV_MODE_AUG,
	INV_MODE_M249,
	INV_MODE_KNIFE,
	INV_MODE_GLOVE,
	INV_MODE_SLEEVE,
};

enum EInventorySkinWeapon
{
	INV_SKIN_WEP_AK47 = 0,
	INV_SKIN_WEP_AWP,
	INV_SKIN_WEP_DEAGLE,
	INV_SKIN_WEP_M4A1,
	INV_SKIN_WEP_GLOCK,
	INV_SKIN_WEP_USP,
	INV_SKIN_WEP_MAC10,
	INV_SKIN_WEP_MP9,
	INV_SKIN_WEP_P90,
	INV_SKIN_WEP_NOVA,
	INV_SKIN_WEP_XM1014,
	INV_SKIN_WEP_P228,
	INV_SKIN_WEP_ELITE,
	INV_SKIN_WEP_FIVESEVEN,
	INV_SKIN_WEP_MP5,
	INV_SKIN_WEP_UMP45,
	INV_SKIN_WEP_GALIL,
	INV_SKIN_WEP_FAMAS,
	INV_SKIN_WEP_SCOUT,
	INV_SKIN_WEP_SG550,
	INV_SKIN_WEP_SG552,
	INV_SKIN_WEP_G3SG1,
	INV_SKIN_WEP_AUG,
	INV_SKIN_WEP_M249,
	INV_SKIN_WEP_COUNT
};

enum EInventoryRarity
{
	RARITY_STOCK = 0,
	RARITY_MILSPEC,
	RARITY_RESTRICTED,
	RARITY_CLASSIFIED,
	RARITY_COVERT,
	RARITY_GOLD
};

enum
{
	INV_MAX_STICKER_SLOTS = 4,
	INV_KNIFE_HINT_BASE   = 1000,
};

struct InventoryWeaponDef_t
{
	const char *pszId;
	const char *pszDisplayName;
	EInventoryCategory eCategory;
	EInventoryWeaponMode eMode;
	int nAvailableTeamsMask; // EInventoryTeamMask
	bool bSupported;
	const char *pszPreviewModel;
	const char *pszIconMaterial;
};

struct InventorySkinDef_t
{
	const char *pszKey;
	const char *pszDisplayName;
	EInventoryRarity eRarity;
};

struct InventoryStickerDef_t
{
	const char *pszKey;
	const char *pszDisplayName;
};

struct InventoryKnifeDef_t
{
	const char *pszKey;
	const char *pszDisplayName;
	const char *pszModelPath;
	const char *pszIconPath;
	EInventoryRarity eRarity;
	const char *pszSkinTexDir;
	const InventorySkinDef_t *pSkins;
	int nSkinCount;
};

struct InventoryGloveDef_t
{
	const char *pszId;           // Unique item key, e.g. "default", "bare", "sporty_slingshot"
	const char *pszFamilyKey;    // Value for cl_arm_glove (e.g. "-1", "bare", "sporty")
	const char *pszSkinKey;      // Value for cl_arm_glove_<family> (e.g. "0", "slingshot", "blue")
	const char *pszDisplayName;
	const char *pszModelPath;    // For hardknuckle_blue / family mdl
	const char *pszIconPath;
	EInventoryRarity eRarity;
};

struct InventorySleeveDef_t
{
	const char *pszKey;          // Value for cl_arm_sleeve (e.g. "-1", "none", "swat", "sas")
	const char *pszDisplayName;
	const char *pszModelPath;
	const char *pszIconPath;
	EInventoryRarity eRarity;
};

struct InventoryWeaponLoadout_t
{
	char szSkinKey[64];
	char szStickerKeys[INV_MAX_STICKER_SLOTS][64];
};

// Weapon Catalog accessors
int Inventory_GetWeaponCount();
const InventoryWeaponDef_t *Inventory_GetWeaponDef( int iIndex );
const char *Inventory_GetCategoryName( EInventoryCategory eCategory );
const char *Inventory_GetCategoryLocToken( EInventoryCategory eCategory );

// Weapon Skins & Stickers (AK-47, AWP, Desert Eagle, M4A1, Glock-18, etc.)
int Inventory_GetWeaponSkinIndexFromMode( EInventoryWeaponMode eMode );
const char *Inventory_GetWeaponSkinDisplayName( int nWep );
int Inventory_GetWeaponSkinCount( int nWep ); // Does NOT include stock "0" default
const InventorySkinDef_t *Inventory_GetWeaponSkinDef( int nWep, int iIndex );
const InventorySkinDef_t *Inventory_FindWeaponSkinByKey( int nWep, const char *pszKey );
void Inventory_GetWeaponSkinIconPath( int nWep, const char *pszSkinKey, char *pszOutPath, int nOutSize );

int Inventory_GetStickerCount(); // Does NOT include "0" none
const InventoryStickerDef_t *Inventory_GetStickerDef( int iIndex );
const InventoryStickerDef_t *Inventory_FindStickerByKey( const char *pszKey );
void Inventory_GetWeaponStickerModelPath( int nWep, int nSlot, char *pszOutPath, int nOutSize );
void Inventory_GetStickerTexturePath( const char *pszKey, char *pszOutPath, int nOutSize );

// Knives
int Inventory_GetKnifeCount(); // Includes "default" + 19 CS:GO knives
const InventoryKnifeDef_t *Inventory_GetKnifeDef( int iIndex );
const InventoryKnifeDef_t *Inventory_FindKnifeByKey( const char *pszKey );
int Inventory_FindKnifeIndexByKey( const char *pszKey );
const char *Inventory_GetEquippedKnifeKey( EInventoryTeam eTeam );
const char *Inventory_GetEquippedKnifeSkinKey( EInventoryTeam eTeam );
int Inventory_GetKnifeEquippedMask( const char *pszKnifeKey );
int Inventory_GetKnifeSkinCount( const char *pszKnifeKey );
const InventorySkinDef_t *Inventory_GetKnifeSkinDef( const char *pszKnifeKey, int iIndex );
const InventorySkinDef_t *Inventory_FindKnifeSkinByKey( const char *pszKnifeKey, const char *pszSkinKey );
void Inventory_GetKnifeSkinIconPath( const char *pszKnifeKey, const char *pszSkinKey, char *pszOutPath, int nOutSize );
int Inventory_GetKnifeSkinEquippedMask( const char *pszKnifeKey, const char *pszSkinKey );
void Inventory_EquipKnife( int nTeamMask, const char *pszKnifeKey, const char *pszSkinKey = NULL );
void Inventory_SetPreviewKnife( const char *pszKnifeKey, const char *pszSkinKey );

// Gloves
int Inventory_GetGloveCount();
const InventoryGloveDef_t *Inventory_GetGloveDef( int iIndex );
const InventoryGloveDef_t *Inventory_FindGloveById( const char *pszId );
const char *Inventory_GetEquippedGloveId( EInventoryTeam eTeam );
int Inventory_GetGloveEquippedMask( const char *pszGloveId );
void Inventory_EquipGlove( int nTeamMask, const char *pszGloveId );

// Sleeves
int Inventory_GetSleeveCount();
const InventorySleeveDef_t *Inventory_GetSleeveDef( int iIndex );
const InventorySleeveDef_t *Inventory_FindSleeveByKey( const char *pszKey );
const char *Inventory_GetEquippedSleeveKey( EInventoryTeam eTeam );
int Inventory_GetSleeveEquippedMask( const char *pszSleeveKey );
void Inventory_EquipSleeve( int nTeamMask, const char *pszSleeveKey );

// Initialization, persistence & team ConVar sync
void Inventory_EnsureLoaded();
void Inventory_SaveLoadout();
void Inventory_SyncActiveTeamConVars( int nTeamNumber );

// Team loadout read/write
const InventoryWeaponLoadout_t &Inventory_GetWeaponLoadout( int nWep, EInventoryTeam eTeam );
int Inventory_GetWeaponSkinEquippedMask( int nWep, const char *pszSkinKey ); // EInventoryTeamMask
void Inventory_EquipWeapon( int nWep, int nTeamMask, const char *pszSkinKey, const char * const pszStickerKeys[INV_MAX_STICKER_SLOTS] );

// ConVar integration
void Inventory_OnWeaponConVarChanged( int nWep );

// Live preview selection & tokens (used by material proxies)
void Inventory_SetPreviewSelection( int nWep, const char *pszSkinKey, const char * const pszStickerKeys[INV_MAX_STICKER_SLOTS] );
void Inventory_SetPreviewGlove( const char *pszGloveId );
void *Inventory_GetPreviewSkinProxyToken();
void *Inventory_GetPreviewStickerProxyToken( int nSlot );
void *Inventory_GetPreviewGloveProxyToken();

// Resolves active arm models + optional baked glove texture for First-Person
void Inventory_ResolvePreviewArmsForTeam( EInventoryTeam eTeam, const char *pszOverrideGloveId, const char *pszOverrideSleeveKey,
	const char *&outGloveModel, const char *&outGloveOverrideTex, const char *&outSleeveModel, bool &outHideBare );

// Proxy & viewmodel helpers
bool Inventory_ResolveWeaponSkinChoiceForRenderable( int nWepHint, void *pRenderable, char *pszOutChoice, int nOutSize, bool &bOutShouldApply );
bool Inventory_ResolveWeaponSkinTexturePath( int nWepHint, const char *pszChoice, char *pszOutPath, int nOutSize );

bool Inventory_ResolveWeaponStickerChoiceForRenderable( int nWepHint, void *pRenderable, int nSlot, char *pszOutChoice, int nOutSize );
bool Inventory_ResolveStickerTexturePath( const char *pszChoice, char *pszOutPath, int nOutSize );

bool Inventory_ResolvePreviewGloveOverrideTexture( void *pRenderable, char *pszOutPath, int nOutSize );

void Inventory_GetActiveStickersForViewmodel( int nWep, C_BaseViewModel *pVM, C_CSPlayer *pPlayer, bool bOutActive[INV_MAX_STICKER_SLOTS] );

#endif // CS_INVENTORY_H
