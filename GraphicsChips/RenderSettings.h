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
#include "Graphics.h"
#include "RingBuffer.h"
#include "ResourceStateTracker.h"
#include "GraphicsChips/RenderState.h"

namespace m3d
{


#define STDCB1_HLSL_TEXT \
	"cbuffer std1 : STD_CBUFFER1\n" \
	"{\n" \
	"	float4x4 view;\n" \
	"	float4x4 projection;\n" \
	"	float4x4 viewInverse;\n" \
	"	float4x4 viewInverseTranspose;\n" \
	"	float4x4 projectionInverse;\n" \
	"	float4x4 viewProjection;\n" \
	"	float4x4 viewProjectionInverse;\n" \
	"};\n"

#define STDCB2_HLSL_TEXT \
	"cbuffer std2 : STD_CBUFFER2\n" \
	"{\n" \
	"	float4x4 world;\n" \
	"	float4x4 worldInverse;\n" \
	"	float4x4 worldInverseTranspose;\n" \
	"	float4x4 worldView;\n" \
	"	float4x4 worldViewInverse;\n" \
	"	float4x4 worldViewInverseTranspose;\n" \
	"	float4x4 worldViewProjection;\n" \
	"	float4x4 worldViewProjectionInverse;\n" \
	"	float4x4 shadow;\n" \
	"	float4x4 worldShadow;\n" \
	"};\n"

/*
"    float value1 : packoffset(c0.x);\n" \
"    float value2 : packoffset(c0.y);\n" \
"    float value3 : packoffset(c0.z);\n" \
"    float value4 : packoffset(c0.w);\n" \
"    float value5 : packoffset(c1.x);\n" \
"    float value6 : packoffset(c1.y);\n" \
"    float value7 : packoffset(c1.z);\n" \
"    float value8 : packoffset(c1.w);\n" \
"    float value9 : packoffset(c2.x);\n" \
"    float value10 : packoffset(c2.y);\n" \
"    float value11 : packoffset(c2.z);\n" \
"    float value12 : packoffset(c2.w);\n" \
"    float value13 : packoffset(c3.x);\n" \
"    float value14 : packoffset(c3.y);\n" \
"    float value15 : packoffset(c3.z);\n" \
"    float value16 : packoffset(c3.w);\n" \
"    float4 vector1 : packoffset(c4);\n" \
"    float4 vector2 : packoffset(c5);\n" \
"    float4 vector3 : packoffset(c6);\n" \
"    float4 vector4 : packoffset(c7);\n" \
"    float4x4 matrix1 : packoffset(c8);\n" \
"    float4 vector5 : packoffset(c12);\n" \
"    float4 vector6 : packoffset(c13);\n" \
"    float4 vector7 : packoffset(c14);\n" \
"    float4 vector8 : packoffset(c15);\n" \
"    float4x4 matrix2 : packoffset(c16);\n" \
"    float4 vector9 : packoffset(c20);\n" \
"    float4 vector10 : packoffset(c21);\n" \
"    float4 vector11 : packoffset(c22);\n" \
"    float4 vector12 : packoffset(c23);\n" \
"    float4x4 matrix3 : packoffset(c24);\n" \
"    float4x4 matrix4 : packoffset(c28);\n" \
"    float4x4 matrix5 : packoffset(c32);\n" \
"    float4x4 matrix6 : packoffset(c36);\n" \
"    float4x4 matrix7 : packoffset(c40);\n" \
"    float4x4 matrix8 : packoffset(c44);\n" \
"    float4x4 matrix9 : packoffset(c48);\n" \
"    float4x4 matrix10 : packoffset(c52);\n" \
"    float4x4 matrix11 : packoffset(c56);\n" \
"    float4x4 matrix12 : packoffset(c60);\n" \
*/
#define STDCB_HLSL_TEXT \
	"struct StdCBuffer\n" \
	"{\n" \
	"    float value1;\n" \
	"    float value2;\n" \
	"    float value3;\n" \
	"    float value4;\n" \
	"    float value5;\n" \
	"    float value6;\n" \
	"    float value7;\n" \
	"    float value8;\n" \
	"    float value9;\n" \
	"    float value10;\n" \
	"    float value11;\n" \
	"    float value12;\n" \
	"    float value13;\n" \
	"    float value14;\n" \
	"    float value15;\n" \
	"    float value16;\n" \
	"    float4 vector1;\n" \
	"    float4 vector2;\n" \
	"    float4 vector3;\n" \
	"    float4 vector4;\n" \
	"    float4 vector5;\n" \
	"    float4 vector6;\n" \
	"    float4 vector7;\n" \
	"    float4 vector8;\n" \
	"    float4 vector9;\n" \
	"    float4 vector10;\n" \
	"    float4 vector11;\n" \
	"    float4 vector12;\n" \
	"    float4x4 matrix1;\n" \
	"    float4x4 matrix2;\n" \
	"    float4x4 matrix3;\n" \
	"    float4x4 matrix4;\n" \
	"    float4x4 matrix5;\n" \
	"    float4x4 matrix6;\n" \
	"    float4x4 matrix7;\n" \
	"    float4x4 matrix8;\n" \
	"    float4x4 matrix9;\n" \
	"    float4x4 matrix10;\n" \
	"    float4x4 matrix11;\n" \
	"    float4x4 matrix12;\n" \
	"};\n"


/*
"ConstantBuffer<Std1> std1 : STD_CB1;\n" \
"ConstantBuffer<Std2> std2 : STD_CB2;\n" \
"#define view std1.View\n" \
"#define projection std1.Projection\n" \
"#define viewInverse std1.ViewInverse\n" \
"#define viewInverseTranspose std1.ViewInverseTranspose\n" \
"#define projectionInverse std1.ProjectionInverse\n" \
"#define viewProjection std1.ViewProjection\n" \
"#define viewProjectionInverse std1.ViewProjectionInverse\n" \
"#define world std2.World\n" \
"#define worldInverse std2.WorldInverse\n" \
"#define worldInverseTranspose std2.WorldInverseTranspose\n" \
"#define worldView std2.WorldView\n" \
"#define worldViewInverse std2.WorldViewInverse\n" \
"#define worldViewInverseTranspose std2.WorldViewInverseTranspose\n" \
"#define worldViewProjection std2.WorldViewProjection\n" \
"#define worldViewProjectionInverse std2.WorldViewProjectionInverse\n" \
"#define shadow std2.Shadow\n" \
"#define worldShadow std2.WorldShadow\n" \
"\n\n" \
*/

#define STD_TEXT \
	"\n" \
	"#define STD_CBUFFER1 register(b0)\n" \
	"#define STD_CBUFFER2 register(b1)\n" \
	"\n\n" \
    "// Camera-matrices invariant of the objects being rendered\n" \
	STDCB1_HLSL_TEXT \
	"\n\n" \
    "// Object-Camera-matrices updated for each 3D-object rendered\n" \
	STDCB2_HLSL_TEXT \
	"\n\n" \
	"#define worldNormalMatrix ((float3x3)worldInverseTranspose)\n" \
	"#define normalMatrix ((float3x3)worldViewInverseTranspose)\n" \
	"\n\n" \
    "// ConstantBuffers (CBV) connected to the Material/Compute-chip\n" \
    "#define CBUFFER1 register(b2)\n" \
    "#define CBUFFER2 register(b3)\n" \
    "#define CBUFFER3 register(b4)\n" \
    "#define CBUFFER4 register(b5)\n" \
	"\n\n" \
    "// Textures/TextureBuffers (SRV) connected to the Material/Compute-chip\n" \
	"#define TEXTURE1 register(t0)\n" \
	"#define TEXTURE2 register(t1)\n" \
	"#define TEXTURE3 register(t2)\n" \
	"#define TEXTURE4 register(t3)\n" \
	"#define TEXTURE5 register(t4)\n" \
	"#define TEXTURE6 register(t5)\n" \
	"#define TEXTURE7 register(t6)\n" \
	"#define TEXTURE8 register(t7)\n" \
	"\n\n" \
    "// Unordered access views (UAV) connected to the Compute-chip\n" \
	"#define UAV1 register(u0)\n" \
	"#define UAV2 register(u1)\n" \
	"#define UAV3 register(u2)\n" \
	"#define UAV4 register(u3)\n" \
	"\n\n" \
    "// Samplers connected to the Material/Compute-chip\n" \
	"#define SAMPLER1 register(s1, space1)\n" \
	"#define SAMPLER2 register(s2, space1)\n" \
	"#define SAMPLER3 register(s3, space1)\n" \
	"#define SAMPLER4 register(s4, space1)\n" \
	"#define SAMPLER5 register(s5, space1)\n" \
	"#define SAMPLER6 register(s6, space1)\n" \
	"#define SAMPLER7 register(s7, space1)\n" \
	"#define SAMPLER8 register(s8, space1)\n" \
	"\n\n" \
    "// Samplers connected to the root signature\n" \
	"#define STATIC_SAMPLER1    register(s10)\n" \
	"#define STATIC_SAMPLER2    register(s11)\n" \
	"#define STATIC_SAMPLER3    register(s12)\n" \
	"#define STATIC_SAMPLER4    register(s13)\n" \
	"#define STATIC_SAMPLER5    register(s14)\n" \
	"#define STATIC_SAMPLER6    register(s15)\n" \
	"#define STATIC_SAMPLER7    register(s16)\n" \
	"#define STATIC_SAMPLER8    register(s17)\n" \
	"#define STATIC_SAMPLER9    register(s18)\n" \
	"#define STATIC_SAMPLER10   register(s19)\n" \
	"#define STATIC_SAMPLER11   register(s20)\n" \
	"#define STATIC_SAMPLER12   register(s21)\n" \
	"#define STATIC_SAMPLER13   register(s22)\n" \
	"#define STATIC_SAMPLER14   register(s23)\n" \
	"#define STATIC_SAMPLER15   register(s24)\n" \
	"#define STATIC_SAMPLER16   register(s25)\n" \
	"\n\n" \
    "// A set of predefined samplers often used\n" \
	"#define DEFAULT_SAMPLER                      register(s6)\n" \
	"#define DEFAULT_POINT_SAMPLER                register(s1)\n" \
	"#define DEFAULT_LINEAR_SAMPLER               register(s2)\n" \
	"#define DEFAULT_ANISOTROPIC_SAMPLER          register(s3)\n" \
	"#define DEFAULT_WRAPPED_POINT_SAMPLER        register(s4)\n" \
	"#define DEFAULT_WRAPPED_LINEAR_SAMPLER       register(s5)\n" \
	"#define DEFAULT_WRAPPED_ANISOTROPIC_SAMPLER  register(s6)\n" \
	"\n\n" \
	"#ifndef NO_DEFAULT_SAMPLER_INSTANCES\n" \
	"sampler defaultSampler     : DEFAULT_SAMPLER;\n" \
	"sampler defaultPSampler    : DEFAULT_POINT_SAMPLER;\n" \
	"sampler defaultLSampler    : DEFAULT_LINEAR_SAMPLER;\n" \
	"sampler defaultASampler    : DEFAULT_ANISOTROPIC_SAMPLER;\n" \
	"sampler defaultWPSampler   : DEFAULT_WRAPPED_POINT_SAMPLER;\n" \
	"sampler defaultWLSampler   : DEFAULT_WRAPPED_LINEAR_SAMPLER;\n" \
	"sampler defaultWASampler   : DEFAULT_WRAPPED_ANISOTROPIC_SAMPLER;\n" \
	"#endif\n"


