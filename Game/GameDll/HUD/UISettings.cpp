////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2011.
// -------------------------------------------------------------------------
//  File name:   UISettings.cpp
//  Version:     v1.00
//  Created:     10/8/2011 by Paul Reindell.
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "UISettings.h"
#include "UIManager.h"

#include <ILevelSystem.h>

SUIEventHelper<CUISettings> CUISettings::s_EventDispatcher;

////////////////////////////////////////////////////////////////////////////
CUISettings::CUISettings()
	: m_pUIEvents(NULL)
	, m_pUIFunctions(NULL)
	, m_currResId(0)
{
	if (!gEnv->pFlashUI) return;

	// CVars
 	m_pRXVar = gEnv->pConsole->GetCVar("r_Width");
 	m_pRYVar = gEnv->pConsole->GetCVar("r_Height");
 	m_pFSVar = gEnv->pConsole->GetCVar("r_Fullscreen");
	m_pMusicVar = gEnv->pConsole->GetCVar("s_MusicVolume");
	m_pSFxVar = gEnv->pConsole->GetCVar("s_SFXVolume");
	m_pVideoVar = gEnv->pConsole->GetCVar("sys_flash_video_soundvolume");
	m_pVideoVar = m_pVideoVar ? m_pVideoVar : m_pMusicVar; // tmp fix to allow using music var as dummy fallback if GFX_VIDEO is not enabled (FreeSDK)
	m_pMouseSensitivity = gEnv->pConsole->GetCVar("cl_sensitivity");
	m_pInvertMouse = gEnv->pConsole->GetCVar("cl_invertMouse");
	m_pInvertController = gEnv->pConsole->GetCVar("cl_invertController");

	assert(m_pRXVar 
		&& m_pRYVar 
		&& m_pFSVar 
		&& m_pMusicVar 
		&& m_pSFxVar 
		&& m_pVideoVar
		&& m_pMouseSensitivity
		&& m_pInvertMouse
		&& m_pInvertController);

	if (!(m_pRXVar 
		&& m_pRYVar 
		&& m_pFSVar 
		&& m_pMusicVar 
		&& m_pSFxVar 
		&& m_pVideoVar
		&& m_pMouseSensitivity
		&& m_pInvertMouse
		&& m_pInvertController)) return;


	m_Resolutions.push_back(std::make_pair(1024,768));
	m_Resolutions.push_back(std::make_pair(1280,720));
	m_Resolutions.push_back(std::make_pair(1280,1050));
	m_Resolutions.push_back(std::make_pair(1680,1050));
	m_Resolutions.push_back(std::make_pair(1920,1080));


	// events to send from this class to UI flowgraphs
	m_pUIFunctions = gEnv->pFlashUI->CreateEventSystem("Settings", IUIEventSystem::eEST_SYSTEM_TO_UI);

	{
		SUIEventDesc eventDesc("OnGraphicChanged", "OnGraphicChanged", "Triggered on graphic settings change");
		eventDesc.Params.push_back(SUIParameterDesc("Resolution", "Resolution", "Resolution ID", SUIParameterDesc::eUIPT_Int));
		eventDesc.Params.push_back(SUIParameterDesc("ResX", "ResX", "Screen X resolution", SUIParameterDesc::eUIPT_Int));
		eventDesc.Params.push_back(SUIParameterDesc("ResY", "ResY", "Screen Y resolution", SUIParameterDesc::eUIPT_Int));
		eventDesc.Params.push_back(SUIParameterDesc("FullScreen", "FullScreen", "Fullscreen", SUIParameterDesc::eUIPT_Bool));
		m_EventMap[eUIE_GraphicSettingsChanged] = m_pUIFunctions->RegisterEvent(eventDesc);
	}

	{
		SUIEventDesc eventDesc("OnSoundChanged", "OnSoundChanged", "Triggered if sound volume changed");
		eventDesc.Params.push_back(SUIParameterDesc("Music", "Music", "Music volume", SUIParameterDesc::eUIPT_Float));
		eventDesc.Params.push_back(SUIParameterDesc("SFx", "SFx", "SFx volume", SUIParameterDesc::eUIPT_Float));
		eventDesc.Params.push_back(SUIParameterDesc("Video", "Video", "Video volume", SUIParameterDesc::eUIPT_Float));
		m_EventMap[eUIE_SoundSettingsChanged] = m_pUIFunctions->RegisterEvent(eventDesc);
	}

	{
		SUIEventDesc eventDesc("OnGameSettingsChanged", "OnGameSettingsChanged", "Triggered if game settings changed");
		eventDesc.Params.push_back(SUIParameterDesc("MouseSensitivity", "MouseSensitivity", "Mouse Sensitivity", SUIParameterDesc::eUIPT_Float));
		eventDesc.Params.push_back(SUIParameterDesc("InvertMouse", "InvertMouse", "Invert Mouse", SUIParameterDesc::eUIPT_Bool));
		eventDesc.Params.push_back(SUIParameterDesc("InvertController", "InvertController", "Invert Controller", SUIParameterDesc::eUIPT_Bool));
		m_EventMap[eUIE_GameSettingsChanged] = m_pUIFunctions->RegisterEvent(eventDesc);
	}

	{
		SUIEventDesc eventDesc("OnResolutions", "OnResolutions", "Triggered if resolutions were requested.");
		eventDesc.IsDynamic = true;
		eventDesc.sDynamicName = "Resolutions";
		eventDesc.sDynamicDesc = "UI array with all resolutions (x1,y1,x2,y2,...)";
		m_EventMap[eUIE_OnGetResolutions] = m_pUIFunctions->RegisterEvent(eventDesc);
	}

	{
		SUIEventDesc eventDesc("OnResolutionItem", "OnResolutionItem", "Triggered once per each resolution if resolutions were requested.");
		eventDesc.Params.push_back(SUIParameterDesc("ResString", "ResString", "Resolution as string (X x Y)", SUIParameterDesc::eUIPT_String));
		eventDesc.Params.push_back(SUIParameterDesc("ID", "ID", "Resolution ID", SUIParameterDesc::eUIPT_Int));
		m_EventMap[eUIE_OnGetResolutionItems] = m_pUIFunctions->RegisterEvent(eventDesc);
	}

	{
		SUIEventDesc eventDesc("OnLevelItem", "OnLevelItem", "Triggered once per level if levels were requested.");
		eventDesc.Params.push_back(SUIParameterDesc("LevelLabel", "LevelLabel", "@ui_<level> for localization", SUIParameterDesc::eUIPT_String));
		eventDesc.Params.push_back(SUIParameterDesc("LevelName", "LevelName", "name of the level", SUIParameterDesc::eUIPT_String));
		m_EventMap[eUIE_OnGetLevelItems] = m_pUIFunctions->RegisterEvent(eventDesc);
	}


	// events that can be sent from UI flowgraphs to this class
	m_pUIEvents = gEnv->pFlashUI->CreateEventSystem("Settings", IUIEventSystem::eEST_UI_TO_SYSTEM);

	{
		SUIEventDesc eventDesc("SetGraphics", "SetGraphics", "Call this to set graphic modes");
		eventDesc.Params.push_back(SUIParameterDesc("Resolution", "Resolution", "Resolution ID", SUIParameterDesc::eUIPT_Int));
		eventDesc.Params.push_back(SUIParameterDesc("Fullscreen", "Fullscreen", "Fullscreen (True/False)", SUIParameterDesc::eUIPT_Bool));
		s_EventDispatcher.RegisterEvent(m_pUIEvents, eventDesc, &CUISettings::OnSetGraphicSettings);
	}

	{
		SUIEventDesc eventDesc("SetResolution", "SetResolution", "Call this to set resolution");
		eventDesc.Params.push_back(SUIParameterDesc("ResX", "ResX", "Screen X resolution", SUIParameterDesc::eUIPT_Int));
		eventDesc.Params.push_back(SUIParameterDesc("ResY", "ResY", "Screen Y resolution", SUIParameterDesc::eUIPT_Int));
		eventDesc.Params.push_back(SUIParameterDesc("Fullscreen", "Fullscreen", "Fullscreen (True/False)", SUIParameterDesc::eUIPT_Bool));
		s_EventDispatcher.RegisterEvent(m_pUIEvents, eventDesc, &CUISettings::OnSetResolution);
	}

	{
		SUIEventDesc eventDesc("SetSound", "SetSound", "Call this to set sound settings");
		eventDesc.Params.push_back(SUIParameterDesc("Music", "Music", "Music volume", SUIParameterDesc::eUIPT_Float));
		eventDesc.Params.push_back(SUIParameterDesc("SFx", "SFx", "SFx volume", SUIParameterDesc::eUIPT_Float));
		eventDesc.Params.push_back(SUIParameterDesc("Video", "Video", "Video volume", SUIParameterDesc::eUIPT_Float));
		s_EventDispatcher.RegisterEvent(m_pUIEvents, eventDesc, &CUISettings::OnSetSoundSettings);
	}

	{
		SUIEventDesc eventDesc("SetGameSettings", "SetGameSettings", "Call this to set game settings");
		eventDesc.Params.push_back(SUIParameterDesc("MouseSensitivity", "MouseSensitivity", "Mouse Sensitivity", SUIParameterDesc::eUIPT_Float));
		eventDesc.Params.push_back(SUIParameterDesc("InvertMouse", "InvertMouse", "Invert Mouse", SUIParameterDesc::eUIPT_Bool));
		eventDesc.Params.push_back(SUIParameterDesc("InvertController", "InvertController", "Invert Controller", SUIParameterDesc::eUIPT_Bool));
		s_EventDispatcher.RegisterEvent(m_pUIEvents, eventDesc, &CUISettings::OnSetGameSettings);
	}

	{
		SUIEventDesc eventDesc("GetResolutionList", "GetResolutionList", "Execute this node will trigger the \"Events:Settings:OnResolutions\" node.");
		s_EventDispatcher.RegisterEvent(m_pUIEvents, eventDesc, &CUISettings::OnGetResolutions);
	}

	{
		SUIEventDesc eventDesc("GetCurrGraphics", "GetCurrGraphics", "Execute this node will trigger the \"Events:Settings:OnGraphicChanged\" node.");
		s_EventDispatcher.RegisterEvent(m_pUIEvents, eventDesc, &CUISettings::OnGetCurrGraphicsSettings);
	}

	{
		SUIEventDesc eventDesc("GetCurrSound", "GetCurrSound", "Execute this node will trigger the \"Events:Settings:OnSoundChanged\" node.");
		s_EventDispatcher.RegisterEvent(m_pUIEvents, eventDesc, &CUISettings::OnGetCurrSoundSettings);
	}

	{
		SUIEventDesc eventDesc("GetCurrGameSettings", "GetCurrGameSettings", "Execute this node will trigger the \"Events:Settings:OnGameSettingsChanged\" node.");
		s_EventDispatcher.RegisterEvent(m_pUIEvents, eventDesc, &CUISettings::OnGetCurrGameSettings);
	}

	{
		SUIEventDesc eventDesc("GetLevels", "GetLevels", "Execute this node will trigger the \"Events:Settings:OnLevelItem\" node once per level.");
		s_EventDispatcher.RegisterEvent(m_pUIEvents, eventDesc, &CUISettings::OnGetLevels);
	}

	{
		SUIEventDesc eventDesc("LogoutUser", "LogoutUser", "Execute this node to save settings and logout user");
		s_EventDispatcher.RegisterEvent(m_pUIEvents, eventDesc, &CUISettings::OnLogoutUser);
	}

	m_pUIEvents->RegisterListener(this, "CUISettings");
	gEnv->pFlashUI->RegisterModule(this, "CUISettings");
}

