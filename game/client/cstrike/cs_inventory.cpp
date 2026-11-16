//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: CS:GO Weapons, Knives, Gloves, Sleeves & Stickers Inventory backend
//
//=============================================================================//

#include "cbase.h"
#include "cs_inventory.h"
#include "cs_shareddefs.h"
#include "c_cs_player.h"
#include "weapon_csbase.h"
#include "c_baseviewmodel.h"
#include "filesystem.h"
#include <KeyValues.h>
#include <ctype.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define INVENTORY_LOADOUT_FILE "cfg/csgo_inventory_loadout.vdf"
#define ADVANCED_ARMS_DIR "models/advanced_inventory_csgo_items/weapons/v_models/arms/"
#define ADVANCED_WEAPONS_DIR "models/advanced_inventory_csgo_items/weapons/"

static void OnWeaponSkinConVarChangeCallback( IConVar *var, const char *pOldValue, float flOldValue );

ConVar cl_ak47_skin( "cl_ak47_skin", "0", FCVAR_ARCHIVE, "AK-47 skin selection (0 = default, 1-34 or skin name e.g. bloodsport, vulcan, asiimov, redline, fireserpent).", OnWeaponSkinConVarChangeCallback );
ConVar cl_ak47_sticker_1( "cl_ak47_sticker_1", "0", FCVAR_ARCHIVE, "AK-47 sticker slot 1 (0 = none, 1-40 or sticker name e.g. ibuypower, titan, crown, howling_dawn).", OnWeaponSkinConVarChangeCallback );
ConVar cl_ak47_sticker_2( "cl_ak47_sticker_2", "0", FCVAR_ARCHIVE, "AK-47 sticker slot 2 (0 = none, 1-40 or sticker name).", OnWeaponSkinConVarChangeCallback );
ConVar cl_ak47_sticker_3( "cl_ak47_sticker_3", "0", FCVAR_ARCHIVE, "AK-47 sticker slot 3 (0 = none, 1-40 or sticker name).", OnWeaponSkinConVarChangeCallback );
ConVar cl_ak47_sticker_4( "cl_ak47_sticker_4", "0", FCVAR_ARCHIVE, "AK-47 sticker slot 4 (0 = none, 1-40 or sticker name).", OnWeaponSkinConVarChangeCallback );

ConVar cl_awp_skin( "cl_awp_skin", "0", FCVAR_ARCHIVE, "AWP skin selection (0 = default, 1-24 or skin name e.g. dragon_lore, asiimov, gungnir, redline).", OnWeaponSkinConVarChangeCallback );
ConVar cl_awp_sticker_1( "cl_awp_sticker_1", "0", FCVAR_ARCHIVE, "AWP sticker slot 1 (0 = none, 1-40 or sticker name).", OnWeaponSkinConVarChangeCallback );
ConVar cl_awp_sticker_2( "cl_awp_sticker_2", "0", FCVAR_ARCHIVE, "AWP sticker slot 2 (0 = none, 1-40 or sticker name).", OnWeaponSkinConVarChangeCallback );
ConVar cl_awp_sticker_3( "cl_awp_sticker_3", "0", FCVAR_ARCHIVE, "AWP sticker slot 3 (0 = none, 1-40 or sticker name).", OnWeaponSkinConVarChangeCallback );
ConVar cl_awp_sticker_4( "cl_awp_sticker_4", "0", FCVAR_ARCHIVE, "AWP sticker slot 4 (0 = none, 1-40 or sticker name).", OnWeaponSkinConVarChangeCallback );

ConVar cl_deagle_skin( "cl_deagle_skin", "0", FCVAR_ARCHIVE, "Desert Eagle skin selection (0 = default, 1-17 or skin name e.g. printstream, code_red, kumicho_dragon).", OnWeaponSkinConVarChangeCallback );
ConVar cl_deagle_sticker_1( "cl_deagle_sticker_1", "0", FCVAR_ARCHIVE, "Desert Eagle sticker slot 1 (0 = none, 1-40 or sticker name).", OnWeaponSkinConVarChangeCallback );
ConVar cl_deagle_sticker_2( "cl_deagle_sticker_2", "0", FCVAR_ARCHIVE, "Desert Eagle sticker slot 2 (0 = none, 1-40 or sticker name).", OnWeaponSkinConVarChangeCallback );
ConVar cl_deagle_sticker_3( "cl_deagle_sticker_3", "0", FCVAR_ARCHIVE, "Desert Eagle sticker slot 3 (0 = none, 1-40 or sticker name).", OnWeaponSkinConVarChangeCallback );
ConVar cl_deagle_sticker_4( "cl_deagle_sticker_4", "0", FCVAR_ARCHIVE, "Desert Eagle sticker slot 4 (0 = none, 1-40 or sticker name).", OnWeaponSkinConVarChangeCallback );

ConVar cl_m4a1_skin( "cl_m4a1_skin", "0", FCVAR_ARCHIVE, "M4A1 skin selection (0 = default, 1-21 or skin name e.g. printstream, hyper_beast, cyrex, golden_coil).", OnWeaponSkinConVarChangeCallback );
ConVar cl_m4a1_sticker_1( "cl_m4a1_sticker_1", "0", FCVAR_ARCHIVE, "M4A1 sticker slot 1 (0 = none, 1-40 or sticker name).", OnWeaponSkinConVarChangeCallback );
ConVar cl_m4a1_sticker_2( "cl_m4a1_sticker_2", "0", FCVAR_ARCHIVE, "M4A1 sticker slot 2 (0 = none, 1-40 or sticker name).", OnWeaponSkinConVarChangeCallback );
ConVar cl_m4a1_sticker_3( "cl_m4a1_sticker_3", "0", FCVAR_ARCHIVE, "M4A1 sticker slot 3 (0 = none, 1-40 or sticker name).", OnWeaponSkinConVarChangeCallback );
ConVar cl_m4a1_sticker_4( "cl_m4a1_sticker_4", "0", FCVAR_ARCHIVE, "M4A1 sticker slot 4 (0 = none, 1-40 or sticker name).", OnWeaponSkinConVarChangeCallback );

ConVar cl_glock_skin( "cl_glock_skin", "0", FCVAR_ARCHIVE, "Glock-18 skin selection.", OnWeaponSkinConVarChangeCallback );
ConVar cl_glock_sticker_1( "cl_glock_sticker_1", "0", FCVAR_ARCHIVE, "Glock-18 sticker slot 1.", OnWeaponSkinConVarChangeCallback );
ConVar cl_glock_sticker_2( "cl_glock_sticker_2", "0", FCVAR_ARCHIVE, "Glock-18 sticker slot 2.", OnWeaponSkinConVarChangeCallback );
ConVar cl_glock_sticker_3( "cl_glock_sticker_3", "0", FCVAR_ARCHIVE, "Glock-18 sticker slot 3.", OnWeaponSkinConVarChangeCallback );
ConVar cl_glock_sticker_4( "cl_glock_sticker_4", "0", FCVAR_ARCHIVE, "Glock-18 sticker slot 4.", OnWeaponSkinConVarChangeCallback );

ConVar cl_usp_skin( "cl_usp_skin", "0", FCVAR_ARCHIVE, "USP-S skin selection.", OnWeaponSkinConVarChangeCallback );
ConVar cl_usp_sticker_1( "cl_usp_sticker_1", "0", FCVAR_ARCHIVE, "USP-S sticker slot 1.", OnWeaponSkinConVarChangeCallback );
ConVar cl_usp_sticker_2( "cl_usp_sticker_2", "0", FCVAR_ARCHIVE, "USP-S sticker slot 2.", OnWeaponSkinConVarChangeCallback );
ConVar cl_usp_sticker_3( "cl_usp_sticker_3", "0", FCVAR_ARCHIVE, "USP-S sticker slot 3.", OnWeaponSkinConVarChangeCallback );
ConVar cl_usp_sticker_4( "cl_usp_sticker_4", "0", FCVAR_ARCHIVE, "USP-S sticker slot 4.", OnWeaponSkinConVarChangeCallback );

ConVar cl_mac10_skin( "cl_mac10_skin", "0", FCVAR_ARCHIVE, "MAC-10 skin selection.", OnWeaponSkinConVarChangeCallback );
ConVar cl_mac10_sticker_1( "cl_mac10_sticker_1", "0", FCVAR_ARCHIVE, "MAC-10 sticker slot 1.", OnWeaponSkinConVarChangeCallback );
ConVar cl_mac10_sticker_2( "cl_mac10_sticker_2", "0", FCVAR_ARCHIVE, "MAC-10 sticker slot 2.", OnWeaponSkinConVarChangeCallback );
ConVar cl_mac10_sticker_3( "cl_mac10_sticker_3", "0", FCVAR_ARCHIVE, "MAC-10 sticker slot 3.", OnWeaponSkinConVarChangeCallback );
ConVar cl_mac10_sticker_4( "cl_mac10_sticker_4", "0", FCVAR_ARCHIVE, "MAC-10 sticker slot 4.", OnWeaponSkinConVarChangeCallback );

ConVar cl_mp9_skin( "cl_mp9_skin", "0", FCVAR_ARCHIVE, "MP9 skin selection.", OnWeaponSkinConVarChangeCallback );
ConVar cl_mp9_sticker_1( "cl_mp9_sticker_1", "0", FCVAR_ARCHIVE, "MP9 sticker slot 1.", OnWeaponSkinConVarChangeCallback );
ConVar cl_mp9_sticker_2( "cl_mp9_sticker_2", "0", FCVAR_ARCHIVE, "MP9 sticker slot 2.", OnWeaponSkinConVarChangeCallback );
ConVar cl_mp9_sticker_3( "cl_mp9_sticker_3", "0", FCVAR_ARCHIVE, "MP9 sticker slot 3.", OnWeaponSkinConVarChangeCallback );
ConVar cl_mp9_sticker_4( "cl_mp9_sticker_4", "0", FCVAR_ARCHIVE, "MP9 sticker slot 4.", OnWeaponSkinConVarChangeCallback );

ConVar cl_p90_skin( "cl_p90_skin", "0", FCVAR_ARCHIVE, "P90 skin selection.", OnWeaponSkinConVarChangeCallback );
ConVar cl_p90_sticker_1( "cl_p90_sticker_1", "0", FCVAR_ARCHIVE, "P90 sticker slot 1.", OnWeaponSkinConVarChangeCallback );
ConVar cl_p90_sticker_2( "cl_p90_sticker_2", "0", FCVAR_ARCHIVE, "P90 sticker slot 2.", OnWeaponSkinConVarChangeCallback );
ConVar cl_p90_sticker_3( "cl_p90_sticker_3", "0", FCVAR_ARCHIVE, "P90 sticker slot 3.", OnWeaponSkinConVarChangeCallback );
ConVar cl_p90_sticker_4( "cl_p90_sticker_4", "0", FCVAR_ARCHIVE, "P90 sticker slot 4.", OnWeaponSkinConVarChangeCallback );

ConVar cl_nova_skin( "cl_nova_skin", "0", FCVAR_ARCHIVE, "Nova skin selection.", OnWeaponSkinConVarChangeCallback );
ConVar cl_nova_sticker_1( "cl_nova_sticker_1", "0", FCVAR_ARCHIVE, "Nova sticker slot 1.", OnWeaponSkinConVarChangeCallback );
ConVar cl_nova_sticker_2( "cl_nova_sticker_2", "0", FCVAR_ARCHIVE, "Nova sticker slot 2.", OnWeaponSkinConVarChangeCallback );
ConVar cl_nova_sticker_3( "cl_nova_sticker_3", "0", FCVAR_ARCHIVE, "Nova sticker slot 3.", OnWeaponSkinConVarChangeCallback );
ConVar cl_nova_sticker_4( "cl_nova_sticker_4", "0", FCVAR_ARCHIVE, "Nova sticker slot 4.", OnWeaponSkinConVarChangeCallback );

ConVar cl_xm1014_skin( "cl_xm1014_skin", "0", FCVAR_ARCHIVE, "XM1014 skin selection.", OnWeaponSkinConVarChangeCallback );
ConVar cl_xm1014_sticker_1( "cl_xm1014_sticker_1", "0", FCVAR_ARCHIVE, "XM1014 sticker slot 1.", OnWeaponSkinConVarChangeCallback );
ConVar cl_xm1014_sticker_2( "cl_xm1014_sticker_2", "0", FCVAR_ARCHIVE, "XM1014 sticker slot 2.", OnWeaponSkinConVarChangeCallback );
ConVar cl_xm1014_sticker_3( "cl_xm1014_sticker_3", "0", FCVAR_ARCHIVE, "XM1014 sticker slot 3.", OnWeaponSkinConVarChangeCallback );
ConVar cl_xm1014_sticker_4( "cl_xm1014_sticker_4", "0", FCVAR_ARCHIVE, "XM1014 sticker slot 4.", OnWeaponSkinConVarChangeCallback );

ConVar cl_p228_skin( "cl_p228_skin", "0", FCVAR_ARCHIVE, "P228 skin selection.", OnWeaponSkinConVarChangeCallback );
ConVar cl_p228_sticker_1( "cl_p228_sticker_1", "0", FCVAR_ARCHIVE, "P228 sticker slot 1.", OnWeaponSkinConVarChangeCallback );
ConVar cl_p228_sticker_2( "cl_p228_sticker_2", "0", FCVAR_ARCHIVE, "P228 sticker slot 2.", OnWeaponSkinConVarChangeCallback );
ConVar cl_p228_sticker_3( "cl_p228_sticker_3", "0", FCVAR_ARCHIVE, "P228 sticker slot 3.", OnWeaponSkinConVarChangeCallback );
ConVar cl_p228_sticker_4( "cl_p228_sticker_4", "0", FCVAR_ARCHIVE, "P228 sticker slot 4.", OnWeaponSkinConVarChangeCallback );

ConVar cl_elite_skin( "cl_elite_skin", "0", FCVAR_ARCHIVE, "Dual Elites skin selection.", OnWeaponSkinConVarChangeCallback );
ConVar cl_elite_sticker_1( "cl_elite_sticker_1", "0", FCVAR_ARCHIVE, "Dual Elites sticker slot 1.", OnWeaponSkinConVarChangeCallback );
ConVar cl_elite_sticker_2( "cl_elite_sticker_2", "0", FCVAR_ARCHIVE, "Dual Elites sticker slot 2.", OnWeaponSkinConVarChangeCallback );
ConVar cl_elite_sticker_3( "cl_elite_sticker_3", "0", FCVAR_ARCHIVE, "Dual Elites sticker slot 3.", OnWeaponSkinConVarChangeCallback );
ConVar cl_elite_sticker_4( "cl_elite_sticker_4", "0", FCVAR_ARCHIVE, "Dual Elites sticker slot 4.", OnWeaponSkinConVarChangeCallback );

ConVar cl_fiveseven_skin( "cl_fiveseven_skin", "0", FCVAR_ARCHIVE, "Five-SeveN skin selection.", OnWeaponSkinConVarChangeCallback );
ConVar cl_fiveseven_sticker_1( "cl_fiveseven_sticker_1", "0", FCVAR_ARCHIVE, "Five-SeveN sticker slot 1.", OnWeaponSkinConVarChangeCallback );
ConVar cl_fiveseven_sticker_2( "cl_fiveseven_sticker_2", "0", FCVAR_ARCHIVE, "Five-SeveN sticker slot 2.", OnWeaponSkinConVarChangeCallback );
ConVar cl_fiveseven_sticker_3( "cl_fiveseven_sticker_3", "0", FCVAR_ARCHIVE, "Five-SeveN sticker slot 3.", OnWeaponSkinConVarChangeCallback );
ConVar cl_fiveseven_sticker_4( "cl_fiveseven_sticker_4", "0", FCVAR_ARCHIVE, "Five-SeveN sticker slot 4.", OnWeaponSkinConVarChangeCallback );

ConVar cl_mp5_skin( "cl_mp5_skin", "0", FCVAR_ARCHIVE, "MP5 skin selection.", OnWeaponSkinConVarChangeCallback );
ConVar cl_mp5_sticker_1( "cl_mp5_sticker_1", "0", FCVAR_ARCHIVE, "MP5 sticker slot 1.", OnWeaponSkinConVarChangeCallback );
ConVar cl_mp5_sticker_2( "cl_mp5_sticker_2", "0", FCVAR_ARCHIVE, "MP5 sticker slot 2.", OnWeaponSkinConVarChangeCallback );
ConVar cl_mp5_sticker_3( "cl_mp5_sticker_3", "0", FCVAR_ARCHIVE, "MP5 sticker slot 3.", OnWeaponSkinConVarChangeCallback );
ConVar cl_mp5_sticker_4( "cl_mp5_sticker_4", "0", FCVAR_ARCHIVE, "MP5 sticker slot 4.", OnWeaponSkinConVarChangeCallback );

ConVar cl_ump45_skin( "cl_ump45_skin", "0", FCVAR_ARCHIVE, "UMP-45 skin selection.", OnWeaponSkinConVarChangeCallback );
ConVar cl_ump45_sticker_1( "cl_ump45_sticker_1", "0", FCVAR_ARCHIVE, "UMP-45 sticker slot 1.", OnWeaponSkinConVarChangeCallback );
ConVar cl_ump45_sticker_2( "cl_ump45_sticker_2", "0", FCVAR_ARCHIVE, "UMP-45 sticker slot 2.", OnWeaponSkinConVarChangeCallback );
ConVar cl_ump45_sticker_3( "cl_ump45_sticker_3", "0", FCVAR_ARCHIVE, "UMP-45 sticker slot 3.", OnWeaponSkinConVarChangeCallback );
ConVar cl_ump45_sticker_4( "cl_ump45_sticker_4", "0", FCVAR_ARCHIVE, "UMP-45 sticker slot 4.", OnWeaponSkinConVarChangeCallback );

ConVar cl_galil_skin( "cl_galil_skin", "0", FCVAR_ARCHIVE, "Galil skin selection.", OnWeaponSkinConVarChangeCallback );
ConVar cl_galil_sticker_1( "cl_galil_sticker_1", "0", FCVAR_ARCHIVE, "Galil sticker slot 1.", OnWeaponSkinConVarChangeCallback );
ConVar cl_galil_sticker_2( "cl_galil_sticker_2", "0", FCVAR_ARCHIVE, "Galil sticker slot 2.", OnWeaponSkinConVarChangeCallback );
ConVar cl_galil_sticker_3( "cl_galil_sticker_3", "0", FCVAR_ARCHIVE, "Galil sticker slot 3.", OnWeaponSkinConVarChangeCallback );
ConVar cl_galil_sticker_4( "cl_galil_sticker_4", "0", FCVAR_ARCHIVE, "Galil sticker slot 4.", OnWeaponSkinConVarChangeCallback );

ConVar cl_famas_skin( "cl_famas_skin", "0", FCVAR_ARCHIVE, "FAMAS skin selection.", OnWeaponSkinConVarChangeCallback );
ConVar cl_famas_sticker_1( "cl_famas_sticker_1", "0", FCVAR_ARCHIVE, "FAMAS sticker slot 1.", OnWeaponSkinConVarChangeCallback );
ConVar cl_famas_sticker_2( "cl_famas_sticker_2", "0", FCVAR_ARCHIVE, "FAMAS sticker slot 2.", OnWeaponSkinConVarChangeCallback );
ConVar cl_famas_sticker_3( "cl_famas_sticker_3", "0", FCVAR_ARCHIVE, "FAMAS sticker slot 3.", OnWeaponSkinConVarChangeCallback );
ConVar cl_famas_sticker_4( "cl_famas_sticker_4", "0", FCVAR_ARCHIVE, "FAMAS sticker slot 4.", OnWeaponSkinConVarChangeCallback );

ConVar cl_scout_skin( "cl_scout_skin", "0", FCVAR_ARCHIVE, "Scout skin selection.", OnWeaponSkinConVarChangeCallback );
ConVar cl_scout_sticker_1( "cl_scout_sticker_1", "0", FCVAR_ARCHIVE, "Scout sticker slot 1.", OnWeaponSkinConVarChangeCallback );
ConVar cl_scout_sticker_2( "cl_scout_sticker_2", "0", FCVAR_ARCHIVE, "Scout sticker slot 2.", OnWeaponSkinConVarChangeCallback );
ConVar cl_scout_sticker_3( "cl_scout_sticker_3", "0", FCVAR_ARCHIVE, "Scout sticker slot 3.", OnWeaponSkinConVarChangeCallback );
ConVar cl_scout_sticker_4( "cl_scout_sticker_4", "0", FCVAR_ARCHIVE, "Scout sticker slot 4.", OnWeaponSkinConVarChangeCallback );

ConVar cl_sg550_skin( "cl_sg550_skin", "0", FCVAR_ARCHIVE, "SG 550 skin selection.", OnWeaponSkinConVarChangeCallback );
ConVar cl_sg550_sticker_1( "cl_sg550_sticker_1", "0", FCVAR_ARCHIVE, "SG 550 sticker slot 1.", OnWeaponSkinConVarChangeCallback );
ConVar cl_sg550_sticker_2( "cl_sg550_sticker_2", "0", FCVAR_ARCHIVE, "SG 550 sticker slot 2.", OnWeaponSkinConVarChangeCallback );
ConVar cl_sg550_sticker_3( "cl_sg550_sticker_3", "0", FCVAR_ARCHIVE, "SG 550 sticker slot 3.", OnWeaponSkinConVarChangeCallback );
ConVar cl_sg550_sticker_4( "cl_sg550_sticker_4", "0", FCVAR_ARCHIVE, "SG 550 sticker slot 4.", OnWeaponSkinConVarChangeCallback );

ConVar cl_sg552_skin( "cl_sg552_skin", "0", FCVAR_ARCHIVE, "SG 552 skin selection.", OnWeaponSkinConVarChangeCallback );
ConVar cl_sg552_sticker_1( "cl_sg552_sticker_1", "0", FCVAR_ARCHIVE, "SG 552 sticker slot 1.", OnWeaponSkinConVarChangeCallback );
ConVar cl_sg552_sticker_2( "cl_sg552_sticker_2", "0", FCVAR_ARCHIVE, "SG 552 sticker slot 2.", OnWeaponSkinConVarChangeCallback );
ConVar cl_sg552_sticker_3( "cl_sg552_sticker_3", "0", FCVAR_ARCHIVE, "SG 552 sticker slot 3.", OnWeaponSkinConVarChangeCallback );
ConVar cl_sg552_sticker_4( "cl_sg552_sticker_4", "0", FCVAR_ARCHIVE, "SG 552 sticker slot 4.", OnWeaponSkinConVarChangeCallback );

ConVar cl_g3sg1_skin( "cl_g3sg1_skin", "0", FCVAR_ARCHIVE, "G3SG1 skin selection.", OnWeaponSkinConVarChangeCallback );
ConVar cl_g3sg1_sticker_1( "cl_g3sg1_sticker_1", "0", FCVAR_ARCHIVE, "G3SG1 sticker slot 1.", OnWeaponSkinConVarChangeCallback );
ConVar cl_g3sg1_sticker_2( "cl_g3sg1_sticker_2", "0", FCVAR_ARCHIVE, "G3SG1 sticker slot 2.", OnWeaponSkinConVarChangeCallback );
ConVar cl_g3sg1_sticker_3( "cl_g3sg1_sticker_3", "0", FCVAR_ARCHIVE, "G3SG1 sticker slot 3.", OnWeaponSkinConVarChangeCallback );
ConVar cl_g3sg1_sticker_4( "cl_g3sg1_sticker_4", "0", FCVAR_ARCHIVE, "G3SG1 sticker slot 4.", OnWeaponSkinConVarChangeCallback );

ConVar cl_aug_skin( "cl_aug_skin", "0", FCVAR_ARCHIVE, "AUG skin selection.", OnWeaponSkinConVarChangeCallback );
ConVar cl_aug_sticker_1( "cl_aug_sticker_1", "0", FCVAR_ARCHIVE, "AUG sticker slot 1.", OnWeaponSkinConVarChangeCallback );
ConVar cl_aug_sticker_2( "cl_aug_sticker_2", "0", FCVAR_ARCHIVE, "AUG sticker slot 2.", OnWeaponSkinConVarChangeCallback );
ConVar cl_aug_sticker_3( "cl_aug_sticker_3", "0", FCVAR_ARCHIVE, "AUG sticker slot 3.", OnWeaponSkinConVarChangeCallback );
ConVar cl_aug_sticker_4( "cl_aug_sticker_4", "0", FCVAR_ARCHIVE, "AUG sticker slot 4.", OnWeaponSkinConVarChangeCallback );

ConVar cl_m249_skin( "cl_m249_skin", "0", FCVAR_ARCHIVE, "M249 skin selection.", OnWeaponSkinConVarChangeCallback );
ConVar cl_m249_sticker_1( "cl_m249_sticker_1", "0", FCVAR_ARCHIVE, "M249 sticker slot 1.", OnWeaponSkinConVarChangeCallback );
ConVar cl_m249_sticker_2( "cl_m249_sticker_2", "0", FCVAR_ARCHIVE, "M249 sticker slot 2.", OnWeaponSkinConVarChangeCallback );
ConVar cl_m249_sticker_3( "cl_m249_sticker_3", "0", FCVAR_ARCHIVE, "M249 sticker slot 3.", OnWeaponSkinConVarChangeCallback );
ConVar cl_m249_sticker_4( "cl_m249_sticker_4", "0", FCVAR_ARCHIVE, "M249 sticker slot 4.", OnWeaponSkinConVarChangeCallback );

extern ConVar cl_knife;
static void OnKnifeSkinConVarChangeCallback( IConVar *var, const char *pOldValue, float flOldValue );
ConVar cl_knife_skin( "cl_knife_skin", "0", FCVAR_ARCHIVE, "Knife skin selection.", OnKnifeSkinConVarChangeCallback );
extern ConVar cl_arm_glove;
extern ConVar cl_arm_glove_hardknuckle;
extern ConVar cl_arm_glove_bloodhound;
extern ConVar cl_arm_glove_sporty;
extern ConVar cl_arm_glove_slick;
extern ConVar cl_arm_glove_handwrap;
extern ConVar cl_arm_glove_moto;
extern ConVar cl_arm_glove_specialist;
extern ConVar cl_arm_glove_hydra;
extern ConVar cl_arm_glove_brokenfang;
extern ConVar cl_arm_sleeve;

static ConVar * const s_pAK47StickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_ak47_sticker_1,
	&cl_ak47_sticker_2,
	&cl_ak47_sticker_3,
	&cl_ak47_sticker_4,
};

static ConVar * const s_pAWPStickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_awp_sticker_1,
	&cl_awp_sticker_2,
	&cl_awp_sticker_3,
	&cl_awp_sticker_4,
};

static ConVar * const s_pDeagleStickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_deagle_sticker_1,
	&cl_deagle_sticker_2,
	&cl_deagle_sticker_3,
	&cl_deagle_sticker_4,
};

static ConVar * const s_pM4A1StickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_m4a1_sticker_1,
	&cl_m4a1_sticker_2,
	&cl_m4a1_sticker_3,
	&cl_m4a1_sticker_4,
};

static ConVar * const s_pGlockStickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_glock_sticker_1,
	&cl_glock_sticker_2,
	&cl_glock_sticker_3,
	&cl_glock_sticker_4,
};

static ConVar * const s_pUSPStickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_usp_sticker_1,
	&cl_usp_sticker_2,
	&cl_usp_sticker_3,
	&cl_usp_sticker_4,
};

static ConVar * const s_pMAC10StickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_mac10_sticker_1,
	&cl_mac10_sticker_2,
	&cl_mac10_sticker_3,
	&cl_mac10_sticker_4,
};

static ConVar * const s_pMP9StickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_mp9_sticker_1,
	&cl_mp9_sticker_2,
	&cl_mp9_sticker_3,
	&cl_mp9_sticker_4,
};

static ConVar * const s_pP90StickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_p90_sticker_1,
	&cl_p90_sticker_2,
	&cl_p90_sticker_3,
	&cl_p90_sticker_4,
};

