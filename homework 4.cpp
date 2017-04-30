#include "sound.h"


//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------




//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE                           g_hInst = NULL;
HWND                                g_hWnd = NULL;
D3D_DRIVER_TYPE                     g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL                   g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*                       g_pd3dDevice = NULL;
ID3D11DeviceContext*                g_pImmediateContext = NULL;
IDXGISwapChain*                     g_pSwapChain = NULL;
ID3D11RenderTargetView*             g_pRenderTargetView = NULL;
ID3D11Texture2D*                    g_pDepthStencil = NULL;
ID3D11DepthStencilView*             g_pDepthStencilView = NULL;
ID3D11VertexShader*                 g_pVertexShader = NULL;
ID3D11PixelShader*                  g_pPixelShader = NULL;
ID3D11InputLayout*                  g_pVertexLayout = NULL;
ID3D11Buffer*                       g_pVertexBuffer = NULL;
ID3D11Buffer*                       g_pVertexBuffer_sky = NULL;
ID3D11Buffer*                       g_pVertexBuffer_enemy = NULL;
ID3D11Buffer*                       g_pVertexBuffer_ammo = NULL;
ID3D11Buffer*                       g_pVertexBuffer_health = NULL;
ID3D11Buffer*                       g_pVertexBuffer_ammodrop = NULL;

ID3D11Buffer*                       g_pVertexBuffer_ = NULL;
ID3D11Buffer*                       g_pVertexBuffer_3ds = NULL;

billboard							ammodrop[1000];
int									AMMODROPCOUNT = 10;

billboard							enemies[1000];
int									ENEMYCOUNT = 1;

bool								playing_sprinting = false;
float								player_lives = 5.0;
float								player_health = 1.0;
static float						player_gun_movement = 1.3;
int									player_ammo_current = 8;
int									player_ammo_total = 0;
bool								player_gun_loaded = true;
bool								player_active_reloading = false;


int									model_vertex_anz = 0;
int									enemy_vertex_anz = 1;
int									health_vertex_anz = 2;
int									ammodrop_vertex_anz = 3;
int									ammo_vertex_anz = 0;


//states for turning off and on the depth buffer
ID3D11DepthStencilState				*ds_on, *ds_off;
ID3D11BlendState*					g_BlendState;

ID3D11Buffer*                       g_pCBuffer = NULL;
music_								music;
ID3D11ShaderResourceView*           g_pTextureRV = NULL;
ID3D11ShaderResourceView*           g_pTextureEnemy = NULL;
ID3D11ShaderResourceView*           g_pTextureA = NULL;
ID3D11ShaderResourceView*			g_pTextureammodrop = NULL;
ID3D11ShaderResourceView*			g_pTextureBull = NULL;
ID3D11RasterizerState				*rs_CW, *rs_CCW, *rs_NO, *rs_Wire;


int thrillsong;
int ambientsong;
ID3D11SamplerState*                 g_pSamplerLinear = NULL;
XMMATRIX                            g_World;
XMMATRIX                            g_View;
XMMATRIX                            g_Projection;
XMFLOAT4                            g_vMeshColor(0.7f, 0.7f, 0.7f, 1.0f);

camera								cam;
level								level1;
level								bottom;
level								top;
level								rightSide;
level								leftSide;
level								front;
level								back;
vector<billboard*>					smokeray;
XMFLOAT3							rocket_position;

XMFLOAT3							enemy_spawn_1(1.0, -1.0, 5);
XMFLOAT3							enemy_spawn_2(1.0, -1.0, 7);
XMFLOAT3							enemy_spawn_3(1.0, -1.0, 9);
XMFLOAT3							enemy_spawn_4(1.0, -1.0, 15);


