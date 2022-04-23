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

#include "pch.h"
#include "Graphics.h"
#include "DebugGeometry.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ChipManager.h"
#include "RenderState.h"
#include "RenderSettings.h"
#include "DebugGeometry.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ChipManager.h"
#include "D3D12RenderWindow.h"
#include "D3D12RenderWindowManager.h"
#include "TextWriter.h"
#include "Font.h"
#include "DebugFont.h"
#include "M3DEngine/Application.h"
#include "ResourceStateTracker.h"
#include <algorithm>
#include <DirectXTK12/ResourceUploadBatch.h>


using namespace m3d;

m3d::DelayedReleaseComPtrTraitsBase::ResourceList m3d::DelayedReleaseComPtrTraitsBase::resources;

m3d::DelayedReleaseComPtrTraitsBase::ResourceList::~ResourceList()
{
	assert(DelayedReleaseComPtrTraitsBase::resources.r.size() == 0);
}


const Char* D3D12_MESSAGE_CATEGORY_STR[] =
{
	MTEXT("APPLICATION_DEFINED"),
	MTEXT("MISCELLANEOUS"),
	MTEXT("INITIALIZATION"),
	MTEXT("CLEANUP"),
	MTEXT("COMPILATION"),
	MTEXT("STATE_CREATION"),
	MTEXT("STATE_SETTING"),
	MTEXT("STATE_GETTING"),
	MTEXT("RESOURCE_MANIPULATION"),
	MTEXT("EXECUTION"),
	MTEXT("INVALID")
};

const Char* D3D12_MESSAGE_SEVERITY_STR[] =
{
	MTEXT("CORRUPTION"),
	MTEXT("ERROR"),
	MTEXT("WARNING"),
	MTEXT("INFO"),
	MTEXT("INVALID")
};

MessageSeverity D3D12_MESSAGE_SEVERITY_CONV[] = { FATAL, FATAL, WARN, NOTICE, FATAL };




using namespace m3d;


CHIPDESCV1_DEF_HIDDEN(Graphics, MTEXT("Graphics"), GRAPHICS_GUID, CHIP_GUID);


Graphics::Graphics()
{
	_adapterIndex = 0;
	_tw = mmnew TextWriter();
	_defaultFont = mmnew Font();
	_defaultFont->Init(DataBuffer(FONT_ARIAL_10, sizeof(FONT_ARIAL_10), nullptr), sizeof(FONT_ARIAL_10));
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_requestDebugLayer = false;
	_allowTearing = FALSE;
	_rbTier = D3D12_RESOURCE_BINDING_TIER_3;

	_statePool = mmnew PipelineStatePool();
	_uploadHeap = mmnew RingBuffer();
	_heapManager = mmnew DescriptorHeapManager();
	_rtvHeapManager = mmnew DescriptorHeapManager();
	_dsvHeapManager = mmnew DescriptorHeapManager();
	_samplerHeapManager = mmnew DescriptorHeapManager();

	_windowManager = mmnew D3D12RenderWindowManager(_rtvHeapManager);

	_rub = nullptr;

	_commandListBufferTail = 0;
	_commandListBufferHead = 0;
}

Graphics::~Graphics()
{
	_releaseD3DDevice();

	mmdelete(_tw);
	mmdelete(_defaultFont);
	mmdelete(_statePool);
	mmdelete(_uploadHeap);
	mmdelete(_heapManager);
	mmdelete(_rtvHeapManager);
	mmdelete(_dsvHeapManager);
	mmdelete(_samplerHeapManager);
	mmdelete(_windowManager);

	if (_rs)
		_rs->Release();
}

bool Graphics::Init()
{
	_rs = dynamic_cast<RenderState*>(engine->GetChipManager()->CreateChip(RENDERSTATE_GUID));

	HRESULT hr = CreateDXGIObjects();
	return SUCCEEDED(hr);
}

void Graphics::OnNewFrame()
{
	_rs->OnNewFrame();
	if (_renderWorldGrid)
		dg()->AddXZGrid();
	DumpMessages();
}

DebugGeometry* Graphics::dg()
{
	return dynamic_cast<DebugGeometry*>(engine->GetChipManager()->GetGlobalChip(DEBUGGEOMETRY_GUID));
}



void Graphics::ClearState()
{
	//	if (_immediateContext)
	//		_immediateContext->ClearState();
}

