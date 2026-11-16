//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: CS:GO VGUI2 Weapons, Knives, Gloves, Sleeves & Stickers Inventory
//
//=============================================================================//

#include "cbase.h"
#include "cstrikeinventory.h"
#include "cs_shareddefs.h"
#include "c_cs_player.h"
#include "filesystem.h"
#include "ienginevgui.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/MaterialSystemUtil.h"
#include "vgui/ISurface.h"
#include "vgui/IInput.h"
#include "vgui/IVGui.h"
#include "vgui/IScheme.h"
#include "vgui/ILocalize.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/TextEntry.h"
#include "KeyValues.h"
#include <ctype.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

namespace
{
	struct InventoryLayoutMetrics_t
	{
		int margin, gap, pad;
		int winX, winY, winW, winH;
		int topH, bodyY, bodyH;
		int leftX, leftW, centerX, centerW, rightX, rightW;
	};

	static void ComputeLayoutMetrics( int sw, int sh, InventoryLayoutMetrics_t &m )
	{
		m.margin = clamp( (int)( sh * 0.022f ), 6, 28 );
		m.gap    = clamp( (int)( sh * 0.012f ), 4, 14 );
		m.winX   = m.margin;
		m.winY   = m.margin;
		m.winW   = Max( 320, sw - m.margin * 2 );
		m.winH   = Max( 240, sh - m.margin * 2 );
		m.pad    = clamp( (int)( m.winW * 0.010f ), 6, 14 );
		m.topH   = clamp( (int)( m.winH * 0.090f ), 38, 64 );
		m.bodyY  = m.winY + m.topH + m.gap;
		m.bodyH  = Max( 160, m.winH - m.topH - m.gap - m.pad );

		int innerW = m.winW - m.pad * 2;
		int availColsW = Max( 300, innerW - m.gap * 2 );
		m.leftW  = clamp( (int)( availColsW * 0.19f ), 130, 340 );
		m.rightW = clamp( (int)( availColsW * 0.34f ), 205, 620 );
		m.centerW = availColsW - m.leftW - m.rightW;
		if ( m.centerW < 230 )
		{
			int deficit = 230 - m.centerW;
			int takeR = Min( deficit, Max( 0, m.rightW - 185 ) );
			m.rightW -= takeR;
			deficit -= takeR;
			int takeL = Min( deficit, Max( 0, m.leftW - 120 ) );
			m.leftW -= takeL;
			m.centerW = Max( 160, availColsW - m.leftW - m.rightW );
		}

		m.leftX   = m.winX + m.pad;
		m.centerX = m.leftX + m.leftW + m.gap;
		m.rightX  = m.centerX + m.centerW + m.gap;
	}

	enum
	{
		INV_GRID_COLS = 3,
		INV_GRID_ROWS = 4,
		INV_GRID_PAGE_SIZE = INV_GRID_COLS * INV_GRID_ROWS,
		INV_MAX_WEAPON_BUTTONS = 13,
	};

	struct InventoryPalette_t
	{
		Color clrBackdrop;
		Color clrWindowBg;
		Color clrTopBarBg;
		Color clrPanelBg;
		Color clrPanelBorder;
		Color clrWindowBorder;
		Color clrAccent;
		Color clrAccentCT;
		Color clrAccentT;
		Color clrPreviewBg;
		Color clrTileBg;
		Color clrTileHoverBg;
		Color clrTileSelectedBg;
		Color clrTileArtBg;
		Color clrTileBorder;
		Color clrTileSelectedText;
		Color clrTextPrimary;
		Color clrTextSecondary;
		Color clrTextMuted;
		Color clrButtonBg;
		Color clrButtonHoverBg;
		Color clrButtonActiveBg;
		Color clrButtonCTBg;
		Color clrButtonCTHoverBg;
		Color clrButtonTBg;
		Color clrButtonTHoverBg;
		Color clrButtonBothBg;
		Color clrButtonBothHoverBg;
		Color clrDangerBg;
		Color clrDangerHoverBg;
		Color clrDangerAccent;

		void LoadDefaults()
		{
			clrBackdrop         = Color( 8, 10, 14, 215 );
			clrWindowBg         = Color( 18, 22, 30, 250 );
			clrTopBarBg         = Color( 24, 30, 41, 255 );
			clrPanelBg          = Color( 22, 27, 36, 255 );
			clrPanelBorder      = Color( 45, 55, 72, 255 );
			clrWindowBorder     = Color( 62, 74, 96, 255 );
			clrAccent           = Color( 229, 169, 59, 255 );
			clrAccentCT         = Color( 70, 145, 225, 255 );
			clrAccentT          = Color( 217, 130, 43, 255 );
			clrPreviewBg        = Color( 20, 25, 34, 255 );
			clrTileBg           = Color( 28, 34, 46, 255 );
			clrTileHoverBg      = Color( 36, 44, 58, 255 );
			clrTileSelectedBg   = Color( 40, 50, 68, 255 );
			clrTileArtBg        = Color( 20, 25, 34, 255 );
			clrTileBorder       = Color( 48, 58, 76, 255 );
			clrTileSelectedText = Color( 255, 224, 145, 255 );
			clrTextPrimary      = Color( 240, 244, 250, 255 );
			clrTextSecondary    = Color( 185, 200, 220, 255 );
			clrTextMuted        = Color( 128, 140, 158, 255 );
			clrButtonBg         = Color( 32, 39, 52, 255 );
			clrButtonHoverBg    = Color( 45, 55, 74, 255 );
			clrButtonActiveBg   = Color( 52, 66, 88, 255 );
			clrButtonCTBg       = Color( 28, 54, 86, 255 );
			clrButtonCTHoverBg  = Color( 44, 82, 128, 255 );
			clrButtonTBg        = Color( 82, 52, 22, 255 );
			clrButtonTHoverBg   = Color( 126, 78, 30, 255 );
			clrButtonBothBg     = Color( 68, 56, 24, 255 );
			clrButtonBothHoverBg= Color( 108, 86, 32, 255 );
			clrDangerBg         = Color( 90, 34, 36, 255 );
			clrDangerHoverBg    = Color( 145, 48, 48, 255 );
			clrDangerAccent     = Color( 235, 75, 75, 255 );
		}

		void LoadFromScheme( vgui::IScheme *pScheme )
		{
			LoadDefaults();
			if ( !pScheme )
				return;

			clrWindowBg         = pScheme->GetColor( "CSInventory.WindowBg",         clrWindowBg );
			clrTopBarBg         = pScheme->GetColor( "CSInventory.TopBarBg",         clrTopBarBg );
			clrPanelBg          = pScheme->GetColor( "CSInventory.PanelBg",          clrPanelBg );
			clrPanelBorder      = pScheme->GetColor( "CSInventory.PanelBorder",      clrPanelBorder );
			clrAccent           = pScheme->GetColor( "CSInventory.Accent",           clrAccent );
			clrAccentCT         = pScheme->GetColor( "CSInventory.AccentCT",         clrAccentCT );
			clrAccentT          = pScheme->GetColor( "CSInventory.AccentT",          clrAccentT );
			clrPreviewBg        = pScheme->GetColor( "CSInventory.PreviewBg",        clrPreviewBg );
			clrTileBg           = pScheme->GetColor( "CSInventory.TileBg",           clrTileBg );
			clrTileHoverBg      = pScheme->GetColor( "CSInventory.TileHoverBg",      clrTileHoverBg );
			clrTileSelectedBg   = pScheme->GetColor( "CSInventory.TileSelectedBg",   clrTileSelectedBg );
			clrTileArtBg        = pScheme->GetColor( "CSInventory.TileArtBg",        clrTileArtBg );
			clrTileBorder       = pScheme->GetColor( "CSInventory.TileBorder",       clrTileBorder );
			clrTileSelectedText = pScheme->GetColor( "CSInventory.TileSelectedText", clrTileSelectedText );
			clrTextPrimary      = pScheme->GetColor( "CSInventory.TextPrimary",      clrTextPrimary );
			clrTextSecondary    = pScheme->GetColor( "CSInventory.TextSecondary",    clrTextSecondary );
			clrTextMuted        = pScheme->GetColor( "CSInventory.TextMuted",        clrTextMuted );
			clrButtonBg         = pScheme->GetColor( "CSInventory.ButtonBg",         clrButtonBg );
			clrButtonHoverBg    = pScheme->GetColor( "CSInventory.ButtonHoverBg",    clrButtonHoverBg );
			clrButtonActiveBg   = pScheme->GetColor( "CSInventory.ButtonActiveBg",   clrButtonActiveBg );
			clrButtonCTBg       = pScheme->GetColor( "CSInventory.ButtonCTBg",       clrButtonCTBg );
			clrButtonCTHoverBg  = pScheme->GetColor( "CSInventory.ButtonCTHoverBg",  clrButtonCTHoverBg );
			clrButtonTBg        = pScheme->GetColor( "CSInventory.ButtonTBg",        clrButtonTBg );
			clrButtonTHoverBg   = pScheme->GetColor( "CSInventory.ButtonTHoverBg",   clrButtonTHoverBg );
			clrButtonBothBg     = pScheme->GetColor( "CSInventory.ButtonBothBg",     clrButtonBothBg );
			clrButtonBothHoverBg= pScheme->GetColor( "CSInventory.ButtonBothHoverBg",clrButtonBothHoverBg );
			clrDangerBg         = pScheme->GetColor( "CSInventory.DangerBg",         clrDangerBg );
			clrDangerHoverBg    = pScheme->GetColor( "CSInventory.DangerHoverBg",    clrDangerHoverBg );
			clrDangerAccent     = pScheme->GetColor( "CSInventory.DangerAccent",     clrDangerAccent );
		}
	};

	static Color GetRarityColor( EInventoryRarity eRarity )
	{
		switch ( eRarity )
		{
		case RARITY_MILSPEC:    return Color( 75, 105, 255, 255 );
		case RARITY_RESTRICTED: return Color( 136, 71, 255, 255 );
		case RARITY_CLASSIFIED: return Color( 211, 44, 230, 255 );
		case RARITY_COVERT:     return Color( 235, 75, 75, 255 );
		case RARITY_GOLD:       return Color( 229, 169, 59, 255 );
		case RARITY_STOCK:
		default:                return Color( 130, 142, 160, 255 );
		}
	}

	static void GetLocalizedRarityLabel( EInventoryRarity eRarity, char *pszOut, int nOutSize );

	static bool ContainsCaseInsensitive( const char *pszHaystack, const char *pszNeedle )
	{
		if ( !pszNeedle || !pszNeedle[0] )
			return true;
		if ( !pszHaystack || !pszHaystack[0] )
			return false;

		return ( V_stristr( pszHaystack, pszNeedle ) != NULL );
	}

	static void EnsureInventoryLocalizationLoaded()
	{
		static bool s_bLocLoaded = false;
		if ( !s_bLocLoaded && g_pVGuiLocalize )
		{
			s_bLocLoaded = true;
			g_pVGuiLocalize->AddFile( "resource/csgo_inventory_%language%.txt", "GAME", true );
			if ( !g_pVGuiLocalize->Find( "#CSInventory_Title" ) )
			{
				g_pVGuiLocalize->AddFile( "resource/csgo_inventory_english.txt", "GAME", true );
			}
		}
	}

	static void LocUTF8( const char *pszToken, const char *pszFallback, char *pszOut, int nOutSize )
	{
		EnsureInventoryLocalizationLoaded();
		if ( g_pVGuiLocalize && pszToken && pszToken[0] )
		{
			const wchar_t *pwsz = g_pVGuiLocalize->Find( pszToken );
			if ( pwsz && pwsz[0] )
			{
				V_UnicodeToUTF8( pwsz, pszOut, nOutSize );
				return;
			}
		}
		V_strncpy( pszOut, pszFallback ? pszFallback : "", nOutSize );
	}

	static void LocFormat1UTF8( const char *pszToken, const char *pszFallbackFmt, const char *pszArg1, char *pszOut, int nOutSize )
	{
		EnsureInventoryLocalizationLoaded();
		if ( g_pVGuiLocalize && pszToken && pszToken[0] )
		{
			const wchar_t *pwszFmt = g_pVGuiLocalize->Find( pszToken );
			if ( pwszFmt && pwszFmt[0] )
			{
				wchar_t wszArg1[128];
				V_UTF8ToUnicode( pszArg1 ? pszArg1 : "", wszArg1, sizeof( wszArg1 ) );
				wchar_t wszOut[256];
				g_pVGuiLocalize->ConstructString( wszOut, sizeof( wszOut ), const_cast<wchar_t *>( pwszFmt ), 1, wszArg1 );
				V_UnicodeToUTF8( wszOut, pszOut, nOutSize );
				return;
			}
		}
		V_snprintf( pszOut, nOutSize, pszFallbackFmt ? pszFallbackFmt : "%s", pszArg1 ? pszArg1 : "" );
	}

	static void LocFormat2UTF8( const char *pszToken, const char *pszFallbackFmt, const char *pszArg1, const char *pszArg2, char *pszOut, int nOutSize )
	{
		EnsureInventoryLocalizationLoaded();
		if ( g_pVGuiLocalize && pszToken && pszToken[0] )
		{
			const wchar_t *pwszFmt = g_pVGuiLocalize->Find( pszToken );
			if ( pwszFmt && pwszFmt[0] )
			{
				wchar_t wszArg1[128], wszArg2[128];
				V_UTF8ToUnicode( pszArg1 ? pszArg1 : "", wszArg1, sizeof( wszArg1 ) );
				V_UTF8ToUnicode( pszArg2 ? pszArg2 : "", wszArg2, sizeof( wszArg2 ) );
				wchar_t wszOut[256];
				g_pVGuiLocalize->ConstructString( wszOut, sizeof( wszOut ), const_cast<wchar_t *>( pwszFmt ), 2, wszArg1, wszArg2 );
				V_UnicodeToUTF8( wszOut, pszOut, nOutSize );
				return;
			}
		}
		V_snprintf( pszOut, nOutSize, pszFallbackFmt ? pszFallbackFmt : "%s %s", pszArg1 ? pszArg1 : "", pszArg2 ? pszArg2 : "" );
	}

	static void GetLocalizedRarityLabel( EInventoryRarity eRarity, char *pszOut, int nOutSize )
	{
		switch ( eRarity )
		{
		case RARITY_MILSPEC:    LocUTF8( "#CSInventory_Rarity_MilSpec", "Mil-Spec Grade", pszOut, nOutSize ); break;
		case RARITY_RESTRICTED: LocUTF8( "#CSInventory_Rarity_Restricted", "Restricted", pszOut, nOutSize ); break;
		case RARITY_CLASSIFIED: LocUTF8( "#CSInventory_Rarity_Classified", "Classified", pszOut, nOutSize ); break;
		case RARITY_COVERT:     LocUTF8( "#CSInventory_Rarity_Covert", "Covert", pszOut, nOutSize ); break;
		case RARITY_GOLD:       LocUTF8( "#CSInventory_Rarity_Extraordinary", "Extraordinary", pszOut, nOutSize ); break;
		case RARITY_STOCK:
		default:                LocUTF8( "#CSInventory_Rarity_Standard", "Standard Issue", pszOut, nOutSize ); break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Cached VGUI icon/sticker artwork textures (lazy-loaded on demand)
//-----------------------------------------------------------------------------
struct CachedVGuiTex_t
{
	CachedVGuiTex_t() : nTexID( -1 ), bAttempted( false ) { szKey[0] = '\0'; }

	char szKey[MAX_PATH];
	int nTexID;
	CMaterialReference matRef;
	bool bAttempted;
};

static int GetOrLoadStickerVGuiTexture( CUtlVector<CachedVGuiTex_t> &cache, int nStickerIdx )
{
	if ( nStickerIdx < 0 || nStickerIdx >= cache.Count() )
		return -1;

	CachedVGuiTex_t &entry = cache[nStickerIdx];
	if ( entry.bAttempted )
		return entry.nTexID;

	entry.bAttempted = true;

	const InventoryStickerDef_t *pSticker = Inventory_GetStickerDef( nStickerIdx );
	if ( !pSticker )
		return -1;

	char szTexPath[MAX_PATH];
	Inventory_GetStickerTexturePath( pSticker->pszKey, szTexPath, sizeof( szTexPath ) );

	KeyValues *pKV = new KeyValues( "UnlitGeneric" );
	pKV->SetString( "$basetexture", szTexPath );
	pKV->SetInt( "$vertexcolor", 1 );
	pKV->SetInt( "$vertexalpha", 1 );
	pKV->SetInt( "$translucent", 1 );
	pKV->SetInt( "$ignorez", 1 );
	pKV->SetInt( "$nocull", 1 );

	char szMatName[128];
	V_snprintf( szMatName, sizeof( szMatName ), "__csinv_sticker_%s", pSticker->pszKey );
	entry.matRef.Init( szMatName, pKV );
	if ( !entry.matRef.IsValid() || !g_pMatSystemSurface )
		return -1;

	entry.nTexID = vgui::surface()->CreateNewTextureID( false );
	g_pMatSystemSurface->DrawSetTextureMaterial( entry.nTexID, entry.matRef );
	return entry.nTexID;
}

static int GetOrLoadPathVGuiTexture( CUtlVector<CachedVGuiTex_t> &cache, const char *pszTexPath )
{
	if ( !pszTexPath || !pszTexPath[0] )
		return -1;

	FOR_EACH_VEC( cache, i )
	{
		if ( !V_stricmp( cache[i].szKey, pszTexPath ) )
			return cache[i].nTexID;
	}

	int idx = cache.AddToTail();
	CachedVGuiTex_t &entry = cache[idx];
	V_strncpy( entry.szKey, pszTexPath, sizeof( entry.szKey ) );
	entry.bAttempted = true;
	entry.nTexID = -1;

	if ( !g_pMatSystemSurface || !vgui::surface() )
		return -1;

	char szMatName[MAX_PATH];
	V_snprintf( szMatName, sizeof( szMatName ), "__csinv_icon_%d", idx );

	KeyValues *pKV = new KeyValues( "UnlitGeneric" );
	pKV->SetString( "$basetexture", pszTexPath );
	pKV->SetInt( "$vertexcolor", 1 );
	pKV->SetInt( "$vertexalpha", 1 );
	pKV->SetInt( "$translucent", 1 );
	pKV->SetInt( "$ignorez", 1 );
	pKV->SetInt( "$nocull", 1 );

	entry.matRef.Init( szMatName, pKV );
	if ( !entry.matRef.IsValid() )
		return -1;

	entry.nTexID = vgui::surface()->CreateNewTextureID( false );
	g_pMatSystemSurface->DrawSetTextureMaterial( entry.nTexID, entry.matRef );
	return entry.nTexID;
}

//-----------------------------------------------------------------------------
// Purpose: 2D Artwork Preview Card (Item Icon + Active AK-47 Sticker Badges)
//-----------------------------------------------------------------------------
class CInventoryPreviewCard : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CInventoryPreviewCard, vgui::Panel );

public:
	CInventoryPreviewCard( vgui::Panel *pParent, const char *pName )
		: BaseClass( pParent, pName )
		, m_nTexID( -1 )
		, m_bSquareIcon( false )
		, m_bPadded43( true )
		, m_clrPreviewBg( 20, 25, 34, 255 )
		, m_clrBorder( 54, 65, 84, 255 )
		, m_clrStripe( 229, 169, 59, 255 )
		, m_clrMuted( 130, 142, 162, 255 )
		, m_hFont( INVALID_FONT )
	{
		m_szPlaceholder[0] = '\0';
		for ( int s = 0; s < INV_MAX_STICKER_SLOTS; ++s )
		{
			m_nStickerTexIDs[s] = -1;
		}
		SetPaintBorderEnabled( false );
		SetPaintBackgroundEnabled( true );
		SetMouseInputEnabled( false );
		SetKeyBoardInputEnabled( false );
	}

	void ApplyPalette( const InventoryPalette_t &pal )
	{
		m_clrPreviewBg = pal.clrPreviewBg;
		m_clrBorder = pal.clrPanelBorder;
		m_clrMuted = pal.clrTextMuted;
	}

	void SetPreviewItem( int nTexID, bool bSquareIcon, bool bPadded43, Color clrStripe, const char *pszPlaceholder, const int stickerTexIDs[INV_MAX_STICKER_SLOTS] )
	{
		m_nTexID = nTexID;
		m_bSquareIcon = bSquareIcon;
		m_bPadded43 = bPadded43;
		m_clrStripe = clrStripe;
		V_strncpy( m_szPlaceholder, pszPlaceholder ? pszPlaceholder : "", sizeof( m_szPlaceholder ) );
		for ( int s = 0; s < INV_MAX_STICKER_SLOTS; ++s )
		{
			m_nStickerTexIDs[s] = stickerTexIDs ? stickerTexIDs[s] : -1;
		}
		Repaint();
	}

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) OVERRIDE
	{
		BaseClass::ApplySchemeSettings( pScheme );
		m_hFont = pScheme->GetFont( "Default", true );
	}

