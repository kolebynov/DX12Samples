#include "stdafx.h"
#include "Dx12SampleApp.h"
#include "Vertex.h"

using namespace Forms;

Dx12Sample::Dx12SampleApp::Dx12SampleApp(Form *form) :
	_frameIndex(0),
	_viewport(0.f, 0.f, form->GetClientWidth(), form->GetClientHeight()), 
	_scissorRect(0, 0, form->GetClientWidth(), form->GetClientHeight()), 
	_form(form),
	_aspectRatio(form->GetClientWidth() / static_cast<float>(form->GetClientHeight()))
{
	_form->OnPaint([this] { Render(); });
}

void Dx12Sample::Dx12SampleApp::Init()
{
	LoadPipeline();
	LoadAssets();
}

void Dx12Sample::Dx12SampleApp::LoadPipeline()
{
	UINT dxgiFactoryFlags = 0;

#ifdef _DEBUG
	{
		ComPtr<ID3D12Debug3> debugController;
		if (D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)) && debugController != nullptr)
		{
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	ComPtr<IDXGIAdapter1> adapter;
	GetHardwareAdapter(factory.Get(), &adapter);
	ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_device)));

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_commandQueue)));
	
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = _form->GetClientWidth();
	swapChainDesc.Height = _form->GetClientHeight();
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	
	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(_commandQueue.Get(), _form->GetHwnd(), &swapChainDesc, nullptr, nullptr, &swapChain));
	ThrowIfFailed(swapChain.As(&_swapChain));

	_frameIndex = _swapChain->GetCurrentBackBufferIndex();

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NumDescriptors = FrameCount;

	ThrowIfFailed(_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&_rtvHeap)));

	_rtvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < FrameCount; ++i)
	{
		ThrowIfFailed(_swapChain->GetBuffer(i, IID_PPV_ARGS(&_renderTargets[i])));
		_device->CreateRenderTargetView(_renderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, _rtvDescriptorSize);
	}

	ThrowIfFailed(_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator)));
}

void Dx12Sample::Dx12SampleApp::LoadAssets()
{
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature, error;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	ThrowIfFailed(_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&_rootSignature)));

	ComPtr<ID3DBlob> vertexShader, pixelShader;

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
	ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
	psoDesc.pRootSignature = _rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	ThrowIfFailed(_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&_pipelineState)));

	ThrowIfFailed(_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator.Get(), _pipelineState.Get(), IID_PPV_ARGS(&_commandList)));
	ThrowIfFailed(_commandList->Close());

	Vertex triangleVertices[] = 
	{
		{ { 0.f, 0.25f * _aspectRatio, 0.f }, { 1.f, 0.f, 0.f, 1.f } },
		{ { 0.25f, -0.25f * _aspectRatio, 0.f }, { 0.f, 1.f, 0.f, 1.f } },
		{ { -0.25f, -0.25f * _aspectRatio, 0.f }, { 0.f, 0.f, 1.f, 1.f } }
	};

	const UINT vertexBufferSize = sizeof(triangleVertices);

	ThrowIfFailed(_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_vertexBuffer)
	));

	uint8_t *vertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&vertexDataBegin)));
	memcpy(vertexDataBegin, triangleVertices, vertexBufferSize);
	_vertexBuffer->Unmap(0, nullptr);

	_vertexBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
	_vertexBufferView.StrideInBytes = sizeof(Vertex);
	_vertexBufferView.SizeInBytes = vertexBufferSize;

	ThrowIfFailed(_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence)));
	_fenceValue = 1;

	_fenceEvent = CreateEvent(nullptr, 0, 0, nullptr);
	if (_fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}

	WaitForPrevFrame();
}

void Dx12Sample::Dx12SampleApp::Render()
{
	PopulateCommandList();

	ID3D12CommandList* commandLists[] = { _commandList.Get() };
	_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	ThrowIfFailed(_swapChain->Present(1, 0));

	WaitForPrevFrame();
}

void Dx12Sample::Dx12SampleApp::Destroy()
{
	WaitForPrevFrame();

	CloseHandle(_fenceEvent);
}

void Dx12Sample::Dx12SampleApp::PopulateCommandList()
{
	ThrowIfFailed(_commandAllocator->Reset());

	ThrowIfFailed(_commandList->Reset(_commandAllocator.Get(), _pipelineState.Get()));

	_commandList->SetGraphicsRootSignature(_rootSignature.Get());
	_commandList->RSSetViewports(1, &_viewport);
	_commandList->RSSetScissorRects(1, &_scissorRect);

	_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_renderTargets[_frameIndex].Get(), 
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_rtvHeap->GetCPUDescriptorHandleForHeapStart(), _frameIndex, _rtvDescriptorSize);
	_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_commandList->IASetVertexBuffers(0, 1, &_vertexBufferView);
	_commandList->DrawInstanced(3, 1, 0, 0);

	_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_renderTargets[_frameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(_commandList->Close());
}

void Dx12Sample::Dx12SampleApp::WaitForPrevFrame()
{
	const UINT64 fenceValue = _fenceValue;
	ThrowIfFailed(_commandQueue->Signal(_fence.Get(), fenceValue));
	_fenceValue++;

	if (_fence->GetCompletedValue() < fenceValue)
	{
		ThrowIfFailed(_fence->SetEventOnCompletion(fenceValue, _fenceEvent));
		WaitForSingleObject(_fenceEvent, INFINITE);
	}

	_frameIndex = _swapChain->GetCurrentBackBufferIndex();
}

void Dx12Sample::Dx12SampleApp::GetHardwareAdapter(IDXGIFactory4 * pFactory, IDXGIAdapter1 ** ppAdapter)
{
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			// If you want a software adapter, pass in "/warp" on the command line.
			continue;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*ppAdapter = adapter.Detach();
}