RenderWindowManager* Graphics::GetRenderWindowManager() { return _windowManager; }
RenderWindow* Graphics::CreateRenderWindow(Window* wnd) { return GetRenderWindowManager()->CreateRenderWindow(wnd); }


void Graphics::DumpMessages()
{
	if (_debugQueue) { // Has debug queue? 
		if (_debugQueue->GetNumStoredMessages() == _debugQueue->GetMessageCountLimit()) { // Message buffer overflow?
			_debugQueueReportStatus.clear(); // Can't tell which messages has already been reported
			msg(WARN, MTEXT("Direct3D debug layer message overflow."));
		}

		for (SIZE_T i = 0; i < _debugQueue->GetNumStoredMessagesAllowedByRetrievalFilter(); ) {
			if (i < _debugQueueReportStatus.size() && _debugQueueReportStatus[i] != 0)
				i = _debugQueueReportStatus[i];
			else {
				HRESULT hr;
				UCHAR buff[1024];
				SIZE_T l = sizeof(buff);
				hr = _debugQueue->GetMessage(i, (D3D12_MESSAGE*)buff, &l);
				const D3D12_MESSAGE* m = (const D3D12_MESSAGE*)buff;
				String message = String(MTEXT("D3D12: ")) + String(m->pDescription ? m->pDescription : "") + MTEXT(" [ ") + D3D12_MESSAGE_CATEGORY_STR[std::min((UINT)m->Category, (UINT)D3D12_MESSAGE_CATEGORY_EXECUTION + 1)] + MTEXT(" ") + D3D12_MESSAGE_SEVERITY_STR[std::min((UINT)m->Severity, (UINT)D3D12_MESSAGE_SEVERITY_INFO + 1)] + MTEXT(" #") + strUtils::fromNum(m->ID) + MTEXT(" ]");
				msg(D3D12_MESSAGE_SEVERITY_CONV[std::min((UINT)m->Severity, (UINT)D3D12_MESSAGE_SEVERITY_INFO + 1)], message);
				i++;
			}
		}
		_debugQueueReportStatus.clear();
		_debugQueue->ClearStoredMessages();
		/*		if (_debugQueue->GetNumStoredMessages() == _debugQueue->GetMessageCountLimit()) // Message buffer owerflow?
		_reportedChips.clear(); // Clear all reported chips. They are probably not valid anymore because of circular d3d debug buffer.
		ReportDebugMsgChip(nullptr, 0, _debugQueue->GetNumStoredMessagesAllowedByRetrievalFilter()); // Add default message range.
		_sendDebugMessages(_reportedChips.front()); // Send messages!
		_debugQueue->ClearStoredMessages();
		_reportedChips.clear();
		*/
	}
}

static const Guid D3DMESSAGE_CLAZZ = { 0x690dbfcd, 0x64d6, 0x4916,{ 0x81, 0xad, 0xdc, 0xf8, 0x24, 0xcd, 0x48, 0x86 } };

void Graphics::ReportDebugMsgChip(Chip* ch, SIZE_T firstMessage, SIZE_T afterLastMessage)
{
	assert(afterLastMessage <= _debugQueue->GetMessageCountLimit());

	if (_debugQueue->GetNumStoredMessages() == _debugQueue->GetMessageCountLimit()) // Message buffer overflow?
		return; // Can't tell which messages belong to this chip anymore...

	if (_debugQueueReportStatus.size() < _debugQueue->GetMessageCountLimit())
		_debugQueueReportStatus.resize((size_t)_debugQueue->GetMessageCountLimit());

	for (SIZE_T i = firstMessage; i < afterLastMessage;) { // Iterate messages in queue.
		if (_debugQueueReportStatus[i] == 0) { // Message not reported?
			HRESULT hr;
			UCHAR buff[1024];
			SIZE_T l = sizeof(buff);
			hr = _debugQueue->GetMessage(i, (D3D12_MESSAGE*)buff, &l);
			const D3D12_MESSAGE* m = (const D3D12_MESSAGE*)buff;
			ChipMessage cm;
			cm.msg = String(MTEXT("D3D12: ")) + String(m->pDescription ? m->pDescription : "") + MTEXT(" [ ") + D3D12_MESSAGE_CATEGORY_STR[std::min((UINT)m->Category, (UINT)D3D12_MESSAGE_CATEGORY_EXECUTION + 1)] + MTEXT(" ") + D3D12_MESSAGE_SEVERITY_STR[std::min((UINT)m->Severity, (UINT)D3D12_MESSAGE_SEVERITY_INFO + 1)] + MTEXT(" #") + strUtils::fromNum(m->ID) + MTEXT(" ]");
			cm.severity = D3D12_MESSAGE_SEVERITY_CONV[std::min((UINT)m->Severity, (UINT)D3D12_MESSAGE_SEVERITY_INFO + 1)];
			if (ch->GetMessageHitCount(cm) == 0) { // chip does not have this message?
			}
			ch->AddMessage(cm); // Add message (Just increases hit count if it exist)

			i++; // go to next message!
		}
		else {
			i = _debugQueueReportStatus[i]; // Skip range of messages already reported.
		}
	}

	_debugQueueReportStatus[firstMessage] = afterLastMessage; // Mark this range as reported!
}