	virtual void PaintBackground() OVERRIDE
	{
		int w = GetWide(), h = GetTall();
		surface()->DrawSetColor( m_clrPreviewBg );
		surface()->DrawFilledRect( 0, 0, w, h );

		surface()->DrawSetColor( m_clrStripe );
		surface()->DrawFilledRect( 0, 0, w, 3 );

		if ( m_nTexID > 0 && w > 24 && h > 24 )
		{
			int padX = Max( 16, w / 10 );
			int padY = Max( 12, h / 10 );
			int availW = Max( 16, w - padX * 2 );
			int availH = Max( 16, h - padY * 2 );

			int drawW = availW;
			int drawH = availH;
			if ( m_bSquareIcon )
			{
				int side = Min( availW, availH );
				drawW = side;
				drawH = side;
			}
			else
			{
				const float aspect = 4.0f / 3.0f;
				drawH = Min( availH, (int)( availW / aspect ) );
				drawW = Min( availW, (int)( drawH * aspect ) );
			}

			int drawX = ( w - drawW ) / 2;
			int drawY = ( h - drawH ) / 2;

			surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
			surface()->DrawSetTexture( m_nTexID );
			if ( m_bSquareIcon || !m_bPadded43 )
			{
				surface()->DrawTexturedRect( drawX, drawY, drawX + drawW, drawY + drawH );
			}
			else
			{
				surface()->DrawTexturedSubRect( drawX, drawY, drawX + drawW, drawY + drawH, 0.0f, 32.0f / 256.0f, 1.0f, 224.0f / 256.0f );
			}
		}
		else if ( m_szPlaceholder[0] && m_hFont != INVALID_FONT )
		{
			wchar_t wszBuf[96];
			V_UTF8ToUnicode( m_szPlaceholder, wszBuf, sizeof( wszBuf ) );
			int tw = 0, th = 0;
			surface()->GetTextSize( m_hFont, wszBuf, tw, th );
			surface()->DrawSetTextFont( m_hFont );
			surface()->DrawSetTextColor( m_clrMuted );
			surface()->DrawSetTextPos( ( w - tw ) / 2, ( h - th ) / 2 );
			surface()->DrawPrintText( wszBuf, wcslen( wszBuf ) );
		}

		// Render compact sticker thumbnails in bottom-right when any AK-47 sticker is active
		int badgeSize = clamp( h / 5, 24, 38 );
		int badgeGap = 5;
		int bx = w - 10;
		int by = h - badgeSize - 8;
		for ( int s = INV_MAX_STICKER_SLOTS - 1; s >= 0; --s )
		{
			if ( m_nStickerTexIDs[s] <= 0 )
				continue;

			bx -= badgeSize;
			surface()->DrawSetColor( Color( 14, 18, 26, 220 ) );
			surface()->DrawFilledRect( bx, by, bx + badgeSize, by + badgeSize );
			surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
			surface()->DrawSetTexture( m_nStickerTexIDs[s] );
			surface()->DrawTexturedRect( bx + 2, by + 2, bx + badgeSize - 2, by + badgeSize - 2 );
			surface()->DrawSetColor( m_clrBorder );
			surface()->DrawOutlinedRect( bx, by, bx + badgeSize, by + badgeSize );
			bx -= badgeGap;
		}

		surface()->DrawSetColor( m_clrBorder );
		surface()->DrawOutlinedRect( 0, 0, w, h );
	}

private:
	int m_nTexID;
	bool m_bSquareIcon;
	bool m_bPadded43;
	int m_nStickerTexIDs[INV_MAX_STICKER_SLOTS];
	char m_szPlaceholder[96];
	Color m_clrPreviewBg;
	Color m_clrBorder;
	Color m_clrStripe;
	Color m_clrMuted;
	vgui::HFont m_hFont;
};

//-----------------------------------------------------------------------------
// Purpose: Sleek tactical VGUI button with scheme colors & active bar
//-----------------------------------------------------------------------------
class CInventoryStyledButton : public vgui::Button
{
	DECLARE_CLASS_SIMPLE( CInventoryStyledButton, vgui::Button );

public:
	CInventoryStyledButton( vgui::Panel *pParent, const char *pName, const char *pText, const char *pCmd )
		: BaseClass( pParent, pName, pText )
		, m_bActive( false )
		, m_clrBg( 32, 39, 52, 255 )
		, m_clrHover( 45, 55, 74, 255 )
		, m_clrActiveBg( 52, 66, 88, 255 )
		, m_clrAccent( 229, 169, 59, 255 )
		, m_clrText( 235, 240, 248, 255 )
		, m_clrBorder( 54, 65, 84, 255 )
	{
		if ( pCmd && pCmd[0] )
		{
			SetCommand( pCmd );
		}
		SetPaintBorderEnabled( false );
		SetContentAlignment( Label::a_center );
	}

	void SetActive( bool bActive ) { m_bActive = bActive; }
	bool IsActive() const { return m_bActive; }

	void SetStyleColors( Color clrBg, Color clrHover, Color clrActiveBg, Color clrAccent, Color clrText = Color( 235, 240, 248, 255 ), Color clrBorder = Color( 54, 65, 84, 255 ) )
	{
		m_clrBg = clrBg;
		m_clrHover = clrHover;
		m_clrActiveBg = clrActiveBg;
		m_clrAccent = clrAccent;
		m_clrText = clrText;
		m_clrBorder = clrBorder;
	}

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) OVERRIDE
	{
		BaseClass::ApplySchemeSettings( pScheme );
		SetFont( pScheme->GetFont( "DefaultSmall", true ) );
	}

	virtual void PaintBackground() OVERRIDE
	{
		int w = GetWide(), h = GetTall();
		Color bg = m_bActive ? m_clrActiveBg : ( IsArmed() ? m_clrHover : m_clrBg );
		if ( IsDepressed() )
		{
			bg = Color( Min( 255, bg.r() + 14 ), Min( 255, bg.g() + 14 ), Min( 255, bg.b() + 14 ), 255 );
		}

		surface()->DrawSetColor( bg );
		surface()->DrawFilledRect( 0, 0, w, h );

		surface()->DrawSetColor( m_bActive ? m_clrAccent : m_clrBorder );
		surface()->DrawOutlinedRect( 0, 0, w, h );

		if ( m_bActive )
		{
			surface()->DrawSetColor( m_clrAccent );
			surface()->DrawFilledRect( 0, h - 2, w, h );
		}
	}

	virtual void Paint() OVERRIDE
	{
		SetFgColor( IsEnabled() ? m_clrText : Color( 110, 120, 136, 255 ) );
		BaseClass::Paint();
	}

private:
	bool m_bActive;
	Color m_clrBg;
	Color m_clrHover;
	Color m_clrActiveBg;
	Color m_clrAccent;
	Color m_clrText;
	Color m_clrBorder;
};

//-----------------------------------------------------------------------------
// Purpose: Reusable Grid Tile Card (Skin/Knife/Glove/Sleeve/Sticker Icons + Badges)
//-----------------------------------------------------------------------------
class CInventoryGridTile : public vgui::Button
{
	DECLARE_CLASS_SIMPLE( CInventoryGridTile, vgui::Button );

public:
	CInventoryGridTile( vgui::Panel *pParent, const char *pName )
		: BaseClass( pParent, pName, "" )
		, m_nEquippedMask( INV_TEAMMASK_NONE )
		, m_bSelected( false )
		, m_bSquareIcon( false )
		, m_nTexID( -1 )
		, m_clrStripe( 130, 142, 160, 255 )
		, m_clrIconTint( 255, 255, 255, 255 )
		, m_clrBg( 28, 34, 46, 255 )
		, m_clrHoverBg( 36, 44, 58, 255 )
		, m_clrSelBg( 40, 50, 68, 255 )
		, m_clrArtBg( 20, 25, 34, 255 )
		, m_clrBorder( 48, 58, 76, 255 )
		, m_clrAccent( 229, 169, 59, 255 )
		, m_clrAccentCT( 70, 145, 225, 255 )
		, m_clrAccentT( 217, 130, 43, 255 )
		, m_clrSelText( 255, 224, 145, 255 )
		, m_clrText( 228, 234, 242, 255 )
		, m_hTitleFont( INVALID_FONT )
	{
		m_szTitle[0] = '\0';
		m_szPlaceholder[0] = '\0';
		SetPaintBorderEnabled( false );
	}

	void ApplyPalette( const InventoryPalette_t &pal )
	{
		m_clrBg       = pal.clrTileBg;
		m_clrHoverBg  = pal.clrTileHoverBg;
		m_clrSelBg    = pal.clrTileSelectedBg;
		m_clrArtBg    = pal.clrTileArtBg;
		m_clrBorder   = pal.clrTileBorder;
		m_clrAccent   = pal.clrAccent;
		m_clrAccentCT = pal.clrAccentCT;
		m_clrAccentT  = pal.clrAccentT;
		m_clrSelText  = pal.clrTileSelectedText;
		m_clrText     = pal.clrTextPrimary;
	}

	void ConfigureTile( const char *pszTitle, Color clrStripe, Color clrIconTint, int nTexID, int nEquippedMask, bool bSelected, bool bSquareIcon, const char *pszPlaceholder = NULL )
	{
		V_strncpy( m_szTitle, pszTitle ? pszTitle : "", sizeof( m_szTitle ) );
		V_strncpy( m_szPlaceholder, pszPlaceholder ? pszPlaceholder : "", sizeof( m_szPlaceholder ) );
		m_clrStripe = clrStripe;
		m_clrIconTint = clrIconTint;
		m_nTexID = nTexID;
		m_nEquippedMask = nEquippedMask;
		m_bSelected = bSelected;
		m_bSquareIcon = bSquareIcon;
		Repaint();
	}

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) OVERRIDE
	{
		BaseClass::ApplySchemeSettings( pScheme );
		m_hTitleFont = pScheme->GetFont( "DefaultVerySmall", true );
		if ( m_hTitleFont == INVALID_FONT )
			m_hTitleFont = pScheme->GetFont( "DefaultSmall", true );
	}

	virtual void PaintBackground() OVERRIDE
	{
		int w = GetWide(), h = GetTall();

		Color bg = m_bSelected ? m_clrSelBg : ( IsArmed() ? m_clrHoverBg : m_clrBg );
		surface()->DrawSetColor( bg );
		surface()->DrawFilledRect( 0, 0, w, h );

		int footerH = Max( 22, (int)( h * 0.24f ) );
		surface()->DrawSetColor( m_clrArtBg );
		surface()->DrawFilledRect( 2, 4, w - 2, h - footerH );

		// Rarity / Accent top stripe
		surface()->DrawSetColor( m_clrStripe );
		surface()->DrawFilledRect( 0, 0, w, 4 );

		// Icon artwork (256x192 4:3 for CS:GO icons, 1:1 for stickers)
		int artW = w - 14;
		int artH = ( h - footerH ) - 10;
		if ( m_nTexID > 0 && artW > 8 && artH > 8 )
		{
			int drawW = artW;
			int drawH = artH;
			if ( m_bSquareIcon )
			{
				int side = Min( artW, artH );
				drawW = side;
				drawH = side;
			}
			else
			{
				float aspect = 4.0f / 3.0f;
				drawH = Min( artH, (int)( artW / aspect ) );
				drawW = Min( artW, (int)( drawH * aspect ) );
			}

			int drawX = ( w - drawW ) / 2;
			int drawY = 4 + ( ( h - footerH - 4 ) - drawH ) / 2;

			surface()->DrawSetColor( m_clrIconTint );
			surface()->DrawSetTexture( m_nTexID );
			if ( m_bSquareIcon )
			{
				surface()->DrawTexturedRect( drawX, drawY, drawX + drawW, drawY + drawH );
			}
			else
			{
				surface()->DrawTexturedSubRect( drawX, drawY, drawX + drawW, drawY + drawH, 0.0f, 32.0f / 256.0f, 1.0f, 224.0f / 256.0f );
			}
		}
		else if ( m_szPlaceholder[0] )
		{
			DrawCenteredText( w / 2, ( h - footerH ) / 2, m_szPlaceholder, Color( 140, 152, 170, 255 ) );
		}

		// Equipped [CT] / [T] corner badges
		char szCT[16], szT[16];
		LocUTF8( "#CSInventory_TeamCT_Short", "CT", szCT, sizeof( szCT ) );
		LocUTF8( "#CSInventory_TeamT_Short", "T", szT, sizeof( szT ) );

		int badgeX = w - 6;
		int badgeY = 8;
		int badgeH = 16;
		if ( m_nEquippedMask & INV_TEAMMASK_T )
		{
			int bw = 20;
			badgeX -= bw;
			surface()->DrawSetColor( m_clrAccentT );
			surface()->DrawFilledRect( badgeX, badgeY, badgeX + bw, badgeY + badgeH );
			DrawCenteredText( badgeX + bw / 2, badgeY + badgeH / 2, szT, Color( 18, 20, 24, 255 ) );
			badgeX -= 3;
		}
		if ( m_nEquippedMask & INV_TEAMMASK_CT )
		{
			int bw = 24;
			badgeX -= bw;
			surface()->DrawSetColor( m_clrAccentCT );
			surface()->DrawFilledRect( badgeX, badgeY, badgeX + bw, badgeY + badgeH );
			DrawCenteredText( badgeX + bw / 2, badgeY + badgeH / 2, szCT, Color( 245, 250, 255, 255 ) );
		}

		// Footer label
		DrawCenteredText( w / 2, h - footerH / 2, m_szTitle, m_bSelected ? m_clrSelText : m_clrText );

		// Outer border
		if ( m_bSelected )
		{
			surface()->DrawSetColor( m_clrAccent );
			surface()->DrawOutlinedRect( 0, 0, w, h );
			surface()->DrawOutlinedRect( 1, 1, w - 1, h - 1 );
		}
		else
		{
			surface()->DrawSetColor( IsArmed() ? Color( 120, 140, 170, 255 ) : m_clrBorder );
			surface()->DrawOutlinedRect( 0, 0, w, h );
		}
	}

	virtual void Paint() OVERRIDE {}

private:
	void DrawCenteredText( int cx, int cy, const char *pszText, Color clr )
	{
		if ( !pszText || !pszText[0] || m_hTitleFont == INVALID_FONT )
			return;

		wchar_t wszBuf[96];
		V_UTF8ToUnicode( pszText, wszBuf, sizeof( wszBuf ) );

		int tw = 0, th = 0;
		surface()->GetTextSize( m_hTitleFont, wszBuf, tw, th );

		int x = cx - tw / 2;
		int y = cy - th / 2;
		int maxW = Max( 10, GetWide() - 8 );
		if ( tw > maxW )
		{
			x = 4;
		}

		surface()->DrawSetTextFont( m_hTitleFont );
		surface()->DrawSetTextColor( clr );
		surface()->DrawSetTextPos( x, y );
		surface()->DrawUnicodeString( wszBuf );
	}

	char m_szTitle[80];
	char m_szPlaceholder[48];
	int m_nEquippedMask;
	bool m_bSelected;
	bool m_bSquareIcon;
	int m_nTexID;
	Color m_clrStripe;
	Color m_clrIconTint;
	Color m_clrBg;
	Color m_clrHoverBg;
	Color m_clrSelBg;
	Color m_clrArtBg;
	Color m_clrBorder;
	Color m_clrAccent;
	Color m_clrAccentCT;
	Color m_clrAccentT;
	Color m_clrSelText;
	Color m_clrText;
	vgui::HFont m_hTitleFont;
};

//-----------------------------------------------------------------------------
// Purpose: Sticker Slot Workbench Button [1]..[4] showing real sticker VTF artwork
//-----------------------------------------------------------------------------
class CInventoryStickerSlotButton : public vgui::Button
{
	DECLARE_CLASS_SIMPLE( CInventoryStickerSlotButton, vgui::Button );

public:
	CInventoryStickerSlotButton( vgui::Panel *pParent, const char *pName, int nSlot )
		: BaseClass( pParent, pName, "" )
		, m_nSlot( nSlot )
		, m_nTexID( -1 )
		, m_bActive( false )
		, m_clrBg( 24, 30, 40, 255 )
		, m_clrHoverBg( 36, 44, 58, 255 )
		, m_clrActiveBg( 44, 56, 76, 255 )
		, m_clrAccent( 229, 169, 59, 255 )
		, m_clrBorder( 52, 62, 80, 255 )
		, m_hFont( INVALID_FONT )
	{
		m_szLabel[0] = '\0';
		m_szEmptyOverride[0] = '\0';
		SetPaintBorderEnabled( false );
	}

	void ApplyPalette( const InventoryPalette_t &pal )
	{
		m_clrBg       = pal.clrPanelBg;
		m_clrHoverBg  = pal.clrTileHoverBg;
		m_clrActiveBg = pal.clrTileSelectedBg;
		m_clrAccent   = pal.clrAccent;
		m_clrBorder   = pal.clrTileBorder;
	}

