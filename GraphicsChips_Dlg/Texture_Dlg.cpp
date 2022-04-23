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

#include "stdafx.h"
#include "Texture_Dlg.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/RenderWindow.h"
#include "ChipDialogs/ChipDialogManager.h"
#include "M3DEngine/RenderWindow.h"
#include "GraphicsChips/D3D12Formats.h"
#include "TextureResampleDialog.h"
#include "GraphicsChips/Graphics.h"
#include "TextureUtil.h"
#include "M3DCore/Util.h"
#include "M3DCore/MagicEnum.h"

using namespace m3d;


DIALOGDESC_DEF(Texture_Dlg, TEXTURE_GUID);


struct FormatSupport
{
	D3D12_FORMAT_SUPPORT1 Support1;
	D3D12_FORMAT_SUPPORT2 Support2;
	UINT QualityLevels[D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT];

};
List<FormatSupport> formatSupport;
UINT maxQualityLevel = 0;
bool formatSupportSet = false;

Texture_Dlg::Texture_Dlg()
{
	ui.setupUi(this);

	_skipSlots = false;

	ID3D12Device *device = ((Graphics*)engine->GetGraphics())->GetDevice();// GetChip()->graphics()->GetDevice();
	
	if (!formatSupportSet) {
		formatSupport.resize(UINT(dxgiformat::PrevFormat(DXGI_FORMAT_UNKNOWN)) + 1); // Find last format to reserve array capacity.
		memset(&formatSupport.front(), 0, sizeof(FormatSupport) * formatSupport.size());

		for (DXGI_FORMAT format = dxgiformat::NextFormat(DXGI_FORMAT_UNKNOWN); format != DXGI_FORMAT_UNKNOWN; format = dxgiformat::NextFormat(format)) {
			D3D12_FEATURE_DATA_FORMAT_SUPPORT support = { format, D3D12_FORMAT_SUPPORT1(), D3D12_FORMAT_SUPPORT2() };
			HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &support, sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT));
			if (FAILED(hr))
				continue;
			formatSupport[format].Support1 |= support.Support1;
			formatSupport[format].Support2 |= support.Support2;
			for (uint32 j = 1; j <= D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT; j++) {
				D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS qLevels = { format, j, D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE, 0 };
				hr = device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &qLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
				formatSupport[format].QualityLevels[j - 1] = SUCCEEDED(hr) ? qLevels.NumQualityLevels : 0;
				maxQualityLevel = qMax(formatSupport[format].QualityLevels[j - 1], maxQualityLevel);
			}
			// Do this to give typeless formats the possible options when being a typed view..
			DXGI_FORMAT typeless = dxgiformat::MakeTypeless(format);
			if (typeless != format) {
				formatSupport[typeless].Support1 |= formatSupport[format].Support1;
				formatSupport[typeless].Support2 |= formatSupport[format].Support2;
			}
		}
		formatSupportSet = true;
	}	
	for (DXGI_FORMAT format = dxgiformat::NextFormat(DXGI_FORMAT_UNKNOWN); format != DXGI_FORMAT_UNKNOWN; format = dxgiformat::NextFormat(format)) {
		const Char *str = dxgiformat::ToString(format);
		if (str) {
			ui.comboBox_format->addItem(dxgiformat::ToString(format), format);
			ui.comboBox_format->setItemData(ui.comboBox_format->count() - 1, dxgiformat::ToCommentString(format), Qt::ToolTipRole);
		}
	}
}

Texture_Dlg::~Texture_Dlg()
{
}

