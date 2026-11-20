//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//
//=============================================================================//
//
// Health.cpp
//
// implementation of CHudHealth class
//
#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"

#include "iclientmode.h"

#define PAIN_NAME "sprites/%d_pain.vmt"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

using namespace vgui;

#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "cs_gamerules.h"

#include "convar.h"

//-----------------------------------------------------------------------------
// Purpose: Health panel
//-----------------------------------------------------------------------------
class CHudHealth : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudHealth, CHudNumericDisplay );

public:
	CHudHealth( const char *pElementName );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();

	virtual void Paint( void );
	virtual void ApplySchemeSettings( IScheme *scheme );

private:
	// old variables
	int		m_iHealth;

	int		m_bitsDamage;

	CHudTexture *m_pHealthIcon;

	CPanelAnimationVarAliasType( float, icon_xpos, "icon_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, icon_ypos, "icon_ypos", "0", "proportional_float" );

//	CPanelAnimationVar( Color, m_LowHealthColor, "LowHealthColor", "255 0 0 255" );

	float icon_tall;
	float icon_wide;

	Color m_clrBaseFg;
	Color m_clrLowHealth;

};

DECLARE_HUDELEMENT( CHudHealth );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHealth::CHudHealth( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay(NULL, "HudHealth"), m_pHealthIcon( NULL )
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudHealth::Init()
{
	m_iHealth		= 100;
	m_bitsDamage	= 0;
	icon_tall		= 0;
	icon_wide		= 0;
	SetIndent(true);
	SetDisplayValue(m_iHealth);
}

void CHudHealth::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	m_clrBaseFg = scheme->GetColor( "Panel.FgColor", Color( 255, 176, 0, 120 ) );
	m_clrLowHealth = scheme->GetColor( "HudIcon_Red", Color( 160, 0, 0, 255 ) );

	if( !m_pHealthIcon )
	{
		m_pHealthIcon = gHUD.GetIcon( "health_icon" );
	}

	if( m_pHealthIcon )
	{

		icon_tall = GetTall() - YRES(2);
		float scale = icon_tall / (float)m_pHealthIcon->Height();
		icon_wide = ( scale ) * (float)m_pHealthIcon->Width();
	}
}

//-----------------------------------------------------------------------------
// Purpose: reset health to normal color at round restart
//-----------------------------------------------------------------------------
void CHudHealth::Reset()
{
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthRestored");
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudHealth::VidInit()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudHealth::OnThink()
{
	int realHealth = 0;
	C_BasePlayer *local = C_BasePlayer::GetLocalPlayer();
	if ( local )
	{
		// Never below zero
		realHealth = MAX( local->GetHealth(), 0 );
	}

	// Only update the fade if we've changed health
	if ( realHealth == m_iHealth )
	{
		return;
	}

	if( realHealth > m_iHealth)
	{
		// round restarted, we have 100 again
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthRestored");
	}
	else if ( realHealth <= 25 )
	{
		// we are badly injured
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthLow");
	}
	else if( realHealth < m_iHealth )
	{
		// took a hit
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthTookDamage");
	}

	m_iHealth = realHealth;

	SetDisplayValue(m_iHealth);
}

void CHudHealth::Paint( void )
{
	Color origFg = GetFgColor();
	Color drawFg = origFg;

	if ( gHUD.IsCustomHudColorEnabled() )
	{
		Color customClr = gHUD.GetCustomHudColor();
		int baseG = MAX( 1, (int)m_clrBaseFg.g() );
		float t = clamp( (float)origFg.g() / (float)baseG, 0.0f, 1.0f );
		drawFg.SetColor(
			(int)( m_clrLowHealth.r() + ( customClr.r() - m_clrLowHealth.r() ) * t ),
			(int)( m_clrLowHealth.g() + ( customClr.g() - m_clrLowHealth.g() ) * t ),
			(int)( m_clrLowHealth.b() + ( customClr.b() - m_clrLowHealth.b() ) * t ),
			(int)( m_clrLowHealth.a() + ( customClr.a() - m_clrLowHealth.a() ) * t ) );
		SetFgColor( drawFg );
	}

	if( m_pHealthIcon )
	{
		m_pHealthIcon->DrawSelf( icon_xpos, icon_ypos, icon_wide, icon_tall, drawFg );
	}

	//draw the health icon
	BaseClass::Paint();

	if ( gHUD.IsCustomHudColorEnabled() )
	{
		SetFgColor( origFg );
	}
}