	void SetSlotData( int nTexID, const char *pszStickerName, bool bActive, const char *pszEmptyOverride = NULL )
	{
		m_nTexID = nTexID;
		V_strncpy( m_szLabel, pszStickerName ? pszStickerName : "", sizeof( m_szLabel ) );
		V_strncpy( m_szEmptyOverride, pszEmptyOverride ? pszEmptyOverride : "", sizeof( m_szEmptyOverride ) );
		m_bActive = bActive;
		Repaint();
	}

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) OVERRIDE
	{
		BaseClass::ApplySchemeSettings( pScheme );
		m_hFont = pScheme->GetFont( "DefaultVerySmall", true );
		if ( m_hFont == INVALID_FONT )
			m_hFont = pScheme->GetFont( "DefaultSmall", true );
	}

	virtual void PaintBackground() OVERRIDE
	{
		int w = GetWide(), h = GetTall();
		Color bg = m_bActive ? m_clrActiveBg : ( IsArmed() ? m_clrHoverBg : m_clrBg );
		surface()->DrawSetColor( bg );
		surface()->DrawFilledRect( 0, 0, w, h );

		int artSpace = Min( w - 10, h - 18 );
		if ( m_nTexID > 0 && artSpace > 8 )
		{
			int ax = ( w - artSpace ) / 2;
			int ay = 4;
			surface()->DrawSetColor( 255, 255, 255, 255 );
			surface()->DrawSetTexture( m_nTexID );
			surface()->DrawTexturedRect( ax, ay, ax + artSpace, ay + artSpace );
		}
		else if ( m_hFont != INVALID_FONT )
		{
			char szSlotNum[16];
			V_snprintf( szSlotNum, sizeof( szSlotNum ), "+ #%d", m_nSlot + 1 );
			wchar_t wsz[16];
			V_UTF8ToUnicode( szSlotNum, wsz, sizeof( wsz ) );
			int tw = 0, th = 0;
			surface()->GetTextSize( m_hFont, wsz, tw, th );
			surface()->DrawSetTextFont( m_hFont );
			surface()->DrawSetTextColor( 128, 140, 158, 255 );
			surface()->DrawSetTextPos( ( w - tw ) / 2, ( h - 14 - th ) / 2 );
			surface()->DrawUnicodeString( wsz );
		}

		if ( m_hFont != INVALID_FONT )
		{
			char szEmpty[48];
			if ( m_szEmptyOverride[0] )
			{
				V_strncpy( szEmpty, m_szEmptyOverride, sizeof( szEmpty ) );
			}
			else
			{
				LocUTF8( "#CSInventory_StickerEmpty", "Empty", szEmpty, sizeof( szEmpty ) );
			}
			const char *pszFooter = m_szLabel[0] ? m_szLabel : szEmpty;
			wchar_t wsz[64];
			V_UTF8ToUnicode( pszFooter, wsz, sizeof( wsz ) );
			int tw = 0, th = 0;
			surface()->GetTextSize( m_hFont, wsz, tw, th );
			int tx = ( tw > w - 6 ) ? 3 : ( w - tw ) / 2;
			surface()->DrawSetTextFont( m_hFont );
			surface()->DrawSetTextColor( m_szLabel[0] ? Color( 230, 236, 245, 255 ) : Color( 110, 120, 138, 255 ) );
			surface()->DrawSetTextPos( tx, h - th - 3 );
			surface()->DrawUnicodeString( wsz );
		}

		Color border = m_bActive ? m_clrAccent : ( IsArmed() ? Color( 110, 130, 160, 255 ) : m_clrBorder );
		surface()->DrawSetColor( border );
		surface()->DrawOutlinedRect( 0, 0, w, h );
		if ( m_bActive )
		{
			surface()->DrawOutlinedRect( 1, 1, w - 1, h - 1 );
		}
	}

	virtual void Paint() OVERRIDE {}

private:
	int m_nSlot;
	int m_nTexID;
	bool m_bActive;
	char m_szLabel[64];
	char m_szEmptyOverride[64];
	Color m_clrBg;
	Color m_clrHoverBg;
	Color m_clrActiveBg;
	Color m_clrAccent;
	Color m_clrBorder;
	vgui::HFont m_hFont;
};

//-----------------------------------------------------------------------------
// Purpose: Main Fullscreen CS:GO VGUI2 Inventory Dialog
//-----------------------------------------------------------------------------
class CCSInventoryPanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( CCSInventoryPanel, vgui::Frame );

public:
	CCSInventoryPanel( vgui::VPANEL parent );
	virtual ~CCSInventoryPanel();

	void OpenInventory();
	void CloseInventory();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) OVERRIDE;
	virtual void PerformLayout() OVERRIDE;
	virtual void PaintBackground() OVERRIDE;
	virtual void OnCommand( const char *command ) OVERRIDE;
	virtual void OnKeyCodePressed( vgui::KeyCode code ) OVERRIDE;
	virtual void OnMouseWheeled( int delta ) OVERRIDE;

	MESSAGE_FUNC( OnSearchTextChanged, "TextChanged" );

private:
	EInventoryWeaponMode GetActiveMode() const;
	int GetActiveSkinWeaponIndex() const;
	EInventoryTeam GetPreviewTeam() const;
	void LoadEquippedStateForActiveTeam();
	void RebuildWeaponList();
	void RebuildGrid();
	void RefreshInspectAndStickerControls();
	void RefreshPreviewPanel();
	void SetStatusBanner( const char *pszText, Color clr );

	InventoryPalette_t m_Palette;

	// Filters & Selection State
	int m_nTeamFilter; // INV_TEAMMASK_BOTH / CT / T
	EInventoryCategory m_eCategoryFilter;
	int m_iSelectedWeaponIndex;
	int m_iWeaponScrollOffset;
	int m_nWeaponMaxScroll;
	int m_nVisibleWeaponBtnLimit;
	int m_nLeftColX;
	int m_nLeftColW;
	bool m_bStickerPickerMode;
	bool m_bKnifeSkinPickerMode;
	int m_iActiveStickerSlot;
	int m_iCurrentPage;
	int m_iTotalPages;

	InventoryWeaponLoadout_t m_PreviewWeaponLoadout[INV_SKIN_WEP_COUNT];
	char m_szLastPickedSticker[64];
	char m_szPreviewKnife[64];
	char m_szPreviewKnifeSkin[64];
	char m_szPreviewKnifeSkinByIdx[32][64];
	char m_szPreviewGloveId[64];
	char m_szPreviewSleeveKey[64];
	char m_szSearchFilter[64];

	int m_nTileItemIndices[INV_GRID_PAGE_SIZE];
	bool m_bTileIsResetItem[INV_GRID_PAGE_SIZE];
	int m_nVisibleWeaponIndices[INV_MAX_WEAPON_BUTTONS];

	CUtlVector<CachedVGuiTex_t> m_StickerTexCache;
	CUtlVector<CachedVGuiTex_t> m_IconTexCache;
	bool m_bShouldHideGameUIOnClose;

	// UI Controls — Top Bar
	vgui::Label *m_pTitleLabel;
	vgui::Label *m_pContextTeamLabel;
	CInventoryStyledButton *m_pTeamFilterBtns[3];
	vgui::TextEntry *m_pSearchEntry;
	CInventoryStyledButton *m_pClearSearchBtn;
	CInventoryStyledButton *m_pCloseBtn;

	// UI Controls — Left Column
	vgui::Label *m_pWeaponsHeader;
	CInventoryStyledButton *m_pCategoryBtns[INV_CAT_COUNT];
	CInventoryStyledButton *m_pWeaponBtns[INV_MAX_WEAPON_BUTTONS];
	CInventoryStyledButton *m_pWepScrollUpBtn;
	CInventoryStyledButton *m_pWepScrollDownBtn;

	// UI Controls — Center Column
	vgui::Label *m_pGridHeaderLabel;
	CInventoryStyledButton *m_pBackToSkinsBtn;
	CInventoryGridTile *m_pGridTiles[INV_GRID_PAGE_SIZE];
	CInventoryStyledButton *m_pPrevPageBtn;
	CInventoryStyledButton *m_pNextPageBtn;
	vgui::Label *m_pPageLabel;
	vgui::Label *m_pUnsupportedLabel;

	// UI Controls — Right Column
	CInventoryPreviewCard *m_pPreviewCard;
	vgui::Label *m_pInspectWeaponLabel;
	vgui::Label *m_pInspectSkinLabel;
	vgui::Label *m_pStatusLabel;
	CInventoryStyledButton *m_pEquipCTBtn;
	CInventoryStyledButton *m_pEquipTBtn;
	CInventoryStyledButton *m_pEquipBothBtn;
	vgui::Label *m_pStickerHeaderLabel;
	CInventoryStickerSlotButton *m_pStickerSlotBtns[INV_MAX_STICKER_SLOTS];
	CInventoryStyledButton *m_pStickerClearBtns[INV_MAX_STICKER_SLOTS];
	CInventoryStyledButton *m_pFillAllStickersBtn;
	CInventoryStyledButton *m_pClearAllStickersBtn;
};

static vgui::DHANDLE<CCSInventoryPanel> g_hCSInventoryPanel;

CCSInventoryPanel::CCSInventoryPanel( vgui::VPANEL parent )
	: BaseClass( NULL, "CSInventoryPanel" )
	, m_nTeamFilter( INV_TEAMMASK_BOTH )
	, m_eCategoryFilter( INV_CAT_ALL )
	, m_iSelectedWeaponIndex( 0 )
	, m_iWeaponScrollOffset( 0 )
	, m_nWeaponMaxScroll( 0 )
	, m_nVisibleWeaponBtnLimit( INV_MAX_WEAPON_BUTTONS )
	, m_nLeftColX( 0 )
	, m_nLeftColW( 0 )
	, m_bStickerPickerMode( false )
	, m_bKnifeSkinPickerMode( false )
	, m_iActiveStickerSlot( 0 )
	, m_iCurrentPage( 0 )
	, m_iTotalPages( 1 )
	, m_bShouldHideGameUIOnClose( false )
{
	SetParent( parent );
	EnsureInventoryLocalizationLoaded();
	m_Palette.LoadDefaults();

	vgui::HScheme hScheme = vgui::scheme()->LoadSchemeFromFileEx( enginevgui->GetPanel( PANEL_CLIENTDLL ), "resource/ClientScheme.res", "ClientScheme" );
	SetScheme( hScheme );
	SetProportional( false );

	SetTitleBarVisible( false );
	SetMoveable( false );
	SetSizeable( false );
	SetCloseButtonVisible( false );
	SetMenuButtonVisible( false );
	SetMinimizeButtonVisible( false );
	SetMaximizeButtonVisible( false );
	SetDeleteSelfOnClose( false );
	SetKeyBoardInputEnabled( true );
	SetMouseInputEnabled( true );

	Inventory_EnsureLoaded();
	for ( int w = 0; w < INV_SKIN_WEP_COUNT; ++w )
	{
		V_strncpy( m_PreviewWeaponLoadout[w].szSkinKey, "0", sizeof( m_PreviewWeaponLoadout[w].szSkinKey ) );
		for ( int s = 0; s < INV_MAX_STICKER_SLOTS; ++s )
		{
			V_strncpy( m_PreviewWeaponLoadout[w].szStickerKeys[s], "0", sizeof( m_PreviewWeaponLoadout[w].szStickerKeys[s] ) );
		}
	}
	V_strncpy( m_szLastPickedSticker, "ibuypower", sizeof( m_szLastPickedSticker ) );
	V_strncpy( m_szPreviewKnife, "0", sizeof( m_szPreviewKnife ) );
	V_strncpy( m_szPreviewGloveId, "default", sizeof( m_szPreviewGloveId ) );
	V_strncpy( m_szPreviewSleeveKey, "-1", sizeof( m_szPreviewSleeveKey ) );
	m_szSearchFilter[0] = '\0';

	m_StickerTexCache.SetCount( Inventory_GetStickerCount() );

	// Default selected weapon to AK-47
	for ( int i = 0; i < Inventory_GetWeaponCount(); ++i )
	{
		const InventoryWeaponDef_t *pDef = Inventory_GetWeaponDef( i );
		if ( pDef && pDef->eMode == INV_MODE_AK47 )
		{
			m_iSelectedWeaponIndex = i;
			break;
		}
	}

	// --- Top Bar ---
	m_pTitleLabel = new Label( this, "InvTitleLabel", "#CSInventory_Title" );
	m_pContextTeamLabel = new Label( this, "InvContextTeamLabel", "" );

	m_pTeamFilterBtns[0] = new CInventoryStyledButton( this, "FilterAllBtn", "#CSInventory_TeamAll", "filter_all" );
	m_pTeamFilterBtns[1] = new CInventoryStyledButton( this, "FilterCTBtn", "#CSInventory_TeamCT", "filter_ct" );
	m_pTeamFilterBtns[2] = new CInventoryStyledButton( this, "FilterTBtn", "#CSInventory_TeamT", "filter_t" );

	m_pSearchEntry = new TextEntry( this, "SearchEntry" );
	m_pSearchEntry->AddActionSignalTarget( this );
	m_pSearchEntry->SetAllowNonAsciiCharacters( true );

	m_pClearSearchBtn = new CInventoryStyledButton( this, "ClearSearchBtn", "#CSInventory_SearchClear", "clear_search" );
	m_pCloseBtn = new CInventoryStyledButton( this, "CloseBtn", "#CSInventory_Close", "close_inventory" );

	// --- Left Column ---
	m_pWeaponsHeader = new Label( this, "WeaponsHeader", "#CSInventory_LoadoutHeader" );
	for ( int c = 0; c < INV_CAT_COUNT; ++c )
	{
		char szName[32], szCmd[32];
		V_snprintf( szName, sizeof( szName ), "CatBtn%d", c );
		V_snprintf( szCmd, sizeof( szCmd ), "cat_%d", c );
		m_pCategoryBtns[c] = new CInventoryStyledButton( this, szName, Inventory_GetCategoryLocToken( (EInventoryCategory)c ), szCmd );
	}
	for ( int i = 0; i < INV_MAX_WEAPON_BUTTONS; ++i )
	{
		char szName[32], szCmd[32];
		V_snprintf( szName, sizeof( szName ), "WeaponBtn%d", i );
		V_snprintf( szCmd, sizeof( szCmd ), "select_weapon_%d", i );
		m_pWeaponBtns[i] = new CInventoryStyledButton( this, szName, "", szCmd );
		m_pWeaponBtns[i]->SetContentAlignment( Label::a_west );
		m_nVisibleWeaponIndices[i] = -1;
	}
	m_pWepScrollUpBtn = new CInventoryStyledButton( this, "WepScrollUpBtn", "#CSInventory_ScrollUp", "wep_scroll_up" );
	m_pWepScrollDownBtn = new CInventoryStyledButton( this, "WepScrollDownBtn", "#CSInventory_ScrollDown", "wep_scroll_down" );

	// --- Center Column ---
	m_pGridHeaderLabel = new Label( this, "GridHeaderLabel", "#CSInventory_Header_AK47Skins" );
	m_pBackToSkinsBtn = new CInventoryStyledButton( this, "BackToSkinsBtn", "#CSInventory_BackToSkins", "back_to_skins" );

	for ( int i = 0; i < INV_GRID_PAGE_SIZE; ++i )
	{
		char szName[32], szCmd[32];
		V_snprintf( szName, sizeof( szName ), "GridTile%d", i );
		V_snprintf( szCmd, sizeof( szCmd ), "grid_tile_%d", i );
		m_pGridTiles[i] = new CInventoryGridTile( this, szName );
		m_pGridTiles[i]->SetCommand( szCmd );
		m_nTileItemIndices[i] = -1;
		m_bTileIsResetItem[i] = false;
	}

	m_pPrevPageBtn = new CInventoryStyledButton( this, "PrevPageBtn", "#CSInventory_PrevPage", "prev_page" );
	m_pNextPageBtn = new CInventoryStyledButton( this, "NextPageBtn", "#CSInventory_NextPage", "next_page" );
	m_pPageLabel = new Label( this, "PageLabel", "" );
	m_pPageLabel->SetContentAlignment( Label::a_center );
	m_pUnsupportedLabel = new Label( this, "UnsupportedLabel", "" );
	m_pUnsupportedLabel->SetContentAlignment( Label::a_center );

	// --- Right Column ---
	m_pPreviewCard = new CInventoryPreviewCard( this, "WeaponPreviewCard" );

	m_pInspectWeaponLabel = new Label( this, "InspectWeaponLabel", "AK-47" );
	m_pInspectSkinLabel = new Label( this, "InspectSkinLabel", "#CSInventory_DefaultStock" );
	m_pStatusLabel = new Label( this, "StatusLabel", "" );

	m_pEquipCTBtn = new CInventoryStyledButton( this, "EquipCTBtn", "#CSInventory_EquipCT", "equip_ct" );
	m_pEquipTBtn = new CInventoryStyledButton( this, "EquipTBtn", "#CSInventory_EquipT", "equip_t" );
	m_pEquipBothBtn = new CInventoryStyledButton( this, "EquipBothBtn", "#CSInventory_EquipBoth", "equip_both" );

	m_pStickerHeaderLabel = new Label( this, "StickerHeaderLabel", "#CSInventory_StickerHeader" );
	for ( int s = 0; s < INV_MAX_STICKER_SLOTS; ++s )
	{
		char szSlotName[32], szSlotCmd[32], szClrName[32], szClrCmd[32];
		V_snprintf( szSlotName, sizeof( szSlotName ), "StickerSlot%d", s );
		V_snprintf( szSlotCmd, sizeof( szSlotCmd ), "sticker_slot_%d", s );
		V_snprintf( szClrName, sizeof( szClrName ), "StickerClear%d", s );
		V_snprintf( szClrCmd, sizeof( szClrCmd ), "sticker_clear_%d", s );

		m_pStickerSlotBtns[s] = new CInventoryStickerSlotButton( this, szSlotName, s );
		m_pStickerSlotBtns[s]->SetCommand( szSlotCmd );

		m_pStickerClearBtns[s] = new CInventoryStyledButton( m_pStickerSlotBtns[s], szClrName, "x", szClrCmd );
		m_pStickerClearBtns[s]->SetTextInset( 0, 0 );
		m_pStickerClearBtns[s]->AddActionSignalTarget( this );
	}

	m_pFillAllStickersBtn = new CInventoryStyledButton( this, "FillAllStickersBtn", "#CSInventory_FillAll4", "stickers_fill_all" );
	m_pClearAllStickersBtn = new CInventoryStyledButton( this, "ClearAllStickersBtn", "#CSInventory_ClearAll", "stickers_clear_all" );
}

