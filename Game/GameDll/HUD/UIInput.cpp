////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2010.
// -------------------------------------------------------------------------
//  File name:   UIInput.cpp
//  Version:     v1.00
//  Created:     17/9/2010 by Paul Reindell.
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "UIInput.h"
#include "UIMenuEvents.h"
#include "UIManager.h"

#include "Game.h"
#include "GameActions.h"

TActionHandler<CUIInput> CUIInput::s_actionHandler;
SUIEventHelper<CUIInput> CUIInput::s_EventDispatcher;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
CUIInput::CUIInput()
	: m_pUIFunctions( NULL )
{
	if ( !gEnv->pFlashUI
		|| !g_pGame->GetIGameFramework() 
		|| !g_pGame->GetIGameFramework()->GetIActionMapManager() )
	{
		assert( false );
		return;
	}

	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	pAmMgr->AddExtraActionListener( this );

	// set up the handlers
	if (s_actionHandler.GetNumHandlers() == 0)
	{
		#define ADD_HANDLER(action, func) s_actionHandler.AddHandler(actions.action, &CUIInput::func)
		const CGameActions& actions = g_pGame->Actions();

		ADD_HANDLER(ui_toggle_pause, OnActionTogglePause);
		ADD_HANDLER(ui_start_pause, OnActionStartPause);

		ADD_HANDLER(ui_up, OnActionUp);
		ADD_HANDLER(ui_down, OnActionDown);
		ADD_HANDLER(ui_left, OnActionLeft);	
		ADD_HANDLER(ui_right, OnActionRight);

		ADD_HANDLER(ui_click, OnActionClick);	
		ADD_HANDLER(ui_back, OnActionBack);	

		ADD_HANDLER(ui_confirm, OnActionConfirm);	
		ADD_HANDLER(ui_reset, OnActionReset);	

		#undef ADD_HANDLER
	}

	// ui events (sent to ui)
	m_pUIEvents = gEnv->pFlashUI->CreateEventSystem( "Input", IUIEventSystem::eEST_SYSTEM_TO_UI );
	{
		SUIEventDesc eventDesc("OnKeyboardDone", "OnKeyboardDone", "triggered once keyboard is done");
		eventDesc.Params.push_back( SUIParameterDesc("String", "String", "String of keyboard input", SUIParameterDesc::eUIPT_String) );
		m_eventMap[eUIE_OnVirtKeyboardDone] = m_pUIEvents->RegisterEvent(eventDesc);
	}

	{
		SUIEventDesc eventDesc("OnKeyboardCancelled", "OnKeyboardCancelled", "triggered once keyboard is cancelled");
		m_eventMap[eUIE_OnVirtKeyboardCancelled] = m_pUIEvents->RegisterEvent(eventDesc);
	}


	// ui fuctions (called from ui)
	m_pUIFunctions = gEnv->pFlashUI->CreateEventSystem( "Input", IUIEventSystem::eEST_UI_TO_SYSTEM );
	{
		SUIEventDesc eventDesc("ShowVirualKeyboard", "ShowVirualKeyboard", "Displays the virtual keyboard");
		eventDesc.Params.push_back( SUIParameterDesc("Title", "Title", "Title for the virtual keyboard", SUIParameterDesc::eUIPT_String) );
		eventDesc.Params.push_back( SUIParameterDesc("InitialStr", "Value", "Initial string of virtual keyboard", SUIParameterDesc::eUIPT_String) );
		eventDesc.Params.push_back( SUIParameterDesc("MaxChars", "MaxChars", "Maximum chars", SUIParameterDesc::eUIPT_Int) );
		s_EventDispatcher.RegisterEvent( m_pUIFunctions, eventDesc, &CUIInput::OnDisplayVirtualKeyboard );
	}
	m_pUIFunctions->RegisterListener( this, "CUIInput" );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
CUIInput::~CUIInput()
{
	if (   gEnv->pGame 
		&& gEnv->pGame->GetIGameFramework() 
		&& gEnv->pGame->GetIGameFramework()->GetIActionMapManager() )
	{
		IActionMapManager* pAmMgr = gEnv->pGame->GetIGameFramework()->GetIActionMapManager();
		pAmMgr->RemoveExtraActionListener( this );
	}
	if ( m_pUIFunctions )
		m_pUIFunctions->UnregisterListener(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUIInput::KeyboardCancelled()
{
	NotifyUI(eUIE_OnVirtKeyboardCancelled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUIInput::KeyboardFinished(const wchar_t *pInString)
{
	SUIArguments args;
	args.AddArgument( pInString );
	NotifyUI(eUIE_OnVirtKeyboardDone, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUIInput::NotifyUI(EUIEvent eventType, const SUIArguments& args)
{
	if( m_pUIEvents )
		m_pUIEvents->SendEvent( SUIEvent(m_eventMap[eventType], args) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////// UI Functions ////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUIInput::OnEvent( const SUIEvent& event )
{
	s_EventDispatcher.Dispatch( this, event );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arg1: Title
// Arg2: InitialStr
// Arg3: maxinput
//
void CUIInput::OnDisplayVirtualKeyboard( const SUIEvent& event )
{
	static wstring title;
	static wstring initialStr;
	int maxinput;
	if (event.args.GetArg(0, title) && event.args.GetArg(1, initialStr) && event.args.GetArg(2, maxinput))
	{
		gEnv->pFlashUI->DisplayVirtualKeyboard(IPlatformOS::KbdFlag_Default, title.c_str(), initialStr.c_str(), maxinput, this );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////// Actions /////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUIInput::OnAction( const ActionId& action, int activationMode, float value )
{
	s_actionHandler.Dispatch( this, 0, action, activationMode, value );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUIInput::OnActionTogglePause(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	if ( g_pGame->GetIGameFramework()->IsGameStarted() )
	{
		const bool bIsIngameMenu = CUIManager::GetInstance()->GetUIMenuEvents()->IsIngameMenuStarted();
		CUIManager::GetInstance()->GetUIMenuEvents()->DisplayIngameMenu(!bIsIngameMenu);
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUIInput::OnActionStartPause(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	if ( g_pGame->GetIGameFramework()->IsGameStarted() && !CUIManager::GetInstance()->GetUIMenuEvents()->IsIngameMenuStarted())
	{
		CUIManager::GetInstance()->GetUIMenuEvents()->DisplayIngameMenu(true);
	}
	return false;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SEND_CONTROLLER_EVENT(evt) 	if ( gEnv->pFlashUI ) \
	{ \
		switch (activationMode) \
		{ \
			case eAAM_OnPress: 	 gEnv->pFlashUI->DispatchControllerEvent( IUIElement::evt, IUIElement::eCIS_OnPress ); break; \
			case eAAM_OnRelease: gEnv->pFlashUI->DispatchControllerEvent( IUIElement::evt, IUIElement::eCIS_OnRelease ); break;\
			case eAAM_OnHold: 	 gEnv->pFlashUI->DispatchControllerEvent( IUIElement::evt, IUIElement::eCIS_OnPress ); \
													gEnv->pFlashUI->DispatchControllerEvent( IUIElement::evt, IUIElement::eCIS_OnRelease ); break;\
		} \
	}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUIInput::OnActionUp(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	SEND_CONTROLLER_EVENT(eCIE_Up);
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUIInput::OnActionDown(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	SEND_CONTROLLER_EVENT(eCIE_Down);
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUIInput::OnActionLeft(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	SEND_CONTROLLER_EVENT(eCIE_Left);
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUIInput::OnActionRight(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	SEND_CONTROLLER_EVENT(eCIE_Right);
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUIInput::OnActionClick(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	SEND_CONTROLLER_EVENT(eCIE_Action);
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUIInput::OnActionBack(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	SEND_CONTROLLER_EVENT(eCIE_Back);
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUIInput::OnActionConfirm(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	SEND_CONTROLLER_EVENT(eCIE_Button3);
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUIInput::OnActionReset(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	SEND_CONTROLLER_EVENT(eCIE_Button4);
	return false;
}

#undef SEND_CONTROLLER_EVENT