void Texture_Dlg::Init()
{
	_initDesc = GetChip()->GetInitDesc();
	_initImageData = GetChip()->GetImageData();
	_initImageDataFileFormat = GetChip()->GetImageDataFileFormat();
/*
	if (GetChip()->HasImageData() && !GetChip()->HasResource()) {
		DirectX::TexMetadata meta;
		TextureDesc desc = _initDesc;
		if (SUCCEEDED(TextureUtil::GetMetainfo(_initImageData, _initImageDataFileFormat, meta))) {
			desc.ArraySize = meta.arraySize;
			desc.Width = meta.width;
			desc.Height = meta.height;
			desc.Depth = meta.depth;
			desc.Format = meta.format;
			desc.MipLevels = meta.mipLevels;
			desc.SampleDesc = {1, 0};
			desc.FlagsEx = 0;
		}
		_setDesc(desc, true);
	}
	else*/
		_setDesc(_initDesc, GetChip()->HasImageData());

	Update();
}

void Texture_Dlg::OnOK()
{
	if (_desc != _initDesc) {
		GetChip()->SetInitDesc(_desc);
		SetDirty();
	}
}

void Texture_Dlg::OnCancel()
{
	if (IsEdited()) {
		GetChip()->SetInitDesc(_initDesc);
		GetChip()->SetImageData(std::move(_initImageData), _initImageDataFileFormat);
		SetDirty();
	}
}

void Texture_Dlg::OnApply() 
{ 
	OnOK();
	_initDesc = GetChip()->GetInitDesc();
	_initImageData = GetChip()->GetImageData();
	_initImageDataFileFormat = GetChip()->GetImageDataFileFormat();
}

void Texture_Dlg::Update() 
{
	try {
		// Update texture view.
		SID3D12Resource r;
		if (GetChip()->HasResource())
			r = GetChip()->GetResource();
		else if (GetChip()->HasImageData()) {
			GetChip()->UpdateChip();
			r = GetChip()->GetResource();
			TextureDesc td = GetChip()->GetInitDesc();
			td.Flags = _desc.Flags;
			td.FlagsEx = _desc.FlagsEx;
			_setDesc(td, true);
		}
		if (r != ui.previewWidget->GetTexture()) {
			ui.previewWidget->SetTexture(r, (DXGI_FORMAT)GetChip()->GetSRVDesc().format);
		}
		ui.previewWidget->update();
	}
	catch (const GraphicsException &e) 
	{
	}
}

void Texture_Dlg::onWidthChanged()
{
	if (_skipSlots) return;
	bool ok;
	uint32 i = ui.lineEdit_width->text().toUInt(&ok);
	if (!ok)
		return;
	if (_desc.Width == i)
		return;
	_desc.Width = qMin(qMax(i, 1u), UINT(_desc.Depth > 1 ? D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION : D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION));
	if (_desc.FlagsEx & TEXTURE_USE_BACKBUFFER_SIZE_BY_PERCENT) 
		_desc.Width = qMin(_desc.Width, 1000Ui64);
	_desc.MipLevels = qMin(UINT16(log2(qMax(UINT(_desc.Width), _desc.Height)) / log2(2) + 1), _desc.MipLevels);
	_updateUI();
}

void Texture_Dlg::onHeightChanged()
{
	if (_skipSlots) return;
	bool ok;
	uint32 i = ui.lineEdit_height->text().toUInt(&ok);
	if (!ok)
		return;
	if (_desc.Height == i)
		return;
	_desc.Height = qMin(qMax(i, 1u), UINT(_desc.Depth > 1 ? D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION : D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION));
	if (_desc.FlagsEx & TEXTURE_USE_BACKBUFFER_SIZE_BY_PERCENT) 
		_desc.Height = qMin(_desc.Height, 1000u);
	_desc.MipLevels = qMin(UINT16(log2(qMax(UINT(_desc.Width), _desc.Height)) / log2(2) + 1), _desc.MipLevels);
	_updateUI();
}

void Texture_Dlg::onDepthChanged()
{
	if (_skipSlots) return;
	bool ok;
	uint32 i = ui.lineEdit_depth->text().toUInt(&ok);
	if (!ok)
		return;
	if (_desc.Depth == i)
		return;
	_desc.Depth = qMin(qMax(i, 1u), UINT(D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION));
	if (_desc.Depth > 1) {
		_desc.Width = qMin(_desc.Width, UINT64(D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION));
		_desc.Height = qMin(_desc.Height, UINT(D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION));
		_desc.ArraySize = 1; // Array of 3d-tex not allowed!
	}
	_updateUI();
}

