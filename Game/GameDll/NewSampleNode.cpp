#include "StdAfx.h"
#include "BaseNode.h"

// Example usage of the new CBaseNode
class CBaseNodeExample : public CBaseNode
{
private:
	enum EInputs
	{
		EIP_Activate,
		EIP_LeftSide,
		EIP_RightSide
	};

	enum EIOutputs
	{
		EOP_Answer
	};

public:
	// Do first-pass node setup stuff here
	CBaseNodeExample(SActivationInfo *pActInfo)
	{

	}

	// Release resources or whatever else might be necessary on node destruction
	~CBaseNodeExample()
	{

	}

	// Set up your inputs, outputs, category and description here
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		// These should correspond to entries in the EInputs enum
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_Void("Activate"),
			InputPortConfig<float>("Left", 0),
			InputPortConfig<float>("Right", 0),
			{0}
		};

		// These should correspond to entries in the EOutputs enum
		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<float>("Answer"),
			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;

		// This description will be displayed for designers
		config.sDescription = "Test of the new base node";

		// This category controls the accessibility of the node in Sandbox. Can be approved, advanced, debug, or obsolete.
		config.SetCategory(EFLN_APPROVED);
	}

	// Called when the eFE_Initialize event is sent
	virtual void OnInit()
	{

	}

	// Called regularly if this node is set to receive updates via this->SetUpdated(true);
	virtual void OnUpdate()
	{

	}

	// Use this to listen for input port activations
	virtual void OnActivate()
	{
		// Check whether a port has been fired with IsActive(portId)
		if(this->IsActive(EIP_Activate))
		{
			// Use GetPortValue<T>(portId) to retrieve an input port's value
			auto sum = this->GetPortValue<float>(EIP_LeftSide) * this->GetPortValue<float>(EIP_RightSide);
			
			// Ports with data can be fired by passing a second parameter
			this->Activate(EOP_Answer, sum);
		}
	}
};

// Register this node using the category:name syntax
REGISTER_FLOW_NODE("Samples:Multiply", CBaseNodeExample);