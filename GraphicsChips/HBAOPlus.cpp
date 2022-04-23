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
#include "HBAOPlus.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "RenderSettings.h"
#include "GraphicsResourceChip.h"
#include "StdChips/MatrixChip.h"
#include "D3D12Formats.h"
#include "HBAOPlus/include/GFSDK_SSAO.h"

using namespace m3d;


CHIPDESCV1_DEF(HBAOPlusChip, MTEXT("HBAO+"), HBAOPLUSCHIP_GUID, CHIP_GUID);


HBAOPlusChip::HBAOPlusChip()
{
	static bool init = []() {
		GFSDK_SSAO_CustomHeap CustomHeap;
		CustomHeap.new_ = ::operator new;
		CustomHeap.delete_ = ::operator delete;
		return true;
	}();

	CREATE_CHILD(0, GRAPHICSRESOURCECHIP_GUID, false, UP, MTEXT("Depth Texture"));
	CREATE_CHILD(1, GRAPHICSRESOURCECHIP_GUID, false, UP, MTEXT("Second Depth Texture [optional]"));
	CREATE_CHILD(2, GRAPHICSRESOURCECHIP_GUID, false, UP, MTEXT("Normal Map [optional]"));
	CREATE_CHILD(3, GRAPHICSRESOURCECHIP_GUID, false, DOWN, MTEXT("Render Target"));
	CREATE_CHILD(4, MATRIXCHIP_GUID, false, UP, MTEXT("Projection Matrix"));
	CREATE_CHILD(5, MATRIXCHIP_GUID, false, UP, MTEXT("View Matrix [if normal map provided only]"));
}

HBAOPlusChip::~HBAOPlusChip()
{
	if (_hbao) {
		_hbao->Release();
		_hbao = nullptr;
		_dt = nullptr;
		_rtvdt = nullptr;
	}
}

bool HBAOPlusChip::CopyChip(Chip* chip)
{
	HBAOPlusChip* c = dynamic_cast<HBAOPlusChip*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_params = _params;
	return true;
}

bool HBAOPlusChip::LoadChip(DocumentLoader& loader)
{
	B_RETURN(Chip::LoadChip(loader));
	LOADDEF("radius", _params.Radius, 1.0f);
	LOADDEF("bias", _params.Bias, 0.1f);
	LOADDEF("smallScaleAO", _params.SmallScaleAO, 1.0f);
	LOADDEF("largeScaleAO", _params.LargeScaleAO, 1.0f);
	LOADDEF("powerExponent", _params.PowerExponent, 2.0f);
	LOADDEF("use8Steps", _params.Use8Steps, false);
	LOADDEF("depthThresholdEnable", _params.DepthThresholdEnable, false);
	LOADDEF("depthThresholdMaxViewDepth", _params.DepthThresholdMaxViewDepth, 0.0f);
	LOADDEF("depthThresholdSharpness", _params.DepthThresholdSharpness, 100.0f);
	LOADDEF("blurEnable", _params.BlurEnable, true);
	LOADDEF("blurRadiusLarge", _params.BlurRadiusLarge, true);
	LOADDEF("blurSharpness", _params.BlurSharpness, 16.0f);
	LOADDEF("blurSharpnessProfileEnable", _params.BlurSharpnessProfileEnable, false);
	LOADDEF("blurSharpnessProfileForegroundSharpnessScale", _params.BlurSharpnessProfileForegroundSharpnessScale, 4.0f);
	LOADDEF("blurSharpnessProfileForegroundViewDepth", _params.BlurSharpnessProfileForegroundViewDepth, 0.0f);
	LOADDEF("blurSharpnessProfileBackgroundViewDepth", _params.BlurSharpnessProfileBackgroundViewDepth, 1.0f);
	LOADDEF("blendMode", (uint32&)_params.BlendMode, (uint32)OVERWRITE_RGB);
	return true;
}

