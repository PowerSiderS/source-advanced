//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side view model implementation. Responsible for drawing
//			the view model.
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_baseviewmodel.h"
#include "model_types.h"
#include "hud.h"
#include "view_shared.h"
#include "iviewrender.h"
#include "view.h"
#include "mathlib/vmatrix.h"
#include "cl_animevent.h"
#include "eventlist.h"
#include "tools/bonelist.h"
#include <KeyValues.h>
#include "hltvcamera.h"
#include "filesystem.h"
#include "proxyentity.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/itexture.h"
#ifdef CSTRIKE_DLL
	#include "cs_shareddefs.h"
	#include "c_cs_player.h"
#endif

#if defined( REPLAY_ENABLED )
#include "replay/replaycamera.h"
#include "replay/ireplaysystem.h"
#include "replay/ienginereplay.h"
#endif

// NVNT haptics system interface
#include "haptics/ihaptics.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CSTRIKE_DLL
	ConVar cl_righthand( "cl_righthand", "1", FCVAR_ARCHIVE, "Use right-handed view models." );
	ConVar cl_arm_glove( "cl_arm_glove", "-1", FCVAR_ARCHIVE, "CS:GO viewmodel glove type (-1 = faction default, 0 = bare, 1 = hardknuckle, 2 = fullfinger, 3 = fingerless, 4 = anarchist, 5 = bloodhound, 6 = sporty, 7 = slick/driver, 8 = handwrap, 9 = moto, 10 = specialist, 11 = hydra, 12 = brokenfang, or alias/.mdl path)." );
	ConVar cl_arm_glove_hardknuckle( "cl_arm_glove_hardknuckle", "0", FCVAR_ARCHIVE, "Hardknuckle glove variant (0 = default, 1 = blue, 2 = black)." );
	ConVar cl_arm_glove_bloodhound( "cl_arm_glove_bloodhound", "0", FCVAR_ARCHIVE, "Bloodhound glove skin (0 = default, 1-4 or skin name)." );
	ConVar cl_arm_glove_sporty( "cl_arm_glove_sporty", "0", FCVAR_ARCHIVE, "Sporty glove skin (0 = default, 1-12 or skin name)." );
	ConVar cl_arm_glove_slick( "cl_arm_glove_slick", "0", FCVAR_ARCHIVE, "Driver/Slick glove skin (0 = default, 1-17 or skin name)." );
	ConVar cl_arm_glove_handwrap( "cl_arm_glove_handwrap", "0", FCVAR_ARCHIVE, "Hand Wraps glove skin (0 = default, 1-15 or skin name)." );
	ConVar cl_arm_glove_moto( "cl_arm_glove_moto", "0", FCVAR_ARCHIVE, "Moto glove skin (0 = default, 1-15 or skin name)." );
	ConVar cl_arm_glove_specialist( "cl_arm_glove_specialist", "0", FCVAR_ARCHIVE, "Specialist glove skin (0 = default, 1-12 or skin name)." );
	ConVar cl_arm_glove_hydra( "cl_arm_glove_hydra", "0", FCVAR_ARCHIVE, "Hydra glove skin (0 = default, 1-8 or skin name)." );
	ConVar cl_arm_glove_brokenfang( "cl_arm_glove_brokenfang", "0", FCVAR_ARCHIVE, "Broken Fang glove skin (0 = default, 1-4 or skin name)." );
	ConVar cl_arm_sleeve( "cl_arm_sleeve", "-1", FCVAR_ARCHIVE, "CS:GO viewmodel sleeve (-1 = faction default, 0/none = no sleeve, 1-50 or alias/.mdl path)." );
	ConVar cl_arm_skintone( "cl_arm_skintone", "-1", FCVAR_ARCHIVE, "Override bare-arm skintone skin index (0-7). -1 = faction default." );
	ConVar cl_ak47_skin( "cl_ak47_skin", "0", FCVAR_ARCHIVE, "AK-47 skin selection (0 = default, 1-34 or skin name e.g. bloodsport, vulcan, asiimov, redline, fireserpent)." );
	ConVar cl_ak47_sticker_1( "cl_ak47_sticker_1", "0", FCVAR_ARCHIVE, "AK-47 sticker slot 1 (0 = none, 1-40 or sticker name e.g. ibuypower, titan, crown, howling_dawn)." );
	ConVar cl_ak47_sticker_2( "cl_ak47_sticker_2", "0", FCVAR_ARCHIVE, "AK-47 sticker slot 2 (0 = none, 1-40 or sticker name)." );
	ConVar cl_ak47_sticker_3( "cl_ak47_sticker_3", "0", FCVAR_ARCHIVE, "AK-47 sticker slot 3 (0 = none, 1-40 or sticker name)." );
	ConVar cl_ak47_sticker_4( "cl_ak47_sticker_4", "0", FCVAR_ARCHIVE, "AK-47 sticker slot 4 (0 = none, 1-40 or sticker name)." );

	static void UpdateViewmodelStickers( C_BaseViewModel *pVM, C_CSPlayer *pPlayer );
	static void RemoveViewmodelStickers( C_BaseViewModel *pVM );
	static void DrawViewmodelStickers( C_BaseViewModel *pVM, int flags );

	static int GetArmGloveSkinConVarValue( const char *pszFamilyKey, char *pszOutStr, int nOutStrSize )
	{
		const ConVar *pVar = NULL;
		if ( !V_stricmp( pszFamilyKey, "hardknuckle" ) )		pVar = &cl_arm_glove_hardknuckle;
		else if ( !V_stricmp( pszFamilyKey, "bloodhound" ) )pVar = &cl_arm_glove_bloodhound;
		else if ( !V_stricmp( pszFamilyKey, "sporty" ) )	pVar = &cl_arm_glove_sporty;
		else if ( !V_stricmp( pszFamilyKey, "slick" ) )		pVar = &cl_arm_glove_slick;
		else if ( !V_stricmp( pszFamilyKey, "handwrap" ) )	pVar = &cl_arm_glove_handwrap;
		else if ( !V_stricmp( pszFamilyKey, "moto" ) )		pVar = &cl_arm_glove_moto;
		else if ( !V_stricmp( pszFamilyKey, "specialist" ) )pVar = &cl_arm_glove_specialist;
		else if ( !V_stricmp( pszFamilyKey, "hydra" ) )		pVar = &cl_arm_glove_hydra;
		else if ( !V_stricmp( pszFamilyKey, "brokenfang" ) )pVar = &cl_arm_glove_brokenfang;

		if ( !pVar )
		{
			if ( pszOutStr && nOutStrSize > 0 )
				pszOutStr[0] = '\0';
			return 0;
		}

		if ( pszOutStr && nOutStrSize > 0 )
		{
			V_strncpy( pszOutStr, pVar->GetString(), nOutStrSize );
		}
		return pVar->GetInt();
	}
