////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2011.
// -------------------------------------------------------------------------
//  File name:   UIMultiPlayer.h
//  Version:     v1.00
//  Created:     26/8/2011 by Paul Reindell.
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __UIMultiPlayer_H__
#define __UIMultiPlayer_H__

#include <IFlashUI.h>
#include <IPlayerProfiles.h>

class CUIMultiPlayer 
	: public IUIEventListener
	, public IUIModule
{
public:
	CUIMultiPlayer();
	virtual ~CUIMultiPlayer();

	// IUIEventListener
	virtual void OnEvent(const SUIEvent& event);
	// ~IUIEventListener

	// IUIModule
	virtual void Reset();
	// ~IUIModule

	// UI functions
	void EnteredGame();
	void PlayerJoined(EntityId playerid, const string& name);
	void PlayerLeft(EntityId playerid, const string& name);
	void PlayerKilled(EntityId playerid, EntityId shooterid);
	void PlayerRenamed(EntityId playerid, const string& newName);

	void LoadProfile(IPlayerProfile* pProfile);
	void SaveProfile(IPlayerProfile* pProfile);

private:
	// UI events
	void RequestPlayers( const SUIEvent& event );
	void GetPlayerName( const SUIEvent& event );
	void SetPlayerName( const SUIEvent& event );
	void ConnectToServer( const SUIEvent& event );
	void GetServerName( const SUIEvent& event );


	void SubmitNewName();
	string GetPlayerNameById( EntityId playerid );

private:
	static SUIEventHelper<CUIMultiPlayer> s_EventDispatcher;
	IUIEventSystem* m_pUIEvents;
	IUIEventSystem* m_pUIFunctions;

	enum EUIEvent
	{
		eUIE_EnteredGame,
		eUIE_PlayerJoined,
		eUIE_PlayerLeft,
		eUIE_PlayerKilled,
		eUIE_PlayerRenamed,
		eUIE_SendName,
		eUIE_SendServer,
	};

	std::map< EUIEvent, uint > m_EventMap;

	struct SPlayerInfo
	{
		SPlayerInfo() : name("<UNDEFINED>"), teamId(-1) {}

		string name;
		int teamId; // todo: set the team id once we support teams in SDK
	};
	typedef std::map<EntityId, SPlayerInfo> TPlayers;
	TPlayers m_Players;

	string m_LocalPlayerName;
	string m_ServerName;
};


#endif // __UIMultiPlayer_H__