void Texture_Dlg::onUseBBSizeChanged()
{
	if (_skipSlots) return;
	if (ui.checkBox_useBBSize->isChecked()) {
		_desc.FlagsEx |= TEXTURE_USE_BACKBUFFER_SIZE_BY_PERCENT;
		_desc.Width = 100Ui64;//qMin(_desc.Width, 1000Ui64);
		_desc.Height = 100u;//qMin(_desc.Height, 1000u);
	}
	else
		_desc.FlagsEx &= ~TEXTURE_USE_BACKBUFFER_SIZE_BY_PERCENT;
	_updateUI();
}


void Texture_Dlg::onFormatChanged()
{
	if (_skipSlots) return;
	if (!(_desc.FlagsEx & TEXTURE_USE_BACKBUFFER_FORMAT)) {
		if (ui.comboBox_format->currentIndex() != -1)
			_desc.Format = (M3D_FORMAT)ui.comboBox_format->currentData().toUInt();
		uint32 qlevels = formatSupport[_desc.Format].QualityLevels[_desc.SampleDesc.Count - 1];
		if (qlevels == 0) 
			_desc.SampleDesc = { 1, 0 };
		else
			_desc.SampleDesc.Quality = qMin(_desc.SampleDesc.Quality, qlevels - 1);
		if (!(formatSupport[_desc.Format].Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE2D))
			_desc.Height = 1;
		if (!(formatSupport[_desc.Format].Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE3D))
			_desc.Depth = 1;
		if (!(formatSupport[_desc.Format].Support1 & D3D12_FORMAT_SUPPORT1_MIP)) 
			_desc.MipLevels = 1; // Mips not supported!
		if (!(formatSupport[_desc.Format].Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET)) 
			_desc.Flags = _desc.Flags & ~M3D_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // RTV not supported!
		if (!(formatSupport[_desc.Format].Support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL)) 
			_desc.Flags = _desc.Flags & ~M3D_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DSV not supported!
		if (!(formatSupport[_desc.Format].Support1 & D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW)) 
			_desc.Flags = _desc.Flags & ~M3D_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; // DSV not supported!
//		if (!(formatSupport[_desc.Format].Support1 & (D3D12_FORMAT_SUPPORT1_SHADER_LOAD | D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE | D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE_COMPARISON | D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE_MONO_TEXT | D3D12_FORMAT_SUPPORT1_SHADER_GATHER | D3D12_FORMAT_SUPPORT1_SHADER_GATHER_COMPARISON)))
//			_desc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE; // SRV not supported! (Not sure about this.. D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE is only for depth buffers I think..
	}
	_updateUI();
}

void Texture_Dlg::onUseBBFormatChanged()
{
	if (_skipSlots) return;
	if (ui.checkBox_useBBFmt->isChecked())
		_desc.FlagsEx |= TEXTURE_USE_BACKBUFFER_FORMAT;
	else
		_desc.FlagsEx &= ~TEXTURE_USE_BACKBUFFER_FORMAT;
	onFormatChanged();
}

void Texture_Dlg::onCountQualityChanged()
{
	if (_skipSlots) return;
	if (!(_desc.FlagsEx & TEXTURE_USE_BACKBUFFER_FORMAT)) {
		if (ui.comboBox_msCount->currentIndex() != -1)
			_desc.SampleDesc.Count = ui.comboBox_msCount->currentData().toUInt();
		if (ui.comboBox_msQuality->currentIndex() != -1)
			_desc.SampleDesc.Quality = ui.comboBox_msQuality->currentData().toUInt();
		uint32 qlevels = formatSupport[_desc.Format].QualityLevels[_desc.SampleDesc.Count - 1];
		if (qlevels == 0) 
			_desc.SampleDesc = { 1, 0 };
		else
			_desc.SampleDesc.Quality = qMin(_desc.SampleDesc.Quality, qlevels - 1);
	}
	_updateUI();
}