#define ROCKETRADIUS				10
//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void Render();


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitWindow(hInstance, nCmdShow)))
		return 0;

	if (FAILED(InitDevice()))
	{
		CleanupDevice();
		return 0;
	}
	srand(time(0));
	// Main message loop
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render();
		}
	}

	CleanupDevice();

	return (int)msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"TutorialWindowClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, 1024, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(L"TutorialWindowClass", L"Direct3D 11 Tutorial 7", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
		NULL);
	if (!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		if (pErrorBlob) pErrorBlob->Release();
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
	music.set_auto_fadein_fadeout(true);
	HRESULT hr = S_OK;
	ambientsong = music.init_music("ambient.mp3");

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return hr;

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_R32_TYPELESS;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencil);
	if (FAILED(hr))
		return hr;


	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
	if (FAILED(hr))
		return hr;



	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);

	// Compile the vertex shader
	ID3DBlob* pVSBlob = NULL;
	hr = CompileShaderFromFile(L"shader.fx", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Set the input layout
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	hr = CompileShaderFromFile(L"shader.fx", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;
	//create skybox vertex buffer



	// Create vertex buffer
	SimpleVertex vertices[] =
	{
		{ XMFLOAT3(1, -1, 0), XMFLOAT2(1, 0), XMFLOAT3(0, 0, 1) },
		{ XMFLOAT3(1, 1, 0), XMFLOAT2(1, 1), XMFLOAT3(0, 0, 1) },
		{ XMFLOAT3(-1, -1, 0), XMFLOAT2(0, 0), XMFLOAT3(0, 0, 1) },

		{ XMFLOAT3(-1, -1, 0), XMFLOAT2(0, 0), XMFLOAT3(0, 0, 1) },
		{ XMFLOAT3(1, 1, 0), XMFLOAT2(1, 1), XMFLOAT3(0, 0, 1) },
		{ XMFLOAT3(-1, 1, 0), XMFLOAT2(0, 1), XMFLOAT3(0, 0, 1) },

		{ XMFLOAT3(1, -1, 0), XMFLOAT2(1, 0), XMFLOAT3(0, 0, -1) },
		{ XMFLOAT3(-1, -1, 0), XMFLOAT2(0, 0), XMFLOAT3(0, 0, -1) },
		{ XMFLOAT3(1, 1, 0), XMFLOAT2(1, 1), XMFLOAT3(0, 0, -1) },

		{ XMFLOAT3(1, 1, 0), XMFLOAT2(1, 1), XMFLOAT3(0, 0, -1) },
		{ XMFLOAT3(-1, -1, 0), XMFLOAT2(0, 0), XMFLOAT3(0, 0, -1) },
		{ XMFLOAT3(-1, 1, 0), XMFLOAT2(0, 1), XMFLOAT3(0, 0, -1) }
	};


	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = sizeof(SimpleVertex) * 12;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.StructureByteStride = sizeof(SimpleVertex);
	D3D11_SUBRESOURCE_DATA InitData =
	{
		vertices, 0, 0
	};
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
	if (FAILED(hr))
		return hr;


	//load model 3ds file


	//carrier.3ds
	//hornet.3ds
	//f15.3ds
	Load3DS("M4a1.3ds", g_pd3dDevice, &g_pVertexBuffer_3ds, &model_vertex_anz);
	Load3DS("zombie.3ds", g_pd3dDevice, &g_pVertexBuffer_enemy, &enemy_vertex_anz);
	Load3DS("bullet.3ds", g_pd3dDevice, &g_pVertexBuffer_ammo, &ammo_vertex_anz);
	Load3DS("box.3ds", g_pd3dDevice, &g_pVertexBuffer_health, &health_vertex_anz);
	Load3DS("Supplies.3ds", g_pd3dDevice, &g_pVertexBuffer_ammodrop, &ammodrop_vertex_anz);

	// Set vertex buffer
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);


	// Set primitive topology
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffers
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = g_pd3dDevice->CreateBuffer(&bd, NULL, &g_pCBuffer);
	if (FAILED(hr))
		return hr;


	// Load the Texture
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"m4_tex.jpg", NULL, NULL, &g_pTextureRV, NULL);
	if (FAILED(hr))
		return hr;
	// Load the Texture
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"zombie.jpg", NULL, NULL, &g_pTextureEnemy, NULL);
	if (FAILED(hr))
		return hr;


	// Load the Texture
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"boxtext.jpg", NULL, NULL, &g_pTextureA, NULL);
	if (FAILED(hr))
		return hr;

	// Load the Texture
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"Crate05.jpg", NULL, NULL, &g_pTextureammodrop, NULL);
	if (FAILED(hr))
		return hr;

	// Load the Texture
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"smoke.dds", NULL, NULL, &g_pTextureBull, NULL);
	if (FAILED(hr))
		return hr;


	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLinear);
	if (FAILED(hr))
		return hr;

	// Initialize the world matrices
	g_World = XMMatrixIdentity();

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);//camera position
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);//look at
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);// normal vector on at vector (always up)
	g_View = XMMatrixLookAtLH(Eye, At, Up);

	// Initialize the projection matrix
	g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (FLOAT)height, 0.01f, 1000.0f);

	ConstantBuffer constantbuffer;
	constantbuffer.View = XMMatrixTranspose(g_View);
	constantbuffer.Projection = XMMatrixTranspose(g_Projection);
	constantbuffer.World = XMMatrixTranspose(XMMatrixIdentity());
	constantbuffer.info = XMFLOAT4(1, 1, 1, 1);
	g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer, 0, 0);

	//blendstate:
	D3D11_BLEND_DESC blendStateDesc;
	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
	blendStateDesc.AlphaToCoverageEnable = FALSE;
	blendStateDesc.IndependentBlendEnable = FALSE;
	blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;
	g_pd3dDevice->CreateBlendState(&blendStateDesc, &g_BlendState);


	float blendFactor[] = { 0, 0, 0, 0 };
	UINT sampleMask = 0xffffffff;
	g_pImmediateContext->OMSetBlendState(g_BlendState, blendFactor, sampleMask);


	//create the depth stencil states for turning the depth buffer on and of:
	D3D11_DEPTH_STENCIL_DESC		DS_ON, DS_OFF;
	DS_ON.DepthEnable = true;
	DS_ON.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DS_ON.DepthFunc = D3D11_COMPARISON_LESS;
	// Stencil test parameters
	DS_ON.StencilEnable = true;
	DS_ON.StencilReadMask = 0xFF;
	DS_ON.StencilWriteMask = 0xFF;
	// Stencil operations if pixel is front-facing
	DS_ON.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DS_ON.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	DS_ON.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DS_ON.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Stencil operations if pixel is back-facing
	DS_ON.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DS_ON.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	DS_ON.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DS_ON.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Create depth stencil state
	DS_OFF = DS_ON;
	DS_OFF.DepthEnable = false;
	g_pd3dDevice->CreateDepthStencilState(&DS_ON, &ds_on);
	g_pd3dDevice->CreateDepthStencilState(&DS_OFF, &ds_off);

	level1.init("level1.bmp");
	level1.init_texture(g_pd3dDevice, L"wall1.jpg");
	level1.init_texture(g_pd3dDevice, L"wall2.jpg");
	level1.init_texture(g_pd3dDevice, L"floor.jpg");
	level1.init_texture(g_pd3dDevice, L"ceiling.jpg");

	bottom.init("Bottom.bmp");
	bottom.init_texture(g_pd3dDevice, L"wall1.jpg");
	bottom.init_texture(g_pd3dDevice, L"wall2.jpg");
	bottom.init_texture(g_pd3dDevice, L"floor.jpg");
	bottom.init_texture(g_pd3dDevice, L"ceiling.jpg");

	top.init("Top.bmp");
	top.init_texture(g_pd3dDevice, L"wall1.jpg");
	top.init_texture(g_pd3dDevice, L"wall2.jpg");
	top.init_texture(g_pd3dDevice, L"floor.jpg");
	top.init_texture(g_pd3dDevice, L"ceiling.jpg");


	rightSide.init("Right.bmp");
	rightSide.init_texture(g_pd3dDevice, L"wall1.jpg");
	rightSide.init_texture(g_pd3dDevice, L"wall2.jpg");
	rightSide.init_texture(g_pd3dDevice, L"floor.jpg");
	rightSide.init_texture(g_pd3dDevice, L"ceiling.jpg");

	leftSide.init("Left.bmp");
	leftSide.init_texture(g_pd3dDevice, L"wall1.jpg");
	leftSide.init_texture(g_pd3dDevice, L"wall2.jpg");
	leftSide.init_texture(g_pd3dDevice, L"floor.jpg");
	leftSide.init_texture(g_pd3dDevice, L"ceiling.jpg");

	front.init("Front.bmp");
	front.init_texture(g_pd3dDevice, L"wall1.jpg");
	front.init_texture(g_pd3dDevice, L"wall2.jpg");
	front.init_texture(g_pd3dDevice, L"floor.jpg");
	front.init_texture(g_pd3dDevice, L"ceiling.jpg");

	back.init("Back.bmp");
	back.init_texture(g_pd3dDevice, L"wall1.jpg");
	back.init_texture(g_pd3dDevice, L"wall2.jpg");
	back.init_texture(g_pd3dDevice, L"floor.jpg");
	back.init_texture(g_pd3dDevice, L"ceiling.jpg");

	rocket_position = XMFLOAT3(0, 0, ROCKETRADIUS);


	//setting the rasterizer:
	D3D11_RASTERIZER_DESC			RS_CW, RS_Wire;

	RS_CW.AntialiasedLineEnable = FALSE;
	RS_CW.CullMode = D3D11_CULL_BACK;
	RS_CW.DepthBias = 0;
	RS_CW.DepthBiasClamp = 0.0f;
	RS_CW.DepthClipEnable = true;
	RS_CW.FillMode = D3D11_FILL_SOLID;
	RS_CW.FrontCounterClockwise = false;
	RS_CW.MultisampleEnable = FALSE;
	RS_CW.ScissorEnable = false;
	RS_CW.SlopeScaledDepthBias = 0.0f;

	RS_Wire = RS_CW;
	RS_Wire.CullMode = D3D11_CULL_NONE;
	RS_Wire.FillMode = D3D11_FILL_WIREFRAME;
	g_pd3dDevice->CreateRasterizerState(&RS_Wire, &rs_Wire);
	g_pd3dDevice->CreateRasterizerState(&RS_CW, &rs_CW);
	
	
	for (int num = 0; num < ENEMYCOUNT; num++) {
		enemies[num].setPosition(-1, -1, 5);
	}
	music.play(ambientsong);
	return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
	if (g_pImmediateContext) g_pImmediateContext->ClearState();

	if (g_pSamplerLinear) g_pSamplerLinear->Release();
	if (g_pTextureRV) g_pTextureRV->Release();
	if (g_pTextureEnemy) g_pTextureEnemy->Release();
	if (g_pTextureBull) g_pTextureBull->Release();
	if (g_pTextureA) g_pTextureA->Release();
	if (g_pTextureammodrop) g_pTextureammodrop->Release();
	if (g_pCBuffer) g_pCBuffer->Release();
	if (g_pVertexBuffer) g_pVertexBuffer->Release();
	if (g_pVertexLayout) g_pVertexLayout->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pPixelShader) g_pPixelShader->Release();
	if (g_pDepthStencil) g_pDepthStencil->Release();
	if (g_pDepthStencilView) g_pDepthStencilView->Release();
	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();
}
///////////////////////////////////
//		This Function is called every time the Left Mouse Button is down
///////////////////////////////////
bullet *bull = NULL;
void OnLBD(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	if (player_ammo_current > 0) {
		//Shhot bullet in here.
		music.play_fx("gunshot.mp3");
		bull = new bullet;
		bull->pos.x = -cam.position.x + 1.4;
		bull->pos.y = -cam.position.y;
		bull->pos.z = -cam.position.z;
		XMMATRIX CR = XMMatrixRotationY(-cam.rotation.y);
		XMMATRIX CR2 = XMMatrixRotationX(-cam.rotation.x);

		XMFLOAT3 forward = XMFLOAT3(0, 0, 3);
		XMVECTOR f = XMLoadFloat3(&forward);
		f = XMVector3TransformCoord(f, CR2);
		f = XMVector3TransformCoord(f, CR);

		XMStoreFloat3(&forward, f);

		bull->imp = forward;
		player_ammo_current--;
	}


}

