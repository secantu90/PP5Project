#pragma once
#include "Utilities.h"
#include "Animation.h"
#include <vector>
class Interpolator
{
public:
	const Animation		*m_animation;
	double				m_currBlendTime;
	double				frameTime;
	double				m_totalBlendTime;
	int					m_currFrame;
	UMLKeyFrame			m_betweenKeyFrame;
	std::vector<Boint>	m_boints;

	Interpolator();
	Boint* GetCurrentBoint(int& _num);
	double GetTime();
	void SetAnimation(const Animation* _animation);
	void SetTime(double _time);
	ANIM_TYPE Update(double _time);
};

