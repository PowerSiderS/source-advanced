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
	#include "cs_inventory.h"
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
#ifdef CSTRIKE_DLL
	if ( ( flags & STUDIO_RENDER ) && pPlayer && GetOwner() == pPlayer )
	{
		UpdateAllViewmodelAddons();
	}
#endif
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

	Inventory_SyncActiveTeamConVars( pPlayer->GetTeamNumber() );

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
		int nSkintone = m_nPendingSkintone;
		if ( nSkintone == 7 ) // BARE_ARM_44 enum index maps to skin 6 on 7-skin MDLs
		{
			nSkintone = 6;
		}
		if ( CStudioHdr *pHdr = GetModelPtr() )
		{
			int nSkins = Max( 1, pHdr->numskinfamilies() );
			if ( nSkintone >= nSkins )
			{
				nSkintone = ( nSkintone == 6 && nSkins == 6 ) ? 1 : 0;
			}
			nSkintone = clamp( nSkintone, 0, nSkins - 1 );
		}
		m_nSkin = nSkintone;
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

class CArmSkinTextureProxy : public IMaterialProxy
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

	virtual void Release()
	{
		delete this;
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

	virtual void OnBind( void *pC_BaseEntity )
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

		char szOverridePath[MAX_PATH];
		if ( pC_BaseEntity && Inventory_ResolvePreviewGloveOverrideTexture( pC_BaseEntity, szOverridePath, sizeof( szOverridePath ) ) )
		{
			ITexture *pPreviewTex = materials->FindTexture( szOverridePath, TEXTURE_GROUP_MODEL, false );
			if ( pPreviewTex && !pPreviewTex->IsError() )
			{
				pTargetTexture = pPreviewTex;
			}
		}
		else if ( pC_BaseEntity )
		{
			IClientRenderable *pRend = reinterpret_cast<IClientRenderable *>( pC_BaseEntity );
			C_BaseEntity *pBaseEntity = ( pRend && pRend->GetIClientUnknown() ) ? pRend->GetIClientUnknown()->GetBaseEntity() : NULL;
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
struct ViewmodelStickerSlotState_t
{
	CHandle<C_BaseViewModel> hViewModel;
	int nWeaponIdx;
	int nLastTeam;
	unsigned int nLastLoadoutRev;
	CHandle<C_ViewmodelAttachmentModel> hStickers[INV_MAX_STICKER_SLOTS];
};

static CUtlVector< ViewmodelStickerSlotState_t > s_vecViewmodelStickers;

static int FindOrCreateStickerStateIndex( C_BaseViewModel *pVM, bool bCreate )
{
	FOR_EACH_VEC_BACK( s_vecViewmodelStickers, i )
	{
		C_BaseViewModel *pExisting = s_vecViewmodelStickers[i].hViewModel.Get();
		if ( !pExisting )
		{
			for ( int s = 0; s < INV_MAX_STICKER_SLOTS; ++s )
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
	s_vecViewmodelStickers[idx].nWeaponIdx = -1;
	s_vecViewmodelStickers[idx].nLastTeam = -1;
	s_vecViewmodelStickers[idx].nLastLoadoutRev = 0;
	return idx;
}

static void RemoveViewmodelStickers( C_BaseViewModel *pVM )
{
	int idx = FindOrCreateStickerStateIndex( pVM, false );
	if ( idx == -1 )
		return;

	for ( int s = 0; s < INV_MAX_STICKER_SLOTS; ++s )
	{
		if ( C_ViewmodelAttachmentModel *pStk = s_vecViewmodelStickers[idx].hStickers[s].Get() )
		{
			pStk->Remove();
		}
		s_vecViewmodelStickers[idx].hStickers[s] = NULL;
	}
	s_vecViewmodelStickers.Remove( idx );
}

static int GetCustomizableViewmodelWeaponIndex( C_BaseViewModel *pVM )
{
	if ( !pVM )
		return -1;

	C_WeaponCSBase *pCSWeapon = dynamic_cast<C_WeaponCSBase *>( pVM->GetOwningWeapon() );
	if ( !pCSWeapon )
		return -1;

	if ( pCSWeapon->IsA( WEAPON_M4A1 ) )
		return INV_SKIN_WEP_M4A1;

	static const struct
	{
		int eWeaponID;
		int nSkinWep;
	} s_WeaponMap[] =
	{
		{ WEAPON_AK47,      INV_SKIN_WEP_AK47 },
		{ WEAPON_AWP,       INV_SKIN_WEP_AWP },
		{ WEAPON_DEAGLE,    INV_SKIN_WEP_DEAGLE },
		{ WEAPON_GLOCK,     INV_SKIN_WEP_GLOCK },
		{ WEAPON_USP,       INV_SKIN_WEP_USP },
		{ WEAPON_MAC10,     INV_SKIN_WEP_MAC10 },
		{ WEAPON_TMP,       INV_SKIN_WEP_MP9 },
		{ WEAPON_P90,       INV_SKIN_WEP_P90 },
		{ WEAPON_M3,        INV_SKIN_WEP_NOVA },
		{ WEAPON_XM1014,    INV_SKIN_WEP_XM1014 },
		{ WEAPON_P228,      INV_SKIN_WEP_P228 },
		{ WEAPON_ELITE,     INV_SKIN_WEP_ELITE },
		{ WEAPON_FIVESEVEN, INV_SKIN_WEP_FIVESEVEN },
		{ WEAPON_MP5NAVY,   INV_SKIN_WEP_MP5 },
		{ WEAPON_UMP45,     INV_SKIN_WEP_UMP45 },
		{ WEAPON_GALIL,     INV_SKIN_WEP_GALIL },
		{ WEAPON_FAMAS,     INV_SKIN_WEP_FAMAS },
		{ WEAPON_SCOUT,     INV_SKIN_WEP_SCOUT },
		{ WEAPON_SG550,     INV_SKIN_WEP_SG550 },
		{ WEAPON_SG552,     INV_SKIN_WEP_SG552 },
		{ WEAPON_G3SG1,     INV_SKIN_WEP_G3SG1 },
		{ WEAPON_AUG,       INV_SKIN_WEP_AUG },
		{ WEAPON_M249,      INV_SKIN_WEP_M249 },
	};

	for ( int i = 0; i < ARRAYSIZE( s_WeaponMap ); ++i )
	{
		if ( pCSWeapon->IsA( (CSWeaponID)s_WeaponMap[i].eWeaponID ) )
			return s_WeaponMap[i].nSkinWep;
	}

	return -1;
}

static void UpdateViewmodelStickers( C_BaseViewModel *pVM, C_CSPlayer *pPlayer )
{
	int nCurWep = GetCustomizableViewmodelWeaponIndex( pVM );
	if ( !pVM || !pPlayer || !pPlayer->IsAlive() || nCurWep < 0 )
	{
		RemoveViewmodelStickers( pVM );
		return;
	}

	int nTeam = pPlayer->GetTeamNumber();
	unsigned int nLoadoutRev = Inventory_GetLoadoutRevision();
	int idx = FindOrCreateStickerStateIndex( pVM, false );
	if ( idx != -1 &&
		 s_vecViewmodelStickers[idx].nWeaponIdx == nCurWep &&
		 s_vecViewmodelStickers[idx].nLastTeam == nTeam &&
		 s_vecViewmodelStickers[idx].nLastLoadoutRev == nLoadoutRev )
	{
		return;
	}

	if ( idx != -1 && s_vecViewmodelStickers[idx].nWeaponIdx != nCurWep )
	{
		RemoveViewmodelStickers( pVM );
		idx = -1;
	}

	bool bNeedAny = false;
	bool bWantSlot[INV_MAX_STICKER_SLOTS] = { false, false, false, false };
	Inventory_GetActiveStickersForViewmodel( nCurWep, pVM, pPlayer, bWantSlot );
	for ( int s = 0; s < INV_MAX_STICKER_SLOTS; ++s )
	{
		if ( bWantSlot[s] )
		{
			bNeedAny = true;
			break;
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
		if ( idx == -1 )
			return;
		s_vecViewmodelStickers[idx].nWeaponIdx = nCurWep;
	}
	s_vecViewmodelStickers[idx].nLastTeam = nTeam;
	s_vecViewmodelStickers[idx].nLastLoadoutRev = nLoadoutRev;

	for ( int s = 0; s < INV_MAX_STICKER_SLOTS; ++s )
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

		char szModelPath[MAX_PATH];
		Inventory_GetWeaponStickerModelPath( nCurWep, s, szModelPath, sizeof( szModelPath ) );
		if ( !szModelPath[0] )
			continue;
		if ( modelinfo->GetModelIndex( szModelPath ) == -1 && ( !filesystem || !filesystem->FileExists( szModelPath, "GAME" ) ) )
			continue;

		C_ViewmodelAttachmentModel *pEnt = new C_ViewmodelAttachmentModel;
		if ( !pEnt )
			continue;

		if ( pEnt->InitializeArmModel( szModelPath, 0, false, NULL ) )
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
	int nCurWep = GetCustomizableViewmodelWeaponIndex( pVM );
	if ( !pVM || !flags || nCurWep < 0 )
		return;

	int idx = FindOrCreateStickerStateIndex( pVM, false );
	if ( idx == -1 || s_vecViewmodelStickers[idx].nWeaponIdx != nCurWep )
		return;

	for ( int s = 0; s < INV_MAX_STICKER_SLOTS; ++s )
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

static int DetectWeaponIndexFromMaterial( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	static const struct
	{
		const char *pszKVName;
		const char *pszMatSubstr;
		int nSkinWep;
	} s_MatMap[] =
	{
		{ "awp",       "snip_awp",      INV_SKIN_WEP_AWP },
		{ "deagle",    "pist_deagle",   INV_SKIN_WEP_DEAGLE },
		{ "m4a1_s",    "rif_m4a1_s",    INV_SKIN_WEP_M4A1 },
		{ "m4a1",      "rif_m4a1_s",    INV_SKIN_WEP_M4A1 },
		{ "ak47",      "rif_ak47",      INV_SKIN_WEP_AK47 },
		{ "glock",     "pist_glock18",  INV_SKIN_WEP_GLOCK },
		{ "usp",       "pist_223",      INV_SKIN_WEP_USP },
		{ "mac10",     "smg_mac10",     INV_SKIN_WEP_MAC10 },
		{ "mp9",       "smg_mp9",       INV_SKIN_WEP_MP9 },
		{ "p90",       "smg_p90",       INV_SKIN_WEP_P90 },
		{ "nova",      "shot_nova",     INV_SKIN_WEP_NOVA },
		{ "xm1014",    "shot_xm1014",   INV_SKIN_WEP_XM1014 },
		{ "p228",      "pist_p250",     INV_SKIN_WEP_P228 },
		{ "elite",     "pist_elite",    INV_SKIN_WEP_ELITE },
		{ "fiveseven", "pist_fiveseven",INV_SKIN_WEP_FIVESEVEN },
		{ "mp5",       "smg_mp5sd",     INV_SKIN_WEP_MP5 },
		{ "ump45",     "smg_ump45",     INV_SKIN_WEP_UMP45 },
		{ "galil",     "rif_galilar",   INV_SKIN_WEP_GALIL },
		{ "famas",     "rif_famas",     INV_SKIN_WEP_FAMAS },
		{ "scout",     "snip_ssg08",    INV_SKIN_WEP_SCOUT },
		{ "sg550",     "snip_scar20",   INV_SKIN_WEP_SG550 },
		{ "sg552",     "rif_sg556",     INV_SKIN_WEP_SG552 },
		{ "g3sg1",     "snip_g3sg1",    INV_SKIN_WEP_G3SG1 },
		{ "aug",       "rif_aug",       INV_SKIN_WEP_AUG },
		{ "m249",      "mach_m249para", INV_SKIN_WEP_M249 },
		{ "knife_bayonet",         "knife_bayonet",           INV_KNIFE_HINT_BASE + 1 },
		{ "knife_css",             "knife_css",               INV_KNIFE_HINT_BASE + 2 },
		{ "knife_flip",            "knife_flip",              INV_KNIFE_HINT_BASE + 3 },
		{ "knife_gut",             "knife_gut",               INV_KNIFE_HINT_BASE + 4 },
		{ "knife_karambit",        "knife_karam",             INV_KNIFE_HINT_BASE + 5 },
		{ "knife_m9_bayonet",      "knife_m9_bay",            INV_KNIFE_HINT_BASE + 6 },
		{ "knife_tactical",        "knife_tactical",          INV_KNIFE_HINT_BASE + 7 },
		{ "knife_falchion",        "knife_falchion_advanced", INV_KNIFE_HINT_BASE + 8 },
		{ "knife_survival_bowie",  "knife_survival_bowie",    INV_KNIFE_HINT_BASE + 9 },
		{ "knife_butterfly",       "knife_butterfly",         INV_KNIFE_HINT_BASE + 10 },
		{ "knife_push",            "knife_push",              INV_KNIFE_HINT_BASE + 11 },
		{ "knife_cord",            "knife_cord",              INV_KNIFE_HINT_BASE + 12 },
		{ "knife_canis",           "knife_canis",             INV_KNIFE_HINT_BASE + 13 },
		{ "knife_ursus",           "knife_ursus",             INV_KNIFE_HINT_BASE + 14 },
		{ "knife_gypsy_jackknife", "knife_gypsy_jack",        INV_KNIFE_HINT_BASE + 15 },
		{ "knife_outdoor",         "knife_outdoor",           INV_KNIFE_HINT_BASE + 16 },
		{ "knife_stiletto",        "knife_stiletto",          INV_KNIFE_HINT_BASE + 17 },
		{ "knife_widowmaker",      "knife_widowmaker",        INV_KNIFE_HINT_BASE + 18 },
		{ "knife_skeleton",        "knife_skeleton",          INV_KNIFE_HINT_BASE + 19 },
	};

	const char *pszWepKV = pKeyValues ? pKeyValues->GetString( "weapon", "" ) : "";
	if ( pszWepKV && pszWepKV[0] )
	{
		for ( int i = 0; i < ARRAYSIZE( s_MatMap ); ++i )
		{
			if ( !V_stricmp( pszWepKV, s_MatMap[i].pszKVName ) )
				return s_MatMap[i].nSkinWep;
		}
	}

	const char *pszMatName = pMaterial ? pMaterial->GetName() : NULL;
	if ( pszMatName && pszMatName[0] )
	{
		for ( int i = 0; i < ARRAYSIZE( s_MatMap ); ++i )
		{
			if ( V_stristr( pszMatName, s_MatMap[i].pszMatSubstr ) )
				return s_MatMap[i].nSkinWep;
		}
	}

	return -1;
}

class CBaseWeaponConVarTextureProxy : public IMaterialProxy
{
public:
	CBaseWeaponConVarTextureProxy() : m_pBaseTextureVar( NULL ), m_pDefaultTexture( NULL ), m_pOverrideTexture( NULL ), m_nWeaponIdx( -1 )
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

	virtual void Release()
	{
		delete this;
	}

	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues )
	{
		bool bFound = false;
		m_pBaseTextureVar = pMaterial->FindVar( "$basetexture", &bFound, false );
		if ( !bFound || !m_pBaseTextureVar )
			return false;

		m_nWeaponIdx = DetectWeaponIndexFromMaterial( pMaterial, pKeyValues );

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
	virtual bool ResolveChoiceToTexturePath( const char *pszChoice, char *pszOutPath, int nOutSize ) = 0;

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

	void UpdateOverrideFromChoice( const char *pszChoice )
	{
		if ( !V_strcmp( m_szLastChoice, pszChoice ) )
			return;

		V_strncpy( m_szLastChoice, pszChoice, sizeof( m_szLastChoice ) );
		ReleaseCachedOverride();

		char szTexturePath[MAX_PATH];
		if ( ResolveChoiceToTexturePath( pszChoice, szTexturePath, sizeof( szTexturePath ) ) )
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
	int m_nWeaponIdx;
	char m_szLastChoice[128];
};

class CWeaponSkinTextureProxy : public CBaseWeaponConVarTextureProxy
{
public:
	virtual void OnBind( void *pRenderable )
	{
		if ( !PrepareBaseTexture() )
			return;

		char szChoice[128];
		bool bApply = false;
		if ( Inventory_ResolveWeaponSkinChoiceForRenderable( m_nWeaponIdx, pRenderable, szChoice, sizeof( szChoice ), bApply ) && bApply )
		{
			UpdateOverrideFromChoice( szChoice );
		}
		ApplyTargetTexture( bApply );
	}

protected:
	virtual bool ResolveChoiceToTexturePath( const char *pszChoice, char *pszOutPath, int nOutSize )
	{
		return Inventory_ResolveWeaponSkinTexturePath( m_nWeaponIdx, pszChoice, pszOutPath, nOutSize );
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

		m_nSlot = pKeyValues ? clamp( pKeyValues->GetInt( "slot", 0 ), 0, INV_MAX_STICKER_SLOTS - 1 ) : 0;
		return true;
	}

	virtual void OnBind( void *pRenderable )
	{
		if ( !PrepareBaseTexture() )
			return;

		char szChoice[128];
		if ( Inventory_ResolveWeaponStickerChoiceForRenderable( m_nWeaponIdx, pRenderable, m_nSlot, szChoice, sizeof( szChoice ) ) )
		{
			UpdateOverrideFromChoice( szChoice );
		}
		ApplyTargetTexture( m_pOverrideTexture != NULL );
	}

protected:
	virtual bool ResolveChoiceToTexturePath( const char *pszChoice, char *pszOutPath, int nOutSize )
	{
		return Inventory_ResolveStickerTexturePath( pszChoice, pszOutPath, nOutSize );
	}

private:
	int m_nSlot;
};

EXPOSE_INTERFACE( CWeaponSkinTextureProxy, IMaterialProxy, "WeaponSkinTexture" IMATERIAL_PROXY_INTERFACE_VERSION );
EXPOSE_INTERFACE( CWeaponStickerTextureProxy, IMaterialProxy, "WeaponStickerTexture" IMATERIAL_PROXY_INTERFACE_VERSION );
#endif
