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

class GFSDK_SSAO_Context_D3D12;

namespace m3d
{


static const Guid HBAOPLUSCHIP_GUID = { 0x87605a8a, 0xfda7, 0x4ace, { 0x81, 0xc7, 0x4a, 0xd2, 0x43, 0x63, 0x26, 0x54 } };


class GRAPHICSCHIPS_API HBAOPlusChip : public Chip, protected GraphicsUsage
{
	CHIPDESC_DECL;
public:
	enum EBlendMode
	{
		OVERWRITE_RGB,                               // Overwrite the destination RGB with the AO, preserving alpha
		MULTIPLY_RGB,                                // Multiply the AO over the destination RGB, preserving alpha
		//GFSDK_SSAO_CUSTOM_BLEND,                                // Composite the AO using a custom blend state
	};

	// params
	struct Params
	{
		float Radius = 1.0f;                     // The AO radius in meters
		float Bias = 0.1f;                       // To hide low-tessellation artifacts // 0.0~0.5
		float SmallScaleAO = 1.0f;               // Scale factor for the small-scale AO, the greater the darker // 0.0~2.0
		float LargeScaleAO = 1.0f;               // Scale factor for the large-scale AO, the greater the darker // 0.0~2.0
		float PowerExponent = 2.0f;              // The final AO output is pow(AO, powerExponent) // 1.0~4.0
		bool Use8Steps = false;                  // The number of steps per direction in the AO-generation pass
		bool DepthClampToBorder = false;		 // To hide possible false-occlusion artifacts near screen borders

		bool DepthThresholdEnable = false;                    // To return white AO for ViewDepths > MaxViewDepth
		float DepthThresholdMaxViewDepth = 0.0f;              // Custom view-depth threshold
		float DepthThresholdSharpness = 100.0f;               // The higher, the sharper are the AO-to-white transitions

		bool BlurEnable = true;
		bool BlurRadiusLarge = true;
		float BlurSharpness = 16.f;

		// When enabled, the actual per-pixel blur sharpness value depends on the per-pixel view depth with:
		//     LerpFactor = (PixelViewDepth - ForegroundViewDepth) / (BackgroundViewDepth - ForegroundViewDepth)
		//     Sharpness = lerp(Sharpness*ForegroundSharpnessScale, Sharpness, saturate(LerpFactor))
		bool BlurSharpnessProfileEnable = false;                         // To make the blur sharper in the foreground
		float BlurSharpnessProfileForegroundSharpnessScale = 4.0f;       // Sharpness scale factor for ViewDepths <= ForegroundViewDepth
		float BlurSharpnessProfileForegroundViewDepth = 0.0f;            // Maximum view depth of the foreground depth range
		float BlurSharpnessProfileBackgroundViewDepth = 1.0f;            // Minimum view depth of the background depth range
	
		EBlendMode BlendMode = OVERWRITE_RGB;
	};

	HBAOPlusChip();
	~HBAOPlusChip();

	bool CopyChip(Chip* chip) override;
	bool LoadChip(DocumentLoader& loader) override;
	bool SaveChip(DocumentSaver& saver) const override;

	void OnDestroyDevice() override;

	void CallChip() override;

	const Params& GetParameters() const { return _params; }
	void SetParams(const Params& p);

protected:
	GFSDK_SSAO_Context_D3D12 *_hbao = nullptr;
	SDescriptorTable _dt;
	SDescriptorTable _rtvdt;

	Params _params;

};

}