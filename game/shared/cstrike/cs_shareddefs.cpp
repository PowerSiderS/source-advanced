//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "cs_shareddefs.h"

const float CS_PLAYER_SPEED_RUN		 = 260.0f;
const float CS_PLAYER_SPEED_VIP		 = 227.0f;
const float CS_PLAYER_SPEED_WALK	 = 100.0f;
const float CS_PLAYER_SPEED_SHIELD	 = 160.0f;
const float CS_PLAYER_SPEED_STOPPED	 =   1.0f;
const float CS_PLAYER_SPEED_OBSERVER = 900.0f;

const float CS_PLAYER_SPEED_DUCK_MODIFIER	= 0.34f;
const float CS_PLAYER_SPEED_WALK_MODIFIER	= 0.52f;
const float CS_PLAYER_SPEED_CLIMB_MODIFIER	= 0.34f;


CCSClassInfo g_ClassInfos[] =
{
	{ "None" },

	{ "Phoenix Connection" },
	{ "L337 KREW" },
	{ "Arctic Avengers" },
	{ "Guerilla Warfare" },

	{ "Seal Team 6" },
	{ "GSG-9" },
	{ "SAS" },
	{ "GIGN" }
};

const CCSClassInfo* GetCSClassInfo( int i )
{
	Assert( i >= 0 && i < ARRAYSIZE( g_ClassInfos ) );
	return &g_ClassInfos[i];
}

const char *pszWinPanelCategoryHeaders[] =
{
	"",
	"#winpanel_topdamage",
	"#winpanel_topheadshots",
	"#winpanel_kills"
};

// Construct some arrays of player model strings, so we can statically initialize CUtlVectors for general usage
const char *CTPlayerModelStrings[] =
{
	"models/player/ct_urban.mdl",
	"models/player/ct_gsg9.mdl",
	"models/player/ct_sas.mdl",
	"models/player/ct_gign.mdl",
};
const char *TerroristPlayerModelStrings[] =
{
	"models/player/t_phoenix.mdl",
	"models/player/t_leet.mdl",
	"models/player/t_arctic.mdl",
	"models/player/t_guerilla.mdl",
};
CUtlVectorInitialized< const char * > CTPlayerModels( CTPlayerModelStrings, ARRAYSIZE( CTPlayerModelStrings ) );
CUtlVectorInitialized< const char * > TerroristPlayerModels( TerroristPlayerModelStrings, ARRAYSIZE( TerroristPlayerModelStrings ) );

#define ADVANCED_ARMS_DIR "models/advanced_inventory_csgo_items/weapons/v_models/arms/"
#define ADVANCED_WEAPONS_DIR "models/advanced_inventory_csgo_items/weapons/"
#define ADVANCED_GLOVES_BAKED_DIR "advanced_inventory_csgo_items/models/weapons/customization/paints_gloves_baked/"