void Texture_Dlg::onMipLevelsChanged()
{
	if (_skipSlots) return;
	_desc.MipLevels = ui.comboBox_mipLevels->currentData().toUInt();
	_updateUI();
}

void Texture_Dlg::onArraySizeChanged()
{
	if (_skipSlots) return;
	bool ok;
	uint32 i = ui.lineEdit_arraySize->text().toUInt(&ok);
	if (!ok)
		return;
	if (_desc.ArraySize == i)
		return;
	_desc.ArraySize = i;
	_desc.Depth = 1; // Array of 3d-textures not supported!
	_updateUI();
}

void Texture_Dlg::onSRVChanged()
{
	if (_skipSlots) return;
	if (ui.checkBox_srv->isChecked())
		_desc.Flags	&= ~M3D_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
	else
		_desc.Flags	|= M3D_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
	_updateUI();
}

void Texture_Dlg::onRTVChanged()
{
	if (_skipSlots) return;
	if (ui.checkBox_rtv->isChecked()) {
		_desc.Flags	|= M3D_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		_desc.Flags	&= ~M3D_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}
	else
		_desc.Flags	&= ~M3D_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	_updateUI();
}

void Texture_Dlg::onDSVChanged()
{
	if (_skipSlots) return;
	if (ui.checkBox_dsv->isChecked()) {
		_desc.Flags	|= M3D_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		_desc.Flags	&= ~M3D_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}
	else {
		_desc.Flags	&= ~M3D_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		_desc.Flags &= ~M3D_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
	}
	_updateUI();
}

void Texture_Dlg::onUAVChanged()
{
	if (_skipSlots) return;
	if (ui.checkBox_uav->isChecked()) {
		_desc.Flags	|= M3D_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}
	else
		_desc.Flags	&= ~M3D_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	_updateUI();
}

void Texture_Dlg::onGenMipsChanged()
{
	if (_skipSlots) return;
	if (ui.checkBox_genMips->isChecked())
		_desc.FlagsEx |= TEXTURE_GEN_MIPMAPS;
	else
		_desc.FlagsEx &= ~TEXTURE_GEN_MIPMAPS;
	_updateUI();
}

void Texture_Dlg::onForceSRGBChanged()
{
	if (_skipSlots) return;
	if (ui.checkBox_forceSRGB->isChecked())
		_desc.FlagsEx |= TEXTURE_FORCE_SRGB;
	else
		_desc.FlagsEx &= ~TEXTURE_FORCE_SRGB;
	_updateUI();
}


template<typename T>
struct AutoReset
{
	T &t;
	T init;
	AutoReset(T &t, T s) : t(t), init(t) { t = s; }
	~AutoReset() { t = init; }
};