/*
void D3DGraphics::ReportDebugMsgChip(Chip *ch, SIZE_T firstMessage, SIZE_T afterLastMessage)
{
ReportedChipsMap::Node n = _reportedChips.find(firstMessage);
if (n.valid()) { // Subblock reported same start message?
if (n->second.first == ch) // Does the subblock belong to the same chip?
n->second.second = std::max(n->second.second, afterLastMessage); // Extend range!
else if (n->second.second < afterLastMessage) // Subblock does not contain the whole range?
ReportDebugMsgChip(ch, n->second.second, afterLastMessage); // Start this blocks range from where the subblock's range ended.
//			_reportedChips.insert(std::make_pair(n->second.second, std::make_pair(ch, afterLastMessage)));
else {
assert(n->second.second == afterLastMessage); // Subblocks range should NEVER exeed this blocks range.
}
}
else {
// Set message range for the block. NOTE: The block does not necessarily "own" the whole range if a subblock
// reported start message >firstMessage. Subblock message range will never exeed afterLastMessage!!
n = _reportedChips.insert(std::make_pair(firstMessage, std::make_pair(ch, afterLastMessage))).first;
assert((++n).invalid() || n->second.second <= afterLastMessage);
}
}
*/



HRESULT Graphics::CreateDXGIObjects()
{
	assert(_dxgiFactory == nullptr);

	HRESULT hr;

	UINT flags = _requestDebugLayer ? DXGI_CREATE_FACTORY_DEBUG : 0;

	msg(DINFO, strUtils::ConstructString(MTEXT("Creating DXGI Factory %1 debugging enabled.")).arg(flags & DXGI_CREATE_FACTORY_DEBUG ? MTEXT("with") : MTEXT("without")));

#ifdef WINDESKTOP
	V_RETURN(CreateDXGIFactory2(flags, IID_PPV_ARGS(&_dxgiFactory)));
#else
	V_RETURN(CreateDXGIFactory1(IID_IDXGIFactory2, (void**)&_dxgiFactory));
#endif


	SIDXGIFactory5 factory5;
	if (SUCCEEDED(_dxgiFactory->QueryInterface(IID_PPV_ARGS(&factory5))))
	{
		if (FAILED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &_allowTearing, sizeof(_allowTearing))))
		{
			_allowTearing = FALSE;
		}
	}

	msg(DINFO, String(MTEXT("Enumerating DXGI adapters...")));

	uint32 prefAdapter = 0;
	SIZE_T maxMem = 0;

	// Enumerate adapters.
	for (UINT i = 0;; i++) {
		SIDXGIAdapter1 adapter;

		hr = _dxgiFactory->EnumAdapters1(i, &adapter);

		if (hr == DXGI_ERROR_NOT_FOUND)
			break; // No more found.

		V_RETURN(hr);

		// Can this adapter create a d3d12 device?
		if (FAILED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
			continue;

		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		// Ignore software adapters!
		if (desc.Flags != DXGI_ADAPTER_FLAG_NONE)
			continue;

		// Prefer adapter with the most video memory available.
		if (desc.DedicatedVideoMemory > maxMem) {
			prefAdapter = i;
			maxMem = desc.DedicatedVideoMemory;
		}

		List<SIDXGIOutput> outputs;

		for (UINT j = 0;; j++) {
			SIDXGIOutput output;
			hr = adapter->EnumOutputs(j, &output);
			if (hr == DXGI_ERROR_NOT_FOUND)
				break;

			V_RETURN(hr);

			outputs.push_back(output);
		}

		// DO NOT EXCLUDE ADAPTERS WITHOUT OUTPUTS. THIS IS PROBABLY A WARP ADAPTER...
//		if (outputs.size()) // Adapters without outputs are ignored!
		_adaptersAndOutputs.push_back(std::make_pair(adapter, outputs));
	}

	_adapterIndex = prefAdapter;

	if (_adaptersAndOutputs.size() == 0)
		return E_FAIL; // No adapters found!

//	if (hWnd)
//		V_RETURN(_dxgiFactory->MakeWindowAssociation(hWnd, 0));

	msg(DINFO, strUtils::ConstructString(MTEXT("DXGI initialization done. %1 adapters found. Tearing allowed: %2.")).arg(_adaptersAndOutputs.size()).arg(_allowTearing ? MTEXT("Yes") : MTEXT("No")));

	return S_OK;
}