///////////////////////////////////
//		This Function is called every time the Right Mouse Button is down
///////////////////////////////////
void OnRBD(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{

}
///////////////////////////////////
//		This Function is called every time a character key is pressed
///////////////////////////////////
void OnChar(HWND hwnd, UINT ch, int cRepeat)
{

}
///////////////////////////////////
//		This Function is called every time the Left Mouse Button is up
///////////////////////////////////
void OnLBU(HWND hwnd, int x, int y, UINT keyFlags)
{


}
///////////////////////////////////
//		This Function is called every time the Right Mouse Button is up
///////////////////////////////////
void OnRBU(HWND hwnd, int x, int y, UINT keyFlags)
{


}
///////////////////////////////////
//		This Function is called every time the Mouse Moves
///////////////////////////////////
void OnMM(HWND hwnd, int x, int y, UINT keyFlags)
{
	static int holdx = x, holdy = y;
	static int reset_cursor = 0;



	RECT rc; 			//rectange structure
	GetWindowRect(hwnd, &rc); 	//retrieves the window size
	int border = 20;
	rc.bottom -= border;
	rc.right -= border;
	rc.left += border;
	rc.top += border;
	ClipCursor(&rc);

	if ((keyFlags & MK_LBUTTON) == MK_LBUTTON)
	{
	}

	if ((keyFlags & MK_RBUTTON) == MK_RBUTTON)
	{
	}
	if (reset_cursor == 1)
	{
		reset_cursor = 0;
		holdx = x;
		holdy = y;
		return;
	}


	int diffx = holdx - x;
	int diffy = holdy - y;
	
	
	float angle_y = (float)diffx / 300.0;
	float angle_x = (float)diffy / 300.0;

	if (cam.w == 1 || cam.a == 1 || cam.s == 1 || cam.d == 1) {
		angle_x = -angle_x;
		angle_y = -angle_y;
	}


	cam.rotation.y += angle_y;
	cam.rotation.x += angle_x;
	int midx = (rc.left + rc.right) / 2;
	int midy = (rc.top + rc.bottom) / 2;
	SetCursorPos(midx, midy);
	reset_cursor = 1;
}

BOOL OnCreate(HWND hwnd, CREATESTRUCT FAR* lpCreateStruct)
{
	RECT rc; 			//rectange structure
	GetWindowRect(hwnd, &rc); 	//retrieves the window size
	int border = 5;
	rc.bottom -= border;
	rc.right -= border;
	rc.left += border;
	rc.top += border;
	ClipCursor(&rc);
	int midx = (rc.left + rc.right) / 2;
	int midy = (rc.top + rc.bottom) / 2;
	SetCursorPos(midx, midy);

	cam.position.z = -1.5;
	return TRUE;
}
void OnTimer(HWND hwnd, UINT id)
{
}
//*************************************************************************
void OnKeyUp(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	switch (vk)
	{
	case 81://q
		cam.q = 0; break;
	case 69://e
		cam.e = 0; break;
	case 65:cam.a = 0;//a
		break;
	case 68: cam.d = 0;//d
		break;
	case 16: //shift in
		cam.sprinting = 0;
		break;
	case 32: //space
		break;
	case 87: cam.w = 0; //w
		break;
	case 83:cam.s = 0; //s
	default:break;

	}

}

void OnKeyDown(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{

	switch (vk)
	{
	default:break;
	case 81://q
		cam.q = 1; break;
	case 69://e
		cam.e = 1; break;
	case 65:cam.a = 1;//a
		break;
	case 68: cam.d = 1;//d
		break;
	case 16: //shift in
		cam.sprinting = 1;
		break;
	case 32: //space
		break;
	case 82: //R - Reload
		player_gun_loaded = false;

		while (!player_gun_loaded) {
			int needed = 8 - player_ammo_current;
			if (needed <= 0) {
				player_gun_loaded = true;
				break;
			}
			if (player_ammo_total > 0) {
				music.play_fx("reload.mp3");
				player_ammo_total--;
				player_ammo_current++;
			}
			else {
				player_gun_loaded = true;
			}

		}
		break;
	case 87: cam.w = 1; //w
		break;
	case 83:cam.s = 1; //s
		break;
	case 27: PostQuitMessage(0);//escape
		break;

	case 84://t
	{
		static int laststate = 0;
		if (laststate == 0)
		{
			g_pImmediateContext->RSSetState(rs_Wire);
			laststate = 1;
		}
		else
		{
			g_pImmediateContext->RSSetState(rs_CW);
			laststate = 0;
		}

	}
	break;

	}
}

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
#include <windowsx.h>
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
		HANDLE_MSG(hWnd, WM_LBUTTONDOWN, OnLBD);
		HANDLE_MSG(hWnd, WM_LBUTTONUP, OnLBU);
		HANDLE_MSG(hWnd, WM_MOUSEMOVE, OnMM);
		HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hWnd, WM_TIMER, OnTimer);
		HANDLE_MSG(hWnd, WM_KEYDOWN, OnKeyDown);
		HANDLE_MSG(hWnd, WM_KEYUP, OnKeyUp);
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}