void Texture_Dlg::_updateUI()
{
	AutoReset<bool> as(_skipSlots, true);

	ui.frame_settings->setEnabled(true);

	ui.checkBox_useBBSize->setChecked(_desc.FlagsEx & TEXTURE_USE_BACKBUFFER_SIZE_BY_PERCENT);
	ui.checkBox_useBBFmt->setChecked(_desc.FlagsEx & TEXTURE_USE_BACKBUFFER_FORMAT);
	
	ui.lineEdit_width->setText(QString::number(_desc.Width));
	if (ui.checkBox_useBBFmt->isChecked() || formatSupport[_desc.Format].Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE2D) {
		ui.lineEdit_height->setText(QString::number(_desc.Height));
		ui.lineEdit_height->setEnabled(true);
	}
	else {
		ui.lineEdit_height->setText("");
		ui.lineEdit_height->setEnabled(false);
	}
	if (ui.checkBox_useBBFmt->isChecked() || formatSupport[_desc.Format].Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE3D) {
		ui.lineEdit_depth->setText(QString::number(_desc.Depth));
		ui.lineEdit_depth->setEnabled(true);
	}
	else {
		ui.lineEdit_depth->setText("");
		ui.lineEdit_depth->setEnabled(false);
	}

	ui.comboBox_format->setCurrentIndex(ui.checkBox_useBBFmt->isChecked() ? -1 : ui.comboBox_format->findData(_desc.Format));
	ui.comboBox_format->setDisabled(ui.checkBox_useBBFmt->isChecked());
	ui.comboBox_msCount->clear();
	if (ui.comboBox_msCount->isEnabled()) {	
		for (uint32 i = 1; i <= D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT; i++) {
			if (ui.checkBox_useBBFmt->isChecked() || formatSupport[_desc.Format].QualityLevels[i - 1] > 0)
				ui.comboBox_msCount->addItem(QString::number(i), i);
		}
		ui.comboBox_msCount->setCurrentIndex(ui.comboBox_msCount->findData(_desc.SampleDesc.Count));
	}
	ui.comboBox_msQuality->clear();
	if (ui.comboBox_msQuality->isEnabled()) {
		UINT qLevels = 0;
		if (ui.checkBox_useBBFmt->isChecked())
			qLevels = maxQualityLevel;
		else
			qLevels = formatSupport[_desc.Format].QualityLevels[std::max(_desc.SampleDesc.Count, 1u) - 1];
		for (UINT i = 0; i < qLevels; i++)
			ui.comboBox_msQuality->addItem(QString::number(i), i);
		ui.comboBox_msQuality->setCurrentIndex(ui.comboBox_msQuality->findData(_desc.SampleDesc.Quality));
	}
	ui.comboBox_mipLevels->clear();
	ui.comboBox_mipLevels->setEnabled(formatSupport[_desc.Format].Support1 & D3D12_FORMAT_SUPPORT1_MIP);
	if (ui.comboBox_mipLevels->isEnabled()) {
		ui.comboBox_mipLevels->addItem("All", 0);
		ui.comboBox_mipLevels->addItem("Single", 1);
		for (uint32 i = 2, j = ui.checkBox_useBBSize->isChecked() ? D3D12_REQ_MIP_LEVELS : uint32(log2(qMax(UINT(_desc.Width), _desc.Height)) / log2(2) + 1); i <= j; i++)
			ui.comboBox_mipLevels->addItem(QString::number(i), i);
		ui.comboBox_mipLevels->setCurrentIndex(ui.comboBox_mipLevels->findData(_desc.MipLevels));
	}
	if (_desc.Depth == 1) {
		ui.lineEdit_arraySize->setText(QString::number(_desc.ArraySize));
		ui.lineEdit_arraySize->setEnabled(true);
	}
	else {
		ui.lineEdit_arraySize->setText("");
		ui.lineEdit_arraySize->setEnabled(false);
	}
	ui.checkBox_rtv->setEnabled(((formatSupport[_desc.Format].Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET)) && (_desc.SampleDesc.Count == 1 || (formatSupport[_desc.Format].Support1 & D3D12_FORMAT_SUPPORT1_MULTISAMPLE_RENDERTARGET)));
	ui.checkBox_rtv->setChecked(ui.checkBox_rtv->isEnabled() && (_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET));
	ui.checkBox_dsv->setEnabled(((formatSupport[_desc.Format].Support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL) || (_desc.FlagsEx & TEXTURE_USE_BACKBUFFER_FORMAT)));
	ui.checkBox_dsv->setChecked(ui.checkBox_dsv->isEnabled() && (_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL));
	ui.checkBox_srv->setEnabled(ui.checkBox_dsv->isChecked());
	ui.checkBox_srv->setChecked(!ui.checkBox_srv->isEnabled() || !(_desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE));
	ui.checkBox_uav->setEnabled(((formatSupport[_desc.Format].Support1 & D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW) || (_desc.FlagsEx & TEXTURE_USE_BACKBUFFER_FORMAT)));
	ui.checkBox_uav->setChecked(ui.checkBox_uav->isEnabled() && (_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS));
	ui.checkBox_genMips->setEnabled(_hasImage);
	ui.checkBox_genMips->setChecked(_desc.FlagsEx & TEXTURE_GEN_MIPMAPS);
	ui.checkBox_forceSRGB->setEnabled(_hasImage);
	ui.checkBox_forceSRGB->setChecked(_desc.FlagsEx & TEXTURE_FORCE_SRGB);

	ui.frame_settings->setEnabled(!_hasImage);

	if (_hasImage) {
		QString txt;
		switch (GetChip()->GetImageDataFileFormat())
		{
		case IFF_UNKNOWN: txt = "Unknown"; break;
		case IFF_DDS: txt = "DDS"; break;
		case IFF_BMP: txt = "BMP"; break;
		case IFF_JPG: txt = "JPG"; break;
		case IFF_PNG: txt = "PNG"; break;
		case IFF_TIFF: txt = "TIFF"; break;
		case IFF_GIF: txt = "GIF"; break;
		case IFF_WMP: txt = "WMP"; break;
		case IFF_TGA: txt = "TGA"; break;
		default: txt = "Invalid"; break;
		}
		size_t s = GetChip()->GetImageData().getBufferSize();
		if (s > 1024 * 1024)
			txt += " (" + QString::number(uint32(s / 1024 / 1024)) + " MB)";
		else if (s > 1024)
			txt += " (" + QString::number(uint32(s / 1024)) + " kB)";
		else
			txt += " (" + QString::number(uint32(s)) + "  Bytes)";
		ui.label_ImageInfo->setText("Format: " + txt);
	}
	else {
		ui.label_ImageInfo->setText("No Image Data");
	}
}

