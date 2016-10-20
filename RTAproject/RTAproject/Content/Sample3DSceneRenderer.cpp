#include "pch.h"
#include "Sample3DSceneRenderer.h"

#include "..\Common\DirectXHelper.h"

using namespace RTAproject;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };
	XMStoreFloat4x4(&camera, XMMatrixLookAtLH(eye, at, up));

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
}

// Initializes view parameters when the window size changes.
void Sample3DSceneRenderer::CreateWindowSizeDependentResources()
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
	);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&m_constantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
	);

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	//static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
	//static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	//static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };
	//XMStoreFloat4x4(&camera, XMMatrixLookAtLH(eye, at, up));

	//XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));

	//XMFLOAT4X4 identity;
	//XMStoreFloat4x4(&identity, XMMatrixIdentkity());
	//m_constantBufferData.world = identity;

	m_constantBufferLightData.Ar = 1;
	m_constantBufferLightData.Ag = 0;
	m_constantBufferLightData.Ab = 0;
	m_constantBufferLightData.Aa = 1;
	m_constantBufferLightData.r = 1;
	m_constantBufferLightData.g = 1;
	m_constantBufferLightData.b = 1;
	m_constantBufferLightData.a = 1;
	m_constantBufferLightData.x = 0;
	m_constantBufferLightData.y = 0;
	m_constantBufferLightData.z = 1;
	m_constantBufferLightData.w = 0;
	m_constantBufferLightData.sX = 0;
	m_constantBufferLightData.sY = -1;
	m_constantBufferLightData.sZ = 0;
	m_constantBufferLightData.sW = 0;

	m_constantBufferLightPosData.x = 0;
	m_constantBufferLightPosData.y = .05f;
	m_constantBufferLightPosData.z = 0;
	m_constantBufferLightPosData.w = 1;
	m_constantBufferLightPosData.sX = 0;
	m_constantBufferLightPosData.sY = .1f;
	m_constantBufferLightPosData.sZ = 0;
	m_constantBufferLightPosData.sW = 1;


	time = 0;
}

//////////////////////////////////////////////////////////////////////////
//Emilio
extern bool mouseMoved;
extern float newX;
extern float newY;
extern bool leftClick;
extern char keys[256];
//End Emilio
//////////////////////////////////////////////////////////////////////////

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{


	//For specular lighting camera tracking
	m_constantBufferLightData.cX = m_constantBufferData.view._41;
	m_constantBufferLightData.cY = m_constantBufferData.view._42;
	m_constantBufferLightData.cZ = m_constantBufferData.view._43;
	m_constantBufferLightData.cW = m_constantBufferData.view._44;

	int timeTemp = static_cast<int>(timer.GetTotalSeconds());
	if (timeTemp - time < 5)
	{
		m_constantBufferLightPosData.sX += .001f;
		m_constantBufferLightPosData.z -= .001f;
		m_constantBufferLightData.z = 1;
		m_constantBufferLightData.sZ = -.75f;
	}
	else
	{
		m_constantBufferLightPosData.sX -= .001f;
		m_constantBufferLightPosData.z += .001f;
		m_constantBufferLightData.z = -1;
		m_constantBufferLightData.sZ = .75f;
		if (timeTemp - time > 9)
			time = timeTemp;
	}


	XMFLOAT4X4 temp;
	XMStoreFloat4x4(&temp, XMMatrixMultiply(XMMatrixScaling(5, 5, 5), XMMatrixIdentity()));
	m_constantBufferData.model = temp;

	//////////////////////////////////////////////////////////////////////////
	//Emilio
	enum CameraMovement
	{
		camMoveX = 0,
		camMoveY,
		camMoveZ,
		camPos
	};

	XMMATRIX newcamera = XMLoadFloat4x4(&camera);
	//move left
	if (keys['A'])
		newcamera.r[camPos] += (newcamera.r[camMoveX] * static_cast<float>(-timer.GetElapsedSeconds()) *5.0f);
	//move right			   																			
	if (keys['D'])
		newcamera.r[camPos] += (newcamera.r[camMoveX] * static_cast<float>(timer.GetElapsedSeconds()) * 5.0f);
	//move up				   																			
	if (keys['R'])
		newcamera.r[camPos] += (newcamera.r[camMoveY] * static_cast<float>(timer.GetElapsedSeconds()) * 5.0f);
	//move down				   
	if (keys['F'])
		newcamera.r[camPos] += (newcamera.r[camMoveY] * static_cast<float>(-timer.GetElapsedSeconds()) * 5.0f);
	//move forward			  																			
	if (keys['W'])
		newcamera.r[camPos] += (newcamera.r[camMoveZ] * static_cast<float>(timer.GetElapsedSeconds()) * 5.0f);
	//move back				   																		 
	if (keys['S'])
		newcamera.r[camPos] += (newcamera.r[camMoveZ] * static_cast<float>(-timer.GetElapsedSeconds()) * 5.0f);


	//AdvanceAnimation
	if (keys[VK_SPACE] || keys['1'])
	{
		currentFrame++;
		if (currentFrame >= m_FBXExporter.m_animation.m_numKeyFrames)
		{
			currentFrame = 0;
		}

		keys['1'] = false;
	}


	if (mouseMoved)
	{
		// Updates the application state once per frame.
		if (leftClick)
		{
			XMVECTOR pos = newcamera.r[camPos];
			newcamera.r[camPos] = XMLoadFloat4(&XMFLOAT4(0, 0, 0, 1));
			newcamera = XMMatrixRotationX(newY * 0.01f) * newcamera * XMMatrixRotationY(newX * 0.01f);
			newcamera.r[camPos] = pos;
		}
	}

	XMStoreFloat4x4(&camera, newcamera);
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(0, newcamera)));
	mouseMoved = false;

	////create skybox world matrix
	//XMMATRIX skyBoxWM = XMMatrixIdentity();
	//skyBoxWM = XMMatrixMultiply(XMMatrixScaling(100.0f, 100.0f, 100.0f), skyBoxWM);
	//skyBox.WM = skyBoxWM;
	skyBox.WM.r[3].m128_f32[0] = newcamera.r[camPos].m128_f32[0];
	skyBox.WM.r[3].m128_f32[1] = newcamera.r[camPos].m128_f32[1];
	skyBox.WM.r[3].m128_f32[2] = newcamera.r[camPos].m128_f32[2];
	skyBox.WM.r[3].m128_f32[3] = newcamera.r[camPos].m128_f32[3];


	//End Emilio
	//////////////////////////////////////////////////////////////////////////
}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixMultiply(XMMatrixTranspose(XMMatrixRotationY(radians)),XMMatrixScaling(0.1f,0.1f,0.1f)));
}