//--------------------------------------------------------------------------------------
// sprites
//--------------------------------------------------------------------------------------
class sprites
{
public:
	XMFLOAT3 position;
	XMFLOAT3 impulse;
	float rotation_x;
	float rotation_y;
	float rotation_z;
	sprites()
	{
		impulse = position = XMFLOAT3(0, 0, 0);
		rotation_x = rotation_y = rotation_z;
	}
	XMMATRIX animation()
	{
		//update position:
		position.x = position.x + impulse.x; //newtons law
		position.y = position.y + impulse.y; //newtons law
		position.z = position.z + impulse.z; //newtons law

		XMMATRIX M;
		//make matrix M:
		XMMATRIX R, Rx, Ry, Rz, T;
		T = XMMatrixTranslation(position.x, position.y, position.z);
		Rx = XMMatrixRotationX(rotation_x);
		Ry = XMMatrixRotationX(rotation_y);
		Rz = XMMatrixRotationX(rotation_z);
		R = Rx*Ry*Rz;
		M = R*T;
		return M;
	}
};
sprites mario;

//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------

void animate_rocket(float elapsed_microseconds)
{

	//going to a new position
	static float rocket_angle = 0.0;
	rocket_angle += elapsed_microseconds / 1000000.0;

	rocket_position.x = sin(rocket_angle) * ROCKETRADIUS;
	rocket_position.z = cos(rocket_angle) * ROCKETRADIUS;
	rocket_position.y = 0;


	//update all billboards (growing and transparency)
	for (int ii = 0; ii < smokeray.size(); ii++)
	{
		smokeray[ii]->transparency -= 0.0002;
		smokeray[ii]->scale += 0.0003;
		if (smokeray[ii]->transparency < 0) // means its dead
		{
			smokeray.erase(smokeray.begin() + ii);
			ii--;
		}
	}

	//apply a new billboard
	static float time = 0;
	time += elapsed_microseconds;
	if (time < 120000)//every 10 milliseconds
		return;
	time = 0;
	billboard *new_bill = new billboard;
	new_bill->position = rocket_position;
	new_bill->scale = 1. + (float)(rand() % 100) / 300.;
	smokeray.push_back(new_bill);

}






