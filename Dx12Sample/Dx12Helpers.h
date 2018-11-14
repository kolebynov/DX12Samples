#pragma once

class Dx12Helpers
{
public:
	class Debug
	{
	public:
		static ID3D12Debug * Debug::Get()
		{
			ID3D12Debug3 *debugController;
			ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));

			return debugController;
		}
	};

	class Factory
	{
	public:
		static IDXGIFactory* Create(UINT flags)
		{
			IDXGIFactory4 *factory;
			ThrowIfFailed(CreateDXGIFactory2(flags, IID_PPV_ARGS(&factory)));

			return factory;
		}
	};

	class Adapter
	{
	public:
		static IDXGIAdapter* GetHardwareAdapter(IDXGIFactory1 *factory, D3D_FEATURE_LEVEL featureLevel)
		{
			ComPtr<IDXGIAdapter1> adapter;
			bool adapterFound = false;

			for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
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
				if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), featureLevel, _uuidof(ID3D12Device), nullptr)))
				{
					adapterFound = true;
					break;
				}
			}

			if (adapterFound)
			{
				return adapter.Detach();
			}

			throw std::exception("Hardware adapter not found");
		}
	};

	class Device
	{
	public:
		static ID3D12Device* Create(IDXGIAdapter *adapater, D3D_FEATURE_LEVEL featureLevel)
		{
			ID3D12Device *device;
			ThrowIfFailed(D3D12CreateDevice(adapater, featureLevel, IID_PPV_ARGS(&device)));

			return device;
		}
	};

	class CommandQueue
	{
	public:
		static ID3D12CommandQueue* Create(ID3D12Device *device, D3D12_COMMAND_QUEUE_DESC *queueDesc)
		{
			ID3D12CommandQueue *result;
			ThrowIfFailed(device->CreateCommandQueue(queueDesc, IID_PPV_ARGS(&result)));

			return result;
		}
	};

	class SwapChain
	{
	public:
		static IDXGISwapChain1* CreateForHwnd(IDXGIFactory2 *factory, ID3D12CommandQueue *commandQueue, HWND hwnd, DXGI_SWAP_CHAIN_DESC1 *desc, 
			DXGI_SWAP_CHAIN_FULLSCREEN_DESC *fullscreenDesc, IDXGIOutput *restrictToOutput)
		{
			IDXGISwapChain1 *swapChain;
			ThrowIfFailed(factory->CreateSwapChainForHwnd(commandQueue, hwnd, desc, fullscreenDesc, restrictToOutput, &swapChain));

			return swapChain;
		}
	};

	class DescriptorHeap
	{
	public:
		static ID3D12DescriptorHeap* Create(ID3D12Device *device, D3D12_DESCRIPTOR_HEAP_DESC *desc)
		{
			ID3D12DescriptorHeap *result;
			ThrowIfFailed(device->CreateDescriptorHeap(desc, IID_PPV_ARGS(&result)));

			return result;
		}
	};

	class CommandAllocator
	{
	public:
		static ID3D12CommandAllocator* Create(ID3D12Device *device, D3D12_COMMAND_LIST_TYPE listType)
		{
			ID3D12CommandAllocator *result;
			ThrowIfFailed(device->CreateCommandAllocator(listType, IID_PPV_ARGS(&result)));

			return result;
		}
	};

	class RootSignature
	{
	public:
		static ID3D12RootSignature* Create(ID3D12Device *device, D3D12_ROOT_SIGNATURE_DESC *desc)
		{
			ComPtr<ID3DBlob> signature, error;
			ThrowIfFailed(D3D12SerializeRootSignature(desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));

			ID3D12RootSignature *result;
			ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&result)));

			return result;
		}
	};

private:
	Dx12Helpers()
	{}
};