CCSInventoryPanel::~CCSInventoryPanel()
{
	FOR_EACH_VEC( m_StickerTexCache, i )
	{
		if ( vgui::surface() && m_StickerTexCache[i].nTexID > 0 )
		{
			vgui::surface()->DestroyTextureID( m_StickerTexCache[i].nTexID );
			m_StickerTexCache[i].nTexID = -1;
		}
		m_StickerTexCache[i].matRef.Shutdown();
	}

	FOR_EACH_VEC( m_IconTexCache, i )
	{
		if ( vgui::surface() && m_IconTexCache[i].nTexID > 0 )
		{
			vgui::surface()->DestroyTextureID( m_IconTexCache[i].nTexID );
			m_IconTexCache[i].nTexID = -1;
		}
		m_IconTexCache[i].matRef.Shutdown();
	}
}

EInventoryWeaponMode CCSInventoryPanel::GetActiveMode() const
{
	const InventoryWeaponDef_t *pWep = Inventory_GetWeaponDef( m_iSelectedWeaponIndex );
	return pWep ? pWep->eMode : INV_MODE_UNSUPPORTED;
}

int CCSInventoryPanel::GetActiveSkinWeaponIndex() const
{
	return Inventory_GetWeaponSkinIndexFromMode( GetActiveMode() );
}

EInventoryTeam CCSInventoryPanel::GetPreviewTeam() const
{
	if ( m_nTeamFilter == INV_TEAMMASK_CT )
		return INV_TEAM_CT;
	if ( m_nTeamFilter == INV_TEAMMASK_T )
		return INV_TEAM_T;

	C_CSPlayer *pLocal = C_CSPlayer::GetLocalCSPlayer();
	if ( pLocal && pLocal->GetTeamNumber() == TEAM_CT )
		return INV_TEAM_CT;

	return INV_TEAM_T;
}

void CCSInventoryPanel::LoadEquippedStateForActiveTeam()
{
	Inventory_EnsureLoaded();
	EInventoryTeam eTeam = GetPreviewTeam();

	for ( int w = 0; w < INV_SKIN_WEP_COUNT; ++w )
	{
		m_PreviewWeaponLoadout[w] = Inventory_GetWeaponLoadout( w, eTeam );
		for ( int s = 0; s < INV_MAX_STICKER_SLOTS; ++s )
		{
			if ( V_stricmp( m_PreviewWeaponLoadout[w].szStickerKeys[s], "0" ) != 0 )
			{
				V_strncpy( m_szLastPickedSticker, m_PreviewWeaponLoadout[w].szStickerKeys[s], sizeof( m_szLastPickedSticker ) );
			}
		}
	}

	V_strncpy( m_szPreviewKnife, Inventory_GetEquippedKnifeKey( eTeam ), sizeof( m_szPreviewKnife ) );
	V_strncpy( m_szPreviewKnifeSkin, Inventory_GetEquippedKnifeSkinKey( eTeam ), sizeof( m_szPreviewKnifeSkin ) );
	for ( int k = 0; k < ARRAYSIZE( m_szPreviewKnifeSkinByIdx ); ++k )
	{
		V_strncpy( m_szPreviewKnifeSkinByIdx[k], "0", sizeof( m_szPreviewKnifeSkinByIdx[k] ) );
	}
	int nEqKnifeIdx = Inventory_FindKnifeIndexByKey( m_szPreviewKnife );
	if ( nEqKnifeIdx >= 0 && nEqKnifeIdx < ARRAYSIZE( m_szPreviewKnifeSkinByIdx ) )
	{
		V_strncpy( m_szPreviewKnifeSkinByIdx[nEqKnifeIdx], m_szPreviewKnifeSkin, sizeof( m_szPreviewKnifeSkinByIdx[nEqKnifeIdx] ) );
	}
	V_strncpy( m_szPreviewGloveId, Inventory_GetEquippedGloveId( eTeam ), sizeof( m_szPreviewGloveId ) );
	V_strncpy( m_szPreviewSleeveKey, Inventory_GetEquippedSleeveKey( eTeam ), sizeof( m_szPreviewSleeveKey ) );
}

void CCSInventoryPanel::OpenInventory()
{
	m_bShouldHideGameUIOnClose = ( !enginevgui->IsGameUIVisible() && engine->IsInGame() && !engine->IsLevelMainMenuBackground() );

	engine->ClientCmd_Unrestricted( "gameui_activate" );
	engine->ClientCmd_Unrestricted( "hideconsole" );

	LoadEquippedStateForActiveTeam();
	m_bStickerPickerMode = false;
	m_bKnifeSkinPickerMode = false;
	m_iCurrentPage = 0;

	InvalidateLayout( true, true );
	RebuildWeaponList();
	RebuildGrid();
	RefreshInspectAndStickerControls();
	RefreshPreviewPanel();

	char szPrompt[160];
	LocUTF8( "#CSInventory_Status_SelectPrompt", "Select an item, customize, then Equip CT / T / Both", szPrompt, sizeof( szPrompt ) );
	SetStatusBanner( szPrompt, m_Palette.clrTextSecondary );

	SetVisible( true );
	Activate();
	MoveToFront();
	RequestFocus();
}

void CCSInventoryPanel::CloseInventory()
{
	SetVisible( false );

	if ( m_bShouldHideGameUIOnClose && engine->IsInGame() )
	{
		engine->ClientCmd_Unrestricted( "gameui_hide" );
	}
	m_bShouldHideGameUIOnClose = false;
}

void CCSInventoryPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	EnsureInventoryLocalizationLoaded();
	m_Palette.LoadFromScheme( pScheme );

	vgui::HFont hTitle = pScheme->GetFont( "Trebuchet24", true );
	if ( hTitle == INVALID_FONT )
		hTitle = pScheme->GetFont( "DefaultLarge", true );
	vgui::HFont hMedium = pScheme->GetFont( "Trebuchet18", true );
	if ( hMedium == INVALID_FONT )
		hMedium = pScheme->GetFont( "Default", true );
	vgui::HFont hSmall = pScheme->GetFont( "DefaultSmall", true );

	m_pTitleLabel->SetFont( hTitle );
	m_pTitleLabel->SetFgColor( m_Palette.clrTextPrimary );

	m_pContextTeamLabel->SetFont( hSmall );
	m_pContextTeamLabel->SetFgColor( m_Palette.clrTextSecondary );

	m_pWeaponsHeader->SetFont( hSmall );
	m_pWeaponsHeader->SetFgColor( m_Palette.clrTextSecondary );

	m_pGridHeaderLabel->SetFont( hMedium );
	m_pGridHeaderLabel->SetFgColor( m_Palette.clrTextPrimary );

	m_pPageLabel->SetFont( hSmall );
	m_pPageLabel->SetFgColor( m_Palette.clrTextSecondary );

	m_pUnsupportedLabel->SetFont( hMedium );
	m_pUnsupportedLabel->SetFgColor( m_Palette.clrTextSecondary );

	m_pInspectWeaponLabel->SetFont( hMedium );
	m_pInspectWeaponLabel->SetFgColor( m_Palette.clrTextSecondary );

	m_pInspectSkinLabel->SetFont( hTitle );
	m_pInspectSkinLabel->SetFgColor( m_Palette.clrTextPrimary );

	m_pStatusLabel->SetFont( hSmall );
	m_pStickerHeaderLabel->SetFont( hSmall );
	m_pStickerHeaderLabel->SetFgColor( m_Palette.clrTextSecondary );

	m_pSearchEntry->SetFont( hSmall );
	m_pSearchEntry->SetBgColor( m_Palette.clrWindowBg );
	m_pSearchEntry->SetFgColor( m_Palette.clrTextPrimary );

	if ( m_pPreviewCard )
	{
		m_pPreviewCard->ApplyPalette( m_Palette );
	}

	m_pTeamFilterBtns[0]->SetStyleColors( m_Palette.clrButtonBg, m_Palette.clrButtonHoverBg, m_Palette.clrButtonActiveBg, m_Palette.clrAccent, m_Palette.clrTextPrimary, m_Palette.clrPanelBorder );
	m_pTeamFilterBtns[1]->SetStyleColors( m_Palette.clrButtonCTBg, m_Palette.clrButtonCTHoverBg, m_Palette.clrButtonCTHoverBg, m_Palette.clrAccentCT, m_Palette.clrTextPrimary, m_Palette.clrPanelBorder );
	m_pTeamFilterBtns[2]->SetStyleColors( m_Palette.clrButtonTBg, m_Palette.clrButtonTHoverBg, m_Palette.clrButtonTHoverBg, m_Palette.clrAccentT, m_Palette.clrTextPrimary, m_Palette.clrPanelBorder );

	m_pClearSearchBtn->SetStyleColors( m_Palette.clrButtonBg, m_Palette.clrButtonHoverBg, m_Palette.clrButtonActiveBg, m_Palette.clrAccent, m_Palette.clrTextPrimary, m_Palette.clrPanelBorder );
	m_pCloseBtn->SetStyleColors( m_Palette.clrDangerBg, m_Palette.clrDangerHoverBg, m_Palette.clrDangerHoverBg, m_Palette.clrDangerAccent, m_Palette.clrTextPrimary, m_Palette.clrPanelBorder );
	m_pBackToSkinsBtn->SetStyleColors( m_Palette.clrButtonBg, m_Palette.clrButtonHoverBg, m_Palette.clrButtonActiveBg, m_Palette.clrAccent, m_Palette.clrTextPrimary, m_Palette.clrPanelBorder );
	m_pPrevPageBtn->SetStyleColors( m_Palette.clrButtonBg, m_Palette.clrButtonHoverBg, m_Palette.clrButtonActiveBg, m_Palette.clrAccent, m_Palette.clrTextPrimary, m_Palette.clrPanelBorder );
	m_pNextPageBtn->SetStyleColors( m_Palette.clrButtonBg, m_Palette.clrButtonHoverBg, m_Palette.clrButtonActiveBg, m_Palette.clrAccent, m_Palette.clrTextPrimary, m_Palette.clrPanelBorder );
	m_pFillAllStickersBtn->SetStyleColors( m_Palette.clrButtonBg, m_Palette.clrButtonHoverBg, m_Palette.clrButtonActiveBg, m_Palette.clrAccent, m_Palette.clrTextPrimary, m_Palette.clrPanelBorder );
	m_pClearAllStickersBtn->SetStyleColors( m_Palette.clrButtonBg, m_Palette.clrButtonHoverBg, m_Palette.clrButtonActiveBg, m_Palette.clrAccent, m_Palette.clrTextPrimary, m_Palette.clrPanelBorder );

	m_pEquipCTBtn->SetStyleColors( m_Palette.clrButtonCTBg, m_Palette.clrButtonCTHoverBg, m_Palette.clrButtonCTHoverBg, m_Palette.clrAccentCT, m_Palette.clrTextPrimary, m_Palette.clrPanelBorder );
	m_pEquipTBtn->SetStyleColors( m_Palette.clrButtonTBg, m_Palette.clrButtonTHoverBg, m_Palette.clrButtonTHoverBg, m_Palette.clrAccentT, m_Palette.clrTextPrimary, m_Palette.clrPanelBorder );
	m_pEquipBothBtn->SetStyleColors( m_Palette.clrButtonBothBg, m_Palette.clrButtonBothHoverBg, m_Palette.clrButtonBothHoverBg, m_Palette.clrAccent, m_Palette.clrTextPrimary, m_Palette.clrPanelBorder );

	for ( int c = 0; c < INV_CAT_COUNT; ++c )
	{
		m_pCategoryBtns[c]->SetStyleColors( m_Palette.clrButtonBg, m_Palette.clrButtonHoverBg, m_Palette.clrButtonActiveBg, m_Palette.clrAccent, m_Palette.clrTextPrimary, m_Palette.clrPanelBorder );
	}
	m_pWepScrollUpBtn->SetStyleColors( m_Palette.clrButtonBg, m_Palette.clrButtonHoverBg, m_Palette.clrButtonActiveBg, m_Palette.clrAccent, m_Palette.clrTextPrimary, m_Palette.clrPanelBorder );
	m_pWepScrollDownBtn->SetStyleColors( m_Palette.clrButtonBg, m_Palette.clrButtonHoverBg, m_Palette.clrButtonActiveBg, m_Palette.clrAccent, m_Palette.clrTextPrimary, m_Palette.clrPanelBorder );
	for ( int i = 0; i < INV_GRID_PAGE_SIZE; ++i )
	{
		m_pGridTiles[i]->ApplyPalette( m_Palette );
	}
	for ( int s = 0; s < INV_MAX_STICKER_SLOTS; ++s )
	{
		m_pStickerSlotBtns[s]->ApplyPalette( m_Palette );
		m_pStickerClearBtns[s]->SetStyleColors( m_Palette.clrDangerBg, m_Palette.clrDangerHoverBg, m_Palette.clrDangerHoverBg, m_Palette.clrDangerAccent, m_Palette.clrTextPrimary, m_Palette.clrPanelBorder );
		m_pStickerClearBtns[s]->SetTextInset( 0, 0 );
	}
}

