////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2011.
// -------------------------------------------------------------------------
//  File name:   UIManager.cpp
//  Version:     v1.00
//  Created:     08/8/2011 by Paul Reindell.
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "UIManager.h"

#include "UIInput.h"
#include "UIObjectives.h"
#include "UISettings.h"
#include "UIMultiPlayer.h"
#include "UIMenuEvents.h"
#include "UIHUD3D.h"
#include "UIEntityDynTexTag.h"

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// Singleton ///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

CUIManager* CUIManager::m_pInstance = NULL;

void CUIManager::Init()
{ 
	assert( m_pInstance == NULL );
	if ( !m_pInstance && !gEnv->IsDedicated()  )
		m_pInstance = new CUIManager();
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIManager::Destroy()
{
	SAFE_DELETE( m_pInstance );
}

/////////////////////////////////////////////////////////////////////////////////////
CUIManager* CUIManager::GetInstance()
{
	if ( !m_pInstance )
		Init();
	return m_pInstance;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// CTor/DTor ///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
CUIManager::CUIManager()
	: m_bPickupMsgVisible(false)
{
	m_pUIInput = new CUIInput();
	m_pUIObjectives = new CUIObjectives();
	m_pUISettings = new CUISettings();
	m_pUIMultiPlayer = new CUIMultiPlayer();
	m_pUIMenuEvents = new CUIMenuEvents();
	m_pHUD3D = new CUIHUD3D();
	m_pUIEntityDynTexTag = new CUIEntityDynTexTag();

	m_soundListener = gEnv->pSoundSystem->CreateListener();
	InitSound();
	gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener( this );

	LoadProfile();
}

/////////////////////////////////////////////////////////////////////////////////////
CUIManager::~CUIManager()
{
	gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener( this );
	SAFE_DELETE( m_pUIInput );
	SAFE_DELETE( m_pUIObjectives );
	SAFE_DELETE( m_pUISettings );
	SAFE_DELETE( m_pUIMultiPlayer );
	SAFE_DELETE( m_pUIMenuEvents );
	SAFE_DELETE( m_pHUD3D );
	SAFE_DELETE( m_pUIEntityDynTexTag );
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIManager::ProcessViewParams(const SViewParams &viewParams)
{
	m_pHUD3D->UpdateView(viewParams);
	m_pUIEntityDynTexTag->UpdateView(viewParams);
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIManager::UpdatePickupMessage(bool bShow)
{
	if (!gEnv->pFlashUI) return;

	if (m_bPickupMsgVisible != bShow)
	{
		m_bPickupMsgVisible = bShow;
		static IUIAction* pAction = gEnv->pFlashUI->GetUIAction("DisplayPickupText");
		if (pAction)
		{
			SUIArguments args;
			args.AddArgument(bShow ? "@ui_pickup" : "");
			gEnv->pFlashUI->GetUIActionManager()->StartAction(pAction, args);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIManager::OnSystemEvent( ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam )
{
	if (event == ESYSTEM_EVENT_LEVEL_POST_UNLOAD)
	{
		InitSound();
	}
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIManager::InitSound()
{
	if (m_soundListener != LISTENERID_INVALID)
	{
		IListener *pListener = gEnv->pSoundSystem->GetListener(m_soundListener);

		if (pListener)
		{
			pListener->SetRecordLevel(1.0f);
			pListener->SetActive(true);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIManager::LoadProfile()
{
	IPlayerProfile* pProfile = GetCurrentProfile();
	if (!pProfile)
	{
		assert(false);
		return;
	}

	m_pUIMultiPlayer->LoadProfile( pProfile );
	m_pUISettings->LoadProfile( pProfile );
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIManager::SaveProfile()
{
	IPlayerProfile* pProfile = GetCurrentProfile();
	if (!pProfile)
	{
		assert(false);
		return;
	}

	m_pUIMultiPlayer->SaveProfile( pProfile );
	m_pUISettings->SaveProfile( pProfile );
}

IPlayerProfile* CUIManager::GetCurrentProfile()
{
	if (!gEnv->pGame || !gEnv->pGame->GetIGameFramework() || !gEnv->pGame->GetIGameFramework()->GetIPlayerProfileManager())
	{
		assert(false);
		return NULL;
	}

	IPlayerProfileManager* pProfileManager = gEnv->pGame->GetIGameFramework()->GetIPlayerProfileManager();
	return pProfileManager->GetCurrentProfile( pProfileManager->GetCurrentUser() );
}
