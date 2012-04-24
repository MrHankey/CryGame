#include "StdAfx.h"
#include "BaseNode.h"

class CBaseNodeExample : public CBaseNode
{
private:
	enum EInputs
	{
		EIP_Activate,
		EIP_String,
		EIP_Int,
		EIP_Float,
		EIP_Vec3
	};

	enum EIOutputs
	{
		EOP_Activated,
		EOP_String,
		EOP_Int,
		EOP_Float,
		EOP_Vec3
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

	// Return a new instance of this node, has to be implemented per class
	virtual IFlowNodePtr Clone(SActivationInfo *pActInfo)
	{
		return new CBaseNodeExample(pActInfo);
	}

	// Set up your inputs, outputs, category and description here
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		// These should correspond to entries in the EInputs enum
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_Void("Activate"),
			InputPortConfig<string>("String Input", ""),
			{0}
		};

		// These should correspond to entries in the EOutputs enum
		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_Void("Activated"),
			OutputPortConfig<string>("String Fired"),
			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;

		// This description will be displayed for designers
		config.sDescription = "Test of the new base node";

		// This category controls the accessibility of the node in Sandbox. Can be approved, advanced, debug, or obsolete.
		config.SetCategory(EFLN_APPROVED);
	}

	// Called when this node is initialised, which is when game mode is entered
	virtual void OnInit()
	{
		CryLogAlways("Initialised!");

		// To subscribe to regular updates via OnUpdate(), use the following:
		//this->SetUpdated(true);
	}

	virtual void OnUpdate()
	{

	}

	virtual void OnActivate()
	{
		// Use IsActive and your port ID to check whether an input has been fired
		if(IsActive(EIP_Activate))
		{
			// Void outputs can be triggered via Activate with no additional paramemeters
			Activate(EOP_Activated);
		}

		if(IsActive(EIP_String))
		{
			// Use GetPortValue<T> to retrieve an input port's value
			string value = GetPortValue<string>(EIP_String);
			
			// Ports with data can be fired by passing a second parameter
			Activate(EOP_String, value);
		}
	}
};

// Register this node with a category and name
REGISTER_FLOW_NODE("Samples:Test", CBaseNodeExample);