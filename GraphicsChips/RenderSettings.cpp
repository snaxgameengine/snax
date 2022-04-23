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
#include "RenderSettings.h"
#include "M3DEngine/Engine.h"
#include "D3D12RenderWindow.h"
#include "ResourceStateTracker.h"
#include "StandardRootSignature.h"
#include "Utils.h"
#include "GraphicsChips/GraphicsException.h"

using namespace m3d;


CHIPDESCV1_DEF_HIDDEN(RenderSettings, MTEXT("Render Settings"), RENDERSTATE_GUID, CHIP_GUID);


RenderSettings::RenderSettings()
{
	_currentBarrier = _barriers;

	_currentGraphicsRootSignature = 0;
	_currentComputeRootSignature = 0;
}

RenderSettings::~RenderSettings()
{
//	mmdelete(_cl);
}

void RenderSettings::OnDestroyDevice()
{
	_stdElements = 0;
	_stdCB1Resource = nullptr;
	_stdCB2Resource = nullptr;
	//	_shaderResources.clear();
	_cl = nullptr;
}

void RenderSettings::OnReleasingBackBuffer(RenderWindow *rw)
{
/*	for (ShaderResourceMap::Node n = _shaderResources.front(); n.valid(); n++) {
		for (size_t i = 0; i < n->second.size(); i++) {
			SID3D11Resource r;
			n->second[i]->GetResource(&r);
			if (r == ((D3DRenderWindow*)rw)->GetBackBuffer())
				n->second[i] = nullptr;
		}
	}*/
}


void RenderSettings::PrepareDraw()
{
	const PipelineState &ps = GetPipelineState();

	if (ps.standardRootSignature) {
		if (ps.standardRootSignature->GetStdUsageMask()) { // Using std buffers?
			_updateStdCB(ps.standardRootSignature->GetStdUsageMask());
			if (_stdCB1Resource)
				ps.standardRootSignature->SetStd1Buffer(this, _stdCB1Resource->GetGPUVirtualAddress());
			if (_stdCB2Resource)
				ps.standardRootSignature->SetStd2Buffer(this, _stdCB2Resource->GetGPUVirtualAddress());
		}
	}
}


HRESULT RenderSettings::Init(ID3D12Device *device, ID3D12CommandAllocator *pAllocator)
{
	HRESULT hr;
	V_RETURN(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pAllocator, nullptr, IID_PPV_ARGS(&_cl)));
	_cl->Close();
	return S_OK;
}