////////////////////////////////////////////////////////////////////////////
CUISettings::~CUISettings()
{
	if (m_pUIEvents) m_pUIEvents->UnregisterListener(this);
	gEnv->pFlashUI->UnregisterModule(this);
}

////////////////////////////////////////////////////////////////////////////
void CUISettings::Init()
{
#ifdef _RELEASE
	for (int i = 0; i < m_Resolutions.size(); ++i)
	{
		if (m_Resolutions[i].first == m_pRXVar->GetIVal() && m_Resolutions[i].second == m_pRYVar->GetIVal())
		{
			m_currResId = i;
			SendGraphicSettingsChange();
			break;
		}
	}
	SendSoundSettingsChange();
	SendGameSettingsChange();
#endif
}

////////////////////////////////////////////////////////////////////////////
void CUISettings::Update(float fDeltaTime)
{
#ifndef _RELEASE
 	static int rX = -1;
 	static int rY = -1;
 	if (rX != m_pRXVar->GetIVal() || rY != m_pRYVar->GetIVal())
	{
		rX = m_pRXVar->GetIVal();
		rY = m_pRYVar->GetIVal();
		m_currResId = 0;

		for (int i = 0; i < m_Resolutions.size(); ++i)
		{
			if (m_Resolutions[i].first == rX && m_Resolutions[i].second == rY)
			{
				m_currResId = i;
				SendGraphicSettingsChange();
				break;
			}
		}

	}

	static float music = -1.f;
	static float sfx = -1.f;
	static float video = -1.f;
	if (music != m_pMusicVar->GetFVal() || sfx != m_pSFxVar->GetFVal() || video != m_pVideoVar->GetFVal())
	{
		SendSoundSettingsChange();
		music = m_pMusicVar->GetFVal();
		sfx = m_pSFxVar->GetFVal();
		video = m_pVideoVar->GetFVal();
	}

	static float sensivity = -1.f;
	static bool invertMouse = m_pInvertMouse->GetIVal() == 1;
	static bool invertController = m_pInvertController->GetIVal() == 1;
	if (sensivity != m_pMouseSensitivity->GetFVal() || invertMouse != (m_pInvertMouse->GetIVal() == 1) || invertController != (m_pInvertController->GetIVal() == 1))
	{
		SendGameSettingsChange();
		sensivity = m_pMouseSensitivity->GetFVal();
		invertMouse = m_pInvertMouse->GetIVal() == 1;
		invertController = m_pInvertController->GetIVal() == 1;
	}

#endif
}

