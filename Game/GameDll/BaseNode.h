// Ruan Pearce-Authers <ruan@crytek.com>
// Simplified base flownode class

#include "StdAfx.h"
#include "Nodes/G2FlowBaseNode.h"

#ifndef BASE_NODE_H
#define BASE_NODE_H

class CBaseNode : public CFlowBaseNode<eNCT_Instanced>
{
protected:
	// Node data
	SActivationInfo *m_pActInfo;

	// Tests whether a given port is active
	bool IsActive(int portId)
	{
		return IsPortActive(m_pActInfo, portId);
	}
	
	// Activates a void output port
	void Activate(int portId)
	{
		ActivateOutput(m_pActInfo, portId, 0);
	}

	// Activates an output port with data
	template<typename T>
	void Activate(int portId, T value)
	{
		ActivateOutput(m_pActInfo, portId, value);
	}

	// Gets a port value
	template<typename T>
	T GetPortValue(int portId)
	{
		return *m_pActInfo->pInputPorts[portId].GetPtr<T>();
	}

public:
	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	// Processes 
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo *pActInfo)
	{
		m_pActInfo = pActInfo;

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
		}
	}

	// Called when the node is initialised
	virtual void OnInit() { }

	// Called when an input port is activated
	virtual void OnActivate() { }

	// Called when a 
	virtual void OnUpdate() { }
};

#endif