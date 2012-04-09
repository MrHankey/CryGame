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

#include <IFlashUI.h>
#include <IGameFramework.h>
#include <IPlayerProfiles.h>

class CUISettings
	: public IUIEventListener
	, public IUIModule
{
public:
	CUISettings();
	~CUISettings();

	// IUIEventListener
	virtual void OnEvent( const SUIEvent& event );

	//IUIModule
	virtual void Init();
	virtual void Update(float fDelta);

	void LoadProfile(IPlayerProfile* pProfile);
	void SaveProfile(IPlayerProfile* pProfile);

private:
	// ui functions
	void SendResolutions();
	void SendGraphicSettingsChange();
	void SendSoundSettingsChange();
	void SendGameSettingsChange();

	// ui events
	void OnSetGraphicSettings( const SUIEvent& event );
	void OnSetResolution( const SUIEvent& event );
	void OnSetSoundSettings( const SUIEvent& event );
	void OnSetGameSettings( const SUIEvent& event );

	void OnGetResolutions( const SUIEvent& event );
	void OnGetCurrGraphicsSettings( const SUIEvent& event );
	void OnGetCurrSoundSettings( const SUIEvent& event );
	void OnGetCurrGameSettings( const SUIEvent& event );

	void OnGetLevels( const SUIEvent& event );

	void OnLogoutUser( const SUIEvent& event );

private:
	static SUIEventHelper<CUISettings> s_EventDispatcher;
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

	enum EUIEvent
	{
		eUIE_GraphicSettingsChanged,
		eUIE_SoundSettingsChanged,
		eUIE_GameSettingsChanged,

		eUIE_OnGetResolutions,
		eUIE_OnGetResolutionItems,
		eUIE_OnGetLevelItems,
	};

	std::map< EUIEvent, uint > m_EventMap;

	typedef std::vector< std::pair<int,int> > TResolutions;
	TResolutions m_Resolutions;
};


#endif // __UISettings_H__