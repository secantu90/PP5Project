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
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
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
	static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));

	XMFLOAT4X4 identity;
	XMStoreFloat4x4(&identity, XMMatrixIdentity());
	m_constantBufferData.world = identity;

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

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	//For specular lighting camera tracking
	//m_constantBufferLightData.cX = cameraX;
	//m_constantBufferLightData.cX = cameraY;
	//m_constantBufferLightData.cX = cameraZ;
	//m_constantBufferLightData.cX = cameraW;

	int timeTemp = static_cast<int>(timer.GetTotalSeconds());
	if (timeTemp - time < 5)
	{
		m_constantBufferLightPosData.sX += .001f;
		m_constantBufferLightPosData.z -= .001f;
		m_constantBufferLightData.z = -1;
		m_constantBufferLightData.sZ = -.75f;
	}
	else
	{
		m_constantBufferLightPosData.sX -= .001f;
		m_constantBufferLightPosData.z += .001f;
		m_constantBufferLightData.z = 1;
		m_constantBufferLightData.sZ = .75f;
		if (timeTemp - time > 9)
			time = timeTemp;
	}


	XMFLOAT4X4 temp;
	XMStoreFloat4x4(&temp, XMMatrixMultiply(XMMatrixScaling(5, 5, 5), XMMatrixIdentity()));
	m_constantBufferData.model = temp;
}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
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

	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
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

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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

	context->PSSetShaderResources(0, 1, &m_shaderView);


	// Draw the objects.
	context->DrawIndexed(
		m_indexCount,
		0,
		0
		);
}

void Sample3DSceneRenderer::CreateDeviceDependentResources()
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

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

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc [] =
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
	});

	
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

	HRESULT hr;

	hr = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"brownishDirt_seamless.dds", nullptr, &m_shaderView);

	// Once both shaders are loaded, create the mesh.
	auto createCubeTask = (createPSTask && createVSTask).then([this] () {

		// Load mesh vertices. Each vertex has a position and a uv.
		static const VertexPositionColor cubeVertices[] = 
		{
			{XMFLOAT3(-0.5f, 0, -0.5f), XMFLOAT2(0, 1)},
			{XMFLOAT3(-0.5f, 0,  0.5f), XMFLOAT2(0, 0)},
			{XMFLOAT3( 0.5f, 0, -0.5f), XMFLOAT2(1, 1)},
			{XMFLOAT3( 0.5f, 0,  0.5f), XMFLOAT2(1, 0)},
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
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
		static const unsigned short cubeIndices [] =
		{
			0,2,1, // -x
			1,2,3,
		};

		m_indexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = {0};
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

	

	// Once the cube is loaded, the object is ready to be rendered.
	createCubeTask.then([this] () {
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
	if (m_shaderView != NULL)
		m_shaderView->Release();
	if(m_sampleState != NULL)
		m_sampleState->Release();
}