void Texture_Dlg::_setDesc(const TextureDesc &desc, bool hasImageData)
{
	_desc = desc;
	_hasImage = hasImageData;
	_updateUI();
	return;
}

void Texture_Dlg::loadImage()
{
	GetDialogManager()->DisableFrameTimer(true);
	QString filename = QFileDialog::getOpenFileName(this, "Load Image", TOQSTRING(GetDialogManager()->GetCurrentPath().AsString()), "Images (*.dds *.bmp *.jpg *.jpeg *.png *.tif *.tiff *.gif *.hdp *.jxr *.wdp *.tga *.hdr);;All Files (*.*)", 0, QFileDialog::Options());
	GetDialogManager()->DisableFrameTimer(false);

	if (filename.isEmpty())
		return; // Cancelled

	Path p(FROMQSTRING(filename));
	if (!p.IsFile())
		return; // Error

	GetDialogManager()->SetCurrentPath(p.GetDirectory());

	DataBuffer db;
	if (!LoadDataBuffer(p, db)) {
		QMessageBox::critical(this, "Texture", "Failed to load \'" + filename + "\' from disk.");
		return;
	}

	ImageFileFormat iff = GetImageFileFormat(p);

	DirectX::TexMetadata meta;

	if (FAILED(TextureUtil::GetMetainfo(db, iff, meta))) {
		QMessageBox::critical(this, "Texture", "Invalid texture file: \'" + filename + "\'.");
		return;
	}

	TextureConvertInfo nfo;
	nfo.width = meta.width;
	nfo.height = meta.height;
	nfo.mipLevels = meta.mipLevels;
	nfo.format = meta.format;
	nfo.filter = TEX_FILTER_DEFAULT;
	nfo.mipFilter = TEX_FILTER_DEFAULT;
	nfo.flipRotate = TEX_FR_ROTATE0;
	nfo.nmConvert = false;
	nfo.nmAmplitude = 1.0f;
	nfo.nmFlags = CNMAP_DEFAULT;
	nfo.forceDDSFormat = false;

	TextureResampleDialog *d = new TextureResampleDialog(this);

	int r = d->show(nfo, meta);
	d->deleteLater();
	if (r != QDialog::Accepted)
		return; // Cancelled

	if (FAILED(TextureUtil::Convert(db, iff, nfo))) {
		QMessageBox::critical(this, "Texture", "Failed to convert texture to your given settings.");
		return;
	}

	TextureDesc td = _desc;
	td.Width = nfo.width;
	td.Height = nfo.height;
	td.Depth = meta.depth;
	td.ArraySize = meta.arraySize;
	td.MipLevels = nfo.mipLevels;
	td.Format = (M3D_FORMAT)nfo.format;
	td.SampleDesc = {1, 0};
	td.Flags = M3D_RESOURCE_FLAG_NONE;
	if (nfo.mipLevels == 1)
		td.FlagsEx = TEXTURE_GEN_MIPMAPS;

	if (!GetChip()->SetImageData(std::move(db), iff, &td)) {
		QMessageBox::critical(this, "Texture", "Failed to set texture.");
		return;
	}

	_setDesc(GetChip()->GetInitDesc(), true); // Set desc to UI
	//Update();
	SetDirty();
}

