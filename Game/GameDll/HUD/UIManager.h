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
#include <IViewSystem.h>
#include <IPlayerProfiles.h>

class CUIInput;
class CUIObjectives;
class CUISettings;
class CUIMultiPlayer;
class CUIMenuEvents;
class CUIHUD3D;
class CUIEntityDynTexTag;

class CUIManager
	: public ISystemEventListener
{
public:
	static void Init();
	static void Shutdown();
	static void Destroy();

	// can return NULL if dedicated server!
	static CUIManager* GetInstance();

	CUIInput* GetUIInput() const { return m_pUIInput; }
	CUIObjectives* GetUIObjectives() const { return m_pUIObjectives; } 
	CUISettings* GetUISettings() const { return m_pUISettings; } 
	CUIMultiPlayer* GetUIMultiPlayer() const { return m_pUIMultiPlayer; }
	CUIMenuEvents* GetUIMenuEvents() const { return m_pUIMenuEvents; }
	CUIHUD3D* GetHUD3D() const { return m_pHUD3D; }
	CUIEntityDynTexTag* Get3DEntityTags() const { return m_pUIEntityDynTexTag; }

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
	CUIInput* m_pUIInput;
	CUIObjectives* m_pUIObjectives;
	CUISettings* m_pUISettings;
	CUIMultiPlayer* m_pUIMultiPlayer;
	CUIMenuEvents* m_pUIMenuEvents;
	CUIHUD3D* m_pHUD3D;
	CUIEntityDynTexTag* m_pUIEntityDynTexTag;

	bool m_bPickupMsgVisible;

	int m_soundListener;
};

#define NOTIFY_UI( fct, module ) { \
	CUIManager* pManager = CUIManager::GetInstance(); \
	if (pManager && pManager->module()) \
	pManager->module()->fct; }

#define NOTIFY_UI_OBJECTIVES( fct ) NOTIFY_UI( fct, GetUIObjectives )
#define NOTIFY_UI_MP( fct ) NOTIFY_UI( fct, GetUIMultiPlayer )

#endif

