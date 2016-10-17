#include "pch.h"
#include "Interpolator.h"


Interpolator::Interpolator() {}


UMLKeyFrame Lerp(UMLKeyFrame _currFrame, UMLKeyFrame _nextFrame, double _deltaTime)
{
	//UMLKeyFrame temp = _currFrame;
	//for (unsigned int bone = 0; bone < temp.m_bones.size(); ++bone)
	//{
	//	for (unsigned int i = 0; i < 4; ++i)
	//	{
	//		for (unsigned int j = 0; j < 4; ++j)
	//		{
	//			temp.m_bones[bone].m_boneMatrix.m[i][j] = _currFrame.m_bones[0].m_boneMatrix.m[i][j] + (_nextFrame.m_bones[0].m_boneMatrix.m[i][j] - _currFrame.m_bones[0].m_boneMatrix.m[i][j]) * _deltaTime;
	//		}
	//	}
	//}
	//return temp;
	UMLKeyFrame temp = _currFrame;


	for (unsigned int bone = 0; bone < _currFrame.m_bones.size(); ++bone)
	{
		DirectX::XMVECTOR scaleCur, scaleNext;
		DirectX::XMVECTOR rotationCur, rotationNext;
		DirectX::XMVECTOR positionCur, positionNext;
		DirectX::XMMatrixDecompose(&scaleCur, &rotationCur, &positionCur, DirectX::XMLoadFloat4x4(&_currFrame.m_bones[bone].m_boneMatrix));
		DirectX::XMMatrixDecompose(&scaleNext, &rotationNext, &positionNext, DirectX::XMLoadFloat4x4(&_currFrame.m_bones[bone].m_boneMatrix));
		DirectX::XMVECTOR rotNow, scaleNow, posNow;
		rotNow = DirectX::XMQuaternionSlerp(rotationCur, rotationNext, _deltaTime);
		scaleNow = DirectX::XMVectorLerp(scaleCur, scaleNext, _deltaTime);
		posNow = DirectX::XMVectorLerp(positionCur, positionNext, _deltaTime);

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
	while (m_currBlendTime > m_animation->m_frames[m_currFrame].m_time)
	{
		++m_currFrame;
		m_currBlendTime -= m_animation->m_frames[m_currFrame].m_time;
	}

	UMLKeyFrame nextKeyFrame;
	UMLKeyFrame prevKeyFrame;
	if ((m_currFrame + 1) != m_animation->m_frames.size())
		nextKeyFrame = m_animation->m_frames[m_currFrame + 1];
	else
		nextKeyFrame = m_animation->m_frames[m_currFrame];
	if ((m_currFrame - 1) != 0)
		prevKeyFrame = m_animation->m_frames[m_currFrame - 1];
	else
		prevKeyFrame = m_animation->m_frames[m_currFrame];

	double tweenTime = prevKeyFrame.m_time - nextKeyFrame.m_time;
	double deltaTime = m_currBlendTime / tweenTime;
	m_betweenKeyFrame = Lerp(m_animation->m_frames[m_currFrame], nextKeyFrame, deltaTime);

	return m_animation->m_animType;
}
