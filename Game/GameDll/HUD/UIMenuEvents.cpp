////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2011.
// -------------------------------------------------------------------------
//  File name:   UIMenuEvents.cpp
//  Version:     v1.00
//  Created:     21/11/2011 by Paul Reindell.
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "UIMenuEvents.h"

#include "Game.h"
#include <ILevelSystem.h>

SUIEventHelper<CUIMenuEvents> CUIMenuEvents::s_EventDispatcher;

////////////////////////////////////////////////////////////////////////////
CUIMenuEvents::CUIMenuEvents()
: m_pUIEvents(NULL)
, m_pUIFunctions(NULL)
, m_bIsIngameMenuStarted(false)
{
	if (!gEnv->pFlashUI) return;

	// events to send from this class to UI flowgraphs
	m_pUIFunctions = gEnv->pFlashUI->CreateEventSystem("MenuEvents", IUIEventSystem::eEST_SYSTEM_TO_UI);

	{
		SUIEventDesc eventDesc("OnStartIngameMenu", "OnStartIngameMenu", "Triggered if the IngameMenu should be displayed");
		m_EventMap[eUIE_StartIngameMenu] = m_pUIFunctions->RegisterEvent(eventDesc);
	}

	{
		SUIEventDesc eventDesc("OnStopIngameMenu", "OnStopIngameMenu", "Triggered if the IngameMenu should be hidden");
		m_EventMap[eUIE_StopIngameMenu] = m_pUIFunctions->RegisterEvent(eventDesc);
	}


	// events that can be sent from UI flowgraphs to this class
	m_pUIEvents = gEnv->pFlashUI->CreateEventSystem("MenuEvents", IUIEventSystem::eEST_UI_TO_SYSTEM);
	{
		SUIEventDesc eventDesc("DisplayIngameMenu", "DisplayIngameMenu", "Call this to Display or Hide the IngameMenu");
		eventDesc.Params.push_back(SUIParameterDesc("Display", "Display", "True or false", SUIParameterDesc::eUIPT_Bool));
		s_EventDispatcher.RegisterEvent(m_pUIEvents, eventDesc, &CUIMenuEvents::OnDisplayIngameMenu);
	}

	m_pUIEvents->RegisterListener(this, "CUIMenuEvents");
	gEnv->pFlashUI->RegisterModule(this, "CUIMenuEvents");
}

////////////////////////////////////////////////////////////////////////////
CUIMenuEvents::~CUIMenuEvents()
{
	if (m_pUIEvents) m_pUIEvents->UnregisterListener(this);
	gEnv->pFlashUI->UnregisterModule(this);
}

////////////////////////////////////////////////////////////////////////////
void CUIMenuEvents::Reset()
{
	DisplayIngameMenu(false);
}

////////////////////////////////////////////////////////////////////////////
void CUIMenuEvents::DisplayIngameMenu(bool bDisplay)
{
	if (m_bIsIngameMenuStarted == bDisplay) return;

	m_bIsIngameMenuStarted = bDisplay;
	if (bDisplay)
		StartIngameMenu();
	else
		StopIngameMenu();
}

////////////////////////////////////////////////////////////////////////////
// ui events
////////////////////////////////////////////////////////////////////////////
void CUIMenuEvents::OnEvent( const SUIEvent& event )
{
	s_EventDispatcher.Dispatch( this, event );
}

////////////////////////////////////////////////////////////////////////////
void CUIMenuEvents::OnDisplayIngameMenu( const SUIEvent& event )
{
	bool bDisplay;
	if (event.args.GetArg(0, bDisplay))
	{
		DisplayIngameMenu(bDisplay);
	}
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void CUIMenuEvents::StartIngameMenu()
{
	if (!gEnv->bMultiplayer && !gEnv->IsEditor())
	{
		g_pGame->GetIGameFramework()->PauseGame( true, true );
	}

	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr)
	{
		pAmMgr->Enable(true);
		pAmMgr->EnableFilter("only_ui", true);
	}

	m_pUIFunctions->SendEvent(SUIEvent(m_EventMap[eUIE_StartIngameMenu], SUIArguments()));
}

////////////////////////////////////////////////////////////////////////////
void CUIMenuEvents::StopIngameMenu()
{
	if (!gEnv->bMultiplayer && !gEnv->IsEditor())
	{
		g_pGame->GetIGameFramework()->PauseGame( false, true );
	}

	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr)
	{
		pAmMgr->EnableFilter("only_ui", false);
	}

	m_pUIFunctions->SendEvent(SUIEvent(m_EventMap[eUIE_StopIngameMenu], SUIArguments()));
}