void Texture_Dlg::resampleImage()
{
	DirectX::TexMetadata meta;
	if (FAILED(TextureUtil::GetMetainfo(GetChip()->GetImageData(), GetChip()->GetImageDataFileFormat(), meta)))
		return;

	TextureConvertInfo nfo;
	nfo.width = meta.width;
	nfo.height = meta.height;
	nfo.mipLevels = meta.mipLevels;
	nfo.format = meta.format;
	nfo.filter = TEX_FILTER_DEFAULT;
	nfo.mipFilter = TEX_FILTER_DEFAULT;
	nfo.flipRotate = TEX_FR_ROTATE0;
	nfo.nmConvert = false;
	nfo.nmAmplitude = 1.0f;
	nfo.nmFlags = CNMAP_DEFAULT;

	TextureResampleDialog *d = new TextureResampleDialog(this);
	if (d->show(nfo, meta) != QDialog::Accepted)
		return;
	d->deleteLater();

	DataBuffer db = GetChip()->GetImageData();
	ImageFileFormat iff = GetChip()->GetImageDataFileFormat();

	if (FAILED(TextureUtil::Convert(db, iff, nfo))) {
		QMessageBox::critical(this, "Texture", "Failed to convert texture to your given settings.");
		return;
	}

	if (!GetChip()->SetImageData(std::move(db), iff)) {
		QMessageBox::critical(this, "Texture", "Failed to set texture.");
		return;
	}

	GetChip()->SetInitDesc(_desc); // This only copies flags and clears the texture.
	try
	{
		GetChip()->GetResource(); // Updates texture.
	}
	catch (const ChipException & e)
	{
	}
_setDesc(GetChip()->GetInitDesc(), GetChip()->HasImageData()); // Set desc to UI
	SetDirty();
}

void Texture_Dlg::clearImage()
{
	GetChip()->ClearImageData();
	GetChip()->ClearResource();
	_hasImage = false;
	_updateUI();
}

