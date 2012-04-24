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

// Register this node with a category and name
REGISTER_FLOW_NODE("Samples:Multiply", CBaseNodeExample);

// Type tests, not useful for anything production-related
class CNodeTypeTests : public CBaseNode
{
private:
	enum EInputs
	{
		EIP_Void,
		EIP_Int,
		EIP_Float,
		EIP_String,
		EIP_Vec3
	};

	enum EOutputs
	{
		EOP_Void,
		EOP_Int,
		EOP_Float,
		EOP_String,
		EOP_Vec3
	};

public:
	CNodeTypeTests(SActivationInfo *pActInfo)
	{

	}
		
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_Void("Activate"),
			InputPortConfig<int>("Integer", 1),
			InputPortConfig<float>("Float", 0.5f),
			InputPortConfig<string>("String"),
			InputPortConfig<Vec3>("Vector", Vec3(0)),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_Void("Activate"),
			OutputPortConfig<int>("Integer"),
			OutputPortConfig<float>("Float"),
			OutputPortConfig<string>("String"),
			OutputPortConfig<Vec3>("Vector"),
			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;

		config.sDescription = "Sanity test for functionality";

		config.SetCategory(EFLN_DEBUG);
	}

	virtual IFlowNodePtr Clone(SActivationInfo *pActInfo)
	{
		return new CNodeTypeTests(pActInfo);
	}

	virtual void OnActivate()
	{
		if(this->IsActive(EIP_Void))
		{
			this->Activate(EOP_Int, this->GetPortValue<int>(EIP_Int));
			this->Activate(EOP_Float, this->GetPortValue<float>(EIP_Float));
			this->Activate(EOP_String, this->GetPortValue<string>(EIP_String));
			this->Activate(EOP_Vec3, this->GetPortValue<Vec3>(EIP_Vec3));

			this->Activate(EOP_Void);
		}
	}
};

REGISTER_FLOW_NODE("Samples:TypeTests", CNodeTypeTests);