void CCSInventoryPanel::PerformLayout()
{
	int sw = 0, sh = 0;
	if ( GetVParent() )
	{
		vgui::ipanel()->GetSize( GetVParent(), sw, sh );
	}
	if ( sw <= 0 || sh <= 0 )
	{
		vgui::surface()->GetScreenSize( sw, sh );
	}
	SetBounds( 0, 0, sw, sh );

	BaseClass::PerformLayout();

	InventoryLayoutMetrics_t m;
	ComputeLayoutMetrics( sw, sh, m );

	const int &winX = m.winX, &winY = m.winY, &winW = m.winW;
	const int &topH = m.topH, &gap = m.gap, &pad = m.pad;
	const int &bodyY = m.bodyY, &bodyH = m.bodyH;
	const int &leftX = m.leftX, &leftW = m.leftW;
	const int &centerX = m.centerX, &centerW = m.centerW;
	const int &rightX = m.rightX, &rightW = m.rightW;

	// --- Top Header Bar (proportionally fitted from 640x480 up to 4K / ultrawide) ---
	int closeW = clamp( topH - 14, 28, 40 );
	int topBtnH = clamp( topH - 16, 24, 34 );
	int rowY = winY + ( topH - topBtnH ) / 2;
	int closeX = winX + winW - pad - closeW;
	m_pCloseBtn->SetBounds( closeX, rowY, closeW, topBtnH );

	int filterW = clamp( (int)( winW * 0.052f ), 34, 78 );
	int clrSearchW = clamp( (int)( winW * 0.048f ), 42, 68 );
	int filtersTotalW = filterW * 3 + 5 * 2 + 8;
	int curX = winX + pad;

	int titleCW = 0, titleCH = 0, ctxCW = 0, ctxCH = 0;
	m_pTitleLabel->GetContentSize( titleCW, titleCH );
	m_pContextTeamLabel->GetContentSize( ctxCW, ctxCH );
	int maxTitleW = Max( 140, closeX - curX - filtersTotalW - clrSearchW - 84 );
	int desiredTitleW = clamp( Max( titleCW + 14, ctxCW + 10 ), 180, Max( 220, (int)( winW * 0.34f ) ) );
	int titleBlockW = Min( desiredTitleW, maxTitleW );

	int titleH = clamp( (int)( topH * 0.50f ), 20, 30 );
	int ctxH = clamp( topH - titleH - 6, 14, 20 );
	m_pTitleLabel->SetBounds( curX, winY + 3, titleBlockW, titleH );
	m_pContextTeamLabel->SetBounds( curX, winY + 3 + titleH, titleBlockW, ctxH );
	curX += titleBlockW + 8;

	for ( int i = 0; i < 3; ++i )
	{
		m_pTeamFilterBtns[i]->SetBounds( curX, rowY, filterW, topBtnH );
		curX += filterW + 5;
	}

	int availForSearch = Max( 48, ( closeX - gap - clrSearchW - 4 ) - ( curX + 6 ) );
	int desiredSearchW = clamp( (int)( winW * 0.19f ), 90, 290 );
	int searchW = Min( availForSearch, desiredSearchW );
	int searchX = closeX - gap - clrSearchW - 4 - searchW;
	m_pSearchEntry->SetBounds( searchX, rowY, searchW, topBtnH );
	m_pClearSearchBtn->SetBounds( searchX + searchW + 4, rowY, clrSearchW, topBtnH );

	// Left Column: Categories (2 cols x 4 rows) + Dynamic Scrollable Weapon Rows
	m_nLeftColX = leftX;
	m_nLeftColW = leftW;
	int leftHdrH = clamp( bodyH / 26, 16, 22 );
	m_pWeaponsHeader->SetBounds( leftX + 6, bodyY + 5, leftW - 12, leftHdrH );

	int catCols = 2;
	int catRows = ( INV_CAT_COUNT + catCols - 1 ) / catCols;
	int catGap = clamp( bodyH / 160, 2, 5 );
	int catBtnW = ( leftW - 12 - catGap ) / catCols;
	int catBtnH = clamp( bodyH / 23, 18, 28 );
	int catStartY = bodyY + 5 + leftHdrH + 2;
	for ( int c = 0; c < INV_CAT_COUNT; ++c )
	{
		int col = c % catCols;
		int row = c / catCols;
		m_pCategoryBtns[c]->SetBounds( leftX + 6 + col * ( catBtnW + catGap ), catStartY + row * ( catBtnH + catGap ), catBtnW, catBtnH );
	}

	int wepStartY = catStartY + catRows * ( catBtnH + catGap ) + clamp( bodyH / 100, 4, 10 );
	int scrollFooterH = clamp( bodyH / 22, 20, 28 );
	int wepAvailH = Max( 24, ( bodyY + bodyH - 6 - scrollFooterH - 4 ) - wepStartY );
	int wepGap = 2;
	int wepRowH = clamp( bodyH / 23, 19, 36 );
	int nFitRows = clamp( ( wepAvailH + wepGap ) / ( wepRowH + wepGap ), 4, INV_MAX_WEAPON_BUTTONS );
	wepRowH = Max( 17, ( wepAvailH - ( nFitRows - 1 ) * wepGap ) / nFitRows );
	for ( int i = 0; i < INV_MAX_WEAPON_BUTTONS; ++i )
	{
		m_pWeaponBtns[i]->SetBounds( leftX + 6, wepStartY + i * ( wepRowH + wepGap ), leftW - 12, wepRowH );
	}
	int scrollY = bodyY + bodyH - 6 - scrollFooterH;
	int scrollGap = 4;
	int scrollBtnW = ( leftW - 12 - scrollGap ) / 2;
	m_pWepScrollUpBtn->SetBounds( leftX + 6, scrollY, scrollBtnW, scrollFooterH );
	m_pWepScrollDownBtn->SetBounds( leftX + 6 + scrollBtnW + scrollGap, scrollY, leftW - 12 - scrollBtnW - scrollGap, scrollFooterH );

	if ( m_nVisibleWeaponBtnLimit != nFitRows )
	{
		m_nVisibleWeaponBtnLimit = nFitRows;
		RebuildWeaponList();
	}

	// Center Column: Header + Grid (3x4) + Pagination Footer
	int cHeadH = clamp( bodyH / 18, 26, 36 );
	int cFootH = clamp( bodyH / 18, 26, 36 );
	int backBtnW = clamp( centerW / 3, 104, 152 );
	m_pGridHeaderLabel->SetBounds( centerX + 10, bodyY + 5, Max( 80, centerW - backBtnW - 24 ), cHeadH - 4 );
	m_pBackToSkinsBtn->SetBounds( centerX + centerW - 10 - backBtnW, bodyY + 5, backBtnW, cHeadH - 4 );

	int gridX = centerX + 10;
	int gridY = bodyY + cHeadH + 6;
	int gridW = centerW - 20;
	int gridH = Max( 80, bodyH - cHeadH - cFootH - 14 );
	int cellGap = clamp( bodyH / 90, 5, 10 );
	int tileW = ( gridW - cellGap * ( INV_GRID_COLS - 1 ) ) / INV_GRID_COLS;
	int tileH = ( gridH - cellGap * ( INV_GRID_ROWS - 1 ) ) / INV_GRID_ROWS;

	for ( int i = 0; i < INV_GRID_PAGE_SIZE; ++i )
	{
		int r = i / INV_GRID_COLS;
		int c = i % INV_GRID_COLS;
		m_pGridTiles[i]->SetBounds( gridX + c * ( tileW + cellGap ), gridY + r * ( tileH + cellGap ), tileW, tileH );
	}
	m_pUnsupportedLabel->SetBounds( gridX, gridY + gridH / 2 - 30, gridW, 60 );

	int footY = bodyY + bodyH - cFootH - 4;
	int pageBtnW = clamp( centerW / 5, 64, 120 );
	m_pPrevPageBtn->SetBounds( centerX + 10, footY, pageBtnW, cFootH - 4 );
	m_pNextPageBtn->SetBounds( centerX + centerW - 10 - pageBtnW, footY, pageBtnW, cFootH - 4 );
	m_pPageLabel->SetBounds( centerX + 10 + pageBtnW, footY, Max( 40, centerW - 20 - pageBtnW * 2 ), cFootH - 4 );

	// Right Column: 2D Artwork Preview + Info + Equip CT/T/Both + (Sticker Slots or Knife Skin Slot)
	int rPad = clamp( rightW / 32, 8, 12 );
	int rx = rightX + rPad;
	int rw = rightW - rPad * 2;
	int ry = bodyY + 8;

	bool bShowStickers = ( GetActiveSkinWeaponIndex() >= 0 );
	bool bShowKnifeSlot = ( GetActiveMode() == INV_MODE_KNIFE );
	bool bShowBottomArea = ( bShowStickers || bShowKnifeSlot );

	int smallBtnH = clamp( bodyH / 22, 20, 30 );
	int eqBtnH    = clamp( bodyH / 20, 22, 32 );
	int lblGap    = clamp( bodyH / 160, 2, 6 );
	int inspH1    = clamp( bodyH / 32, 16, 22 );
	int inspH2    = clamp( bodyH / 22, 22, 30 );
	int statusH   = clamp( bodyH / 32, 16, 20 );
	int hdrH      = clamp( bodyH / 34, 15, 20 );
	int stkH      = clamp( bodyH / 9, 44, 96 );
	int bottomAreaH = bShowBottomArea ? ( hdrH + 4 + stkH + 6 + smallBtnH + 6 ) : 6;
	int infoAndEquipH = inspH1 + inspH2 + statusH + lblGap * 3 + eqBtnH * 2 + 12;
	int previewH = Max( 84, bodyH - bottomAreaH - infoAndEquipH - 18 );

	m_pPreviewCard->SetBounds( rx, ry, rw, previewH );
	ry += previewH + 6;

	m_pInspectWeaponLabel->SetBounds( rx, ry, rw, inspH1 );
	ry += inspH1 + lblGap / 2;
	m_pInspectSkinLabel->SetBounds( rx, ry, rw, inspH2 );
	ry += inspH2 + lblGap / 2;
	m_pStatusLabel->SetBounds( rx, ry, rw, statusH );
	ry += statusH + lblGap + 2;

	int halfW = ( rw - 8 ) / 2;
	m_pEquipCTBtn->SetBounds( rx, ry, halfW, eqBtnH );
	m_pEquipTBtn->SetBounds( rx + halfW + 8, ry, rw - halfW - 8, eqBtnH );
	ry += eqBtnH + 4;
	m_pEquipBothBtn->SetBounds( rx, ry, rw, eqBtnH );
	ry += eqBtnH + 6;

	int clrSz = clamp( stkH / 4, 16, 22 );
	if ( bShowStickers )
	{
		m_pStickerHeaderLabel->SetBounds( rx, ry, rw, hdrH );
		ry += hdrH + 3;

		int stkGap = clamp( rw / 40, 5, 8 );
		int stkW = ( rw - stkGap * ( INV_MAX_STICKER_SLOTS - 1 ) ) / INV_MAX_STICKER_SLOTS;
		int availStkH = Max( 40, ( bodyY + bodyH - smallBtnH - 12 ) - ry );
		int actualStkH = Min( stkH, availStkH );
		for ( int s = 0; s < INV_MAX_STICKER_SLOTS; ++s )
		{
			int sx = rx + s * ( stkW + stkGap );
			m_pStickerSlotBtns[s]->SetBounds( sx, ry, stkW, actualStkH );
			m_pStickerClearBtns[s]->SetBounds( stkW - clrSz - 2, 2, clrSz, clrSz );
			m_pStickerClearBtns[s]->SetTextInset( 0, 0 );
		}

		int actionY = ry + actualStkH + 6;
		m_pFillAllStickersBtn->SetBounds( rx, actionY, halfW, smallBtnH );
		m_pClearAllStickersBtn->SetBounds( rx + halfW + 8, actionY, rw - halfW - 8, smallBtnH );
	}
	else if ( bShowKnifeSlot )
	{
		m_pStickerHeaderLabel->SetBounds( rx, ry, rw, hdrH );
		ry += hdrH + 3;

		int availStkH = Max( 40, ( bodyY + bodyH - smallBtnH - 12 ) - ry );
		int actualStkH = Min( stkH, availStkH );
		m_pStickerSlotBtns[0]->SetBounds( rx, ry, rw, actualStkH );
		m_pStickerClearBtns[0]->SetBounds( rw - clrSz - 2, 2, clrSz, clrSz );
		m_pStickerClearBtns[0]->SetTextInset( 0, 0 );

		int actionY = ry + actualStkH + 6;
		m_pFillAllStickersBtn->SetBounds( rx, actionY, halfW, smallBtnH );
		m_pClearAllStickersBtn->SetBounds( rx + halfW + 8, actionY, rw - halfW - 8, smallBtnH );
	}
}

void CCSInventoryPanel::PaintBackground()
{
	int w = GetWide(), h = GetTall();

	surface()->DrawSetColor( m_Palette.clrBackdrop );
	surface()->DrawFilledRect( 0, 0, w, h );

	InventoryLayoutMetrics_t m;
	ComputeLayoutMetrics( w, h, m );

	surface()->DrawSetColor( m_Palette.clrWindowBg );
	surface()->DrawFilledRect( m.winX, m.winY, m.winX + m.winW, m.winY + m.winH );

	surface()->DrawSetColor( m_Palette.clrTopBarBg );
	surface()->DrawFilledRect( m.winX, m.winY, m.winX + m.winW, m.winY + m.topH );
	surface()->DrawSetColor( m_Palette.clrAccent );
	surface()->DrawFilledRect( m.winX, m.winY + m.topH - 2, m.winX + m.winW, m.winY + m.topH );

	// Left, Center, Right Cards
	surface()->DrawSetColor( m_Palette.clrPanelBg );
	surface()->DrawFilledRect( m.leftX, m.bodyY, m.leftX + m.leftW, m.bodyY + m.bodyH );
	surface()->DrawFilledRect( m.centerX, m.bodyY, m.centerX + m.centerW, m.bodyY + m.bodyH );
	surface()->DrawFilledRect( m.rightX, m.bodyY, m.rightX + m.rightW, m.bodyY + m.bodyH );

	surface()->DrawSetColor( m_Palette.clrPanelBorder );
	surface()->DrawOutlinedRect( m.leftX, m.bodyY, m.leftX + m.leftW, m.bodyY + m.bodyH );
	surface()->DrawOutlinedRect( m.centerX, m.bodyY, m.centerX + m.centerW, m.bodyY + m.bodyH );
	surface()->DrawOutlinedRect( m.rightX, m.bodyY, m.rightX + m.rightW, m.bodyY + m.bodyH );

	surface()->DrawSetColor( m_Palette.clrWindowBorder );
	surface()->DrawOutlinedRect( m.winX, m.winY, m.winX + m.winW, m.winY + m.winH );
}

void CCSInventoryPanel::RebuildWeaponList()
{
	m_pTeamFilterBtns[0]->SetActive( m_nTeamFilter == INV_TEAMMASK_BOTH );
	m_pTeamFilterBtns[1]->SetActive( m_nTeamFilter == INV_TEAMMASK_CT );
	m_pTeamFilterBtns[2]->SetActive( m_nTeamFilter == INV_TEAMMASK_T );

	for ( int c = 0; c < INV_CAT_COUNT; ++c )
	{
		m_pCategoryBtns[c]->SetActive( m_eCategoryFilter == c );
	}

	C_CSPlayer *pLocal = C_CSPlayer::GetLocalCSPlayer();
	char szTeamDisplay[48];
	if ( m_nTeamFilter == INV_TEAMMASK_CT || ( m_nTeamFilter == INV_TEAMMASK_BOTH && pLocal && pLocal->GetTeamNumber() == TEAM_CT ) )
		LocUTF8( "#CSInventory_TeamCT", "CT", szTeamDisplay, sizeof( szTeamDisplay ) );
	else if ( m_nTeamFilter == INV_TEAMMASK_T || ( m_nTeamFilter == INV_TEAMMASK_BOTH && pLocal && pLocal->GetTeamNumber() == TEAM_TERRORIST ) )
		LocUTF8( "#CSInventory_TeamT", "T", szTeamDisplay, sizeof( szTeamDisplay ) );
	else
		LocUTF8( "#CSInventory_TeamBothMenu", "Both / Menu", szTeamDisplay, sizeof( szTeamDisplay ) );

	char szContext[128];
	LocFormat1UTF8( "#CSInventory_ContextFormat", "Editing Loadout  |  Active Team: %s", szTeamDisplay, szContext, sizeof( szContext ) );
	m_pContextTeamLabel->SetText( szContext );

	int filteredIndices[128];
	int nFilteredCount = 0;
	for ( int i = 0; i < Inventory_GetWeaponCount() && nFilteredCount < ARRAYSIZE( filteredIndices ); ++i )
	{
		const InventoryWeaponDef_t *pWep = Inventory_GetWeaponDef( i );
		if ( !pWep )
			continue;

		if ( m_eCategoryFilter != INV_CAT_ALL && pWep->eCategory != m_eCategoryFilter )
			continue;

		if ( ( pWep->nAvailableTeamsMask & m_nTeamFilter ) == 0 )
			continue;

		filteredIndices[nFilteredCount++] = i;
	}

	int maxButtons = clamp( m_nVisibleWeaponBtnLimit, 4, INV_MAX_WEAPON_BUTTONS );
	m_nWeaponMaxScroll = Max( 0, nFilteredCount - maxButtons );
	m_iWeaponScrollOffset = clamp( m_iWeaponScrollOffset, 0, m_nWeaponMaxScroll );

	int btnIdx = 0;
	for ( int idx = m_iWeaponScrollOffset; idx < nFilteredCount && btnIdx < maxButtons; ++idx )
	{
		int i = filteredIndices[idx];
		const InventoryWeaponDef_t *pWep = Inventory_GetWeaponDef( i );
		if ( !pWep )
			continue;

		m_nVisibleWeaponIndices[btnIdx] = i;

		char szLabel[64];
		V_snprintf( szLabel, sizeof( szLabel ), "  %s", pWep->pszDisplayName );
		if ( pWep->bSupported )
		{
			m_pWeaponBtns[btnIdx]->SetStyleColors( m_Palette.clrButtonBg, m_Palette.clrButtonHoverBg, m_Palette.clrButtonActiveBg, m_Palette.clrAccent, m_Palette.clrTextPrimary, m_Palette.clrPanelBorder );
		}
		else
		{
			Color mutedBg( Max( 0, m_Palette.clrPanelBg.r() + 2 ), Max( 0, m_Palette.clrPanelBg.g() + 2 ), Max( 0, m_Palette.clrPanelBg.b() + 3 ), 255 );
			m_pWeaponBtns[btnIdx]->SetStyleColors( mutedBg, m_Palette.clrButtonHoverBg, m_Palette.clrButtonActiveBg, m_Palette.clrTextMuted, m_Palette.clrTextMuted, m_Palette.clrPanelBorder );
		}

		m_pWeaponBtns[btnIdx]->SetText( szLabel );
		m_pWeaponBtns[btnIdx]->SetActive( i == m_iSelectedWeaponIndex );
		m_pWeaponBtns[btnIdx]->SetVisible( true );
		++btnIdx;
	}

	for ( int i = btnIdx; i < INV_MAX_WEAPON_BUTTONS; ++i )
	{
		m_nVisibleWeaponIndices[i] = -1;
		m_pWeaponBtns[i]->SetVisible( false );
	}

	bool bCanScroll = ( m_nWeaponMaxScroll > 0 );
	m_pWepScrollUpBtn->SetVisible( bCanScroll );
	m_pWepScrollDownBtn->SetVisible( bCanScroll );
	if ( bCanScroll )
	{
		m_pWepScrollUpBtn->SetEnabled( m_iWeaponScrollOffset > 0 );
		m_pWepScrollDownBtn->SetEnabled( m_iWeaponScrollOffset < m_nWeaponMaxScroll );
	}
}

