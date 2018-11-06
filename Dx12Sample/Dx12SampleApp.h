#pragma once

#include "stdafx.h"

using Microsoft::WRL::ComPtr;
using namespace Forms;

namespace Dx12Sample
{
	class Dx12SampleApp
	{
	public:
		Dx12SampleApp(Form *form);

		void Init();
	private:
		static const UINT FrameCount = 2;
		
		CD3DX12_VIEWPORT _viewport;
		CD3DX12_RECT _scissorRect;
		ComPtr<IDXGISwapChain3> _swapChain;
		ComPtr<ID3D12Device3> _device;
		std::array<ComPtr<ID3D12Resource>, FrameCount> _renderTargets;
		ComPtr<ID3D12CommandAllocator> _commandAllocator;
		ComPtr<ID3D12CommandQueue> _commandQueue;
		ComPtr<ID3D12RootSignature> _rootSignature;
		ComPtr<ID3D12DescriptorHeap> _rtvHeap;
		ComPtr<ID3D12PipelineState> _pipelineState;
		ComPtr<ID3D12GraphicsCommandList2> _commandList;
		UINT _rtvDescriptorSize;

		ComPtr<ID3D12Resource> _vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;

		UINT _frameIndex;
		HANDLE _fenceEvent;
		ComPtr<ID3D12Fence1> _fence;
		UINT64 _fenceValue;

		Form *_form;

		void LoadPipeline();
		void LoadAssets();
		void PopulateCommandList();
		void WaitForPrevFrame();
		void GetHardwareAdapter(IDXGIFactory4 *pFactory, IDXGIAdapter1 **ppAdapter);
	};
}