#endif

#ifdef TF_CLIENT_DLL
	ConVar cl_flipviewmodels( "cl_flipviewmodels", "0", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_NOT_CONNECTED, "Flip view models." );
#endif

void PostToolMessage( HTOOLHANDLE hEntity, KeyValues *msg );

void FormatViewModelAttachment( Vector &vOrigin, bool bInverse )
{
	// Presumably, SetUpView has been called so we know our FOV and render origin.
	const CViewSetup *pViewSetup = view->GetPlayerViewSetup();
	
	float worldx = tan( pViewSetup->fov * M_PI/360.0 );
	float viewx = tan( pViewSetup->fovViewmodel * M_PI/360.0 );

	// aspect ratio cancels out, so only need one factor
	// the difference between the screen coordinates of the 2 systems is the ratio
	// of the coefficients of the projection matrices (tan (fov/2) is that coefficient)
	float factorX = worldx / viewx;

	float factorY = factorX;
	
	// Get the coordinates in the viewer's space.
	Vector tmp = vOrigin - pViewSetup->origin;
	Vector vTransformed( MainViewRight().Dot( tmp ), MainViewUp().Dot( tmp ), MainViewForward().Dot( tmp ) );

	// Now squash X and Y.
	if ( bInverse )
	{
		if ( factorX != 0 && factorY != 0 )
		{
			vTransformed.x /= factorX;
			vTransformed.y /= factorY;
		}
		else
		{
			vTransformed.x = 0.0f;
			vTransformed.y = 0.0f;
		}
	}
	else
	{
		vTransformed.x *= factorX;
		vTransformed.y *= factorY;
	}



	// Transform back to world space.
	Vector vOut = (MainViewRight() * vTransformed.x) + (MainViewUp() * vTransformed.y) + (MainViewForward() * vTransformed.z);
	vOrigin = pViewSetup->origin + vOut;
}


void C_BaseViewModel::FormatViewModelAttachment( int nAttachment, matrix3x4_t &attachmentToWorld )
{
	Vector vecOrigin;
	MatrixPosition( attachmentToWorld, vecOrigin );
	::FormatViewModelAttachment( vecOrigin, false );
	PositionMatrix( vecOrigin, attachmentToWorld );
}


bool C_BaseViewModel::IsViewModel() const
{
	return true;
}

void C_BaseViewModel::UncorrectViewModelAttachment( Vector &vOrigin )
{
	// Unformat the attachment.
	::FormatViewModelAttachment( vOrigin, true );
}


//-----------------------------------------------------------------------------
// Purpose
//-----------------------------------------------------------------------------
void C_BaseViewModel::FireEvent( const Vector& origin, const QAngle& angles, int event, const char *options )
{
	// We override sound requests so that we can play them locally on the owning player
	if ( ( event == AE_CL_PLAYSOUND ) || ( event == CL_EVENT_SOUND ) )
	{
		// Only do this if we're owned by someone
		if ( GetOwner() != NULL )
		{
			CLocalPlayerFilter filter;
			EmitSound( filter, GetOwner()->GetSoundSourceIndex(), options, &GetAbsOrigin() );
			return;
		}
	}

	// Otherwise pass the event to our associated weapon
	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if ( pWeapon )
	{
		// NVNT notify the haptics system of our viewmodel's event
		if ( haptics )
			haptics->ProcessHapticEvent(4,"Weapons",pWeapon->GetName(),"AnimationEvents",VarArgs("%i",event));

		bool bResult = pWeapon->OnFireEvent( this, origin, angles, event, options );
		if ( !bResult )
		{
			BaseClass::FireEvent( origin, angles, event, options );
		}
	}
}

bool C_BaseViewModel::Interpolate( float currentTime )
{
	CStudioHdr *pStudioHdr = GetModelPtr();
	// Make sure we reset our animation information if we've switch sequences
	UpdateAnimationParity();

	bool bret = BaseClass::Interpolate( currentTime );

	// Hack to extrapolate cycle counter for view model
	float elapsed_time = currentTime - m_flAnimTime;
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	// Predicted viewmodels have fixed up interval
	if ( GetPredictable() || IsClientCreated() )
	{
		Assert( pPlayer );
		float curtime = pPlayer ? pPlayer->GetFinalPredictedTime() : gpGlobals->curtime;
		elapsed_time = curtime - m_flAnimTime;
		// Adjust for interpolated partial frame
		if ( !engine->IsPaused() )
		{
			elapsed_time += ( gpGlobals->interpolation_amount * TICK_INTERVAL );
		}
	}

	// Prediction errors?	
	if ( elapsed_time < 0 )
	{
		elapsed_time = 0;
	}

	float dt = elapsed_time * GetSequenceCycleRate( pStudioHdr, GetSequence() ) * GetPlaybackRate();
	if ( dt >= 1.0f )
	{
		if ( !IsSequenceLooping( GetSequence() ) )
		{
			dt = 0.999f;
		}
		else
		{
			dt = fmod( dt, 1.0f );
		}
	}

	SetCycle( dt );
	return bret;
}


inline bool C_BaseViewModel::ShouldFlipViewModel()
{
#ifdef CSTRIKE_DLL
	// If cl_righthand is set, then we want them all right-handed.
	CBaseCombatWeapon *pWeapon = m_hWeapon.Get();
	if ( pWeapon )
	{
		const FileWeaponInfo_t *pInfo = &pWeapon->GetWpnData();
		return pInfo->m_bAllowFlipping && pInfo->m_bBuiltRightHanded != cl_righthand.GetBool();
	}
#endif

#ifdef TF_CLIENT_DLL
	CBaseCombatWeapon *pWeapon = m_hWeapon.Get();
	if ( pWeapon )
	{
		return pWeapon->m_bFlipViewModel != cl_flipviewmodels.GetBool();
	}
#endif

	return false;
}


void C_BaseViewModel::ApplyBoneMatrixTransform( matrix3x4_t& transform )
{
	if ( ShouldFlipViewModel() )
	{
		matrix3x4_t viewMatrix, viewMatrixInverse;

		// We could get MATERIAL_VIEW here, but this is called sometimes before the renderer
		// has set that matrix. Luckily, this is called AFTER the CViewSetup has been initialized.
		const CViewSetup *pSetup = view->GetPlayerViewSetup();
		AngleMatrix( pSetup->angles, pSetup->origin, viewMatrixInverse );
		MatrixInvert( viewMatrixInverse, viewMatrix );

		// Transform into view space.
		matrix3x4_t temp, temp2;
		ConcatTransforms( viewMatrix, transform, temp );
		
		// Flip it along X.
		
		// (This is the slower way to do it, and it equates to negating the top row).
		//matrix3x4_t mScale;
		//SetIdentityMatrix( mScale );
		//mScale[0][0] = 1;
		//mScale[1][1] = -1;
		//mScale[2][2] = 1;
		//ConcatTransforms( mScale, temp, temp2 );
		temp[1][0] = -temp[1][0];
		temp[1][1] = -temp[1][1];
		temp[1][2] = -temp[1][2];
		temp[1][3] = -temp[1][3];

		// Transform back out of view space.
		ConcatTransforms( viewMatrixInverse, temp, transform );
	}
}

