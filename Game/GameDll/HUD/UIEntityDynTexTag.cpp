////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2011.
// -------------------------------------------------------------------------
//  File name:   UIEntityDynTexTag.cpp
//  Version:     v1.00
//  Created:     22/11/2011 by Paul Reindell.
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIEntityDynTexTag.h"

SUIEventHelper<CUIEntityDynTexTag> CUIEntityDynTexTag::s_EventDispatcher;

////////////////////////////////////////////////////////////////////////////
CUIEntityDynTexTag::CUIEntityDynTexTag()
{
	assert(gEnv->pFlashUI);
	if (!gEnv->pFlashUI)
		return;

	// event system to receive events from UI
	m_pUIOFct = gEnv->pFlashUI->CreateEventSystem( "UIEntityTagsDynTex", IUIEventSystem::eEST_UI_TO_SYSTEM );
	m_pUIOFct->RegisterListener( this, "CUIEntityDynTexTag" );

	{
		SUIEventDesc evtDesc( "AddEntityTag", "AddEntityTag", "Adds a 3D entity Tag" );
		evtDesc.Params.push_back( SUIParameterDesc("EntityID", "EntityID", "Entity ID of tagged entity", SUIParameterDesc::eUIPT_Int) );
		evtDesc.Params.push_back( SUIParameterDesc("uiElements_UIElement", "uiElements_UIElement", "UIElement that is used for this tag (Instance with EntityId as instanceId will be created)", SUIParameterDesc::eUIPT_String) );
		evtDesc.Params.push_back( SUIParameterDesc("EntityClass", "EntityClass", "EntityClass of the spawned entity", SUIParameterDesc::eUIPT_String) );
		evtDesc.Params.push_back( SUIParameterDesc("Material", "Material", "Material template that is used for the dyn texture", SUIParameterDesc::eUIPT_String) );
		evtDesc.Params.push_back( SUIParameterDesc("OffsetX", "OffsetX", "X Offset in camera space relative to entity pos", SUIParameterDesc::eUIPT_Float) );
		evtDesc.Params.push_back( SUIParameterDesc("OffsetY", "OffsetY", "Y Offset in camera space relative to entity pos", SUIParameterDesc::eUIPT_Float) );
		evtDesc.Params.push_back( SUIParameterDesc("OffsetZ", "OffsetZ", "Z Offset in camera space relative to entity pos", SUIParameterDesc::eUIPT_Float) );
		evtDesc.Params.push_back( SUIParameterDesc("TagIDX", "TagIDX", "Custom IDX to identify entity tag.", SUIParameterDesc::eUIPT_String) );
		s_EventDispatcher.RegisterEvent( m_pUIOFct, evtDesc, &CUIEntityDynTexTag::OnAddTaggedEntity );
	}

	{
		SUIEventDesc evtDesc( "UpdateEntityTag", "UpdateEntityTag", "Updates a 3D entity Tag" );
		evtDesc.Params.push_back( SUIParameterDesc("EntityID", "EntityID", "Entity ID of tagged entity", SUIParameterDesc::eUIPT_Int) );
		evtDesc.Params.push_back( SUIParameterDesc("TagIDX", "TagIDX", "Custom IDX to identify entity tag.", SUIParameterDesc::eUIPT_String) );
		evtDesc.Params.push_back( SUIParameterDesc("OffsetX", "OffsetX", "X Offset in camera space relative to entity pos", SUIParameterDesc::eUIPT_Float) );
		evtDesc.Params.push_back( SUIParameterDesc("OffsetY", "OffsetY", "Y Offset in camera space relative to entity pos", SUIParameterDesc::eUIPT_Float) );
		evtDesc.Params.push_back( SUIParameterDesc("OffsetZ", "OffsetZ", "Z Offset in camera space relative to entity pos", SUIParameterDesc::eUIPT_Float) );
		evtDesc.Params.push_back( SUIParameterDesc("LerpSpeed", "LerpSpeed", "Define speed of lerp between old and new offset, 0=instant", SUIParameterDesc::eUIPT_Float) );
	s_EventDispatcher.RegisterEvent( m_pUIOFct, evtDesc, &CUIEntityDynTexTag::OnUpdateTaggedEntity );
	}

	{
		SUIEventDesc evtDesc( "RemoveEntityTag", "RemoveEntityTag", "Removes a 3D entity Tag" );
		evtDesc.Params.push_back( SUIParameterDesc("EntityID", "EntityID", "Entity ID of tagged entity", SUIParameterDesc::eUIPT_Int) );
		evtDesc.Params.push_back( SUIParameterDesc("TagIDX", "TagIDX", "Custom IDX to identify entity tag.", SUIParameterDesc::eUIPT_String) );
		s_EventDispatcher.RegisterEvent( m_pUIOFct, evtDesc, &CUIEntityDynTexTag::OnRemoveTaggedEntity );
	}

	{
		SUIEventDesc evtDesc( "RemoveAllEntityTag", "RemoveAllEntityTag", "Removes all 3D entity Tags for given entity" );
		evtDesc.Params.push_back( SUIParameterDesc("EntityID", "EntityID", "Entity ID of tagged entity", SUIParameterDesc::eUIPT_Int) );
		s_EventDispatcher.RegisterEvent( m_pUIOFct, evtDesc, &CUIEntityDynTexTag::OnRemoveAllTaggedEntity );
	}

	gEnv->pFlashUI->RegisterModule(this, "CUIEntityDynTexTag");
}