void ShowAmmo(float x, float y) {
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	XMMATRIX view = cam.get_matrix(&g_View);

	// Update skybox constant buffer
	ConstantBuffer constantbuffer;
	constantbuffer.View = XMMatrixTranspose(view);
	constantbuffer.Projection = XMMatrixTranspose(g_Projection);
	constantbuffer.CameraPos = XMFLOAT4(cam.position.x, cam.position.y, cam.position.z, 1);
	//render model:
	float scale = 0.001;
	XMMATRIX S = XMMatrixScaling(scale, scale, scale);


	//S = XMMatrixScaling(10, 10, 10);
	XMMATRIX T, R, R2, M, T_off;
	T = XMMatrixTranslation(-cam.position.x, -cam.position.y, -cam.position.z);
	T_off = XMMatrixTranslation(x, y, 1.3);		//OFFSET FROM THE CENTER
	R = XMMatrixRotationY(-XM_PIDIV2);
	R2 = XMMatrixRotationX(-XM_PIDIV2);
	XMMATRIX Rx = XMMatrixRotationX(-cam.rotation.x);
	XMMATRIX Ry = XMMatrixRotationY(-cam.rotation.y);
	XMMATRIX R_gun = Rx*Ry;
	M = S*R*R2*T_off*R_gun*T;


	constantbuffer.World = XMMatrixTranspose(M);
	g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer, 0, 0);


	// Render terrain
	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
	g_pImmediateContext->VSSetShaderResources(0, 1, &g_pTextureRV);
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_ammo, &stride, &offset);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	g_pImmediateContext->VSSetSamplers(0, 1, &g_pSamplerLinear);

	g_pImmediateContext->OMSetDepthStencilState(ds_on, 1);
	g_pImmediateContext->Draw(ammo_vertex_anz, 0);
}