void Graphics::ReleaseDXGIFactory()
{
	msg(DINFO, String(MTEXT("DXGI objects released")));

	_releaseD3DDevice();
	_adaptersAndOutputs.clear();
	_dxgiFactory = nullptr;
}

void Graphics::SetAdapterIndex(uint32 adapterIndex)
{
	if (adapterIndex >= _adaptersAndOutputs.size())
		adapterIndex = 0;
	if (adapterIndex == _adapterIndex)
		return;

	msg(DINFO, strUtils::ConstructString(MTEXT("DXGI adapter %1 requested")).arg(adapterIndex));

	_adapterIndex = adapterIndex;
	_releaseD3DDevice();
}

IDXGIAdapter1* Graphics::GetAdapter()
{
	return _adaptersAndOutputs[_adapterIndex].first;
}

ID3D12Device* Graphics::GetDevice()
{
	if (!_device) {
		try {
			_createD3DDevice();
		}
		catch (const GraphicsException&) {
			_releaseD3DDevice(true);
			throw;
		}
	}
	return _device;
}

/*
Font *D3DGraphics::CreateFont()
{
	return mmnew Font();
}

TextWriter *D3DGraphics::CreateTextWriter(bool tech2)
{
	return mmnew TextWriter(tech2);
}*/

void Graphics::DestroyDevice()
{
	_releaseD3DDevice();
}

void Graphics::PostFrame()
{
	if (!_device)
		return;

	D3D12RenderWindow* rw = dynamic_cast<D3D12RenderWindow*>(GetRenderWindowManager()->GetRenderWindow());
	if (rw) {

		if (IsRenderFPS())
		{
			PipelineOutputStateDesc oDesc;
			oDesc.NumRenderTargets = 1;
			oDesc.RTVFormats[0] = rw->GetBackBuffer()->GetDesc().Format;
			oDesc.SampleDesc = rw->GetBackBuffer()->GetDesc().SampleDesc;
			oDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
			PipelineStateDescID id = _statePool->RegisterPipelineOutputStateDesc(oDesc);

			D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = rw->GetDescriptor().GetCPUHandle();
			rs()->OMSetRenderTargets(1, &cpuHandle, nullptr);
			rs()->SetPipelineOutputStateDesc(id);

			String txt = strUtils::format(MTEXT("FPS: %i"), uint32(engine->GetFPS().GetFPS() + 0.5));

			_tw->Write(this, XMFLOAT2(10, 10), TextWriter::LEFT, TextWriter::TOP, false, XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), txt);
		}

		// Clear render targets to release lock of back buffer.
		rs()->OMSetRenderTargets(0, nullptr, nullptr);

		SResourceStateTracker rst;
		GetResourceStateTracker(rw->GetBackBuffer(), &rst);
		rs()->EnterState(rst, D3D12_RESOURCE_STATE_PRESENT); // Can throw. Release lock immediately.
	}

	Flush();

	//	if (rw)
	//		rw->Present();
}

void Graphics::Flush()
{
	if (!_device)
		return;

	rs()->CloseAndExecuteCommandList();

	_signalFence();
	_checkFence();
	_prepareFrame();
}

void Graphics::Sync()
{
	if (!_device)
		return;

	HRESULT hr;

	UINT64 fenceValue = _commandListBufferHead - 1;

	V(_fence->SetEventOnCompletion(fenceValue, _fenceEvent));
	WaitForSingleObject(_fenceEvent, INFINITE);

	_checkFence();
}