void RenderSettings::_updateStdCB(UINT requiredElements)
{
	if ((_stdElements & requiredElements) != requiredElements) 
	{
		UINT updateElements = ~_stdElements & requiredElements; // bitmask for elements to update.

		XMVECTOR det;
		
		// Std1 buffer
		if (updateElements & STD1_VIEW)
			XMStoreFloat4x4(&_std1Data.view, XMMatrixTranspose(_std1Matrices->view));
		if (updateElements & STD1_PROJECTION)
			XMStoreFloat4x4(&_std1Data.projection, XMMatrixTranspose(_std1Matrices->projection));
		if (updateElements & (STD1_VIEWINVERSE | STD1_VIEWINVERSETRANSPOSE))
			_std1Matrices->viewInverse = XMMatrixInverse(&det, _std1Matrices->view);
		if (updateElements & (STD1_VIEWINVERSE))
			XMStoreFloat4x4(&_std1Data.viewInverse, XMMatrixTranspose(_std1Matrices->viewInverse));
		if (updateElements & (STD1_VIEWINVERSETRANSPOSE))
			XMStoreFloat4x4(&_std1Data.viewInverseTranspose, _std1Matrices->viewInverse);
		if (updateElements & (STD1_PROJECTIONINVERSE))
			XMStoreFloat4x4(&_std1Data.projectionInverse, XMMatrixTranspose(_std1Matrices->projectionInverse = XMMatrixInverse(&det, _std1Matrices->projection)));
		if (updateElements & (STD1_VIEWPROJECTION | STD1_VIEWPROJECTIONINVERSE))
			_std1Matrices->viewProjection = XMMatrixMultiply(_std1Matrices->view, _std1Matrices->projection);
		if (updateElements & (STD1_VIEWPROJECTION))
			XMStoreFloat4x4(&_std1Data.viewProjection, XMMatrixTranspose(_std1Matrices->viewProjection));
		if (updateElements & (STD1_VIEWPROJECTIONINVERSE))
			XMStoreFloat4x4(&_std1Data.viewProjectionInverse,  XMMatrixTranspose(_std1Matrices->viewProjectionInverse = XMMatrixInverse(&det, _std1Matrices->viewProjection)));

		// Std2 buffer
		if (updateElements & STD2_WORLD)
			XMStoreFloat4x4(&_std2Data.world, XMMatrixTranspose(_std2Matrices->world));
		if (updateElements & (STD2_WORLDINVERSE | STD2_WORLDINVERSETRANSPOSE))
			_std2Matrices->worldInverse = XMMatrixInverse(&det, _std2Matrices->world);
		if (updateElements & STD2_WORLDINVERSE)
			XMStoreFloat4x4(&_std2Data.worldInverse, XMMatrixTranspose(_std2Matrices->worldInverse));
		if (updateElements & STD2_WORLDINVERSETRANSPOSE)
			XMStoreFloat4x4(&_std2Data.worldInverseTranspose, _std2Matrices->worldInverse);
		if (updateElements & (STD2_WORLDVIEW | STD2_WORLDVIEWINVERSE | STD2_WORLDVIEWINVERSETRANSPOSE | STD2_WORLDVIEWPROJECTION | STD2_WORLDVIEWPROJECTIONINVERSE))
			_std2Matrices->worldView = XMMatrixMultiply(_std2Matrices->world, _std1Matrices->view);
		if (updateElements & STD2_WORLDVIEW)
			XMStoreFloat4x4(&_std2Data.worldView, XMMatrixTranspose(_std2Matrices->worldView));
		if (updateElements & (STD2_WORLDVIEWINVERSE | STD2_WORLDVIEWINVERSETRANSPOSE))
			_std2Matrices->worldViewInverse = XMMatrixInverse(&det, _std2Matrices->worldView);
		if (updateElements & STD2_WORLDVIEWINVERSE)
			XMStoreFloat4x4(&_std2Data.worldViewInverse, XMMatrixTranspose(_std2Matrices->worldViewInverse));
		if (updateElements & STD2_WORLDVIEWINVERSETRANSPOSE)
			XMStoreFloat4x4(&_std2Data.worldViewInverseTranspose, _std2Matrices->worldViewInverse);
		if (updateElements & (STD2_WORLDVIEWPROJECTION | STD2_WORLDVIEWPROJECTIONINVERSE))
			_std2Matrices->worldViewProjection = XMMatrixMultiply(_std2Matrices->worldView, _std1Matrices->projection);
		if (updateElements & STD2_WORLDVIEWPROJECTION)
			XMStoreFloat4x4(&_std2Data.worldViewProjection, XMMatrixTranspose(_std2Matrices->worldViewProjection));
		if (updateElements & STD2_WORLDVIEWPROJECTIONINVERSE)
			XMStoreFloat4x4(&_std2Data.worldViewProjectionInverse, XMMatrixTranspose(_std2Matrices->worldViewProjectionInverse = XMMatrixInverse(&det, _std2Matrices->worldViewProjection)));
		if (updateElements & STD2_SHADOW)
			XMStoreFloat4x4(&_std2Data.shadow, XMMatrixTranspose(_std2Matrices->shadow * XMMatrixSet(0.5f,0.0f,0.0f,0.0f,0.0f,-0.5f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.5f,0.5f,0.0f,1.0f)));
		if (updateElements & STD2_WORLDSHADOW)
			XMStoreFloat4x4(&_std2Data.worldShadow, XMMatrixTranspose(_std2Matrices->worldShadow = XMMatrixMultiply(_std2Matrices->world, _std2Matrices->shadow * XMMatrixSet(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f))));

		_stdElements |= updateElements;

		UINT64 offset1 = -1, offset2 = -1;
		bool barriers = false;

		if (STD1_ELEMENTS & updateElements) {
			BYTE *data;
			D3D12_GPU_VIRTUAL_ADDRESS gpuAddr;
			graphics()->GetUploadHeap()->Allocate(Align(sizeof(StdCB1), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, &data, &gpuAddr, &offset1);
			std::memcpy(data, &_std1Data, sizeof(_std1Data));

			if (_stdCB1Resource) {
				CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(_stdCB1Resource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
				ResourceBarrier(1, &barrier);
				barriers = true;
			}
			else {
				CD3DX12_HEAP_PROPERTIES hp(D3D12_HEAP_TYPE_DEFAULT);
				CD3DX12_RESOURCE_DESC b = CD3DX12_RESOURCE_DESC::Buffer(Align(sizeof(StdCB1), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), D3D12_RESOURCE_FLAG_NONE, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
				if (FAILED(device()->CreateCommittedResource(
					&hp,
					D3D12_HEAP_FLAG_NONE, 
					&b,
					D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr,
					IID_PPV_ARGS(&_stdCB1Resource))))
				{
				}
				String s = "Std1Buffer";
				_stdCB1Resource->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT)s.size(), s.c_str() );
			}
		}

		if (STD2_ELEMENTS & updateElements) {
			BYTE *data;
			D3D12_GPU_VIRTUAL_ADDRESS gpuAddr;
			graphics()->GetUploadHeap()->Allocate(Align(sizeof(StdCB2), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, &data, &gpuAddr, &offset2);
			std::memcpy(data, &_std2Data, sizeof(_std2Data));

			if (_stdCB2Resource) {
				CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(_stdCB2Resource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
				ResourceBarrier(1, &barrier);
				barriers = true;
			}
			else {
				CD3DX12_HEAP_PROPERTIES hp(D3D12_HEAP_TYPE_DEFAULT);
				CD3DX12_RESOURCE_DESC b = CD3DX12_RESOURCE_DESC::Buffer(Align(sizeof(StdCB2), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), D3D12_RESOURCE_FLAG_NONE, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
				if (FAILED(device()->CreateCommittedResource(
					&hp,
					D3D12_HEAP_FLAG_NONE, 
					&b,
					D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr,
					IID_PPV_ARGS(&_stdCB2Resource))))
				{
				}
				String s = "Std2Buffer";
				_stdCB2Resource->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT)s.size(), s.c_str() );
			}
		}

		if (barriers)
			CommitResourceBarriers();

		if (offset1 != -1) {
			_cl->CopyBufferRegion(_stdCB1Resource, 0, graphics()->GetUploadHeap()->GetBufferResource(), offset1, sizeof(StdCB1));
			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(_stdCB1Resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			ResourceBarrier(1, &barrier);
		}
		if (offset2 != -1) {
			_cl->CopyBufferRegion(_stdCB2Resource, 0, graphics()->GetUploadHeap()->GetBufferResource(), offset2, sizeof(StdCB2));
			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(_stdCB2Resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			ResourceBarrier(1, &barrier);
		}
	}
}

void RenderSettings::CloseAndExecuteCommandList()
{
	if (!_cl)
		return;

	CommitResourceBarriers();
	
	HRESULT hr = device()->GetDeviceRemovedReason();
	assert(hr == S_OK);

	V(_cl->Close());

	ID3D12CommandList* ppCommandLists[] = { _cl };
	graphics()->GetCommandQueue()->ExecuteCommandLists(1, ppCommandLists);

	ClearCachedState();
}

const PipelineState &RenderSettings::GetPipelineState()
{
	return graphics()->GetPipelineStatePool()->GetState(_psoID);
}

const PipelineState &RenderSettings::GetComputePipelineState()
{
	return graphics()->GetPipelineStatePool()->GetComputeState(_psoID);
}

void RenderSettings::ResourceBarrier(UINT NumBarriers, const D3D12_RESOURCE_BARRIER *pBarriers)
{
	std::memcpy(_currentBarrier, pBarriers, sizeof(D3D12_RESOURCE_BARRIER) * NumBarriers);
	_currentBarrier += NumBarriers;
}

ResourceStateLock RenderSettings::EnterState(ResourceStateTracker *rst, const D3D12_SHADER_RESOURCE_VIEW_DESC *srv, D3D12_RESOURCE_STATES afterState)
{
	assert(rst);
	return rst->EnterState(&_currentBarrier, srv, afterState);
}

ResourceStateLock RenderSettings::EnterState(ResourceStateTracker *rst, const D3D12_RENDER_TARGET_VIEW_DESC *rtv, D3D12_RESOURCE_STATES afterState)
{
	assert(rst);
	return rst->EnterState(&_currentBarrier, rtv, afterState);
}

ResourceStateLock RenderSettings::EnterState(ResourceStateTracker *rst, const D3D12_DEPTH_STENCIL_VIEW_DESC *dsv, D3D12_RESOURCE_STATES afterState)
{
	assert(rst);
	return rst->EnterState(&_currentBarrier, dsv, afterState);
}

ResourceStateLock RenderSettings::EnterState(ResourceStateTracker *rst, const D3D12_UNORDERED_ACCESS_VIEW_DESC *uav, D3D12_RESOURCE_STATES afterState)
{
	assert(uav);
	return rst->EnterState(&_currentBarrier, uav, afterState);
}

ResourceStateLock RenderSettings::EnterState(ResourceStateTracker* rst, const D3D12_CONSTANT_BUFFER_VIEW_DESC* cbv, D3D12_RESOURCE_STATES afterState)
{
	assert(cbv);
	return rst->EnterState(&_currentBarrier, afterState);
}

ResourceStateLock RenderSettings::EnterState(ResourceStateTracker *rst, const SubresourceRange &srr, D3D12_RESOURCE_STATES afterState)
{
	return rst->EnterState(&_currentBarrier, srr, afterState);
}

ResourceStateLock RenderSettings::EnterState(ResourceStateTracker *rst, D3D12_RESOURCE_STATES afterState)
{
	assert(rst);
	return rst->EnterState(&_currentBarrier, afterState);
}

void RenderSettings::CommitResourceBarriers()
{
	if (_currentBarrier > _barriers) {
		_cl->ResourceBarrier(UINT(_currentBarrier - _barriers), _barriers);
		_currentBarrier = _barriers;
	}
}

void RenderSettings::CommitGraphicsRootSignature()
{
	if (_psoID.rsID != _currentGraphicsRootSignature) {
		ID3D12RootSignature *s = graphics()->GetPipelineStatePool()->GetRootSignature(_psoID.rsID);
		if (s)
			_cl->SetGraphicsRootSignature(s);
		_currentGraphicsRootSignature = _currentPsoID.rsID = _psoID.rsID;
	}
}

void RenderSettings::CommitGraphicsRootSignatureAndPipelineState()
{
	if (_psoID != _currentPsoID || _psoID.rsID != _currentGraphicsRootSignature) {
		auto s = graphics()->GetPipelineStatePool()->GetState(_psoID);
		if (_psoID.rsID != _currentGraphicsRootSignature) {
			_currentGraphicsRootSignature = _psoID.rsID;
			_cl->SetGraphicsRootSignature(s.rootSignature);
		}
		if ((_psoID.id ^ _currentPsoID.id) & 0xFFFFFF00)
			_cl->SetPipelineState(s.pso);
		_currentPsoID = _psoID;
	}
}

void RenderSettings::CommitComputeRootSignature()
{
	if (_psoID.rsID != _currentComputeRootSignature) {
		ID3D12RootSignature *s = graphics()->GetPipelineStatePool()->GetRootSignature(_psoID.rsID);
		if (s)
			_cl->SetComputeRootSignature(s);
		_currentComputeRootSignature = _currentPsoID.rsID = _psoID.rsID;
	}
}

void RenderSettings::CommitComputeRootSignatureAndPipelineState()
{
	if (_psoID != _currentPsoID || _psoID.rsID != _currentComputeRootSignature) {
		auto s = graphics()->GetPipelineStatePool()->GetState(_psoID);
		if (_psoID.rsID != _currentComputeRootSignature) {
			_currentComputeRootSignature = _psoID.rsID;
			_cl->SetComputeRootSignature(s.rootSignature);
		}
		if ((_psoID.id ^ _currentPsoID.id) & 0xFFFFFF00)
			_cl->SetPipelineState(s.pso);
		_currentPsoID = _psoID;
	}
}

void RenderSettings::ClearCachedState()
{
	_rtWidth = _rtHeight = 0;
	_numRenderTargets = 0;
	memset(_renderTargets, 0, sizeof(_renderTargets));
	_depthBuffer = CD3DX12_CPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT());

	_numViewportsAndScissorRects = 0;
	memset(_viewports, 0, sizeof(_viewports));
	memset(_scissorRects, 0, sizeof(_scissorRects));

	_pt = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	_blendFactor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	_stencilRef = 0;

	_psoID = _currentPsoID = 0;

	_currentGraphicsRootSignature = 0;
	_currentComputeRootSignature = 0;

	_lockedGraphicsRootDescriptorTables.clear();
	_lockedComputeRootDescriptorTables.clear();
	_renderTargetLocks.clear();
}

void RenderSettings::IASetPrimitiveTopology(M3D_PRIMITIVE_TOPOLOGY PrimitiveTopology)
{
	if (_pt != PrimitiveTopology)
		_cl->IASetPrimitiveTopology(_pt = (D3D12_PRIMITIVE_TOPOLOGY)PrimitiveTopology);
}

void RenderSettings::OMSetBlendFactor(const XMFLOAT4 &BlendFactor)
{
	if (std::memcmp(&_blendFactor, &BlendFactor, sizeof(XMFLOAT4)))
		_cl->OMSetBlendFactor((const FLOAT*)&(_blendFactor = BlendFactor));
}

void RenderSettings::OMSetStencilRef(UINT StencilRef)
{
	if (_stencilRef != StencilRef)
		_cl->OMSetStencilRef(_stencilRef = StencilRef);
}

void RenderSettings::OMSetRenderTargets(UINT NumRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE *pRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE *pDepthStencilDescriptor)
{
	UINT width = 0, height = 0;
	PipelineOutputStateDesc posDesc;
	posDesc.NumRenderTargets = NumRenderTargetDescriptors;
	posDesc.SampleDesc.Count = -1;

	_renderTargetLocks.clear();
	if (NumRenderTargetDescriptors == 0 && pDepthStencilDescriptor == nullptr) {
		SetPipelineOutputStateDesc(0);
		_numRenderTargets = 0;
		_depthBuffer = CD3DX12_CPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT());
		_rtWidth = _rtHeight = 0;
		return _cl->OMSetRenderTargets(0, nullptr, 0, 0);
	}

	if (pDepthStencilDescriptor) {
		const Descriptor &desc = graphics()->GetDSVHeapManager()->GetDescriptor(*pDepthStencilDescriptor);
		D3D12_RESOURCE_DESC rDesc = desc.resource->GetDesc();
		posDesc.DSVFormat = desc.dsv ? desc.dsv->Format : rDesc.Format;
		posDesc.SampleDesc = rDesc.SampleDesc;
		CalculateDepthStencilSize(width, height, desc.resource, desc.dsv);
		SResourceStateTracker rst;
		GetResourceStateTracker(desc.resource, &rst);
		if (rst)
			_renderTargetLocks.push_back(EnterState(rst, desc.dsv));
	}

	for (UINT i = 0; i < NumRenderTargetDescriptors; i++) {
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pRenderTargetDescriptors[i];
		const Descriptor &desc = graphics()->GetRTVHeapManager()->GetDescriptor(rtvHandle);
		D3D12_RESOURCE_DESC rDesc = desc.resource->GetDesc();
		if (posDesc.SampleDesc.Count == -1)
			posDesc.SampleDesc = rDesc.SampleDesc;
		else if (posDesc.SampleDesc.Count != rDesc.SampleDesc.Count || posDesc.SampleDesc.Quality != rDesc.SampleDesc.Quality)
			throw GraphicsException(MTEXT("All render targets must have the same settings for multisampling!"), WARN);
		UINT w, h;
		CalculateRenderTargetSize(w, h, desc.resource, desc.rtv);
		if (width != 0 && width != w || height != 0 && height != h)
			throw GraphicsException(MTEXT("All render targets must have the same width and height."), WARN);
		width = w;
		height = h;
		posDesc.RTVFormats[i] = desc.rtv ? desc.rtv->Format : rDesc.Format;
		SResourceStateTracker rst;
		GetResourceStateTracker(desc.resource, &rst);
		if (rst)
			_renderTargetLocks.push_back(EnterState(rst, desc.rtv));
	}

	if (posDesc.SampleDesc.Count == -1)
		posDesc.SampleDesc.Count = 1;

	PipelineStateDescID posDescID = graphics()->GetPipelineStatePool()->RegisterPipelineOutputStateDesc(posDesc);
	if (posDescID == 0)
		throw GraphicsException(MTEXT("Invalid pipeline output desc."), WARN);

	SetPipelineOutputStateDesc(posDescID);
	if (NumRenderTargetDescriptors > 0 || pDepthStencilDescriptor != nullptr)
		CommitResourceBarriers();
	_cl->OMSetRenderTargets(NumRenderTargetDescriptors, pRenderTargetDescriptors, FALSE, pDepthStencilDescriptor);

	// Preserve state!
	_numRenderTargets = NumRenderTargetDescriptors;
	if (_numRenderTargets > 0)
		std::memcpy(_renderTargets, pRenderTargetDescriptors, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * NumRenderTargetDescriptors);
	_depthBuffer = pDepthStencilDescriptor ? *pDepthStencilDescriptor : CD3DX12_CPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT());
	_rtWidth = width;
	_rtHeight = height;
}

void RenderSettings::RSSetViewportsAndScissorRects(UINT NumViewportsAndScissorRects, const D3D12_VIEWPORT *pViewports, const D3D12_RECT *pRects)
{
	assert(NumViewportsAndScissorRects <= D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);

	_cl->RSSetViewports(NumViewportsAndScissorRects, pViewports);
	_cl->RSSetScissorRects(NumViewportsAndScissorRects, pRects);

	// Preserve State!
	_numViewportsAndScissorRects = NumViewportsAndScissorRects;
	std::memcpy(_viewports, pViewports, sizeof(D3D12_VIEWPORT) * NumViewportsAndScissorRects);
	std::memcpy(_scissorRects, pRects, sizeof(D3D12_RECT) * NumViewportsAndScissorRects);
}

void RenderSettings::UpdateSubresources(ID3D12Resource* pDestinationResource, ID3D12Resource* pIntermediate, UINT64 IntermediateOffset, UINT FirstSubresource, UINT NumSubresources, D3D12_SUBRESOURCE_DATA* pSrcData)
{
	::UpdateSubresources(_cl, pDestinationResource, pIntermediate, IntermediateOffset, FirstSubresource, NumSubresources, pSrcData);
}

void RenderSettings::SetGraphicsRootConstantBufferView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) 
{

	return _cl->SetGraphicsRootConstantBufferView(RootParameterIndex, BufferLocation); 
}