void Sample3DSceneRenderer::StartTracking()
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void Sample3DSceneRenderer::TrackingUpdate(float positionX)
{
	if (m_tracking)
	{
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians);
	}
}

void Sample3DSceneRenderer::StopTracking()
{
	m_tracking = false;
}

// Renders one frame using the vertex and pixel shaders.
void Sample3DSceneRenderer::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();




	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	//Prepare the directional light constant buffer.
	context->UpdateSubresource1(
		m_constantBufferLights.Get(),
		0,
		NULL,
		&m_constantBufferLightData,
		0,
		0,
		0
	);

	//Prepare the spot light and point light position constant buffer.
	context->UpdateSubresource1(
		m_constantBufferLightsPosition.Get(),
		0,
		NULL,
		&m_constantBufferLightPosData,
		0,
		0,
		0
	);



	//Bind Sampler state
	context->PSSetSamplers(0, 1, &m_sampleState);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	////////////////////////////////////////////////////////////////////////////
	//Emilio
	//Draw SkyBox
	//context->RSSetState(m_counterClockwise.Get());
	context->RSSetState(m_clockwise.Get());

	context->IASetInputLayout(m_skyBoxInputLayout.Get());
	context->VSSetShader(
		m_skyBoxVS.Get(),
		nullptr,
		0
	);
	context->PSSetShader(
		m_skyBoxPS.Get(),
		nullptr,
		0
	);
	//context->PSSetSamplers(
	//	0,
	//	1,
	//	m_sampleState.GetAddressOf()
	//	);
	//context->PSSetSamplers(
	//	1,
	//	1,
	//	m_sampleState.GetAddressOf()
	//	);
	XMMATRIX newSkyBoxWM = XMMatrixTranspose(skyBox.WM);
	context->PSSetShaderResources(0, 1, skyBox.resourceView.GetAddressOf());
	skyBox.SetBuffers(context);
	XMStoreFloat4x4(&m_constantBufferData.model, newSkyBoxWM);
	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);
	skyBox.Draw(context);
	//context->RSSetState(m_clockwise.Get());
	context->RSSetState(m_counterClockwise.Get());
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0F, 0);

	//End Emilio
	////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////
	//Antonio
	////////////
	XMStoreFloat4x4(&m_modelconstantBufferData.model, DirectX::XMMatrixIdentity());
	m_geoconstantBufferData.projection = m_constantBufferData.projection;
	m_geoconstantBufferData.view = m_constantBufferData.view;
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBufferQuad.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBufferQuad.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	context->IASetInputLayout(m_geoinputLayout.Get());
	context->UpdateSubresource1(
		m_modelconstantBuffer.Get(),
		0,
		NULL,
		&m_modelconstantBufferData,
		0,
		0,
		0
	);
	context->UpdateSubresource1(
		m_vpconstantBuffer.Get(),
		0,
		NULL,
		&m_geoconstantBufferData,
		0,
		0,
		0
	);
	context->VSSetShader(
		m_geovertexShader.Get(),
		nullptr,
		0
	);

	context->PSSetShader(
		m_pixelShaderSimple.Get(),
		nullptr,
		0
	);

	context->VSSetConstantBuffers1(
		0,
		1,
		m_modelconstantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->GSSetConstantBuffers1(
		0,
		1,
		m_vpconstantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->GSSetShader(
		m_geometryShader.Get(),
		nullptr,
		0
	);

	context->DrawIndexed(
		m_indexCountQuad,
		0,
		0
	);
	m_nullShader = nullptr;
	context->GSSetShader(
		m_nullShader.Get(),
		nullptr,
		0
	);
	//End Antonio
	////////////////////////////////////////////////////////////////////////////

	XMStoreFloat4x4(&m_constantBufferData.model, DirectX::XMMatrixIdentity());


	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	//Prepare the directional light constant buffer.
	context->UpdateSubresource1(
		m_constantBufferLights.Get(),
		0,
		NULL,
		&m_constantBufferLightData,
		0,
		0,
		0
	);

	//Prepare the spot light and point light position constant buffer.
	context->UpdateSubresource1(
		m_constantBufferLightsPosition.Get(),
		0,
		NULL,
		&m_constantBufferLightPosData,
		0,
		0,
		0
	);

	//Bind Sampler state
	context->PSSetSamplers(0, 1, &m_sampleState);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// Each vertex is one instance of the VertexPositionColor struct.
	 stride = sizeof(VertexPositionColor);
	 offset = 0;
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBuffer.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);


	context->IASetInputLayout(m_inputLayout.Get());


	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShader.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Send the constant buffer for the directional light.
	context->PSSetConstantBuffers1(
		0,
		1,
		m_constantBufferLights.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->PSSetConstantBuffers1(
		1,
		1,
		m_constantBufferLightsPosition.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShader.Get(),
		nullptr,
		0
	);

	context->PSSetShaderResources(0, 3, m_shaderView);


	// Draw the objects.
	context->DrawIndexed(
		m_indexCount,
		0,
		0
	);
	

	///////////////////////////////////////////////////
	//Dallas
	// Prepare the constant buffer to send it to the graphics device.
	static float angle = 0;
	//++angle;
	XMStoreFloat4x4(&m_constantBufferData.model, DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(angle)) * DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f));

	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
		);
	for (size_t i = 0; i < m_FBXExporter.m_animation.GetFrame(0).size(); ++i)
	{
		XMStoreFloat4x4(&m_boneOffsetsBufferData.offsets[i], XMMatrixTranspose(XMMatrixMultiply(XMLoadFloat4x4(&m_FBXExporter.m_Skeleton.m_joints[i].m_globalBindposeInverse), XMLoadFloat4x4(&m_FBXExporter.m_animation.GetFrame(currentFrame)[i].m_boneMatrix))));
		//XMStoreFloat4x4(&m_boneOffsetsBufferData.offsets[1], XMMatrixTranspose(XMMatrixMultiply(XMLoadFloat4x4(&m_FBXExporter.m_Skeleton.m_joints[1].m_globalBindposeInverse), XMLoadFloat4x4(&m_FBXExporter.m_animation.GetFrame(currentFrame)[1].m_boneMatrix))));
		//XMStoreFloat4x4(&m_boneOffsetsBufferData.offsets[2], XMMatrixTranspose(XMMatrixMultiply(XMLoadFloat4x4(&m_FBXExporter.m_Skeleton.m_joints[2].m_globalBindposeInverse), XMLoadFloat4x4(&m_FBXExporter.m_animation.GetFrame(currentFrame)[2].m_boneMatrix))));
		//XMStoreFloat4x4(&m_boneOffsetsBufferData.offsets[3], XMMatrixTranspose(XMMatrixMultiply(XMLoadFloat4x4(&m_FBXExporter.m_Skeleton.m_joints[3].m_globalBindposeInverse), XMLoadFloat4x4(&m_FBXExporter.m_animation.GetFrame(currentFrame)[3].m_boneMatrix))));
	}

	context->UpdateSubresource1(
		m_boneOffsetsBuffer.Get(),
		0,
		NULL,
		&m_boneOffsetsBufferData,
		0,
		0,
		0
	);

	// Each vertex is one instance of the VertexPositionColor struct.
	stride = sizeof(RobustVertex);
	offset = 0;
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBufferBox.GetAddressOf(),
		&stride,
		&offset
		);

	context->IASetIndexBuffer(
		m_indexBufferBox.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
		);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayoutAni.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShaderAnimation.Get(),
		nullptr,
		0
		);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
		);

	context->VSSetConstantBuffers1(
		1,
		1,
		m_boneOffsetsBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShader.Get(),// m_pixelShaderSimple.Get(),
		nullptr,
		0
		);

	// Draw the objects.
	context->DrawIndexed(
		m_indexCountBox,
		0,
		0
		);
	//End Dallas
	///////////////////////////////////////////////////
	
}