void Graphics::_createD3DDevice()
{
	HRESULT hr;

	// NOTE: IF THIS FUNCTIONS FAILS, THE APPLICATION WILL CURRENTLY CRASH! A MESSAGE WILL BE AVAILABLE IN THE LOG!

	// Enable the D3D12 debug layer.
	if (_requestDebugLayer) {
		msg(DINFO, String(MTEXT("Requesting Direct3D 12 debug interface...")));
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&_debug)))) {
			msg(DINFO, String(MTEXT("Enabling Direct3D 12 debug layer.")));
			_debug->EnableDebugLayer();
		}
	}

	UINT originalAdapterIndex = _adapterIndex;

	D3D12_FEATURE_DATA_D3D12_OPTIONS featureSupport = { 0 };
	D3D12_FEATURE_DATA_SHADER_MODEL featureShaderModel = { D3D_SHADER_MODEL_5_1 };

	while (true) {
		IDXGIAdapter1* dxgiAdapter = GetAdapter();

		DXGI_ADAPTER_DESC desc;
		dxgiAdapter->GetDesc(&desc);

		msg(INFO, strUtils::ConstructString(MTEXT("Creating Direct3D 12 Device at Adapter %1 (\'%2\')...")).arg(_adapterIndex).arg(strUtils::trim(strUtils::narrow2(desc.Description))));

		// Try to create device...
		hr = D3D12CreateDevice(dxgiAdapter, _featureLevel, IID_PPV_ARGS(&_device));

		if (SUCCEEDED(hr)) {
			_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &featureSupport, sizeof(featureSupport));
			_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &featureShaderModel, sizeof(featureShaderModel));

			/*if (featureSupport.ResourceBindingTier == D3D12_RESOURCE_BINDING_TIER_1) {
				_device = nullptr;
				msg(WARN, MTEXT("Ouch! This adapter does only support Resource Binding Tier 1. SnaX requires Tier 2 or Tier 3 to work. Trying another adapter!"));
				hr = E_FAIL;
			}*/
		}

		if (FAILED(hr)) { // Failed?

			_adapterIndex++;
			if (_adapterIndex == _adaptersAndOutputs.size())
				_adapterIndex = 0;

			if (_adapterIndex == originalAdapterIndex) {
				msg(WARN, MTEXT("Failed to create Direct3D 12 Device. We tried all available adapters. Sorry, but it seems you need to upgrade your hardware to run SnaX!"));
				throw GraphicsException(MTEXT("Failed to create Direct3D 12 Device. We tried all available adapters. Sorry, but it seems you need to upgrade your hardware to run SnaX!"), FATAL);
			}

			msg(WARN, MTEXT("Failed to create Direct3D 12 Device. Trying another adapter!"));
			continue;

			/*			// If the initialization fails, fall back to the WARP device.
						// For more information on WARP, see:
						// http://go.microsoft.com/fwlink/?LinkId=286690

						SIDXGIAdapter3 warpAdapter;
						_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));

						hr = D3D12CreateDevice(warpAdapter, _featureLevel, IID_PPV_ARGS(&_device));

						if (FAILED(hr))
						{
							msg(WARN, MTEXT("Failed to create Direct3D 12 Device! Sorry!"));
							return hr; // Fail! Return!
						}*/
		}

		break; // Nice! Continue! :)
	}

	for (uint32 i = 0; i < COMMAND_LIST_BUFFER_SIZE; i++) {
		if (FAILED(_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandListBuffer[i].commandAllocator))))
			throw GraphicsException(MTEXT("Failed in ID3D12Device::CreateCommandAllocator(...)"), FATAL);
	}

	// Create synchronization objects.
	{
		if (FAILED(_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence))))
			throw GraphicsException(MTEXT("Failed in ID3D12Device::CreateFence(...)"), FATAL);

		_commandListBufferHead = 1;
		_commandListBufferTail = 1;

		// Create an event handle to use for frame synchronization.
		_fenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		if (_fenceEvent == nullptr)
		{
			throw GraphicsException(MTEXT("Failed in CreateEventEx(...)"), FATAL);
			//V_RETURN(HRESULT_FROM_WIN32(GetLastError()));
		}
	}

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	if (FAILED(_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_commandQueue))))
		throw GraphicsException(MTEXT("Failed in ID3D12Device::CreateCommandQueue(...)"), FATAL);

	if (FAILED(rs()->Init(_device, _commandListBuffer[_commandListBufferHead % COMMAND_LIST_BUFFER_SIZE].commandAllocator)))
		throw GraphicsException(MTEXT("Failed in RenderSettings::Init(...)"), FATAL);

	_statePool->SetDevice(_device);

	if (FAILED(_uploadHeap->Init(_device, 1024 * 1024 * 64)))
		throw GraphicsException(MTEXT("Failed in RingBuffer::Init(...)"), FATAL);

	if (FAILED(_heapManager->Init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 16384, TRUE)))
		throw GraphicsException(MTEXT("Failed in DescriptorHeapManager::Init(...)"), FATAL);

	if (FAILED(_rtvHeapManager->Init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1024)))
		throw GraphicsException(MTEXT("Failed in DescriptorHeapManager::Init(...)"), FATAL);

	if (FAILED(_dsvHeapManager->Init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1024)))
		throw GraphicsException(MTEXT("Failed in DescriptorHeapManager::Init(...)"), FATAL);

	if (FAILED(_samplerHeapManager->Init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 1024, TRUE)))
		throw GraphicsException(MTEXT("Failed in DescriptorHeapManager::Init(...)"), FATAL);

	_rub = mmnew ResourceUploadBatch(_device);

	if (_debug) {
		if (SUCCEEDED(_device->QueryInterface(IID_PPV_ARGS(&_debugQueue)))) {
			_debugQueue->SetMuteDebugOutput(false); // We display these in the editor instead!
#if defined( DEBUG ) || defined( _DEBUG )
//				_debugQueue->SetBreakOnSeverity( D3D11_MESSAGE_SEVERITY_CORRUPTION, true );
//				_debugQueue->SetBreakOnSeverity( D3D11_MESSAGE_SEVERITY_ERROR, true );
#endif
		}
	}

	if (FAILED(_tw->Init(this, _defaultFont)))
		throw GraphicsException(MTEXT("Failed in TextWriter::Init(...)"), FATAL);

	_rbTier = featureSupport.ResourceBindingTier;

	String rbt;
	switch (_rbTier)
	{
	case D3D12_RESOURCE_BINDING_TIER_1: rbt = MTEXT("Tier 1"); break;
	case D3D12_RESOURCE_BINDING_TIER_2: rbt = MTEXT("Tier 2"); break;
	case D3D12_RESOURCE_BINDING_TIER_3: rbt = MTEXT("Tier 3"); break;
	default: rbt = MTEXT("Buggy"); break;
	}

	String hsm;
	switch (featureShaderModel.HighestShaderModel)
	{
	case D3D_SHADER_MODEL_5_1: hsm = MTEXT("5.1"); break;
	case D3D_SHADER_MODEL_6_0: hsm = MTEXT("6.0"); break;
	default: hsm = MTEXT("Buggy"); break;
	}

	String fl;
	switch (_featureLevel)
	{
	case D3D_FEATURE_LEVEL_9_1: fl = MTEXT("9.1"); break;
	case D3D_FEATURE_LEVEL_9_2: fl = MTEXT("9.2"); break;
	case D3D_FEATURE_LEVEL_9_3:  fl = MTEXT("9.3"); break;
	case D3D_FEATURE_LEVEL_10_0: fl = MTEXT("10.0"); break;
	case D3D_FEATURE_LEVEL_10_1: fl = MTEXT("10.1"); break;
	case D3D_FEATURE_LEVEL_11_0: fl = MTEXT("11.0"); break;
	case D3D_FEATURE_LEVEL_11_1: fl = MTEXT("11.1"); break;
	case D3D_FEATURE_LEVEL_12_0: fl = MTEXT("12.0"); break;
	case D3D_FEATURE_LEVEL_12_1: fl = MTEXT("12.1"); break;
	default: fl = MTEXT("Buggy"); break;
	}

	msg(INFO, MTEXT("Direct3D 12 Device created at Adapter ") + strUtils::fromNum(_adapterIndex) + MTEXT(" with Feature Level ") + fl + MTEXT("."));
	msg(INFO, MTEXT("The Direct3D 12 Resource Binding Tier is: ") + rbt + MTEXT("."));
	msg(INFO, MTEXT("The highest Shader Model supported is: ") + hsm + MTEXT("."));

	_prepareFrame();
}