bool HBAOPlusChip::SaveChip(DocumentSaver& saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVEDEF("radius", _params.Radius, 1.0f);
	SAVEDEF("bias", _params.Bias, 0.1f);
	SAVEDEF("smallScaleAO", _params.SmallScaleAO, 1.0f);
	SAVEDEF("largeScaleAO", _params.LargeScaleAO, 1.0f);
	SAVEDEF("powerExponent", _params.PowerExponent, 2.0f);
	SAVEDEF("use8Steps", _params.Use8Steps, false);
	SAVEDEF("depthThresholdEnable", _params.DepthThresholdEnable, false);
	SAVEDEF("depthThresholdMaxViewDepth", _params.DepthThresholdMaxViewDepth, 0.0f);
	SAVEDEF("depthThresholdSharpness", _params.DepthThresholdSharpness, 100.0f);
	SAVEDEF("blurEnable", _params.BlurEnable, true);
	SAVEDEF("blurRadiusLarge", _params.BlurRadiusLarge, true);
	SAVEDEF("blurSharpness", _params.BlurSharpness, 16.0f);
	SAVEDEF("blurSharpnessProfileEnable", _params.BlurSharpnessProfileEnable, false);
	SAVEDEF("blurSharpnessProfileForegroundSharpnessScale", _params.BlurSharpnessProfileForegroundSharpnessScale, 4.0f);
	SAVEDEF("blurSharpnessProfileForegroundViewDepth", _params.BlurSharpnessProfileForegroundViewDepth, 0.0f);
	SAVEDEF("blurSharpnessProfileBackgroundViewDepth", _params.BlurSharpnessProfileBackgroundViewDepth, 1.0f);
	SAVEDEF("blendMode", (uint32)_params.BlendMode, (uint32)OVERWRITE_RGB);
	return true;
}

void HBAOPlusChip::OnDestroyDevice()
{
	if (_hbao) {
		_hbao->Release();
		_hbao = nullptr;
		_dt = nullptr;
		_rtvdt = nullptr;
	}
}

String _GFSDK_SSAO_StatusToString(GFSDK_SSAO_Status status)
{
	String r;
	switch (status) 
	{
	case GFSDK_SSAO_OK: break;
	case GFSDK_SSAO_NULL_ARGUMENT:								 r = MTEXT("One of the required argument pointers is NULL."); break;
	case GFSDK_SSAO_VERSION_MISMATCH:							 r = MTEXT("The header version number does not match the DLL version number."); break;
	case GFSDK_SSAO_INVALID_PROJECTION_MATRIX:                   r = MTEXT("The projection matrix is not valid."); break;
	case GFSDK_SSAO_INVALID_WORLD_TO_VIEW_MATRIX:                r = MTEXT("The world-to-view matrix is not valid (transposing it may help)."); break;
	case GFSDK_SSAO_INVALID_NORMAL_TEXTURE_RESOLUTION:           r = MTEXT("The normal-texture resolution does not match the depth-texture resolution."); break;
	case GFSDK_SSAO_INVALID_NORMAL_TEXTURE_SAMPLE_COUNT:         r = MTEXT("The normal-texture sample count does not match the depth-texture sample count."); break;
	case GFSDK_SSAO_INVALID_VIEWPORT_DIMENSIONS:                 r = MTEXT("One of the viewport dimensions (width or height) is 0."); break;
	case GFSDK_SSAO_INVALID_VIEWPORT_DEPTH_RANGE:                r = MTEXT("The viewport depth range is not a sub-range of [0.f,1.f]."); break;
	case GFSDK_SSAO_INVALID_SECOND_DEPTH_TEXTURE_RESOLUTION:     r = MTEXT("The resolution of the second depth texture does not match the one of the first depth texture."); break;
	case GFSDK_SSAO_INVALID_SECOND_DEPTH_TEXTURE_SAMPLE_COUNT:   r = MTEXT("The sample count of the second depth texture does not match the one of the first depth texture."); break;
	case GFSDK_SSAO_MEMORY_ALLOCATION_FAILED:                    r = MTEXT("Failed to allocate memory on the heap."); break;
	case GFSDK_SSAO_INVALID_DEPTH_STENCIL_RESOLUTION:            r = MTEXT("The depth-stencil resolution does not match the output render-target resolution."); break;
	case GFSDK_SSAO_INVALID_DEPTH_STENCIL_SAMPLE_COUNT:          r = MTEXT("The depth-stencil sample count does not match the output render-target sample count."); break;
	case GFSDK_SSAO_D3D_FEATURE_LEVEL_NOT_SUPPORTED:             r = MTEXT("The current D3D11 feature level is lower than 11_0."); break;
	case GFSDK_SSAO_D3D_RESOURCE_CREATION_FAILED:                r = MTEXT("A resource-creation call has failed (running out of memory?)."); break;
	case GFSDK_SSAO_D3D12_UNSUPPORTED_DEPTH_CLAMP_MODE:          r = MTEXT("CLAMP_TO_BORDER is used (implemented on D3D11 & GL, but not on D3D12)."); break;
	case GFSDK_SSAO_D3D12_INVALID_HEAP_TYPE:                     r = MTEXT("One of the heaps provided to GFSDK_SSAO_CreateContext_D3D12 has an unexpected type."); break;
	case GFSDK_SSAO_D3D12_INSUFFICIENT_DESCRIPTORS:              r = MTEXT("One of the heaps provided to GFSDK_SSAO_CreateContext_D3D12 has an insufficient number of descriptors."); break;
	case GFSDK_SSAO_D3D12_INVALID_NODE_MASK:                     r = MTEXT("NodeMask has more than one bit set. HBAO+ only supports operation on one D3D12 device node."); break;
	case GFSDK_SSAO_NO_SECOND_LAYER_PROVIDED:                    r = MTEXT("FullResDepthTexture2ndLayerSRV is not set, but DualLayerAO is enabled."); break;
	default: r = MTEXT("Unknown status."); break;
	}
	return r;
}