		/*
		"#if VERSION >= 5100\n" \
		"sampler material_samplers[128] : register(s1);\n" \
		"sampler static_samplers[128] : register(s129);\n" \
		"#else\n" \
		"sampler material_samplers[4] : register(s1);\n" \
		"sampler static_samplers[11] : register(s5);\n" \
		"#endif\n" \
		"\n\n" \
		"#define material_sampler(x) material_samplers[x]\n" \
		"#define static_sampler(x) static_samplers[x]\n\n" \
		"#define material_sampler1 material_sampler(0)\n" \
		"#define material_sampler2 material_sampler(1)\n" \
		"#define material_sampler3 material_sampler(2)\n" \
		"#define material_sampler4 material_sampler(3)\n" \
		"#define static_sampler1 static_sampler(0)\n" \
		"#define static_sampler2 static_sampler(1)\n" \
		"#define static_sampler3 static_sampler(2)\n" \
		"#define static_sampler4 static_sampler(3)\n" \
		"#define static_sampler5 static_sampler(4)\n" \
		"#define static_sampler6 static_sampler(5)\n" \
		"#define static_sampler7 static_sampler(6)\n" \
		"#define static_sampler8 static_sampler(7)\n" \
		"#define static_sampler9 static_sampler(8)\n" \
		"#define static_sampler10 static_sampler(9)\n" \
		"#define static_sampler11 static_sampler(10)\n"
*/



static SIZE_T Align(SIZE_T p, UINT alignment) { return (p + (alignment - 1)) & ~SIZE_T(alignment - 1); }


class GRAPHICSCHIPS_API RenderSettings : public RenderState, public GraphicsUsage
{
	CHIPDESC_DECL;
public:
	RenderSettings();
	virtual ~RenderSettings();