void Graphics::_releaseD3DDevice(bool localOnly)
{
	if (!_device)
		return;

	msg(DINFO, String(MTEXT("Releasing Direct3D 12 device...")));

	if (!localOnly) {
		engine->DestroyDeviceObjects();

		msg(DINFO, String(MTEXT("Engine survived. Continuing...")));
	}

	_tw->OnDestroyDevice();
	_defaultFont->OnDestroyDevice();

	if (!localOnly) {
		Flush();
		Sync();
	}

	rs()->OnDestroyDevice();

	for (uint32 i = 0; i < COMMAND_LIST_BUFFER_SIZE; i++) {
		_commandListBuffer[i].commandAllocator = nullptr;
		assert(_commandListBuffer[i].toBeReleased.size() == 0);
	}

	_fence = nullptr;
	if (_fenceEvent)
		CloseHandle(_fenceEvent);
	_fenceEvent = NULL;
	_commandQueue = nullptr;

	_statePool->OnDestroyDevice();
	_uploadHeap->Clear();
	_heapManager->Clear();
	_rtvHeapManager->Clear();
	_dsvHeapManager->Clear();
	_samplerHeapManager->Clear();

	mmdelete(_rub);

	assert(DelayedReleaseComPtrTraitsBase::resources.r.size() == 0);

	_debug = nullptr;
	_debugQueue = nullptr;
	//#if defined( DEBUG ) || defined( _DEBUG )
	_device->AddRef();
	ULONG ref = _device->Release();
	assert(ref == 1);
	if (ref > 1) {
		SID3D12DebugDevice debugDevice;
		SID3D12InfoQueue debugQueue;
		if (SUCCEEDED(_device->QueryInterface(IID_PPV_ARGS(&debugDevice)))) {
			debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
			if (SUCCEEDED(_device->QueryInterface(IID_PPV_ARGS(&_debugQueue))))
				DumpMessages();
		}
		_debugQueue = nullptr;
	}
	//#endif

	_device = nullptr;

	//#if defined( DEBUG ) || defined( _DEBUG )
	msg(INFO, strUtils::ConstructString(MTEXT("Direct3D device released with %1 remaining references.")).arg(uint32(ref - 1)));
	//#else
	//	msg(INFO, String(MTEXT("Direct3D 12 device released.")));
	//#endif
}

