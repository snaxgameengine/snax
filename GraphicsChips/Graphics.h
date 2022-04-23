// SnaX Game Engine - https://github.com/snaxgameengine/snax
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2013 - 2022 Frank-Vegar Mortensen <franksvm(at)outlook(dot)com>.
//
// Permission is hereby  granted, free of charge, to any  person obtaining a copy
// of this software and associated  documentation files (the "Software"), to deal
// in the Software  without restriction, including without  limitation the rights
// to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
// copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
// IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
// FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
// AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
// LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once


#include "Exports.h"
#include "M3DEngine/Chip.h"
#include "D3D12Include.h"
#include "PipelineStatePool.h"
#include "RingBuffer.h"
#include "DescriptorHeapManager.h"
#include "GraphicsChips/GraphicsException.h"
#include "StandardRootSignature.h"

namespace DirectX
{
	class ResourceUploadBatch;
}


namespace m3d
{

static const Guid GRAPHICS_GUID = { 0x13278213, 0x8360, 0x4a1b, { 0xa8, 0x5d, 0xc7, 0x8c, 0x42, 0xb3, 0xed, 0x94 } };

class RenderWindow;
class Window;
class RenderWindowManager;
class DebugGeometry;
class RenderState;

class RenderSettings;
class DebugGeometry;
class D3DManager;
class D3D12RenderWindowManager;
class Font;
class TextWriter;


typedef List<SIDXGIOutput> OutputList;
typedef std::pair<SIDXGIAdapter1, OutputList> AdapterAndOutputs;
typedef List<AdapterAndOutputs> AdapterAndOutputsList;



class GRAPHICSCHIPS_API Graphics : public Chip
{
	CHIPDESC_DECL;
public:
	Graphics();
	virtual ~Graphics();

	virtual bool IsRenderWorldGrid() const { return _renderWorldGrid; }
	virtual bool IsRenderWorldSpaceAABB() const { return _renderWorldSpaceAABB; }
	virtual bool IsRenderLocalAABB() const { return _renderLocalAABB; }
	virtual bool IsRenderObjectAxis() const { return _renderObjectAxis; }
	virtual bool IsRenderFPS() const { return _renderFPS; }

	virtual void SetRenderWorldGrid(bool b) { _renderWorldGrid = b; }
	virtual void SetRenderWorldSpaceAABB(bool b) { _renderWorldSpaceAABB = b; }
	virtual void SetRenderLocalAABB(bool b) { _renderLocalAABB = b; }
	virtual void SetRenderObjectAxis(bool b) { _renderObjectAxis = b; }
	virtual void SetRenderFPS(bool b) { _renderFPS = b; }

	void OnNewFrame() override;

	DebugGeometry* dg();
	//RenderState* rs() { return _rs; }


	virtual bool Init() ;
	virtual void ClearState() ;
	virtual RenderWindowManager* GetRenderWindowManager() ;
	virtual RenderWindow* CreateRenderWindow(Window* wnd) ;
	virtual void SetRequestDebugDevice(bool b)  { _requestDebugLayer = b; }
	virtual bool IsRequestDebugDevice() const  { return  _requestDebugLayer; }
	virtual void DumpMessages() ;
	virtual void PostFrame() ;
	virtual void DestroyDevice() ;

	//virtual void PrintDebugText(String txt, XMFLOAT2 pos, XMCOLOR color) override;
	//virtual void PrintDebugText(String txt, UINT row, UINT column, XMCOLOR color) override;

	// Creates the DXGIFactory and enumerate all adapters and outputs.
	virtual HRESULT CreateDXGIObjects();
	// Releases the device and the factory.
	virtual void ReleaseDXGIFactory();
	// Return the DXGI Factory.
	virtual IDXGIFactory4* GetDXGIFactory() const { return _dxgiFactory; }
	// Returns adapters and outputs available.
	virtual const AdapterAndOutputsList& GetAdaptersAndOutputs() const { return _adaptersAndOutputs; }
	// 
	virtual BOOL GetAllowTearing() const { return _allowTearing; }