////////////////////////////////////////////////////////////////////////////
// ui events
////////////////////////////////////////////////////////////////////////////
void CUISettings::OnEvent( const SUIEvent& event )
{
	s_EventDispatcher.Dispatch( this, event );
}

////////////////////////////////////////////////////////////////////////////
// Arg1: Resolution
// Arg2: Fullscreen
//
void CUISettings::OnSetGraphicSettings( const SUIEvent& event )
{
#if !defined(PS3) && !defined(XENON)
	int resIndex;
	bool fs;

	if (event.args.GetArg(0, resIndex) && event.args.GetArg(1, fs) && resIndex >= 0 && resIndex < m_Resolutions.size())
	{
		m_currResId = resIndex;

		m_pRXVar->Set(m_Resolutions[resIndex].first);
		m_pRYVar->Set(m_Resolutions[resIndex].second);
		m_pFSVar->Set(fs);

		SendGraphicSettingsChange();
		return;
	}
	assert(false);
#endif
}

////////////////////////////////////////////////////////////////////////////
// Arg1: ResX
// Arg2: ResY
// Arg3: Fullscreen
//
// DESC: this is depricated; shoud consider to use OnSetGraphicSettings
void CUISettings::OnSetResolution( const SUIEvent& event )
{
#if !defined(PS3) && !defined(XENON)
	int resX, resY;
	bool fs;
	if (event.args.GetArg(0, resX) && event.args.GetArg(1, resY) && event.args.GetArg(2, fs))
	{
		m_pRXVar->Set(resX);
		m_pRYVar->Set(resY);
		m_pFSVar->Set(fs);
		return;
	}
	assert(false);
#endif
}

