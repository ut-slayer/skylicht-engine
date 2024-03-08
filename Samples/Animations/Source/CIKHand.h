#pragma once

#include "Components/ILateUpdate.h"
#include "Components/CComponentSystem.h"
#include "Debug/CSceneDebug.h"
#include "Animation/Skeleton/CAnimationTransformData.h"
#include "Animation/IK/CIKSolver.h"

class CIKHand :
	public CComponentSystem,
	public ILateUpdate
{
protected:
	struct SHand
	{
		CAnimationTransformData* Start;
		CAnimationTransformData* Mid;
		CAnimationTransformData* End;

		SHand()
		{
			Start = NULL;
			Mid = NULL;
			End = NULL;
		}
	};

	SHand m_leftHand;
	SHand m_rightHand;
	CAnimationTransformData* m_gun;

	core::vector3df m_aimTarget;

	CIKSolver m_ikLeftHand;
	CIKSolver m_ikRightHand;

	float m_aimWeight;

public:
	CIKHand();

	virtual ~CIKHand();

	virtual void initComponent();

	virtual void updateComponent();

	virtual void lateUpdate();

	void setGun(CAnimationTransformData* gun);

	void setLeftHand(CAnimationTransformData* start, CAnimationTransformData* mid, CAnimationTransformData* end);

	void setRightHand(CAnimationTransformData* start, CAnimationTransformData* mid, CAnimationTransformData* end);

	void setAimTarget(const core::vector3df& aimTarget, float aimWeight)
	{
		m_aimTarget = aimTarget;
		m_aimWeight = aimWeight;
	}

private:

	core::vector3df getPoleVec(const core::vector3df* p);
};