void Graphics::_signalFence()
{
	HRESULT hr;

	// Signal the command queue for job done!
	_commandQueue->Signal(_fence, _commandListBufferHead);

	// Set objects released this frame to the release queue. They will be released when the gpu is guarantied to be done with them.
	assert(_commandListBuffer[_commandListBufferHead % COMMAND_LIST_BUFFER_SIZE].toBeReleased.empty());
	std::swap(_commandListBuffer[_commandListBufferHead % COMMAND_LIST_BUFFER_SIZE].toBeReleased, DelayedReleaseComPtrTraitsBase::resources.r);

	// Iterate current execution nr.
	_commandListBufferHead++;

	// Get current GPU progress against submitted workload. Resources still scheduled 
	// for GPU execution cannot be modified or else undefined behavior will result.
	UINT64 lastCompletedFence = _fence->GetCompletedValue();

	// Running out of buffer size?
	if (_commandListBufferHead - lastCompletedFence > COMMAND_LIST_BUFFER_SIZE) {
		// Wait for at least one execution to finish!
		V(_fence->SetEventOnCompletion(lastCompletedFence + 1, _fenceEvent));
		WaitForSingleObject(_fenceEvent, INFINITE);
	}
}

void Graphics::_checkFence()
{
	// Get current GPU progress against submitted workload. Resources still scheduled 
	// for GPU execution cannot be modified or else undefined behavior will result.
	UINT64 lastCompletedFence = _fence->GetCompletedValue();

	assert(lastCompletedFence != UINT64_MAX);

	// Iterate from the back of buffer, releasing resources for completed executions!
	for (; _commandListBufferTail <= lastCompletedFence; _commandListBufferTail++) {
		CommandListBuffer& c = _commandListBuffer[_commandListBufferTail % COMMAND_LIST_BUFFER_SIZE];
		if (c.toBeReleased.size()) {
			std::for_each(c.toBeReleased.begin(), c.toBeReleased.end(), [](IUnknown* e) { e->Release(); });
			c.toBeReleased.clear();
		}
	}

	_uploadHeap->UpdateFrame(lastCompletedFence, _commandListBufferHead);
	_heapManager->UpdateFrame(lastCompletedFence, _commandListBufferHead);
	_rtvHeapManager->UpdateFrame(lastCompletedFence, _commandListBufferHead);
	_dsvHeapManager->UpdateFrame(lastCompletedFence, _commandListBufferHead);
	_samplerHeapManager->UpdateFrame(lastCompletedFence, _commandListBufferHead);
}