////////////////////////////////////////////////////////////////////////////
// Arg1: Music
// Arg2: SFx
// Arg3: Video
//
void CUISettings::OnSetSoundSettings( const SUIEvent& event )
{
	float music, sfx, video;
	if (event.args.GetArg(0, music) && event.args.GetArg(1, sfx) && event.args.GetArg(2, video))
	{
		m_pMusicVar->Set(music);
		m_pSFxVar->Set(sfx);
		m_pVideoVar->Set(video);
		SendSoundSettingsChange();
		return;
	}
	assert(false);
}

////////////////////////////////////////////////////////////////////////////
// Arg1: MouseSensitivity
// Arg2: InvertMouse
// Arg3: InvertController
//
void CUISettings::OnSetGameSettings( const SUIEvent& event )
{
	float sensitivity;
	bool invertMouse, invertController;
	if (event.args.GetArg(0, sensitivity) && event.args.GetArg(1, invertMouse) && event.args.GetArg(2, invertController))
	{
		m_pMouseSensitivity->Set(sensitivity);
		m_pInvertMouse->Set(invertMouse);
		m_pInvertController->Set(invertController);
		SendGameSettingsChange();
		return;
	}
	assert(false);
}

////////////////////////////////////////////////////////////////////////////
//
void CUISettings::OnGetResolutions( const SUIEvent& event )
{
	SendResolutions();
}

