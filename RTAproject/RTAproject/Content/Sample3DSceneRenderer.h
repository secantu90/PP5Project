#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include "FBXExporter.h"
#include "DDSTextureLoader.h"
#include <DirectXMath.h>
#include <vector>
#include "Model.h"
#include "FileManage.h"
namespace RTAproject
{
	// This sample renderer instantiates a basic rendering pipeline.
	class Sample3DSceneRenderer
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();
		void StartTracking();
		void TrackingUpdate(float positionX);
		void StopTracking();
		bool IsTracking() { return m_tracking; }


	private:
		void Rotate(float radians);

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Direct3D resources for cube geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayoutAni;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_geoinputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vpconstantBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBufferBox;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBufferQuad;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBufferBox;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBufferQuad;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_modelconstantBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_geovertexShader;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShaderAnimation;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShaderSimple;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader>	m_geometryShader;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader>	m_nullShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_boneOffsetsBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBufferLights;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBufferLightsPosition;
		ID3D11ShaderResourceView*					m_shaderView[3];
		ID3D11SamplerState*						    m_sampleState = NULL;


		// System resources for cube geometry.
		ModelViewProjectionConstantBuffer	m_constantBufferData;
		ModelConstantBuffer				m_modelconstantBufferData;
		ViewProjectionConstantBuffer	m_geoconstantBufferData;
		BoneOffsets							m_boneOffsetsBufferData;
		LightBuffer							m_constantBufferLightData;
		PLightPosBuffer						m_constantBufferLightPosData;

		uint32	m_indexCount;
		uint32	m_indexCountBox;
		uint32  m_indexCountQuad;

		FBXExporter m_FBXExporter;
		Animation* m_Animation;
		Animation m_temp;
		BindPose* m_Bind;
		BindPose m_tempbind;
		Filemanage m_FileIO;
		std::vector<RTAproject::RobustVertex> m_Vertices;
		std::vector<unsigned short> m_indices;
		std::vector<RTAproject::RobustVertex> m_tempVert;
		std::vector<unsigned short> m_tempind;

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool	m_tracking;

		int time;
		int currentFrame = 0;

		//////////////////////////////////////////////////////////////////////////////////////
		//Emilio
		//used for camera movement
		DirectX::XMFLOAT4X4 world, camera, proj;

		//Microsoft::WRL::ComPtr<ID3D11SamplerState>		m_sampleState;
		//
		////Earth resources
		//Model											Earth;
		//Microsoft::WRL::ComPtr<ID3D11InputLayout>		m_earthInputLayout;
		//Microsoft::WRL::ComPtr<ID3D11VertexShader>		m_earthVS;
		//Microsoft::WRL::ComPtr<ID3D11PixelShader>		m_earthPS;
		//Microsoft::WRL::ComPtr<ID3D11Buffer>			m_earthConstBuff;
		//WorldWorldInvWorldViewProjTexTransform			m_earthConstBuffData;
		//EyeFog											m_earthEyeFogConstBuffData;
		//
		//
		//
		//sky box resources
		Model											skyBox;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>		m_skyBoxInputLayout;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>		m_skyBoxVS;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>		m_skyBoxPS;
		Microsoft::WRL::ComPtr<ID3D11Buffer>			m_skyBoxConstBuff;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>	m_clockwise;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>	m_counterClockwise;
		//
		////used for loading OBJ files
		//void LoadObject(std::vector<VERTEX3>& interVert, std::vector<unsigned int>& interIndex, const char* file);
		//
		//End Emilio
		/////////////////////////////////////////////////////////////////////////////////////

	};
}