////////////////////////////////////////////////////////////////////////////
CUIEntityDynTexTag::~CUIEntityDynTexTag()
{
	ClearAllTags();

	if (m_pUIOFct)
		m_pUIOFct->UnregisterListener(this);

	if (gEnv->pFlashUI)
		gEnv->pFlashUI->UnregisterModule(this);
}

////////////////////////////////////////////////////////////////////////////
void CUIEntityDynTexTag::UpdateView(const SViewParams &viewParams)
{
	static const Quat rot90Deg = Quat::CreateRotationXYZ( Ang3(gf_PI * 0.5f, 0, 0) );
	const Vec3& vSafeVec = viewParams.rotation.GetColumn1();

	for (TTags::iterator it = m_Tags.begin(); it != m_Tags.end(); ++it)
	{
		IEntity* pOwner = gEnv->pEntitySystem->GetEntity(it->OwnerId);
		IEntity* pTagEntity = gEnv->pEntitySystem->GetEntity(it->TagEntityId);
		if (pOwner && pTagEntity)
		{
			const Vec3 offset = it->fLerp < 1 ? Vec3::CreateLerp(it->vOffset, it->vNewOffset, it->fLerp) : it->vOffset;
			const Vec3& vPos = pOwner->GetWorldPos();
			const Vec3 vDir = (vPos - viewParams.position).GetNormalizedSafe(vSafeVec);
			const Vec3 vOffsetX = vDir.Cross(Vec3Constants<float>::fVec3_OneZ).GetNormalized() * offset.x;
			const Vec3 vOffsetY = vDir * offset.y;
			const Vec3 vOffsetZ = Vec3(0, 0, offset.z);
			const Vec3 vOffset = vOffsetX + vOffsetY + vOffsetZ;


			const Vec3 vNewPos = vPos + vOffset;
			const Vec3 vNewDir = (vNewPos - viewParams.position).GetNormalizedSafe(vSafeVec);

			const Quat qTagRot = Quat::CreateRotationVDir(vNewDir) * rot90Deg; // rotate 90 degrees around X-Axis
			pTagEntity->SetPos(vNewPos);
			pTagEntity->SetRotation(qTagRot);

			if (it->fLerp < 1)
			{
				assert(it->fSpeed > 0);
				it->fLerp += viewParams.frameTime * it->fSpeed;
				it->vOffset = offset;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CUIEntityDynTexTag::OnEvent( const SUIEvent& event )
{
	s_EventDispatcher.Dispatch( this, event );
}

////////////////////////////////////////////////////////////////////////////
void CUIEntityDynTexTag::Reset()
{
	ClearAllTags();
}

////////////////////////////////////////////////////////////////////////////
void CUIEntityDynTexTag::Reload()
{
	if (gEnv->IsEditor())
	{
		ClearAllTags();
	}
}

////////////////////////////////////////////////////////////////////////////
void CUIEntityDynTexTag::OnInstanceDestroyed( IUIElement* pSender, IUIElement* pDeletedInstance )
{
	for (TTags::iterator it = m_Tags.begin(); it != m_Tags.end(); ++it)
	{
		if (it->pInstance == pDeletedInstance)
			it->pInstance = NULL;
	}
}

void CUIEntityDynTexTag::OnEntityEvent( IEntity *pEntity,SEntityEvent &event )
{
	assert(event.event == ENTITY_EVENT_DONE);
	RemoveAllEntityTags( pEntity->GetId(), false );
}


////////////////////////////////////////////////////////////////////////////
// Arg1: EntityID
// Arg2: UIElement
// Arg3: EntityClass
// Arg4: Material
// Arg5: OffsetX
// Arg6: OffsetY
// Arg7: OffsetZ
// Arg8: IDX
//
void CUIEntityDynTexTag::OnAddTaggedEntity( const SUIEvent& event )
{
	EntityId entityId;
	string uiElementName;
	string entityClass;
	string materialTemplate;
	Vec3 offset;
	string idx;
	if ( event.args.GetArg(0, entityId)
		&& event.args.GetArg(1, uiElementName)
		&& event.args.GetArg(2, entityClass)
		&& event.args.GetArg(3, materialTemplate)
		&& event.args.GetArg(4, offset.x)
		&& event.args.GetArg(5, offset.y)
		&& event.args.GetArg(6, offset.z)
		&& event.args.GetArg(7, idx) )
	{
		AddNewTag(entityId, idx, entityClass, uiElementName, materialTemplate, offset);
	}
}

////////////////////////////////////////////////////////////////////////////
// Arg1: EntityID
// Arg2: IDX
// Arg3: OffsetX
// Arg4: OffsetY
// Arg5: OffsetZ
// Arg6: LerpSpeed
//
void CUIEntityDynTexTag::OnUpdateTaggedEntity( const SUIEvent& event )
{
	EntityId entityId;
	string idx;
	Vec3 offset;
	float speed;
	if ( event.args.GetArg(0, entityId)
		&& event.args.GetArg(1, idx)
		&& event.args.GetArg(2, offset.x)
		&& event.args.GetArg(3, offset.y)
		&& event.args.GetArg(4, offset.z)
		&& event.args.GetArg(5, speed) )
	{
		UpdateEntityTagOffset(entityId, idx, offset, speed);
	}
}

////////////////////////////////////////////////////////////////////////////
// Arg1: EntityID
// Arg2: IDX
//
void CUIEntityDynTexTag::OnRemoveTaggedEntity( const SUIEvent& event )
{
	EntityId entityId;
	string idx;
	if ( event.args.GetArg(0, entityId) && event.args.GetArg(1, idx) )
	{
		RemoveEntityTag(entityId, idx);
	}
}

////////////////////////////////////////////////////////////////////////////
// Arg1: EntityID
//
void CUIEntityDynTexTag::OnRemoveAllTaggedEntity( const SUIEvent& event )
{
	EntityId entityId;
	if ( event.args.GetArg(0, entityId) )
	{
		RemoveAllEntityTags(entityId);
	}
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void CUIEntityDynTexTag::AddNewTag( EntityId entityId, const string& idx, const string& entityClass, const string& uiElementName, const string& materialTemplate, const Vec3& offset )
{
	RemoveEntityTag(entityId, idx);

	IEntityClass* pEntClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass( entityClass.c_str() );
	if (pEntClass)
	{
		SEntitySpawnParams params;
		params.nFlags = ENTITY_FLAG_CLIENT_ONLY;
		params.pClass = pEntClass;

		IEntity* pTagEntity = gEnv->pEntitySystem->SpawnEntity(params);
		IUIElement* pElement = gEnv->pFlashUI->GetUIElement(uiElementName.c_str());
		if (pTagEntity && pElement)
		{
			IMaterial* pMatTemplate = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial(materialTemplate.c_str(), false);
			if (pMatTemplate && pMatTemplate->GetShaderItem().m_pShaderResources->GetTexture(0))
			{
				pMatTemplate->GetShaderItem().m_pShaderResources->GetTexture(0)->m_Name.Format("%s@%d.ui", uiElementName.c_str(), entityId);
				IMaterial* pMat = gEnv->p3DEngine->GetMaterialManager()->CloneMaterial(pMatTemplate);
				pTagEntity->SetMaterial(pMat);
			}
			IEntityRenderProxy* pRenderProxy = (IEntityRenderProxy*) pTagEntity->GetProxy( ENTITY_PROXY_RENDER );
			if (pRenderProxy)
			{
				IRenderNode *pRenderNode = pRenderProxy->GetRenderNode();
				if (pRenderNode)
				{
					pRenderNode->SetViewDistRatio(256);
				}
			}
			pElement->RemoveEventListener(this); // first remove to avoid assert if already registered!
			pElement->AddEventListener(this, "CUIEntityDynTexTag");
			gEnv->pEntitySystem->AddEntityEventListener(entityId, ENTITY_EVENT_DONE, this);
			m_Tags.push_back( STagInfo(entityId, pTagEntity->GetId(), idx, offset, pElement->GetInstance((uint)entityId)) );
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CUIEntityDynTexTag::RemoveEntityTag( EntityId entityId, const string& idx )
{
	for (TTags::iterator it = m_Tags.begin(); it != m_Tags.end(); ++it)
	{
		if (it->OwnerId == entityId && it->Idx == idx)
		{
			gEnv->pEntitySystem->RemoveEntity(it->TagEntityId);
			if (it->pInstance)
				it->pInstance->DestroyThis();
			m_Tags.erase(it);
			break;
		}
	}

	if (!HasEntityTag(entityId))
		gEnv->pEntitySystem->RemoveEntityEventListener(entityId, ENTITY_EVENT_DONE, this);
}

////////////////////////////////////////////////////////////////////////////
void CUIEntityDynTexTag::RemoveAllEntityTags( EntityId entityId, bool bUnregisterListener )
{
	for (TTags::iterator it = m_Tags.begin(); it != m_Tags.end();)
	{
		if (it->OwnerId == entityId)
		{
			gEnv->pEntitySystem->RemoveEntity(it->TagEntityId);
			if (it->pInstance)
				it->pInstance->DestroyThis();
			it = m_Tags.erase(it);
		}
		else
		{
			++it;
		}
	}
	if (bUnregisterListener)
		gEnv->pEntitySystem->RemoveEntityEventListener(entityId, ENTITY_EVENT_DONE, this);
}

////////////////////////////////////////////////////////////////////////////
void CUIEntityDynTexTag::UpdateEntityTagOffset( EntityId entityId, const string& idx, Vec3 offset, float speed )
{
	for (TTags::iterator it = m_Tags.begin(); it != m_Tags.end(); ++it)
	{
		if (it->OwnerId == entityId && it->Idx == idx)
		{
			it->fSpeed = speed;
			if (speed > 0)
			{
				it->fLerp = 0;
				it->vNewOffset = offset;
			}
			else
			{
				it->fLerp = 1;
				it->vOffset = offset;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CUIEntityDynTexTag::ClearAllTags()
{
	for (TTags::const_iterator it = m_Tags.begin(); it != m_Tags.end(); ++it)
	{
		gEnv->pEntitySystem->RemoveEntity(it->TagEntityId);
		if (it->pInstance)
			it->pInstance->DestroyThis();
		gEnv->pEntitySystem->RemoveEntityEventListener(it->OwnerId, ENTITY_EVENT_DONE, this);
	}
	m_Tags.clear();
}

////////////////////////////////////////////////////////////////////////////
bool CUIEntityDynTexTag::HasEntityTag( EntityId entityId ) const
{
	for (TTags::const_iterator it = m_Tags.begin(); it != m_Tags.end(); ++it)
	{
		if (it->OwnerId == entityId)
		{
			return true;
		}
	}
	return false;
}