/*
/*
!@
MIT License

Copyright (c) 2022 Skylicht Technology CO., LTD

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify,
merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

This file is part of the "Skylicht Engine".
https://github.com/skylicht-lab/skylicht-engine
!#
*/

#include "pch.h"
#include "CPrimitive.h"
#include "CPrimitiveRenderer.h"
#include "GameObject/CGameObject.h"
#include "Entity/CEntityManager.h"
#include "Transform/CWorldTransformData.h"
#include "Transform/CWorldInverseTransformData.h"
#include "Culling/CCullingData.h"
#include "Culling/CCullingBBoxData.h"
#include "IndirectLighting/CIndirectLightingData.h"


namespace Skylicht
{
	CPrimitive::CPrimitive() :
		m_type(CPrimiviteData::Unknown)
	{

	}

	CPrimitive::~CPrimitive()
	{

	}

	void CPrimitive::initComponent()
	{
		m_gameObject->getEntityManager()->addRenderSystem<CPrimitiveRenderer>();

		// add default primitive
		if (m_type != CPrimiviteData::Unknown)
		{
			addPrimitive(
				core::vector3df(),
				core::vector3df(),
				core::vector3df(1.0f, 1.0f, 1.0f)
			);
		}
	}

	CEntity* CPrimitive::spawn()
	{
		return addPrimitive(core::vector3df(), core::vector3df(), core::vector3df(1.0f, 1.0f, 1.0f));
	}

	CEntity* CPrimitive::addPrimitive(const core::vector3df& pos, const core::vector3df& rotDeg, const core::vector3df& scale)
	{
		CEntity* entity = createEntity();

		CPrimiviteData* primitiveData = entity->addData<CPrimiviteData>();
		primitiveData->Type = m_type;

		// Culling
		entity->addData<CWorldInverseTransformData>();
		entity->addData<CCullingData>();
		entity->addData<CIndirectLightingData>();

		CCullingBBoxData* cullingBBox = entity->addData<CCullingBBoxData>();
		cullingBBox->BBox.MinEdge.set(-1.0f, -1.0f, -1.0f);
		cullingBBox->BBox.MaxEdge.set(1.0f, 1.0f, 1.0f);

		// Position
		CWorldTransformData* transform = (CWorldTransformData*)entity->getDataByIndex(CWorldTransformData::DataTypeIndex);
		transform->Relative.setTranslation(pos);
		transform->Relative.setRotationDegrees(rotDeg);
		transform->Relative.setScale(scale);

		// Indirect lighting
		CIndirectLightingData* indirect = (CIndirectLightingData*)entity->getDataByIndex(CIndirectLightingData::DataTypeIndex);
		indirect->Type = CIndirectLightingData::SH9;
		indirect->AutoSH = new bool();
		indirect->SH = new core::vector3df[9];
		indirect->ReleaseSH = true;

		*indirect->AutoSH = true;
		return entity;
	}
}