void CCSInventoryPanel::RebuildGrid()
{
	const InventoryWeaponDef_t *pWeapon = Inventory_GetWeaponDef( m_iSelectedWeaponIndex );
	EInventoryWeaponMode eMode = pWeapon ? pWeapon->eMode : INV_MODE_UNSUPPORTED;
	int nSkinWep = GetActiveSkinWeaponIndex();

	if ( eMode == INV_MODE_UNSUPPORTED )
	{
		m_pGridHeaderLabel->SetText( pWeapon ? pWeapon->pszDisplayName : "WEAPON" );
		m_pBackToSkinsBtn->SetVisible( false );
		m_pPrevPageBtn->SetVisible( false );
		m_pNextPageBtn->SetVisible( false );
		m_pPageLabel->SetVisible( false );

		char szMsg[224];
		LocFormat1UTF8( "#CSInventory_ComingSoon", "%s skins are not available yet.", pWeapon ? pWeapon->pszDisplayName : "Weapon", szMsg, sizeof( szMsg ) );
		m_pUnsupportedLabel->SetText( szMsg );
		m_pUnsupportedLabel->SetVisible( true );

		for ( int i = 0; i < INV_GRID_PAGE_SIZE; ++i )
		{
			m_nTileItemIndices[i] = -1;
			m_pGridTiles[i]->SetVisible( false );
		}
		return;
	}

	m_pUnsupportedLabel->SetVisible( false );
	m_pPrevPageBtn->SetVisible( true );
	m_pNextPageBtn->SetVisible( true );
	m_pPageLabel->SetVisible( true );

	CUtlVector<int> matchingIndices; // -1 represents Stock/None for weapon skins/stickers, or raw catalog index for Knife/Glove/Sleeve

	if ( nSkinWep >= 0 && m_bStickerPickerMode )
	{
		char szSlotNum[16], szHeader[128];
		V_snprintf( szSlotNum, sizeof( szSlotNum ), "%d", m_iActiveStickerSlot + 1 );
		LocFormat1UTF8( "#CSInventory_Header_Stickers", "SELECT STICKER — SLOT %s", szSlotNum, szHeader, sizeof( szHeader ) );
		m_pGridHeaderLabel->SetText( szHeader );
		m_pBackToSkinsBtn->SetText( "#CSInventory_BackToSkins" );
		m_pBackToSkinsBtn->SetVisible( true );

		char szNoneSearch[128], szNoneLoc[64];
		LocUTF8( "#CSInventory_StickerNone", "None (Remove)", szNoneLoc, sizeof( szNoneLoc ) );
		V_snprintf( szNoneSearch, sizeof( szNoneSearch ), "None Remove Empty %s", szNoneLoc );
		if ( ContainsCaseInsensitive( szNoneSearch, m_szSearchFilter ) )
			matchingIndices.AddToTail( -1 );

		int nStickers = Inventory_GetStickerCount();
		for ( int i = 0; i < nStickers; ++i )
		{
			const InventoryStickerDef_t *pStk = Inventory_GetStickerDef( i );
			if ( pStk && ( ContainsCaseInsensitive( pStk->pszDisplayName, m_szSearchFilter ) || ContainsCaseInsensitive( pStk->pszKey, m_szSearchFilter ) ) )
				matchingIndices.AddToTail( i );
		}
	}
	else if ( nSkinWep >= 0 )
	{
		const char *pszWepName = Inventory_GetWeaponSkinDisplayName( nSkinWep );
		if ( nSkinWep == INV_SKIN_WEP_AK47 )
		{
			m_pGridHeaderLabel->SetText( "#CSInventory_Header_AK47Skins" );
		}
		else
		{
			char szHeader[128];
			LocFormat1UTF8( "#CSInventory_Header_WeaponSkins", "%s SKINS", pszWepName, szHeader, sizeof( szHeader ) );
			m_pGridHeaderLabel->SetText( szHeader );
		}
		m_pBackToSkinsBtn->SetVisible( false );

		char szStockFilter[128], szStockLoc[64];
		LocUTF8( "#CSInventory_DefaultStock", "Default (Stock)", szStockLoc, sizeof( szStockLoc ) );
		V_snprintf( szStockFilter, sizeof( szStockFilter ), "Default Stock %s %s", szStockLoc, pszWepName );
		if ( ContainsCaseInsensitive( szStockFilter, m_szSearchFilter ) )
			matchingIndices.AddToTail( -1 );

		int nSkins = Inventory_GetWeaponSkinCount( nSkinWep );
		for ( int i = 0; i < nSkins; ++i )
		{
			const InventorySkinDef_t *pSkin = Inventory_GetWeaponSkinDef( nSkinWep, i );
			if ( pSkin && ( ContainsCaseInsensitive( pSkin->pszDisplayName, m_szSearchFilter ) || ContainsCaseInsensitive( pSkin->pszKey, m_szSearchFilter ) ) )
				matchingIndices.AddToTail( i );
		}
	}
	else if ( eMode == INV_MODE_KNIFE && m_bKnifeSkinPickerMode )
	{
		const InventoryKnifeDef_t *pKnife = Inventory_FindKnifeByKey( m_szPreviewKnife );
		const char *pszKnifeName = pKnife ? pKnife->pszDisplayName : "KNIFE";
		char szHeader[128];
		LocFormat1UTF8( "#CSInventory_Header_WeaponSkins", "%s SKINS", pszKnifeName, szHeader, sizeof( szHeader ) );
		m_pGridHeaderLabel->SetText( szHeader );
		m_pBackToSkinsBtn->SetText( "#CSInventory_BackToKnives" );
		m_pBackToSkinsBtn->SetVisible( true );

		char szStockFilter[128], szVanillaLoc[64];
		LocUTF8( "#CSInventory_DefaultVanilla", "Default (Vanilla)", szVanillaLoc, sizeof( szVanillaLoc ) );
		V_snprintf( szStockFilter, sizeof( szStockFilter ), "Default Vanilla %s %s", szVanillaLoc, pszKnifeName );
		if ( ContainsCaseInsensitive( szStockFilter, m_szSearchFilter ) )
			matchingIndices.AddToTail( -1 );

		int nSkins = pKnife ? pKnife->nSkinCount : 0;
		for ( int i = 0; i < nSkins; ++i )
		{
			const InventorySkinDef_t *pSkin = Inventory_GetKnifeSkinDef( pKnife->pszKey, i );
			if ( pSkin && ( ContainsCaseInsensitive( pSkin->pszDisplayName, m_szSearchFilter ) || ContainsCaseInsensitive( pSkin->pszKey, m_szSearchFilter ) ) )
				matchingIndices.AddToTail( i );
		}
	}
	else if ( eMode == INV_MODE_KNIFE )
	{
		m_pGridHeaderLabel->SetText( "#CSInventory_Header_Knives" );
		m_pBackToSkinsBtn->SetVisible( false );

		int nKnives = Inventory_GetKnifeCount();
		for ( int i = 0; i < nKnives; ++i )
		{
			const InventoryKnifeDef_t *pKnife = Inventory_GetKnifeDef( i );
			if ( pKnife && ( ContainsCaseInsensitive( pKnife->pszDisplayName, m_szSearchFilter ) || ContainsCaseInsensitive( pKnife->pszKey, m_szSearchFilter ) ) )
				matchingIndices.AddToTail( i );
		}
	}
	else if ( eMode == INV_MODE_GLOVE )
	{
		m_pGridHeaderLabel->SetText( "#CSInventory_Header_Gloves" );
		m_pBackToSkinsBtn->SetVisible( false );

		int nGloves = Inventory_GetGloveCount();
		for ( int i = 0; i < nGloves; ++i )
		{
			const InventoryGloveDef_t *pGlove = Inventory_GetGloveDef( i );
			if ( pGlove && ( ContainsCaseInsensitive( pGlove->pszDisplayName, m_szSearchFilter ) || ContainsCaseInsensitive( pGlove->pszId, m_szSearchFilter ) || ContainsCaseInsensitive( pGlove->pszFamilyKey, m_szSearchFilter ) ) )
				matchingIndices.AddToTail( i );
		}
	}
	else if ( eMode == INV_MODE_SLEEVE )
	{
		m_pGridHeaderLabel->SetText( "#CSInventory_Header_Sleeves" );
		m_pBackToSkinsBtn->SetVisible( false );

		int nSleeves = Inventory_GetSleeveCount();
		for ( int i = 0; i < nSleeves; ++i )
		{
			const InventorySleeveDef_t *pSleeve = Inventory_GetSleeveDef( i );
			if ( pSleeve && ( ContainsCaseInsensitive( pSleeve->pszDisplayName, m_szSearchFilter ) || ContainsCaseInsensitive( pSleeve->pszKey, m_szSearchFilter ) ) )
				matchingIndices.AddToTail( i );
		}
	}

	int nTotal = matchingIndices.Count();
	m_iTotalPages = Max( 1, ( nTotal + INV_GRID_PAGE_SIZE - 1 ) / INV_GRID_PAGE_SIZE );
	m_iCurrentPage = clamp( m_iCurrentPage, 0, m_iTotalPages - 1 );

	int startOffset = m_iCurrentPage * INV_GRID_PAGE_SIZE;

	for ( int slot = 0; slot < INV_GRID_PAGE_SIZE; ++slot )
	{
		int listPos = startOffset + slot;
		if ( listPos >= nTotal )
		{
			m_nTileItemIndices[slot] = -1;
			m_bTileIsResetItem[slot] = false;
			m_pGridTiles[slot]->SetVisible( false );
			continue;
		}

		int itemIdx = matchingIndices[listPos];
		m_pGridTiles[slot]->SetVisible( true );

		if ( nSkinWep >= 0 && m_bStickerPickerMode )
		{
			if ( itemIdx < 0 )
			{
				m_bTileIsResetItem[slot] = true;
				m_nTileItemIndices[slot] = 0;
				bool bSelected = ( V_stricmp( m_PreviewWeaponLoadout[nSkinWep].szStickerKeys[m_iActiveStickerSlot], "0" ) == 0 );
				char szNone[64], szPlaceholder[48];
				LocUTF8( "#CSInventory_StickerNone", "None (Remove)", szNone, sizeof( szNone ) );
				LocUTF8( "#CSInventory_NoSticker", "NO STICKER", szPlaceholder, sizeof( szPlaceholder ) );
				m_pGridTiles[slot]->ConfigureTile( szNone, GetRarityColor( RARITY_STOCK ), Color( 255, 255, 255, 255 ), -1, INV_TEAMMASK_NONE, bSelected, true, szPlaceholder );
			}
			else
			{
				m_bTileIsResetItem[slot] = false;
				m_nTileItemIndices[slot] = itemIdx;
				const InventoryStickerDef_t *pStk = Inventory_GetStickerDef( itemIdx );
				int nStkTex = GetOrLoadStickerVGuiTexture( m_StickerTexCache, itemIdx );
				bool bSelected = ( pStk && V_stricmp( m_PreviewWeaponLoadout[nSkinWep].szStickerKeys[m_iActiveStickerSlot], pStk->pszKey ) == 0 );
				m_pGridTiles[slot]->ConfigureTile( pStk ? pStk->pszDisplayName : "", m_Palette.clrAccent, Color( 255, 255, 255, 255 ), nStkTex, INV_TEAMMASK_NONE, bSelected, true );
			}
		}
		else if ( nSkinWep >= 0 )
		{
			char szIconPath[MAX_PATH];
			if ( itemIdx < 0 )
			{
				m_bTileIsResetItem[slot] = true;
				m_nTileItemIndices[slot] = 0;
				Inventory_GetWeaponSkinIconPath( nSkinWep, "0", szIconPath, sizeof( szIconPath ) );
				int nTexID = GetOrLoadPathVGuiTexture( m_IconTexCache, szIconPath );
				int nEqMask = Inventory_GetWeaponSkinEquippedMask( nSkinWep, "0" );
				bool bSelected = ( V_stricmp( m_PreviewWeaponLoadout[nSkinWep].szSkinKey, "0" ) == 0 );
				char szDefaultStock[64];
				LocUTF8( "#CSInventory_DefaultStock", "Default (Stock)", szDefaultStock, sizeof( szDefaultStock ) );
				m_pGridTiles[slot]->ConfigureTile( szDefaultStock, GetRarityColor( RARITY_STOCK ), Color( 255, 255, 255, 255 ), nTexID, nEqMask, bSelected, false );
			}
			else
			{
				m_bTileIsResetItem[slot] = false;
				m_nTileItemIndices[slot] = itemIdx;
				const InventorySkinDef_t *pSkin = Inventory_GetWeaponSkinDef( nSkinWep, itemIdx );
				Inventory_GetWeaponSkinIconPath( nSkinWep, pSkin ? pSkin->pszKey : "0", szIconPath, sizeof( szIconPath ) );
				int nTexID = GetOrLoadPathVGuiTexture( m_IconTexCache, szIconPath );
				Color clrRarity = GetRarityColor( pSkin ? pSkin->eRarity : RARITY_STOCK );
				int nEqMask = pSkin ? Inventory_GetWeaponSkinEquippedMask( nSkinWep, pSkin->pszKey ) : INV_TEAMMASK_NONE;
				bool bSelected = ( pSkin && V_stricmp( m_PreviewWeaponLoadout[nSkinWep].szSkinKey, pSkin->pszKey ) == 0 );
				m_pGridTiles[slot]->ConfigureTile( pSkin ? pSkin->pszDisplayName : "", clrRarity, Color( 255, 255, 255, 255 ), nTexID, nEqMask, bSelected, false );
			}
		}
		else if ( eMode == INV_MODE_KNIFE && m_bKnifeSkinPickerMode )
		{
			const InventoryKnifeDef_t *pKnife = Inventory_FindKnifeByKey( m_szPreviewKnife );
			const char *pszKnifeKey = pKnife ? pKnife->pszKey : "0";
			char szIconPath[MAX_PATH];
			if ( itemIdx < 0 )
			{
				m_bTileIsResetItem[slot] = true;
				m_nTileItemIndices[slot] = 0;
				Inventory_GetKnifeSkinIconPath( pszKnifeKey, "0", szIconPath, sizeof( szIconPath ) );
				int nTexID = GetOrLoadPathVGuiTexture( m_IconTexCache, szIconPath );
				int nEqMask = Inventory_GetKnifeSkinEquippedMask( pszKnifeKey, "0" );
				bool bSelected = ( V_stricmp( m_szPreviewKnifeSkin, "0" ) == 0 );
				char szDefaultVanilla[64];
				LocUTF8( "#CSInventory_DefaultVanilla", "Default (Vanilla)", szDefaultVanilla, sizeof( szDefaultVanilla ) );
				m_pGridTiles[slot]->ConfigureTile( szDefaultVanilla, GetRarityColor( RARITY_STOCK ), Color( 255, 255, 255, 255 ), nTexID, nEqMask, bSelected, false );
			}
			else
			{
				m_bTileIsResetItem[slot] = false;
				m_nTileItemIndices[slot] = itemIdx;
				const InventorySkinDef_t *pSkin = Inventory_GetKnifeSkinDef( pszKnifeKey, itemIdx );
				Inventory_GetKnifeSkinIconPath( pszKnifeKey, pSkin ? pSkin->pszKey : "0", szIconPath, sizeof( szIconPath ) );
				int nTexID = GetOrLoadPathVGuiTexture( m_IconTexCache, szIconPath );
				Color clrRarity = GetRarityColor( pSkin ? pSkin->eRarity : RARITY_STOCK );
				int nEqMask = pSkin ? Inventory_GetKnifeSkinEquippedMask( pszKnifeKey, pSkin->pszKey ) : INV_TEAMMASK_NONE;
				bool bSelected = ( pSkin && V_stricmp( m_szPreviewKnifeSkin, pSkin->pszKey ) == 0 );
				m_pGridTiles[slot]->ConfigureTile( pSkin ? pSkin->pszDisplayName : "", clrRarity, Color( 255, 255, 255, 255 ), nTexID, nEqMask, bSelected, false );
			}
		}
		else if ( eMode == INV_MODE_KNIFE )
		{
			m_bTileIsResetItem[slot] = false;
			m_nTileItemIndices[slot] = itemIdx;
			const InventoryKnifeDef_t *pKnife = Inventory_GetKnifeDef( itemIdx );
			char szIconPath[MAX_PATH];
			const char *pszIconPath = ( pKnife && pKnife->pszIconPath ) ? pKnife->pszIconPath : "vgui/inventory/knives/default";
			Color clrTile = GetRarityColor( pKnife ? pKnife->eRarity : RARITY_STOCK );
			int nEqMask = pKnife ? Inventory_GetKnifeEquippedMask( pKnife->pszKey ) : INV_TEAMMASK_NONE;
			bool bSelected = ( pKnife && !V_stricmp( m_szPreviewKnife, pKnife->pszKey ) );
			if ( pKnife && !V_strcmp( pKnife->pszKey, "0" ) )
			{
				pszIconPath = ( GetPreviewTeam() == INV_TEAM_CT ) ? "vgui/inventory/knives/default_ct" : "vgui/inventory/knives/default_t";
			}
			else if ( pKnife && pKnife->nSkinCount > 0 )
			{
				const char *pszTileSkin = bSelected ? m_szPreviewKnifeSkin : m_szPreviewKnifeSkinByIdx[itemIdx < ARRAYSIZE( m_szPreviewKnifeSkinByIdx ) ? itemIdx : 0];
				if ( const InventorySkinDef_t *pTileSkin = Inventory_FindKnifeSkinByKey( pKnife->pszKey, pszTileSkin ) )
				{
					Inventory_GetKnifeSkinIconPath( pKnife->pszKey, pTileSkin->pszKey, szIconPath, sizeof( szIconPath ) );
					pszIconPath = szIconPath;
					clrTile = GetRarityColor( pTileSkin->eRarity );
				}
			}
			int nTexID = GetOrLoadPathVGuiTexture( m_IconTexCache, pszIconPath );
			m_pGridTiles[slot]->ConfigureTile( pKnife ? pKnife->pszDisplayName : "", clrTile, Color( 255, 255, 255, 255 ), nTexID, nEqMask, bSelected, false );
		}
		else if ( eMode == INV_MODE_GLOVE )
		{
			m_bTileIsResetItem[slot] = false;
			m_nTileItemIndices[slot] = itemIdx;
			const InventoryGloveDef_t *pGlove = Inventory_GetGloveDef( itemIdx );
			int nTexID = GetOrLoadPathVGuiTexture( m_IconTexCache, pGlove ? pGlove->pszIconPath : "" );
			int nEqMask = pGlove ? Inventory_GetGloveEquippedMask( pGlove->pszId ) : INV_TEAMMASK_NONE;
			bool bSelected = ( pGlove && !V_stricmp( m_szPreviewGloveId, pGlove->pszId ) );
			m_pGridTiles[slot]->ConfigureTile( pGlove ? pGlove->pszDisplayName : "", GetRarityColor( pGlove ? pGlove->eRarity : RARITY_STOCK ), Color( 255, 255, 255, 255 ), nTexID, nEqMask, bSelected, false );
		}
		else if ( eMode == INV_MODE_SLEEVE )
		{
			m_bTileIsResetItem[slot] = false;
			m_nTileItemIndices[slot] = itemIdx;
			const InventorySleeveDef_t *pSleeve = Inventory_GetSleeveDef( itemIdx );
			int nTexID = GetOrLoadPathVGuiTexture( m_IconTexCache, pSleeve ? pSleeve->pszIconPath : "" );
			int nEqMask = pSleeve ? Inventory_GetSleeveEquippedMask( pSleeve->pszKey ) : INV_TEAMMASK_NONE;
			bool bSelected = ( pSleeve && !V_stricmp( m_szPreviewSleeveKey, pSleeve->pszKey ) );
			m_pGridTiles[slot]->ConfigureTile( pSleeve ? pSleeve->pszDisplayName : "", GetRarityColor( pSleeve ? pSleeve->eRarity : RARITY_STOCK ), Color( 255, 255, 255, 255 ), nTexID, nEqMask, bSelected, false );
		}
	}

	char szCur[16], szTot[16], szPage[64];
	V_snprintf( szCur, sizeof( szCur ), "%d", m_iCurrentPage + 1 );
	V_snprintf( szTot, sizeof( szTot ), "%d", m_iTotalPages );
	LocFormat2UTF8( "#CSInventory_PageFormat", "Page %s / %s", szCur, szTot, szPage, sizeof( szPage ) );
	m_pPageLabel->SetText( szPage );
	m_pPrevPageBtn->SetEnabled( m_iCurrentPage > 0 );
	m_pNextPageBtn->SetEnabled( m_iCurrentPage + 1 < m_iTotalPages );
}

static int FindStickerCatalogIndex( const InventoryStickerDef_t *pStk )
{
	if ( !pStk )
		return -1;
	int nStickers = Inventory_GetStickerCount();
	for ( int i = 0; i < nStickers; ++i )
	{
		if ( Inventory_GetStickerDef( i ) == pStk )
			return i;
	}
	return -1;
}