static ConVar * const s_pNovaStickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_nova_sticker_1,
	&cl_nova_sticker_2,
	&cl_nova_sticker_3,
	&cl_nova_sticker_4,
};

static ConVar * const s_pXM1014StickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_xm1014_sticker_1,
	&cl_xm1014_sticker_2,
	&cl_xm1014_sticker_3,
	&cl_xm1014_sticker_4,
};

static ConVar * const s_pP228StickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_p228_sticker_1,
	&cl_p228_sticker_2,
	&cl_p228_sticker_3,
	&cl_p228_sticker_4,
};

static ConVar * const s_pEliteStickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_elite_sticker_1,
	&cl_elite_sticker_2,
	&cl_elite_sticker_3,
	&cl_elite_sticker_4,
};

static ConVar * const s_pFiveSevenStickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_fiveseven_sticker_1,
	&cl_fiveseven_sticker_2,
	&cl_fiveseven_sticker_3,
	&cl_fiveseven_sticker_4,
};

static ConVar * const s_pMP5StickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_mp5_sticker_1,
	&cl_mp5_sticker_2,
	&cl_mp5_sticker_3,
	&cl_mp5_sticker_4,
};

static ConVar * const s_pUMP45StickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_ump45_sticker_1,
	&cl_ump45_sticker_2,
	&cl_ump45_sticker_3,
	&cl_ump45_sticker_4,
};

static ConVar * const s_pGalilStickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_galil_sticker_1,
	&cl_galil_sticker_2,
	&cl_galil_sticker_3,
	&cl_galil_sticker_4,
};

static ConVar * const s_pFAMASStickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_famas_sticker_1,
	&cl_famas_sticker_2,
	&cl_famas_sticker_3,
	&cl_famas_sticker_4,
};

static ConVar * const s_pScoutStickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_scout_sticker_1,
	&cl_scout_sticker_2,
	&cl_scout_sticker_3,
	&cl_scout_sticker_4,
};

static ConVar * const s_pSG550StickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_sg550_sticker_1,
	&cl_sg550_sticker_2,
	&cl_sg550_sticker_3,
	&cl_sg550_sticker_4,
};

static ConVar * const s_pSG552StickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_sg552_sticker_1,
	&cl_sg552_sticker_2,
	&cl_sg552_sticker_3,
	&cl_sg552_sticker_4,
};

static ConVar * const s_pG3SG1StickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_g3sg1_sticker_1,
	&cl_g3sg1_sticker_2,
	&cl_g3sg1_sticker_3,
	&cl_g3sg1_sticker_4,
};

static ConVar * const s_pAUGStickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_aug_sticker_1,
	&cl_aug_sticker_2,
	&cl_aug_sticker_3,
	&cl_aug_sticker_4,
};

static ConVar * const s_pM249StickerConVars[INV_MAX_STICKER_SLOTS] =
{
	&cl_m249_sticker_1,
	&cl_m249_sticker_2,
	&cl_m249_sticker_3,
	&cl_m249_sticker_4,
};

static ConVar *GetGloveSkinConVarForFamily( const char *pszFamilyKey )
{
	if ( !pszFamilyKey )
		return NULL;
	if ( !V_stricmp( pszFamilyKey, "hardknuckle" ) ) return &cl_arm_glove_hardknuckle;
	if ( !V_stricmp( pszFamilyKey, "bloodhound" ) )  return &cl_arm_glove_bloodhound;
	if ( !V_stricmp( pszFamilyKey, "sporty" ) )      return &cl_arm_glove_sporty;
	if ( !V_stricmp( pszFamilyKey, "slick" ) )       return &cl_arm_glove_slick;
	if ( !V_stricmp( pszFamilyKey, "handwrap" ) )    return &cl_arm_glove_handwrap;
	if ( !V_stricmp( pszFamilyKey, "moto" ) )        return &cl_arm_glove_moto;
	if ( !V_stricmp( pszFamilyKey, "specialist" ) )  return &cl_arm_glove_specialist;
	if ( !V_stricmp( pszFamilyKey, "hydra" ) )       return &cl_arm_glove_hydra;
	if ( !V_stricmp( pszFamilyKey, "brokenfang" ) )  return &cl_arm_glove_brokenfang;
	return NULL;
}

static const InventoryWeaponDef_t s_WeaponCatalog[] =
{
	// Pistols
	{ "glock",     "Glock-18",     INV_CAT_PISTOLS, INV_MODE_GLOCK,     INV_TEAMMASK_T,    true,  "models/advanced_inventory_csgo_items/weapons/v_pist_glock18.mdl",   "vgui/inventory/glock/0" },
	{ "usp",       "USP-S",        INV_CAT_PISTOLS, INV_MODE_USP,       INV_TEAMMASK_CT,   true,  "models/advanced_inventory_csgo_items/weapons/v_pist_223.mdl",       "vgui/inventory/usp/0" },
	{ "p228",      "P228",         INV_CAT_PISTOLS, INV_MODE_P228,      INV_TEAMMASK_BOTH, true,  "models/advanced_inventory_csgo_items/weapons/v_pist_p250.mdl",      "vgui/inventory/p228/0" },
	{ "elite",     "Dual Elites",  INV_CAT_PISTOLS, INV_MODE_ELITE,     INV_TEAMMASK_T,    true,  "models/advanced_inventory_csgo_items/weapons/v_pist_elite.mdl",     "vgui/inventory/elite/0" },
	{ "fiveseven", "Five-SeveN",   INV_CAT_PISTOLS, INV_MODE_FIVESEVEN, INV_TEAMMASK_CT,   true,  "models/advanced_inventory_csgo_items/weapons/v_pist_fiveseven.mdl", "vgui/inventory/fiveseven/0" },
	{ "deagle",    "Desert Eagle", INV_CAT_PISTOLS, INV_MODE_DEAGLE,    INV_TEAMMASK_BOTH, true,  "models/advanced_inventory_csgo_items/weapons/v_pist_deagle.mdl",    "vgui/inventory/deagle/0" },
	// SMGs
	{ "mac10",     "MAC-10",       INV_CAT_SMGS,    INV_MODE_MAC10,     INV_TEAMMASK_T,    true,  "models/advanced_inventory_csgo_items/weapons/v_smg_mac10.mdl",      "vgui/inventory/mac10/0" },
	{ "mp9",       "MP9",          INV_CAT_SMGS,    INV_MODE_MP9,       INV_TEAMMASK_CT,   true,  "models/advanced_inventory_csgo_items/weapons/v_smg_mp9.mdl",        "vgui/inventory/mp9/0" },
	{ "mp5",       "MP5",          INV_CAT_SMGS,    INV_MODE_MP5,       INV_TEAMMASK_BOTH, true,  "models/advanced_inventory_csgo_items/weapons/v_smg_mp5sd.mdl",      "vgui/inventory/mp5/0" },
	{ "ump45",     "UMP-45",       INV_CAT_SMGS,    INV_MODE_UMP45,     INV_TEAMMASK_BOTH, true,  "models/advanced_inventory_csgo_items/weapons/v_smg_ump45.mdl",      "vgui/inventory/ump45/0" },
	{ "p90",       "P90",          INV_CAT_SMGS,    INV_MODE_P90,       INV_TEAMMASK_BOTH, true,  "models/advanced_inventory_csgo_items/weapons/v_smg_p90.mdl",        "vgui/inventory/p90/0" },
	// Rifles
	{ "galil",     "Galil",        INV_CAT_RIFLES,  INV_MODE_GALIL,     INV_TEAMMASK_T,    true,  "models/advanced_inventory_csgo_items/weapons/v_rif_galilar.mdl",    "vgui/inventory/galil/0" },
	{ "famas",     "FAMAS",        INV_CAT_RIFLES,  INV_MODE_FAMAS,     INV_TEAMMASK_CT,   true,  "models/advanced_inventory_csgo_items/weapons/v_rif_famas.mdl",      "vgui/inventory/famas/0" },
	{ "ak47",      "AK-47",        INV_CAT_RIFLES,  INV_MODE_AK47,      INV_TEAMMASK_BOTH, true,  "models/advanced_inventory_csgo_items/weapons/v_rif_ak47.mdl",       "vgui/inventory/ak47/0" },
	{ "m4a1",      "M4A1",         INV_CAT_RIFLES,  INV_MODE_M4A1,      INV_TEAMMASK_CT,   true,  "models/advanced_inventory_csgo_items/weapons/v_rif_m4a1_s.mdl",     "vgui/inventory/m4a1/0" },
	{ "scout",     "Scout",        INV_CAT_RIFLES,  INV_MODE_SCOUT,     INV_TEAMMASK_BOTH, true,  "models/advanced_inventory_csgo_items/weapons/v_snip_ssg08.mdl",     "vgui/inventory/scout/0" },
	{ "sg552",     "SG 552",       INV_CAT_RIFLES,  INV_MODE_SG552,     INV_TEAMMASK_T,    true,  "models/advanced_inventory_csgo_items/weapons/v_rif_sg556.mdl",      "vgui/inventory/sg552/0" },
	{ "aug",       "AUG",          INV_CAT_RIFLES,  INV_MODE_AUG,       INV_TEAMMASK_CT,   true,  "models/advanced_inventory_csgo_items/weapons/v_rif_aug.mdl",        "vgui/inventory/aug/0" },
	{ "sg550",     "SG 550",       INV_CAT_RIFLES,  INV_MODE_SG550,     INV_TEAMMASK_CT,   true,  "models/advanced_inventory_csgo_items/weapons/v_snip_scar20.mdl",    "vgui/inventory/sg550/0" },
	{ "g3sg1",     "G3SG1",        INV_CAT_RIFLES,  INV_MODE_G3SG1,     INV_TEAMMASK_T,    true,  "models/advanced_inventory_csgo_items/weapons/v_snip_g3sg1.mdl",     "vgui/inventory/g3sg1/0" },
	{ "awp",       "AWP",          INV_CAT_RIFLES,  INV_MODE_AWP,       INV_TEAMMASK_BOTH, true,  "models/advanced_inventory_csgo_items/weapons/v_snip_awp.mdl",       "vgui/inventory/awp/0" },
	// Heavy
	{ "nova",      "Nova",         INV_CAT_HEAVY,   INV_MODE_NOVA,      INV_TEAMMASK_BOTH, true,  "models/advanced_inventory_csgo_items/weapons/v_shot_nova.mdl",      "vgui/inventory/nova/0" },
	{ "xm1014",    "XM1014",       INV_CAT_HEAVY,   INV_MODE_XM1014,    INV_TEAMMASK_BOTH, true,  "models/advanced_inventory_csgo_items/weapons/v_shot_xm1014.mdl",    "vgui/inventory/xm1014/0" },
	{ "m249",      "M249",         INV_CAT_HEAVY,   INV_MODE_M249,      INV_TEAMMASK_BOTH, true,  "models/advanced_inventory_csgo_items/weapons/v_mach_m249para.mdl",  "vgui/inventory/m249/0" },
	// Knives, Gloves & Sleeves (all active!)
	{ "knife",     "Knives",       INV_CAT_KNIVES,  INV_MODE_KNIFE,     INV_TEAMMASK_BOTH, true,  "models/advanced_inventory_csgo_items/weapons/v_knife_karam.mdl",    "vgui/inventory/knives/karambit" },
	{ "gloves",    "Gloves",       INV_CAT_GLOVES,  INV_MODE_GLOVE,     INV_TEAMMASK_BOTH, true,  "models/advanced_inventory_csgo_items/weapons/v_knife_default_ct.mdl", "vgui/inventory/gloves/sporty_gloves" },
	{ "sleeves",   "Sleeves",      INV_CAT_SLEEVES, INV_MODE_SLEEVE,    INV_TEAMMASK_BOTH, true,  "models/advanced_inventory_csgo_items/weapons/v_knife_default_ct.mdl", "vgui/inventory/sleeves/swat" },
};

static const InventorySkinDef_t s_AK47Skins[] =
{
	{ "bloodsport",        "Bloodsport",         RARITY_COVERT },
	{ "vulcan",            "Vulcan",             RARITY_COVERT },
	{ "asiimov",           "Asiimov",            RARITY_COVERT },
	{ "redline",           "Redline",            RARITY_CLASSIFIED },
	{ "fireserpent",       "Fire Serpent",       RARITY_COVERT },
	{ "empress",           "The Empress",        RARITY_COVERT },
	{ "neon_rider",        "Neon Rider",         RARITY_COVERT },
	{ "aquamarine",        "Aquamarine Revenge", RARITY_COVERT },
	{ "fuel_injector",     "Fuel Injector",      RARITY_COVERT },
	{ "frontside_misty",   "Frontside Misty",    RARITY_CLASSIFIED },
	{ "wild_lotus",        "Wild Lotus",         RARITY_COVERT },
	{ "gold_arabesque",    "Gold Arabesque",     RARITY_COVERT },
	{ "neon_revolution",   "Neon Revolution",    RARITY_COVERT },
	{ "nightwish",         "Nightwish",          RARITY_COVERT },
	{ "head_shot",         "Head Shot",          RARITY_COVERT },
	{ "ice_coaled",        "Ice Coaled",         RARITY_CLASSIFIED },
	{ "leet_museo",        "Leet Museo",         RARITY_COVERT },
	{ "slate",             "Slate",              RARITY_RESTRICTED },
	{ "xray",              "X-Ray",              RARITY_COVERT },
	{ "anubis",            "Legion of Anubis",   RARITY_COVERT },
	{ "point_disarray",    "Point Disarray",     RARITY_CLASSIFIED },
	{ "wasteland_rebel",   "Wasteland Rebel",    RARITY_COVERT },
	{ "jaguar",            "Jaguar",             RARITY_COVERT },
	{ "panthera_onca",     "Panthera onca",      RARITY_CLASSIFIED },
	{ "cartel",            "Cartel",             RARITY_CLASSIFIED },
	{ "elite_build",       "Elite Build",        RARITY_MILSPEC },
	{ "phantom_disruptor", "Phantom Disruptor",  RARITY_CLASSIFIED },
	{ "jet_set",           "Jet Set",            RARITY_CLASSIFIED },
	{ "first_class",       "First Class",        RARITY_RESTRICTED },
	{ "emerald_pinstripe", "Emerald Pinstripe",  RARITY_RESTRICTED },
	{ "orbit_mk01",        "Orbit Mk01",         RARITY_RESTRICTED },
	{ "rat_rod",           "Rat Rod",            RARITY_RESTRICTED },
	{ "uncharted",         "Uncharted",          RARITY_MILSPEC },
	{ "steel_delta",       "Steel Delta",        RARITY_MILSPEC },
};

static const InventorySkinDef_t s_AWPSkins[] =
{
	{ "corticera",            "Corticera",            RARITY_CLASSIFIED },
	{ "redline",              "Redline",              RARITY_CLASSIFIED },
	{ "asiimov",              "Asiimov",              RARITY_COVERT },
	{ "dragon_lore",          "Dragon Lore",          RARITY_COVERT },
	{ "worm_god",             "Worm God",             RARITY_RESTRICTED },
	{ "medusa",               "Medusa",               RARITY_COVERT },
	{ "hyper_beast",          "Hyper Beast",          RARITY_COVERT },
	{ "elite_build",          "Elite Build",          RARITY_CLASSIFIED },
	{ "phobos",               "Phobos",               RARITY_RESTRICTED },
	{ "fever_dream",          "Fever Dream",          RARITY_CLASSIFIED },
	{ "oni_taiji",            "Oni Taiji",            RARITY_COVERT },
	{ "mortis",               "Mortis",               RARITY_CLASSIFIED },
	{ "the_prince",           "The Prince",           RARITY_COVERT },
	{ "gungnir",              "Gungnir",              RARITY_COVERT },
	{ "neo_noir",             "Neo-Noir",             RARITY_COVERT },
	{ "atheris",              "Atheris",              RARITY_RESTRICTED },
	{ "containment_breach",   "Containment Breach",   RARITY_COVERT },
	{ "wildfire",             "Wildfire",             RARITY_COVERT },
	{ "desert_hydra",         "Desert Hydra",         RARITY_COVERT },
	{ "capillary",            "Capillary",            RARITY_RESTRICTED },
	{ "exoskeleton",          "Exoskeleton",          RARITY_RESTRICTED },
	{ "black_nile",           "Black Nile",           RARITY_RESTRICTED },
	{ "chromatic_aberration", "Chromatic Aberration", RARITY_COVERT },
	{ "duality",              "Duality",              RARITY_CLASSIFIED },
};

static const InventorySkinDef_t s_DeagleSkins[] =
{
	{ "heirloom",           "Heirloom",           RARITY_RESTRICTED },
	{ "hand_cannon",        "Hand Cannon",        RARITY_RESTRICTED },
	{ "pilot",              "Pilot",              RARITY_RESTRICTED },
	{ "conspiracy",         "Conspiracy",         RARITY_CLASSIFIED },
	{ "naga",               "Naga",               RARITY_RESTRICTED },
	{ "corinthian",         "Corinthian",         RARITY_RESTRICTED },
	{ "kumicho_dragon",     "Kumicho Dragon",     RARITY_CLASSIFIED },
	{ "directive",          "Directive",          RARITY_RESTRICTED },
	{ "oxide_blaze",        "Oxide Blaze",        RARITY_RESTRICTED },
	{ "code_red",           "Code Red",           RARITY_COVERT },
	{ "mecha_industries",   "Mecha Industries",   RARITY_CLASSIFIED },
	{ "light_rail",         "Light Rail",         RARITY_RESTRICTED },
	{ "fennec_fox",         "Fennec Fox",         RARITY_CLASSIFIED },
	{ "blue_ply",           "Blue Ply",           RARITY_RESTRICTED },
	{ "printstream",        "Printstream",        RARITY_COVERT },
	{ "trigger_discipline", "Trigger Discipline", RARITY_RESTRICTED },
	{ "ocean_drive",        "Ocean Drive",        RARITY_COVERT },
};

static const InventorySkinDef_t s_M4A1Skins[] =
{
	{ "guardian",         "Guardian",              RARITY_CLASSIFIED },
	{ "master_piece",     "Master Piece",          RARITY_CLASSIFIED },
	{ "cyrex",            "Cyrex",                 RARITY_COVERT },
	{ "basilisk",         "Basilisk",              RARITY_RESTRICTED },
	{ "hyper_beast",      "Hyper Beast",           RARITY_COVERT },
	{ "golden_coil",      "Golden Coil",           RARITY_COVERT },
	{ "control_panel",    "Control Panel",         RARITY_CLASSIFIED },
	{ "chanticos_fire",   "Chantico's Fire",       RARITY_COVERT },
	{ "mecha_industries", "Mecha Industries",      RARITY_COVERT },
	{ "flashback",        "Flashback",             RARITY_RESTRICTED },
	{ "decimator",        "Decimator",             RARITY_CLASSIFIED },
	{ "briefing",         "Briefing",              RARITY_RESTRICTED },
	{ "leaded_glass",     "Leaded Glass",          RARITY_CLASSIFIED },
	{ "nightmare",        "Nightmare",             RARITY_CLASSIFIED },
	{ "player_two",       "Player Two",            RARITY_COVERT },
	{ "printstream",      "Printstream",           RARITY_COVERT },
	{ "welcome_jungle",   "Welcome to the Jungle", RARITY_COVERT },
	{ "imminent_danger",  "Imminent Danger",       RARITY_COVERT },
	{ "mud_spec",         "Mud-Spec",              RARITY_MILSPEC },
	{ "night_terror",     "Night Terror",          RARITY_RESTRICTED },
	{ "emphorosaur_s",    "Emphorosaur-S",         RARITY_RESTRICTED },
};

static const InventorySkinDef_t s_GlockSkins[] =
{
	{ "brass",          "Brass",          RARITY_RESTRICTED },
	{ "water_elemental","Water Elemental",RARITY_CLASSIFIED },
	{ "grinder",        "Grinder",        RARITY_RESTRICTED },
	{ "bunsen_burner",  "Bunsen Burner",  RARITY_RESTRICTED },
	{ "wraiths",        "Wraiths",        RARITY_RESTRICTED },
	{ "royal_legion",   "Royal Legion",   RARITY_RESTRICTED },
	{ "wasteland_rebel","Wasteland Rebel",RARITY_COVERT },
	{ "weasel",         "Weasel",         RARITY_RESTRICTED },
	{ "ironwork",       "Ironwork",       RARITY_RESTRICTED },
	{ "off_world",      "Off World",      RARITY_RESTRICTED },
	{ "warhawk",        "Warhawk",        RARITY_RESTRICTED },
	{ "oxide_blaze",    "Oxide Blaze",    RARITY_RESTRICTED },
	{ "sacrifice",      "Sacrifice",      RARITY_RESTRICTED },
	{ "bullet_queen",   "Bullet Queen",   RARITY_COVERT },
	{ "vogue",          "Vogue",          RARITY_CLASSIFIED },
	{ "neo_noir",       "Neo-Noir",       RARITY_COVERT },
	{ "franklin",       "Franklin",       RARITY_RESTRICTED },
	{ "clear_polymer",  "Clear Polymer",  RARITY_RESTRICTED },
	{ "ramese_s_reach", "Ramese's Reach", RARITY_RESTRICTED },
	{ "snack_attack",   "Snack Attack",   RARITY_CLASSIFIED },
	{ "winterized",     "Winterized",     RARITY_RESTRICTED },
	{ "umbral_rabbit",  "Umbral Rabbit",  RARITY_RESTRICTED },
};

static const InventorySkinDef_t s_USPSkins[] =
{
	{ "stainless",      "Stainless",      RARITY_RESTRICTED },
	{ "guardian",       "Guardian",       RARITY_RESTRICTED },
	{ "orion",          "Orion",          RARITY_RESTRICTED },
	{ "road_rash",      "Road Rash",      RARITY_RESTRICTED },
	{ "caiman",         "Caiman",         RARITY_CLASSIFIED },
	{ "business_class", "Business Class", RARITY_RESTRICTED },
	{ "torque",         "Torque",         RARITY_RESTRICTED },
	{ "kill_confirmed", "Kill Confirmed", RARITY_COVERT },
	{ "lead_conduit",   "Lead Conduit",   RARITY_RESTRICTED },
	{ "cyrex",          "Cyrex",          RARITY_RESTRICTED },
	{ "neo_noir",       "Neo-Noir",       RARITY_COVERT },
	{ "blueprint",      "Blueprint",      RARITY_RESTRICTED },
	{ "cortex",         "Cortex",         RARITY_CLASSIFIED },
	{ "flashback",      "Flashback",      RARITY_RESTRICTED },
	{ "monster_mashup", "Monster Mashup", RARITY_CLASSIFIED },
	{ "the_traitor",    "The Traitor",    RARITY_COVERT },
	{ "desert_tactical","Desert Tactical",RARITY_MILSPEC },
	{ "black_lotus",    "Black Lotus",    RARITY_RESTRICTED },
	{ "ticket_to_hell", "Ticket to Hell", RARITY_RESTRICTED },
	{ "printstream",    "Printstream",    RARITY_COVERT },
};

static const InventorySkinDef_t s_MAC10Skins[] =
{
	{ "case_hardened",  "Case Hardened",  RARITY_RESTRICTED },
	{ "graven",         "Graven",         RARITY_RESTRICTED },
	{ "heat",           "Heat",           RARITY_RESTRICTED },
	{ "curse",          "Curse",          RARITY_RESTRICTED },
	{ "tatter",         "Tatter",         RARITY_RESTRICTED },
	{ "commuter",       "Commuter",       RARITY_MILSPEC },
	{ "neon_rider",     "Neon Rider",     RARITY_COVERT },
	{ "rangeen",        "Rangeen",        RARITY_RESTRICTED },
	{ "calf_skin",      "Calf Skin",      RARITY_MILSPEC },
	{ "carnivore",      "Carnivore",      RARITY_RESTRICTED },
	{ "pipe_down",      "Pipe Down",      RARITY_RESTRICTED },
	{ "whitefish",      "Whitefish",      RARITY_RESTRICTED },
	{ "stalker",        "Stalker",        RARITY_COVERT },
	{ "classic_crate",  "Classic Crate",  RARITY_RESTRICTED },
	{ "disco_tech",     "Disco Tech",     RARITY_CLASSIFIED },
	{ "allure",         "Allure",         RARITY_RESTRICTED },
	{ "hot_snakes",     "Hot Snakes",     RARITY_CLASSIFIED },
	{ "button_masher",  "Button Masher",  RARITY_RESTRICTED },
	{ "propaganda",     "Propaganda",     RARITY_CLASSIFIED },
	{ "echoing_sands",  "Echoing Sands",  RARITY_MILSPEC },
	{ "toybox",         "Toybox",         RARITY_CLASSIFIED },
	{ "ensnared",       "Ensnared",       RARITY_RESTRICTED },
	{ "monkeyflage",    "Monkeyflage",    RARITY_RESTRICTED },
	{ "sakkaku",        "Sakkaku",        RARITY_RESTRICTED },
};

static const InventorySkinDef_t s_MP9Skins[] =
{
	{ "dart",                "Dart",                RARITY_RESTRICTED },
	{ "deadly_poison",       "Deadly Poison",       RARITY_RESTRICTED },
	{ "airlock",             "Airlock",             RARITY_CLASSIFIED },
	{ "goo",                 "Goo",                 RARITY_RESTRICTED },
	{ "black_sand",          "Black Sand",          RARITY_RESTRICTED },
	{ "capillary",           "Capillary",           RARITY_RESTRICTED },
	{ "wild_lily",           "Wild Lily",           RARITY_CLASSIFIED },
	{ "modest_threat",       "Modest Threat",       RARITY_RESTRICTED },
	{ "hydra",               "Hydra",               RARITY_CLASSIFIED },
	{ "food_chain",          "Food Chain",          RARITY_CLASSIFIED },
	{ "starlight_protector", "Starlight Protector", RARITY_COVERT },
	{ "featherweight",       "Featherweight",       RARITY_RESTRICTED },
};

static const InventorySkinDef_t s_P90Skins[] =
{
	{ "death_by_kitty", "Death by Kitty", RARITY_COVERT },
	{ "emerald_dragon", "Emerald Dragon", RARITY_CLASSIFIED },
	{ "trigon",         "Trigon",         RARITY_CLASSIFIED },
	{ "desert_warfare", "Desert Warfare", RARITY_RESTRICTED },
	{ "module",         "Module",         RARITY_RESTRICTED },
	{ "leather",        "Leather",        RARITY_MILSPEC },
	{ "asiimov",        "Asiimov",        RARITY_COVERT },
	{ "elite_build",    "Elite Build",    RARITY_RESTRICTED },
	{ "shapewood",      "Shapewood",      RARITY_CLASSIFIED },
	{ "chopper",        "Chopper",        RARITY_RESTRICTED },
	{ "grim",           "Grim",           RARITY_RESTRICTED },
	{ "shallow_grave",  "Shallow Grave",  RARITY_CLASSIFIED },
	{ "traction",       "Traction",       RARITY_RESTRICTED },
	{ "off_world",      "Off World",      RARITY_RESTRICTED },
	{ "nostalgia",      "Nostalgia",      RARITY_CLASSIFIED },
	{ "freight",        "Freight",        RARITY_RESTRICTED },
	{ "run_and_hide",   "Run and Hide",   RARITY_CLASSIFIED },
	{ "scarab_rush",    "ScaraB Rush",    RARITY_RESTRICTED },
	{ "vent_rush",      "Vent Rush",      RARITY_RESTRICTED },
	{ "neoqueen",       "Neoqueen",       RARITY_RESTRICTED },
};

