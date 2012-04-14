////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2011.
// -------------------------------------------------------------------------
//  File name:   UIManager.h
//  Version:     v1.00
//  Created:     08/8/2011 by Paul Reindell.
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __UIManager_H__
#define __UIManager_H__

#include <ISystem.h>
#include "IUIGameEventSystem.h"

class CUIManager
	: public ISystemEventListener
{
public:
	static void Init();
	static void Shutdown();
	static void Destroy();

	// can return NULL if dedicated server!
	static CUIManager* GetInstance();

	IUIGameEventSystem* GetUIEventSystem(const char* typeName) const;

	void ProcessViewParams(const SViewParams &viewParams);
	void UpdatePickupMessage(bool bShow);

	// ISystemEventListener
	virtual void OnSystemEvent( ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam );
	// ~ISystemEventListener

	void LoadProfile();
	void SaveProfile();

private:
	CUIManager();
	CUIManager(const CUIManager& ) {}
	CUIManager operator=(const CUIManager& ) { return *this; }
	~CUIManager();

	static CUIManager* m_pInstance;

	void InitSound();

	IPlayerProfile* GetCurrentProfile();

private:
	bool m_bPickupMsgVisible;
	int m_soundListener;

	typedef std::map<string, IUIGameEventSystem*> TUIEventSystems;
	TUIEventSystems m_EventSystems;
};

namespace UIEvents
{
	template <class T>
	T* Get()
	{
		CUIManager* pManager = CUIManager::GetInstance();
		return pManager ? (T*) pManager->GetUIEventSystem(T::GetTypeNameS()) : NULL;
	}
}


#endif