static const PlayerViewmodelArmConfig s_playerViewmodelArmConfigs[] =
{
	// CS:S player models (mapped to compatible VertexLitGeneric CS:GO arms & gloves)
	{ "ct_sas",						BARE_ARM_133,		ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle_black.mdl",	ADVANCED_ARMS_DIR "sas/v_sleeve_sas.mdl",						true },
	{ "ct_gsg9",					BARE_ARM_133,		ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle_blue.mdl",		ADVANCED_ARMS_DIR "gsg9/v_sleeve_gsg9.mdl",						true },
	{ "ct_gign",					BARE_ARM_133,		ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle_black.mdl",	ADVANCED_ARMS_DIR "gign/v_sleeve_gign.mdl",						true },
	{ "ct_urban",					BARE_ARM_133,		ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle.mdl",			ADVANCED_ARMS_DIR "swat/v_sleeve_swat.mdl",						true },
	{ "t_phoenix",					BARE_ARM_133,		ADVANCED_ARMS_DIR "glove_fullfinger/v_glove_fullfinger.mdl",			"",															false },
	{ "t_leet",						BARE_ARM_55,		ADVANCED_ARMS_DIR "glove_fingerless/v_glove_fingerless.mdl",			"",															false },
	{ "t_arctic",					BARE_ARM_133,		ADVANCED_ARMS_DIR "glove_fullfinger/v_glove_fullfinger.mdl",			ADVANCED_ARMS_DIR "balkan/v_sleeve_balkan.mdl",					true },
	{ "t_guerilla",					BARE_ARM_103,		ADVANCED_ARMS_DIR "glove_fingerless/v_glove_fingerless.mdl",			ADVANCED_ARMS_DIR "separatist/v_sleeve_separatist.mdl",			false },

	// CS:GO player model substrings
	{ "tm_leet",					BARE_ARM_55,		ADVANCED_ARMS_DIR "glove_fingerless/v_glove_fingerless.mdl",			"",															false },
	{ "tm_jumpsuit",				BARE_ARM_133,		ADVANCED_ARMS_DIR "glove_fingerless/v_glove_fingerless.mdl",			ADVANCED_ARMS_DIR "jumpsuit/v_sleeve_jumpsuit.mdl",				false },
	{ "tm_phoenix",					BARE_ARM_133,		ADVANCED_ARMS_DIR "glove_fullfinger/v_glove_fullfinger.mdl",			"",															false },
	{ "tm_separatist",				BARE_ARM_133,		ADVANCED_ARMS_DIR "glove_fullfinger/v_glove_fullfinger.mdl",			ADVANCED_ARMS_DIR "separatist/v_sleeve_separatist.mdl",			true },
	{ "tm_balkan",					BARE_ARM_133,		ADVANCED_ARMS_DIR "glove_fullfinger/v_glove_fullfinger.mdl",			ADVANCED_ARMS_DIR "balkan/v_sleeve_balkan.mdl",					true },
	{ "tm_professional",			BARE_ARM_133,		ADVANCED_ARMS_DIR "glove_fullfinger/v_glove_fullfinger.mdl",			ADVANCED_ARMS_DIR "professional/v_sleeve_professional.mdl",		true },
	{ "tm_anarchist",				BARE_ARM_133,		ADVANCED_ARMS_DIR "anarchist/v_glove_anarchist.mdl",					ADVANCED_ARMS_DIR "anarchist/v_sleeve_anarchist.mdl",			false },
	{ "tm_pirate",					BARE_ARM_55,		ADVANCED_ARMS_DIR "bare/v_bare_hands.mdl",								ADVANCED_ARMS_DIR "pirate/v_pirate_watch.mdl",					false },
	{ "ctm_st6",					BARE_ARM_133,		ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle.mdl",			ADVANCED_ARMS_DIR "st6/v_sleeve_st6.mdl",						true },
	{ "ctm_idf",					BARE_ARM_133,		ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle.mdl",			ADVANCED_ARMS_DIR "idf/v_sleeve_idf.mdl",						true },
	{ "ctm_gign",					BARE_ARM_133,		ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle_black.mdl",	ADVANCED_ARMS_DIR "gign/v_sleeve_gign.mdl",						true },
	{ "ctm_swat",					BARE_ARM_133,		ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle_black.mdl",	ADVANCED_ARMS_DIR "swat/v_sleeve_swat.mdl",						true },
	{ "ctm_gsg9",					BARE_ARM_133,		ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle_blue.mdl",		ADVANCED_ARMS_DIR "gsg9/v_sleeve_gsg9.mdl",						true },
	{ "ctm_sas",					BARE_ARM_133,		ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle_black.mdl",	ADVANCED_ARMS_DIR "sas/v_sleeve_sas.mdl",						true },
	{ "ctm_gendarmerie",			BARE_ARM_133,		ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle_black.mdl",	ADVANCED_ARMS_DIR "gendarmerie/v_sleeve_gendarmerie.mdl",		true },
	{ "ctm_diver",					BARE_ARM_133,		ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle_black.mdl",	ADVANCED_ARMS_DIR "diver/v_sleeve_diver_wetsuit.mdl",			false },
	{ "ctm_fbi",					BARE_ARM_133,		ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle_black.mdl",	ADVANCED_ARMS_DIR "fbi/v_sleeve_fbi.mdl",						true },
};

const PlayerViewmodelArmConfig *GetPlayerViewmodelArmConfigForPlayerModel( const char *szPlayerModel )
{
	if ( szPlayerModel != NULL && szPlayerModel[0] != '\0' )
	{
		for ( int i = 0; i < ARRAYSIZE( s_playerViewmodelArmConfigs ); ++i )
		{
			if ( V_stristr( szPlayerModel, s_playerViewmodelArmConfigs[i].szPlayerModelSearchSubStr ) )
				return &s_playerViewmodelArmConfigs[i];
		}
	}

	return &s_playerViewmodelArmConfigs[0];
}

int GetViewmodelArmConfigCount( void )
{
	return ARRAYSIZE( s_playerViewmodelArmConfigs );
}

const PlayerViewmodelArmConfig *GetViewmodelArmConfig( int i )
{
	if ( i < 0 || i >= ARRAYSIZE( s_playerViewmodelArmConfigs ) )
		return &s_playerViewmodelArmConfigs[0];
	return &s_playerViewmodelArmConfigs[i];
}

static bool TryParseStrictInt( const char *pszInput, int &nOut )
{
	if ( !pszInput || !pszInput[0] )
		return false;

	const char *p = pszInput;
	if ( *p == '+' || *p == '-' )
		++p;
	if ( !*p )
		return false;

	for ( ; *p; ++p )
	{
		if ( *p < '0' || *p > '9' )
			return false;
	}

	nOut = atoi( pszInput );
	return true;
}

struct ArmGloveSkinEntry
{
	const char *szAlias;
	const char *szBakedTexturePath;
};

static const ArmGloveSkinEntry s_bloodhoundSkins[] =
{
	{ "black_silver",	ADVANCED_GLOVES_BAKED_DIR "bloodhound_black_silver" },
	{ "snakeskin_brass",ADVANCED_GLOVES_BAKED_DIR "bloodhound_snakeskin_brass" },
	{ "metallic",		ADVANCED_GLOVES_BAKED_DIR "bloodhound_metallic" },
	{ "guerrilla",		ADVANCED_GLOVES_BAKED_DIR "bloodhound_guerrilla" },
};

static const ArmGloveSkinEntry s_sportySkins[] =
{
	{ "lightblue",					ADVANCED_GLOVES_BAKED_DIR "sporty_lightblue" },
	{ "military",					ADVANCED_GLOVES_BAKED_DIR "sporty_military" },
	{ "purple",						ADVANCED_GLOVES_BAKED_DIR "sporty_purple" },
	{ "green",						ADVANCED_GLOVES_BAKED_DIR "sporty_green" },
	{ "poison_frog_blue_white",		ADVANCED_GLOVES_BAKED_DIR "sporty_poison_frog_blue_white" },
	{ "poison_frog_red_green",		ADVANCED_GLOVES_BAKED_DIR "sporty_poison_frog_red_green" },
	{ "black_webbing_yellow",		ADVANCED_GLOVES_BAKED_DIR "sporty_black_webbing_yellow" },
	{ "blue_pink",					ADVANCED_GLOVES_BAKED_DIR "sporty_blue_pink" },
	{ "slingshot",					ADVANCED_GLOVES_BAKED_DIR "sporty_slingshot" },
	{ "hunter",						ADVANCED_GLOVES_BAKED_DIR "sporty_hunter" },
	{ "houndstooth_red",			ADVANCED_GLOVES_BAKED_DIR "sporty_houndstooth_red" },
	{ "jaguar",						ADVANCED_GLOVES_BAKED_DIR "sporty_jaguar" },
};

static const ArmGloveSkinEntry s_slickSkins[] =
{
	{ "black",						ADVANCED_GLOVES_BAKED_DIR "slick_black" },
	{ "military",					ADVANCED_GLOVES_BAKED_DIR "slick_military" },
	{ "red",						ADVANCED_GLOVES_BAKED_DIR "slick_red" },
	{ "snakeskin_yellow",			ADVANCED_GLOVES_BAKED_DIR "slick_snakeskin_yellow" },
	{ "snakeskin_white",			ADVANCED_GLOVES_BAKED_DIR "slick_snakeskin_white" },
	{ "plaid_purple",				ADVANCED_GLOVES_BAKED_DIR "slick_plaid_purple" },
	{ "stitched_black_orange",		ADVANCED_GLOVES_BAKED_DIR "slick_stitched_black_orange" },
	{ "stitched_green_grey",		ADVANCED_GLOVES_BAKED_DIR "slick_stitched_green_grey" },
	{ "rezan",						ADVANCED_GLOVES_BAKED_DIR "slick_rezan" },
	{ "jaguar_white",				ADVANCED_GLOVES_BAKED_DIR "slick_jaguar_white" },
	{ "jaguar_yellow",				ADVANCED_GLOVES_BAKED_DIR "slick_jaguar_yellow" },
	{ "stitched_black_white",		ADVANCED_GLOVES_BAKED_DIR "slick_stitched_black_white" },
	{ "bamboo",						ADVANCED_GLOVES_BAKED_DIR "slick_bamboo" },
	{ "chainmail_gold",				ADVANCED_GLOVES_BAKED_DIR "slick_chainmail_gold" },
	{ "chainmail_knight",			ADVANCED_GLOVES_BAKED_DIR "slick_chainmail_knight" },
	{ "snakeskin_green",			ADVANCED_GLOVES_BAKED_DIR "slick_snakeskin_green" },
	{ "stitched_black",				ADVANCED_GLOVES_BAKED_DIR "slick_stitched_black" },
};

static const ArmGloveSkinEntry s_handwrapSkins[] =
{
	{ "brown_beige",				ADVANCED_GLOVES_BAKED_DIR "handwrap_leathery_brown_beige" },
	{ "grey_camo_blob",				ADVANCED_GLOVES_BAKED_DIR "handwrap_leathery_fabric_grey_camo_blob" },
	{ "red_slaughter",				ADVANCED_GLOVES_BAKED_DIR "handwrap_leathery_fabric_red_slaughter" },
	{ "orange_camo",				ADVANCED_GLOVES_BAKED_DIR "handwrap_leathery_fabric_orange_camo" },
	{ "blue_skulls",				ADVANCED_GLOVES_BAKED_DIR "handwrap_leathery_fabric_blue_skulls" },
	{ "geometric_blue",				ADVANCED_GLOVES_BAKED_DIR "handwrap_leathery_fabric_geometric_blue" },
	{ "ducttape",					ADVANCED_GLOVES_BAKED_DIR "handwrap_leathery_ducttape" },
	{ "green_camo",					ADVANCED_GLOVES_BAKED_DIR "handwrap_leathery_fabric_green_camo" },
	{ "houndstooth_orange",			ADVANCED_GLOVES_BAKED_DIR "handwrap_fabric_houndstooth_orange" },
	{ "giraffe",					ADVANCED_GLOVES_BAKED_DIR "handwrap_leathery_fabric_giraffe" },
	{ "snakeskin_orange",			ADVANCED_GLOVES_BAKED_DIR "handwrap_leathery_snakeskin_orange" },
	{ "caution",					ADVANCED_GLOVES_BAKED_DIR "handwrap_leathery_caution" },
	{ "catskulls_bw",				ADVANCED_GLOVES_BAKED_DIR "handwrap_leathery_catskulls_bw" },
	{ "beige_camo",					ADVANCED_GLOVES_BAKED_DIR "handwrap_leathery_fabric_beige_camo" },
	{ "nuclear",					ADVANCED_GLOVES_BAKED_DIR "handwrap_leathery_fabric_nuclear" },
};

static const ArmGloveSkinEntry s_motoSkins[] =
{
	{ "basic_black",				ADVANCED_GLOVES_BAKED_DIR "motorcycle_basic_black" },
	{ "mint_triangle",				ADVANCED_GLOVES_BAKED_DIR "motorcycle_mint_triangle" },
	{ "mono_boom",					ADVANCED_GLOVES_BAKED_DIR "motorcycle_mono_boom" },
	{ "triangle_blue",				ADVANCED_GLOVES_BAKED_DIR "motorcycle_triangle_blue" },
	{ "choco_boom",					ADVANCED_GLOVES_BAKED_DIR "motorcycle_choco_boom" },
	{ "basic_green_orange",			ADVANCED_GLOVES_BAKED_DIR "motorcycle_basic_green_orange" },
	{ "yellow_camo",				ADVANCED_GLOVES_BAKED_DIR "motorcycle_yellow_camo" },
	{ "trigrid_blue",				ADVANCED_GLOVES_BAKED_DIR "motorcycle_trigrid_blue" },
	{ "checker_flag_blue_green",	ADVANCED_GLOVES_BAKED_DIR "motorcycle_checker_flag_blue_green" },
	{ "smoke",						ADVANCED_GLOVES_BAKED_DIR "motorcycle_smoke" },
	{ "carbonfiber_red",			ADVANCED_GLOVES_BAKED_DIR "motorcycle_carbonfiber_red" },
	{ "commando_ksk",				ADVANCED_GLOVES_BAKED_DIR "motorcycle_commando_ksk" },
	{ "doppler_phase2",				ADVANCED_GLOVES_BAKED_DIR "motorcycle_doppler_phase2" },
	{ "checker_flag_green",			ADVANCED_GLOVES_BAKED_DIR "motorcycle_checker_flag_green" },
	{ "green_circuitboard",			ADVANCED_GLOVES_BAKED_DIR "motorcycle_green_circuitboard" },
};

static const ArmGloveSkinEntry s_specialistSkins[] =
{
	{ "ddpat_dense_green_brown",		ADVANCED_GLOVES_BAKED_DIR "specialist_ddpat_dense_green_brown" },
	{ "kimono_diamonds_black_red",		ADVANCED_GLOVES_BAKED_DIR "specialist_kimono_diamonds_black_red" },
	{ "webs_green",						ADVANCED_GLOVES_BAKED_DIR "specialist_webs_green" },
	{ "white_orange_grey",				ADVANCED_GLOVES_BAKED_DIR "specialist_white_orange_grey" },
	{ "webs_red",						ADVANCED_GLOVES_BAKED_DIR "specialist_webs_red" },
	{ "forest_brown",					ADVANCED_GLOVES_BAKED_DIR "specialist_forest_brown" },
	{ "fade",							ADVANCED_GLOVES_BAKED_DIR "specialist_fade" },
	{ "winterhex",						ADVANCED_GLOVES_BAKED_DIR "specialist_winterhex" },
	{ "marble_fade",					ADVANCED_GLOVES_BAKED_DIR "specialist_marble_fade" },
	{ "ricksaw_camo",					ADVANCED_GLOVES_BAKED_DIR "specialist_ricksaw_camo" },
	{ "tiger_orange",					ADVANCED_GLOVES_BAKED_DIR "specialist_tiger_orange" },
	{ "fbi",							ADVANCED_GLOVES_BAKED_DIR "specialist_fbi" },
};

static const ArmGloveSkinEntry s_hydraSkins[] =
{
	{ "black_green",					ADVANCED_GLOVES_BAKED_DIR "bloodhound_hydra_black_green" },
	{ "green_leather_mesh_brass",		ADVANCED_GLOVES_BAKED_DIR "bloodhound_hydra_green_leather_mesh_brass" },
	{ "snakeskin_brass",				ADVANCED_GLOVES_BAKED_DIR "bloodhound_hydra_snakeskin_brass" },
	{ "case_hardened",					ADVANCED_GLOVES_BAKED_DIR "bloodhound_hydra_case_hardened" },
	{ "poison_frog_red_green",			ADVANCED_GLOVES_BAKED_DIR "operation10_poison_frog_red_green" },
	{ "jaguar_purple",					ADVANCED_GLOVES_BAKED_DIR "operation10_jaguar_purple" },
	{ "snakeskin_black",				ADVANCED_GLOVES_BAKED_DIR "operation_10_snakeskin_black" },
	{ "metalic_green",					ADVANCED_GLOVES_BAKED_DIR "operation_10_metalic_green" },
};

static const ArmGloveSkinEntry s_brokenfangSkins[] =
{
	{ "metalic_green",					ADVANCED_GLOVES_BAKED_DIR "operation10_metalic_green" },
	{ "poison_frog_black_yellow",		ADVANCED_GLOVES_BAKED_DIR "operation10_poison_frog_black_yellow" },
	{ "floral",							ADVANCED_GLOVES_BAKED_DIR "operation10_floral" },
	{ "snakeskin_black",				ADVANCED_GLOVES_BAKED_DIR "operation_10_snakeskin_black" },
};

struct ArmGloveFamily
{
	const char *szFamilyKey;
	const char *szAltAlias;
	const char *szModelPath;
	const ArmGloveSkinEntry *pSkins;
	int nSkinCount;
};

static const ArmGloveFamily s_gloveFamilies[] =
{
	{ "bare",			"bare_hands",	ADVANCED_ARMS_DIR "bare/v_bare_hands.mdl",									NULL,				0 }, // 0
	{ "hardknuckle",	NULL,			ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle.mdl",				NULL,				0 }, // 1 (0=default, 1=blue, 2=black)
	{ "fullfinger",		NULL,			ADVANCED_ARMS_DIR "glove_fullfinger/v_glove_fullfinger.mdl",				NULL,				0 }, // 2
	{ "fingerless",		NULL,			ADVANCED_ARMS_DIR "glove_fingerless/v_glove_fingerless.mdl",				NULL,				0 }, // 3
	{ "anarchist",		NULL,			ADVANCED_ARMS_DIR "anarchist/v_glove_anarchist.mdl",						NULL,				0 }, // 4
	{ "bloodhound",		NULL,			ADVANCED_ARMS_DIR "glove_bloodhound/v_glove_bloodhound.mdl",				s_bloodhoundSkins,	ARRAYSIZE( s_bloodhoundSkins ) }, // 5
	{ "sporty",			NULL,			ADVANCED_ARMS_DIR "glove_sporty/v_glove_sporty.mdl",						s_sportySkins,		ARRAYSIZE( s_sportySkins ) }, // 6
	{ "slick",			"driver",		ADVANCED_ARMS_DIR "glove_slick/v_glove_slick.mdl",							s_slickSkins,		ARRAYSIZE( s_slickSkins ) }, // 7
	{ "handwrap",		NULL,			ADVANCED_ARMS_DIR "glove_handwrap_leathery/v_glove_handwrap_leathery.mdl",	s_handwrapSkins,	ARRAYSIZE( s_handwrapSkins ) }, // 8
	{ "moto",			"motorcycle",	ADVANCED_ARMS_DIR "glove_motorcycle/v_glove_motorcycle.mdl",				s_motoSkins,		ARRAYSIZE( s_motoSkins ) }, // 9
	{ "specialist",		NULL,			ADVANCED_ARMS_DIR "glove_specialist/v_glove_specialist.mdl",				s_specialistSkins,	ARRAYSIZE( s_specialistSkins ) }, // 10
	{ "hydra",			NULL,			ADVANCED_ARMS_DIR "glove_bloodhound/v_glove_bloodhound_hydra.mdl",			s_hydraSkins,		ARRAYSIZE( s_hydraSkins ) }, // 11
	{ "brokenfang",		NULL,			ADVANCED_ARMS_DIR "glove_bloodhound/v_glove_bloodhound_brokenfang.mdl",		s_brokenfangSkins,	ARRAYSIZE( s_brokenfangSkins ) }, // 12
};

struct ArmModelAlias
{
	const char *szAlias;
	const char *szModelPath;
};

static const ArmModelAlias s_extraGloveAliases[] =
{
	{ "hardknuckle_blue",	ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle_blue.mdl" },
	{ "hardknuckle_black",	ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle_black.mdl" },
};

// 50 sleeves in exact ClientMod items_clientmod.txt order (1..50)
static const ArmModelAlias s_numberedSleeves[] =
{
	{ "jumpsuit",					ADVANCED_ARMS_DIR "jumpsuit/v_sleeve_jumpsuit.mdl" },						// 1
	{ "separatist",					ADVANCED_ARMS_DIR "separatist/v_sleeve_separatist.mdl" },					// 2
	{ "balkan_v2_variantf",			ADVANCED_ARMS_DIR "balkan/v_sleeve_balkan_v2_variantf.mdl" },				// 3
	{ "balkan_v2_variantg",			ADVANCED_ARMS_DIR "balkan/v_sleeve_balkan_v2_variantg.mdl" },				// 4
	{ "balkan_v2_varianth",			ADVANCED_ARMS_DIR "balkan/v_sleeve_balkan_v2_varianth.mdl" },				// 5
	{ "balkan_v2_variantj",			ADVANCED_ARMS_DIR "balkan/v_sleeve_balkan_v2_variantj.mdl" },				// 6
	{ "balkan_v2_variantk",			ADVANCED_ARMS_DIR "balkan/v_sleeve_balkan_v2_variantk.mdl" },				// 7
	{ "balkan_v2_variantl",			ADVANCED_ARMS_DIR "balkan/v_sleeve_balkan_v2_variantl.mdl" },				// 8
	{ "balkan",						ADVANCED_ARMS_DIR "balkan/v_sleeve_balkan.mdl" },							// 9
	{ "professional",				ADVANCED_ARMS_DIR "professional/v_sleeve_professional.mdl" },				// 10
	{ "professional_watch_silver",	ADVANCED_ARMS_DIR "professional/v_professional_watch_silver.mdl" },			// 11
	{ "professional_watch",			ADVANCED_ARMS_DIR "professional/v_professional_watch.mdl" },				// 12
	{ "pirate_watch",				ADVANCED_ARMS_DIR "pirate/v_pirate_watch.mdl" },							// 13
	{ "st6_v2_variante",			ADVANCED_ARMS_DIR "st6/v_sleeve_st6_v2_variante.mdl" },						// 14
	{ "green",						ADVANCED_ARMS_DIR "st6/v_sleeve_green.mdl" },								// 15
	{ "st6_v2_variantg",			ADVANCED_ARMS_DIR "st6/v_sleeve_st6_v2_variantg.mdl" },						// 16
	{ "st6_v2_variantj",			ADVANCED_ARMS_DIR "st6/v_sleeve_st6_v2_variantj.mdl" },						// 17
	{ "st6_v2_variantk",			ADVANCED_ARMS_DIR "st6/v_sleeve_st6_v2_variantk.mdl" },						// 18
	{ "st6_v2_variantl",			ADVANCED_ARMS_DIR "st6/v_sleeve_st6_v2_variantl.mdl" },						// 19
	{ "st6_v2_variantm",			ADVANCED_ARMS_DIR "st6/v_sleeve_st6_v2_variantm.mdl" },						// 20
	{ "st6_v2_variantn",			ADVANCED_ARMS_DIR "st6/v_sleeve_st6_v2_variantn.mdl" },						// 21
	{ "st6",						ADVANCED_ARMS_DIR "st6/v_sleeve_st6.mdl" },									// 22
	{ "idf",						ADVANCED_ARMS_DIR "idf/v_sleeve_idf.mdl" },									// 23
	{ "gign",						ADVANCED_ARMS_DIR "gign/v_sleeve_gign.mdl" },								// 24
	{ "swat",						ADVANCED_ARMS_DIR "swat/v_sleeve_swat.mdl" },								// 25
	{ "swat_leader",				ADVANCED_ARMS_DIR "swat/v_sleeve_swat_leader.mdl" },						// 26
	{ "swat_breecher",				ADVANCED_ARMS_DIR "swat/v_sleeve_swat_breecher.mdl" },						// 27
	{ "swat_medic",					ADVANCED_ARMS_DIR "swat/v_sleeve_swat_medic.mdl" },							// 28
	{ "swat_gasmask_green",			ADVANCED_ARMS_DIR "swat/v_sleeve_swat_gasmask_green.mdl" },					// 29
	{ "swat_generic",				ADVANCED_ARMS_DIR "swat/v_sleeve_swat_generic.mdl" },						// 30
	{ "swat_gasmask_blue",			ADVANCED_ARMS_DIR "swat/v_sleeve_swat_gasmask_blue.mdl" },					// 31
	{ "swat_breecher_variantk",		ADVANCED_ARMS_DIR "swat/v_sleeve_swat_breecher_variantk.mdl" },				// 32
	{ "gsg9",						ADVANCED_ARMS_DIR "gsg9/v_sleeve_gsg9.mdl" },								// 33
	{ "sas_ukmtp",					ADVANCED_ARMS_DIR "sas/v_sleeve_sas_ukmtp.mdl" },							// 34
	{ "sas_variantg",				ADVANCED_ARMS_DIR "sas/v_sleeve_sas_variantg.mdl" },						// 35
	{ "sas",						ADVANCED_ARMS_DIR "sas/v_sleeve_sas.mdl" },									// 36
	{ "gendarmerie_variantc",		ADVANCED_ARMS_DIR "gendarmerie/v_sleeve_gendarmerie_variantc.mdl" },		// 37
	{ "gendarmerie",				ADVANCED_ARMS_DIR "gendarmerie/v_sleeve_gendarmerie.mdl" },					// 38
	{ "fbi_dark",					ADVANCED_ARMS_DIR "fbi/v_sleeve_fbi_dark.mdl" },							// 39
	{ "fbi",						ADVANCED_ARMS_DIR "fbi/v_sleeve_fbi.mdl" },									// 40
	{ "fbi_green",					ADVANCED_ARMS_DIR "fbi/v_sleeve_fbi_green.mdl" },							// 41
	{ "fbi_light_green",			ADVANCED_ARMS_DIR "fbi/v_sleeve_fbi_light_green.mdl" },						// 42
	{ "diver_wetsuit",				ADVANCED_ARMS_DIR "diver/v_sleeve_diver_wetsuit.mdl" },						// 43
	{ "fbi_gray",					ADVANCED_ARMS_DIR "fbi/v_sleeve_fbi_gray.mdl" },							// 44
	{ "flektarn",					ADVANCED_ARMS_DIR "st6/v_sleeve_flektarn.mdl" },							// 45
	{ "usaf",						ADVANCED_ARMS_DIR "st6/v_sleeve_usaf.mdl" },								// 46
	{ "swat_blue",					ADVANCED_ARMS_DIR "swat/v_sleeve_swat_blue.mdl" },							// 47
	{ "swat_green",					ADVANCED_ARMS_DIR "swat/v_sleeve_swat_green.mdl" },							// 48
	{ "anarchist",					ADVANCED_ARMS_DIR "anarchist/v_sleeve_anarchist.mdl" },						// 49
	{ "wristband",					ADVANCED_ARMS_DIR "wristband/v_sleeve_wristband.mdl" },						// 50
};

static const ArmModelAlias s_extraSleeveAliases[] =
{
	{ "st6_green",		ADVANCED_ARMS_DIR "st6/v_sleeve_green.mdl" },
	{ "st6_flektarn",	ADVANCED_ARMS_DIR "st6/v_sleeve_flektarn.mdl" },
	{ "st6_usaf",		ADVANCED_ARMS_DIR "st6/v_sleeve_usaf.mdl" },
	{ "diver",			ADVANCED_ARMS_DIR "diver/v_sleeve_diver_wetsuit.mdl" },
};

static const ArmGloveFamily *FindGloveFamily( const char *pszGloveInput )
{
	if ( !pszGloveInput || !pszGloveInput[0] )
		return NULL;

	int nIndex = -1;
	if ( TryParseStrictInt( pszGloveInput, nIndex ) )
	{
		if ( nIndex >= 0 && nIndex < ARRAYSIZE( s_gloveFamilies ) )
			return &s_gloveFamilies[nIndex];
		return NULL;
	}

	if ( !V_stricmp( pszGloveInput, "none" ) || !V_stricmp( pszGloveInput, "off" ) )
		return &s_gloveFamilies[0];

	for ( int i = 0; i < ARRAYSIZE( s_gloveFamilies ); ++i )
	{
		if ( !V_stricmp( pszGloveInput, s_gloveFamilies[i].szFamilyKey ) ||
			( s_gloveFamilies[i].szAltAlias && !V_stricmp( pszGloveInput, s_gloveFamilies[i].szAltAlias ) ) )
			return &s_gloveFamilies[i];
	}

	return NULL;
}

static const char *ResolveBakedSkinTexture( const ArmGloveFamily &family, const char *pszSkinStr, int nSkinInt )
{
	if ( !family.pSkins || family.nSkinCount <= 0 )
		return NULL;

	if ( pszSkinStr && pszSkinStr[0] )
	{
		int nParsed = -1;
		if ( TryParseStrictInt( pszSkinStr, nParsed ) )
		{
			if ( nParsed >= 1 && nParsed <= family.nSkinCount )
				return family.pSkins[nParsed - 1].szBakedTexturePath;
			return NULL;
		}

		for ( int i = 0; i < family.nSkinCount; ++i )
		{
			if ( !V_stricmp( pszSkinStr, family.pSkins[i].szAlias ) )
				return family.pSkins[i].szBakedTexturePath;

			const char *pszStem = V_GetFileName( family.pSkins[i].szBakedTexturePath );
			if ( pszStem && !V_stricmp( pszSkinStr, pszStem ) )
				return family.pSkins[i].szBakedTexturePath;
		}
		return NULL;
	}

	if ( nSkinInt >= 1 && nSkinInt <= family.nSkinCount )
		return family.pSkins[nSkinInt - 1].szBakedTexturePath;

	return NULL;
}

bool ResolveViewmodelGloveSelection( const char *pszGloveInput, FnGetGloveSkinValue pfnGetSkin, ArmGloveSelection &outSelection )
{
	outSelection.pszModelPath = NULL;
	outSelection.pszOverrideBaseTexture = NULL;

	if ( !pszGloveInput || !pszGloveInput[0] )
		return false;

	if ( V_stristr( pszGloveInput, ".mdl" ) )
	{
		static char s_szDirectGlovePath[MAX_PATH];
		V_strncpy( s_szDirectGlovePath, pszGloveInput, sizeof( s_szDirectGlovePath ) );
		outSelection.pszModelPath = s_szDirectGlovePath;
		return true;
	}

	const ArmGloveFamily *pFamily = FindGloveFamily( pszGloveInput );
	if ( pFamily )
	{
		outSelection.pszModelPath = pFamily->szModelPath;

		char szSkinStr[64] = { 0 };
		int nSkinInt = pfnGetSkin ? pfnGetSkin( pFamily->szFamilyKey, szSkinStr, sizeof( szSkinStr ) ) : 0;

		if ( !V_stricmp( pFamily->szFamilyKey, "hardknuckle" ) )
		{
			if ( nSkinInt == 1 || !V_stricmp( szSkinStr, "blue" ) )
				outSelection.pszModelPath = ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle_blue.mdl";
			else if ( nSkinInt == 2 || !V_stricmp( szSkinStr, "black" ) )
				outSelection.pszModelPath = ADVANCED_ARMS_DIR "glove_hardknuckle/v_glove_hardknuckle_black.mdl";
		}
		else if ( pFamily->nSkinCount > 0 )
		{
			outSelection.pszOverrideBaseTexture = ResolveBakedSkinTexture( *pFamily, szSkinStr, nSkinInt );
		}

		return true;
	}

	for ( int i = 0; i < ARRAYSIZE( s_extraGloveAliases ); ++i )
	{
		if ( !V_stricmp( pszGloveInput, s_extraGloveAliases[i].szAlias ) )
		{
			outSelection.pszModelPath = s_extraGloveAliases[i].szModelPath;
			return true;
		}
	}

	return false;
}

const char *ResolveViewmodelGloveModelPath( const char *pszInput, char *pszOut, int nOutSize )
{
	if ( !pszInput || !pszInput[0] )
		return NULL;

	if ( V_stristr( pszInput, ".mdl" ) )
	{
		V_strncpy( pszOut, pszInput, nOutSize );
		return pszOut;
	}

	ArmGloveSelection selection;
	if ( ResolveViewmodelGloveSelection( pszInput, NULL, selection ) && selection.pszModelPath )
	{
		V_strncpy( pszOut, selection.pszModelPath, nOutSize );
		return pszOut;
	}

	return NULL;
}

const char *ResolveViewmodelSleeveModelPath( const char *pszInput, char *pszOut, int nOutSize )
{
	if ( !pszInput || !pszInput[0] )
		return NULL;

	if ( V_stristr( pszInput, ".mdl" ) )
	{
		V_strncpy( pszOut, pszInput, nOutSize );
		return pszOut;
	}

	int nIndex = -1;
	if ( TryParseStrictInt( pszInput, nIndex ) )
	{
		if ( nIndex >= 1 && nIndex <= ARRAYSIZE( s_numberedSleeves ) )
		{
			V_strncpy( pszOut, s_numberedSleeves[nIndex - 1].szModelPath, nOutSize );
			return pszOut;
		}
		return NULL;
	}

	for ( int i = 0; i < ARRAYSIZE( s_numberedSleeves ); ++i )
	{
		if ( !V_stricmp( pszInput, s_numberedSleeves[i].szAlias ) )
		{
			V_strncpy( pszOut, s_numberedSleeves[i].szModelPath, nOutSize );
			return pszOut;
		}
	}

	for ( int i = 0; i < ARRAYSIZE( s_extraSleeveAliases ); ++i )
	{
		if ( !V_stricmp( pszInput, s_extraSleeveAliases[i].szAlias ) )
		{
			V_strncpy( pszOut, s_extraSleeveAliases[i].szModelPath, nOutSize );
			return pszOut;
		}
	}

	return NULL;
}

struct KnifeModelEntry
{
	const char *szPrimaryAlias;
	const char *szAltAlias;
	const char *szModelPath;
};

static const char *s_szKnifeDefaultT = ADVANCED_WEAPONS_DIR "v_knife_default_t.mdl";
static const char *s_szKnifeDefaultCT = ADVANCED_WEAPONS_DIR "v_knife_default_ct.mdl";

// 19 knives in exact ClientMod items_clientmod.txt order (1..19)
static const KnifeModelEntry s_knifeEntries[] =
{
	{ "bayonet",			NULL,				ADVANCED_WEAPONS_DIR "v_knife_bayonet.mdl" },			// 1
	{ "css",				"classic",			ADVANCED_WEAPONS_DIR "v_knife_css.mdl" },				// 2
	{ "flip",				NULL,				ADVANCED_WEAPONS_DIR "v_knife_flip.mdl" },				// 3
	{ "gut",				NULL,				ADVANCED_WEAPONS_DIR "v_knife_gut.mdl" },				// 4
	{ "karambit",			"karam",			ADVANCED_WEAPONS_DIR "v_knife_karam.mdl" },				// 5
	{ "m9_bayonet",			"m9",				ADVANCED_WEAPONS_DIR "v_knife_m9_bay.mdl" },			// 6
	{ "tactical",			"huntsman",			ADVANCED_WEAPONS_DIR "v_knife_tactical.mdl" },			// 7
	{ "falchion",			NULL,				ADVANCED_WEAPONS_DIR "v_knife_falchion_advanced.mdl" },	// 8
	{ "survival_bowie",		"bowie",			ADVANCED_WEAPONS_DIR "v_knife_survival_bowie.mdl" },	// 9
	{ "butterfly",			NULL,				ADVANCED_WEAPONS_DIR "v_knife_butterfly.mdl" },			// 10
	{ "push",				"daggers",			ADVANCED_WEAPONS_DIR "v_knife_push.mdl" },				// 11
	{ "cord",				"paracord",			ADVANCED_WEAPONS_DIR "v_knife_cord.mdl" },				// 12
	{ "canis",				"survival",			ADVANCED_WEAPONS_DIR "v_knife_canis.mdl" },				// 13
	{ "ursus",				NULL,				ADVANCED_WEAPONS_DIR "v_knife_ursus.mdl" },				// 14
	{ "gypsy_jackknife",	"navaja",			ADVANCED_WEAPONS_DIR "v_knife_gypsy_jackknife.mdl" },	// 15
	{ "outdoor",			"nomad",			ADVANCED_WEAPONS_DIR "v_knife_outdoor.mdl" },			// 16
	{ "stiletto",			NULL,				ADVANCED_WEAPONS_DIR "v_knife_stiletto.mdl" },			// 17
	{ "widowmaker",			"talon",			ADVANCED_WEAPONS_DIR "v_knife_widowmaker.mdl" },		// 18
	{ "skeleton",			NULL,				ADVANCED_WEAPONS_DIR "v_knife_skeleton.mdl" },			// 19
};

const char *GetKnifeViewModelForSelection( int nTeam, const char *pszKnifeInput )
{
	const char *pszTeamDefault = ( nTeam == TEAM_CT ) ? s_szKnifeDefaultCT : s_szKnifeDefaultT;

	if ( !pszKnifeInput || !pszKnifeInput[0] )
		return pszTeamDefault;

	int nIndex = -1;
	if ( TryParseStrictInt( pszKnifeInput, nIndex ) )
	{
		if ( nIndex >= 1 && nIndex <= ARRAYSIZE( s_knifeEntries ) )
			return s_knifeEntries[nIndex - 1].szModelPath;
		return pszTeamDefault;
	}

	if ( !V_stricmp( pszKnifeInput, "default" ) || !V_stricmp( pszKnifeInput, "none" ) )
		return pszTeamDefault;

	if ( !V_stricmp( pszKnifeInput, "default_t" ) )
		return s_szKnifeDefaultT;
	if ( !V_stricmp( pszKnifeInput, "default_ct" ) )
		return s_szKnifeDefaultCT;

	for ( int i = 0; i < ARRAYSIZE( s_knifeEntries ); ++i )
	{
		if ( !V_stricmp( pszKnifeInput, s_knifeEntries[i].szPrimaryAlias ) ||
			( s_knifeEntries[i].szAltAlias && !V_stricmp( pszKnifeInput, s_knifeEntries[i].szAltAlias ) ) )
			return s_knifeEntries[i].szModelPath;
	}

	return pszTeamDefault;
}

int GetKnifeViewModelCount( void )
{
	return 2 + ARRAYSIZE( s_knifeEntries );
}

const char *GetKnifeViewModelByIndex( int i )
{
	if ( i == 0 )
		return s_szKnifeDefaultT;
	if ( i == 1 )
		return s_szKnifeDefaultCT;

	int nCustomIdx = i - 2;
	if ( nCustomIdx >= 0 && nCustomIdx < ARRAYSIZE( s_knifeEntries ) )
		return s_knifeEntries[nCustomIdx].szModelPath;

	return s_szKnifeDefaultT;
}