static const InventorySkinDef_t s_P228Skins[] =
{
	{ "gunsmoke", "Gunsmoke", RARITY_MILSPEC },
	{ "bone_mask", "Bone Mask", RARITY_MILSPEC },
	{ "metallic_ddpat", "Metallic DDPAT", RARITY_MILSPEC },
	{ "boreal_forest", "Boreal Forest", RARITY_MILSPEC },
	{ "forest_night", "Forest Night", RARITY_MILSPEC },
	{ "sand_dune", "Sand Dune", RARITY_MILSPEC },
	{ "whiteout", "Whiteout", RARITY_RESTRICTED },
	{ "splash", "Splash", RARITY_RESTRICTED },
	{ "modern_hunter", "Modern Hunter", RARITY_RESTRICTED },
	{ "nuclear_threat", "Nuclear Threat", RARITY_RESTRICTED },
	{ "facets", "Facets", RARITY_MILSPEC },
	{ "hive", "Hive", RARITY_RESTRICTED },
	{ "steel_disruption", "Steel Disruption", RARITY_RESTRICTED },
	{ "mehndi", "Mehndi", RARITY_CLASSIFIED },
	{ "undertow", "Undertow", RARITY_CLASSIFIED },
	{ "franklin", "Franklin", RARITY_CLASSIFIED },
	{ "supernova", "Supernova", RARITY_RESTRICTED },
	{ "contamination", "Contamination", RARITY_MILSPEC },
	{ "cartel", "Cartel", RARITY_CLASSIFIED },
	{ "muertos", "Muertos", RARITY_CLASSIFIED },
	{ "valence", "Valence", RARITY_RESTRICTED },
	{ "crimson_kimono", "Crimson Kimono", RARITY_MILSPEC },
	{ "mint_kimono", "Mint Kimono", RARITY_MILSPEC },
	{ "wingshot", "Wingshot", RARITY_RESTRICTED },
	{ "facility_draft", "Facility Draft", RARITY_MILSPEC },
	{ "exchanger", "Exchanger", RARITY_RESTRICTED },
	{ "vino_primo", "Vino Primo", RARITY_RESTRICTED },
	{ "x_ray", "X-Ray", RARITY_RESTRICTED },
	{ "asiimov", "Asiimov", RARITY_CLASSIFIED },
	{ "iron_clad", "Iron Clad", RARITY_RESTRICTED },
	{ "ripple", "Ripple", RARITY_RESTRICTED },
	{ "red_rock", "Red Rock", RARITY_RESTRICTED },
	{ "see_ya_later", "See Ya Later", RARITY_COVERT },
	{ "dark_filigree", "Dark Filigree", RARITY_MILSPEC },
	{ "nevermore", "Nevermore", RARITY_RESTRICTED },
	{ "verdigris", "Verdigris", RARITY_RESTRICTED },
	{ "inferno", "Inferno", RARITY_RESTRICTED },
	{ "black_tan", "Black & Tan", RARITY_RESTRICTED },
	{ "drought", "Drought", RARITY_MILSPEC },
	{ "cassette", "Cassette", RARITY_RESTRICTED },
	{ "contaminant", "Contaminant", RARITY_RESTRICTED },
	{ "bengal_tiger", "Bengal Tiger", RARITY_RESTRICTED },
	{ "cyber_shell", "Cyber Shell", RARITY_RESTRICTED },
	{ "digital_architect", "Digital Architect", RARITY_RESTRICTED },
	{ "apep_s_curse", "Apep's Curse", RARITY_CLASSIFIED },
	{ "visions", "Visions", RARITY_CLASSIFIED },
	{ "re_built", "Re.built", RARITY_RESTRICTED },
};

static const InventorySkinDef_t s_EliteSkins[] =
{
	{ "anodized_navy", "Anodized Navy", RARITY_RESTRICTED },
	{ "stained", "Stained", RARITY_MILSPEC },
	{ "contractor", "Contractor", RARITY_MILSPEC },
	{ "colony", "Colony", RARITY_MILSPEC },
	{ "demolition", "Demolition", RARITY_RESTRICTED },
	{ "black_limba", "Black Limba", RARITY_RESTRICTED },
	{ "hemoglobin", "Hemoglobin", RARITY_RESTRICTED },
	{ "cobalt_quartz", "Cobalt Quartz", RARITY_RESTRICTED },
	{ "marina", "Marina", RARITY_RESTRICTED },
	{ "panther", "Panther", RARITY_RESTRICTED },
	{ "retribution", "Retribution", RARITY_RESTRICTED },
	{ "briar", "Briar", RARITY_MILSPEC },
	{ "urban_shock", "Urban Shock", RARITY_RESTRICTED },
	{ "duelist", "Duelist", RARITY_RESTRICTED },
	{ "moon_in_libra", "Moon in Libra", RARITY_MILSPEC },
	{ "emerald", "Emerald", RARITY_RESTRICTED },
	{ "dualing_dragons", "Dualing Dragons", RARITY_RESTRICTED },
	{ "cartel", "Cartel", RARITY_RESTRICTED },
	{ "twin_turbo", "Twin Turbo", RARITY_CLASSIFIED },
	{ "ventilators", "Ventilators", RARITY_RESTRICTED },
	{ "royal_consorts", "Royal Consorts", RARITY_RESTRICTED },
	{ "cobra_strike", "Cobra Strike", RARITY_CLASSIFIED },
	{ "shred", "Shred", RARITY_RESTRICTED },
	{ "pyre", "Pyre", RARITY_MILSPEC },
	{ "balance", "Balance", RARITY_RESTRICTED },
	{ "elite_1_6", "Elite 1.6", RARITY_RESTRICTED },
	{ "drift_wood", "Drift Wood", RARITY_MILSPEC },
	{ "dezastre", "Dezastre", RARITY_RESTRICTED },
	{ "switch_board", "Switch Board", RARITY_MILSPEC },
	{ "heist", "Heist", RARITY_MILSPEC },
	{ "oil_change", "Oil Change", RARITY_MILSPEC },
	{ "tread", "Tread", RARITY_RESTRICTED },
	{ "melondrama", "Melondrama", RARITY_CLASSIFIED },
	{ "flora_carnivora", "Flora Carnivora", RARITY_RESTRICTED },
};

static const InventorySkinDef_t s_FiveSevenSkins[] =
{
	{ "candy_apple", "Candy Apple", RARITY_MILSPEC },
	{ "case_hardened", "Case Hardened", RARITY_RESTRICTED },
	{ "contractor", "Contractor", RARITY_MILSPEC },
	{ "forest_night", "Forest Night", RARITY_MILSPEC },
	{ "orange_peel", "Orange Peel", RARITY_MILSPEC },
	{ "jungle", "Jungle", RARITY_MILSPEC },
	{ "anodized_gunmetal", "Anodized Gunmetal", RARITY_MILSPEC },
	{ "nightshade", "Nightshade", RARITY_RESTRICTED },
	{ "silver_quartz", "Silver Quartz", RARITY_RESTRICTED },
	{ "nitro", "Nitro", RARITY_RESTRICTED },
	{ "kami", "Kami", RARITY_RESTRICTED },
	{ "copper_galaxy", "Copper Galaxy", RARITY_RESTRICTED },
	{ "fowl_play", "Fowl Play", RARITY_CLASSIFIED },
	{ "hot_shot", "Hot Shot", RARITY_MILSPEC },
	{ "urban_hazard", "Urban Hazard", RARITY_RESTRICTED },
	{ "monkey_business", "Monkey Business", RARITY_CLASSIFIED },
	{ "neon_kimono", "Neon Kimono", RARITY_RESTRICTED },
	{ "retrobution", "Retrobution", RARITY_RESTRICTED },
	{ "triumvirate", "Triumvirate", RARITY_RESTRICTED },
	{ "coolant", "Coolant", RARITY_MILSPEC },
	{ "violent_daimyo", "Violent Daimyo", RARITY_RESTRICTED },
	{ "scumbria", "Scumbria", RARITY_RESTRICTED },
	{ "capillary", "Capillary", RARITY_RESTRICTED },
	{ "hyper_beast", "Hyper Beast", RARITY_COVERT },
	{ "flame_test", "Flame Test", RARITY_RESTRICTED },
	{ "crimson_blossom", "Crimson Blossom", RARITY_RESTRICTED },
	{ "angry_mob", "Angry Mob", RARITY_COVERT },
	{ "buddy", "Buddy", RARITY_RESTRICTED },
	{ "withered_vine", "Withered Vine", RARITY_MILSPEC },
	{ "fairy_tale", "Fairy Tale", RARITY_CLASSIFIED },
	{ "berries_and_cherries", "Berries And Cherries", RARITY_RESTRICTED },
	{ "fall_hazard", "Fall Hazard", RARITY_CLASSIFIED },
	{ "boost_protocol", "Boost Protocol", RARITY_RESTRICTED },
	{ "scrawl", "Scrawl", RARITY_RESTRICTED },
};

static const InventorySkinDef_t s_MP5Skins[] =
{
	{ "co_processor", "Co-Processor", RARITY_RESTRICTED },
	{ "dirt_drop", "Dirt Drop", RARITY_MILSPEC },
	{ "nitro", "Nitro", RARITY_MILSPEC },
	{ "lab_rats", "Lab Rats", RARITY_RESTRICTED },
	{ "bamboo_garden", "Bamboo Garden", RARITY_MILSPEC },
	{ "phosphor", "Phosphor", RARITY_CLASSIFIED },
	{ "gauss", "Gauss", RARITY_RESTRICTED },
	{ "acid_wash", "Acid Wash", RARITY_RESTRICTED },
	{ "agent", "Agent", RARITY_RESTRICTED },
	{ "oxide_oasis", "Oxide Oasis", RARITY_CLASSIFIED },
	{ "desert_strike", "Desert Strike", RARITY_RESTRICTED },
	{ "kitbash", "Kitbash", RARITY_RESTRICTED },
	{ "condition_zero", "Condition Zero", RARITY_RESTRICTED },
	{ "autumn_twilly", "Autumn Twilly", RARITY_RESTRICTED },
	{ "necro_jr", "Necro Jr.", RARITY_RESTRICTED },
	{ "liquidation", "Liquidation", RARITY_RESTRICTED },
};

static const InventorySkinDef_t s_UMP45Skins[] =
{
	{ "gunsmoke", "Gunsmoke", RARITY_MILSPEC },
	{ "urban_ddpat", "Urban DDPAT", RARITY_MILSPEC },
	{ "blaze", "Blaze", RARITY_RESTRICTED },
	{ "carbon_fiber", "Carbon Fiber", RARITY_MILSPEC },
	{ "mudder", "Mudder", RARITY_MILSPEC },
	{ "caramel", "Caramel", RARITY_MILSPEC },
	{ "fallout_warning", "Fallout Warning", RARITY_MILSPEC },
	{ "scorched", "Scorched", RARITY_MILSPEC },
	{ "bone_pile", "Bone Pile", RARITY_RESTRICTED },
	{ "full_stop", "Full Stop", RARITY_RESTRICTED },
	{ "corporal", "Corporal", RARITY_RESTRICTED },
	{ "indigo", "Indigo", RARITY_MILSPEC },
	{ "labyrinth", "Labyrinth", RARITY_RESTRICTED },
	{ "delusion", "Delusion", RARITY_RESTRICTED },
	{ "grand_prix", "Grand Prix", RARITY_RESTRICTED },
	{ "minotaur_s_labyrinth", "Minotaur's Labyrinth", RARITY_RESTRICTED },
	{ "riot", "Riot", RARITY_RESTRICTED },
	{ "facility_dark", "Facility Dark", RARITY_MILSPEC },
	{ "primal_saber", "Primal Saber", RARITY_CLASSIFIED },
	{ "briefing", "Briefing", RARITY_RESTRICTED },
	{ "scaffold", "Scaffold", RARITY_RESTRICTED },
	{ "metal_flowers", "Metal Flowers", RARITY_RESTRICTED },
	{ "exposure", "Exposure", RARITY_RESTRICTED },
	{ "arctic_wolf", "Arctic Wolf", RARITY_RESTRICTED },
	{ "day_lily", "Day Lily", RARITY_RESTRICTED },
	{ "momentum", "Momentum", RARITY_CLASSIFIED },
	{ "moonrise", "Moonrise", RARITY_RESTRICTED },
	{ "plastique", "Plastique", RARITY_RESTRICTED },
	{ "fade", "Fade", RARITY_CLASSIFIED },
	{ "gold_bismuth", "Gold Bismuth", RARITY_RESTRICTED },
	{ "crime_scene", "Crime Scene", RARITY_RESTRICTED },
	{ "houndstooth", "Houndstooth", RARITY_MILSPEC },
	{ "oscillator", "Oscillator", RARITY_RESTRICTED },
	{ "mechanism", "Mechanism", RARITY_MILSPEC },
	{ "roadblock", "Roadblock", RARITY_RESTRICTED },
	{ "wild_child", "Wild Child", RARITY_CLASSIFIED },
};

static const InventorySkinDef_t s_GalilSkins[] =
{
	{ "winter_forest", "Winter Forest", RARITY_MILSPEC },
	{ "orange_ddpat", "Orange DDPAT", RARITY_RESTRICTED },
	{ "tornado", "Tornado", RARITY_MILSPEC },
	{ "sage_spray", "Sage Spray", RARITY_MILSPEC },
	{ "shattered", "Shattered", RARITY_RESTRICTED },
	{ "blue_titanium", "Blue Titanium", RARITY_RESTRICTED },
	{ "varicamo", "VariCamo", RARITY_MILSPEC },
	{ "urban_rubble", "Urban Rubble", RARITY_MILSPEC },
	{ "hunting_blind", "Hunting Blind", RARITY_MILSPEC },
	{ "amber_fade", "Amber Fade", RARITY_RESTRICTED },
	{ "sandstorm", "Sandstorm", RARITY_RESTRICTED },
	{ "tuxedo", "Tuxedo", RARITY_RESTRICTED },
	{ "kami", "Kami", RARITY_RESTRICTED },
	{ "cerberus", "Cerberus", RARITY_RESTRICTED },
	{ "chatterbox", "Chatterbox", RARITY_COVERT },
	{ "eco", "Eco", RARITY_CLASSIFIED },
	{ "aqua_terrace", "Aqua Terrace", RARITY_RESTRICTED },
	{ "rocket_pop", "Rocket Pop", RARITY_RESTRICTED },
	{ "stone_cold", "Stone Cold", RARITY_RESTRICTED },
	{ "cold_fusion", "Cold Fusion", RARITY_MILSPEC },
	{ "firefight", "Firefight", RARITY_RESTRICTED },
	{ "black_sand", "Black Sand", RARITY_RESTRICTED },
	{ "crimson_tsunami", "Crimson Tsunami", RARITY_RESTRICTED },
	{ "sugar_rush", "Sugar Rush", RARITY_CLASSIFIED },
	{ "signal", "Signal", RARITY_RESTRICTED },
	{ "akoben", "Akoben", RARITY_RESTRICTED },
	{ "connexion", "Connexion", RARITY_RESTRICTED },
	{ "vandal", "Vandal", RARITY_RESTRICTED },
	{ "phoenix_blacklight", "Phoenix Blacklight", RARITY_RESTRICTED },
	{ "dusk_ruins", "Dusk Ruins", RARITY_RESTRICTED },
	{ "chromatic_aberration", "Chromatic Aberration", RARITY_CLASSIFIED },
	{ "caution", "CAUTION!", RARITY_RESTRICTED },
	{ "destroyer", "Destroyer", RARITY_RESTRICTED },
};

static const InventorySkinDef_t s_FAMASSkins[] =
{
	{ "contrast_spray", "Contrast Spray", RARITY_MILSPEC },
	{ "colony", "Colony", RARITY_MILSPEC },
	{ "dark_water", "Dark Water", RARITY_RESTRICTED },
	{ "cyanospatter", "Cyanospatter", RARITY_MILSPEC },
	{ "afterimage", "Afterimage", RARITY_CLASSIFIED },
	{ "doomkitty", "Doomkitty", RARITY_RESTRICTED },
	{ "spitfire", "Spitfire", RARITY_RESTRICTED },
	{ "hexane", "Hexane", RARITY_RESTRICTED },
	{ "calicamo", "CaliCamo", RARITY_MILSPEC },
	{ "teardown", "Teardown", RARITY_RESTRICTED },
	{ "pulse", "Pulse", RARITY_RESTRICTED },
	{ "sergeant", "Sergeant", RARITY_RESTRICTED },
	{ "styx", "Styx", RARITY_RESTRICTED },
	{ "djinn", "Djinn", RARITY_CLASSIFIED },
	{ "neural_net", "Neural Net", RARITY_RESTRICTED },
	{ "survivor_z", "Survivor Z", RARITY_RESTRICTED },
	{ "valence", "Valence", RARITY_RESTRICTED },
	{ "roll_cage", "Roll Cage", RARITY_COVERT },
	{ "mecha_industries", "Mecha Industries", RARITY_CLASSIFIED },
	{ "macabre", "Macabre", RARITY_RESTRICTED },
	{ "eye_of_athena", "Eye of Athena", RARITY_CLASSIFIED },
	{ "sundown", "Sundown", RARITY_RESTRICTED },
	{ "night_borre", "Night Borre", RARITY_MILSPEC },
	{ "crypsis", "Crypsis", RARITY_RESTRICTED },
	{ "decommissioned", "Decommissioned", RARITY_RESTRICTED },
	{ "commemoration", "Commemoration", RARITY_COVERT },
	{ "prime_conspiracy", "Prime Conspiracy", RARITY_RESTRICTED },
	{ "meltdown", "Meltdown", RARITY_CLASSIFIED },
	{ "faulty_wiring", "Faulty Wiring", RARITY_MILSPEC },
	{ "waters_of_nephthys", "Waters of Nephthys", RARITY_CLASSIFIED },
	{ "zx_spectron", "ZX Spectron", RARITY_RESTRICTED },
	{ "rapid_eye_movement", "Rapid Eye Movement", RARITY_CLASSIFIED },
	{ "meow_36", "Meow 36", RARITY_RESTRICTED },
};

static const InventorySkinDef_t s_ScoutSkins[] =
{
	{ "lichen_dashed", "Lichen Dashed", RARITY_MILSPEC },
	{ "dark_water", "Dark Water", RARITY_RESTRICTED },
	{ "carbon_fiber", "Carbon Fiber", RARITY_MILSPEC },
	{ "blue_spruce", "Blue Spruce", RARITY_MILSPEC },
	{ "sand_dune", "Sand Dune", RARITY_MILSPEC },
	{ "jungle_dashed", "Jungle Dashed", RARITY_MILSPEC },
	{ "mayan_dreams", "Mayan Dreams", RARITY_MILSPEC },
	{ "blood_in_the_water", "Blood in the Water", RARITY_COVERT },
	{ "tropical_storm", "Tropical Storm", RARITY_MILSPEC },
	{ "acid_fade", "Acid Fade", RARITY_RESTRICTED },
	{ "slashed", "Slashed", RARITY_RESTRICTED },
	{ "detour", "Detour", RARITY_RESTRICTED },
	{ "abyss", "Abyss", RARITY_RESTRICTED },
	{ "big_iron", "Big Iron", RARITY_CLASSIFIED },
	{ "necropos", "Necropos", RARITY_RESTRICTED },
	{ "hand_brake", "Hand Brake", RARITY_RESTRICTED },
	{ "ghost_crusader", "Ghost Crusader", RARITY_RESTRICTED },
	{ "dragonfire", "Dragonfire", RARITY_COVERT },
	{ "death_s_head", "Death's Head", RARITY_RESTRICTED },
	{ "sea_calico", "Sea Calico", RARITY_RESTRICTED },
	{ "orange_filigree", "Orange Filigree", RARITY_RESTRICTED },
	{ "red_stone", "Red Stone", RARITY_MILSPEC },
	{ "bloodshot", "Bloodshot", RARITY_CLASSIFIED },
	{ "prey", "Prey", RARITY_MILSPEC },
	{ "death_strike", "Death Strike", RARITY_CLASSIFIED },
	{ "fever_dream", "Fever Dream", RARITY_RESTRICTED },
	{ "mainframe_001", "Mainframe 001", RARITY_RESTRICTED },
	{ "parallax", "Parallax", RARITY_RESTRICTED },
	{ "threat_detected", "Threat Detected", RARITY_RESTRICTED },
	{ "spring_twilly", "Spring Twilly", RARITY_RESTRICTED },
	{ "azure_glyph", "Azure Glyph", RARITY_MILSPEC },
	{ "turbo_peek", "Turbo Peek", RARITY_CLASSIFIED },
};

static const InventorySkinDef_t s_SG550Skins[] =
{
	{ "contractor", "Contractor", RARITY_MILSPEC },
	{ "carbon_fiber", "Carbon Fiber", RARITY_MILSPEC },
	{ "storm", "Storm", RARITY_MILSPEC },
	{ "sand_mesh", "Sand Mesh", RARITY_MILSPEC },
	{ "palm", "Palm", RARITY_MILSPEC },
	{ "brass", "Brass", RARITY_RESTRICTED },
	{ "splash_jam", "Splash Jam", RARITY_CLASSIFIED },
	{ "emerald", "Emerald", RARITY_RESTRICTED },
	{ "crimson_web", "Crimson Web", RARITY_RESTRICTED },
	{ "army_sheen", "Army Sheen", RARITY_MILSPEC },
	{ "cyrex", "Cyrex", RARITY_CLASSIFIED },
	{ "cardiac", "Cardiac", RARITY_CLASSIFIED },
	{ "grotto", "Grotto", RARITY_RESTRICTED },
	{ "green_marine", "Green Marine", RARITY_RESTRICTED },
	{ "outbreak", "Outbreak", RARITY_RESTRICTED },
	{ "bloodsport", "Bloodsport", RARITY_CLASSIFIED },
	{ "powercore", "Powercore", RARITY_RESTRICTED },
	{ "blueprint", "Blueprint", RARITY_RESTRICTED },
	{ "jungle_slipstream", "Jungle Slipstream", RARITY_RESTRICTED },
	{ "stone_mosaico", "Stone Mosaico", RARITY_MILSPEC },
	{ "torn", "Torn", RARITY_RESTRICTED },
	{ "assault", "Assault", RARITY_RESTRICTED },
	{ "enforcer", "Enforcer", RARITY_RESTRICTED },
	{ "magna_carta", "Magna Carta", RARITY_RESTRICTED },
	{ "poultrygeist", "Poultrygeist", RARITY_RESTRICTED },
	{ "fragments", "Fragments", RARITY_RESTRICTED },
};

static const InventorySkinDef_t s_SG552Skins[] =
{
	{ "anodized_navy", "Anodized Navy", RARITY_RESTRICTED },
	{ "bulldozer", "Bulldozer", RARITY_RESTRICTED },
	{ "hypnotic", "Hypnotic", RARITY_RESTRICTED },
	{ "ultraviolet", "Ultraviolet", RARITY_RESTRICTED },
	{ "tornado", "Tornado", RARITY_MILSPEC },
	{ "waves_perforated", "Waves Perforated", RARITY_MILSPEC },
	{ "wave_spray", "Wave Spray", RARITY_RESTRICTED },
	{ "gator_mesh", "Gator Mesh", RARITY_MILSPEC },
	{ "damascus_steel", "Damascus Steel", RARITY_RESTRICTED },
	{ "pulse", "Pulse", RARITY_RESTRICTED },
	{ "army_sheen", "Army Sheen", RARITY_MILSPEC },
	{ "traveler", "Traveler", RARITY_MILSPEC },
	{ "fallout_warning", "Fallout Warning", RARITY_MILSPEC },
	{ "cyrex", "Cyrex", RARITY_CLASSIFIED },
	{ "tiger_moth", "Tiger Moth", RARITY_RESTRICTED },
	{ "integrale", "Integrale", RARITY_CLASSIFIED },
	{ "atlas", "Atlas", RARITY_RESTRICTED },
	{ "aerial", "Aerial", RARITY_RESTRICTED },
	{ "triarch", "Triarch", RARITY_RESTRICTED },
	{ "phantom", "Phantom", RARITY_RESTRICTED },
	{ "aloha", "Aloha", RARITY_RESTRICTED },
	{ "candy_apple", "Candy Apple", RARITY_MILSPEC },
	{ "barricade", "Barricade", RARITY_MILSPEC },
	{ "danger_close", "Danger Close", RARITY_RESTRICTED },
	{ "colony_iv", "Colony IV", RARITY_CLASSIFIED },
	{ "bleached", "Bleached", RARITY_MILSPEC },
	{ "desert_blossom", "Desert Blossom", RARITY_RESTRICTED },
	{ "darkwing", "Darkwing", RARITY_RESTRICTED },
	{ "ol_rusty", "Ol' Rusty", RARITY_RESTRICTED },
	{ "lush_ruins", "Lush Ruins", RARITY_MILSPEC },
	{ "heavy_metal", "Heavy Metal", RARITY_RESTRICTED },
	{ "hazard_pay", "Hazard Pay", RARITY_CLASSIFIED },
	{ "dragon_tech", "Dragon Tech", RARITY_RESTRICTED },
	{ "cyberforce", "Cyberforce", RARITY_RESTRICTED },
};

static const InventorySkinDef_t s_G3SG1Skins[] =
{
	{ "arctic_camo", "Arctic Camo", RARITY_MILSPEC },
	{ "desert_storm", "Desert Storm", RARITY_MILSPEC },
	{ "contractor", "Contractor", RARITY_MILSPEC },
	{ "safari_mesh", "Safari Mesh", RARITY_MILSPEC },
	{ "polar_camo", "Polar Camo", RARITY_MILSPEC },
	{ "jungle_dashed", "Jungle Dashed", RARITY_MILSPEC },
	{ "demeter", "Demeter", RARITY_RESTRICTED },
	{ "azure_zebra", "Azure Zebra", RARITY_RESTRICTED },
	{ "varicamo", "VariCamo", RARITY_MILSPEC },
	{ "green_apple", "Green Apple", RARITY_MILSPEC },
	{ "murky", "Murky", RARITY_RESTRICTED },
	{ "chronos", "Chronos", RARITY_RESTRICTED },
	{ "orange_kimono", "Orange Kimono", RARITY_MILSPEC },
	{ "flux", "Flux", RARITY_CLASSIFIED },
	{ "the_executioner", "The Executioner", RARITY_CLASSIFIED },
	{ "orange_crash", "Orange Crash", RARITY_RESTRICTED },
	{ "ventilator", "Ventilator", RARITY_RESTRICTED },
	{ "stinger", "Stinger", RARITY_RESTRICTED },
	{ "hunter", "Hunter", RARITY_RESTRICTED },
	{ "high_seas", "High Seas", RARITY_RESTRICTED },
	{ "violet_murano", "Violet Murano", RARITY_RESTRICTED },
	{ "scavenger", "Scavenger", RARITY_RESTRICTED },
	{ "black_sand", "Black Sand", RARITY_RESTRICTED },
	{ "new_roots", "New Roots", RARITY_RESTRICTED },
	{ "digital_mesh", "Digital Mesh", RARITY_RESTRICTED },
	{ "ancient_ritual", "Ancient Ritual", RARITY_MILSPEC },
	{ "keeping_tabs", "Keeping Tabs", RARITY_RESTRICTED },
	{ "dream_glade", "Dream Glade", RARITY_RESTRICTED },
};