void HBAOPlusChip::CallChip()
{
	try {
		if (!_hbao) {
			GFSDK_SSAO_DescriptorHeaps_D3D12 heaps;
			GetHeapManager()->GetDescriptorTable(GFSDK_SSAO_NUM_DESCRIPTORS_CBV_SRV_UAV_HEAP_D3D12, &_dt);
			GetRTVHeapManager()->GetDescriptorTable(GFSDK_SSAO_NUM_DESCRIPTORS_RTV_HEAP_D3D12, &_rtvdt);
			heaps.CBV_SRV_UAV.pDescHeap = GetHeapManager()->GetHeap();
			heaps.CBV_SRV_UAV.BaseIndex = _dt->offset;
			heaps.RTV.pDescHeap = GetRTVHeapManager()->GetHeap();
			heaps.RTV.BaseIndex = _rtvdt->offset;
			GFSDK_SSAO_Status r = GFSDK_SSAO_CreateContext_D3D12(device(), 1, heaps, &_hbao);
			if (r != GFSDK_SSAO_OK)
				throw GraphicsException(this, MTEXT("GFSDK_SSAO_CreateContext_D3D12(...) failed: ") + _GFSDK_SSAO_StatusToString(r));
		}

		//    * The input depth & normal textures are assumed to have state D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE.
		//    * The output AO render target is assumed to have state D3D12_RESOURCE_STATE_RENDER_TARGET.

		ChildPtr<GraphicsResourceChip> ch0 = GetChild(0);
		if (!ch0)
			throw MissingChildException(this, 0);
		
		ch0->UpdateChip();
		ID3D12Resource* depthResource = ch0->GetResource();
		const Descriptor& depthSRV = ch0->GetShaderResourceViewDescriptor();
		{
			SResourceStateTracker rst;
			GetResourceStateTracker(depthResource, &rst);
			if (rst)
				rs()->EnterState(rst, depthSRV.srv, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); // Can throw. Release lock immediately.
		}

		ChildPtr<GraphicsResourceChip> ch1 = GetChild(1);
		ID3D12Resource* secondDepthResource = nullptr;
		const Descriptor* secondDepthSRV = nullptr;
		if (ch1) {
			ch1->UpdateChip();
			secondDepthResource = ch1->GetResource();
			secondDepthSRV = &ch1->GetShaderResourceViewDescriptor();
			{
				SResourceStateTracker rst;
				GetResourceStateTracker(secondDepthResource, &rst);
				if (rst)
					rs()->EnterState(rst, secondDepthSRV->srv, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); // Can throw. Release lock immediately.
			}
		}

		ChildPtr<GraphicsResourceChip> ch2 = GetChild(2);
		ID3D12Resource* normalMapResource = nullptr;
		const Descriptor* normalMapSRV = nullptr;
		XMFLOAT4X4 vMatrix;
		if (ch2) {
			ch2->UpdateChip();
			normalMapResource = ch2->GetResource();
			normalMapSRV = &ch2->GetShaderResourceViewDescriptor();
			{
				SResourceStateTracker rst;
				GetResourceStateTracker(secondDepthResource, &rst);
				if (rst)
					rs()->EnterState(rst, normalMapSRV->srv, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); // Can throw. Release lock immediately.
			}
			ChildPtr<MatrixChip> ch5 = GetChild(5);
			vMatrix = ch5 ? ch5->GetMatrix() : MatrixChip::IDENTITY;
		}

		ChildPtr<GraphicsResourceChip> ch3 = GetChild(3);
		if (!ch3)
			throw MissingChildException(this, 3);

		ch3->UpdateChip();
		ID3D12Resource* targetResource = ch3->GetResource();
		{
			if (dxgiformat::IsTypeless(targetResource->GetDesc().Format))
				throw GraphicsException(this, MTEXT("Typeless formats for the render target is currently not allowed!"));
		}
		const Descriptor& rtvDesc = ch3->GetRenderTargetViewDescriptor();
		{
			SResourceStateTracker rst;
			GetResourceStateTracker(targetResource, &rst);
			if (rst)
				rs()->EnterState(rst, rtvDesc.rtv, D3D12_RESOURCE_STATE_RENDER_TARGET); // Can throw. Release lock immediately.
		}

		ChildPtr<MatrixChip> ch4 = GetChild(4);
		XMFLOAT4X4 pMatrix = ch4 ? ch4->GetMatrix() : MatrixChip::IDENTITY;

		GFSDK_SSAO_InputData_D3D12 InputData;
		{
			InputData.DepthData.MetersToViewSpaceUnits = 1.0f;
			InputData.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;
			std::memcpy(InputData.DepthData.ProjectionMatrix.Data.Array, &pMatrix, sizeof(FLOAT) * 16);

			InputData.DepthData.FullResDepthTextureSRV.pResource = depthResource;
			InputData.DepthData.FullResDepthTextureSRV.GpuHandle = depthSRV.GetGPUHandle().ptr;

			if (secondDepthResource) {
				InputData.DepthData.FullResDepthTexture2ndLayerSRV.pResource = secondDepthResource;
				InputData.DepthData.FullResDepthTexture2ndLayerSRV.GpuHandle = secondDepthSRV->GetGPUHandle().ptr;
			}
			if (normalMapResource) {
				InputData.NormalData.Enable = true;
				InputData.NormalData.DecodeBias = 0.0f;
				InputData.NormalData.DecodeScale = 1.0f;
				InputData.NormalData.FullResNormalTextureSRV.pResource = normalMapResource;
				InputData.NormalData.FullResNormalTextureSRV.GpuHandle = normalMapSRV->GetGPUHandle().ptr;
				InputData.NormalData.WorldToViewMatrix;
			}
			else
				InputData.NormalData.Enable = false;
		}

		GFSDK_SSAO_Parameters Parameters;
		{
			Parameters.Radius = _params.Radius;
			Parameters.Bias = _params.Bias;
			Parameters.SmallScaleAO = _params.SmallScaleAO;
			Parameters.LargeScaleAO = _params.LargeScaleAO;
			Parameters.PowerExponent = _params.PowerExponent;
			Parameters.StepCount = _params.Use8Steps ? GFSDK_SSAO_STEP_COUNT_8 : GFSDK_SSAO_STEP_COUNT_4;
			Parameters.DepthStorage = GFSDK_SSAO_FP16_VIEW_DEPTHS;
			Parameters.DepthClampMode = _params.DepthClampToBorder ? GFSDK_SSAO_CLAMP_TO_BORDER : GFSDK_SSAO_CLAMP_TO_EDGE;
			Parameters.DepthThreshold.Enable = _params.DepthThresholdEnable;
			Parameters.DepthThreshold.MaxViewDepth = _params.DepthThresholdMaxViewDepth;
			Parameters.DepthThreshold.Sharpness = _params.DepthThresholdSharpness;
			Parameters.Blur.Enable = _params.BlurEnable;
			Parameters.Blur.Radius = _params.BlurRadiusLarge ? GFSDK_SSAO_BLUR_RADIUS_4 : GFSDK_SSAO_BLUR_RADIUS_2;
			Parameters.Blur.Sharpness = _params.BlurSharpness;
			Parameters.Blur.SharpnessProfile.Enable = _params.BlurSharpnessProfileEnable;
			Parameters.Blur.SharpnessProfile.BackgroundViewDepth = _params.BlurSharpnessProfileBackgroundViewDepth;
			Parameters.Blur.SharpnessProfile.ForegroundViewDepth = _params.BlurSharpnessProfileForegroundViewDepth;
			Parameters.Blur.SharpnessProfile.ForegroundSharpnessScale = _params.BlurSharpnessProfileForegroundSharpnessScale;
			Parameters.EnableDualLayerAO = secondDepthResource != nullptr;
		}

		GFSDK_SSAO_Output_D3D12 Output;
		GFSDK_SSAO_RenderTargetView_D3D12 rtv;
		{
			Output.Blend.Mode = (GFSDK_SSAO_BlendMode)_params.BlendMode;
			rtv.pResource = targetResource;
			rtv.CpuHandle = rtvDesc.GetCPUHandle().ptr;
			Output.pRenderTargetView = &rtv;
		}

		rs()->CommitResourceBarriers();

		GFSDK_SSAO_Status r = _hbao->RenderAO(graphics()->GetCommandQueue(), rs()->GetCommandList1(), InputData, Parameters, Output);
		if (r != GFSDK_SSAO_OK)
			throw GraphicsException(this, MTEXT("RenderAO(...) failed: ") + _GFSDK_SSAO_StatusToString(r));
	}
	catch (const ChipException & e)
	{
		AddException(e);
	}

	rs()->ClearCachedState();
}

void HBAOPlusChip::SetParams(const Params& p)
{
	_params = p;
}