	// Sets the index of the adapter to use. Will free device if present on another adapter.
	virtual void SetAdapterIndex(uint32 adapterIndex);
	// Returns current adapter index.
	virtual uint32 GetAdapterIndex() const { return _adapterIndex; }
	// Returns current adapeter.
	virtual IDXGIAdapter1* GetAdapter();
	// Returns the device. Creates it if it does not exist. Throws on fail!
	virtual ID3D12Device* GetDevice();
	// Returns true if device currently exist.
	virtual bool HasDevice() const { return _device != nullptr; }
	// Current feature level.
	virtual D3D_FEATURE_LEVEL GetFeatureLevel() const { return _featureLevel; }
	// Returns d3d debug layer if it exist.
	virtual ID3D12Debug* GetDebug() const { return _debug; }
	// Message queue for messages from d3d.
	virtual ID3D12InfoQueue* GetDebugQueue() const { return _debugQueue; }

	virtual D3D12_RESOURCE_BINDING_TIER GetResourceBindingTier() const { return _rbTier; }

	// Called by D3DDebugReporterBlock only.
	virtual void ReportDebugMsgChip(Chip* ch, SIZE_T firstMessage, SIZE_T afterLastMessage);

	// Get default font used for rendering debug text.
	virtual Font* GetDefaultFont() { return _defaultFont; }
	// Get default text writer used for rendering debug text.
	virtual TextWriter* GetTextWriter() { return _tw; }
	// Creates a new font. Free using mmdelete.
//	virtual Font *CreateFont();
	// Create a new text writer. FRee using mmdelete.
//	virtual TextWriter *CreateTextWriter(bool tech2 = false);

	RenderSettings* rs() { return (RenderSettings*)_rs; }

	// The common command queue.
	virtual ID3D12CommandQueue* GetCommandQueue() { return _commandQueue; }
	// Pool of Pipeline State Objects.
	virtual PipelineStatePool* GetPipelineStatePool() { return _statePool; }
	// Gets buffer for frame based upload allocations.
	virtual RingBuffer* GetUploadHeap() { return _uploadHeap; }
	// Get the descriptor heap manager for CBV, SRV and UAV.
	virtual DescriptorHeapManager* GetHeapManager() { return _heapManager; }
	// Get the descriptor heap manager for render targets.
	virtual DescriptorHeapManager* GetRTVHeapManager() { return _rtvHeapManager; }
	// Get the descriptor heap manager for depth stencil views.
	virtual DescriptorHeapManager* GetDSVHeapManager() { return _dsvHeapManager; }
	// Get the descriptor heap manager for samplers.
	virtual DescriptorHeapManager* GetSamplerHeapManager() { return _samplerHeapManager; }
	// Flush all pending GPU commands.
	virtual void Flush();
	// Wait for gpu to catch up with current frame.
	virtual void Sync();

	virtual UINT64 GetCurrentFrameIndex() const { return _commandListBufferHead; }
	virtual UINT64 GetLastCompletedFrameIndex() const;

	ResourceUploadBatch* GetResourceUploadBatch() { return _rub; }



private:
	bool _renderWorldGrid = true;
	bool _renderWorldSpaceAABB = false;
	bool _renderLocalAABB = false;
	bool _renderObjectAxis = false;
	bool _renderFPS = false;
	
	// Render settings.
	RenderState* _rs = nullptr;

	// The Global DXGI Factory
	SIDXGIFactory4 _dxgiFactory;
	// The Adapters and outputs available. These are valid for the entire lifetime of the factory.
	// If adapters are removed/added to the system the factory must be recreated.
	// From what I've tested, it looks like enumerating outputs always returns the same even if
	// a display is added/removed.
	AdapterAndOutputsList _adaptersAndOutputs;
	// The adapter to go with.
	uint32 _adapterIndex;
	// Direct3D device
	SID3D12Device _device;
	// Current feature level.
	D3D_FEATURE_LEVEL _featureLevel;
	// Debug text writer
	Font* _defaultFont;
	TextWriter* _tw;
	// Render window manager keeping track of the render windows.
	D3D12RenderWindowManager* _windowManager;
	// Use d3d-debug layer if available.
	bool _requestDebugLayer;
	// TRUE for tearing support.
	BOOL _allowTearing;
	//
	D3D12_RESOURCE_BINDING_TIER _rbTier;