static const InventorySkinDef_t s_AUGSkins[] =
{
	{ "bengal_tiger", "Bengal Tiger", RARITY_CLASSIFIED },
	{ "copperhead", "Copperhead", RARITY_RESTRICTED },
	{ "hot_rod", "Hot Rod", RARITY_RESTRICTED },
	{ "contractor", "Contractor", RARITY_MILSPEC },
	{ "colony", "Colony", RARITY_MILSPEC },
	{ "wings", "Wings", RARITY_RESTRICTED },
	{ "storm", "Storm", RARITY_MILSPEC },
	{ "condemned", "Condemned", RARITY_MILSPEC },
	{ "anodized_navy", "Anodized Navy", RARITY_RESTRICTED },
	{ "amber_fade", "Amber Fade", RARITY_RESTRICTED },
	{ "chameleon", "Chameleon", RARITY_COVERT },
	{ "torque", "Torque", RARITY_RESTRICTED },
	{ "radiation_hazard", "Radiation Hazard", RARITY_MILSPEC },
	{ "daedalus", "Daedalus", RARITY_MILSPEC },
	{ "akihabara_accept", "Akihabara Accept", RARITY_COVERT },
	{ "ricochet", "Ricochet", RARITY_RESTRICTED },
	{ "random_access", "Random Access", RARITY_RESTRICTED },
	{ "sweeper", "Sweeper", RARITY_MILSPEC },
	{ "fleet_flock", "Fleet Flock", RARITY_CLASSIFIED },
	{ "aristocrat", "Aristocrat", RARITY_RESTRICTED },
	{ "syd_mead", "Syd Mead", RARITY_CLASSIFIED },
	{ "triqua", "Triqua", RARITY_RESTRICTED },
	{ "stymphalian", "Stymphalian", RARITY_CLASSIFIED },
	{ "amber_slipstream", "Amber Slipstream", RARITY_RESTRICTED },
	{ "midnight_lily", "Midnight Lily", RARITY_RESTRICTED },
	{ "navy_murano", "Navy Murano", RARITY_MILSPEC },
	{ "flame_j_rmungandr", "Flame Jörmungandr", RARITY_RESTRICTED },
	{ "momentum", "Momentum", RARITY_CLASSIFIED },
	{ "arctic_wolf", "Arctic Wolf", RARITY_RESTRICTED },
	{ "death_by_puppy", "Death by Puppy", RARITY_CLASSIFIED },
	{ "spalted_wood", "Spalted Wood", RARITY_MILSPEC },
	{ "sand_storm", "Sand Storm", RARITY_RESTRICTED },
	{ "tom_cat", "Tom Cat", RARITY_RESTRICTED },
	{ "surveillance", "Surveillance", RARITY_MILSPEC },
	{ "carved_jade", "Carved Jade", RARITY_RESTRICTED },
	{ "snake_pit", "Snake Pit", RARITY_MILSPEC },
	{ "plague", "Plague", RARITY_RESTRICTED },
};

static const InventorySkinDef_t s_M249Skins[] =
{
	{ "contrast_spray", "Contrast Spray", RARITY_MILSPEC },
	{ "blizzard_marbleized", "Blizzard Marbleized", RARITY_MILSPEC },
	{ "jungle", "Jungle", RARITY_MILSPEC },
	{ "predator", "Predator", RARITY_MILSPEC },
	{ "jungle_ddpat", "Jungle DDPAT", RARITY_MILSPEC },
	{ "gator_mesh", "Gator Mesh", RARITY_MILSPEC },
	{ "magma", "Magma", RARITY_RESTRICTED },
	{ "system_lock", "System Lock", RARITY_RESTRICTED },
	{ "shipping_forecast", "Shipping Forecast", RARITY_MILSPEC },
	{ "impact_drill", "Impact Drill", RARITY_MILSPEC },
	{ "nebula_crusader", "Nebula Crusader", RARITY_RESTRICTED },
	{ "spectre", "Spectre", RARITY_RESTRICTED },
	{ "emerald_poison_dart", "Emerald Poison Dart", RARITY_RESTRICTED },
	{ "warbird", "Warbird", RARITY_RESTRICTED },
	{ "aztec", "Aztec", RARITY_RESTRICTED },
	{ "midnight_palm", "Midnight Palm", RARITY_MILSPEC },
	{ "humidor", "Humidor", RARITY_RESTRICTED },
	{ "deep_relief", "Deep Relief", RARITY_RESTRICTED },
	{ "o_s_i_p_r", "O.S.I.P.R.", RARITY_RESTRICTED },
	{ "submerged", "Submerged", RARITY_MILSPEC },
	{ "downtown", "Downtown", RARITY_RESTRICTED },
};

static const InventorySkinDef_t s_NovaSkins[] =
{
	{ "bloomstick",    "Bloomstick",    RARITY_CLASSIFIED },
	{ "walnut",        "Walnut",        RARITY_MILSPEC },
	{ "rising_skull",  "Rising Skull",  RARITY_RESTRICTED },
	{ "antique",       "Antique",       RARITY_CLASSIFIED },
	{ "rust_coat",     "Rust Coat",     RARITY_MILSPEC },
	{ "koi",           "Koi",           RARITY_RESTRICTED },
	{ "ranger",        "Ranger",        RARITY_RESTRICTED },
	{ "hyper_beast",   "Hyper Beast",   RARITY_CLASSIFIED },
	{ "exo",           "Exo",           RARITY_RESTRICTED },
	{ "wild_six",      "Wild Six",      RARITY_RESTRICTED },
	{ "toy_soldier",   "Toy Soldier",   RARITY_RESTRICTED },
	{ "wood_fired",    "Wood Fired",    RARITY_RESTRICTED },
	{ "plume",         "Plume",         RARITY_RESTRICTED },
	{ "clear_polymer", "Clear Polymer", RARITY_RESTRICTED },
	{ "sobek_s_bite",  "Sobek's Bite",  RARITY_RESTRICTED },
};

static const InventorySkinDef_t s_XM1014Skins[] =
{
	{ "blue_steel",   "Blue Steel",   RARITY_MILSPEC },
	{ "heaven_guard", "Heaven Guard", RARITY_RESTRICTED },
	{ "red_leather",  "Red Leather",  RARITY_RESTRICTED },
	{ "tranquility",  "Tranquility",  RARITY_CLASSIFIED },
	{ "quicksilver",  "Quicksilver",  RARITY_RESTRICTED },
	{ "scumbria",     "Scumbria",     RARITY_RESTRICTED },
	{ "teclu_burner", "Teclu Burner", RARITY_RESTRICTED },
	{ "black_tie",    "Black Tie",    RARITY_RESTRICTED },
	{ "seasons",      "Seasons",      RARITY_RESTRICTED },
	{ "ziggy",        "Ziggy",        RARITY_RESTRICTED },
	{ "oxide_blaze",  "Oxide Blaze",  RARITY_RESTRICTED },
	{ "incinegator",  "Incinegator",  RARITY_CLASSIFIED },
	{ "entombed",     "Entombed",     RARITY_CLASSIFIED },
	{ "xoxo",         "XOXO",         RARITY_CLASSIFIED },
	{ "hieroglyph",   "Hieroglyph",   RARITY_MILSPEC },
	{ "watchdog",     "Watchdog",     RARITY_RESTRICTED },
};

static const InventoryStickerDef_t s_Stickers[] =
{
	{ "ibuypower",         "iBUYPOWER (Katowice 2014)" },
	{ "titan",             "Titan (Katowice 2014)" },
	{ "crown",             "Crown (Foil)" },
	{ "howling_dawn",      "Howling Dawn" },
	{ "reason",            "Reason Gaming (Kato 2014)" },
	{ "lgb",               "LGB eSports (Kato 2014)" },
	{ "navi_kato14",       "Natus Vincere (Kato 2014)" },
	{ "virtuspro_kato14",  "Virtus.pro (Kato 2014)" },
	{ "fnatic_kato14",     "Fnatic (Kato 2014)" },
	{ "vox",               "Vox Eminor (Kato 2014)" },
	{ "dignitas",          "Team Dignitas (Kato 2014)" },
	{ "ldlc",              "Team LDLC (Kato 2014)" },
	{ "nip_kato14",        "Ninjas in Pyjamas (Kato 2014)" },
	{ "hellraisers",       "HellRaisers (Kato 2014)" },
	{ "complexity",        "compLexity (Kato 2014)" },
	{ "3dmax",             "3DMAX (Kato 2014)" },
	{ "mousesports",       "mousesports (Kato 2014)" },
	{ "mystik",            "Clan-Mystik (Kato 2014)" },
	{ "flammable",         "Flammable (Foil)" },
	{ "headhunter",        "Headhunter (Foil)" },
	{ "swag",              "Swag (Foil)" },
	{ "new_sheriff",       "New Sheriff (Foil)" },
	{ "firestarter",       "Firestarter (Holo)" },
	{ "dragon_lore",       "Dragon Lore (Foil)" },
	{ "gold_web",          "Gold Web (Foil)" },
	{ "battle_scarred",    "Battle Scarred (Holo)" },
	{ "broken_fang",       "Broken Fang" },
	{ "ak47_recoil",       "AK-47 Recoil" },
	{ "ak47_recoil_gold",  "AK-47 Recoil (Gold)" },
	{ "baby_howl",         "Baby Howl" },
	{ "baby_lore",         "Baby Lore" },
	{ "baby_fire_serpent", "Baby Fire Serpent" },
	{ "blue_gem",          "Blue Gem" },
	{ "bomb_doge",         "Bomb Doge" },
	{ "drug_war",          "Drug War Veteran" },
	{ "welcome_clutch",    "Welcome to the Clutch" },
	{ "bish",              "Bish (Holo)" },
	{ "bash",              "Bash (Holo)" },
	{ "bosh",              "Bosh (Holo)" },
	{ "lambda",            "Half-Life Lambda" },
};

#include "cs_inventory_knife_skins.h"

#define KNIFE_SKIN_TEX_BASE "advanced_inventory_csgo_items/models/weapons/v_models/"

static const InventoryKnifeDef_t s_Knives[] =
{
	{ "0",               "Default Knife (Team)",   ADVANCED_WEAPONS_DIR "v_knife_default_ct.mdl",          "vgui/inventory/knives/default",         RARITY_STOCK, NULL, NULL, 0 },
	{ "bayonet",         "Bayonet",                ADVANCED_WEAPONS_DIR "v_knife_bayonet.mdl",             "vgui/inventory/knives/bayonet",         RARITY_GOLD,  KNIFE_SKIN_TEX_BASE "knife_bayonet/skins",           s_KnifeSkins_bayonet,         ARRAYSIZE( s_KnifeSkins_bayonet ) },
	{ "css",             "Classic Knife",          ADVANCED_WEAPONS_DIR "v_knife_css.mdl",                 "vgui/inventory/knives/css",             RARITY_GOLD,  KNIFE_SKIN_TEX_BASE "knife_css/skins",               s_KnifeSkins_css,             ARRAYSIZE( s_KnifeSkins_css ) },
	{ "flip",            "Flip Knife",             ADVANCED_WEAPONS_DIR "v_knife_flip.mdl",                "vgui/inventory/knives/flip",            RARITY_GOLD,  KNIFE_SKIN_TEX_BASE "knife_flip/skins",              s_KnifeSkins_flip,            ARRAYSIZE( s_KnifeSkins_flip ) },
	{ "gut",             "Gut Knife",              ADVANCED_WEAPONS_DIR "v_knife_gut.mdl",                 "vgui/inventory/knives/gut",             RARITY_GOLD,  KNIFE_SKIN_TEX_BASE "knife_gut/skins",               s_KnifeSkins_gut,             ARRAYSIZE( s_KnifeSkins_gut ) },
	{ "karambit",        "Karambit",               ADVANCED_WEAPONS_DIR "v_knife_karam.mdl",               "vgui/inventory/knives/karambit",        RARITY_GOLD,  KNIFE_SKIN_TEX_BASE "knife_karam/skins",             s_KnifeSkins_karambit,        ARRAYSIZE( s_KnifeSkins_karambit ) },
	{ "m9_bayonet",      "M9 Bayonet",             ADVANCED_WEAPONS_DIR "v_knife_m9_bay.mdl",              "vgui/inventory/knives/m9_bayonet",      RARITY_GOLD,  KNIFE_SKIN_TEX_BASE "knife_m9_bay/skins",            s_KnifeSkins_m9_bayonet,      ARRAYSIZE( s_KnifeSkins_m9_bayonet ) },
	{ "tactical",        "Huntsman Knife",         ADVANCED_WEAPONS_DIR "v_knife_tactical.mdl",            "vgui/inventory/knives/tactical",        RARITY_GOLD,  KNIFE_SKIN_TEX_BASE "knife_tactical/skins",          s_KnifeSkins_tactical,        ARRAYSIZE( s_KnifeSkins_tactical ) },
	{ "falchion",        "Falchion Knife",         ADVANCED_WEAPONS_DIR "v_knife_falchion_advanced.mdl",   "vgui/inventory/knives/falchion",        RARITY_GOLD,  KNIFE_SKIN_TEX_BASE "knife_falchion_advanced/skins", s_KnifeSkins_falchion,        ARRAYSIZE( s_KnifeSkins_falchion ) },
	{ "survival_bowie",  "Bowie Knife",            ADVANCED_WEAPONS_DIR "v_knife_survival_bowie.mdl",      "vgui/inventory/knives/survival_bowie",  RARITY_GOLD,  KNIFE_SKIN_TEX_BASE "knife_survival_bowie/skins",    s_KnifeSkins_survival_bowie,  ARRAYSIZE( s_KnifeSkins_survival_bowie ) },
	{ "butterfly",       "Butterfly Knife",        ADVANCED_WEAPONS_DIR "v_knife_butterfly.mdl",           "vgui/inventory/knives/butterfly",       RARITY_GOLD,  KNIFE_SKIN_TEX_BASE "knife_butterfly/skins",         s_KnifeSkins_butterfly,       ARRAYSIZE( s_KnifeSkins_butterfly ) },
	{ "push",            "Shadow Daggers",         ADVANCED_WEAPONS_DIR "v_knife_push.mdl",                "vgui/inventory/knives/push",            RARITY_GOLD,  KNIFE_SKIN_TEX_BASE "knife_push/skins",              s_KnifeSkins_push,            ARRAYSIZE( s_KnifeSkins_push ) },
	{ "cord",            "Paracord Knife",         ADVANCED_WEAPONS_DIR "v_knife_cord.mdl",                "vgui/inventory/knives/cord",            RARITY_GOLD,  KNIFE_SKIN_TEX_BASE "knife_cord/skins",              s_KnifeSkins_cord,            ARRAYSIZE( s_KnifeSkins_cord ) },
	{ "canis",           "Survival Knife",         ADVANCED_WEAPONS_DIR "v_knife_canis.mdl",               "vgui/inventory/knives/canis",           RARITY_GOLD,  KNIFE_SKIN_TEX_BASE "knife_canis/skins",             s_KnifeSkins_canis,           ARRAYSIZE( s_KnifeSkins_canis ) },
	{ "ursus",           "Ursus Knife",            ADVANCED_WEAPONS_DIR "v_knife_ursus.mdl",               "vgui/inventory/knives/ursus",           RARITY_GOLD,  KNIFE_SKIN_TEX_BASE "knife_ursus/skins",             s_KnifeSkins_ursus,           ARRAYSIZE( s_KnifeSkins_ursus ) },
	{ "gypsy_jackknife", "Navaja Knife",           ADVANCED_WEAPONS_DIR "v_knife_gypsy_jackknife.mdl",     "vgui/inventory/knives/gypsy_jackknife", RARITY_GOLD,  KNIFE_SKIN_TEX_BASE "knife_gypsy_jack/skins",        s_KnifeSkins_gypsy_jackknife, ARRAYSIZE( s_KnifeSkins_gypsy_jackknife ) },
	{ "outdoor",         "Nomad Knife",            ADVANCED_WEAPONS_DIR "v_knife_outdoor.mdl",             "vgui/inventory/knives/outdoor",         RARITY_GOLD,  KNIFE_SKIN_TEX_BASE "knife_outdoor/skins",           s_KnifeSkins_outdoor,         ARRAYSIZE( s_KnifeSkins_outdoor ) },
	{ "stiletto",        "Stiletto Knife",         ADVANCED_WEAPONS_DIR "v_knife_stiletto.mdl",            "vgui/inventory/knives/stiletto",        RARITY_GOLD,  KNIFE_SKIN_TEX_BASE "knife_stiletto/skins",          s_KnifeSkins_stiletto,        ARRAYSIZE( s_KnifeSkins_stiletto ) },
	{ "widowmaker",      "Talon Knife",            ADVANCED_WEAPONS_DIR "v_knife_widowmaker.mdl",          "vgui/inventory/knives/widowmaker",      RARITY_GOLD,  KNIFE_SKIN_TEX_BASE "knife_widowmaker/skins",        s_KnifeSkins_widowmaker,      ARRAYSIZE( s_KnifeSkins_widowmaker ) },
	{ "skeleton",        "Skeleton Knife",         ADVANCED_WEAPONS_DIR "v_knife_skeleton.mdl",            "vgui/inventory/knives/skeleton",        RARITY_GOLD,  KNIFE_SKIN_TEX_BASE "knife_skeleton/skins",          s_KnifeSkins_skeleton,        ARRAYSIZE( s_KnifeSkins_skeleton ) },
};