billboard DropAmmo(float x, float y, float z, billboard ammodrop) {
	ammodrop.setPosition(x, y, z);

	ammodrop.ammodropanimation(-cam.position.x, -cam.position.y, -cam.position.z, ammodrop.position.x, ammodrop.position.y, ammodrop.position.z);
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	if (ammodrop.refill) {
		player_ammo_total += 8;
		ammodrop.used = true;
	}

	XMMATRIX view = cam.get_matrix(&g_View);

	ConstantBuffer constantbuffer2;
	constantbuffer2.View = XMMatrixTranspose(view);
	constantbuffer2.Projection = XMMatrixTranspose(g_Projection);
	constantbuffer2.CameraPos = XMFLOAT4(cam.position.x, cam.position.y, cam.position.z, 1);

	//animate_rocket(elapsed);ssssssssss
	//worldmatrix = enemy.get_matrix_y(view);
	XMMATRIX S2 = XMMatrixScaling(0.01, 0.01, 0.01);
	XMMATRIX R2 = XMMatrixRotationX(-XM_PIDIV2);


	//S = XMMatrixScaling(10, 10, 10);


	XMMATRIX T2 = XMMatrixTranslation(ammodrop.position.x, ammodrop.position.y, ammodrop.position.z);
	XMMATRIX M2 = S2*R2*T2;

	constantbuffer2.World = XMMatrixTranspose(M2);
	g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer2, 0, 0);

	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureammodrop);
	g_pImmediateContext->VSSetShaderResources(0, 1, &g_pTextureammodrop);
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_ammodrop, &stride, &offset);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	g_pImmediateContext->VSSetSamplers(0, 1, &g_pSamplerLinear);
	g_pImmediateContext->Draw(enemy_vertex_anz, 0);
	g_pImmediateContext->OMSetDepthStencilState(ds_off, 1);

	return ammodrop;
}


void playerHealth(float x, float y, float life) {
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	XMMATRIX view = cam.get_matrix(&g_View);

	// Update skybox constant buffer
	ConstantBuffer constantbuffer;
	constantbuffer.View = XMMatrixTranspose(view);
	constantbuffer.Projection = XMMatrixTranspose(g_Projection);
	constantbuffer.CameraPos = XMFLOAT4(cam.position.x, cam.position.y, cam.position.z, 1);
	//render model:
	float scale = player_health / 100;
	XMMATRIX S = XMMatrixScaling(scale, scale / 2, scale / 2);


	//S = XMMatrixScaling(10, 10, 10);
	XMMATRIX T, R, R2, M, T_off;
	T = XMMatrixTranslation(-cam.position.x, -cam.position.y, -cam.position.z);
	T_off = XMMatrixTranslation(x, y, 1.3);		//OFFSET FROM THE CENTER
	R = XMMatrixRotationY(-XM_PIDIV2);
	R2 = XMMatrixRotationX(-XM_PIDIV2);
	XMMATRIX Rx = XMMatrixRotationX(-cam.rotation.x);
	XMMATRIX Ry = XMMatrixRotationY(-cam.rotation.y);
	XMMATRIX R_gun = Rx*Ry;
	S = XMMatrixScaling(1.2, 0.3, 0.1);
	XMMATRIX S2 = XMMatrixScaling(0.01*life, 0.006, 0.01);
	M = S*S2*R*T_off*R_gun*T;


	constantbuffer.World = XMMatrixTranspose(M);
	g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer, 0, 0);


	// Render terrain
	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureA);
	g_pImmediateContext->VSSetShaderResources(0, 1, &g_pTextureA);
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_health, &stride, &offset);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	g_pImmediateContext->VSSetSamplers(0, 1, &g_pSamplerLinear);

	g_pImmediateContext->OMSetDepthStencilState(ds_on, 1);
	g_pImmediateContext->Draw(ammo_vertex_anz, 0);
}

