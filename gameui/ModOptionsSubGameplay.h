//========= Copyright Valve Corporation, All rights reserved. ============//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef MODOPTIONSSUBGAMEPLAY_H
#define MODOPTIONSSUBGAMEPLAY_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>

class CCvarToggleCheckButton;
class CCvarSlider;
class CColorPickerButton;

//-----------------------------------------------------------------------------
// Purpose: gameplay options property page
//-----------------------------------------------------------------------------
class CModOptionsSubGameplay : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( CModOptionsSubGameplay, vgui::PropertyPage );

public:
	CModOptionsSubGameplay( vgui::Panel *parent );
	~CModOptionsSubGameplay();

protected:
	virtual void OnResetData();
	virtual void OnApplyChanges();

	MESSAGE_FUNC( OnControlModified, "ControlModified" );
	MESSAGE_FUNC_PTR( OnTextChanged, "TextChanged", panel );
	MESSAGE_FUNC_PARAMS( OnSliderMoved, "SliderMoved", data );
	MESSAGE_FUNC( OnCheckButtonChecked, "CheckButtonChecked" );
	MESSAGE_FUNC_PARAMS( OnColorPickerPicked, "ColorPickerPicked", data );
	MESSAGE_FUNC_PARAMS( OnColorPickerPreview, "ColorPickerPreview", data );
	MESSAGE_FUNC_PARAMS( OnColorPickerCancel, "ColorPickerCancel", data );

private:
	void ApplyHudColorConVar( const Color &clr );

	CCvarSlider *m_pViewModelFOV;
	CCvarSlider *m_pViewModelRight;
	CCvarSlider *m_pViewModelUp;
	CCvarSlider *m_pRadarPanelScale;
	CCvarSlider *m_pRadarScale;
	CCvarSlider *m_pRadarAlpha;

	CCvarToggleCheckButton *m_pRadarRotate;
	CCvarToggleCheckButton *m_pRadarSquare;
	CCvarToggleCheckButton *m_pDisplayC4Timer;
	CCvarToggleCheckButton *m_pDisableTouchOnBuyMenu;
	CCvarToggleCheckButton *m_pDrawTracers;
	CCvarToggleCheckButton *m_pUseNewHeadbob;
	CCvarToggleCheckButton *m_pCloseBuyMenuAfterPurchase;
	CCvarToggleCheckButton *m_pColoredHud;
	CColorPickerButton *m_pHudColor;

	Color m_HudColor;
	Color m_SavedHudColor;
	bool m_bHudColorDirty;
};

#endif // MODOPTIONSSUBGAMEPLAY_H