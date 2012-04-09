////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2011.
// -------------------------------------------------------------------------
//  File name:   UIMultiPlayer.cpp
//  Version:     v1.00
//  Created:     26/8/2011 by Paul Reindell.
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "UIMultiPlayer.h"

#include <IGameFramework.h>
#include "Game.h"
#include "Actor.h"
#include "GameRules.h"

SUIEventHelper<CUIMultiPlayer> CUIMultiPlayer::s_EventDispatcher;

////////////////////////////////////////////////////////////////////////////
CUIMultiPlayer::CUIMultiPlayer()
	: m_pUIEvents(NULL)
	, m_pUIFunctions(NULL)
	, m_LocalPlayerName("Dude")
{
	if (!gEnv->pFlashUI)
		return;

	ICVar* pServerVar = gEnv->pConsole->GetCVar("cl_serveraddr");
	m_ServerName = pServerVar ? pServerVar->GetString() : "";
	if (m_ServerName == "")
		m_ServerName = "localhost";

	// events to send from this class to UI flowgraphs
	m_pUIFunctions = gEnv->pFlashUI->CreateEventSystem("MP", IUIEventSystem::eEST_SYSTEM_TO_UI);
	{
		SUIEventDesc evtDesc("EnteredGame", "EnteredGame", "Triggered once the local player enters the game");
		m_EventMap[eUIE_EnteredGame] = m_pUIFunctions->RegisterEvent(evtDesc);
	}

	{
		SUIEventDesc evtDesc("PlayerJoined", "PlayerJoined", "Triggered if a player joins the game");
		evtDesc.Params.push_back(SUIParameterDesc("ID", "ID", "", SUIParameterDesc::eUIPT_Int));
		evtDesc.Params.push_back(SUIParameterDesc("Name", "Name", "", SUIParameterDesc::eUIPT_String));
		m_EventMap[eUIE_PlayerJoined] = m_pUIFunctions->RegisterEvent(evtDesc);
	}

	{
		SUIEventDesc evtDesc("PlayerLeft", "PlayerLeft", "Triggered if a player left the game");
		evtDesc.Params.push_back(SUIParameterDesc("ID", "ID", "", SUIParameterDesc::eUIPT_Int));
		evtDesc.Params.push_back(SUIParameterDesc("Name", "Name", "", SUIParameterDesc::eUIPT_String));
		m_EventMap[eUIE_PlayerLeft] = m_pUIFunctions->RegisterEvent(evtDesc);
	}

	{
		SUIEventDesc evtDesc("PlayerKilled", "PlayerKilled", "Triggered if a player gets killed");
		evtDesc.Params.push_back(SUIParameterDesc("ID", "ID", "", SUIParameterDesc::eUIPT_Int));
		evtDesc.Params.push_back(SUIParameterDesc("Name", "Name", "", SUIParameterDesc::eUIPT_String));
		evtDesc.Params.push_back(SUIParameterDesc("ShooterID", "ShooterID", "", SUIParameterDesc::eUIPT_Int));
		evtDesc.Params.push_back(SUIParameterDesc("ShooterName", "ShooterName", "", SUIParameterDesc::eUIPT_String));
		m_EventMap[eUIE_PlayerKilled] = m_pUIFunctions->RegisterEvent(evtDesc);
	}

	{
		SUIEventDesc evtDesc("PlayerRenamed", "PlayerRenamed", "Triggered if a player was renamed");
		evtDesc.Params.push_back(SUIParameterDesc("ID", "ID", "", SUIParameterDesc::eUIPT_Int));
		evtDesc.Params.push_back(SUIParameterDesc("NewName", "NewName", "", SUIParameterDesc::eUIPT_String));
		m_EventMap[eUIE_PlayerRenamed] = m_pUIFunctions->RegisterEvent(evtDesc);
	}

	{
		SUIEventDesc evtDesc("OnGetName", "OnGetName", "Triggers once the \"GetPlayerName\" node was called");
		evtDesc.Params.push_back(SUIParameterDesc("Name", "Name", "", SUIParameterDesc::eUIPT_String));
		m_EventMap[eUIE_SendName] = m_pUIFunctions->RegisterEvent(evtDesc);
	}

	{
		SUIEventDesc evtDesc("OnGetServerName", "OnGetServerName", "Triggers once the \"GetLastServer\" node was called");
		evtDesc.Params.push_back(SUIParameterDesc("Address", "Address", "", SUIParameterDesc::eUIPT_String));
		m_EventMap[eUIE_SendServer] = m_pUIFunctions->RegisterEvent(evtDesc);
	}


	// events that can be sent from UI flowgraphs to this class
	m_pUIEvents = gEnv->pFlashUI->CreateEventSystem("MP", IUIEventSystem::eEST_UI_TO_SYSTEM);
	{
		SUIEventDesc evtDesc("GetPlayers", "GetPlayers", "");
		s_EventDispatcher.RegisterEvent(m_pUIEvents, evtDesc, &CUIMultiPlayer::RequestPlayers);
	}

	{
		SUIEventDesc evtDesc("GetPlayerName", "GetPlayerName", "Get the name of the local player in mp (will trigger the \"OnGetName\" node");
		s_EventDispatcher.RegisterEvent(m_pUIEvents, evtDesc, &CUIMultiPlayer::GetPlayerName);
	}

	{
		SUIEventDesc evtDesc("SetPlayerName", "SetPlayerName", "Set the name of the local player in mp");
		evtDesc.Params.push_back(SUIParameterDesc("Name", "Name", "Local player name", SUIParameterDesc::eUIPT_String));
		s_EventDispatcher.RegisterEvent(m_pUIEvents, evtDesc, &CUIMultiPlayer::SetPlayerName);
	}

	{
		SUIEventDesc evtDesc("ConnectToServer", "ConnectToServer", "Connect to a server");
		evtDesc.Params.push_back(SUIParameterDesc("Address", "Address", "server address", SUIParameterDesc::eUIPT_String));
		s_EventDispatcher.RegisterEvent(m_pUIEvents, evtDesc, &CUIMultiPlayer::ConnectToServer);
	}

	{
		SUIEventDesc evtDesc("GetLastServer", "GetLastServer", "Get the server name that was last used");
		s_EventDispatcher.RegisterEvent(m_pUIEvents, evtDesc, &CUIMultiPlayer::GetServerName);
	}


	m_pUIEvents->RegisterListener(this, "CUIMultiPlayer");

	gEnv->pFlashUI->RegisterModule(this, "CUIMultiPlayer");
}

