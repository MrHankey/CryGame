////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2010.
// -------------------------------------------------------------------------
//  File name:   UIInput.h
//  Version:     v1.00
//  Created:     17/9/2010 by Paul Reindell.
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __UIInput_H__
#define __UIInput_H__

#include "IActionMapManager.h"
#include <IFlashUI.h>
#include <IPlatformOS.h>

class CUIInput 
	: public IActionListener
	, public IUIEventListener
	, public IVirtualKeyboardEvents
{
public:

	CUIInput();
	~CUIInput();

	// IActionListener
	virtual void OnAction( const ActionId& action, int activationMode, float value );
	// ~IActionListener

	// IUIEventListener
	virtual void OnEvent( const SUIEvent& event );
	// ~IUIEventListener

	// IVirtualKeyboardEvents
	virtual void KeyboardCancelled();
	virtual void KeyboardFinished(const wchar_t *pInString);
	// ~IVirtualKeyboardEvents

private:
	enum EUIEvent
	{
		eUIE_OnVirtKeyboardDone,
		eUIE_OnVirtKeyboardCancelled,
	};
	void NotifyUI(EUIEvent eventType, const SUIArguments& args = SUIArguments());

	// ui functions
	void OnDisplayVirtualKeyboard( const SUIEvent& event );

	// actions
	bool OnActionTogglePause(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool OnActionStartPause(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool OnActionUp(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool OnActionDown(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool OnActionLeft(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool OnActionRight(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool OnActionClick(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool OnActionBack(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool OnActionConfirm(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool OnActionReset(EntityId entityId, const ActionId& actionId, int activationMode, float value);

private:
	static TActionHandler<CUIInput>	s_actionHandler;
	static SUIEventHelper<CUIInput> s_EventDispatcher;
	IUIEventSystem* m_pUIFunctions;
	IUIEventSystem* m_pUIEvents;
	std::map< EUIEvent, uint > m_eventMap;
};


#endif