static const InventoryGloveDef_t s_Gloves[] =
{
	// Base / Default / Tactical hands
	{ "default",           "-1",          "0",      "Default Gloves (Faction)",            ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle.mdl",             "vgui/inventory/gloves/default_faction",   RARITY_STOCK },
	{ "bare",              "bare",        "0",      "Bare Hands",                          ADVANCED_ARMS_DIR "bare/v_bare_hands.mdl",                               "vgui/inventory/gloves/bare_hands",        RARITY_MILSPEC },
	{ "hardknuckle",       "hardknuckle", "0",      "Hard Knuckles (Default)",             ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle.mdl",             "vgui/inventory/gloves/hardknuckle",       RARITY_MILSPEC },
	{ "hardknuckle_blue",  "hardknuckle", "blue",   "Hard Knuckles | Blue",                ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle_blue.mdl",        "vgui/inventory/gloves/hardknuckle_blue",  RARITY_RESTRICTED },
	{ "hardknuckle_black", "hardknuckle", "black",  "Hard Knuckles | Black",               ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle_black.mdl",       "vgui/inventory/gloves/hardknuckle_black", RARITY_RESTRICTED },
	{ "fullfinger",        "fullfinger",  "0",      "Full Finger Gloves",                  ADVANCED_ARMS_DIR "glove_fullfinger/v_glove_fullfinger.mdl",               "vgui/inventory/gloves/fullfinger",        RARITY_MILSPEC },
	{ "fingerless",        "fingerless",  "0",      "Fingerless Gloves",                   ADVANCED_ARMS_DIR "glove_fingerless/v_glove_fingerless.mdl",               "vgui/inventory/gloves/fingerless",        RARITY_MILSPEC },
	{ "anarchist",         "anarchist",   "0",      "Anarchist Gloves",                    ADVANCED_ARMS_DIR "anarchist/v_glove_anarchist.mdl",                       "vgui/inventory/gloves/anarchist",         RARITY_MILSPEC },

	// Bloodhound Gloves
	{ "bloodhound_black_silver",    "bloodhound", "black_silver",    "Bloodhound | Charred",               ADVANCED_ARMS_DIR "glove_bloodhound/v_glove_bloodhound.mdl", "vgui/inventory/gloves/studded_bloodhound_gloves_bloodhound_black_silver", RARITY_COVERT },
	{ "bloodhound_snakeskin_brass", "bloodhound", "snakeskin_brass", "Bloodhound | Bronzed",               ADVANCED_ARMS_DIR "glove_bloodhound/v_glove_bloodhound.mdl", "vgui/inventory/gloves/studded_bloodhound_gloves_bloodhound_snakeskin_brass", RARITY_COVERT },
	{ "bloodhound_metallic",        "bloodhound", "metallic",        "Bloodhound | Snakebite",             ADVANCED_ARMS_DIR "glove_bloodhound/v_glove_bloodhound.mdl", "vgui/inventory/gloves/studded_bloodhound_gloves_bloodhound_metallic",    RARITY_COVERT },
	{ "bloodhound_guerrilla",       "bloodhound", "guerrilla",       "Bloodhound | Guerrilla",             ADVANCED_ARMS_DIR "glove_bloodhound/v_glove_bloodhound.mdl", "vgui/inventory/gloves/studded_bloodhound_gloves_bloodhound_guerrilla",   RARITY_COVERT },

	// Sport Gloves
	{ "sporty_lightblue",              "sporty", "lightblue",              "Sport | Superconductor",         ADVANCED_ARMS_DIR "glove_sporty/v_glove_sporty.mdl", "vgui/inventory/gloves/sporty_gloves_sporty_light_blue",                  RARITY_COVERT },
	{ "sporty_military",               "sporty", "military",               "Sport | Arid",                   ADVANCED_ARMS_DIR "glove_sporty/v_glove_sporty.mdl", "vgui/inventory/gloves/sporty_gloves_sporty_military",                    RARITY_COVERT },
	{ "sporty_purple",                 "sporty", "purple",                 "Sport | Pandora's Box",          ADVANCED_ARMS_DIR "glove_sporty/v_glove_sporty.mdl", "vgui/inventory/gloves/sporty_gloves_sporty_purple",                      RARITY_GOLD },
	{ "sporty_green",                  "sporty", "green",                  "Sport | Hedge Maze",             ADVANCED_ARMS_DIR "glove_sporty/v_glove_sporty.mdl", "vgui/inventory/gloves/sporty_gloves_sporty_green",                       RARITY_COVERT },
	{ "sporty_poison_frog_blue_white", "sporty", "poison_frog_blue_white", "Sport | Amphibious",             ADVANCED_ARMS_DIR "glove_sporty/v_glove_sporty.mdl", "vgui/inventory/gloves/sporty_gloves_sporty_poison_frog_blue_white",      RARITY_COVERT },
	{ "sporty_poison_frog_red_green",  "sporty", "poison_frog_red_green",  "Sport | Bronze Morph",           ADVANCED_ARMS_DIR "glove_sporty/v_glove_sporty.mdl", "vgui/inventory/gloves/sporty_gloves_sporty_poison_frog_red_green",       RARITY_COVERT },
	{ "sporty_black_webbing_yellow",   "sporty", "black_webbing_yellow",   "Sport | Omega",                  ADVANCED_ARMS_DIR "glove_sporty/v_glove_sporty.mdl", "vgui/inventory/gloves/sporty_gloves_sporty_black_webbing_yellow",        RARITY_COVERT },
	{ "sporty_blue_pink",              "sporty", "blue_pink",              "Sport | Vice",                   ADVANCED_ARMS_DIR "glove_sporty/v_glove_sporty.mdl", "vgui/inventory/gloves/sporty_gloves_sporty_blue_pink",                   RARITY_GOLD },
	{ "sporty_slingshot",              "sporty", "slingshot",              "Sport | Slingshot",              ADVANCED_ARMS_DIR "glove_sporty/v_glove_sporty.mdl", "vgui/inventory/gloves/sporty_gloves_sporty_slingshot",                   RARITY_COVERT },
	{ "sporty_hunter",                 "sporty", "hunter",                 "Sport | Big Game",               ADVANCED_ARMS_DIR "glove_sporty/v_glove_sporty.mdl", "vgui/inventory/gloves/sporty_gloves_sporty_hunter",                      RARITY_COVERT },
	{ "sporty_houndstooth_red",        "sporty", "houndstooth_red",        "Sport | Scarlet Shamagh",        ADVANCED_ARMS_DIR "glove_sporty/v_glove_sporty.mdl", "vgui/inventory/gloves/sporty_gloves_sporty_houndstooth_red",             RARITY_COVERT },
	{ "sporty_jaguar",                 "sporty", "jaguar",                 "Sport | Nocturnal / Jaguar",     ADVANCED_ARMS_DIR "glove_sporty/v_glove_sporty.mdl", "vgui/inventory/gloves/sporty_gloves_sporty_jaguar",                      RARITY_COVERT },

	// Driver / Slick Gloves
	{ "slick_black",                 "slick", "black",                 "Driver | Black Tie",               ADVANCED_ARMS_DIR "glove_slick/v_glove_slick.mdl", "vgui/inventory/gloves/slick_gloves_slick_black",                  RARITY_COVERT },
	{ "slick_military",              "slick", "military",              "Driver | Convoy",                  ADVANCED_ARMS_DIR "glove_slick/v_glove_slick.mdl", "vgui/inventory/gloves/slick_gloves_slick_military",               RARITY_COVERT },
	{ "slick_red",                   "slick", "red",                   "Driver | Crimson Weave",           ADVANCED_ARMS_DIR "glove_slick/v_glove_slick.mdl", "vgui/inventory/gloves/slick_gloves_slick_red",                    RARITY_COVERT },
	{ "slick_snakeskin_yellow",      "slick", "snakeskin_yellow",      "Driver | Diamondback",             ADVANCED_ARMS_DIR "glove_slick/v_glove_slick.mdl", "vgui/inventory/gloves/slick_gloves_slick_snakeskin_yellow",       RARITY_COVERT },
	{ "slick_snakeskin_white",       "slick", "snakeskin_white",       "Driver | King Snake",              ADVANCED_ARMS_DIR "glove_slick/v_glove_slick.mdl", "vgui/inventory/gloves/slick_gloves_slick_snakeskin_white",        RARITY_COVERT },
	{ "slick_plaid_purple",          "slick", "plaid_purple",          "Driver | Imperial Plaid",          ADVANCED_ARMS_DIR "glove_slick/v_glove_slick.mdl", "vgui/inventory/gloves/slick_gloves_slick_plaid_purple",           RARITY_COVERT },
	{ "slick_stitched_black_orange", "slick", "stitched_black_orange", "Driver | Overtake",                ADVANCED_ARMS_DIR "glove_slick/v_glove_slick.mdl", "vgui/inventory/gloves/slick_gloves_slick_stitched_black_orange",  RARITY_COVERT },
	{ "slick_stitched_green_grey",   "slick", "stitched_green_grey",   "Driver | Racing Green",            ADVANCED_ARMS_DIR "glove_slick/v_glove_slick.mdl", "vgui/inventory/gloves/slick_gloves_slick_stitched_green_grey",    RARITY_COVERT },
	{ "slick_rezan",                 "slick", "rezan",                 "Driver | Rezan the Red",           ADVANCED_ARMS_DIR "glove_slick/v_glove_slick.mdl", "vgui/inventory/gloves/slick_gloves_slick_rezan",                  RARITY_COVERT },
	{ "slick_jaguar_white",          "slick", "jaguar_white",          "Driver | Snow Leopard",            ADVANCED_ARMS_DIR "glove_slick/v_glove_slick.mdl", "vgui/inventory/gloves/slick_gloves_slick_jaguar_white",           RARITY_COVERT },
	{ "slick_jaguar_yellow",         "slick", "jaguar_yellow",         "Driver | Queen Jaguar",            ADVANCED_ARMS_DIR "glove_slick/v_glove_slick.mdl", "vgui/inventory/gloves/slick_gloves_slick_jaguar_yellow",          RARITY_COVERT },
	{ "slick_stitched_black_white",  "slick", "stitched_black_white",  "Driver | Black Tie Stitched",      ADVANCED_ARMS_DIR "glove_slick/v_glove_slick.mdl", "vgui/inventory/gloves/slick_gloves_slick_stitched_black_white",   RARITY_COVERT },
	{ "slick_bamboo",                "slick", "bamboo",                "Driver | Bamboo",                  ADVANCED_ARMS_DIR "glove_slick/v_glove_slick.mdl", "vgui/inventory/gloves/slick_gloves_slick_bamboo",                 RARITY_COVERT },
	{ "slick_chainmail_gold",        "slick", "chainmail_gold",        "Driver | Chainmail Gold",          ADVANCED_ARMS_DIR "glove_slick/v_glove_slick.mdl", "vgui/inventory/gloves/slick_gloves_slick_chainmail_gold",         RARITY_COVERT },
	{ "slick_chainmail_knight",      "slick", "chainmail_knight",      "Driver | Chainmail Knight",        ADVANCED_ARMS_DIR "glove_slick/v_glove_slick.mdl", "vgui/inventory/gloves/slick_gloves_slick_chainmail_knight",       RARITY_COVERT },
	{ "slick_snakeskin_green",       "slick", "snakeskin_green",       "Driver | Snakeskin Green",         ADVANCED_ARMS_DIR "glove_slick/v_glove_slick.mdl", "vgui/inventory/gloves/slick_gloves_slick_snakeskin_green",        RARITY_COVERT },
	{ "slick_stitched_black",        "slick", "stitched_black",        "Driver | Stitched Black",          ADVANCED_ARMS_DIR "glove_slick/v_glove_slick.mdl", "vgui/inventory/gloves/slick_gloves_slick_stitched_black",         RARITY_COVERT },

	// Hand Wraps
	{ "handwrap_brown_beige",        "handwrap", "brown_beige",        "Hand Wraps | Badlands Brown",      ADVANCED_ARMS_DIR "glove_handwrap_leathery/v_glove_handwrap_leathery.mdl", "vgui/inventory/gloves/leather_handwraps_handwrap_leathery",                     RARITY_COVERT },
	{ "handwrap_grey_camo_blob",     "handwrap", "grey_camo_blob",     "Hand Wraps | Spruce DDPAT",        ADVANCED_ARMS_DIR "glove_handwrap_leathery/v_glove_handwrap_leathery.mdl", "vgui/inventory/gloves/leather_handwraps_handwrap_camo_grey",                    RARITY_COVERT },
	{ "handwrap_red_slaughter",      "handwrap", "red_slaughter",      "Hand Wraps | Slaughter",           ADVANCED_ARMS_DIR "glove_handwrap_leathery/v_glove_handwrap_leathery.mdl", "vgui/inventory/gloves/leather_handwraps_handwrap_red_slaughter",                RARITY_COVERT },
	{ "handwrap_orange_camo",        "handwrap", "orange_camo",        "Hand Wraps | Badlands",            ADVANCED_ARMS_DIR "glove_handwrap_leathery/v_glove_handwrap_leathery.mdl", "vgui/inventory/gloves/leather_handwraps_handwrap_fabric_orange_camo",           RARITY_COVERT },
	{ "handwrap_blue_skulls",        "handwrap", "blue_skulls",        "Hand Wraps | Cobalt Skulls",       ADVANCED_ARMS_DIR "glove_handwrap_leathery/v_glove_handwrap_leathery.mdl", "vgui/inventory/gloves/leather_handwraps_handwrap_leathery_fabric_blue_skulls",  RARITY_COVERT },
	{ "handwrap_geometric_blue",     "handwrap", "geometric_blue",     "Hand Wraps | Overprint",           ADVANCED_ARMS_DIR "glove_handwrap_leathery/v_glove_handwrap_leathery.mdl", "vgui/inventory/gloves/leather_handwraps_handwrap_leathery_fabric_geometric_blue", RARITY_COVERT },
	{ "handwrap_ducttape",           "handwrap", "ducttape",           "Hand Wraps | Duct Tape",           ADVANCED_ARMS_DIR "glove_handwrap_leathery/v_glove_handwrap_leathery.mdl", "vgui/inventory/gloves/leather_handwraps_handwrap_leathery_ducttape",            RARITY_COVERT },
	{ "handwrap_green_camo",         "handwrap", "green_camo",         "Hand Wraps | Arboreal",            ADVANCED_ARMS_DIR "glove_handwrap_leathery/v_glove_handwrap_leathery.mdl", "vgui/inventory/gloves/leather_handwraps_handwrap_leathery_fabric_green_camo",   RARITY_COVERT },
	{ "handwrap_houndstooth_orange", "handwrap", "houndstooth_orange", "Hand Wraps | CAUTION / Hound",     ADVANCED_ARMS_DIR "glove_handwrap_leathery/v_glove_handwrap_leathery.mdl", "vgui/inventory/gloves/leather_handwraps_handwrap_fabric_houndstooth_orange",    RARITY_COVERT },
	{ "handwrap_giraffe",            "handwrap", "giraffe",            "Hand Wraps | Giraffe",             ADVANCED_ARMS_DIR "glove_handwrap_leathery/v_glove_handwrap_leathery.mdl", "vgui/inventory/gloves/leather_handwraps_handwrap_leathery_fabric_giraffe",      RARITY_COVERT },
	{ "handwrap_snakeskin_orange",   "handwrap", "snakeskin_orange",   "Hand Wraps | Constrictor",         ADVANCED_ARMS_DIR "glove_handwrap_leathery/v_glove_handwrap_leathery.mdl", "vgui/inventory/gloves/leather_handwraps_handwrap_leathery_snakeskin_orange",    RARITY_COVERT },
	{ "handwrap_caution",            "handwrap", "caution",            "Hand Wraps | CAUTION!",            ADVANCED_ARMS_DIR "glove_handwrap_leathery/v_glove_handwrap_leathery.mdl", "vgui/inventory/gloves/leather_handwraps_handwrap_leathery_caution",             RARITY_COVERT },
	{ "handwrap_catskulls_bw",       "handwrap", "catskulls_bw",       "Hand Wraps | Desert Shamagh",      ADVANCED_ARMS_DIR "glove_handwrap_leathery/v_glove_handwrap_leathery.mdl", "vgui/inventory/gloves/leather_handwraps_handwrap_leathery_catskulls_bw",        RARITY_COVERT },
	{ "handwrap_beige_camo",         "handwrap", "beige_camo",         "Hand Wraps | Beige Camo",          ADVANCED_ARMS_DIR "glove_handwrap_leathery/v_glove_handwrap_leathery.mdl", "vgui/inventory/gloves/leather_handwraps_handwrap_leathery_fabric_beige_camo",   RARITY_COVERT },
	{ "handwrap_nuclear",            "handwrap", "nuclear",            "Hand Wraps | Nuclear",             ADVANCED_ARMS_DIR "glove_handwrap_leathery/v_glove_handwrap_leathery.mdl", "vgui/inventory/gloves/leather_handwraps_handwrap_leathery_fabric_nuclear",      RARITY_COVERT },

	// Moto Gloves
	{ "moto_basic_black",             "moto", "basic_black",            "Moto | Eclipse",                  ADVANCED_ARMS_DIR "glove_motorcycle/v_glove_motorcycle.mdl", "vgui/inventory/gloves/motorcycle_gloves_motorcycle_basic_black",            RARITY_COVERT },
	{ "moto_mint_triangle",           "moto", "mint_triangle",          "Moto | Spearmint",                ADVANCED_ARMS_DIR "glove_motorcycle/v_glove_motorcycle.mdl", "vgui/inventory/gloves/motorcycle_gloves_motorcycle_mint_triangle",          RARITY_COVERT },
	{ "moto_mono_boom",               "moto", "mono_boom",              "Moto | Boom!",                    ADVANCED_ARMS_DIR "glove_motorcycle/v_glove_motorcycle.mdl", "vgui/inventory/gloves/motorcycle_gloves_motorcycle_mono_boom",              RARITY_COVERT },
	{ "moto_triangle_blue",           "moto", "triangle_blue",          "Moto | Cool Mint",                ADVANCED_ARMS_DIR "glove_motorcycle/v_glove_motorcycle.mdl", "vgui/inventory/gloves/motorcycle_gloves_motorcycle_triangle_blue",          RARITY_COVERT },
	{ "moto_choco_boom",              "moto", "choco_boom",             "Moto | POW!",                     ADVANCED_ARMS_DIR "glove_motorcycle/v_glove_motorcycle.mdl", "vgui/inventory/gloves/motorcycle_gloves_motorcycle_choco_boom",             RARITY_COVERT },
	{ "moto_basic_green_orange",      "moto", "basic_green_orange",     "Moto | Turtle",                   ADVANCED_ARMS_DIR "glove_motorcycle/v_glove_motorcycle.mdl", "vgui/inventory/gloves/motorcycle_gloves_motorcycle_basic_green_orange",     RARITY_COVERT },
	{ "moto_yellow_camo",             "moto", "yellow_camo",            "Moto | Transport",                ADVANCED_ARMS_DIR "glove_motorcycle/v_glove_motorcycle.mdl", "vgui/inventory/gloves/motorcycle_gloves_motorcycle_yellow_camo",            RARITY_COVERT },
	{ "moto_trigrid_blue",            "moto", "trigrid_blue",           "Moto | Polygon",                  ADVANCED_ARMS_DIR "glove_motorcycle/v_glove_motorcycle.mdl", "vgui/inventory/gloves/motorcycle_gloves_motorcycle_trigrid_blue",           RARITY_COVERT },
	{ "moto_checker_flag_blue_green", "moto", "checker_flag_blue_green","Moto | Finish Line",              ADVANCED_ARMS_DIR "glove_motorcycle/v_glove_motorcycle.mdl", "vgui/inventory/gloves/motorcycle_gloves_motorcycle_checker_flag_blue_green",RARITY_COVERT },
	{ "moto_smoke",                   "moto", "smoke",                  "Moto | Smoke Out",                ADVANCED_ARMS_DIR "glove_motorcycle/v_glove_motorcycle.mdl", "vgui/inventory/gloves/motorcycle_gloves_motorcycle_smoke",                  RARITY_COVERT },
	{ "moto_carbonfiber_red",         "moto", "carbonfiber_red",        "Moto | Blood Pressure",           ADVANCED_ARMS_DIR "glove_motorcycle/v_glove_motorcycle.mdl", "vgui/inventory/gloves/motorcycle_gloves_motorcycle_carbonfiber_red",        RARITY_COVERT },
	{ "moto_commando_ksk",            "moto", "commando_ksk",           "Moto | 3rd Commando",             ADVANCED_ARMS_DIR "glove_motorcycle/v_glove_motorcycle.mdl", "vgui/inventory/gloves/motorcycle_gloves_motorcycle_commando_ksk",           RARITY_COVERT },
	{ "moto_doppler_phase2",          "moto", "doppler_phase2",         "Moto | Doppler Phase 2",          ADVANCED_ARMS_DIR "glove_motorcycle/v_glove_motorcycle.mdl", "vgui/inventory/gloves/motorcycle_gloves_motorcycle_doppler_phase2",         RARITY_COVERT },
	{ "moto_checker_flag_green",      "moto", "checker_flag_green",     "Moto | Checker Green",            ADVANCED_ARMS_DIR "glove_motorcycle/v_glove_motorcycle.mdl", "vgui/inventory/gloves/motorcycle_gloves_motorcycle_checker_flag_green",     RARITY_COVERT },
	{ "moto_green_circuitboard",      "moto", "green_circuitboard",     "Moto | Circuitboard",             ADVANCED_ARMS_DIR "glove_motorcycle/v_glove_motorcycle.mdl", "vgui/inventory/gloves/motorcycle_gloves_motorcycle_green_circuitboard",     RARITY_COVERT },

	// Specialist Gloves
	{ "specialist_ddpat_dense_green_brown",  "specialist", "ddpat_dense_green_brown",   "Specialist | Forest DDPAT",    ADVANCED_ARMS_DIR "glove_specialist/v_glove_specialist.mdl", "vgui/inventory/gloves/specialist_gloves_specialist_ddpat_green_camo", RARITY_COVERT },
	{ "specialist_kimono_diamonds_black_red","specialist", "kimono_diamonds_black_red", "Specialist | Crimson Kimono",  ADVANCED_ARMS_DIR "glove_specialist/v_glove_specialist.mdl", "vgui/inventory/gloves/specialist_gloves_specialist_kimono_diamonds_red", RARITY_GOLD },
	{ "specialist_webs_green",               "specialist", "webs_green",                "Specialist | Emerald Web",     ADVANCED_ARMS_DIR "glove_specialist/v_glove_specialist.mdl", "vgui/inventory/gloves/specialist_gloves_specialist_emerald_web",      RARITY_COVERT },
	{ "specialist_white_orange_grey",        "specialist", "white_orange_grey",         "Specialist | Foundation",      ADVANCED_ARMS_DIR "glove_specialist/v_glove_specialist.mdl", "vgui/inventory/gloves/specialist_gloves_specialist_orange_white",     RARITY_COVERT },
	{ "specialist_webs_red",                 "specialist", "webs_red",                  "Specialist | Crimson Web",     ADVANCED_ARMS_DIR "glove_specialist/v_glove_specialist.mdl", "vgui/inventory/gloves/specialist_gloves_specialist_webs_red",         RARITY_COVERT },
	{ "specialist_forest_brown",             "specialist", "forest_brown",              "Specialist | Buckshot",        ADVANCED_ARMS_DIR "glove_specialist/v_glove_specialist.mdl", "vgui/inventory/gloves/specialist_gloves_specialist_forest_brown",     RARITY_COVERT },
	{ "specialist_fade",                     "specialist", "fade",                      "Specialist | Fade",            ADVANCED_ARMS_DIR "glove_specialist/v_glove_specialist.mdl", "vgui/inventory/gloves/specialist_gloves_specialist_fade",             RARITY_GOLD },
	{ "specialist_winterhex",                "specialist", "winterhex",                 "Specialist | Mogul",           ADVANCED_ARMS_DIR "glove_specialist/v_glove_specialist.mdl", "vgui/inventory/gloves/specialist_gloves_specialist_winterhex",        RARITY_COVERT },
	{ "specialist_marble_fade",              "specialist", "marble_fade",               "Specialist | Marble Fade",     ADVANCED_ARMS_DIR "glove_specialist/v_glove_specialist.mdl", "vgui/inventory/gloves/specialist_gloves_specialist_marble_fade",      RARITY_COVERT },
	{ "specialist_ricksaw_camo",             "specialist", "ricksaw_camo",              "Specialist | Lt. Commander",   ADVANCED_ARMS_DIR "glove_specialist/v_glove_specialist.mdl", "vgui/inventory/gloves/specialist_gloves_specialist_ricksaw_camo",     RARITY_COVERT },
	{ "specialist_tiger_orange",             "specialist", "tiger_orange",              "Specialist | Tiger Strike",    ADVANCED_ARMS_DIR "glove_specialist/v_glove_specialist.mdl", "vgui/inventory/gloves/specialist_gloves_specialist_tiger_orange",     RARITY_COVERT },
	{ "specialist_fbi",                      "specialist", "fbi",                       "Specialist | Field Agent",     ADVANCED_ARMS_DIR "glove_specialist/v_glove_specialist.mdl", "vgui/inventory/gloves/specialist_gloves_specialist_fbi",              RARITY_COVERT },

	// Hydra Gloves
	{ "hydra_black_green",              "hydra", "black_green",              "Hydra | Emerald",              ADVANCED_ARMS_DIR "glove_bloodhound/v_glove_bloodhound_hydra.mdl", "vgui/inventory/gloves/studded_hydra_gloves_bloodhound_hydra_black_green",         RARITY_COVERT },
	{ "hydra_green_leather_mesh_brass", "hydra", "green_leather_mesh_brass", "Hydra | Mangrove",             ADVANCED_ARMS_DIR "glove_bloodhound/v_glove_bloodhound_hydra.mdl", "vgui/inventory/gloves/studded_hydra_gloves_bloodhound_hydra_green_leather_mesh_brass", RARITY_COVERT },
	{ "hydra_snakeskin_brass",          "hydra", "snakeskin_brass",          "Hydra | Rattler",              ADVANCED_ARMS_DIR "glove_bloodhound/v_glove_bloodhound_hydra.mdl", "vgui/inventory/gloves/studded_hydra_gloves_bloodhound_hydra_snakeskin_brass",     RARITY_COVERT },
	{ "hydra_case_hardened",            "hydra", "case_hardened",            "Hydra | Case Hardened",        ADVANCED_ARMS_DIR "glove_bloodhound/v_glove_bloodhound_hydra.mdl", "vgui/inventory/gloves/studded_hydra_gloves_bloodhound_hydra_case_hardened",       RARITY_COVERT },
	{ "hydra_poison_frog_red_green",    "hydra", "poison_frog_red_green",    "Hydra | Poison Frog",          ADVANCED_ARMS_DIR "glove_bloodhound/v_glove_bloodhound_hydra.mdl", "vgui/inventory/gloves/studded_hydra_gloves_operation10_poison_frog_red_green",    RARITY_COVERT },
	{ "hydra_jaguar_purple",            "hydra", "jaguar_purple",            "Hydra | Purple Jaguar",        ADVANCED_ARMS_DIR "glove_bloodhound/v_glove_bloodhound_hydra.mdl", "vgui/inventory/gloves/studded_hydra_gloves_operation10_jaguar_purple",            RARITY_COVERT },
	{ "hydra_snakeskin_black",          "hydra", "snakeskin_black",          "Hydra | Snakeskin Black",      ADVANCED_ARMS_DIR "glove_bloodhound/v_glove_bloodhound_hydra.mdl", "vgui/inventory/gloves/studded_hydra_gloves_operation_10_snakeskin_black",         RARITY_COVERT },
	{ "hydra_metalic_green",            "hydra", "metalic_green",            "Hydra | Metallic Green",       ADVANCED_ARMS_DIR "glove_bloodhound/v_glove_bloodhound_hydra.mdl", "vgui/inventory/gloves/studded_hydra_gloves_operation_10_metalic_green",           RARITY_COVERT },

	// Broken Fang Gloves
	{ "brokenfang_metalic_green",           "brokenfang", "metalic_green",            "Broken Fang | Jade",         ADVANCED_ARMS_DIR "glove_bloodhound/v_glove_bloodhound_brokenfang.mdl", "vgui/inventory/gloves/studded_brokenfang_gloves_operation10_metalic_green",        RARITY_COVERT },
	{ "brokenfang_poison_frog_black_yellow","brokenfang", "poison_frog_black_yellow", "Broken Fang | Yellow-banded",ADVANCED_ARMS_DIR "glove_bloodhound/v_glove_bloodhound_brokenfang.mdl", "vgui/inventory/gloves/studded_brokenfang_gloves_operation10_poison_frog_black_yellow", RARITY_COVERT },
	{ "brokenfang_floral",                  "brokenfang", "floral",                   "Broken Fang | Needle Point", ADVANCED_ARMS_DIR "glove_bloodhound/v_glove_bloodhound_brokenfang.mdl", "vgui/inventory/gloves/studded_brokenfang_gloves_operation10_floral",               RARITY_COVERT },
	{ "brokenfang_snakeskin_black",         "brokenfang", "snakeskin_black",          "Broken Fang | Unhinged",     ADVANCED_ARMS_DIR "glove_bloodhound/v_glove_bloodhound_brokenfang.mdl", "vgui/inventory/gloves/studded_brokenfang_gloves_operation10_snakeskin_black",      RARITY_COVERT },
};

static const InventorySleeveDef_t s_Sleeves[] =
{
	{ "-1",                        "Default Sleeve (Faction)",   "",                                                                      "vgui/inventory/sleeves/default_faction",           RARITY_STOCK },
	{ "none",                      "No Sleeve (Bare Arms)",      "",                                                                      "vgui/inventory/gloves/bare_hands",                 RARITY_MILSPEC },
	{ "jumpsuit",                  "Phoenix Jumpsuit",           ADVANCED_ARMS_DIR "jumpsuit/v_sleeve_jumpsuit.mdl",                      "vgui/inventory/sleeves/jumpsuit",                  RARITY_MILSPEC },
	{ "separatist",                "Separatist",                 ADVANCED_ARMS_DIR "separatist/v_sleeve_separatist.mdl",                  "vgui/inventory/sleeves/separatist",                RARITY_MILSPEC },
	{ "balkan_v2_variantf",        "Balkan | Dragomir",          ADVANCED_ARMS_DIR "balkan/v_sleeve_balkan_v2_variantf.mdl",              "vgui/inventory/sleeves/balkan_v2_variantf",        RARITY_RESTRICTED },
	{ "balkan_v2_variantg",        "Balkan | Rezan",             ADVANCED_ARMS_DIR "balkan/v_sleeve_balkan_v2_variantg.mdl",              "vgui/inventory/sleeves/balkan_v2_variantg",        RARITY_RESTRICTED },
	{ "balkan_v2_varianth",        "Balkan | Romanov",           ADVANCED_ARMS_DIR "balkan/v_sleeve_balkan_v2_varianth.mdl",              "vgui/inventory/sleeves/balkan_v2_varianth",        RARITY_RESTRICTED },
	{ "balkan_v2_variantj",        "Balkan | Maximus",           ADVANCED_ARMS_DIR "balkan/v_sleeve_balkan_v2_variantj.mdl",              "vgui/inventory/sleeves/balkan_v2_variantj",        RARITY_RESTRICTED },
	{ "balkan_v2_variantk",        "Balkan | Variant K",         ADVANCED_ARMS_DIR "balkan/v_sleeve_balkan_v2_variantk.mdl",              "vgui/inventory/sleeves/balkan_v2_variantk",        RARITY_RESTRICTED },
	{ "balkan_v2_variantl",        "Balkan | Variant L",         ADVANCED_ARMS_DIR "balkan/v_sleeve_balkan_v2_variantl.mdl",              "vgui/inventory/sleeves/balkan_v2_variantl",        RARITY_RESTRICTED },
	{ "balkan",                    "Balkan Classic",             ADVANCED_ARMS_DIR "balkan/v_sleeve_balkan.mdl",                          "vgui/inventory/sleeves/balkan",                    RARITY_MILSPEC },
	{ "professional",              "Professional",               ADVANCED_ARMS_DIR "professional/v_sleeve_professional.mdl",              "vgui/inventory/sleeves/professional",              RARITY_RESTRICTED },
	{ "professional_watch_silver", "Professional (Silver Watch)",ADVANCED_ARMS_DIR "professional/v_professional_watch_silver.mdl",        "vgui/inventory/sleeves/professional_watch_silver", RARITY_CLASSIFIED },
	{ "professional_watch",        "Professional (Gold Watch)",  ADVANCED_ARMS_DIR "professional/v_professional_watch.mdl",               "vgui/inventory/sleeves/professional_watch",        RARITY_CLASSIFIED },
	{ "pirate_watch",              "Pirate (Watch)",             ADVANCED_ARMS_DIR "pirate/v_pirate_watch.mdl",                           "vgui/inventory/sleeves/pirate_watch",              RARITY_CLASSIFIED },
	{ "st6_v2_variante",           "SEAL Team 6 | Deux",         ADVANCED_ARMS_DIR "st6/v_sleeve_st6_v2_variante.mdl",                    "vgui/inventory/sleeves/st6_v2_variante",           RARITY_RESTRICTED },
	{ "green",                     "ST6 Green",                  ADVANCED_ARMS_DIR "st6/v_sleeve_green.mdl",                              "vgui/inventory/sleeves/green",                     RARITY_MILSPEC },
	{ "st6_v2_variantg",           "ST6 | Variant G",            ADVANCED_ARMS_DIR "st6/v_sleeve_st6_v2_variantg.mdl",                    "vgui/inventory/sleeves/st6_v2_variantg",           RARITY_RESTRICTED },
	{ "st6_v2_variantj",           "ST6 | Variant J",            ADVANCED_ARMS_DIR "st6/v_sleeve_st6_v2_variantj.mdl",                    "vgui/inventory/sleeves/st6_v2_variantj",           RARITY_RESTRICTED },
	{ "st6_v2_variantk",           "ST6 | Variant K",            ADVANCED_ARMS_DIR "st6/v_sleeve_st6_v2_variantk.mdl",                    "vgui/inventory/sleeves/st6_v2_variantk",           RARITY_RESTRICTED },
	{ "st6_v2_variantl",           "ST6 | Variant L",            ADVANCED_ARMS_DIR "st6/v_sleeve_st6_v2_variantl.mdl",                    "vgui/inventory/sleeves/st6_v2_variantl",           RARITY_RESTRICTED },
	{ "st6_v2_variantm",           "ST6 | Variant M",            ADVANCED_ARMS_DIR "st6/v_sleeve_st6_v2_variantm.mdl",                    "vgui/inventory/sleeves/st6_v2_variantm",           RARITY_RESTRICTED },
	{ "st6_v2_variantn",           "ST6 | Variant N",            ADVANCED_ARMS_DIR "st6/v_sleeve_st6_v2_variantn.mdl",                    "vgui/inventory/sleeves/st6_v2_variantn",           RARITY_RESTRICTED },
	{ "st6",                       "SEAL Team 6 Classic",        ADVANCED_ARMS_DIR "st6/v_sleeve_st6.mdl",                                "vgui/inventory/sleeves/st6",                       RARITY_MILSPEC },
	{ "idf",                       "IDF",                        ADVANCED_ARMS_DIR "idf/v_sleeve_idf.mdl",                                "vgui/inventory/sleeves/idf",                       RARITY_MILSPEC },
	{ "gign",                      "GIGN",                       ADVANCED_ARMS_DIR "gign/v_sleeve_gign.mdl",                              "vgui/inventory/sleeves/gign",                      RARITY_MILSPEC },
	{ "swat",                      "SWAT",                       ADVANCED_ARMS_DIR "swat/v_sleeve_swat.mdl",                              "vgui/inventory/sleeves/swat",                      RARITY_MILSPEC },
	{ "swat_leader",               "SWAT Leader",                ADVANCED_ARMS_DIR "swat/v_sleeve_swat_leader.mdl",                       "vgui/inventory/sleeves/swat_leader",               RARITY_RESTRICTED },
	{ "swat_breecher",             "SWAT Breecher",              ADVANCED_ARMS_DIR "swat/v_sleeve_swat_breecher.mdl",                     "vgui/inventory/sleeves/swat_breecher",             RARITY_RESTRICTED },
	{ "swat_medic",                "SWAT Medic",                 ADVANCED_ARMS_DIR "swat/v_sleeve_swat_medic.mdl",                        "vgui/inventory/sleeves/swat_medic",                RARITY_RESTRICTED },
	{ "swat_gasmask_green",        "SWAT Hazmat Green",          ADVANCED_ARMS_DIR "swat/v_sleeve_swat_gasmask_green.mdl",                "vgui/inventory/sleeves/swat_gasmask_green",        RARITY_RESTRICTED },
	{ "swat_generic",              "SWAT Generic",               ADVANCED_ARMS_DIR "swat/v_sleeve_swat_generic.mdl",                      "vgui/inventory/sleeves/swat_generic",              RARITY_MILSPEC },
	{ "swat_gasmask_blue",         "SWAT Hazmat Blue",           ADVANCED_ARMS_DIR "swat/v_sleeve_swat_gasmask_blue.mdl",                 "vgui/inventory/sleeves/swat_gasmask_blue",         RARITY_RESTRICTED },
	{ "swat_breecher_variantk",    "SWAT Breecher K",            ADVANCED_ARMS_DIR "swat/v_sleeve_swat_breecher_variantk.mdl",            "vgui/inventory/sleeves/swat_breecher_variantk",    RARITY_RESTRICTED },
	{ "gsg9",                      "GSG-9",                      ADVANCED_ARMS_DIR "gsg9/v_sleeve_gsg9.mdl",                              "vgui/inventory/sleeves/gsg9",                      RARITY_MILSPEC },
	{ "sas_ukmtp",                 "SAS | UKMTP",                ADVANCED_ARMS_DIR "sas/v_sleeve_sas_ukmtp.mdl",                          "vgui/inventory/sleeves/sas_ukmtp",                 RARITY_RESTRICTED },
	{ "sas_variantg",              "SAS | B Squadron",           ADVANCED_ARMS_DIR "sas/v_sleeve_sas_variantg.mdl",                       "vgui/inventory/sleeves/sas_variantg",              RARITY_RESTRICTED },
	{ "sas",                       "SAS Classic",                ADVANCED_ARMS_DIR "sas/v_sleeve_sas.mdl",                                "vgui/inventory/sleeves/sas",                       RARITY_MILSPEC },
	{ "gendarmerie_variantc",      "Gendarmerie C",              ADVANCED_ARMS_DIR "gendarmerie/v_sleeve_gendarmerie_variantc.mdl",       "vgui/inventory/sleeves/gendarmerie_variantc",      RARITY_RESTRICTED },
	{ "gendarmerie",               "Gendarmerie",                ADVANCED_ARMS_DIR "gendarmerie/v_sleeve_gendarmerie.mdl",                "vgui/inventory/sleeves/gendarmerie",               RARITY_RESTRICTED },
	{ "fbi_dark",                  "FBI Dark",                   ADVANCED_ARMS_DIR "fbi/v_sleeve_fbi_dark.mdl",                           "vgui/inventory/sleeves/fbi_dark",                  RARITY_RESTRICTED },
	{ "fbi",                       "FBI Classic",                ADVANCED_ARMS_DIR "fbi/v_sleeve_fbi.mdl",                                "vgui/inventory/sleeves/fbi",                       RARITY_MILSPEC },
	{ "fbi_green",                 "FBI Green",                  ADVANCED_ARMS_DIR "fbi/v_sleeve_fbi_green.mdl",                          "vgui/inventory/sleeves/fbi_green",                 RARITY_RESTRICTED },
	{ "fbi_light_green",           "FBI Light Green",            ADVANCED_ARMS_DIR "fbi/v_sleeve_fbi_light_green.mdl",                    "vgui/inventory/sleeves/fbi_light_green",           RARITY_RESTRICTED },
	{ "diver_wetsuit",             "Diver Wetsuit",              ADVANCED_ARMS_DIR "diver/v_sleeve_diver_wetsuit.mdl",                    "vgui/inventory/sleeves/diver_wetsuit",             RARITY_CLASSIFIED },
	{ "fbi_gray",                  "FBI Gray",                   ADVANCED_ARMS_DIR "fbi/v_sleeve_fbi_gray.mdl",                           "vgui/inventory/sleeves/fbi_gray",                  RARITY_RESTRICTED },
	{ "flektarn",                  "KHS Flecktarn",              ADVANCED_ARMS_DIR "st6/v_sleeve_flektarn.mdl",                           "vgui/inventory/sleeves/flektarn",                  RARITY_RESTRICTED },
	{ "usaf",                      "USAF TACP",                  ADVANCED_ARMS_DIR "st6/v_sleeve_usaf.mdl",                               "vgui/inventory/sleeves/usaf",                      RARITY_RESTRICTED },
	{ "swat_blue",                 "SWAT Blue",                  ADVANCED_ARMS_DIR "swat/v_sleeve_swat_blue.mdl",                         "vgui/inventory/sleeves/swat_blue",                 RARITY_RESTRICTED },
	{ "swat_green",                "SWAT Green",                 ADVANCED_ARMS_DIR "swat/v_sleeve_swat_green.mdl",                        "vgui/inventory/sleeves/swat_green",                RARITY_RESTRICTED },
	{ "anarchist",                 "Anarchist",                  ADVANCED_ARMS_DIR "anarchist/v_sleeve_anarchist.mdl",                    "vgui/inventory/sleeves/anarchist",                 RARITY_MILSPEC },
	{ "wristband",                 "Guerilla Wristband",         ADVANCED_ARMS_DIR "wristband/v_sleeve_wristband.mdl",                    "vgui/inventory/sleeves/wristband",                 RARITY_CLASSIFIED },
};

struct CustomizableWeaponInfo_t
{
	const char *pszWeaponKey;
	const char *pszDisplayName;
	EInventoryWeaponMode eMode;
	int eWeaponID;
	const InventorySkinDef_t *pSkins;
	int nSkinCount;
	const char *pszIconDir;
	const char *pszSkinTexDir;
	const char *pszStickerPrefix;
	ConVar *pSkinConVar;
	ConVar * const *ppStickerConVars;
};

static const CustomizableWeaponInfo_t s_CustomWeapons[INV_SKIN_WEP_COUNT] =
{
	{
		"ak47", "AK-47", INV_MODE_AK47, WEAPON_AK47,
		s_AK47Skins, ARRAYSIZE( s_AK47Skins ),
		"vgui/inventory/ak47",
		"advanced_inventory_csgo_items/models/weapons/v_models/rif_ak47/skins",
		"rif_ak47",
		&cl_ak47_skin, s_pAK47StickerConVars
	},
	{
		"awp", "AWP", INV_MODE_AWP, WEAPON_AWP,
		s_AWPSkins, ARRAYSIZE( s_AWPSkins ),
		"vgui/inventory/awp",
		"advanced_inventory_csgo_items/models/weapons/v_models/snip_awp/skins",
		"snip_awp",
		&cl_awp_skin, s_pAWPStickerConVars
	},
	{
		"deagle", "Desert Eagle", INV_MODE_DEAGLE, WEAPON_DEAGLE,
		s_DeagleSkins, ARRAYSIZE( s_DeagleSkins ),
		"vgui/inventory/deagle",
		"advanced_inventory_csgo_items/models/weapons/v_models/pist_deagle/skins",
		"pist_deagle",
		&cl_deagle_skin, s_pDeagleStickerConVars
	},
	{
		"m4a1", "M4A1", INV_MODE_M4A1, WEAPON_M4A1,
		s_M4A1Skins, ARRAYSIZE( s_M4A1Skins ),
		"vgui/inventory/m4a1",
		"advanced_inventory_csgo_items/models/weapons/v_models/rif_m4a1_s/skins",
		"rif_m4a1_s",
		&cl_m4a1_skin, s_pM4A1StickerConVars
	},
	{
		"glock", "Glock-18", INV_MODE_GLOCK, WEAPON_GLOCK,
		s_GlockSkins, ARRAYSIZE( s_GlockSkins ),
		"vgui/inventory/glock",
		"advanced_inventory_csgo_items/models/weapons/v_models/pist_glock18/skins",
		"pist_glock18",
		&cl_glock_skin, s_pGlockStickerConVars
	},
	{
		"usp", "USP-S", INV_MODE_USP, WEAPON_USP,
		s_USPSkins, ARRAYSIZE( s_USPSkins ),
		"vgui/inventory/usp",
		"advanced_inventory_csgo_items/models/weapons/v_models/pist_223/skins",
		"pist_223",
		&cl_usp_skin, s_pUSPStickerConVars
	},
	{
		"mac10", "MAC-10", INV_MODE_MAC10, WEAPON_MAC10,
		s_MAC10Skins, ARRAYSIZE( s_MAC10Skins ),
		"vgui/inventory/mac10",
		"advanced_inventory_csgo_items/models/weapons/v_models/smg_mac10/skins",
		"smg_mac10",
		&cl_mac10_skin, s_pMAC10StickerConVars
	},
	{
		"mp9", "MP9", INV_MODE_MP9, WEAPON_TMP,
		s_MP9Skins, ARRAYSIZE( s_MP9Skins ),
		"vgui/inventory/mp9",
		"advanced_inventory_csgo_items/models/weapons/v_models/smg_mp9/skins",
		"smg_mp9",
		&cl_mp9_skin, s_pMP9StickerConVars
	},
	{
		"p90", "P90", INV_MODE_P90, WEAPON_P90,
		s_P90Skins, ARRAYSIZE( s_P90Skins ),
		"vgui/inventory/p90",
		"advanced_inventory_csgo_items/models/weapons/v_models/smg_p90/skins",
		"smg_p90",
		&cl_p90_skin, s_pP90StickerConVars
	},
	{
		"nova", "Nova", INV_MODE_NOVA, WEAPON_M3,
		s_NovaSkins, ARRAYSIZE( s_NovaSkins ),
		"vgui/inventory/nova",
		"advanced_inventory_csgo_items/models/weapons/v_models/shot_nova/skins",
		"shot_nova",
		&cl_nova_skin, s_pNovaStickerConVars
	},
	{
		"xm1014", "XM1014", INV_MODE_XM1014, WEAPON_XM1014,
		s_XM1014Skins, ARRAYSIZE( s_XM1014Skins ),
		"vgui/inventory/xm1014",
		"advanced_inventory_csgo_items/models/weapons/v_models/shot_xm1014/skins",
		"shot_xm1014",
		&cl_xm1014_skin, s_pXM1014StickerConVars
	},
	{
		"p228", "P228", INV_MODE_P228, WEAPON_P228,
		s_P228Skins, ARRAYSIZE( s_P228Skins ),
		"vgui/inventory/p228",
		"advanced_inventory_csgo_items/models/weapons/v_models/pist_p250/skins",
		"pist_p250",
		&cl_p228_skin, s_pP228StickerConVars
	},
	{
		"elite", "Dual Elites", INV_MODE_ELITE, WEAPON_ELITE,
		s_EliteSkins, ARRAYSIZE( s_EliteSkins ),
		"vgui/inventory/elite",
		"advanced_inventory_csgo_items/models/weapons/v_models/pist_elite/skins",
		"pist_elite",
		&cl_elite_skin, s_pEliteStickerConVars
	},
	{
		"fiveseven", "Five-SeveN", INV_MODE_FIVESEVEN, WEAPON_FIVESEVEN,
		s_FiveSevenSkins, ARRAYSIZE( s_FiveSevenSkins ),
		"vgui/inventory/fiveseven",
		"advanced_inventory_csgo_items/models/weapons/v_models/pist_fiveseven/skins",
		"pist_fiveseven",
		&cl_fiveseven_skin, s_pFiveSevenStickerConVars
	},
	{
		"mp5", "MP5", INV_MODE_MP5, WEAPON_MP5NAVY,
		s_MP5Skins, ARRAYSIZE( s_MP5Skins ),
		"vgui/inventory/mp5",
		"advanced_inventory_csgo_items/models/weapons/v_models/smg_mp5sd/skins",
		"smg_mp5sd",
		&cl_mp5_skin, s_pMP5StickerConVars
	},
	{
		"ump45", "UMP-45", INV_MODE_UMP45, WEAPON_UMP45,
		s_UMP45Skins, ARRAYSIZE( s_UMP45Skins ),
		"vgui/inventory/ump45",
		"advanced_inventory_csgo_items/models/weapons/v_models/smg_ump45/skins",
		"smg_ump45",
		&cl_ump45_skin, s_pUMP45StickerConVars
	},
	{
		"galil", "Galil", INV_MODE_GALIL, WEAPON_GALIL,
		s_GalilSkins, ARRAYSIZE( s_GalilSkins ),
		"vgui/inventory/galil",
		"advanced_inventory_csgo_items/models/weapons/v_models/rif_galilar/skins",
		"rif_galilar",
		&cl_galil_skin, s_pGalilStickerConVars
	},
	{
		"famas", "FAMAS", INV_MODE_FAMAS, WEAPON_FAMAS,
		s_FAMASSkins, ARRAYSIZE( s_FAMASSkins ),
		"vgui/inventory/famas",
		"advanced_inventory_csgo_items/models/weapons/v_models/rif_famas/skins",
		"rif_famas",
		&cl_famas_skin, s_pFAMASStickerConVars
	},
	{
		"scout", "Scout", INV_MODE_SCOUT, WEAPON_SCOUT,
		s_ScoutSkins, ARRAYSIZE( s_ScoutSkins ),
		"vgui/inventory/scout",
		"advanced_inventory_csgo_items/models/weapons/v_models/snip_ssg08/skins",
		"snip_ssg08",
		&cl_scout_skin, s_pScoutStickerConVars
	},
	{
		"sg550", "SG 550", INV_MODE_SG550, WEAPON_SG550,
		s_SG550Skins, ARRAYSIZE( s_SG550Skins ),
		"vgui/inventory/sg550",
		"advanced_inventory_csgo_items/models/weapons/v_models/snip_scar20/skins",
		"snip_scar20",
		&cl_sg550_skin, s_pSG550StickerConVars
	},
	{
		"sg552", "SG 552", INV_MODE_SG552, WEAPON_SG552,
		s_SG552Skins, ARRAYSIZE( s_SG552Skins ),
		"vgui/inventory/sg552",
		"advanced_inventory_csgo_items/models/weapons/v_models/rif_sg556/skins",
		"rif_sg556",
		&cl_sg552_skin, s_pSG552StickerConVars
	},
	{
		"g3sg1", "G3SG1", INV_MODE_G3SG1, WEAPON_G3SG1,
		s_G3SG1Skins, ARRAYSIZE( s_G3SG1Skins ),
		"vgui/inventory/g3sg1",
		"advanced_inventory_csgo_items/models/weapons/v_models/snip_g3sg1/skins",
		"snip_g3sg1",
		&cl_g3sg1_skin, s_pG3SG1StickerConVars
	},
	{
		"aug", "AUG", INV_MODE_AUG, WEAPON_AUG,
		s_AUGSkins, ARRAYSIZE( s_AUGSkins ),
		"vgui/inventory/aug",
		"advanced_inventory_csgo_items/models/weapons/v_models/rif_aug/skins",
		"rif_aug",
		&cl_aug_skin, s_pAUGStickerConVars
	},
	{
		"m249", "M249", INV_MODE_M249, WEAPON_M249,
		s_M249Skins, ARRAYSIZE( s_M249Skins ),
		"vgui/inventory/m249",
		"advanced_inventory_csgo_items/models/weapons/v_models/mach_m249para/skins",
		"mach_m249para",
		&cl_m249_skin, s_pM249StickerConVars
	},
};

static const CustomizableWeaponInfo_t *GetCustomWeaponInfo( int nWep )
{
	if ( nWep < 0 || nWep >= INV_SKIN_WEP_COUNT )
		return NULL;
	return &s_CustomWeapons[nWep];
}

static InventoryWeaponLoadout_t s_WeaponLoadouts[INV_SKIN_WEP_COUNT][INV_TEAM_COUNT];
static InventoryWeaponLoadout_t s_PreviewLoadout;
static int s_nPreviewWeaponIdx = INV_SKIN_WEP_AK47;
static char s_szKnifeLoadout[INV_TEAM_COUNT][64];
static char s_szKnifeSkinLoadout[INV_TEAM_COUNT][64];
static char s_szGloveLoadout[INV_TEAM_COUNT][64];
static char s_szSleeveLoadout[INV_TEAM_COUNT][64];
static char s_szPreviewGloveId[64] = "default";
static char s_szPreviewKnifeKey[64] = "0";
static char s_szPreviewKnifeSkin[64] = "0";

static bool s_bLoadoutInitialized = false;
static bool s_bSyncingConVars = false;
static int s_nLastSyncedTeamNumber = -1;

static int s_nPreviewSkinToken = 0;
static int s_nPreviewStickerTokens[INV_MAX_STICKER_SLOTS] = { 0, 1, 2, 3 };
static int s_nPreviewGloveToken = 0;

static bool MatchesNormalizedToken( const char *pszInput, const char *pszCandidate )
{
	if ( !pszInput || !pszCandidate )
		return false;

	const char *a = pszInput;
	const char *b = pszCandidate;
	while ( *a && *b )
	{
		while ( *a == ' ' || *a == '_' || *a == '-' ) ++a;
		while ( *b == ' ' || *b == '_' || *b == '-' ) ++b;
		if ( !*a || !*b )
			break;
		if ( tolower( (unsigned char)*a ) != tolower( (unsigned char)*b ) )
			return false;
		++a;
		++b;
	}
	while ( *a == ' ' || *a == '_' || *a == '-' ) ++a;
	while ( *b == ' ' || *b == '_' || *b == '-' ) ++b;
	return ( *a == '\0' && *b == '\0' );
}

template<typename T>
static const char *ResolveTableKey( const char *pszChoice, const T *pEntries, int nCount )
{
	if ( !pszChoice || !pszChoice[0] || !V_stricmp( pszChoice, "0" ) || !V_stricmp( pszChoice, "default" ) || !V_stricmp( pszChoice, "none" ) )
		return NULL;

	bool bDigitsOnly = true;
	for ( const char *p = pszChoice; *p; ++p )
	{
		if ( !isdigit( (unsigned char)*p ) )
		{
			bDigitsOnly = false;
			break;
		}
	}

	if ( bDigitsOnly )
	{
		int nIndex = atoi( pszChoice );
		if ( nIndex >= 1 && nIndex <= nCount )
			return pEntries[nIndex - 1].pszKey;
		return NULL;
	}

	for ( int i = 0; i < nCount; ++i )
	{
		if ( MatchesNormalizedToken( pszChoice, pEntries[i].pszKey ) ||
			 MatchesNormalizedToken( pszChoice, pEntries[i].pszDisplayName ) )
			return pEntries[i].pszKey;
	}

	return NULL;
}

static void NormalizeSkinKey( int nWep, const char *pszInput, char *pszOut, int nOutSize )
{
	const CustomizableWeaponInfo_t *pInfo = GetCustomWeaponInfo( nWep );
	const char *pszKey = pInfo ? ResolveTableKey( pszInput, pInfo->pSkins, pInfo->nSkinCount ) : NULL;
	V_strncpy( pszOut, pszKey ? pszKey : "0", nOutSize );
}

static void NormalizeStickerKey( const char *pszInput, char *pszOut, int nOutSize )
{
	const char *pszKey = ResolveTableKey( pszInput, s_Stickers, ARRAYSIZE( s_Stickers ) );
	V_strncpy( pszOut, pszKey ? pszKey : "0", nOutSize );
}

static void NormalizeKnifeKey( const char *pszInput, char *pszOut, int nOutSize )
{
	const InventoryKnifeDef_t *pDef = Inventory_FindKnifeByKey( pszInput );
	V_strncpy( pszOut, pDef ? pDef->pszKey : "0", nOutSize );
}

static void NormalizeKnifeSkinKey( const char *pszKnifeKey, const char *pszSkinInput, char *pszOut, int nOutSize )
{
	const InventoryKnifeDef_t *pKnife = Inventory_FindKnifeByKey( pszKnifeKey );
	const char *pszKey = ( pKnife && pKnife->pSkins && pKnife->nSkinCount > 0 )
		? ResolveTableKey( pszSkinInput, pKnife->pSkins, pKnife->nSkinCount )
		: NULL;
	V_strncpy( pszOut, pszKey ? pszKey : "0", nOutSize );
}

static void NormalizeGloveId( const char *pszInput, char *pszOut, int nOutSize )
{
	const InventoryGloveDef_t *pDef = Inventory_FindGloveById( pszInput );
	V_strncpy( pszOut, pDef ? pDef->pszId : "default", nOutSize );
}

static void NormalizeSleeveKey( const char *pszInput, char *pszOut, int nOutSize )
{
	const InventorySleeveDef_t *pDef = Inventory_FindSleeveByKey( pszInput );
	V_strncpy( pszOut, pDef ? pDef->pszKey : "-1", nOutSize );
}

static void ResetLoadoutEntry( InventoryWeaponLoadout_t &entry )
{
	V_strncpy( entry.szSkinKey, "0", sizeof( entry.szSkinKey ) );
	for ( int i = 0; i < INV_MAX_STICKER_SLOTS; ++i )
	{
		V_strncpy( entry.szStickerKeys[i], "0", sizeof( entry.szStickerKeys[i] ) );
	}
}

static void PushWeaponLoadoutToConVars( int nWep, const InventoryWeaponLoadout_t &entry )
{
	const CustomizableWeaponInfo_t *pInfo = GetCustomWeaponInfo( nWep );
	if ( !pInfo )
		return;

	s_bSyncingConVars = true;
	pInfo->pSkinConVar->SetValue( entry.szSkinKey );
	for ( int i = 0; i < INV_MAX_STICKER_SLOTS; ++i )
	{
		pInfo->ppStickerConVars[i]->SetValue( entry.szStickerKeys[i] );
	}
	s_bSyncingConVars = false;
}

static void PushKnifeToConVar( const char *pszKnifeKey, const char *pszKnifeSkin )
{
	s_bSyncingConVars = true;
	cl_knife.SetValue( ( pszKnifeKey && pszKnifeKey[0] ) ? pszKnifeKey : "0" );
	cl_knife_skin.SetValue( ( pszKnifeSkin && pszKnifeSkin[0] ) ? pszKnifeSkin : "0" );
	s_bSyncingConVars = false;
}

static void PushGloveToConVars( const char *pszGloveId )
{
	const InventoryGloveDef_t *pDef = Inventory_FindGloveById( pszGloveId );
	if ( !pDef )
	{
		cl_arm_glove.SetValue( "-1" );
		return;
	}

	cl_arm_glove.SetValue( pDef->pszFamilyKey );
	ConVar *pSkinVar = GetGloveSkinConVarForFamily( pDef->pszFamilyKey );
	if ( pSkinVar )
	{
		pSkinVar->SetValue( pDef->pszSkinKey );
	}
}

static void PushSleeveToConVar( const char *pszSleeveKey )
{
	cl_arm_sleeve.SetValue( ( pszSleeveKey && pszSleeveKey[0] ) ? pszSleeveKey : "-1" );
}

static void PushTeamAllToConVars( EInventoryTeam eTeam )
{
	int idx = clamp( (int)eTeam, 0, INV_TEAM_COUNT - 1 );
	for ( int w = 0; w < INV_SKIN_WEP_COUNT; ++w )
	{
		PushWeaponLoadoutToConVars( w, s_WeaponLoadouts[w][idx] );
	}
	PushKnifeToConVar( s_szKnifeLoadout[idx], s_szKnifeSkinLoadout[idx] );
	PushGloveToConVars( s_szGloveLoadout[idx] );
	PushSleeveToConVar( s_szSleeveLoadout[idx] );
}

static void LoadSingleTeamSection( int nWep, KeyValues *pSection, InventoryWeaponLoadout_t &outEntry )
{
	if ( !pSection )
		return;

	NormalizeSkinKey( nWep, pSection->GetString( "skin", "0" ), outEntry.szSkinKey, sizeof( outEntry.szSkinKey ) );
	for ( int i = 0; i < INV_MAX_STICKER_SLOTS; ++i )
	{
		char szKeyName[16];
		V_snprintf( szKeyName, sizeof( szKeyName ), "sticker_%d", i + 1 );
		NormalizeStickerKey( pSection->GetString( szKeyName, "0" ), outEntry.szStickerKeys[i], sizeof( outEntry.szStickerKeys[i] ) );
	}
}

static void SaveSingleTeamSection( KeyValues *pParent, const char *pszSectionName, const InventoryWeaponLoadout_t &entry )
{
	KeyValues *pSec = pParent->FindKey( pszSectionName, true );
	if ( !pSec )
		return;

	pSec->SetString( "skin", entry.szSkinKey[0] ? entry.szSkinKey : "0" );
	for ( int i = 0; i < INV_MAX_STICKER_SLOTS; ++i )
	{
		char szKeyName[16];
		V_snprintf( szKeyName, sizeof( szKeyName ), "sticker_%d", i + 1 );
		pSec->SetString( szKeyName, entry.szStickerKeys[i][0] ? entry.szStickerKeys[i] : "0" );
	}
}

static bool GetActiveLocalTeam( EInventoryTeam &outTeam )
{
	C_CSPlayer *pLocal = C_CSPlayer::GetLocalCSPlayer();
	if ( !pLocal )
		return false;

	int nTeam = pLocal->GetTeamNumber();
	if ( nTeam == TEAM_CT )
	{
		outTeam = INV_TEAM_CT;
		return true;
	}
	if ( nTeam == TEAM_TERRORIST )
	{
		outTeam = INV_TEAM_T;
		return true;
	}
	return false;
}

static const char *ResolveFamilyBaseGloveId( const char *pszFamilyKey )
{
	if ( !pszFamilyKey || !pszFamilyKey[0] )
		return NULL;
	static const struct
	{
		const char *pszFamily;
		const char *pszBaseId;
	} s_FamilyDefaults[] =
	{
		{ "hardknuckle", "hardknuckle" },
		{ "bloodhound",  "bloodhound_guerrilla" },
		{ "sporty",      "sporty_black_webbing_yellow" },
		{ "slick",       "slick_snakeskin_white" },
		{ "handwrap",    "handwrap_brown_beige" },
		{ "moto",        "moto_trigrid_blue" },
		{ "specialist",  "specialist_ddpat_dense_green_brown" },
		{ "hydra",       "hydra_black_green" },
		{ "brokenfang",  "brokenfang_poison_frog_black_yellow" },
	};
	for ( int i = 0; i < ARRAYSIZE( s_FamilyDefaults ); ++i )
	{
		if ( !V_stricmp( pszFamilyKey, s_FamilyDefaults[i].pszFamily ) )
			return s_FamilyDefaults[i].pszBaseId;
	}
	return NULL;
}

static void SeedGloveFromCurrentConVars( char *pszOutId, int nOutSize )
{
	const char *pszGlove = cl_arm_glove.GetString();
	if ( !pszGlove || !pszGlove[0] || !V_strcmp( pszGlove, "-1" ) )
	{
		V_strncpy( pszOutId, "default", nOutSize );
		return;
	}

	const InventoryGloveDef_t *pMatch = Inventory_FindGloveById( pszGlove );
	if ( pMatch )
	{
		// Check family skin ConVar too
		ConVar *pSkinVar = GetGloveSkinConVarForFamily( pMatch->pszFamilyKey );
		if ( pSkinVar && pSkinVar->GetString()[0] && V_strcmp( pSkinVar->GetString(), "0" ) )
		{
			const char *pszSkinVal = pSkinVar->GetString();
			if ( !V_stricmp( pMatch->pszFamilyKey, "hardknuckle" ) )
			{
				if ( !V_strcmp( pszSkinVal, "1" ) ) pszSkinVal = "blue";
				else if ( !V_strcmp( pszSkinVal, "2" ) ) pszSkinVal = "black";
			}
			char szCandidate[96];
			V_snprintf( szCandidate, sizeof( szCandidate ), "%s_%s", pMatch->pszFamilyKey, pszSkinVal );
			if ( const InventoryGloveDef_t *pSkinMatch = Inventory_FindGloveById( szCandidate ) )
			{
				V_strncpy( pszOutId, pSkinMatch->pszId, nOutSize );
				return;
			}
		}
		V_strncpy( pszOutId, pMatch->pszId, nOutSize );
		return;
	}

	V_strncpy( pszOutId, "default", nOutSize );
}

static void SeedWeaponFromConVars( int nWep, InventoryWeaponLoadout_t &outEntry )
{
	const CustomizableWeaponInfo_t *pInfo = GetCustomWeaponInfo( nWep );
	if ( !pInfo )
	{
		ResetLoadoutEntry( outEntry );
		return;
	}

	NormalizeSkinKey( nWep, pInfo->pSkinConVar->GetString(), outEntry.szSkinKey, sizeof( outEntry.szSkinKey ) );
	for ( int i = 0; i < INV_MAX_STICKER_SLOTS; ++i )
	{
		NormalizeStickerKey( pInfo->ppStickerConVars[i]->GetString(), outEntry.szStickerKeys[i], sizeof( outEntry.szStickerKeys[i] ) );
	}
}

void Inventory_EnsureLoaded()
{
	if ( s_bLoadoutInitialized )
		return;

	s_bLoadoutInitialized = true;
	for ( int t = 0; t < INV_TEAM_COUNT; ++t )
	{
		for ( int w = 0; w < INV_SKIN_WEP_COUNT; ++w )
		{
			ResetLoadoutEntry( s_WeaponLoadouts[w][t] );
		}
		V_strncpy( s_szKnifeLoadout[t], "0", sizeof( s_szKnifeLoadout[t] ) );
		V_strncpy( s_szKnifeSkinLoadout[t], "0", sizeof( s_szKnifeSkinLoadout[t] ) );
		V_strncpy( s_szGloveLoadout[t], "default", sizeof( s_szGloveLoadout[t] ) );
		V_strncpy( s_szSleeveLoadout[t], "-1", sizeof( s_szSleeveLoadout[t] ) );
	}
	ResetLoadoutEntry( s_PreviewLoadout );

	KeyValues *pKV = new KeyValues( "CSGOInventory" );
	if ( filesystem && pKV->LoadFromFile( filesystem, INVENTORY_LOADOUT_FILE, "MOD" ) )
	{
		for ( int w = 0; w < INV_SKIN_WEP_COUNT; ++w )
		{
			const CustomizableWeaponInfo_t *pInfo = &s_CustomWeapons[w];
			if ( KeyValues *pWepSec = pKV->FindKey( pInfo->pszWeaponKey ) )
			{
				LoadSingleTeamSection( w, pWepSec->FindKey( "ct" ), s_WeaponLoadouts[w][INV_TEAM_CT] );
				LoadSingleTeamSection( w, pWepSec->FindKey( "t" ), s_WeaponLoadouts[w][INV_TEAM_T] );
			}
			else
			{
				InventoryWeaponLoadout_t seed;
				SeedWeaponFromConVars( w, seed );
				s_WeaponLoadouts[w][INV_TEAM_CT] = seed;
				s_WeaponLoadouts[w][INV_TEAM_T] = seed;
			}
		}
		if ( KeyValues *pKnives = pKV->FindKey( "knives" ) )
		{
			NormalizeKnifeKey( pKnives->GetString( "ct", "0" ), s_szKnifeLoadout[INV_TEAM_CT], sizeof( s_szKnifeLoadout[INV_TEAM_CT] ) );
			NormalizeKnifeSkinKey( s_szKnifeLoadout[INV_TEAM_CT], pKnives->GetString( "ct_skin", "0" ), s_szKnifeSkinLoadout[INV_TEAM_CT], sizeof( s_szKnifeSkinLoadout[INV_TEAM_CT] ) );
			NormalizeKnifeKey( pKnives->GetString( "t", "0" ), s_szKnifeLoadout[INV_TEAM_T], sizeof( s_szKnifeLoadout[INV_TEAM_T] ) );
			NormalizeKnifeSkinKey( s_szKnifeLoadout[INV_TEAM_T], pKnives->GetString( "t_skin", "0" ), s_szKnifeSkinLoadout[INV_TEAM_T], sizeof( s_szKnifeSkinLoadout[INV_TEAM_T] ) );
		}
		else
		{
			NormalizeKnifeKey( cl_knife.GetString(), s_szKnifeLoadout[INV_TEAM_CT], sizeof( s_szKnifeLoadout[INV_TEAM_CT] ) );
			NormalizeKnifeSkinKey( s_szKnifeLoadout[INV_TEAM_CT], cl_knife_skin.GetString(), s_szKnifeSkinLoadout[INV_TEAM_CT], sizeof( s_szKnifeSkinLoadout[INV_TEAM_CT] ) );
			V_strncpy( s_szKnifeLoadout[INV_TEAM_T], s_szKnifeLoadout[INV_TEAM_CT], sizeof( s_szKnifeLoadout[INV_TEAM_T] ) );
			V_strncpy( s_szKnifeSkinLoadout[INV_TEAM_T], s_szKnifeSkinLoadout[INV_TEAM_CT], sizeof( s_szKnifeSkinLoadout[INV_TEAM_T] ) );
		}
		if ( KeyValues *pGloves = pKV->FindKey( "gloves" ) )
		{
			NormalizeGloveId( pGloves->GetString( "ct", "default" ), s_szGloveLoadout[INV_TEAM_CT], sizeof( s_szGloveLoadout[INV_TEAM_CT] ) );
			NormalizeGloveId( pGloves->GetString( "t", "default" ), s_szGloveLoadout[INV_TEAM_T], sizeof( s_szGloveLoadout[INV_TEAM_T] ) );
		}
		else
		{
			SeedGloveFromCurrentConVars( s_szGloveLoadout[INV_TEAM_CT], sizeof( s_szGloveLoadout[INV_TEAM_CT] ) );
			V_strncpy( s_szGloveLoadout[INV_TEAM_T], s_szGloveLoadout[INV_TEAM_CT], sizeof( s_szGloveLoadout[INV_TEAM_T] ) );
		}
		if ( KeyValues *pSleeves = pKV->FindKey( "sleeves" ) )
		{
			NormalizeSleeveKey( pSleeves->GetString( "ct", "-1" ), s_szSleeveLoadout[INV_TEAM_CT], sizeof( s_szSleeveLoadout[INV_TEAM_CT] ) );
			NormalizeSleeveKey( pSleeves->GetString( "t", "-1" ), s_szSleeveLoadout[INV_TEAM_T], sizeof( s_szSleeveLoadout[INV_TEAM_T] ) );
		}
		else
		{
			NormalizeSleeveKey( cl_arm_sleeve.GetString(), s_szSleeveLoadout[INV_TEAM_CT], sizeof( s_szSleeveLoadout[INV_TEAM_CT] ) );
			V_strncpy( s_szSleeveLoadout[INV_TEAM_T], s_szSleeveLoadout[INV_TEAM_CT], sizeof( s_szSleeveLoadout[INV_TEAM_T] ) );
		}
	}
	else
	{
		for ( int w = 0; w < INV_SKIN_WEP_COUNT; ++w )
		{
			InventoryWeaponLoadout_t seed;
			SeedWeaponFromConVars( w, seed );
			s_WeaponLoadouts[w][INV_TEAM_CT] = seed;
			s_WeaponLoadouts[w][INV_TEAM_T] = seed;
		}

		NormalizeKnifeKey( cl_knife.GetString(), s_szKnifeLoadout[INV_TEAM_CT], sizeof( s_szKnifeLoadout[INV_TEAM_CT] ) );
		NormalizeKnifeSkinKey( s_szKnifeLoadout[INV_TEAM_CT], cl_knife_skin.GetString(), s_szKnifeSkinLoadout[INV_TEAM_CT], sizeof( s_szKnifeSkinLoadout[INV_TEAM_CT] ) );
		V_strncpy( s_szKnifeLoadout[INV_TEAM_T], s_szKnifeLoadout[INV_TEAM_CT], sizeof( s_szKnifeLoadout[INV_TEAM_T] ) );
		V_strncpy( s_szKnifeSkinLoadout[INV_TEAM_T], s_szKnifeSkinLoadout[INV_TEAM_CT], sizeof( s_szKnifeSkinLoadout[INV_TEAM_T] ) );

		SeedGloveFromCurrentConVars( s_szGloveLoadout[INV_TEAM_CT], sizeof( s_szGloveLoadout[INV_TEAM_CT] ) );
		V_strncpy( s_szGloveLoadout[INV_TEAM_T], s_szGloveLoadout[INV_TEAM_CT], sizeof( s_szGloveLoadout[INV_TEAM_T] ) );

		NormalizeSleeveKey( cl_arm_sleeve.GetString(), s_szSleeveLoadout[INV_TEAM_CT], sizeof( s_szSleeveLoadout[INV_TEAM_CT] ) );
		V_strncpy( s_szSleeveLoadout[INV_TEAM_T], s_szSleeveLoadout[INV_TEAM_CT], sizeof( s_szSleeveLoadout[INV_TEAM_T] ) );
	}
	pKV->deleteThis();

	EInventoryTeam eTeam = INV_TEAM_T;
	if ( !GetActiveLocalTeam( eTeam ) )
	{
		eTeam = INV_TEAM_T;
	}
	PushTeamAllToConVars( eTeam );
}

void Inventory_SaveLoadout()
{
	Inventory_EnsureLoaded();
	if ( !filesystem )
		return;

	filesystem->CreateDirHierarchy( "cfg", "MOD" );

	KeyValues *pKV = new KeyValues( "CSGOInventory" );
	for ( int w = 0; w < INV_SKIN_WEP_COUNT; ++w )
	{
		if ( KeyValues *pWepSec = pKV->FindKey( s_CustomWeapons[w].pszWeaponKey, true ) )
		{
			SaveSingleTeamSection( pWepSec, "ct", s_WeaponLoadouts[w][INV_TEAM_CT] );
			SaveSingleTeamSection( pWepSec, "t", s_WeaponLoadouts[w][INV_TEAM_T] );
		}
	}
	if ( KeyValues *pKnives = pKV->FindKey( "knives", true ) )
	{
		pKnives->SetString( "ct", s_szKnifeLoadout[INV_TEAM_CT] );
		pKnives->SetString( "ct_skin", s_szKnifeSkinLoadout[INV_TEAM_CT][0] ? s_szKnifeSkinLoadout[INV_TEAM_CT] : "0" );
		pKnives->SetString( "t", s_szKnifeLoadout[INV_TEAM_T] );
		pKnives->SetString( "t_skin", s_szKnifeSkinLoadout[INV_TEAM_T][0] ? s_szKnifeSkinLoadout[INV_TEAM_T] : "0" );
	}
	if ( KeyValues *pGloves = pKV->FindKey( "gloves", true ) )
	{
		pGloves->SetString( "ct", s_szGloveLoadout[INV_TEAM_CT] );
		pGloves->SetString( "t", s_szGloveLoadout[INV_TEAM_T] );
	}
	if ( KeyValues *pSleeves = pKV->FindKey( "sleeves", true ) )
	{
		pSleeves->SetString( "ct", s_szSleeveLoadout[INV_TEAM_CT] );
		pSleeves->SetString( "t", s_szSleeveLoadout[INV_TEAM_T] );
	}

	pKV->SaveToFile( filesystem, INVENTORY_LOADOUT_FILE, "MOD" );
	pKV->deleteThis();
}

void Inventory_SyncActiveTeamConVars( int nTeamNumber )
{
	Inventory_EnsureLoaded();
	if ( nTeamNumber != TEAM_CT && nTeamNumber != TEAM_TERRORIST )
		return;

	if ( s_nLastSyncedTeamNumber == nTeamNumber )
		return;

	s_nLastSyncedTeamNumber = nTeamNumber;
	EInventoryTeam eTeam = ( nTeamNumber == TEAM_CT ) ? INV_TEAM_CT : INV_TEAM_T;
	PushTeamAllToConVars( eTeam );
}

const InventoryWeaponLoadout_t &Inventory_GetWeaponLoadout( int nWep, EInventoryTeam eTeam )
{
	Inventory_EnsureLoaded();
	int w = clamp( nWep, 0, INV_SKIN_WEP_COUNT - 1 );
	return s_WeaponLoadouts[w][clamp( (int)eTeam, 0, INV_TEAM_COUNT - 1 )];
}

int Inventory_GetWeaponSkinEquippedMask( int nWep, const char *pszSkinKey )
{
	Inventory_EnsureLoaded();
	if ( nWep < 0 || nWep >= INV_SKIN_WEP_COUNT )
		return INV_TEAMMASK_NONE;

	char szNorm[64];
	NormalizeSkinKey( nWep, pszSkinKey, szNorm, sizeof( szNorm ) );

	int nMask = INV_TEAMMASK_NONE;
	if ( !V_stricmp( s_WeaponLoadouts[nWep][INV_TEAM_CT].szSkinKey, szNorm ) )
		nMask |= INV_TEAMMASK_CT;
	if ( !V_stricmp( s_WeaponLoadouts[nWep][INV_TEAM_T].szSkinKey, szNorm ) )
		nMask |= INV_TEAMMASK_T;
	return nMask;
}

void Inventory_EquipWeapon( int nWep, int nTeamMask, const char *pszSkinKey, const char * const pszStickerKeys[INV_MAX_STICKER_SLOTS] )
{
	Inventory_EnsureLoaded();
	if ( nWep < 0 || nWep >= INV_SKIN_WEP_COUNT || nTeamMask == INV_TEAMMASK_NONE )
		return;

	InventoryWeaponLoadout_t updated;
	NormalizeSkinKey( nWep, pszSkinKey, updated.szSkinKey, sizeof( updated.szSkinKey ) );
	for ( int i = 0; i < INV_MAX_STICKER_SLOTS; ++i )
	{
		NormalizeStickerKey( pszStickerKeys ? pszStickerKeys[i] : "0", updated.szStickerKeys[i], sizeof( updated.szStickerKeys[i] ) );
	}

	if ( nTeamMask & INV_TEAMMASK_CT )
		s_WeaponLoadouts[nWep][INV_TEAM_CT] = updated;
	if ( nTeamMask & INV_TEAMMASK_T )
		s_WeaponLoadouts[nWep][INV_TEAM_T] = updated;

	EInventoryTeam activeTeam = INV_TEAM_T;
	if ( GetActiveLocalTeam( activeTeam ) )
	{
		PushWeaponLoadoutToConVars( nWep, s_WeaponLoadouts[nWep][activeTeam] );
	}
	else if ( nTeamMask & INV_TEAMMASK_T )
	{
		PushWeaponLoadoutToConVars( nWep, s_WeaponLoadouts[nWep][INV_TEAM_T] );
	}
	else
	{
		PushWeaponLoadoutToConVars( nWep, s_WeaponLoadouts[nWep][INV_TEAM_CT] );
	}

	Inventory_SaveLoadout();
}

void Inventory_OnWeaponConVarChanged( int nWep )
{
	if ( s_bSyncingConVars || !s_bLoadoutInitialized || nWep < 0 || nWep >= INV_SKIN_WEP_COUNT )
		return;

	InventoryWeaponLoadout_t entry;
	SeedWeaponFromConVars( nWep, entry );

	EInventoryTeam activeTeam = INV_TEAM_T;
	if ( GetActiveLocalTeam( activeTeam ) )
	{
		s_WeaponLoadouts[nWep][activeTeam] = entry;
	}
	else
	{
		s_WeaponLoadouts[nWep][INV_TEAM_CT] = entry;
		s_WeaponLoadouts[nWep][INV_TEAM_T] = entry;
	}
	Inventory_SaveLoadout();
}

static void OnWeaponSkinConVarChangeCallback( IConVar *var, const char *pOldValue, float flOldValue )
{
	if ( !var )
		return;

	const char *pszName = var->GetName();
	if ( !pszName )
		return;

	for ( int w = 0; w < INV_SKIN_WEP_COUNT; ++w )
	{
		char szPrefix[32];
		V_snprintf( szPrefix, sizeof( szPrefix ), "cl_%s_", s_CustomWeapons[w].pszWeaponKey );
		if ( !V_strnicmp( pszName, szPrefix, V_strlen( szPrefix ) ) )
		{
			Inventory_OnWeaponConVarChanged( w );
			return;
		}
	}
}

//-----------------------------------------------------------------------------
// Knives accessors & team loadout
//-----------------------------------------------------------------------------
static void OnKnifeSkinConVarChangeCallback( IConVar *var, const char *pOldValue, float flOldValue )
{
	if ( s_bSyncingConVars || !s_bLoadoutInitialized )
		return;

	char szNormKnife[64], szNormSkin[64];
	NormalizeKnifeKey( cl_knife.GetString(), szNormKnife, sizeof( szNormKnife ) );
	NormalizeKnifeSkinKey( szNormKnife, cl_knife_skin.GetString(), szNormSkin, sizeof( szNormSkin ) );

	EInventoryTeam activeTeam = INV_TEAM_T;
	if ( GetActiveLocalTeam( activeTeam ) )
	{
		V_strncpy( s_szKnifeLoadout[activeTeam], szNormKnife, sizeof( s_szKnifeLoadout[activeTeam] ) );
		V_strncpy( s_szKnifeSkinLoadout[activeTeam], szNormSkin, sizeof( s_szKnifeSkinLoadout[activeTeam] ) );
	}
	else
	{
		V_strncpy( s_szKnifeLoadout[INV_TEAM_CT], szNormKnife, sizeof( s_szKnifeLoadout[INV_TEAM_CT] ) );
		V_strncpy( s_szKnifeLoadout[INV_TEAM_T], szNormKnife, sizeof( s_szKnifeLoadout[INV_TEAM_T] ) );
		V_strncpy( s_szKnifeSkinLoadout[INV_TEAM_CT], szNormSkin, sizeof( s_szKnifeSkinLoadout[INV_TEAM_CT] ) );
		V_strncpy( s_szKnifeSkinLoadout[INV_TEAM_T], szNormSkin, sizeof( s_szKnifeSkinLoadout[INV_TEAM_T] ) );
	}
	Inventory_SaveLoadout();
}

int Inventory_GetKnifeCount()
{
	return ARRAYSIZE( s_Knives );
}

const InventoryKnifeDef_t *Inventory_GetKnifeDef( int iIndex )
{
	if ( iIndex < 0 || iIndex >= ARRAYSIZE( s_Knives ) )
		return NULL;
	return &s_Knives[iIndex];
}

int Inventory_FindKnifeIndexByKey( const char *pszKey )
{
	const InventoryKnifeDef_t *pDef = Inventory_FindKnifeByKey( pszKey );
	if ( !pDef )
		return 0;
	return (int)( pDef - s_Knives );
}

const InventoryKnifeDef_t *Inventory_FindKnifeByKey( const char *pszKey )
{
	if ( !pszKey || !pszKey[0] || !V_stricmp( pszKey, "0" ) || !V_stricmp( pszKey, "default" ) || !V_stricmp( pszKey, "none" ) )
		return &s_Knives[0];

	bool bDigitsOnly = true;
	for ( const char *p = pszKey; *p; ++p )
	{
		if ( !isdigit( (unsigned char)*p ) ) { bDigitsOnly = false; break; }
	}
	if ( bDigitsOnly )
	{
		int nIdx = atoi( pszKey );
		if ( nIdx >= 1 && nIdx < ARRAYSIZE( s_Knives ) )
			return &s_Knives[nIdx];
		return &s_Knives[0];
	}

	for ( int i = 0; i < ARRAYSIZE( s_Knives ); ++i )
	{
		if ( MatchesNormalizedToken( pszKey, s_Knives[i].pszKey ) ||
			 MatchesNormalizedToken( pszKey, s_Knives[i].pszDisplayName ) )
			return &s_Knives[i];
	}

	if ( !V_stricmp( pszKey, "karam" ) ) return Inventory_FindKnifeByKey( "karambit" );
	if ( !V_stricmp( pszKey, "m9" ) ) return Inventory_FindKnifeByKey( "m9_bayonet" );
	if ( !V_stricmp( pszKey, "huntsman" ) ) return Inventory_FindKnifeByKey( "tactical" );
	if ( !V_stricmp( pszKey, "bowie" ) ) return Inventory_FindKnifeByKey( "survival_bowie" );
	if ( !V_stricmp( pszKey, "daggers" ) ) return Inventory_FindKnifeByKey( "push" );
	if ( !V_stricmp( pszKey, "paracord" ) ) return Inventory_FindKnifeByKey( "cord" );
	if ( !V_stricmp( pszKey, "survival" ) ) return Inventory_FindKnifeByKey( "canis" );
	if ( !V_stricmp( pszKey, "navaja" ) ) return Inventory_FindKnifeByKey( "gypsy_jackknife" );
	if ( !V_stricmp( pszKey, "nomad" ) ) return Inventory_FindKnifeByKey( "outdoor" );
	if ( !V_stricmp( pszKey, "talon" ) ) return Inventory_FindKnifeByKey( "widowmaker" );

	return &s_Knives[0];
}

const char *Inventory_GetEquippedKnifeKey( EInventoryTeam eTeam )
{
	Inventory_EnsureLoaded();
	return s_szKnifeLoadout[clamp( (int)eTeam, 0, INV_TEAM_COUNT - 1 )];
}

const char *Inventory_GetEquippedKnifeSkinKey( EInventoryTeam eTeam )
{
	Inventory_EnsureLoaded();
	return s_szKnifeSkinLoadout[clamp( (int)eTeam, 0, INV_TEAM_COUNT - 1 )];
}

int Inventory_GetKnifeEquippedMask( const char *pszKnifeKey )
{
	Inventory_EnsureLoaded();
	char szNorm[64];
	NormalizeKnifeKey( pszKnifeKey, szNorm, sizeof( szNorm ) );

	int nMask = INV_TEAMMASK_NONE;
	if ( !V_stricmp( s_szKnifeLoadout[INV_TEAM_CT], szNorm ) )
		nMask |= INV_TEAMMASK_CT;
	if ( !V_stricmp( s_szKnifeLoadout[INV_TEAM_T], szNorm ) )
		nMask |= INV_TEAMMASK_T;
	return nMask;
}

int Inventory_GetKnifeSkinCount( const char *pszKnifeKey )
{
	const InventoryKnifeDef_t *pDef = Inventory_FindKnifeByKey( pszKnifeKey );
	return pDef ? pDef->nSkinCount : 0;
}

const InventorySkinDef_t *Inventory_GetKnifeSkinDef( const char *pszKnifeKey, int iIndex )
{
	const InventoryKnifeDef_t *pDef = Inventory_FindKnifeByKey( pszKnifeKey );
	if ( !pDef || !pDef->pSkins || iIndex < 0 || iIndex >= pDef->nSkinCount )
		return NULL;
	return &pDef->pSkins[iIndex];
}

const InventorySkinDef_t *Inventory_FindKnifeSkinByKey( const char *pszKnifeKey, const char *pszSkinKey )
{
	const InventoryKnifeDef_t *pDef = Inventory_FindKnifeByKey( pszKnifeKey );
	if ( !pDef || !pDef->pSkins || pDef->nSkinCount <= 0 )
		return NULL;

	const char *pszResolved = ResolveTableKey( pszSkinKey, pDef->pSkins, pDef->nSkinCount );
	if ( !pszResolved )
		return NULL;

	for ( int i = 0; i < pDef->nSkinCount; ++i )
	{
		if ( !V_stricmp( pDef->pSkins[i].pszKey, pszResolved ) )
			return &pDef->pSkins[i];
	}
	return NULL;
}

void Inventory_GetKnifeSkinIconPath( const char *pszKnifeKey, const char *pszSkinKey, char *pszOutPath, int nOutSize )
{
	const InventoryKnifeDef_t *pDef = Inventory_FindKnifeByKey( pszKnifeKey );
	if ( !pDef || !V_stricmp( pDef->pszKey, "0" ) )
	{
		V_strncpy( pszOutPath, "vgui/inventory/knives/default", nOutSize );
		return;
	}

	const InventorySkinDef_t *pSkin = Inventory_FindKnifeSkinByKey( pDef->pszKey, pszSkinKey );
	if ( pSkin )
	{
		V_snprintf( pszOutPath, nOutSize, "vgui/inventory/knives/%s/%s", pDef->pszKey, pSkin->pszKey );
	}
	else
	{
		V_strncpy( pszOutPath, pDef->pszIconPath ? pDef->pszIconPath : "vgui/inventory/knives/default", nOutSize );
	}
}

int Inventory_GetKnifeSkinEquippedMask( const char *pszKnifeKey, const char *pszSkinKey )
{
	Inventory_EnsureLoaded();
	char szNormKnife[64], szNormSkin[64];
	NormalizeKnifeKey( pszKnifeKey, szNormKnife, sizeof( szNormKnife ) );
	NormalizeKnifeSkinKey( szNormKnife, pszSkinKey, szNormSkin, sizeof( szNormSkin ) );

	int nMask = INV_TEAMMASK_NONE;
	if ( !V_stricmp( s_szKnifeLoadout[INV_TEAM_CT], szNormKnife ) && !V_stricmp( s_szKnifeSkinLoadout[INV_TEAM_CT], szNormSkin ) )
		nMask |= INV_TEAMMASK_CT;
	if ( !V_stricmp( s_szKnifeLoadout[INV_TEAM_T], szNormKnife ) && !V_stricmp( s_szKnifeSkinLoadout[INV_TEAM_T], szNormSkin ) )
		nMask |= INV_TEAMMASK_T;
	return nMask;
}

void Inventory_SetPreviewKnife( const char *pszKnifeKey, const char *pszSkinKey )
{
	NormalizeKnifeKey( pszKnifeKey, s_szPreviewKnifeKey, sizeof( s_szPreviewKnifeKey ) );
	NormalizeKnifeSkinKey( s_szPreviewKnifeKey, pszSkinKey, s_szPreviewKnifeSkin, sizeof( s_szPreviewKnifeSkin ) );
}

void Inventory_EquipKnife( int nTeamMask, const char *pszKnifeKey, const char *pszSkinKey )
{
	Inventory_EnsureLoaded();
	if ( nTeamMask == INV_TEAMMASK_NONE )
		return;

	char szNorm[64], szSkinNorm[64];
	NormalizeKnifeKey( pszKnifeKey, szNorm, sizeof( szNorm ) );
	NormalizeKnifeSkinKey( szNorm, pszSkinKey ? pszSkinKey : "0", szSkinNorm, sizeof( szSkinNorm ) );
	if ( nTeamMask & INV_TEAMMASK_CT )
	{
		V_strncpy( s_szKnifeLoadout[INV_TEAM_CT], szNorm, sizeof( s_szKnifeLoadout[INV_TEAM_CT] ) );
		V_strncpy( s_szKnifeSkinLoadout[INV_TEAM_CT], szSkinNorm, sizeof( s_szKnifeSkinLoadout[INV_TEAM_CT] ) );
	}
	if ( nTeamMask & INV_TEAMMASK_T )
	{
		V_strncpy( s_szKnifeLoadout[INV_TEAM_T], szNorm, sizeof( s_szKnifeLoadout[INV_TEAM_T] ) );
		V_strncpy( s_szKnifeSkinLoadout[INV_TEAM_T], szSkinNorm, sizeof( s_szKnifeSkinLoadout[INV_TEAM_T] ) );
	}

	EInventoryTeam activeTeam = INV_TEAM_T;
	if ( GetActiveLocalTeam( activeTeam ) )
	{
		PushKnifeToConVar( s_szKnifeLoadout[activeTeam], s_szKnifeSkinLoadout[activeTeam] );
	}
	else if ( nTeamMask & INV_TEAMMASK_T )
	{
		PushKnifeToConVar( s_szKnifeLoadout[INV_TEAM_T], s_szKnifeSkinLoadout[INV_TEAM_T] );
	}
	else
	{
		PushKnifeToConVar( s_szKnifeLoadout[INV_TEAM_CT], s_szKnifeSkinLoadout[INV_TEAM_CT] );
	}
	Inventory_SaveLoadout();
}

//-----------------------------------------------------------------------------
// Gloves accessors & team loadout
//-----------------------------------------------------------------------------
int Inventory_GetGloveCount()
{
	return ARRAYSIZE( s_Gloves );
}

const InventoryGloveDef_t *Inventory_GetGloveDef( int iIndex )
{
	if ( iIndex < 0 || iIndex >= ARRAYSIZE( s_Gloves ) )
		return NULL;
	return &s_Gloves[iIndex];
}

const InventoryGloveDef_t *Inventory_FindGloveById( const char *pszId )
{
	if ( !pszId || !pszId[0] || !V_stricmp( pszId, "-1" ) || !V_stricmp( pszId, "default" ) )
		return &s_Gloves[0];

	if ( !V_stricmp( pszId, "0" ) || !V_stricmp( pszId, "none" ) || !V_stricmp( pszId, "bare_hands" ) )
		return &s_Gloves[1];

	for ( int i = 0; i < ARRAYSIZE( s_Gloves ); ++i )
	{
		if ( !V_stricmp( pszId, s_Gloves[i].pszId ) || MatchesNormalizedToken( pszId, s_Gloves[i].pszDisplayName ) )
			return &s_Gloves[i];
	}

	// Match family default / legacy "*_default" IDs to the canonical base skin
	const char *pszBaseId = ResolveFamilyBaseGloveId( pszId );
	if ( !pszBaseId )
	{
		const char *pszSuffix = V_stristr( pszId, "_default" );
		if ( pszSuffix && pszSuffix[8] == '\0' && pszSuffix > pszId )
		{
			char szFamily[48];
			int nLen = (int)( pszSuffix - pszId );
			if ( nLen > 0 && nLen < (int)sizeof( szFamily ) )
			{
				memcpy( szFamily, pszId, nLen );
				szFamily[nLen] = '\0';
				pszBaseId = ResolveFamilyBaseGloveId( szFamily );
			}
		}
	}
	if ( pszBaseId )
	{
		for ( int i = 0; i < ARRAYSIZE( s_Gloves ); ++i )
		{
			if ( !V_stricmp( pszBaseId, s_Gloves[i].pszId ) )
				return &s_Gloves[i];
		}
	}
	return &s_Gloves[0];
}

const char *Inventory_GetEquippedGloveId( EInventoryTeam eTeam )
{
	Inventory_EnsureLoaded();
	return s_szGloveLoadout[clamp( (int)eTeam, 0, INV_TEAM_COUNT - 1 )];
}

int Inventory_GetGloveEquippedMask( const char *pszGloveId )
{
	Inventory_EnsureLoaded();
	char szNorm[64];
	NormalizeGloveId( pszGloveId, szNorm, sizeof( szNorm ) );

	int nMask = INV_TEAMMASK_NONE;
	if ( !V_stricmp( s_szGloveLoadout[INV_TEAM_CT], szNorm ) )
		nMask |= INV_TEAMMASK_CT;
	if ( !V_stricmp( s_szGloveLoadout[INV_TEAM_T], szNorm ) )
		nMask |= INV_TEAMMASK_T;
	return nMask;
}

void Inventory_EquipGlove( int nTeamMask, const char *pszGloveId )
{
	Inventory_EnsureLoaded();
	if ( nTeamMask == INV_TEAMMASK_NONE )
		return;

	char szNorm[64];
	NormalizeGloveId( pszGloveId, szNorm, sizeof( szNorm ) );
	if ( nTeamMask & INV_TEAMMASK_CT )
		V_strncpy( s_szGloveLoadout[INV_TEAM_CT], szNorm, sizeof( s_szGloveLoadout[INV_TEAM_CT] ) );
	if ( nTeamMask & INV_TEAMMASK_T )
		V_strncpy( s_szGloveLoadout[INV_TEAM_T], szNorm, sizeof( s_szGloveLoadout[INV_TEAM_T] ) );

	EInventoryTeam activeTeam = INV_TEAM_T;
	if ( GetActiveLocalTeam( activeTeam ) )
	{
		PushGloveToConVars( s_szGloveLoadout[activeTeam] );
	}
	else
	{
		PushGloveToConVars( szNorm );
	}
	Inventory_SaveLoadout();
}

//-----------------------------------------------------------------------------
// Sleeves accessors & team loadout
//-----------------------------------------------------------------------------
int Inventory_GetSleeveCount()
{
	return ARRAYSIZE( s_Sleeves );
}

const InventorySleeveDef_t *Inventory_GetSleeveDef( int iIndex )
{
	if ( iIndex < 0 || iIndex >= ARRAYSIZE( s_Sleeves ) )
		return NULL;
	return &s_Sleeves[iIndex];
}

const InventorySleeveDef_t *Inventory_FindSleeveByKey( const char *pszKey )
{
	if ( !pszKey || !pszKey[0] || !V_stricmp( pszKey, "-1" ) || !V_stricmp( pszKey, "default" ) )
		return &s_Sleeves[0];
	if ( !V_stricmp( pszKey, "0" ) || !V_stricmp( pszKey, "none" ) || !V_stricmp( pszKey, "off" ) )
		return &s_Sleeves[1];

	bool bDigitsOnly = true;
	for ( const char *p = pszKey; *p; ++p )
	{
		if ( !isdigit( (unsigned char)*p ) ) { bDigitsOnly = false; break; }
	}
	if ( bDigitsOnly )
	{
		int nIdx = atoi( pszKey );
		if ( nIdx >= 1 && ( nIdx + 1 ) < ARRAYSIZE( s_Sleeves ) )
			return &s_Sleeves[nIdx + 1];
		return &s_Sleeves[0];
	}

	for ( int i = 0; i < ARRAYSIZE( s_Sleeves ); ++i )
	{
		if ( MatchesNormalizedToken( pszKey, s_Sleeves[i].pszKey ) ||
			 MatchesNormalizedToken( pszKey, s_Sleeves[i].pszDisplayName ) )
			return &s_Sleeves[i];
	}
	return &s_Sleeves[0];
}

const char *Inventory_GetEquippedSleeveKey( EInventoryTeam eTeam )
{
	Inventory_EnsureLoaded();
	return s_szSleeveLoadout[clamp( (int)eTeam, 0, INV_TEAM_COUNT - 1 )];
}

int Inventory_GetSleeveEquippedMask( const char *pszSleeveKey )
{
	Inventory_EnsureLoaded();
	char szNorm[64];
	NormalizeSleeveKey( pszSleeveKey, szNorm, sizeof( szNorm ) );

	int nMask = INV_TEAMMASK_NONE;
	if ( !V_stricmp( s_szSleeveLoadout[INV_TEAM_CT], szNorm ) )
		nMask |= INV_TEAMMASK_CT;
	if ( !V_stricmp( s_szSleeveLoadout[INV_TEAM_T], szNorm ) )
		nMask |= INV_TEAMMASK_T;
	return nMask;
}

void Inventory_EquipSleeve( int nTeamMask, const char *pszSleeveKey )
{
	Inventory_EnsureLoaded();
	if ( nTeamMask == INV_TEAMMASK_NONE )
		return;

	char szNorm[64];
	NormalizeSleeveKey( pszSleeveKey, szNorm, sizeof( szNorm ) );
	if ( nTeamMask & INV_TEAMMASK_CT )
		V_strncpy( s_szSleeveLoadout[INV_TEAM_CT], szNorm, sizeof( s_szSleeveLoadout[INV_TEAM_CT] ) );
	if ( nTeamMask & INV_TEAMMASK_T )
		V_strncpy( s_szSleeveLoadout[INV_TEAM_T], szNorm, sizeof( s_szSleeveLoadout[INV_TEAM_T] ) );

	EInventoryTeam activeTeam = INV_TEAM_T;
	if ( GetActiveLocalTeam( activeTeam ) )
	{
		PushSleeveToConVar( s_szSleeveLoadout[activeTeam] );
	}
	else
	{
		PushSleeveToConVar( szNorm );
	}
	Inventory_SaveLoadout();
}

//-----------------------------------------------------------------------------
// Live Preview & Proxy Resolvers
//-----------------------------------------------------------------------------
void Inventory_SetPreviewSelection( int nWep, const char *pszSkinKey, const char * const pszStickerKeys[INV_MAX_STICKER_SLOTS] )
{
	s_nPreviewWeaponIdx = ( nWep >= 0 && nWep < INV_SKIN_WEP_COUNT ) ? nWep : INV_SKIN_WEP_AK47;
	NormalizeSkinKey( s_nPreviewWeaponIdx, pszSkinKey, s_PreviewLoadout.szSkinKey, sizeof( s_PreviewLoadout.szSkinKey ) );
	for ( int i = 0; i < INV_MAX_STICKER_SLOTS; ++i )
	{
		NormalizeStickerKey( pszStickerKeys ? pszStickerKeys[i] : "0", s_PreviewLoadout.szStickerKeys[i], sizeof( s_PreviewLoadout.szStickerKeys[i] ) );
	}
}

void Inventory_SetPreviewGlove( const char *pszGloveId )
{
	NormalizeGloveId( pszGloveId, s_szPreviewGloveId, sizeof( s_szPreviewGloveId ) );
}

void *Inventory_GetPreviewSkinProxyToken()
{
	return &s_nPreviewSkinToken;
}

void *Inventory_GetPreviewStickerProxyToken( int nSlot )
{
	return &s_nPreviewStickerTokens[clamp( nSlot, 0, INV_MAX_STICKER_SLOTS - 1 )];
}

void *Inventory_GetPreviewGloveProxyToken()
{
	return &s_nPreviewGloveToken;
}

static int PreviewGloveSkinCallback( const char *pszFamilyKey, char *pszOutStr, int nOutSize )
{
	const InventoryGloveDef_t *pDef = Inventory_FindGloveById( s_szPreviewGloveId );
	if ( pDef && pszFamilyKey && !V_stricmp( pDef->pszFamilyKey, pszFamilyKey ) )
	{
		if ( pszOutStr && nOutSize > 0 )
		{
			V_strncpy( pszOutStr, pDef->pszSkinKey, nOutSize );
		}
		return atoi( pDef->pszSkinKey );
	}
	if ( pszOutStr && nOutSize > 0 )
		pszOutStr[0] = '\0';
	return 0;
}

bool Inventory_ResolvePreviewGloveOverrideTexture( void *pRenderable, char *pszOutPath, int nOutSize )
{
	if ( !pszOutPath || nOutSize <= 0 || pRenderable != &s_nPreviewGloveToken )
		return false;

	const InventoryGloveDef_t *pDef = Inventory_FindGloveById( s_szPreviewGloveId );
	if ( !pDef || !V_strcmp( pDef->pszFamilyKey, "-1" ) )
		return false;

	ArmGloveSelection sel;
	if ( ResolveViewmodelGloveSelection( pDef->pszFamilyKey, PreviewGloveSkinCallback, sel ) &&
		 sel.pszOverrideBaseTexture && sel.pszOverrideBaseTexture[0] )
	{
		V_strncpy( pszOutPath, sel.pszOverrideBaseTexture, nOutSize );
		return true;
	}
	return false;
}

void Inventory_ResolvePreviewArmsForTeam( EInventoryTeam eTeam, const char *pszOverrideGloveId, const char *pszOverrideSleeveKey,
	const char *&outGloveModel, const char *&outGloveOverrideTex, const char *&outSleeveModel, bool &outHideBare )
{
	Inventory_EnsureLoaded();
	int idx = clamp( (int)eTeam, 0, INV_TEAM_COUNT - 1 );

	// Default faction arms
	const char *pszDefaultGlove = ( idx == INV_TEAM_CT )
		? ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle.mdl"
		: ADVANCED_ARMS_DIR "glove_fullfinger/v_glove_fullfinger.mdl";
	const char *pszDefaultSleeve = ( idx == INV_TEAM_CT )
		? ADVANCED_ARMS_DIR "swat/v_sleeve_swat.mdl"
		: "";

	const char *pszGloveId = ( pszOverrideGloveId && pszOverrideGloveId[0] ) ? pszOverrideGloveId : s_szGloveLoadout[idx];
	const InventoryGloveDef_t *pGlove = Inventory_FindGloveById( pszGloveId );

	outGloveModel = pszDefaultGlove;
	outGloveOverrideTex = NULL;

	if ( pGlove && V_strcmp( pGlove->pszFamilyKey, "-1" ) != 0 )
	{
		V_strncpy( s_szPreviewGloveId, pGlove->pszId, sizeof( s_szPreviewGloveId ) );
		ArmGloveSelection sel;
		if ( ResolveViewmodelGloveSelection( pGlove->pszFamilyKey, PreviewGloveSkinCallback, sel ) && sel.pszModelPath )
		{
			outGloveModel = sel.pszModelPath;
			outGloveOverrideTex = sel.pszOverrideBaseTexture;
		}
		else if ( pGlove->pszModelPath && pGlove->pszModelPath[0] )
		{
			outGloveModel = pGlove->pszModelPath;
		}
	}

	const char *pszSleeveKey = ( pszOverrideSleeveKey && pszOverrideSleeveKey[0] ) ? pszOverrideSleeveKey : s_szSleeveLoadout[idx];
	const InventorySleeveDef_t *pSleeve = Inventory_FindSleeveByKey( pszSleeveKey );
	outSleeveModel = "";
	if ( !pSleeve || !V_strcmp( pSleeve->pszKey, "-1" ) )
	{
		outSleeveModel = pszDefaultSleeve;
	}
	else if ( !V_stricmp( pSleeve->pszKey, "none" ) )
	{
		outSleeveModel = "";
	}
	else if ( pSleeve->pszModelPath && pSleeve->pszModelPath[0] )
	{
		outSleeveModel = pSleeve->pszModelPath;
	}

	outHideBare = ( outSleeveModel[0] != '\0' );
}

int Inventory_GetWeaponCount()
{
	return ARRAYSIZE( s_WeaponCatalog );
}

const InventoryWeaponDef_t *Inventory_GetWeaponDef( int iIndex )
{
	if ( iIndex < 0 || iIndex >= ARRAYSIZE( s_WeaponCatalog ) )
		return NULL;
	return &s_WeaponCatalog[iIndex];
}

const char *Inventory_GetCategoryName( EInventoryCategory eCategory )
{
	switch ( eCategory )
	{
	case INV_CAT_ALL:     return "All";
	case INV_CAT_PISTOLS: return "Pistols";
	case INV_CAT_SMGS:    return "SMGs";
	case INV_CAT_RIFLES:  return "Rifles";
	case INV_CAT_HEAVY:   return "Heavy";
	case INV_CAT_KNIVES:  return "Knives";
	case INV_CAT_GLOVES:  return "Gloves";
	case INV_CAT_SLEEVES: return "Sleeves";
	default:              return "";
	}
}

const char *Inventory_GetCategoryLocToken( EInventoryCategory eCategory )
{
	switch ( eCategory )
	{
	case INV_CAT_ALL:     return "#CSInventory_Cat_All";
	case INV_CAT_PISTOLS: return "#CSInventory_Cat_Pistols";
	case INV_CAT_SMGS:    return "#CSInventory_Cat_SMGs";
	case INV_CAT_RIFLES:  return "#CSInventory_Cat_Rifles";
	case INV_CAT_HEAVY:   return "#CSInventory_Cat_Heavy";
	case INV_CAT_KNIVES:  return "#CSInventory_Cat_Knives";
	case INV_CAT_GLOVES:  return "#CSInventory_Cat_Gloves";
	case INV_CAT_SLEEVES: return "#CSInventory_Cat_Sleeves";
	default:              return "";
	}
}

int Inventory_GetWeaponSkinIndexFromMode( EInventoryWeaponMode eMode )
{
	for ( int w = 0; w < INV_SKIN_WEP_COUNT; ++w )
	{
		if ( s_CustomWeapons[w].eMode == eMode )
			return w;
	}
	return -1;
}

const char *Inventory_GetWeaponSkinDisplayName( int nWep )
{
	const CustomizableWeaponInfo_t *pInfo = GetCustomWeaponInfo( nWep );
	return pInfo ? pInfo->pszDisplayName : "";
}

int Inventory_GetWeaponSkinCount( int nWep )
{
	const CustomizableWeaponInfo_t *pInfo = GetCustomWeaponInfo( nWep );
	return pInfo ? pInfo->nSkinCount : 0;
}

const InventorySkinDef_t *Inventory_GetWeaponSkinDef( int nWep, int iIndex )
{
	const CustomizableWeaponInfo_t *pInfo = GetCustomWeaponInfo( nWep );
	if ( !pInfo || iIndex < 0 || iIndex >= pInfo->nSkinCount )
		return NULL;
	return &pInfo->pSkins[iIndex];
}

const InventorySkinDef_t *Inventory_FindWeaponSkinByKey( int nWep, const char *pszKey )
{
	const CustomizableWeaponInfo_t *pInfo = GetCustomWeaponInfo( nWep );
	if ( !pInfo )
		return NULL;

	const char *pszResolved = ResolveTableKey( pszKey, pInfo->pSkins, pInfo->nSkinCount );
	if ( !pszResolved )
		return NULL;

	for ( int i = 0; i < pInfo->nSkinCount; ++i )
	{
		if ( !V_stricmp( pInfo->pSkins[i].pszKey, pszResolved ) )
			return &pInfo->pSkins[i];
	}
	return NULL;
}

void Inventory_GetWeaponSkinIconPath( int nWep, const char *pszSkinKey, char *pszOutPath, int nOutSize )
{
	const CustomizableWeaponInfo_t *pInfo = GetCustomWeaponInfo( nWep );
	if ( !pInfo )
	{
		V_strncpy( pszOutPath, "", nOutSize );
		return;
	}

	const InventorySkinDef_t *pSkin = Inventory_FindWeaponSkinByKey( nWep, pszSkinKey );
	V_snprintf( pszOutPath, nOutSize, "%s/%s", pInfo->pszIconDir, pSkin ? pSkin->pszKey : "0" );
}

int Inventory_GetStickerCount()
{
	return ARRAYSIZE( s_Stickers );
}

const InventoryStickerDef_t *Inventory_GetStickerDef( int iIndex )
{
	if ( iIndex < 0 || iIndex >= ARRAYSIZE( s_Stickers ) )
		return NULL;
	return &s_Stickers[iIndex];
}

const InventoryStickerDef_t *Inventory_FindStickerByKey( const char *pszKey )
{
	const char *pszResolved = ResolveTableKey( pszKey, s_Stickers, ARRAYSIZE( s_Stickers ) );
	if ( !pszResolved )
		return NULL;

	for ( int i = 0; i < ARRAYSIZE( s_Stickers ); ++i )
	{
		if ( !V_stricmp( s_Stickers[i].pszKey, pszResolved ) )
			return &s_Stickers[i];
	}
	return NULL;
}

void Inventory_GetWeaponStickerModelPath( int nWep, int nSlot, char *pszOutPath, int nOutSize )
{
	const CustomizableWeaponInfo_t *pInfo = GetCustomWeaponInfo( nWep );
	int s = clamp( nSlot, 0, INV_MAX_STICKER_SLOTS - 1 );
	if ( !pInfo )
	{
		V_strncpy( pszOutPath, "", nOutSize );
		return;
	}

	static const char s_szSlotLetters[INV_MAX_STICKER_SLOTS] = { 'a', 'b', 'c', 'd' };
	V_snprintf( pszOutPath, nOutSize, "models/advanced_inventory_csgo_items/weapons/stickers/v_models/%s_decal_%c.mdl", pInfo->pszStickerPrefix, s_szSlotLetters[s] );
}

void Inventory_GetStickerTexturePath( const char *pszKey, char *pszOutPath, int nOutSize )
{
	V_snprintf( pszOutPath, nOutSize, "advanced_inventory_csgo_items/models/weapons/customization/stickers/%s", pszKey ? pszKey : "" );
}

static bool IsPreviewStickerToken( void *pRenderable, int &outSlot )
{
	for ( int s = 0; s < INV_MAX_STICKER_SLOTS; ++s )
	{
		if ( pRenderable == &s_nPreviewStickerTokens[s] )
		{
			outSlot = s;
			return true;
		}
	}
	return false;
}

static C_BaseEntity *SafeGetBaseEntityFromRenderable( void *pRenderable )
{
	if ( !pRenderable )
		return NULL;
	IClientRenderable *pRend = (IClientRenderable *)pRenderable;
	IClientUnknown *pUnknown = pRend->GetIClientUnknown();
	return pUnknown ? pUnknown->GetBaseEntity() : NULL;
}

static int FindSkinWeaponByCSWeapon( C_WeaponCSBase *pCSWeapon, C_BaseViewModel *pVM = NULL, int nWepHint = -1 )
{
	if ( !pCSWeapon )
		return -1;

	for ( int w = 0; w < INV_SKIN_WEP_COUNT; ++w )
	{
		if ( pCSWeapon->IsA( (CSWeaponID)s_CustomWeapons[w].eWeaponID ) )
			return w;
	}
	return -1;
}

static const char *GetEquippedWeaponSkinForTeamNumber( int nWep, int nTeam )
{
	Inventory_EnsureLoaded();
	const CustomizableWeaponInfo_t *pInfo = GetCustomWeaponInfo( nWep );
	if ( !pInfo )
		return "0";

	if ( nTeam == TEAM_CT )
		return s_WeaponLoadouts[nWep][INV_TEAM_CT].szSkinKey;
	if ( nTeam == TEAM_TERRORIST )
		return s_WeaponLoadouts[nWep][INV_TEAM_T].szSkinKey;
	return pInfo->pSkinConVar->GetString();
}

static const char *GetEquippedWeaponStickerForTeamNumber( int nWep, int nTeam, int nSlot )
{
	Inventory_EnsureLoaded();
	const CustomizableWeaponInfo_t *pInfo = GetCustomWeaponInfo( nWep );
	if ( !pInfo )
		return "0";

	int s = clamp( nSlot, 0, INV_MAX_STICKER_SLOTS - 1 );
	if ( nTeam == TEAM_CT )
		return s_WeaponLoadouts[nWep][INV_TEAM_CT].szStickerKeys[s];
	if ( nTeam == TEAM_TERRORIST )
		return s_WeaponLoadouts[nWep][INV_TEAM_T].szStickerKeys[s];
	return pInfo->ppStickerConVars[s]->GetString();
}

bool Inventory_ResolveWeaponSkinChoiceForRenderable( int nWepHint, void *pRenderable, char *pszOutChoice, int nOutSize, bool &bOutShouldApply )
{
	bOutShouldApply = false;
	if ( !pszOutChoice || nOutSize <= 0 )
		return false;
	pszOutChoice[0] = '\0';

	if ( nWepHint >= INV_KNIFE_HINT_BASE )
	{
		int nKnifeIdx = nWepHint - INV_KNIFE_HINT_BASE;
		const InventoryKnifeDef_t *pKnifeDef = Inventory_GetKnifeDef( nKnifeIdx );
		if ( !pKnifeDef || pKnifeDef->nSkinCount <= 0 )
			return false;

		if ( pRenderable == &s_nPreviewSkinToken )
		{
			if ( V_stricmp( s_szPreviewKnifeKey, pKnifeDef->pszKey ) != 0 )
				return false;
			bOutShouldApply = true;
			V_strncpy( pszOutChoice, s_szPreviewKnifeSkin, nOutSize );
			return true;
		}

		C_BaseEntity *pBaseEntity = SafeGetBaseEntityFromRenderable( pRenderable );
		if ( !pBaseEntity )
			return false;

		C_BaseEntity *pOwner = NULL;
		C_WeaponCSBase *pCSWeapon = NULL;
		if ( C_BaseViewModel *pVM = dynamic_cast<C_BaseViewModel *>( pBaseEntity ) )
		{
			pCSWeapon = dynamic_cast<C_WeaponCSBase *>( pVM->GetOwningWeapon() );
			pOwner = pVM->GetOwner();
		}
		else if ( ( pCSWeapon = dynamic_cast<C_WeaponCSBase *>( pBaseEntity ) ) != NULL )
		{
			pOwner = pCSWeapon->GetPlayerOwner();
		}

		if ( !pCSWeapon || !pCSWeapon->IsA( WEAPON_KNIFE ) )
			return false;

		C_BasePlayer *pLocal = C_BasePlayer::GetLocalPlayer();
		if ( pOwner && pOwner != pLocal )
			return false;

		int nTeam = pOwner ? pOwner->GetTeamNumber() : ( pLocal ? pLocal->GetTeamNumber() : TEAM_UNASSIGNED );
		Inventory_EnsureLoaded();
		const char *pszEqKnife = ( nTeam == TEAM_CT ) ? s_szKnifeLoadout[INV_TEAM_CT] : ( ( nTeam == TEAM_TERRORIST ) ? s_szKnifeLoadout[INV_TEAM_T] : cl_knife.GetString() );
		char szEqNorm[64];
		NormalizeKnifeKey( pszEqKnife, szEqNorm, sizeof( szEqNorm ) );
		if ( V_stricmp( szEqNorm, pKnifeDef->pszKey ) != 0 )
			return false;

		const char *pszSkin = ( nTeam == TEAM_CT ) ? s_szKnifeSkinLoadout[INV_TEAM_CT] : ( ( nTeam == TEAM_TERRORIST ) ? s_szKnifeSkinLoadout[INV_TEAM_T] : cl_knife_skin.GetString() );
		V_strncpy( pszOutChoice, ( pszSkin && pszSkin[0] ) ? pszSkin : "0", nOutSize );
		bOutShouldApply = true;
		return true;
	}

	if ( pRenderable == &s_nPreviewSkinToken )
	{
		if ( nWepHint >= 0 && nWepHint != s_nPreviewWeaponIdx )
			return false;
		bOutShouldApply = true;
		V_strncpy( pszOutChoice, s_PreviewLoadout.szSkinKey, nOutSize );
		return true;
	}

	C_BaseEntity *pBaseEntity = SafeGetBaseEntityFromRenderable( pRenderable );
	if ( !pBaseEntity )
		return false;

	if ( C_BaseViewModel *pVM = dynamic_cast<C_BaseViewModel *>( pBaseEntity ) )
	{
		C_WeaponCSBase *pCSWeapon = dynamic_cast<C_WeaponCSBase *>( pVM->GetOwningWeapon() );
		int nWep = FindSkinWeaponByCSWeapon( pCSWeapon, pVM, nWepHint );
		if ( nWep < 0 || ( nWepHint >= 0 && nWepHint != nWep ) )
			return false;

		C_BaseEntity *pOwner = pVM->GetOwner();
		int nTeam = pOwner ? pOwner->GetTeamNumber() : TEAM_UNASSIGNED;
		V_strncpy( pszOutChoice, GetEquippedWeaponSkinForTeamNumber( nWep, nTeam ), nOutSize );
		bOutShouldApply = true;
		return true;
	}

	if ( C_WeaponCSBase *pCSWeapon = dynamic_cast<C_WeaponCSBase *>( pBaseEntity ) )
	{
		int nWep = FindSkinWeaponByCSWeapon( pCSWeapon, NULL, nWepHint );
		if ( nWep < 0 || ( nWepHint >= 0 && nWepHint != nWep ) )
			return false;

		C_BasePlayer *pLocal = C_BasePlayer::GetLocalPlayer();
		C_BasePlayer *pOwner = pCSWeapon->GetPlayerOwner();
		if ( pOwner && pOwner != pLocal )
			return false;

		int nTeam = pOwner ? pOwner->GetTeamNumber() : ( pLocal ? pLocal->GetTeamNumber() : TEAM_UNASSIGNED );
		V_strncpy( pszOutChoice, GetEquippedWeaponSkinForTeamNumber( nWep, nTeam ), nOutSize );
		bOutShouldApply = true;
		return true;
	}

	return false;
}

bool Inventory_ResolveWeaponSkinTexturePath( int nWepHint, const char *pszChoice, char *pszOutPath, int nOutSize )
{
	if ( nWepHint >= INV_KNIFE_HINT_BASE )
	{
		const InventoryKnifeDef_t *pKnife = Inventory_GetKnifeDef( nWepHint - INV_KNIFE_HINT_BASE );
		if ( !pKnife || !pKnife->pSkins || !pKnife->pszSkinTexDir )
			return false;

		const char *pszKey = ResolveTableKey( pszChoice, pKnife->pSkins, pKnife->nSkinCount );
		if ( !pszKey )
			return false;

		V_snprintf( pszOutPath, nOutSize, "%s/%s", pKnife->pszSkinTexDir, pszKey );
		return true;
	}

	const CustomizableWeaponInfo_t *pInfo = GetCustomWeaponInfo( nWepHint );
	if ( !pInfo )
		return false;

	const char *pszKey = ResolveTableKey( pszChoice, pInfo->pSkins, pInfo->nSkinCount );
	if ( !pszKey )
		return false;

	V_snprintf( pszOutPath, nOutSize, "%s/%s", pInfo->pszSkinTexDir, pszKey );
	return true;
}

bool Inventory_ResolveWeaponStickerChoiceForRenderable( int nWepHint, void *pRenderable, int nSlot, char *pszOutChoice, int nOutSize )
{
	if ( !pszOutChoice || nOutSize <= 0 )
		return false;
	pszOutChoice[0] = '\0';

	int nPreviewSlot = -1;
	if ( IsPreviewStickerToken( pRenderable, nPreviewSlot ) )
	{
		if ( nWepHint >= 0 && nWepHint != s_nPreviewWeaponIdx )
			return false;
		V_strncpy( pszOutChoice, s_PreviewLoadout.szStickerKeys[clamp( nSlot, 0, INV_MAX_STICKER_SLOTS - 1 )], nOutSize );
		return true;
	}

	int nWep = nWepHint;
	if ( nWep < 0 || nWep >= INV_SKIN_WEP_COUNT )
	{
		C_CSPlayer *pLocal = C_CSPlayer::GetLocalCSPlayer();
		C_BaseViewModel *pVM = pLocal ? pLocal->GetViewModel() : NULL;
		C_WeaponCSBase *pCSWeapon = pVM ? dynamic_cast<C_WeaponCSBase *>( pVM->GetOwningWeapon() ) : NULL;
		nWep = FindSkinWeaponByCSWeapon( pCSWeapon, pVM, nWepHint );
	}
	if ( nWep < 0 || nWep >= INV_SKIN_WEP_COUNT )
		return false;

	C_CSPlayer *pLocal = C_CSPlayer::GetLocalCSPlayer();
	int nTeam = pLocal ? pLocal->GetTeamNumber() : TEAM_UNASSIGNED;
	V_strncpy( pszOutChoice, GetEquippedWeaponStickerForTeamNumber( nWep, nTeam, nSlot ), nOutSize );
	return true;
}

bool Inventory_ResolveStickerTexturePath( const char *pszChoice, char *pszOutPath, int nOutSize )
{
	const char *pszKey = ResolveTableKey( pszChoice, s_Stickers, ARRAYSIZE( s_Stickers ) );
	if ( !pszKey )
		return false;

	Inventory_GetStickerTexturePath( pszKey, pszOutPath, nOutSize );
	return true;
}

void Inventory_GetActiveStickersForViewmodel( int nWep, C_BaseViewModel *pVM, C_CSPlayer *pPlayer, bool bOutActive[INV_MAX_STICKER_SLOTS] )
{
	for ( int s = 0; s < INV_MAX_STICKER_SLOTS; ++s )
	{
		bOutActive[s] = false;
	}

	if ( !pVM || !pPlayer || nWep < 0 || nWep >= INV_SKIN_WEP_COUNT )
		return;

	int nTeam = pPlayer->GetTeamNumber();
	char szTexPath[MAX_PATH];
	for ( int s = 0; s < INV_MAX_STICKER_SLOTS; ++s )
	{
		const char *pszChoice = GetEquippedWeaponStickerForTeamNumber( nWep, nTeam, s );
		bOutActive[s] = Inventory_ResolveStickerTexturePath( pszChoice, szTexPath, sizeof( szTexPath ) );
	}
}