void RenderSettings::SetGraphicsRootDescriptorTable(UINT RootParameterIndex, const DescriptorTable *dt)
{
	List<ResourceStateLock> &locks = _lockedGraphicsRootDescriptorTables[RootParameterIndex];
	locks.clear();

	for (UINT i = 0; i < dt->count; i++) {
		const Descriptor &d = dt->GetDescriptor(i);
		if (d.resource) {
			SResourceStateTracker rst;
			GetResourceStateTracker(d.resource, &rst);
			if (rst) {
				switch (d.type)
				{
				case DescriptorType::SRV:
					locks.push_back(EnterState(rst, d.srv));
					break;
				case DescriptorType::CBV: 
					locks.push_back(EnterState(rst, d.cbv));
					break;
				case DescriptorType::UAV:
					locks.push_back(EnterState(rst, d.uav));
					break;
				}
			}
		}
	}
	return _cl->SetGraphicsRootDescriptorTable(RootParameterIndex, dt->GetGPUHandle());
}

void RenderSettings::SetComputeRootConstantBufferView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
{
	return _cl->SetComputeRootConstantBufferView(RootParameterIndex, BufferLocation);
}

void RenderSettings::SetComputeRootDescriptorTable(UINT RootParameterIndex, const DescriptorTable *dt)
{
	List<ResourceStateLock> &locks = _lockedComputeRootDescriptorTables[RootParameterIndex];
	locks.clear();

	for (UINT i = 0; i < dt->count; i++) {
		const Descriptor &d = dt->GetDescriptor(i);
		if (d.resource) {
			SResourceStateTracker rst;
			GetResourceStateTracker(d.resource, &rst);
			if (rst) {
				switch (d.type)
				{
				case DescriptorType::SRV:
					locks.push_back(EnterState(rst, d.srv));
					break;
				case DescriptorType::CBV: 
					locks.push_back(EnterState(rst, d.cbv));
					break;
				case DescriptorType::UAV:
					locks.push_back(EnterState(rst, d.uav));
					break;
				}
			}
		}
	}
	return _cl->SetComputeRootDescriptorTable(RootParameterIndex, dt->GetGPUHandle());
}


void RenderSettings::ClearGraphicsRootDescriptorTables()
{
	/*for (auto a = _lockedGraphicsRootDescriptorTables.front(); a.valid(); a++) {
		D3D12_GPU_DESCRIPTOR_HANDLE d;
		d.ptr = 0;
		_cl->SetGraphicsRootDescriptorTable(a->first, d);
	}*/
	_lockedGraphicsRootDescriptorTables.clear();
	_lockedComputeRootDescriptorTables.clear(); 
}

void RenderSettings::DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation) 
{ 
	return _cl->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation); 
}

void RenderSettings::DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation) 
{ 
	return _cl->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation); 
}

void RenderSettings::PushState() 
{
	_pipelineStateStack[_pipelineStateStackSize++] = _psoID;
}

void RenderSettings::PopState()
{
	_psoID = _pipelineStateStack[--_pipelineStateStackSize];
}
