#include "pch.h"
#include "Interpolator.h"


Interpolator::Interpolator()
{
	m_currBlendTime = 0;
	m_totalBlendTime = 0;
}


UMLKeyFrame Interpolator::Lerp(UMLKeyFrame _currFrame, UMLKeyFrame _nextFrame, float _deltaTime)
{

	UMLKeyFrame temp = _currFrame;


	for (unsigned int bone = 0; bone < _currFrame.m_bones.size(); ++bone)
	{
		DirectX::XMVECTOR scaleCur, scaleNext;
		DirectX::XMVECTOR rotationCur, rotationNext;
		DirectX::XMVECTOR positionCur, positionNext;
		DirectX::XMMatrixDecompose(&scaleCur, &rotationCur, &positionCur, DirectX::XMLoadFloat4x4(&_currFrame.m_bones[bone].m_boneMatrix));
		DirectX::XMMatrixDecompose(&scaleNext, &rotationNext, &positionNext, DirectX::XMLoadFloat4x4(&_nextFrame.m_bones[bone].m_boneMatrix));
		DirectX::XMVECTOR rotNow = DirectX::XMQuaternionSlerp(rotationCur, rotationNext, _deltaTime);
		DirectX::XMVECTOR scaleNow = DirectX::XMVectorLerp(scaleCur, scaleNext, _deltaTime);
		DirectX::XMVECTOR posNow = DirectX::XMVectorLerp(positionCur, positionNext, _deltaTime);

		DirectX::XMMATRIX matrixNow = DirectX::XMMatrixAffineTransformation(scaleNow, DirectX::XMVectorZero(), rotNow, posNow);

		DirectX::XMStoreFloat4x4(&temp.m_bones[bone].m_boneMatrix, matrixNow);
	}
	return temp;
}


Boint* Interpolator::GetCurrentBoint(int& _num)
{
	return &m_boints[_num];
}
double Interpolator::GetTime()
{
	return m_currBlendTime;
}
void Interpolator::SetAnimation(const Animation* _animation)
{
	m_animation = _animation;
}
void Interpolator::SetTime(double _time)
{
	m_totalBlendTime = _time;
}
ANIM_TYPE Interpolator::Update(double _time)
{

	m_currBlendTime += _time;
	UMLKeyFrame currKeyFrame = m_animation->m_frames[m_currFrame];
	m_totalBlendTime += currKeyFrame.m_time;

	UMLKeyFrame nextKeyFrame;
	if ((m_currFrame) >= m_animation->m_frames.size() - 1)
	{
		m_currFrame = 0;
		currKeyFrame = m_animation->m_frames[0];
		nextKeyFrame = m_animation->m_frames[1];
	}
	else
		nextKeyFrame = m_animation->m_frames[m_currFrame + 1];

	frameTime = m_currBlendTime - currKeyFrame.m_time;

	double tweenTime = nextKeyFrame.m_time - currKeyFrame.m_time;
	while (frameTime > tweenTime)
	{
		++m_currFrame;
		if (m_currFrame >= m_animation->m_frames.size() - 1)
		{
			m_currFrame = 0;
		}
		currKeyFrame = m_animation->m_frames[m_currFrame];
		nextKeyFrame = m_animation->m_frames[m_currFrame + 1];
		frameTime -= tweenTime;
	}

	double deltaTime = frameTime / tweenTime;
	if (m_currFrame == m_animation->m_frames.size())
		m_currFrame = 0;
	m_betweenKeyFrame = Lerp(currKeyFrame, nextKeyFrame, static_cast<float>(deltaTime));

	return m_animation->m_animType;
}