void enemyHealth(XMMATRIX &wm, float x, float y, float life, billboard enemy) //life between 0 and 1
{
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	XMMATRIX view = cam.get_matrix(&g_View);

	// Update skybox constant buffer
	ConstantBuffer constantbuffer;
	constantbuffer.View = XMMatrixTranspose(view);
	constantbuffer.Projection = XMMatrixTranspose(g_Projection);
	//constantbuffer.CameraPos = XMFLOAT4(cam.position.x, cam.position.y, cam.position.z, 1);
	//render model:

	XMMATRIX Ry = XMMatrixRotationY(-cam.rotation.y);

	//iV = XMMatrixTranspose(iV);

	//S = XMMatrixScaling(10, 10, 10);
	XMMATRIX T, R, R2, M, T_off;
	T = XMMatrixTranslation(enemy.position.x, enemy.position.y + 0.5, enemy.position.z);
	T_off = XMMatrixTranslation(0, 1.7, 0);		//OFFSET FROM THE CENTE
	R2 = XMMatrixRotationX(-XM_PIDIV2);
	XMMATRIX S = XMMatrixScaling(1.2, 0.3, 0.1);
	XMMATRIX S2 = XMMatrixScaling(0.01*life, 0.006, 0.01);

	XMMATRIX WM;
	WM = S*S2*Ry  *T_off* wm;



	constantbuffer.World = XMMatrixTranspose(WM);
	g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer, 0, 0);


	// Render terrain
	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureA);
	g_pImmediateContext->VSSetShaderResources(0, 1, &g_pTextureA);
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_health, &stride, &offset);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	g_pImmediateContext->VSSetSamplers(0, 1, &g_pSamplerLinear);

	g_pImmediateContext->OMSetDepthStencilState(ds_on, 1);
	g_pImmediateContext->Draw(ammo_vertex_anz, 0);
}

billboard RenderEnemy(billboard enemy,float elapsed) {
	enemy.enemyanimation(-cam.position.x, -cam.position.y, -cam.position.z, elapsed * 2);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	XMMATRIX view = cam.get_matrix(&g_View);
	ConstantBuffer constantbuffer2;
	constantbuffer2.View = XMMatrixTranspose(view);
	constantbuffer2.Projection = XMMatrixTranspose(g_Projection);
	constantbuffer2.CameraPos = XMFLOAT4(cam.position.x, cam.position.y, cam.position.z, 1);

	//animate_rocket(elapsed);ssssssssss
	XMMATRIX S = XMMatrixScaling(.008, .008, .008);


	//S = XMMatrixScaling(10, 10, 10);

	XMMATRIX R = XMMatrixRotationX(-XM_PIDIV2);
	XMMATRIX RY = XMMatrixRotationY(-XM_PI);
	XMMATRIX Rlook = XMMatrixRotationY(-cam.rotation.y);
	XMMATRIX T = XMMatrixTranslation(enemy.position.x, enemy.position.y, enemy.position.z);
	XMMATRIX M = S*R*RY*Rlook*T;

	constantbuffer2.World = XMMatrixTranspose(M);
	g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer2, 0, 0);

	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureEnemy);
	g_pImmediateContext->VSSetShaderResources(0, 1, &g_pTextureEnemy);
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_enemy, &stride, &offset);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	g_pImmediateContext->VSSetSamplers(0, 1, &g_pSamplerLinear);
	g_pImmediateContext->Draw(enemy_vertex_anz, 0);
	g_pImmediateContext->OMSetDepthStencilState(ds_off, 1);

	enemyHealth(T, 0, 0.5, enemy.life, enemy);
	if (enemy.attacking) {
	enemy.attacking = false;
	music.play_fx("zombie.wav");
	player_health -= 0.1;
	}
	return enemy;
}
//*******************************************************
void renderGun() {
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	XMMATRIX view = cam.get_matrix(&g_View);

	// Update skybox constant buffer
	ConstantBuffer constantbuffer;
	constantbuffer.View = XMMatrixTranspose(view);
	constantbuffer.Projection = XMMatrixTranspose(g_Projection);
	constantbuffer.CameraPos = XMFLOAT4(cam.position.x, cam.position.y, cam.position.z, 1);
	//render model:
	XMMATRIX S = XMMatrixScaling(0.001, 0.001, 0.001);


	S = XMMatrixScaling(0.21, 0.21, 0.21);
	//S = XMMatrixScaling(10, 10, 10);
	XMMATRIX T, R, M, T_off;
	T = XMMatrixTranslation(-cam.position.x, -cam.position.y, -cam.position.z);
	float zmove = 1.3;

	if (cam.w == 1 || cam.a == 1 || cam.s == 1 || cam.d == 1) {
		float mov = player_gun_movement;
		if (cam.sprinting) {
			mov = player_gun_movement * 2;
		}
		zmove = (sin(mov)+3) / 3 + 0.2;
		player_gun_movement += 0.2;
	}

	T_off = XMMatrixTranslation(0.5, -0.5, zmove);		//OFFSET FROM THE CENTER
	R = XMMatrixRotationX(XM_PIDIV2);
	XMMATRIX Rz = XMMatrixRotationZ(-XM_PI);

	XMMATRIX Rx = XMMatrixRotationX(-cam.rotation.x);
	XMMATRIX Ry = XMMatrixRotationY(-cam.rotation.y);
	XMMATRIX Rxx = XMMatrixRotationZ(.1);			//NOT SURE WHAT YOU WANT TO DO HERE
	XMMATRIX R_gun = Rxx*Rx*Ry;
	M = S*R*Rz*T_off*R_gun*T;


	constantbuffer.World = XMMatrixTranspose(M);
	g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer, 0, 0);


	// Render terrain
	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
	g_pImmediateContext->VSSetShaderResources(0, 1, &g_pTextureRV);
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_3ds, &stride, &offset);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	g_pImmediateContext->VSSetSamplers(0, 1, &g_pSamplerLinear);

	g_pImmediateContext->OMSetDepthStencilState(ds_on, 1);
	g_pImmediateContext->Draw(model_vertex_anz, 0);

}