////////////////////////////////////////////////////////////////////////////
//
void CUISettings::OnGetCurrGraphicsSettings( const SUIEvent& event )
{
	SendGraphicSettingsChange();
}

////////////////////////////////////////////////////////////////////////////
//
void CUISettings::OnGetCurrSoundSettings( const SUIEvent& event )
{
	SendSoundSettingsChange();
}

////////////////////////////////////////////////////////////////////////////
//
void CUISettings::OnGetCurrGameSettings( const SUIEvent& event )
{
	SendGameSettingsChange();
}

////////////////////////////////////////////////////////////////////////////
//
void CUISettings::OnGetLevels( const SUIEvent& event )
{
	if (gEnv->pGame && gEnv->pGame->GetIGameFramework() && gEnv->pGame->GetIGameFramework()->GetILevelSystem())
	{
		int i = 0;
		while ( ILevelInfo* pLevelInfo = gEnv->pGame->GetIGameFramework()->GetILevelSystem()->GetLevelInfo( i++ ) )
		{
			SUIArguments args;
			args.AddArgument( pLevelInfo->GetDisplayName() );
			args.AddArgument( pLevelInfo->GetName() );
			m_pUIFunctions->SendEvent(SUIEvent(m_EventMap[eUIE_OnGetLevelItems], args));
		}
	}
}

////////////////////////////////////////////////////////////////////////////
//
void CUISettings::OnLogoutUser( const SUIEvent& event )
{
	CUIManager* pManager = CUIManager::GetInstance();
	if (pManager)
		pManager->SaveProfile();

	if (gEnv->pGame && gEnv->pGame->GetIGameFramework() && gEnv->pGame->GetIGameFramework()->GetIPlayerProfileManager())
	{
		IPlayerProfileManager* pProfileManager = gEnv->pGame->GetIGameFramework()->GetIPlayerProfileManager();
		pProfileManager->LogoutUser(pProfileManager->GetCurrentUser());
	}
}


////////////////////////////////////////////////////////////////////////////
// ui functions
////////////////////////////////////////////////////////////////////////////
void CUISettings::SendResolutions()
{
	SUIArguments resolutions;
	for (int i = 0; i < m_Resolutions.size(); ++i)
	{
		string res;
		res.Format("%i x %i", m_Resolutions[i].first, m_Resolutions[i].second);

		SUIArguments args;
		args.AddArgument(res);
		args.AddArgument(i);

		m_pUIFunctions->SendEvent(SUIEvent(m_EventMap[eUIE_OnGetResolutionItems], args));

		resolutions.AddArgument(m_Resolutions[i].first);
		resolutions.AddArgument(m_Resolutions[i].second);
	}
	m_pUIFunctions->SendEvent(SUIEvent(m_EventMap[eUIE_OnGetResolutions], resolutions));
}

////////////////////////////////////////////////////////////////////////////
void CUISettings::SendGraphicSettingsChange()
{
	SUIArguments args;
	args.AddArgument(m_currResId);
	args.AddArgument(m_Resolutions[m_currResId].first);
	args.AddArgument(m_Resolutions[m_currResId].second);
	args.AddArgument(m_pFSVar->GetIVal() != 0);
	m_pUIFunctions->SendEvent(SUIEvent(m_EventMap[eUIE_GraphicSettingsChanged], args));
}