void Graphics::_prepareFrame()
{
	HRESULT hr;

	V(_commandListBuffer[_commandListBufferHead % COMMAND_LIST_BUFFER_SIZE].commandAllocator->Reset());

	// However, when ExecuteCommandList() is called on a particular command
	// list, that command list can then be reset at any time and must be before
	// re-recording.
	V(rs()->Reset(_commandListBuffer[_commandListBufferHead % COMMAND_LIST_BUFFER_SIZE].commandAllocator, nullptr));

	ID3D12DescriptorHeap* heaps[2] = { _heapManager->GetHeap(), _samplerHeapManager->GetHeap() };
	rs()->SetDescriptorHeaps(2, heaps);
}

UINT64 Graphics::GetLastCompletedFrameIndex() const
{
	return _fence ? _fence->GetCompletedValue() : 0;
}

HRESULT Graphics::_sendDebugMessage(SIZE_T index, Chip* ch)
{
	HRESULT hr;
	UCHAR buff[1024];
	SIZE_T l = 1024;
	V_RETURN(_debugQueue->GetMessage(index, (D3D12_MESSAGE*)buff, &l));
	const D3D12_MESSAGE* m = (const D3D12_MESSAGE*)buff;
	String message = String(MTEXT("D3D12: ")) + String(m->pDescription ? m->pDescription : "") + MTEXT("[ ") + D3D12_MESSAGE_CATEGORY_STR[std::min((UINT)m->Category, (UINT)D3D12_MESSAGE_CATEGORY_EXECUTION + 1)] + MTEXT(" ") + D3D12_MESSAGE_SEVERITY_STR[std::min((UINT)m->Severity, (UINT)D3D12_MESSAGE_SEVERITY_INFO + 1)] + MTEXT(" #") + strUtils::fromNum(m->ID) + MTEXT(" ]");
	msg(D3D12_MESSAGE_SEVERITY_CONV[std::min((UINT)m->Severity, (UINT)D3D12_MESSAGE_SEVERITY_INFO + 1)], message, ch);
	return S_OK;
}

Graphics::ReportedChipsMap::iterator Graphics::_sendDebugMessages(ReportedChipsMap::iterator n)
{
	ReportedChipsMap::iterator next = n;
	std::advance(next, 1); // The next node
	for (SIZE_T i = n->first; i < n->second.second; ) { // Iterate message range for this node
		if (next != _reportedChips.end() && i == next->first) { // Next node starting at this message?
			next = _sendDebugMessages(next); // Do iteration for next node...
			i = next->second.second; // ...then continue where it ended.
			next++; // Find next node
			assert(i <= next->second.second);
		}
		else
			_sendDebugMessage(i++, n->second.first); // "our" message. Send it!
	}
	return next != _reportedChips.end() ? _sendDebugMessages(next) : n;
}



GraphicsUsage::GraphicsUsage()
{
	_graphics = dynamic_cast<Graphics*>(engine->GetGraphics());
}

GraphicsUsage::~GraphicsUsage()
{
}

D3DDebugReporterBlock::D3DDebugReporterBlock(Chip* chip, Graphics* graphics) : chip(chip), graphics(graphics)
{
	ID3D12InfoQueue* q = graphics->GetDebugQueue();
	nMsg = q ? (size_t)q->GetNumStoredMessagesAllowedByRetrievalFilter() : -1;
}

D3DDebugReporterBlock::~D3DDebugReporterBlock()
{
	if (nMsg != -1) {
		ID3D12InfoQueue* q = graphics->GetDebugQueue();
		if (q->GetNumStoredMessagesAllowedByRetrievalFilter() > nMsg)
			graphics->ReportDebugMsgChip(chip, nMsg, (size_t)q->GetNumStoredMessagesAllowedByRetrievalFilter());
	}
}
