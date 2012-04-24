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

	// Return a new instance of this node
	virtual IFlowNodePtr Clone(SActivationInfo *pActInfo)
	{
		return new CBaseNodeExample(pActInfo);
	}

	// Set up your inputs, outputs, category and description here
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_Void("Activate"),
			InputPortConfig<string>("String Input", ""),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_Void("Activated"),
			OutputPortConfig<string>("String Fired"),
			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = "Test of the new base node";
		config.SetCategory(EFLN_APPROVED);
	}

	// Called when this node is initialised, which is when game mode is entered
	virtual void OnInit()
	{
		CryLogAlways("Initialised!");
	}

	virtual void OnActivate()
	{
		if(IsActive(EIP_Activate))
		{
			Activate(EOP_Activated);
		}

		if(IsActive(EIP_String))
		{
			string value = GetPortValue<string>(EIP_String);
			Activate(EOP_String, value);
		}
	}
};

REGISTER_FLOW_NODE("Samples:Test", CBaseNodeExample);