void Texture_Dlg::saveImage()
{
	HRESULT hr;

	if (!GetChip()->HasImageData() && !GetChip()->HasResource())
		return;
	
	GetDialogManager()->DisableFrameTimer(true);
	QString filename = QFileDialog::getSaveFileName(this, "Save Image", TOQSTRING(GetDialogManager()->GetCurrentPath().AsString()), "DDS (*.dds);;BMP (*.bmp);;JPG (*.jpg);;PNG (*.png);;TIF (*.tif);;GIF (*.gif);;HDP (*.hdp);;JXR (*.jxr);;TGA (*.tga);;HDR (*.hdr)", 0, QFileDialog::Options());
	GetDialogManager()->DisableFrameTimer(false);

	if (filename.isEmpty())
		return;

	GetDialogManager()->SetCurrentPath(Path(FROMQSTRING(filename)).GetDirectory());

	QApplication::setOverrideCursor(Qt::WaitCursor);

	if (GetChip()->HasImageData()) { // Have image data? Use it!
		hr = TextureUtil::SaveToFile( GetChip()->GetImageData(), GetChip()->GetImageDataFileFormat(), Path(FROMQSTRING(filename)));
	}
	else {
		DirectX::ScratchImage image;
		hr = CaptureTexture(GetChip()->graphics()->GetCommandQueue(), GetChip()->GetResource(), false, image, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		if (SUCCEEDED(hr)) {
			hr = TextureUtil::SaveToFile(image, Path(FROMQSTRING(filename)));
		}
	}

	QApplication::restoreOverrideCursor();

	if (FAILED(hr)) {
		QMessageBox::critical(this, "Texture", "Failed to save texture.");
	}
}

void Texture_Dlg::saveTextureToData()
{
	if (!GetChip()->HasResource())
		return;

	QApplication::setOverrideCursor(Qt::WaitCursor);

	DirectX::ScratchImage image;
	HRESULT hr = CaptureTexture(GetChip()->graphics()->GetCommandQueue(), GetChip()->GetResource(), false, image);
	if (SUCCEEDED(hr)) {
		DirectX::Blob db;
		hr = TextureUtil::SaveToMemory(image, db, ImageFileFormat::IFF_DDS);
		if (SUCCEEDED(hr)) {
			DataBuffer db1((const uint8*)db.GetBufferPointer(), db.GetBufferSize());
			if (!GetChip()->SetImageData(std::move(db1), ImageFileFormat::IFF_DDS))
				hr = E_FAIL;
		}
	}

	QApplication::restoreOverrideCursor();

	if (FAILED(hr)) {
		QMessageBox::critical(this, "Texture", "Failed to save texture to image data.");
	}
}

void Texture_Dlg::saveToCArray()
{
	HRESULT hr;

	if (!GetChip()->HasImageData() && !GetChip()->HasResource())
		return;

	QApplication::setOverrideCursor(Qt::WaitCursor);

	DirectX::ScratchImage image;

	if (GetChip()->HasImageData()) { // Have image data? Use it!
		hr = TextureUtil::LoadFromDataBuffer(GetChip()->GetImageData(), GetChip()->GetImageDataFileFormat(), image);
	}
	else {
		hr = CaptureTexture(GetChip()->graphics()->GetCommandQueue(), GetChip()->GetResource(), false, image);
	}
	DirectX::Blob b;
	if (SUCCEEDED(hr)) {
		hr = TextureUtil::SaveToMemory(image, b, ImageFileFormat::IFF_DDS);
	}
	if (SUCCEEDED(hr)) {
		QString s = QString("const uint8 TEXTURE_DATA[%1] =\n{\n\t").arg((uint32)b.GetBufferSize());
		String t;
		const uint8 *C = (const uint8*)b.GetBufferPointer();
		for (size_t i = 0, j = b.GetBufferSize(); i < j;) {
			for (size_t k = std::min(i + 20, j); i < k; i++) {
				s += QString("0x%1, ").arg((uint32)C[i], 2, 16, QChar('0'));
			}
			if (i < j - 1)
				s += "\n\t";
		}
		s.remove(s.size() - 2, 2);
		s += "\n};";

		QApplication::clipboard()->setText(s);	
	}

	QApplication::restoreOverrideCursor();

	if (SUCCEEDED(hr)) {
		QMessageBox::information(this, "Texture", "Texture data as a C-array successfully copied to clipboard!");
	}
	else {
		QMessageBox::critical(this, "Texture", "Failed to save texture to C-array.");
	}
}


void Texture_Dlg::DestroyDeviceObject()
{
	ui.previewWidget->DestroyDeviceObject();
}