//-----------------------------------------------------------------------------
// Purpose: check if weapon viewmodel should be drawn
//-----------------------------------------------------------------------------
bool C_BaseViewModel::ShouldDraw()
{
	if ( engine->IsHLTV() )
	{
		return ( HLTVCamera()->GetMode() == OBS_MODE_IN_EYE &&
				 HLTVCamera()->GetPrimaryTarget() == GetOwner()	);
	}
#if defined( REPLAY_ENABLED )
	else if ( g_pEngineClientReplay->IsPlayingReplayDemo() )
	{
		return ( ReplayCamera()->GetMode() == OBS_MODE_IN_EYE &&
				 ReplayCamera()->GetPrimaryTarget() == GetOwner() );
	}
#endif
	else
	{
		return BaseClass::ShouldDraw();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Render the weapon. Draw the Viewmodel if the weapon's being carried
//			by this player, otherwise draw the worldmodel.
//-----------------------------------------------------------------------------
int C_BaseViewModel::DrawModel( int flags )
{
	if ( !m_bReadyToDraw )
		return 0;

	if ( flags & STUDIO_RENDER )
	{
		// Determine blending amount and tell engine
		float blend = (float)( GetFxBlend() / 255.0f );

		// Totally gone
		if ( blend <= 0.0f )
			return 0;

		// Tell engine
		render->SetBlend( blend );

		float color[3];
		GetColorModulation( color );
		render->SetColorModulation(	color );
	}
		
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	C_BaseCombatWeapon *pWeapon = GetOwningWeapon();
	int ret;
	// If the local player's overriding the viewmodel rendering, let him do it
	if ( pPlayer && pPlayer->IsOverridingViewmodel() )
	{
		ret = pPlayer->DrawOverriddenViewmodel( this, flags );
	}
	else if ( pWeapon && pWeapon->IsOverridingViewmodel() )
	{
		ret = pWeapon->DrawOverriddenViewmodel( this, flags );
	}
	else
	{
		ret = BaseClass::DrawModel( flags );
	}

#ifdef CSTRIKE_DLL
	if ( flags )
	{
		FOR_EACH_VEC( m_vecViewmodelArmModels, i )
		{
			if ( m_vecViewmodelArmModels[i] )
			{
				if ( m_vecViewmodelArmModels[i]->GetMoveParent() != this )
				{
					m_vecViewmodelArmModels[i]->AddEffects( EF_BONEMERGE | EF_BONEMERGE_FASTCULL );
					m_vecViewmodelArmModels[i]->SetParent( this );
				}

				m_vecViewmodelArmModels[i]->DrawModel( flags );
			}
		}
		DrawViewmodelStickers( this, flags );
	}
#endif

	// Now that we've rendered, reset the animation restart flag
	if ( flags & STUDIO_RENDER )
	{
		if ( m_nOldAnimationParity != m_nAnimationParity )
		{
			m_nOldAnimationParity = m_nAnimationParity;
		}
		// Tell the weapon itself that we've rendered, in case it wants to do something
		if ( pWeapon )
		{
			pWeapon->ViewModelDrawn( this );
		}
	}

	return ret;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_BaseViewModel::InternalDrawModel( int flags )
{
	CMatRenderContextPtr pRenderContext( materials );
	if ( ShouldFlipViewModel() )
		pRenderContext->CullMode( MATERIAL_CULLMODE_CW );

	int ret = BaseClass::InternalDrawModel( flags );

	pRenderContext->CullMode( MATERIAL_CULLMODE_CCW );

	return ret;
}

//-----------------------------------------------------------------------------
// Purpose: Called by the player when the player's overriding the viewmodel drawing. Avoids infinite recursion.
//-----------------------------------------------------------------------------
int C_BaseViewModel::DrawOverriddenViewmodel( int flags )
{
	return BaseClass::DrawModel( flags );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
int C_BaseViewModel::GetFxBlend( void )
{
	// See if the local player wants to override the viewmodel's rendering
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( pPlayer && pPlayer->IsOverridingViewmodel() )
	{
		pPlayer->ComputeFxBlend();
		return pPlayer->GetFxBlend();
	}

	C_BaseCombatWeapon *pWeapon = GetOwningWeapon();
	if ( pWeapon && pWeapon->IsOverridingViewmodel() )
	{
		pWeapon->ComputeFxBlend();
		return pWeapon->GetFxBlend();
	}

	return BaseClass::GetFxBlend();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_BaseViewModel::IsTransparent( void )
{
	// See if the local player wants to override the viewmodel's rendering
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( pPlayer && pPlayer->IsOverridingViewmodel() )
	{
		return pPlayer->ViewModel_IsTransparent();
	}

	C_BaseCombatWeapon *pWeapon = GetOwningWeapon();
	if ( pWeapon && pWeapon->IsOverridingViewmodel() )
		return pWeapon->ViewModel_IsTransparent();

	return BaseClass::IsTransparent();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_BaseViewModel::UsesPowerOfTwoFrameBufferTexture( void )
{
	// See if the local player wants to override the viewmodel's rendering
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( pPlayer && pPlayer->IsOverridingViewmodel() )
	{
		return pPlayer->ViewModel_IsUsingFBTexture();
	}

	C_BaseCombatWeapon *pWeapon = GetOwningWeapon();
	if ( pWeapon && pWeapon->IsOverridingViewmodel() )
	{
		return pWeapon->ViewModel_IsUsingFBTexture();
	}

	return BaseClass::UsesPowerOfTwoFrameBufferTexture();
}

//-----------------------------------------------------------------------------
// Purpose: If the animation parity of the weapon has changed, we reset cycle to avoid popping
//-----------------------------------------------------------------------------
void C_BaseViewModel::UpdateAnimationParity( void )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	
	// If we're predicting, then we don't use animation parity because we change the animations on the clientside
	// while predicting. When not predicting, only the server changes the animations, so a parity mismatch
	// tells us if we need to reset the animation.
	if ( m_nOldAnimationParity != m_nAnimationParity && !GetPredictable() )
	{
		float curtime = (pPlayer && IsIntermediateDataAllocated()) ? pPlayer->GetFinalPredictedTime() : gpGlobals->curtime;
		// FIXME: this is bad
		// Simulate a networked m_flAnimTime and m_flCycle
		// FIXME:  Do we need the magic 0.1?
		SetCycle( 0.0f ); // GetSequenceCycleRate( GetSequence() ) * 0.1;
		m_flAnimTime = curtime;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Update global map state based on data received
// Input  : bnewentity - 
//-----------------------------------------------------------------------------
void C_BaseViewModel::OnDataChanged( DataUpdateType_t updateType )
{
	SetPredictionEligible( true );
	BaseClass::OnDataChanged(updateType);
}

void C_BaseViewModel::PostDataUpdate( DataUpdateType_t updateType )
{
	int nPrevModelIndex = m_nModelIndex;
	BaseClass::PostDataUpdate(updateType);
	if ( nPrevModelIndex != m_nModelIndex )
	{
		SetCycle( 0.0f );
		m_flAnimTime = gpGlobals->curtime;
	}
	OnLatchInterpolatedVariables( LATCH_ANIMATION_VAR );
}


//-----------------------------------------------------------------------------
// Purpose: Add entity to visible view models list
//-----------------------------------------------------------------------------
void C_BaseViewModel::AddEntity( void )
{
	// Server says don't interpolate this frame, so set previous info to new info.
	if ( IsNoInterpolationFrame() )
	{
		ResetLatched();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BaseViewModel::GetBoneControllers(float controllers[MAXSTUDIOBONECTRLS])
{
	BaseClass::GetBoneControllers( controllers );

	// Tell the weapon itself that we've rendered, in case it wants to do something
	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if ( pWeapon )
	{
		pWeapon->GetViewmodelBoneControllers( this, controllers );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : RenderGroup_t
//-----------------------------------------------------------------------------
RenderGroup_t C_BaseViewModel::GetRenderGroup()
{
	return RENDER_GROUP_VIEW_MODEL_OPAQUE;
}

void C_BaseViewModel::UpdateAllViewmodelAddons( void )
{
#ifdef CSTRIKE_DLL
	C_CSPlayer *pPlayer = ToCSPlayer( GetOwner() );
	if ( !pPlayer || !pPlayer->IsAlive() )
	{
		RemoveViewmodelArmModels();
		m_szLastArmPlayerModel[0] = '\0';
		return;
	}

	UpdateViewmodelStickers( this, pPlayer );

	const model_t *pPlayerModel = pPlayer->GetModel();
	const char *pszPlayerModel = pPlayerModel ? modelinfo->GetModelName( pPlayerModel ) : "";
	const char *pszGloveCvar = cl_arm_glove.GetString();
	const char *pszSleeveCvar = cl_arm_sleeve.GetString();
	int nSkintoneCvar = cl_arm_skintone.GetInt();

	const PlayerViewmodelArmConfig *pConfig = GetPlayerViewmodelArmConfigForPlayerModel( pszPlayerModel );
	if ( !pConfig )
		return;

	const char *pszGloveModel = pConfig->szAssociatedGloveModel;
	const char *pszOverrideTex = "";
	if ( pszGloveCvar && pszGloveCvar[0] != '\0' && V_strcmp( pszGloveCvar, "-1" ) != 0 )
	{
		ArmGloveSelection gloveSelection;
		if ( ResolveViewmodelGloveSelection( pszGloveCvar, GetArmGloveSkinConVarValue, gloveSelection ) && gloveSelection.pszModelPath )
		{
			pszGloveModel = gloveSelection.pszModelPath;
			if ( gloveSelection.pszOverrideBaseTexture )
			{
				pszOverrideTex = gloveSelection.pszOverrideBaseTexture;
			}
		}
	}

	char szResolvedSleeve[MAX_PATH];
	const char *pszSleeveModel = pConfig->szAssociatedSleeveModel;
	bool bHasExplicitSleeve = false;
	if ( pszSleeveCvar && pszSleeveCvar[0] != '\0' && V_strcmp( pszSleeveCvar, "-1" ) != 0 )
	{
		if ( !V_stricmp( pszSleeveCvar, "none" ) || !V_stricmp( pszSleeveCvar, "off" ) || !V_stricmp( pszSleeveCvar, "0" ) )
		{
			pszSleeveModel = "";
		}
		else
		{
			const char *pszResolved = ResolveViewmodelSleeveModelPath( pszSleeveCvar, szResolvedSleeve, sizeof( szResolvedSleeve ) );
			if ( pszResolved )
			{
				pszSleeveModel = pszResolved;
				bHasExplicitSleeve = true;
			}
		}
	}

	int nDesiredSkintone = ( nSkintoneCvar >= 0 ) ? nSkintoneCvar : pConfig->iSkintoneIndex;

	bool bConfigChanged =
		( pPlayer->m_pViewmodelArmConfig != pConfig ) ||
		( V_strcmp( m_szLastArmPlayerModel, pszPlayerModel ) != 0 ) ||
		( V_strcmp( m_szLastGloveConVar, pszGloveModel ) != 0 ) ||
		( V_strcmp( m_szLastGloveOverrideTex, pszOverrideTex ) != 0 ) ||
		( V_strcmp( m_szLastSleeveConVar, pszSleeveModel ) != 0 ) ||
		( m_nLastSkintone != nDesiredSkintone ) ||
		( m_vecViewmodelArmModels.Count() == 0 );

	if ( !bConfigChanged )
		return;

	RemoveViewmodelArmModels();
	pPlayer->m_pViewmodelArmConfig = pConfig;

	V_strncpy( m_szLastArmPlayerModel, pszPlayerModel, sizeof( m_szLastArmPlayerModel ) );
	V_strncpy( m_szLastGloveConVar, pszGloveModel, sizeof( m_szLastGloveConVar ) );
	V_strncpy( m_szLastGloveOverrideTex, pszOverrideTex, sizeof( m_szLastGloveOverrideTex ) );
	V_strncpy( m_szLastSleeveConVar, pszSleeveModel, sizeof( m_szLastSleeveConVar ) );
	m_nLastSkintone = nDesiredSkintone;

	bool bHideBare = ( pszSleeveModel && pszSleeveModel[0] != '\0' ) && ( bHasExplicitSleeve || pConfig->bHideBareArms );

	if ( pszGloveModel && pszGloveModel[0] != '\0' )
	{
		AddViewmodelArmModel( pszGloveModel, nDesiredSkintone, bHideBare, pszOverrideTex[0] ? pszOverrideTex : NULL );
	}
	if ( pszSleeveModel && pszSleeveModel[0] != '\0' )
	{
		AddViewmodelArmModel( pszSleeveModel, 0, false, NULL );
	}
#endif
}

void C_BaseViewModel::AddViewmodelArmModel( const char *pszArmsModel, int nSkintoneIndex, bool bHideBareArms, const char *pszOverrideBaseTexture )
{
	if ( pszArmsModel == NULL || pszArmsModel[0] == '\0' )
		return;

	if ( modelinfo->GetModelIndex( pszArmsModel ) == -1 && ( !filesystem || !filesystem->FileExists( pszArmsModel, "GAME" ) ) )
		return;

	C_ViewmodelAttachmentModel *pEnt = new C_ViewmodelAttachmentModel;
	if ( !pEnt )
		return;

	if ( pEnt->InitializeArmModel( pszArmsModel, nSkintoneIndex, bHideBareArms, pszOverrideBaseTexture ) )
	{
		m_vecViewmodelArmModels.AddToTail( pEnt );

		pEnt->SetParent( this );
		pEnt->SetLocalOrigin( vec3_origin );
		pEnt->UpdatePartitionListEntry();
		pEnt->CollisionProp()->MarkPartitionHandleDirty();
		pEnt->UpdateVisibility();
		pEnt->SetViewmodel( this );

		RemoveEffects( EF_NODRAW );
	}
	else
	{
		pEnt->Release();
	}
}

void C_BaseViewModel::RemoveViewmodelArmModels( void )
{
	FOR_EACH_VEC_BACK( m_vecViewmodelArmModels, i )
	{
		C_ViewmodelAttachmentModel *pEnt = m_vecViewmodelArmModels[i].Get();
		if ( pEnt )
		{
			pEnt->Remove();
		}
	}
	m_vecViewmodelArmModels.RemoveAll();
	RemoveViewmodelStickers( this );
}

static CUtlVector< C_ViewmodelAttachmentModel * > s_vecActiveAttachmentModels;

C_ViewmodelAttachmentModel::C_ViewmodelAttachmentModel()
{
	EnableDynamicModels();
	m_pOverrideBaseTexture = NULL;
	m_nPendingSkintone = -1;
	m_bPendingHideBareArms = false;
	s_vecActiveAttachmentModels.AddToTail( this );
}

C_ViewmodelAttachmentModel::~C_ViewmodelAttachmentModel()
{
	s_vecActiveAttachmentModels.FindAndRemove( this );
	if ( m_pOverrideBaseTexture )
	{
		m_pOverrideBaseTexture->DecrementReferenceCount();
		m_pOverrideBaseTexture = NULL;
	}
}

void C_ViewmodelAttachmentModel::SetOverrideBaseTexture( const char *pszTexturePath )
{
	if ( m_pOverrideBaseTexture )
	{
		m_pOverrideBaseTexture->DecrementReferenceCount();
		m_pOverrideBaseTexture = NULL;
	}

	if ( pszTexturePath && pszTexturePath[0] != '\0' )
	{
		ITexture *pTex = materials->FindTexture( pszTexturePath, TEXTURE_GROUP_MODEL, false );
		if ( pTex && !IsErrorTexture( pTex ) )
		{
			pTex->IncrementReferenceCount();
			m_pOverrideBaseTexture = pTex;
		}
	}
}

bool C_ViewmodelAttachmentModel::InitializeAsClientEntity( const char *pszModelName, RenderGroup_t renderGroup )
{
	if ( !pszModelName || !pszModelName[0] )
		return false;

	int nModelIndex = modelinfo->GetModelIndex( pszModelName );
	if ( nModelIndex == -1 )
	{
		nModelIndex = modelinfo->RegisterDynamicModel( pszModelName, true );
	}
	if ( nModelIndex == -1 )
		return false;

	Interp_SetupMappings( GetVarMapping() );

	if ( !InitializeAsClientEntityByIndex( nModelIndex, renderGroup ) )
		return false;

	AddEffects( EF_BONEMERGE );
	AddEffects( EF_BONEMERGE_FASTCULL );

	// Invisible to the main world pass; explicitly rendered inside C_BaseViewModel::DrawModel
	AddEffects( EF_NODRAW );
	return true;
}

bool C_ViewmodelAttachmentModel::InitializeArmModel( const char *pszModelName, int nSkintoneIndex, bool bHideBareArms, const char *pszOverrideBaseTexture )
{
	m_nPendingSkintone = nSkintoneIndex;
	m_bPendingHideBareArms = bHideBareArms;
	SetOverrideBaseTexture( pszOverrideBaseTexture );

	if ( !InitializeAsClientEntity( pszModelName, RENDER_GROUP_VIEW_MODEL_OPAQUE ) )
		return false;

	ApplyArmAppearance();
	return true;
}

void C_ViewmodelAttachmentModel::ApplyArmAppearance( void )
{
	if ( m_nPendingSkintone != -1 )
	{
		m_nSkin = m_nPendingSkintone;
	}

	if ( GetModelPtr() )
	{
		int iBodygroup = FindBodygroupByName( "bare" );
		if ( iBodygroup != -1 )
		{
			SetBodygroup( iBodygroup, m_bPendingHideBareArms ? 1 : 0 );
		}
	}
}

CStudioHdr *C_ViewmodelAttachmentModel::OnNewModel( void )
{
	CStudioHdr *pHdr = BaseClass::OnNewModel();
	if ( pHdr )
	{
		ApplyArmAppearance();
	}
	return pHdr;
}

void C_ViewmodelAttachmentModel::SetViewmodel( C_BaseViewModel *pVM )
{
	m_hViewmodel = pVM;
}

int C_ViewmodelAttachmentModel::InternalDrawModel( int flags )
{
	CMatRenderContextPtr pRenderContext( materials );
	C_BaseViewModel *pViewmodel = m_hViewmodel;
	if ( pViewmodel && pViewmodel->ShouldFlipViewModel() )
		pRenderContext->CullMode( MATERIAL_CULLMODE_CW );

	int r = BaseClass::InternalDrawModel( flags );

	pRenderContext->CullMode( MATERIAL_CULLMODE_CCW );

	return r;
}

class CArmSkinTextureProxy : public CEntityMaterialProxy
{
public:
	CArmSkinTextureProxy() : m_pBaseTextureVar( NULL ), m_pDefaultTexture( NULL ) {}
	~CArmSkinTextureProxy()
	{
		if ( m_pDefaultTexture )
		{
			m_pDefaultTexture->DecrementReferenceCount();
			m_pDefaultTexture = NULL;
		}
	}

	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues )
	{
		bool bFound = false;
		m_pBaseTextureVar = pMaterial->FindVar( "$basetexture", &bFound, false );
		if ( !bFound || !m_pBaseTextureVar )
			return false;

		if ( m_pBaseTextureVar->IsTexture() )
		{
			m_pDefaultTexture = m_pBaseTextureVar->GetTextureValue();
			if ( m_pDefaultTexture )
			{
				m_pDefaultTexture->IncrementReferenceCount();
			}
		}
		return true;
	}

	virtual void OnBind( C_BaseEntity *pBaseEntity )
	{
		if ( !m_pBaseTextureVar || !m_pBaseTextureVar->IsTexture() )
			return;

		if ( !m_pDefaultTexture )
		{
			m_pDefaultTexture = m_pBaseTextureVar->GetTextureValue();
			if ( m_pDefaultTexture )
			{
				m_pDefaultTexture->IncrementReferenceCount();
			}
		}

		ITexture *pTargetTexture = m_pDefaultTexture;
		if ( pBaseEntity )
		{
			FOR_EACH_VEC( s_vecActiveAttachmentModels, i )
			{
				if ( s_vecActiveAttachmentModels[i] == pBaseEntity )
				{
					if ( s_vecActiveAttachmentModels[i]->GetOverrideBaseTexture() )
					{
						pTargetTexture = s_vecActiveAttachmentModels[i]->GetOverrideBaseTexture();
					}
					break;
				}
			}
		}

		if ( pTargetTexture && m_pBaseTextureVar->GetTextureValue() != pTargetTexture )
		{
			m_pBaseTextureVar->SetTextureValue( pTargetTexture );
		}
	}

	virtual IMaterial *GetMaterial()
	{
		return m_pBaseTextureVar ? m_pBaseTextureVar->GetOwningMaterial() : NULL;
	}

private:
	IMaterialVar *m_pBaseTextureVar;
	ITexture *m_pDefaultTexture;
};

EXPOSE_INTERFACE( CArmSkinTextureProxy, IMaterialProxy, "ArmSkinTexture" IMATERIAL_PROXY_INTERFACE_VERSION );

#ifdef CSTRIKE_DLL
struct AK47CosmeticEntry_t
{
	const char *pszKey;
	const char *pszDisplayName;
};

static const AK47CosmeticEntry_t s_AK47Skins[] =
{
	{ "bloodsport",			"Bloodsport" },
	{ "vulcan",				"Vulcan" },
	{ "asiimov",			"Asiimov" },
	{ "redline",			"Redline" },
	{ "fireserpent",		"Fire Serpent" },
	{ "empress",			"The Empress" },
	{ "neon_rider",			"Neon Rider" },
	{ "aquamarine",			"Aquamarine Revenge" },
	{ "fuel_injector",		"Fuel Injector" },
	{ "frontside_misty",	"Frontside Misty" },
	{ "wild_lotus",			"Wild Lotus" },
	{ "gold_arabesque",		"Gold Arabesque" },
	{ "neon_revolution",	"Neon Revolution" },
	{ "nightwish",			"Nightwish" },
	{ "head_shot",			"Head Shot" },
	{ "ice_coaled",			"Ice Coaled" },
	{ "leet_museo",			"Leet Museo" },
	{ "slate",				"Slate" },
	{ "xray",				"X-Ray" },
	{ "anubis",				"Legion of Anubis" },
	{ "point_disarray",		"Point Disarray" },
	{ "wasteland_rebel",	"Wasteland Rebel" },
	{ "jaguar",				"Jaguar" },
	{ "panthera_onca",		"Panthera onca" },
	{ "cartel",				"Cartel" },
	{ "elite_build",		"Elite Build" },
	{ "phantom_disruptor",	"Phantom Disruptor" },
	{ "jet_set",			"Jet Set" },
	{ "first_class",		"First Class" },
	{ "emerald_pinstripe",	"Emerald Pinstripe" },
	{ "orbit_mk01",			"Orbit Mk01" },
	{ "rat_rod",			"Rat Rod" },
	{ "uncharted",			"Uncharted" },
	{ "steel_delta",		"Steel Delta" },
};

static const AK47CosmeticEntry_t s_AK47Stickers[] =
{
	{ "ibuypower",			"iBUYPOWER (Katowice 2014)" },
	{ "titan",				"Titan (Katowice 2014)" },
	{ "crown",				"Crown" },
	{ "howling_dawn",		"Howling Dawn" },
	{ "reason",				"Reason Gaming (Katowice 2014)" },
	{ "lgb",				"LGB eSports (Katowice 2014)" },
	{ "navi_kato14",		"Natus Vincere (Katowice 2014)" },
	{ "virtuspro_kato14",	"Virtus.pro (Katowice 2014)" },
	{ "fnatic_kato14",		"Fnatic (Katowice 2014)" },
	{ "vox",				"Vox Eminor (Katowice 2014)" },
	{ "dignitas",			"Team Dignitas (Katowice 2014)" },
	{ "ldlc",				"Team LDLC (Katowice 2014)" },
	{ "nip_kato14",			"Ninjas in Pyjamas (Katowice 2014)" },
	{ "hellraisers",		"HellRaisers (Katowice 2014)" },
	{ "complexity",			"compLexity (Katowice 2014)" },
	{ "3dmax",				"3DMAX (Katowice 2014)" },
	{ "mousesports",		"mousesports (Katowice 2014)" },
	{ "mystik",				"Clan-Mystik (Katowice 2014)" },
	{ "flammable",			"Flammable" },
	{ "headhunter",			"Headhunter" },
	{ "swag",				"Swag" },
	{ "new_sheriff",		"New Sheriff" },
	{ "firestarter",		"Firestarter" },
	{ "dragon_lore",		"Dragon Lore" },
	{ "gold_web",			"Gold Web" },
	{ "battle_scarred",		"Battle Scarred" },
	{ "broken_fang",		"Broken Fang" },
	{ "ak47_recoil",		"AK-47 Recoil" },
	{ "ak47_recoil_gold",	"AK-47 Recoil (Gold)" },
	{ "baby_howl",			"Baby Howl" },
	{ "baby_lore",			"Baby Lore" },
	{ "baby_fire_serpent",	"Baby Fire Serpent" },
	{ "blue_gem",			"Blue Gem" },
	{ "bomb_doge",			"Bomb Doge" },
	{ "drug_war",			"Drug War Veteran" },
	{ "welcome_clutch",		"Welcome to the Clutch" },
	{ "bish",				"Bish" },
	{ "bash",				"Bash" },
	{ "bosh",				"Bosh" },
	{ "lambda",				"Half-Life Lambda" },
};

static ConVar * const s_pAK47StickerConVars[4] =
{
	&cl_ak47_sticker_1,
	&cl_ak47_sticker_2,
	&cl_ak47_sticker_3,
	&cl_ak47_sticker_4,
};

static const char * const s_pszAK47StickerModels[4] =
{
	"models/advanced_inventory_csgo_items/weapons/stickers/v_models/rif_ak47_decal_a.mdl",
	"models/advanced_inventory_csgo_items/weapons/stickers/v_models/rif_ak47_decal_b.mdl",
	"models/advanced_inventory_csgo_items/weapons/stickers/v_models/rif_ak47_decal_c.mdl",
	"models/advanced_inventory_csgo_items/weapons/stickers/v_models/rif_ak47_decal_d.mdl",
};

static bool MatchesNormalizedSkinToken( const char *pszInput, const char *pszCandidate )
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

static const char *ResolveAK47TableKey( const char *pszChoice, const AK47CosmeticEntry_t *pEntries, int nCount )
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
		if ( MatchesNormalizedSkinToken( pszChoice, pEntries[i].pszKey ) ||
			 MatchesNormalizedSkinToken( pszChoice, pEntries[i].pszDisplayName ) )
			return pEntries[i].pszKey;
	}

	return NULL;
}

static bool ResolveAK47SkinTexturePath( const char *pszChoice, char *pszOutPath, int nOutSize )
{
	const char *pszKey = ResolveAK47TableKey( pszChoice, s_AK47Skins, ARRAYSIZE( s_AK47Skins ) );
	if ( !pszKey )
		return false;

	V_snprintf( pszOutPath, nOutSize, "advanced_inventory_csgo_items/models/weapons/v_models/rif_ak47/skins/%s", pszKey );
	return true;
}

static bool ResolveAK47StickerTexturePath( const char *pszChoice, char *pszOutPath, int nOutSize )
{
	const char *pszKey = ResolveAK47TableKey( pszChoice, s_AK47Stickers, ARRAYSIZE( s_AK47Stickers ) );
	if ( !pszKey )
		return false;

	V_snprintf( pszOutPath, nOutSize, "advanced_inventory_csgo_items/models/weapons/customization/stickers/%s", pszKey );
	return true;
}

struct ViewmodelStickerSlotState_t
{
	CHandle<C_BaseViewModel> hViewModel;
	CHandle<C_ViewmodelAttachmentModel> hStickers[4];
};

static CUtlVector< ViewmodelStickerSlotState_t > s_vecViewmodelStickers;

static int FindOrCreateStickerStateIndex( C_BaseViewModel *pVM, bool bCreate )
{
	FOR_EACH_VEC_BACK( s_vecViewmodelStickers, i )
	{
		C_BaseViewModel *pExisting = s_vecViewmodelStickers[i].hViewModel.Get();
		if ( !pExisting )
		{
			for ( int s = 0; s < 4; ++s )
			{
				if ( C_ViewmodelAttachmentModel *pStk = s_vecViewmodelStickers[i].hStickers[s].Get() )
				{
					pStk->Remove();
				}
			}
			s_vecViewmodelStickers.Remove( i );
			continue;
		}
		if ( pExisting == pVM )
			return i;
	}

	if ( !bCreate || !pVM )
		return -1;

	int idx = s_vecViewmodelStickers.AddToTail();
	s_vecViewmodelStickers[idx].hViewModel = pVM;
	return idx;
}

static void RemoveViewmodelStickers( C_BaseViewModel *pVM )
{
	int idx = FindOrCreateStickerStateIndex( pVM, false );
	if ( idx == -1 )
		return;

	for ( int s = 0; s < 4; ++s )
	{
		if ( C_ViewmodelAttachmentModel *pStk = s_vecViewmodelStickers[idx].hStickers[s].Get() )
		{
			pStk->Remove();
		}
		s_vecViewmodelStickers[idx].hStickers[s] = NULL;
	}
	s_vecViewmodelStickers.Remove( idx );
}

static bool IsAK47ViewmodelActive( C_BaseViewModel *pVM )
{
	if ( !pVM )
		return false;

	C_WeaponCSBase *pCSWeapon = dynamic_cast<C_WeaponCSBase *>( pVM->GetOwningWeapon() );
	return ( pCSWeapon && pCSWeapon->IsA( WEAPON_AK47 ) );
}

static void UpdateViewmodelStickers( C_BaseViewModel *pVM, C_CSPlayer *pPlayer )
{
	if ( !pVM || !pPlayer || !pPlayer->IsAlive() || !IsAK47ViewmodelActive( pVM ) )
	{
		RemoveViewmodelStickers( pVM );
		return;
	}

	int idx = FindOrCreateStickerStateIndex( pVM, false );
	bool bNeedAny = false;
	bool bWantSlot[4] = { false, false, false, false };
	char szTexPath[MAX_PATH];
	for ( int s = 0; s < 4; ++s )
	{
		bWantSlot[s] = ResolveAK47StickerTexturePath( s_pAK47StickerConVars[s]->GetString(), szTexPath, sizeof( szTexPath ) );
		if ( bWantSlot[s] )
		{
			bNeedAny = true;
		}
	}

	if ( !bNeedAny )
	{
		RemoveViewmodelStickers( pVM );
		return;
	}

	if ( idx == -1 )
	{
		idx = FindOrCreateStickerStateIndex( pVM, true );
	}

	for ( int s = 0; s < 4; ++s )
	{
		C_ViewmodelAttachmentModel *pExisting = s_vecViewmodelStickers[idx].hStickers[s].Get();
		if ( !bWantSlot[s] )
		{
			if ( pExisting )
			{
				pExisting->Remove();
				s_vecViewmodelStickers[idx].hStickers[s] = NULL;
			}
			continue;
		}

		if ( pExisting )
			continue;

		const char *pszModelPath = s_pszAK47StickerModels[s];
		if ( modelinfo->GetModelIndex( pszModelPath ) == -1 && ( !filesystem || !filesystem->FileExists( pszModelPath, "GAME" ) ) )
			continue;

		C_ViewmodelAttachmentModel *pEnt = new C_ViewmodelAttachmentModel;
		if ( !pEnt )
			continue;

		if ( pEnt->InitializeArmModel( pszModelPath, 0, false, NULL ) )
		{
			s_vecViewmodelStickers[idx].hStickers[s] = pEnt;
			pEnt->SetParent( pVM );
			pEnt->SetLocalOrigin( vec3_origin );
			pEnt->UpdatePartitionListEntry();
			pEnt->CollisionProp()->MarkPartitionHandleDirty();
			pEnt->UpdateVisibility();
			pEnt->SetViewmodel( pVM );
		}
		else
		{
			pEnt->Release();
		}
	}
}

static void DrawViewmodelStickers( C_BaseViewModel *pVM, int flags )
{
	if ( !pVM || !flags || !IsAK47ViewmodelActive( pVM ) )
		return;

	int idx = FindOrCreateStickerStateIndex( pVM, false );
	if ( idx == -1 )
		return;

	for ( int s = 0; s < 4; ++s )
	{
		C_ViewmodelAttachmentModel *pStk = s_vecViewmodelStickers[idx].hStickers[s].Get();
		if ( !pStk )
			continue;

		if ( pStk->GetMoveParent() != pVM )
		{
			pStk->AddEffects( EF_BONEMERGE | EF_BONEMERGE_FASTCULL );
			pStk->SetParent( pVM );
		}

		pStk->DrawModel( flags );
	}
}

typedef bool (*ResolveWeaponTexturePathFn)( const char *pszChoice, char *pszOutPath, int nOutSize );

class CBaseWeaponConVarTextureProxy : public CEntityMaterialProxy
{
public:
	CBaseWeaponConVarTextureProxy() : m_pBaseTextureVar( NULL ), m_pDefaultTexture( NULL ), m_pOverrideTexture( NULL )
	{
		m_szLastChoice[0] = '\0';
	}

	virtual ~CBaseWeaponConVarTextureProxy()
	{
		ReleaseCachedOverride();
		if ( m_pDefaultTexture )
		{
			m_pDefaultTexture->DecrementReferenceCount();
			m_pDefaultTexture = NULL;
		}
	}

	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues )
	{
		bool bFound = false;
		m_pBaseTextureVar = pMaterial->FindVar( "$basetexture", &bFound, false );
		if ( !bFound || !m_pBaseTextureVar )
			return false;

		if ( m_pBaseTextureVar->IsTexture() )
		{
			m_pDefaultTexture = m_pBaseTextureVar->GetTextureValue();
			if ( m_pDefaultTexture )
			{
				m_pDefaultTexture->IncrementReferenceCount();
			}
		}

		return true;
	}

	virtual IMaterial *GetMaterial()
	{
		return m_pBaseTextureVar ? m_pBaseTextureVar->GetOwningMaterial() : NULL;
	}

protected:
	void ReleaseCachedOverride()
	{
		if ( m_pOverrideTexture )
		{
			m_pOverrideTexture->DecrementReferenceCount();
			m_pOverrideTexture = NULL;
		}
	}

	bool PrepareBaseTexture()
	{
		if ( !m_pBaseTextureVar || !m_pBaseTextureVar->IsTexture() )
			return false;

		if ( !m_pDefaultTexture )
		{
			m_pDefaultTexture = m_pBaseTextureVar->GetTextureValue();
			if ( m_pDefaultTexture )
			{
				m_pDefaultTexture->IncrementReferenceCount();
			}
		}

		return true;
	}

	void UpdateOverrideFromChoice( const char *pszChoice, ResolveWeaponTexturePathFn pfnResolve )
	{
		if ( !V_strcmp( m_szLastChoice, pszChoice ) )
			return;

		V_strncpy( m_szLastChoice, pszChoice, sizeof( m_szLastChoice ) );
		ReleaseCachedOverride();

		char szTexturePath[MAX_PATH];
		if ( pfnResolve( pszChoice, szTexturePath, sizeof( szTexturePath ) ) )
		{
			ITexture *pTex = materials->FindTexture( szTexturePath, TEXTURE_GROUP_MODEL, false );
			if ( pTex && !IsErrorTexture( pTex ) )
			{
				pTex->IncrementReferenceCount();
				m_pOverrideTexture = pTex;
			}
		}
	}

	void ApplyTargetTexture( bool bUseOverride )
	{
		ITexture *pTargetTexture = ( bUseOverride && m_pOverrideTexture ) ? m_pOverrideTexture : m_pDefaultTexture;
		if ( pTargetTexture && m_pBaseTextureVar->GetTextureValue() != pTargetTexture )
		{
			m_pBaseTextureVar->SetTextureValue( pTargetTexture );
		}
	}

	IMaterialVar *m_pBaseTextureVar;
	ITexture *m_pDefaultTexture;
	ITexture *m_pOverrideTexture;
	char m_szLastChoice[128];
};

class CWeaponSkinTextureProxy : public CBaseWeaponConVarTextureProxy
{
public:
	virtual void OnBind( C_BaseEntity *pBaseEntity )
	{
		if ( !PrepareBaseTexture() )
			return;

		bool bApply = ShouldApplyAK47Skin( pBaseEntity );
		if ( bApply )
		{
			UpdateOverrideFromChoice( cl_ak47_skin.GetString(), ResolveAK47SkinTexturePath );
		}
		ApplyTargetTexture( bApply );
	}

private:
	bool ShouldApplyAK47Skin( C_BaseEntity *pBaseEntity ) const
	{
		if ( !pBaseEntity )
			return false;

		if ( C_BaseViewModel *pVM = dynamic_cast<C_BaseViewModel *>( pBaseEntity ) )
			return IsAK47ViewmodelActive( pVM );

		if ( C_WeaponCSBase *pCSWeapon = dynamic_cast<C_WeaponCSBase *>( pBaseEntity ) )
		{
			if ( !pCSWeapon->IsA( WEAPON_AK47 ) )
				return false;

			C_BasePlayer *pLocal = C_BasePlayer::GetLocalPlayer();
			C_BasePlayer *pOwner = pCSWeapon->GetPlayerOwner();
			return ( !pOwner || pOwner == pLocal );
		}

		return false;
	}
};

class CWeaponStickerTextureProxy : public CBaseWeaponConVarTextureProxy
{
public:
	CWeaponStickerTextureProxy() : m_nSlot( 0 ) {}

	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues )
	{
		if ( !CBaseWeaponConVarTextureProxy::Init( pMaterial, pKeyValues ) )
			return false;

		m_nSlot = pKeyValues ? clamp( pKeyValues->GetInt( "slot", 0 ), 0, 3 ) : 0;
		return true;
	}

	virtual void OnBind( C_BaseEntity *pBaseEntity )
	{
		if ( !PrepareBaseTexture() )
			return;

		UpdateOverrideFromChoice( s_pAK47StickerConVars[m_nSlot]->GetString(), ResolveAK47StickerTexturePath );
		ApplyTargetTexture( m_pOverrideTexture != NULL );
	}

private:
	int m_nSlot;
};

EXPOSE_INTERFACE( CWeaponSkinTextureProxy, IMaterialProxy, "WeaponSkinTexture" IMATERIAL_PROXY_INTERFACE_VERSION );
EXPOSE_INTERFACE( CWeaponStickerTextureProxy, IMaterialProxy, "WeaponStickerTexture" IMATERIAL_PROXY_INTERFACE_VERSION );
#endif