	void OnDestroyDevice() override;  
	void OnReleasingBackBuffer(RenderWindow *rw) override;

/*
	typedef Map<String, List<SID3D11ShaderResourceView>> ShaderResourceMap;

	void SetShaderResources(String semantic, const List<SID3D11ShaderResourceView> &r) { _shaderResources[semantic] = r; }
	const ShaderResourceMap &GetShaderResources() const { return _shaderResources; }
*/
	HRESULT Init(ID3D12Device *device, ID3D12CommandAllocator *pAllocator);

	ID3D12GraphicsCommandList *GetCommandList1() { return _cl; }

	void PrepareDraw() override;


	void CloseAndExecuteCommandList();

	inline HRESULT Reset(ID3D12CommandAllocator *pAllocator, ID3D12PipelineState *pInitialState) { return _cl->Reset(pAllocator, pInitialState); }

	UINT GetRenderTargetWidth() const { return _rtWidth; }
	UINT GetRenderTargetHeight() const  { return _rtHeight; }

	UINT GetNumViewportsAndScissorRects() const { return _numViewportsAndScissorRects; }
	const D3D12_VIEWPORT *GetViewports() const { return _viewports; }
	const D3D12_RECT *GetScissorRects() const { return _scissorRects; }

	PipelineStateID GetPipelineStateID() const { return _psoID; }
	const PipelineState &GetPipelineState();
	const PipelineState &GetComputePipelineState();

