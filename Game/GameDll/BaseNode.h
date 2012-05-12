
// Simplified base flownode class
// by Ruan Pearce-Authers <ruan@crytek.com>

#include "StdAfx.h"
#include "Nodes/G2FlowBaseNode.h"

#pragma once

class CBaseNode : public CFlowBaseNode<eNCT_Instanced>
{
protected:
	// Node data
	SActivationInfo m_actInfo;

	// Tests whether a given port is active
	bool IsActive(int portId)
	{
		return IsPortActive(&m_actInfo, portId);
	}

	// Activates a void output port
	void Activate(int portId)
	{
		ActivateOutput(&m_actInfo, portId, 0);
	}

	// Activates an output port with data
	template<typename T>
	void Activate(int portId, T value)
	{
		ActivateOutput(&m_actInfo, portId, value);
	}

	// Gets a port value
	template<typename T>
	T GetPortValue(int portId)
	{
		return *((&m_actInfo)->pInputPorts[portId].GetPtr<T>());
	}

	// Enables and disables sending of regular update events
	void SetUpdated(bool enabled)
	{
		(&m_actInfo)->pGraph->SetRegularlyUpdated((&m_actInfo)->myID, enabled);
	}

public:

	// Should be overriden if you need to register additional resources
	virtual void GetMemoryUsage(ICrySizer *s) const
	{
		s->Add(*this);
	}

	// This is only used for singleton nodes, which we don't support for simplicity's sake
	virtual IFlowNodePtr Clone(SActivationInfo *pActInfo)
	{
		return NULL;
	}

	// Processes events sent from the FlowSystem
	// TODO: Add more stuff.
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo *pActInfo)
	{
		m_actInfo = *pActInfo;

		switch(event)
		{
			case eFE_Initialize:
			{
				OnInit();
			}
			break;

			case eFE_Activate:
			{
				OnActivate();
			}
			break;

			case eFE_Update:
			{
				OnUpdate();
			}
			break;
		}
	}

	// Called when initialisation events are sent
	virtual void OnInit() { }

	// Called when an input port is activated
	virtual void OnActivate() { }

	// Called when this node is updated, provided this node is registered to receive updates
	virtual void OnUpdate() { }
};