void DisplayHUD() {
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	//Display ammo on hud
	float x = -1.0;
	float y = -0.2;
	for (int i = 0; i < player_ammo_current; i++) {
		if (i == 4) {
			y = -0.4;
			x = -1.0;
		}

		ShowAmmo(x, y);
		x += 0.05;
	}

	playerHealth(-1.0, 0.0, player_health);
}

void renderBullet(float elapsed) {
	if (bull != NULL)
	{
		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;
		XMMATRIX view = cam.get_matrix(&g_View);
		XMMATRIX worldmatrix;
		worldmatrix = XMMatrixIdentity();

		ConstantBuffer constantbuffer;
		worldmatrix = bull->getmatrix(elapsed, view);
		g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureBull);
		constantbuffer.World = XMMatrixTranspose(worldmatrix);
		constantbuffer.View = XMMatrixTranspose(view);
		constantbuffer.Projection = XMMatrixTranspose(g_Projection);
		constantbuffer.info = XMFLOAT4(1, 1, 1, 1);
		g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer, 0, 0);
		g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

		g_pImmediateContext->Draw(12, 0);
	}
}

//*******************************************************

void Render()
{
	static StopWatchMicro_ stopwatch;
	long elapsed = stopwatch.elapse_micro();
	stopwatch.start();//restart

					  //music.play(ambientsong);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);


	// Clear the back buffer
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);

	// Clear the depth buffer to 1.0 (max depth)
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	cam.animation(elapsed, bottom.get_bitmap());
	XMMATRIX view = cam.get_matrix(&g_View);

	XMMATRIX Iview = view;
	Iview._41 = Iview._42 = Iview._43 = 0.0;
	XMVECTOR det;
	Iview = XMMatrixInverse(&det, Iview);

	//ENTER MATRIX CALCULATION HERE
	XMMATRIX worldmatrix;
	worldmatrix = XMMatrixIdentity();

	//XMMATRIX S, T, R, R2;
	//worldmatrix = .... probably define some other matrices here!

	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
	g_pImmediateContext->VSSetShaderResources(0, 1, &g_pTextureRV);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	g_pImmediateContext->VSSetSamplers(0, 1, &g_pSamplerLinear);

	//level1.render_level(g_pImmediateContext, g_pVertexBuffer, &view, &g_Projection, g_pCBuffer);
	bottom.render_level(g_pImmediateContext, g_pVertexBuffer, &view, &g_Projection, g_pCBuffer);
	top.render_level(g_pImmediateContext, g_pVertexBuffer, &view, &g_Projection, g_pCBuffer);
	rightSide.render_level(g_pImmediateContext, g_pVertexBuffer, &view, &g_Projection, g_pCBuffer);
	leftSide.render_level(g_pImmediateContext, g_pVertexBuffer, &view, &g_Projection, g_pCBuffer);
	front.render_level(g_pImmediateContext, g_pVertexBuffer, &view, &g_Projection, g_pCBuffer);
	back.render_level(g_pImmediateContext, g_pVertexBuffer, &view, &g_Projection, g_pCBuffer);

	//////////////// Render AmmoDrops ///////////////
	for (int amm = 0; amm < AMMODROPCOUNT; amm++) {
		if (!ammodrop[amm].used) {
			ammodrop[amm] = DropAmmo(1 + amm, -1, 5, ammodrop[amm]);
		}
	}

	//////////////// Render Enemies ///////////////
	for (int num = 0; num < ENEMYCOUNT; num++) {

		if (!enemies[num].used) {
			enemies[num] = RenderEnemy(enemies[num], elapsed);
		}
	}
	//////////////// Render Player and Info///////////////
	
	//Generate User Gun
	renderGun();
	//Display the User HUD
	DisplayHUD();

	renderBullet(elapsed);

	//Player health and life
	if (player_health <= 0.0) {
		player_lives -= 1;
		player_health = 1.0;

		if (player_lives == 0)
			PostQuitMessage(0);
	}

	//
	// Present our back buffer to our front buffer
	//
	g_pSwapChain->Present(0, 0);
}