	void ResourceBarrier(UINT NumBarriers, const D3D12_RESOURCE_BARRIER *pBarriers);
	ResourceStateLock EnterState(ResourceStateTracker *rst, const D3D12_SHADER_RESOURCE_VIEW_DESC *srv, D3D12_RESOURCE_STATES afterState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	ResourceStateLock EnterState(ResourceStateTracker *rst, const D3D12_RENDER_TARGET_VIEW_DESC *rtv, D3D12_RESOURCE_STATES afterState = D3D12_RESOURCE_STATE_RENDER_TARGET);
	ResourceStateLock EnterState(ResourceStateTracker *rst, const D3D12_DEPTH_STENCIL_VIEW_DESC *dsv, D3D12_RESOURCE_STATES afterState = D3D12_RESOURCE_STATE_DEPTH_WRITE);
	ResourceStateLock EnterState(ResourceStateTracker *rst, const D3D12_UNORDERED_ACCESS_VIEW_DESC *uav, D3D12_RESOURCE_STATES afterState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	ResourceStateLock EnterState(ResourceStateTracker* rst, const D3D12_CONSTANT_BUFFER_VIEW_DESC* cbv, D3D12_RESOURCE_STATES afterState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	ResourceStateLock EnterState(ResourceStateTracker *rst, const SubresourceRange &srr, D3D12_RESOURCE_STATES afterState);
	ResourceStateLock EnterState(ResourceStateTracker *rst, D3D12_RESOURCE_STATES afterState);
	void CommitResourceBarriers() override; // This will add all barriers to the command list. Batch as much as possible!

	void SetGraphicsRootSignature(RootSignatureID id) { _psoID.rsID = id; }
	void SetPipelineStateDesc(PipelineStateDescID id) { _psoID.psID = id; }
	void SetPipelineInputStateDesc(PipelineStateDescID id) { _psoID.pisID = id; }
	void SetPipelineOutputStateDesc(PipelineStateDescID id) { _psoID.posID = id; }
	void SetPipelineStateID(PipelineStateID psID) { _psoID = psID; }

	void CommitGraphicsRootSignature();
	void CommitGraphicsRootSignatureAndPipelineState();

	void CommitComputeRootSignature();
	void CommitComputeRootSignatureAndPipelineState();

	void ClearCachedState();

	inline void SetDescriptorHeaps(UINT NumDescriptorHeaps, ID3D12DescriptorHeap **ppDescriptorHeaps) { return _cl->SetDescriptorHeaps(NumDescriptorHeaps, ppDescriptorHeaps); }

	void IASetPrimitiveTopology(M3D_PRIMITIVE_TOPOLOGY PrimitiveTopology) override;
	inline void IASetVertexBuffers(UINT StartSlot, UINT NumViews, D3D12_VERTEX_BUFFER_VIEW *pViews) { return _cl->IASetVertexBuffers(StartSlot, NumViews, pViews); }
	inline void IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW *pView) { return _cl->IASetIndexBuffer(pView); }

	void OMSetBlendFactor(const XMFLOAT4 &BlendFactor);
	void OMSetStencilRef(UINT StencilRef);
	void OMSetRenderTargets(UINT NumRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE *pRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE *pDepthStencilDescriptor);

	void RSSetViewportsAndScissorRects(UINT NumViewportsAndScissorRects, const D3D12_VIEWPORT *pViewports, const D3D12_RECT *pRects);

	// Note for CBV: These functions does NOT set the state for CBV. Handle it elsewhere!
	void SetGraphicsRootConstantBufferView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
	void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, const DescriptorTable *dt);

	void SetComputeRootConstantBufferView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
	void SetComputeRootDescriptorTable(UINT RootParameterIndex, const DescriptorTable *dt);

	// Make sure resource is in the correct state!
	inline void ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView, const FLOAT ColorRGBA[4], UINT NumRects, D3D12_RECT *pRects) { return _cl->ClearRenderTargetView(RenderTargetView, ColorRGBA, NumRects, pRects); }
	inline void ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView, D3D12_CLEAR_FLAGS ClearFlags, FLOAT Depth, UINT8 Stencil, UINT NumRects, const D3D12_RECT *pRects) { _cl->ClearDepthStencilView(DepthStencilView, ClearFlags, Depth, Stencil, NumRects, pRects); }
	inline void ResolveSubresource(ID3D12Resource *pDstResource, UINT DstSubresource, ID3D12Resource *pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format) { _cl->ResolveSubresource(pDstResource, DstSubresource, pSrcResource, SrcSubresource, Format); }
	inline void CopyResource(ID3D12Resource *pDstResource, ID3D12Resource *pSrcResource) { _cl->CopyResource(pDstResource, pSrcResource); }