////////////////////////////////////////////////////////////////////////////
void CUISettings::SendSoundSettingsChange()
{
	SUIArguments args;
	args.AddArgument(m_pMusicVar->GetFVal());
	args.AddArgument(m_pSFxVar->GetFVal());
	args.AddArgument(m_pVideoVar->GetFVal());
	m_pUIFunctions->SendEvent(SUIEvent(m_EventMap[eUIE_SoundSettingsChanged], args));
}

////////////////////////////////////////////////////////////////////////////
void CUISettings::SendGameSettingsChange()
{
	SUIArguments args;
	args.AddArgument(m_pMouseSensitivity->GetFVal());
	args.AddArgument((m_pInvertMouse->GetIVal() == 1));
	args.AddArgument((m_pInvertController->GetIVal() == 1));
	m_pUIFunctions->SendEvent(SUIEvent(m_EventMap[eUIE_GameSettingsChanged], args));
}

////////////////////////////////////////////////////////////////////////////
void CUISettings::LoadProfile(IPlayerProfile* pProfile)
{
	if (!(m_pRXVar 
		&& m_pRYVar 
		&& m_pFSVar 
		&& m_pMusicVar 
		&& m_pSFxVar 
		&& m_pVideoVar
		&& m_pMouseSensitivity
		&& m_pInvertMouse
		&& m_pInvertController)) return;

	int rx = m_pRXVar->GetIVal();
	int ry = m_pRYVar->GetIVal();
	int fs = m_pFSVar->GetIVal();
	pProfile->GetAttribute( "res_x",  rx, false);
	pProfile->GetAttribute( "res_y",  ry, false);
	pProfile->GetAttribute( "res_fs", fs, false);
	m_pRXVar->Set(rx);
	m_pRYVar->Set(ry);
	m_pFSVar->Set(fs);

	float music = m_pMusicVar->GetFVal();
	float sound = m_pSFxVar->GetFVal();
	float video = m_pVideoVar->GetFVal();
	pProfile->GetAttribute( "sound_music", music, false);
	pProfile->GetAttribute( "sound_sound", sound, false);
	pProfile->GetAttribute( "sound_video", video, false);
	m_pMusicVar->Set(music);
	m_pSFxVar->Set(sound);
	m_pVideoVar->Set(video);

	float sensitivity = m_pMouseSensitivity->GetFVal();
	int invertMouse = m_pInvertMouse->GetIVal();
	int invertController = m_pInvertController->GetIVal();
	pProfile->GetAttribute( "controls_sensitivity", sensitivity, false);
	pProfile->GetAttribute( "controls_invertMouse", invertMouse, false);
	pProfile->GetAttribute( "controls_invertController", invertController, false);
	m_pMouseSensitivity->Set(sensitivity);
	m_pInvertMouse->Set(invertMouse);
	m_pInvertController->Set(invertController);
}

////////////////////////////////////////////////////////////////////////////
void CUISettings::SaveProfile(IPlayerProfile* pProfile)
{
	if (!(m_pRXVar 
		&& m_pRYVar 
		&& m_pFSVar 
		&& m_pMusicVar 
		&& m_pSFxVar 
		&& m_pVideoVar
		&& m_pMouseSensitivity
		&& m_pInvertMouse
		&& m_pInvertController)) return;

	pProfile->SetAttribute( "res_x",  m_pRXVar->GetIVal());
	pProfile->SetAttribute( "res_y",  m_pRYVar->GetIVal());
	pProfile->SetAttribute( "res_fs", m_pFSVar->GetIVal());

	pProfile->SetAttribute( "sound_music", m_pMusicVar->GetFVal());
	pProfile->SetAttribute( "sound_sound", m_pSFxVar->GetFVal());
	pProfile->SetAttribute( "sound_video", m_pVideoVar->GetFVal());

	pProfile->SetAttribute( "controls_sensitivity", m_pMouseSensitivity->GetFVal());
	pProfile->SetAttribute( "controls_invertMouse", m_pInvertMouse->GetIVal());
	pProfile->SetAttribute( "controls_invertController", m_pInvertController->GetIVal());
}
