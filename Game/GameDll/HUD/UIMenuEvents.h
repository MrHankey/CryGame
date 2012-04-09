////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2011.
// -------------------------------------------------------------------------
//  File name:   UIMenuEvents.h
//  Version:     v1.00
//  Created:     21/11/2011 by Paul Reindell.
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __UIMenuEvents_H__
#define __UIMenuEvents_H__

#include <IFlashUI.h>
#include <IGameFramework.h>

class CUIMenuEvents
	: public IUIEventListener
	, public IUIModule
{
public:
	CUIMenuEvents();
	~CUIMenuEvents();

	// IUIEventListener
	virtual void OnEvent( const SUIEvent& event );

	//IUIModule
	virtual void Reset();

	void DisplayIngameMenu(bool bDisplay);
	bool IsIngameMenuStarted() const { return m_bIsIngameMenuStarted; }


private:
	void StartIngameMenu();
	void StopIngameMenu();

	void OnDisplayIngameMenu( const SUIEvent& event );

private:
	static SUIEventHelper<CUIMenuEvents> s_EventDispatcher;
	IUIEventSystem* m_pUIEvents;
	IUIEventSystem* m_pUIFunctions;

	enum EUIEvent
	{
		eUIE_StartIngameMenu,
		eUIE_StopIngameMenu,
	};

	std::map< EUIEvent, uint > m_EventMap;

	bool m_bIsIngameMenuStarted;
};


#endif // __UISettings_H__