	PipelineStatePool* _statePool;
	RingBuffer* _uploadHeap;
	DescriptorHeapManager* _heapManager;
	DescriptorHeapManager* _rtvHeapManager;
	DescriptorHeapManager* _dsvHeapManager;
	DescriptorHeapManager* _samplerHeapManager;

	SID3D12CommandQueue _commandQueue;

	SID3D12Debug _debug;
	SID3D12InfoQueue _debugQueue;

	ResourceUploadBatch* _rub;

	struct CommandListBuffer
	{
		// The command allocator for this entry
		SID3D12CommandAllocator commandAllocator;
		// Resources we must keep until the gpu finished using them (our fence value is reached!)
		List<IUnknown*> toBeReleased;
	};
	CommandListBuffer _commandListBuffer[COMMAND_LIST_BUFFER_SIZE];

	UINT64 _commandListBufferTail;
	UINT64 _commandListBufferHead;

	// Synchronization objects.
	UINT _frameIndex;
	// Sync object used while waiting for gpu.
	HANDLE _fenceEvent;
	SID3D12Fence _fence;
	//	UINT64 _fenceValue;
	//	UINT64 _fenceTailValue;

	List<size_t> _debugQueueReportStatus;

	typedef Map<SIZE_T, std::pair<Chip*, SIZE_T>> ReportedChipsMap;
	ReportedChipsMap _reportedChips;

	ReportedChipsMap::iterator _sendDebugMessages(ReportedChipsMap::iterator n);
	HRESULT _sendDebugMessage(SIZE_T index, Chip* ch = nullptr);

	// Creates the D3D Device using the adapter at current index. Throws on fail!
	void _createD3DDevice();
	void _releaseD3DDevice(bool localOnly = false);

	void _signalFence();
	void _checkFence();
	void _prepareFrame();

};


class GRAPHICSCHIPS_API GraphicsUsage
{
public:
	GraphicsUsage();
	virtual ~GraphicsUsage();

	inline Graphics* graphics() const { return _graphics; }
	inline ID3D12Device* device() const { return _graphics->GetDevice(); }
	inline RenderSettings* rs() const { return _graphics->rs(); }
	inline DebugGeometry* dg() const { return _graphics->dg(); }
	inline PipelineStatePool* GetPipelineStatePool() const { return _graphics->GetPipelineStatePool(); }
	inline RingBuffer* GetUploadHeap() const { return _graphics->GetUploadHeap(); }
	inline DescriptorHeapManager* GetHeapManager() const { return _graphics->GetHeapManager(); }
	inline DescriptorHeapManager* GetRTVHeapManager() const { return _graphics->GetRTVHeapManager(); }
	inline DescriptorHeapManager* GetDSVHeapManager() const { return _graphics->GetDSVHeapManager(); }
	inline DescriptorHeapManager* GetSamplerHeapManager() const { return _graphics->GetSamplerHeapManager(); }

private:
	Graphics* _graphics;

};




class GRAPHICSCHIPS_API D3DDebugReporterBlock
{
public:
	D3DDebugReporterBlock(Chip* chip, Graphics* graphics);
	~D3DDebugReporterBlock();

	Chip* chip;
	Graphics* graphics;
	SIZE_T nMsg;
};

#define D3D_DEBUG_REPORTER_BLOCK D3DDebugReporterBlock debug(this, graphics());
#define D3D_DEBUG_REPORTER_BLOCK_EX(chip) D3DDebugReporterBlock debug(chip, graphics());


}