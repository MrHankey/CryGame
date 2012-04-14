////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2011.
// -------------------------------------------------------------------------
//  File name:   UISettings.h
//  Version:     v1.00
//  Created:     10/8/2011 by Paul Reindell.
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __UISettings_H__
#define __UISettings_H__

#include "IUIGameEventSystem.h"
#include <IFlashUI.h>
#include <IGameFramework.h>

class CUISettings
	: public IUIGameEventSystem
	, public IUIModule
{
public:
	CUISettings();

	// IUIGameEventSystem
	UIEVENTSYSTEM( "UISettings" );
	virtual void InitEventSystem();
	virtual void UnloadEventSystem();
	virtual void LoadProfile( IPlayerProfile* pProfile );
	virtual void SaveProfile( IPlayerProfile* pProfile ) const;

	//IUIModule
	virtual void Init();
	virtual void Update(float fDelta);

private:
	// ui functions
	void SendResolutions();
	void SendGraphicSettingsChange();
	void SendSoundSettingsChange();
	void SendGameSettingsChange();

	// ui events
	void OnSetGraphicSettings( int resIndex, bool fullscreen );
	void OnSetResolution( int resX, int resY, bool fullscreen );
	void OnSetSoundSettings( float music, float sfx, float video );
	void OnSetGameSettings( float sensitivity, bool invertMouse, bool invertController );

	void OnGetResolutions();
	void OnGetCurrGraphicsSettings();
	void OnGetCurrSoundSettings();
	void OnGetCurrGameSettings();

	void OnGetLevels();

	void OnLogoutUser();

private:
	enum EUIEvent
	{
		eUIE_GraphicSettingsChanged,
		eUIE_SoundSettingsChanged,
		eUIE_GameSettingsChanged,

		eUIE_OnGetResolutions,
		eUIE_OnGetResolutionItems,
		eUIE_OnGetLevelItems,
	};

	SUIEventReceiverDispatcher<CUISettings> m_eventDispatcher;
	SUIEventSenderDispatcher<EUIEvent> m_eventSender;
	IUIEventSystem* m_pUIEvents;
	IUIEventSystem* m_pUIFunctions;

	ICVar* m_pRXVar;
	ICVar* m_pRYVar;
 	ICVar* m_pFSVar;

	ICVar* m_pMusicVar;
	ICVar* m_pSFxVar;
	ICVar* m_pVideoVar;

	ICVar* m_pMouseSensitivity;
	ICVar* m_pInvertMouse;
	ICVar* m_pInvertController;

	int m_currResId;

	typedef std::vector< std::pair<int,int> > TResolutions;
	TResolutions m_Resolutions;
};


#endif // __UISettings_H__