void Sample3DSceneRenderer::CreateDeviceDependentResources()
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");
	auto loadPSSimpleTask = DX::ReadDataAsync(L"SimplePS.cso");
	auto loadVSAniTask = DX::ReadDataAsync(L"VertexAnimationShader.cso");
	///////////////////////////////////////////////////////////////////////////////
	//Emilio
	//Load skyBox tasks
	auto loadSkyVSTask = DX::ReadDataAsync(L"SkyBoxVS.cso");
	auto loadSkyPSTask = DX::ReadDataAsync(L"SkyBoxPS.cso");

	//End Emilio
	/////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	//Antonio
	//Geometry
	auto loadGSTask = DX::ReadDataAsync(L"GeometryShader.cso");
	auto loadGVSTask = DX::ReadDataAsync(L"VertexShader.cso");
	//end
	///////////////////////////////////////////////////////////////
	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShader
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayout
			)
		);
	});
	auto createGVSTask = loadGVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_geovertexShader
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_geoinputLayout
			)
		);
	});
	auto createGSTask = loadGSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateGeometryShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_geometryShader
			)
		);


	});
	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShader
				)
			);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer) , D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
				)
			);

		//Create buffer for light constant buffer.
		CD3D11_BUFFER_DESC constantBufferDesc2(sizeof(LightBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc2,
				nullptr,
				&m_constantBufferLights
			)
		);

		CD3D11_BUFFER_DESC constantBufferDesc3(sizeof(PLightPosBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc3,
				nullptr,
				&m_constantBufferLightsPosition
			)
		);

		CD3D11_BUFFER_DESC vpconstantBufferDesc(sizeof(ViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vpconstantBufferDesc,
				nullptr,
				&m_vpconstantBuffer
			)
		);
		CD3D11_BUFFER_DESC modelconstantBufferDesc(sizeof(ModelConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&modelconstantBufferDesc,
				nullptr,
				&m_modelconstantBuffer
			)
		);
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSSimpleTask = loadPSSimpleTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShaderSimple
			)
		);
	});


	auto createVSAniTask = loadVSAniTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShaderAnimation
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINDICIES", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayoutAni
			)
		);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(BoneOffsets), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_boneOffsetsBuffer
			)
		);
	});

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	//Emilio
	//SkyBox VertexShader
	auto createSkyVSTask = loadSkyVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_skyBoxVS
			)
		);
		static const D3D11_INPUT_ELEMENT_DESC skyVertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV",	  0, DXGI_FORMAT_R32G32_FLOAT, 0,  D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",	  0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		};
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				skyVertexDesc,
				ARRAYSIZE(skyVertexDesc),
				&fileData[0],
				fileData.size(),
				&m_skyBoxInputLayout
			)
		);
	});
	//SkyBox PixelShader
	auto createSkyPSTask = loadSkyPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_skyBoxPS
			)
		);

		CD3D11_BUFFER_DESC constantSkyBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantSkyBufferDesc,
				nullptr,
				&m_skyBoxConstBuff
			)
		);
	});

	CD3D11_RASTERIZER_DESC skyboxFrontDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
	skyboxFrontDesc.FrontCounterClockwise = false;
	m_deviceResources->GetD3DDevice()->CreateRasterizerState(&skyboxFrontDesc, m_clockwise.GetAddressOf());

	CD3D11_RASTERIZER_DESC skyboxBackDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
	skyboxBackDesc.FrontCounterClockwise = true;
	m_deviceResources->GetD3DDevice()->CreateRasterizerState(&skyboxBackDesc, m_counterClockwise.GetAddressOf());
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//Creating Sampler
	D3D11_SAMPLER_DESC sampleDesc;
	ZeroMemory(&sampleDesc, sizeof(sampleDesc));
	sampleDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.MinLOD = 0.1f;
	sampleDesc.MaxLOD = 100;
	sampleDesc.MaxAnisotropy = 1;
	sampleDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

	m_deviceResources->GetD3DDevice()->CreateSamplerState(&sampleDesc, &m_sampleState);


	// Once both shaders are loaded, create the mesh.
	auto createCubeTask = (createPSTask && createVSTask).then([this]() {

		// Load mesh vertices. Each vertex has a position and a uv.
		static const VertexPositionColor cubeVertices[] =
		{
			{ XMFLOAT3(-0.5f, 0, -0.5f), XMFLOAT2(0, 1) },
			{ XMFLOAT3(-0.5f, 0,  0.5f), XMFLOAT2(0, 0) },
			{ XMFLOAT3(0.5f, 0, -0.5f), XMFLOAT2(1, 1) },
			{ XMFLOAT3(0.5f, 0,  0.5f), XMFLOAT2(1, 0) },
		};


		HRESULT hr;

		hr = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"brownishDirt_seamless.dds", nullptr, &m_shaderView[0]);
		hr = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"WizardDiffuse.dds", nullptr, &m_shaderView[1]);//Wizard texture
		hr = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"WizardNormal.dds", nullptr, &m_shaderView[2]);//Wizard normal map texture

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBuffer
			)
		);

		// Load mesh indices. Each trio of indices represents
		// a triangle to be rendered on the screen.
		// For example: 0,2,1 means that the vertices with indexes
		// 0, 2 and 1 from the vertex buffer compose the 
		// first triangle of this mesh.
		static const unsigned short cubeIndices[] =
		{
			0,2,1, // -x
			1,2,3,
		};

		m_indexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBuffer
			)
		);
	});
	auto loadBoxTask = (createPSTask && createVSTask).then([this]() {

		this->m_FBXExporter.Initialize();
		this->m_FBXExporter.LoadScene("Mage with Bind and Textures.fbx");

		m_FBXExporter.ProcessSkeletonHierarchy(m_FBXExporter.m_FBXScene->GetRootNode());

		if (m_FBXExporter.m_Skeleton.m_joints.empty())
		{
			m_FBXExporter.m_HasAnimation = false;
		}

		m_FBXExporter.ProcessGeometry(m_FBXExporter.m_FBXScene->GetRootNode());
		m_FBXExporter.Optimize();

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = m_FBXExporter.m_Vertices.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(static_cast<UINT>(m_FBXExporter.m_Vertices.size() * sizeof(RobustVertex)), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBufferBox
			)
		);

		m_indexCountBox = static_cast<unsigned int>(m_FBXExporter.m_Indices.size());

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = m_FBXExporter.m_Indices.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(static_cast<UINT>(m_FBXExporter.m_Indices.size() * sizeof(unsigned short)), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBufferBox
			)
		);

		// point to quad (Antonio)
		static const GeoVertexPositionColor QuadVert[] =
		{
			{ XMFLOAT3(0.0f, -5.0f, 0.0f), XMFLOAT3(1.0f,0.0f,0.0f) }

		};

		D3D11_SUBRESOURCE_DATA vertexBufferData1 = { 0 };
		vertexBufferData.pSysMem = QuadVert;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc1(sizeof(QuadVert), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBufferQuad
			)
		);

		static const unsigned short QuadIndex[] =
		{
			0
		};

		m_indexCountQuad = ARRAYSIZE(QuadIndex);

		D3D11_SUBRESOURCE_DATA indexBufferData1 = { 0 };
		indexBufferData.pSysMem = QuadIndex;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc1(sizeof(QuadIndex), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBufferQuad
			)
		);

	});


	//create the skybox Cube (Emilio)
	auto createSkyBoxTask = (createSkyPSTask && createSkyVSTask).then([this]() {
		char* fileName = "Cube.obj";
		wchar_t* textureName = L"mountains.dds";
		Model CreateSkyBox(fileName, textureName, m_deviceResources->GetD3DDevice(), XMFLOAT3(0.0f, 0.0f, 0.0f));
		CreateSkyBox.WM = XMMatrixIdentity();
		skyBox = CreateSkyBox;
	});

	// Once the cube is loaded, the object is ready to be rendered.
	(loadBoxTask && createSkyBoxTask).then([this] () {
		m_loadingComplete = true;
	});
}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_constantBufferLights.Reset();
	m_constantBufferLightsPosition.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
	for (int i = 0; i < 3; ++i)
	{
		if (m_shaderView[i] != NULL)
			m_shaderView[i]->Release();
	}
	if (m_sampleState != NULL)
		m_sampleState->Release();
}