void CCSInventoryPanel::RefreshInspectAndStickerControls()
{
	const InventoryWeaponDef_t *pWeapon = Inventory_GetWeaponDef( m_iSelectedWeaponIndex );
	EInventoryWeaponMode eMode = pWeapon ? pWeapon->eMode : INV_MODE_UNSUPPORTED;
	int nSkinWep = GetActiveSkinWeaponIndex();
	bool bSupported = ( eMode != INV_MODE_UNSUPPORTED );
	bool bShowStickers = ( nSkinWep >= 0 );
	bool bShowKnifeSlot = ( eMode == INV_MODE_KNIFE );
	bool bShowBottomArea = ( bShowStickers || bShowKnifeSlot );

	m_pEquipCTBtn->SetEnabled( bSupported );
	m_pEquipTBtn->SetEnabled( bSupported );
	m_pEquipBothBtn->SetEnabled( bSupported );

	m_pStickerHeaderLabel->SetVisible( bShowBottomArea );
	m_pFillAllStickersBtn->SetVisible( bShowBottomArea );
	m_pClearAllStickersBtn->SetVisible( bShowBottomArea );
	if ( bShowStickers )
	{
		m_pStickerHeaderLabel->SetText( "#CSInventory_StickerHeader" );
		m_pFillAllStickersBtn->SetText( "#CSInventory_FillAll4" );
		m_pClearAllStickersBtn->SetText( "#CSInventory_ClearAll" );
		m_pFillAllStickersBtn->SetEnabled( true );
		m_pClearAllStickersBtn->SetEnabled( true );
	}
	for ( int s = 0; s < INV_MAX_STICKER_SLOTS; ++s )
	{
		bool bSlotVis = bShowStickers || ( bShowKnifeSlot && s == 0 );
		m_pStickerSlotBtns[s]->SetVisible( bSlotVis );
		if ( !bSlotVis )
		{
			m_pStickerClearBtns[s]->SetVisible( false );
		}
	}

	if ( !bSupported )
	{
		m_pInspectWeaponLabel->SetText( pWeapon ? pWeapon->pszDisplayName : "WEAPON" );
		m_pInspectSkinLabel->SetText( "#CSInventory_InspectComingSoon" );
		m_pInspectSkinLabel->SetFgColor( m_Palette.clrTextMuted );
		return;
	}

	if ( nSkinWep >= 0 )
	{
		const char *pszWepName = Inventory_GetWeaponSkinDisplayName( nSkinWep );
		const InventoryWeaponLoadout_t &loadout = m_PreviewWeaponLoadout[nSkinWep];
		const InventorySkinDef_t *pSkin = Inventory_FindWeaponSkinByKey( nSkinWep, loadout.szSkinKey );
		if ( pSkin )
		{
			char szRarity[48], szSub[128];
			GetLocalizedRarityLabel( pSkin->eRarity, szRarity, sizeof( szRarity ) );
			V_snprintf( szSub, sizeof( szSub ), "%s  |  %s", pszWepName, szRarity );
			m_pInspectWeaponLabel->SetText( szSub );
			m_pInspectSkinLabel->SetText( pSkin->pszDisplayName );
			m_pInspectSkinLabel->SetFgColor( GetRarityColor( pSkin->eRarity ) );
		}
		else
		{
			char szRarity[48], szSub[128];
			GetLocalizedRarityLabel( RARITY_STOCK, szRarity, sizeof( szRarity ) );
			V_snprintf( szSub, sizeof( szSub ), "%s  |  %s", pszWepName, szRarity );
			m_pInspectWeaponLabel->SetText( szSub );
			m_pInspectSkinLabel->SetText( "#CSInventory_DefaultStock" );
			m_pInspectSkinLabel->SetFgColor( m_Palette.clrTextPrimary );
		}

		for ( int s = 0; s < INV_MAX_STICKER_SLOTS; ++s )
		{
			m_pStickerSlotBtns[s]->SetEnabled( true );
			bool bSlotActive = ( m_bStickerPickerMode && m_iActiveStickerSlot == s );
			const InventoryStickerDef_t *pStk = Inventory_FindStickerByKey( loadout.szStickerKeys[s] );
			if ( pStk )
			{
				int nStkIdx = FindStickerCatalogIndex( pStk );
				int nTexID = ( nStkIdx >= 0 ) ? GetOrLoadStickerVGuiTexture( m_StickerTexCache, nStkIdx ) : -1;
				m_pStickerSlotBtns[s]->SetSlotData( nTexID, pStk->pszDisplayName, bSlotActive );
				m_pStickerClearBtns[s]->SetVisible( true );
				m_pStickerClearBtns[s]->MoveToFront();
				m_pStickerClearBtns[s]->Repaint();
			}
			else
			{
				m_pStickerSlotBtns[s]->SetSlotData( -1, "", bSlotActive );
				m_pStickerClearBtns[s]->SetVisible( false );
			}
		}
	}
	else if ( eMode == INV_MODE_KNIFE )
	{
		const InventoryKnifeDef_t *pKnife = Inventory_FindKnifeByKey( m_szPreviewKnife );
		const InventorySkinDef_t *pKnifeSkin = ( pKnife && pKnife->nSkinCount > 0 ) ? Inventory_FindKnifeSkinByKey( pKnife->pszKey, m_szPreviewKnifeSkin ) : NULL;
		if ( pKnife && pKnifeSkin )
		{
			char szRarity[48], szSub[128];
			GetLocalizedRarityLabel( pKnifeSkin->eRarity, szRarity, sizeof( szRarity ) );
			V_snprintf( szSub, sizeof( szSub ), "%s  |  %s", pKnife->pszDisplayName, szRarity );
			m_pInspectWeaponLabel->SetText( szSub );
			m_pInspectSkinLabel->SetText( pKnifeSkin->pszDisplayName );
			m_pInspectSkinLabel->SetFgColor( GetRarityColor( pKnifeSkin->eRarity ) );
		}
		else
		{
			char szCat[48], szRarity[48], szSub[128], szDefaultKnife[64];
			LocUTF8( "#CSInventory_Header_Knives", "KNIVES", szCat, sizeof( szCat ) );
			GetLocalizedRarityLabel( pKnife ? pKnife->eRarity : RARITY_STOCK, szRarity, sizeof( szRarity ) );
			LocUTF8( "#CSInventory_DefaultKnife", "Default Knife", szDefaultKnife, sizeof( szDefaultKnife ) );
			V_snprintf( szSub, sizeof( szSub ), "%s  |  %s", szCat, szRarity );
			m_pInspectWeaponLabel->SetText( szSub );
			m_pInspectSkinLabel->SetText( pKnife ? pKnife->pszDisplayName : szDefaultKnife );
			m_pInspectSkinLabel->SetFgColor( pKnife && !V_strcmp( pKnife->pszKey, "0" ) ? m_Palette.clrTextPrimary : GetRarityColor( pKnife ? pKnife->eRarity : RARITY_GOLD ) );
		}

		m_pStickerHeaderLabel->SetText( "#CSInventory_KnifeSkinHeader" );
		bool bHasSkins = ( pKnife && pKnife->nSkinCount > 0 );
		m_pStickerSlotBtns[0]->SetEnabled( bHasSkins );
		m_pFillAllStickersBtn->SetText( m_bKnifeSkinPickerMode ? "#CSInventory_BackToKnives" : "#CSInventory_KnifeBrowseSkins" );
		m_pFillAllStickersBtn->SetEnabled( bHasSkins );
		m_pClearAllStickersBtn->SetText( "#CSInventory_KnifeResetVanilla" );
		m_pClearAllStickersBtn->SetEnabled( bHasSkins && pKnifeSkin != NULL );

		if ( bHasSkins )
		{
			char szSlotIcon[MAX_PATH], szDefaultVanilla[64];
			LocUTF8( "#CSInventory_DefaultVanilla", "Default (Vanilla)", szDefaultVanilla, sizeof( szDefaultVanilla ) );
			Inventory_GetKnifeSkinIconPath( pKnife->pszKey, pKnifeSkin ? pKnifeSkin->pszKey : "0", szSlotIcon, sizeof( szSlotIcon ) );
			int nSlotTex = GetOrLoadPathVGuiTexture( m_IconTexCache, szSlotIcon );
			m_pStickerSlotBtns[0]->SetSlotData( nSlotTex, pKnifeSkin ? pKnifeSkin->pszDisplayName : "", m_bKnifeSkinPickerMode, szDefaultVanilla );
			if ( pKnifeSkin )
			{
				m_pStickerClearBtns[0]->SetVisible( true );
				m_pStickerClearBtns[0]->MoveToFront();
				m_pStickerClearBtns[0]->Repaint();
			}
			else
			{
				m_pStickerClearBtns[0]->SetVisible( false );
			}
		}
		else
		{
			char szNoSkins[64];
			LocUTF8( "#CSInventory_DefaultNoSkins", "Default (No Skins)", szNoSkins, sizeof( szNoSkins ) );
			m_pStickerSlotBtns[0]->SetSlotData( -1, "", false, szNoSkins );
			m_pStickerClearBtns[0]->SetVisible( false );
		}
	}
	else if ( eMode == INV_MODE_GLOVE )
	{
		const InventoryGloveDef_t *pGlove = Inventory_FindGloveById( m_szPreviewGloveId );
		char szCat[48], szRarity[48], szSub[128], szDefaultGloves[64];
		LocUTF8( "#CSInventory_Header_Gloves", "GLOVES", szCat, sizeof( szCat ) );
		GetLocalizedRarityLabel( pGlove ? pGlove->eRarity : RARITY_STOCK, szRarity, sizeof( szRarity ) );
		LocUTF8( "#CSInventory_DefaultGloves", "Default Gloves", szDefaultGloves, sizeof( szDefaultGloves ) );
		V_snprintf( szSub, sizeof( szSub ), "%s  |  %s", szCat, szRarity );
		m_pInspectWeaponLabel->SetText( szSub );
		m_pInspectSkinLabel->SetText( pGlove ? pGlove->pszDisplayName : szDefaultGloves );
		m_pInspectSkinLabel->SetFgColor( pGlove && !V_strcmp( pGlove->pszId, "default" ) ? m_Palette.clrTextPrimary : GetRarityColor( pGlove ? pGlove->eRarity : RARITY_COVERT ) );
	}
	else if ( eMode == INV_MODE_SLEEVE )
	{
		const InventorySleeveDef_t *pSleeve = Inventory_FindSleeveByKey( m_szPreviewSleeveKey );
		char szCat[48], szRarity[48], szSub[128], szDefaultSleeve[64];
		LocUTF8( "#CSInventory_Header_Sleeves", "SLEEVES", szCat, sizeof( szCat ) );
		GetLocalizedRarityLabel( pSleeve ? pSleeve->eRarity : RARITY_STOCK, szRarity, sizeof( szRarity ) );
		LocUTF8( "#CSInventory_DefaultSleeve", "Default Sleeve", szDefaultSleeve, sizeof( szDefaultSleeve ) );
		V_snprintf( szSub, sizeof( szSub ), "%s  |  %s", szCat, szRarity );
		m_pInspectWeaponLabel->SetText( szSub );
		m_pInspectSkinLabel->SetText( pSleeve ? pSleeve->pszDisplayName : szDefaultSleeve );
		m_pInspectSkinLabel->SetFgColor( pSleeve && !V_strcmp( pSleeve->pszKey, "-1" ) ? m_Palette.clrTextPrimary : GetRarityColor( pSleeve ? pSleeve->eRarity : RARITY_RESTRICTED ) );
	}
}

void CCSInventoryPanel::RefreshPreviewPanel()
{
	if ( !m_pPreviewCard )
		return;

	const InventoryWeaponDef_t *pWeapon = Inventory_GetWeaponDef( m_iSelectedWeaponIndex );
	EInventoryWeaponMode eMode = pWeapon ? pWeapon->eMode : INV_MODE_UNSUPPORTED;
	int nSkinWep = GetActiveSkinWeaponIndex();

	int nStickerTexIDs[INV_MAX_STICKER_SLOTS] = { -1, -1, -1, -1 };
	int nMainTexID = -1;
	bool bSquareIcon = false;
	bool bPadded43 = true;
	Color clrStripe = m_Palette.clrAccent;
	const char *pszPlaceholder = pWeapon ? pWeapon->pszDisplayName : "";

	if ( nSkinWep >= 0 )
	{
		const InventoryWeaponLoadout_t &loadout = m_PreviewWeaponLoadout[nSkinWep];
		char szIconPath[MAX_PATH];
		Inventory_GetWeaponSkinIconPath( nSkinWep, loadout.szSkinKey, szIconPath, sizeof( szIconPath ) );
		nMainTexID = GetOrLoadPathVGuiTexture( m_IconTexCache, szIconPath );

		const InventorySkinDef_t *pSkin = Inventory_FindWeaponSkinByKey( nSkinWep, loadout.szSkinKey );
		clrStripe = GetRarityColor( pSkin ? pSkin->eRarity : RARITY_STOCK );

		const char *pszStkPtrs[INV_MAX_STICKER_SLOTS] =
		{
			loadout.szStickerKeys[0],
			loadout.szStickerKeys[1],
			loadout.szStickerKeys[2],
			loadout.szStickerKeys[3],
		};
		Inventory_SetPreviewSelection( nSkinWep, loadout.szSkinKey, pszStkPtrs );

		for ( int s = 0; s < INV_MAX_STICKER_SLOTS; ++s )
		{
			int nStkIdx = FindStickerCatalogIndex( Inventory_FindStickerByKey( loadout.szStickerKeys[s] ) );
			if ( nStkIdx >= 0 )
			{
				nStickerTexIDs[s] = GetOrLoadStickerVGuiTexture( m_StickerTexCache, nStkIdx );
			}
		}
	}
	else if ( eMode == INV_MODE_KNIFE )
	{
		const InventoryKnifeDef_t *pKnife = Inventory_FindKnifeByKey( m_szPreviewKnife );
		const InventorySkinDef_t *pKnifeSkin = ( pKnife && pKnife->nSkinCount > 0 ) ? Inventory_FindKnifeSkinByKey( pKnife->pszKey, m_szPreviewKnifeSkin ) : NULL;
		char szSkinIcon[MAX_PATH];
		const char *pszIconPath = ( pKnife && pKnife->pszIconPath ) ? pKnife->pszIconPath : "vgui/inventory/knives/default_ct";
		if ( pKnife && !V_strcmp( pKnife->pszKey, "0" ) )
		{
			pszIconPath = ( GetPreviewTeam() == INV_TEAM_CT ) ? "vgui/inventory/knives/default_ct" : "vgui/inventory/knives/default_t";
		}
		else if ( pKnife && pKnifeSkin )
		{
			Inventory_GetKnifeSkinIconPath( pKnife->pszKey, pKnifeSkin->pszKey, szSkinIcon, sizeof( szSkinIcon ) );
			pszIconPath = szSkinIcon;
		}
		Inventory_SetPreviewKnife( pKnife ? pKnife->pszKey : "0", pKnifeSkin ? pKnifeSkin->pszKey : "0" );
		nMainTexID = GetOrLoadPathVGuiTexture( m_IconTexCache, pszIconPath );
		clrStripe = GetRarityColor( pKnifeSkin ? pKnifeSkin->eRarity : ( pKnife ? pKnife->eRarity : RARITY_STOCK ) );
	}
	else if ( eMode == INV_MODE_GLOVE )
	{
		const InventoryGloveDef_t *pGlove = Inventory_FindGloveById( m_szPreviewGloveId );
		nMainTexID = GetOrLoadPathVGuiTexture( m_IconTexCache, pGlove ? pGlove->pszIconPath : "" );
		clrStripe = GetRarityColor( pGlove ? pGlove->eRarity : RARITY_STOCK );
	}
	else if ( eMode == INV_MODE_SLEEVE )
	{
		const InventorySleeveDef_t *pSleeve = Inventory_FindSleeveByKey( m_szPreviewSleeveKey );
		nMainTexID = GetOrLoadPathVGuiTexture( m_IconTexCache, pSleeve ? pSleeve->pszIconPath : "" );
		clrStripe = GetRarityColor( pSleeve ? pSleeve->eRarity : RARITY_STOCK );
	}
	else if ( pWeapon && pWeapon->pszIconMaterial && pWeapon->pszIconMaterial[0] )
	{
		nMainTexID = GetOrLoadPathVGuiTexture( m_IconTexCache, pWeapon->pszIconMaterial );
		bPadded43 = false;
		clrStripe = m_Palette.clrTextMuted;
	}

	m_pPreviewCard->SetPreviewItem( nMainTexID, bSquareIcon, bPadded43, clrStripe, pszPlaceholder, nStickerTexIDs );
}

void CCSInventoryPanel::SetStatusBanner( const char *pszText, Color clr )
{
	m_pStatusLabel->SetText( pszText ? pszText : "" );
	m_pStatusLabel->SetFgColor( clr );
}

void CCSInventoryPanel::OnSearchTextChanged()
{
	m_pSearchEntry->GetText( m_szSearchFilter, sizeof( m_szSearchFilter ) );
	m_iCurrentPage = 0;
	RebuildGrid();
}

void CCSInventoryPanel::OnKeyCodePressed( vgui::KeyCode code )
{
	if ( code == KEY_ESCAPE )
	{
		CloseInventory();
		return;
	}
	BaseClass::OnKeyCodePressed( code );
}

void CCSInventoryPanel::OnMouseWheeled( int delta )
{
	if ( delta == 0 )
		return;

	int mx = 0, my = 0;
	vgui::input()->GetCursorPos( mx, my );
	ScreenToLocal( mx, my );
	if ( mx >= m_nLeftColX && mx <= m_nLeftColX + m_nLeftColW && m_nWeaponMaxScroll > 0 )
	{
		int nNewOffset = clamp( m_iWeaponScrollOffset + ( delta > 0 ? -1 : 1 ), 0, m_nWeaponMaxScroll );
		if ( nNewOffset != m_iWeaponScrollOffset )
		{
			m_iWeaponScrollOffset = nNewOffset;
			RebuildWeaponList();
		}
		return;
	}

	if ( delta > 0 && m_iCurrentPage > 0 )
	{
		--m_iCurrentPage;
		RebuildGrid();
	}
	else if ( delta < 0 && m_iCurrentPage + 1 < m_iTotalPages )
	{
		++m_iCurrentPage;
		RebuildGrid();
	}
}

