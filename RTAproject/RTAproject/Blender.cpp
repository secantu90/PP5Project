#include "pch.h"
#include "Blender.h"
#include "Interpolator.h"

Blender::Blender()
{
	m_currBlendTime = 0;
	m_totalBlendTime = 0.2f;
}

void Blender::Update(double _time)
{
	currAnim->Update(_time);
	betweenFrame = currAnim->m_betweenKeyFrame;

	if (nextAnim && nextAnim != currAnim)
	{
		nextAnim->Update(_time);
		m_currBlendTime += _time;

		double deltaTime = m_currBlendTime / m_totalBlendTime;
		betweenFrame = currAnim->Lerp(currAnim->m_betweenKeyFrame, nextAnim->m_betweenKeyFrame, (float)deltaTime);

		if (m_currBlendTime > m_totalBlendTime)
		{
			currAnim->m_currBlendTime = 0;
			currAnim->m_currFrame = 0;
			m_currBlendTime = 0;
			currAnim = nextAnim;
			nextAnim = nullptr;
		}
	}

	//for (unsigned int i = 0; i < m_FBXExporter.m_animation.GetFrame(0).size(); i++)
	//{
	//	XMStoreFloat4x4(&m_boneOffsetsBufferData.offsets[i], XMMatrixTranspose(XMMatrixMultiply(XMLoadFloat4x4(&m_FBXExporter.m_bindPose.GetBindPose()[i]), XMLoadFloat4x4(&m_FBXExporter.m_animation.GetFrame(currentFrame)[i].m_boneMatrix))));
	//}
	skinningMatrixArray.clear();

	for (int i = 0; i < betweenFrame.m_numBones; i++)
	{
		DirectX::XMFLOAT4X4 tempBone;
		XMStoreFloat4x4(&tempBone, DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&(*invBindPose)[i]), DirectX::XMLoadFloat4x4(&betweenFrame.m_bones[i].m_boneMatrix))));

		skinningMatrixArray.push_back(tempBone);
	}

}