	void UpdateSubresources(ID3D12Resource* pDestinationResource, ID3D12Resource* pIntermediate, UINT64 IntermediateOffset, UINT FirstSubresource, UINT NumSubresources, D3D12_SUBRESOURCE_DATA* pSrcData);

	inline void CopyBufferRegion(ID3D12Resource *pDstBuffer, UINT64 DstOffset, ID3D12Resource *pSrcBuffer, UINT64 SrcOffset, UINT64 NumBytes) { return _cl->CopyBufferRegion(pDstBuffer, DstOffset, pSrcBuffer, SrcOffset, NumBytes); }
	inline void CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION *pDst, UINT DstX, UINT DstY, UINT DstZ, const D3D12_TEXTURE_COPY_LOCATION *pSrc, const D3D12_BOX *pSrcBox) { return _cl->CopyTextureRegion(pDst, DstX, DstY, DstZ, pSrc, pSrcBox); }
	void DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation) override;
	void DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation) override;

	void Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ) { return _cl->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ); }

	void ClearGraphicsRootDescriptorTables();

	void PushState() override;
	void PopState() override;

protected:
	// Std1&Std2 buffers.
	SID3D12Resource _stdCB1Resource;
	SID3D12Resource _stdCB2Resource;

	UINT _numRenderTargets;
	UINT _rtWidth;
	UINT _rtHeight;
	D3D12_CPU_DESCRIPTOR_HANDLE _renderTargets[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
	D3D12_CPU_DESCRIPTOR_HANDLE _depthBuffer;

	UINT _numViewportsAndScissorRects;
	D3D12_VIEWPORT _viewports[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	D3D12_RECT _scissorRects[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

	D3D12_RESOURCE_BARRIER _barriers[256];
	D3D12_RESOURCE_BARRIER *_currentBarrier;

	PipelineStateID _psoID;
	PipelineStateID _currentPsoID;
	RootSignatureID _currentGraphicsRootSignature;
	RootSignatureID _currentComputeRootSignature;

//	ShaderResourceMap _shaderResources;

	// RenderSettings own the command list.
	SID3D12GraphicsCommandList _cl;


	D3D12_PRIMITIVE_TOPOLOGY _pt;
	XMFLOAT4 _blendFactor;
	UINT _stencilRef;

	Map<UINT, List<ResourceStateLock>> _lockedGraphicsRootDescriptorTables;
	Map<UINT, List<ResourceStateLock>> _lockedComputeRootDescriptorTables;
	List<ResourceStateLock> _renderTargetLocks;

	PipelineStateID _pipelineStateStack[4] = { 0, 0, 0, 0 };
	UINT _pipelineStateStackSize = 0;
/*
	struct RootElements
	{
		enum Type { UNASSIGNED, CBV, DT, SRV };
		Type type;
		union
		{
			D3D12_GPU_VIRTUAL_ADDRESS BufferLocation;
			D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor;
		};
	};

	RootElements _graphicsRootElements[64];
*/
	void _updateStdCB(UINT requiredElements);

};

}