void CCSInventoryPanel::OnCommand( const char *command )
{
	if ( !command || !command[0] )
		return;

	if ( !V_stricmp( command, "close_inventory" ) )
	{
		CloseInventory();
		return;
	}
	if ( !V_stricmp( command, "filter_all" ) )
	{
		m_nTeamFilter = INV_TEAMMASK_BOTH;
		m_iWeaponScrollOffset = 0;
		RebuildWeaponList();
		RebuildGrid();
		RefreshPreviewPanel();
		return;
	}
	if ( !V_stricmp( command, "filter_ct" ) )
	{
		m_nTeamFilter = INV_TEAMMASK_CT;
		m_iWeaponScrollOffset = 0;
		LoadEquippedStateForActiveTeam();
		if ( m_bKnifeSkinPickerMode && Inventory_GetKnifeSkinCount( m_szPreviewKnife ) == 0 )
			m_bKnifeSkinPickerMode = false;
		RebuildWeaponList();
		RebuildGrid();
		RefreshInspectAndStickerControls();
		RefreshPreviewPanel();
		return;
	}
	if ( !V_stricmp( command, "filter_t" ) )
	{
		m_nTeamFilter = INV_TEAMMASK_T;
		m_iWeaponScrollOffset = 0;
		LoadEquippedStateForActiveTeam();
		if ( m_bKnifeSkinPickerMode && Inventory_GetKnifeSkinCount( m_szPreviewKnife ) == 0 )
			m_bKnifeSkinPickerMode = false;
		RebuildWeaponList();
		RebuildGrid();
		RefreshInspectAndStickerControls();
		RefreshPreviewPanel();
		return;
	}
	if ( !V_stricmp( command, "wep_scroll_up" ) )
	{
		if ( m_iWeaponScrollOffset > 0 )
		{
			--m_iWeaponScrollOffset;
			RebuildWeaponList();
		}
		return;
	}
	if ( !V_stricmp( command, "wep_scroll_down" ) )
	{
		if ( m_iWeaponScrollOffset < m_nWeaponMaxScroll )
		{
			++m_iWeaponScrollOffset;
			RebuildWeaponList();
		}
		return;
	}
	if ( !V_stricmp( command, "clear_search" ) )
	{
		m_pSearchEntry->SetText( "" );
		m_szSearchFilter[0] = '\0';
		m_iCurrentPage = 0;
		RebuildGrid();
		return;
	}
	if ( !V_strnicmp( command, "cat_", 4 ) )
	{
		int cat = clamp( atoi( command + 4 ), 0, (int)INV_CAT_COUNT - 1 );
		m_eCategoryFilter = (EInventoryCategory)cat;
		m_iWeaponScrollOffset = 0;
		// Auto-select the first item in Gloves/Sleeves/Knives category for quick 1-tap switching!
		for ( int i = 0; i < Inventory_GetWeaponCount(); ++i )
		{
			const InventoryWeaponDef_t *pDef = Inventory_GetWeaponDef( i );
			if ( pDef && ( m_eCategoryFilter == INV_CAT_ALL || pDef->eCategory == m_eCategoryFilter ) && pDef->bSupported )
			{
				if ( m_eCategoryFilter != INV_CAT_ALL )
				{
					m_iSelectedWeaponIndex = i;
					m_bStickerPickerMode = false;
					m_bKnifeSkinPickerMode = false;
					m_iCurrentPage = 0;
				}
				break;
			}
		}
		InvalidateLayout( false );
		RebuildWeaponList();
		RebuildGrid();
		RefreshInspectAndStickerControls();
		RefreshPreviewPanel();
		return;
	}
	if ( !V_strnicmp( command, "select_weapon_", 14 ) )
	{
		int btnSlot = atoi( command + 14 );
		if ( btnSlot >= 0 && btnSlot < INV_MAX_WEAPON_BUTTONS && m_nVisibleWeaponIndices[btnSlot] >= 0 )
		{
			m_iSelectedWeaponIndex = m_nVisibleWeaponIndices[btnSlot];
			m_bStickerPickerMode = false;
			m_bKnifeSkinPickerMode = false;
			m_iCurrentPage = 0;
			InvalidateLayout( false );
			RebuildWeaponList();
			RebuildGrid();
			RefreshInspectAndStickerControls();
			RefreshPreviewPanel();
		}
		return;
	}
	if ( !V_stricmp( command, "back_to_skins" ) )
	{
		m_bStickerPickerMode = false;
		m_bKnifeSkinPickerMode = false;
		m_iCurrentPage = 0;
		RebuildGrid();
		RefreshInspectAndStickerControls();
		return;
	}
	if ( !V_stricmp( command, "prev_page" ) )
	{
		if ( m_iCurrentPage > 0 )
		{
			--m_iCurrentPage;
			RebuildGrid();
		}
		return;
	}
	if ( !V_stricmp( command, "next_page" ) )
	{
		if ( m_iCurrentPage + 1 < m_iTotalPages )
		{
			++m_iCurrentPage;
			RebuildGrid();
		}
		return;
	}
	if ( !V_strnicmp( command, "sticker_slot_", 13 ) )
	{
		if ( GetActiveMode() == INV_MODE_KNIFE )
		{
			const InventoryKnifeDef_t *pKnife = Inventory_FindKnifeByKey( m_szPreviewKnife );
			if ( pKnife && pKnife->nSkinCount > 0 )
			{
				m_bKnifeSkinPickerMode = !m_bKnifeSkinPickerMode;
				m_iCurrentPage = 0;
				RebuildGrid();
				RefreshInspectAndStickerControls();
			}
			return;
		}
		int slot = clamp( atoi( command + 13 ), 0, INV_MAX_STICKER_SLOTS - 1 );
		m_iActiveStickerSlot = slot;
		m_bStickerPickerMode = true;
		m_iCurrentPage = 0;
		RebuildGrid();
		RefreshInspectAndStickerControls();
		char szNum[16], szHint[128];
		V_snprintf( szNum, sizeof( szNum ), "%d", slot + 1 );
		LocFormat1UTF8( "#CSInventory_Status_PickSticker", "Picking sticker for Slot #%s", szNum, szHint, sizeof( szHint ) );
		SetStatusBanner( szHint, m_Palette.clrAccent );
		return;
	}
	if ( !V_strnicmp( command, "sticker_clear_", 14 ) )
	{
		if ( GetActiveMode() == INV_MODE_KNIFE )
		{
			V_strncpy( m_szPreviewKnifeSkin, "0", sizeof( m_szPreviewKnifeSkin ) );
			int nKnifeIdx = Inventory_FindKnifeIndexByKey( m_szPreviewKnife );
			if ( nKnifeIdx >= 0 && nKnifeIdx < ARRAYSIZE( m_szPreviewKnifeSkinByIdx ) )
				V_strncpy( m_szPreviewKnifeSkinByIdx[nKnifeIdx], "0", sizeof( m_szPreviewKnifeSkinByIdx[nKnifeIdx] ) );
			RefreshInspectAndStickerControls();
			RefreshPreviewPanel();
			RebuildGrid();
			char szMsg[128];
			LocUTF8( "#CSInventory_Status_KnifeSkinCleared", "Knife skin reset to Vanilla (press Equip to save)", szMsg, sizeof( szMsg ) );
			SetStatusBanner( szMsg, m_Palette.clrTextSecondary );
			return;
		}
		int nSkinWep = GetActiveSkinWeaponIndex();
		if ( nSkinWep >= 0 )
		{
			int slot = clamp( atoi( command + 14 ), 0, INV_MAX_STICKER_SLOTS - 1 );
			V_strncpy( m_PreviewWeaponLoadout[nSkinWep].szStickerKeys[slot], "0", sizeof( m_PreviewWeaponLoadout[nSkinWep].szStickerKeys[slot] ) );
			RefreshInspectAndStickerControls();
			RefreshPreviewPanel();
			RebuildGrid();
			char szNum[16], szMsg[128];
			V_snprintf( szNum, sizeof( szNum ), "%d", slot + 1 );
			LocFormat1UTF8( "#CSInventory_Status_SlotCleared", "Sticker Slot #%s cleared (press Equip to save)", szNum, szMsg, sizeof( szMsg ) );
			SetStatusBanner( szMsg, m_Palette.clrTextSecondary );
		}
		return;
	}
	if ( !V_stricmp( command, "stickers_fill_all" ) )
	{
		if ( GetActiveMode() == INV_MODE_KNIFE )
		{
			const InventoryKnifeDef_t *pKnife = Inventory_FindKnifeByKey( m_szPreviewKnife );
			if ( pKnife && pKnife->nSkinCount > 0 )
			{
				m_bKnifeSkinPickerMode = !m_bKnifeSkinPickerMode;
				m_iCurrentPage = 0;
				RebuildGrid();
				RefreshInspectAndStickerControls();
			}
			return;
		}
		int nSkinWep = GetActiveSkinWeaponIndex();
		if ( nSkinWep >= 0 )
		{
			InventoryWeaponLoadout_t &loadout = m_PreviewWeaponLoadout[nSkinWep];
			const char *pszToFill = "0";
			if ( V_stricmp( loadout.szStickerKeys[m_iActiveStickerSlot], "0" ) != 0 )
			{
				pszToFill = loadout.szStickerKeys[m_iActiveStickerSlot];
			}
			else if ( V_stricmp( m_szLastPickedSticker, "0" ) != 0 )
			{
				pszToFill = m_szLastPickedSticker;
			}
			for ( int s = 0; s < INV_MAX_STICKER_SLOTS; ++s )
			{
				V_strncpy( loadout.szStickerKeys[s], pszToFill, sizeof( loadout.szStickerKeys[s] ) );
			}
			RefreshInspectAndStickerControls();
			RefreshPreviewPanel();
			if ( m_bStickerPickerMode )
				RebuildGrid();
			char szMsg[128];
			LocUTF8( "#CSInventory_Status_Filled4", "Applied sticker to all 4 slots (press Equip to save)", szMsg, sizeof( szMsg ) );
			SetStatusBanner( szMsg, m_Palette.clrAccent );
		}
		return;
	}
	if ( !V_stricmp( command, "stickers_clear_all" ) )
	{
		if ( GetActiveMode() == INV_MODE_KNIFE )
		{
			V_strncpy( m_szPreviewKnifeSkin, "0", sizeof( m_szPreviewKnifeSkin ) );
			int nKnifeIdx = Inventory_FindKnifeIndexByKey( m_szPreviewKnife );
			if ( nKnifeIdx >= 0 && nKnifeIdx < ARRAYSIZE( m_szPreviewKnifeSkinByIdx ) )
				V_strncpy( m_szPreviewKnifeSkinByIdx[nKnifeIdx], "0", sizeof( m_szPreviewKnifeSkinByIdx[nKnifeIdx] ) );
			RefreshInspectAndStickerControls();
			RefreshPreviewPanel();
			RebuildGrid();
			char szMsg[128];
			LocUTF8( "#CSInventory_Status_KnifeSkinCleared", "Knife skin reset to Vanilla (press Equip to save)", szMsg, sizeof( szMsg ) );
			SetStatusBanner( szMsg, m_Palette.clrTextSecondary );
			return;
		}
		int nSkinWep = GetActiveSkinWeaponIndex();
		if ( nSkinWep >= 0 )
		{
			for ( int s = 0; s < INV_MAX_STICKER_SLOTS; ++s )
			{
				V_strncpy( m_PreviewWeaponLoadout[nSkinWep].szStickerKeys[s], "0", sizeof( m_PreviewWeaponLoadout[nSkinWep].szStickerKeys[s] ) );
			}
			RefreshInspectAndStickerControls();
			RefreshPreviewPanel();
			if ( m_bStickerPickerMode )
				RebuildGrid();
			char szMsg[128];
			LocUTF8( "#CSInventory_Status_ClearedAll", "Cleared all 4 sticker slots (press Equip to save)", szMsg, sizeof( szMsg ) );
			SetStatusBanner( szMsg, m_Palette.clrTextSecondary );
		}
		return;
	}
	if ( !V_strnicmp( command, "grid_tile_", 10 ) )
	{
		int tile = atoi( command + 10 );
		if ( tile < 0 || tile >= INV_GRID_PAGE_SIZE )
			return;

		int itemIdx = m_nTileItemIndices[tile];
		EInventoryWeaponMode eMode = GetActiveMode();
		int nSkinWep = GetActiveSkinWeaponIndex();

		if ( nSkinWep >= 0 && m_bStickerPickerMode )
		{
			InventoryWeaponLoadout_t &loadout = m_PreviewWeaponLoadout[nSkinWep];
			if ( m_bTileIsResetItem[tile] )
			{
				V_strncpy( loadout.szStickerKeys[m_iActiveStickerSlot], "0", sizeof( loadout.szStickerKeys[m_iActiveStickerSlot] ) );
			}
			else if ( const InventoryStickerDef_t *pStk = Inventory_GetStickerDef( itemIdx ) )
			{
				V_strncpy( loadout.szStickerKeys[m_iActiveStickerSlot], pStk->pszKey, sizeof( loadout.szStickerKeys[m_iActiveStickerSlot] ) );
				V_strncpy( m_szLastPickedSticker, pStk->pszKey, sizeof( m_szLastPickedSticker ) );
			}
		}
		else if ( nSkinWep >= 0 )
		{
			InventoryWeaponLoadout_t &loadout = m_PreviewWeaponLoadout[nSkinWep];
			if ( m_bTileIsResetItem[tile] )
			{
				V_strncpy( loadout.szSkinKey, "0", sizeof( loadout.szSkinKey ) );
			}
			else if ( const InventorySkinDef_t *pSkin = Inventory_GetWeaponSkinDef( nSkinWep, itemIdx ) )
			{
				V_strncpy( loadout.szSkinKey, pSkin->pszKey, sizeof( loadout.szSkinKey ) );
			}
		}
		else if ( eMode == INV_MODE_KNIFE && m_bKnifeSkinPickerMode )
		{
			int nKnifeIdx = Inventory_FindKnifeIndexByKey( m_szPreviewKnife );
			if ( m_bTileIsResetItem[tile] )
			{
				V_strncpy( m_szPreviewKnifeSkin, "0", sizeof( m_szPreviewKnifeSkin ) );
			}
			else if ( const InventorySkinDef_t *pSkin = Inventory_GetKnifeSkinDef( m_szPreviewKnife, itemIdx ) )
			{
				V_strncpy( m_szPreviewKnifeSkin, pSkin->pszKey, sizeof( m_szPreviewKnifeSkin ) );
			}
			if ( nKnifeIdx >= 0 && nKnifeIdx < ARRAYSIZE( m_szPreviewKnifeSkinByIdx ) )
			{
				V_strncpy( m_szPreviewKnifeSkinByIdx[nKnifeIdx], m_szPreviewKnifeSkin, sizeof( m_szPreviewKnifeSkinByIdx[nKnifeIdx] ) );
			}
		}
		else if ( eMode == INV_MODE_KNIFE )
		{
			if ( const InventoryKnifeDef_t *pKnife = Inventory_GetKnifeDef( itemIdx ) )
			{
				V_strncpy( m_szPreviewKnife, pKnife->pszKey, sizeof( m_szPreviewKnife ) );
				if ( pKnife->nSkinCount > 0 && itemIdx >= 0 && itemIdx < ARRAYSIZE( m_szPreviewKnifeSkinByIdx ) )
					V_strncpy( m_szPreviewKnifeSkin, m_szPreviewKnifeSkinByIdx[itemIdx], sizeof( m_szPreviewKnifeSkin ) );
				else
					V_strncpy( m_szPreviewKnifeSkin, "0", sizeof( m_szPreviewKnifeSkin ) );
			}
		}
		else if ( eMode == INV_MODE_GLOVE )
		{
			if ( const InventoryGloveDef_t *pGlove = Inventory_GetGloveDef( itemIdx ) )
			{
				V_strncpy( m_szPreviewGloveId, pGlove->pszId, sizeof( m_szPreviewGloveId ) );
			}
		}
		else if ( eMode == INV_MODE_SLEEVE )
		{
			if ( const InventorySleeveDef_t *pSleeve = Inventory_GetSleeveDef( itemIdx ) )
			{
				V_strncpy( m_szPreviewSleeveKey, pSleeve->pszKey, sizeof( m_szPreviewSleeveKey ) );
			}
		}

		RefreshPreviewPanel();
		RefreshInspectAndStickerControls();
		RebuildGrid();
		return;
	}

	int nEquipMask = INV_TEAMMASK_NONE;
	char szTeamLabel[48] = "";
	if ( !V_stricmp( command, "equip_ct" ) )
	{
		nEquipMask = INV_TEAMMASK_CT;
		LocUTF8( "#CSInventory_TeamCT", "CT", szTeamLabel, sizeof( szTeamLabel ) );
	}
	else if ( !V_stricmp( command, "equip_t" ) )
	{
		nEquipMask = INV_TEAMMASK_T;
		LocUTF8( "#CSInventory_TeamT", "T", szTeamLabel, sizeof( szTeamLabel ) );
	}
	else if ( !V_stricmp( command, "equip_both" ) )
	{
		nEquipMask = INV_TEAMMASK_BOTH;
		LocUTF8( "#CSInventory_Status_BothTeams", "Both (CT+T)", szTeamLabel, sizeof( szTeamLabel ) );
	}

	if ( nEquipMask != INV_TEAMMASK_NONE )
	{
		EInventoryWeaponMode eMode = GetActiveMode();
		int nSkinWep = GetActiveSkinWeaponIndex();
		char szItemLabel[128] = "Default";

		if ( nSkinWep >= 0 )
		{
			const InventoryWeaponLoadout_t &loadout = m_PreviewWeaponLoadout[nSkinWep];
			const char *pszStkPtrs[INV_MAX_STICKER_SLOTS] =
			{
				loadout.szStickerKeys[0],
				loadout.szStickerKeys[1],
				loadout.szStickerKeys[2],
				loadout.szStickerKeys[3],
			};
			Inventory_EquipWeapon( nSkinWep, nEquipMask, loadout.szSkinKey, pszStkPtrs );
			const InventorySkinDef_t *pSkin = Inventory_FindWeaponSkinByKey( nSkinWep, loadout.szSkinKey );
			char szDefaultStock[64];
			LocUTF8( "#CSInventory_DefaultStock", "Default (Stock)", szDefaultStock, sizeof( szDefaultStock ) );
			V_snprintf( szItemLabel, sizeof( szItemLabel ), "%s | %s", Inventory_GetWeaponSkinDisplayName( nSkinWep ), pSkin ? pSkin->pszDisplayName : szDefaultStock );
		}
		else if ( eMode == INV_MODE_KNIFE )
		{
			Inventory_EquipKnife( nEquipMask, m_szPreviewKnife, m_szPreviewKnifeSkin );
			const InventoryKnifeDef_t *pKnife = Inventory_FindKnifeByKey( m_szPreviewKnife );
			const InventorySkinDef_t *pKnifeSkin = ( pKnife && pKnife->nSkinCount > 0 ) ? Inventory_FindKnifeSkinByKey( pKnife->pszKey, m_szPreviewKnifeSkin ) : NULL;
			if ( pKnife && pKnifeSkin )
				V_snprintf( szItemLabel, sizeof( szItemLabel ), "%s | %s", pKnife->pszDisplayName, pKnifeSkin->pszDisplayName );
			else if ( pKnife )
				V_strncpy( szItemLabel, pKnife->pszDisplayName, sizeof( szItemLabel ) );
			else
				LocUTF8( "#CSInventory_DefaultKnife", "Default Knife", szItemLabel, sizeof( szItemLabel ) );
		}
		else if ( eMode == INV_MODE_GLOVE )
		{
			Inventory_EquipGlove( nEquipMask, m_szPreviewGloveId );
			const InventoryGloveDef_t *pGlove = Inventory_FindGloveById( m_szPreviewGloveId );
			if ( pGlove )
				V_strncpy( szItemLabel, pGlove->pszDisplayName, sizeof( szItemLabel ) );
			else
				LocUTF8( "#CSInventory_DefaultGloves", "Default Gloves", szItemLabel, sizeof( szItemLabel ) );
		}
		else if ( eMode == INV_MODE_SLEEVE )
		{
			Inventory_EquipSleeve( nEquipMask, m_szPreviewSleeveKey );
			const InventorySleeveDef_t *pSleeve = Inventory_FindSleeveByKey( m_szPreviewSleeveKey );
			if ( pSleeve )
				V_strncpy( szItemLabel, pSleeve->pszDisplayName, sizeof( szItemLabel ) );
			else
				LocUTF8( "#CSInventory_DefaultSleeve", "Default Sleeve", szItemLabel, sizeof( szItemLabel ) );
		}

		RebuildGrid();

		char szMsg[160];
		LocFormat2UTF8( "#CSInventory_Status_Equipped", "Equipped %s for %s!", szItemLabel, szTeamLabel, szMsg, sizeof( szMsg ) );
		SetStatusBanner( szMsg, Color( 130, 220, 140, 255 ) );
		return;
	}

	BaseClass::OnCommand( command );
}

void CSInventory_Init()
{
	EnsureInventoryLocalizationLoaded();
	Inventory_EnsureLoaded();
}

void CSInventory_Shutdown()
{
	if ( g_hCSInventoryPanel.Get() )
	{
		g_hCSInventoryPanel->MarkForDeletion();
		g_hCSInventoryPanel = NULL;
	}
}

void CSInventory_Open( bool bToggle )
{
	if ( !g_hCSInventoryPanel.Get() )
	{
		VPANEL parent = enginevgui ? enginevgui->GetPanel( PANEL_GAMEUIDLL ) : NULL;
		if ( !parent )
			return;

		g_hCSInventoryPanel = vgui::SETUP_PANEL( new CCSInventoryPanel( parent ) );
	}

	if ( bToggle && g_hCSInventoryPanel->IsVisible() )
	{
		g_hCSInventoryPanel->CloseInventory();
	}
	else
	{
		g_hCSInventoryPanel->OpenInventory();
	}
}

CON_COMMAND_F( open_inventory, "Open the CS:GO weapons inventory menu.", FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE )
{
	CSInventory_Open( false );
}

CON_COMMAND_F( inventory, "Open or toggle the CS:GO weapons inventory menu.", FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE )
{
	CSInventory_Open( true );
}
