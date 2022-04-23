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
#include "ChipDialogs/StandardDialogPage.h"
#include "GraphicsChips/Texture.h"
#include "ui_Texture_Dlg.h"

namespace m3d
{



class GRAPHICSCHIPS_DLG_EXPORT Texture_Dlg : public StandardDialogPage
{
	Q_OBJECT
	DIALOGDESC_DECL
protected:

public:
	Texture_Dlg();
	~Texture_Dlg();

	Texture*GetChip() { return dynamic_cast<Texture*>(DialogPage::GetChip()); }

	virtual void Init();
	
	virtual void OnCancel();
	virtual void OnOK();
	virtual void OnApply();
	
	virtual void Update();

	virtual void DestroyDeviceObject();

private:
	Ui::Texture_Dlg ui;
	
	// Initial values
	TextureDesc _initDesc;
	DataBuffer _initImageData;
	ImageFileFormat _initImageDataFileFormat;

	// Current ui settings.
	TextureDesc _desc;
	bool _hasImage;

	bool _skipSlots;

	void _setDesc(const TextureDesc &desc, bool hasImageData);
	//void _getDesc(TextureDesc &desc);
	void _updateUI();

private slots:
	void onWidthChanged();
	void onHeightChanged();
	void onDepthChanged();
	void onUseBBSizeChanged();
	void onFormatChanged();
	void onUseBBFormatChanged();
	void onCountQualityChanged();
	void onMipLevelsChanged();
	void onArraySizeChanged();
	void onSRVChanged();
	void onRTVChanged();
	void onDSVChanged();
	void onUAVChanged();
	void onGenMipsChanged();
	void onForceSRGBChanged();

	void loadImage();
	void saveImage();
	void saveTextureToData();
	void resampleImage();
	void clearImage();
	void saveToCArray();
};


}