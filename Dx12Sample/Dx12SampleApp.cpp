#include "stdafx.h"
#include "Dx12SampleApp.h"

using namespace Forms;

Dx12Sample::Dx12SampleApp::Dx12SampleApp(Form *form) :
	_frameIndex(0),
	_viewport(0.f, 0.f, form->GetClientWidth(), form->GetClientHeight()), 
	_scissorRect(0, 0, form->GetClientWidth(), form->GetClientHeight()), 
	_form(form)
{
	
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
		if (D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))
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
	swapChainDesc.Width = 2;
}

void Dx12Sample::Dx12SampleApp::LoadAssets()
{
}

void Dx12Sample::Dx12SampleApp::PopulateCommandList()
{
}

void Dx12Sample::Dx12SampleApp::WaitForPrevFrame()
{
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