////////////////////////////////////////////////////////////////////////////
CUIMultiPlayer::~CUIMultiPlayer()
{
	if (m_pUIEvents)
		m_pUIEvents->UnregisterListener(this);

	if (gEnv->pFlashUI)
		gEnv->pFlashUI->UnregisterModule(this);
}

////////////////////////////////////////////////////////////////////////////
void CUIMultiPlayer::Reset()
{
	m_Players.clear();
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// ui functions
void CUIMultiPlayer::EnteredGame()
{
	m_pUIFunctions->SendEvent(SUIEvent(m_EventMap[eUIE_EnteredGame], SUIArguments()));
}

////////////////////////////////////////////////////////////////////////////
void CUIMultiPlayer::PlayerJoined(EntityId playerid, const string& name)
{
	CryLogAlways("[CUIMultiPlayer] PlayerJoined %i %s", playerid, name.c_str() );

	m_Players[playerid].name = name;

	if (gEnv->pGame->GetIGameFramework()->GetClientActorId() == playerid)
	{
		SubmitNewName();
		return;
	}

	SUIArguments args;
	args.AddArgument(playerid);
	args.AddArgument(name);

	m_pUIFunctions->SendEvent(SUIEvent(m_EventMap[eUIE_PlayerJoined], args));
}

////////////////////////////////////////////////////////////////////////////
void CUIMultiPlayer::PlayerLeft(EntityId playerid, const string& name)
{
	CryLogAlways("[CUIMultiPlayer] PlayerLeft %i %s", playerid, name.c_str() );

	if (gEnv->pGame->GetIGameFramework()->GetClientActorId() == playerid)
		return;

	// fix up player id in case that the entity was already removed and the Network was not able to resolve entity id
	if (playerid == NULL)
	{
		for (TPlayers::const_iterator it = m_Players.begin(); it != m_Players.end(); ++it)
		{
			if (it->second.name == name)
			{
				playerid = it->first;
			}
		}
	}

	SUIArguments args;
	args.AddArgument(playerid);
	args.AddArgument(name);

	const bool ok = stl::member_find_and_erase(m_Players, playerid);
 	assert( ok );

	m_pUIFunctions->SendEvent(SUIEvent(m_EventMap[eUIE_PlayerLeft], args));
}

////////////////////////////////////////////////////////////////////////////
void CUIMultiPlayer::PlayerKilled(EntityId playerid, EntityId shooterid)
{
	SUIArguments args;
	args.AddArgument(playerid);
	args.AddArgument(GetPlayerNameById(playerid));
	args.AddArgument(shooterid);
	args.AddArgument(GetPlayerNameById(shooterid));

	m_pUIFunctions->SendEvent(SUIEvent(m_EventMap[eUIE_PlayerKilled], args));
}

////////////////////////////////////////////////////////////////////////////
void CUIMultiPlayer::PlayerRenamed(EntityId playerid, const string& newName)
{
	CryLogAlways("[CUIMultiPlayer] PlayerRenamed %i %s", playerid, newName.c_str() );

	m_Players[playerid].name = newName;

	SUIArguments args;
	args.AddArgument(playerid);
	args.AddArgument(newName);

	m_pUIFunctions->SendEvent(SUIEvent(m_EventMap[eUIE_PlayerRenamed], args));
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// ui events
void CUIMultiPlayer::OnEvent( const SUIEvent& event )
{
	s_EventDispatcher.Dispatch( this, event );
}

////////////////////////////////////////////////////////////////////////////
//
void CUIMultiPlayer::RequestPlayers( const SUIEvent& event )
{
	IActorIteratorPtr actors = gEnv->pGame->GetIGameFramework()->GetIActorSystem()->CreateActorIterator();
	while (IActor* pActor = actors->Next())
	{
		if (pActor->IsPlayer() && m_Players.find(pActor->GetEntityId()) == m_Players.end())
		{
			PlayerJoined(pActor->GetEntityId(), pActor->GetEntity()->GetName());
		}
	}
}

////////////////////////////////////////////////////////////////////////////
//
void CUIMultiPlayer::GetPlayerName( const SUIEvent& event )
{
	SUIArguments args;
	args.AddArgument(m_LocalPlayerName);
	m_pUIFunctions->SendEvent(SUIEvent(m_EventMap[eUIE_SendName], args));
}

////////////////////////////////////////////////////////////////////////////
// Arg1: Name
//
void CUIMultiPlayer::SetPlayerName( const SUIEvent& event )
{
	event.args.GetArg(0, m_LocalPlayerName);
	SubmitNewName();
}

////////////////////////////////////////////////////////////////////////////
// Arg1: Server address
//
void CUIMultiPlayer::ConnectToServer( const SUIEvent& event )
{
	if (gEnv->IsEditor()) return;

	string address;
	if (event.args.GetArg(0, address))
	{
		m_ServerName = address;
		g_pGame->GetIGameFramework()->ExecuteCommandNextFrame(string().Format("connect %s", address.c_str()));
	}
}

////////////////////////////////////////////////////////////////////////////
//
void CUIMultiPlayer::GetServerName( const SUIEvent& event )
{
	SUIArguments args;
	args.AddArgument(m_ServerName);
	m_pUIFunctions->SendEvent(SUIEvent(m_EventMap[eUIE_SendServer], args));
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void CUIMultiPlayer::SubmitNewName()
{
	if (m_LocalPlayerName.empty())
		return;

	INetChannel* pNetChannel = g_pGame->GetIGameFramework()->GetClientChannel();
	if (pNetChannel && pNetChannel->GetNickname() && strcmp(pNetChannel->GetNickname(), m_LocalPlayerName.c_str()) == 0 )
		return;

	CActor* pLocalPlayer = (CActor*) g_pGame->GetIGameFramework()->GetClientActor();
	if (pLocalPlayer && g_pGame->GetGameRules())
		g_pGame->GetGameRules()->RenamePlayer( pLocalPlayer, m_LocalPlayerName.c_str() );
};

////////////////////////////////////////////////////////////////////////////
string CUIMultiPlayer::GetPlayerNameById( EntityId playerid )
{
	TPlayers::const_iterator it = m_Players.find(playerid);
	if (it != m_Players.end() )
	{
		return it->second.name;
	}

	IEntity* pEntity = gEnv->pEntitySystem->GetEntity(playerid);
	return pEntity ? pEntity->GetName() : "<UNDEFINED>";
}

////////////////////////////////////////////////////////////////////////////
void CUIMultiPlayer::LoadProfile(IPlayerProfile* pProfile)
{
	pProfile->GetAttribute( "mp_username", m_LocalPlayerName);
	pProfile->GetAttribute( "mp_server",  m_ServerName);

	// override if setup in system cfg
	ICVar* pServerVar = gEnv->pConsole->GetCVar("cl_serveraddr");
	if (pServerVar && pServerVar->GetString() != "")
		m_ServerName = pServerVar->GetString();
}

////////////////////////////////////////////////////////////////////////////
void CUIMultiPlayer::SaveProfile(IPlayerProfile* pProfile)
{
	pProfile->SetAttribute( "mp_username", m_LocalPlayerName);
	pProfile->SetAttribute( "mp_server",  m_ServerName);
}
