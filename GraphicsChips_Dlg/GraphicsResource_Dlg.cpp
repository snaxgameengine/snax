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
#include "GraphicsResource_Dlg.h"
#include "M3DEngine/Engine.h"
#include "GraphicsChips/D3D12Formats.h"
#include "M3DCore/MagicEnum.h"


using namespace m3d;


DIALOGDESC_DEF(GraphicsResource_Dlg, GRAPHICSRESOURCECHIP_GUID);


GraphicsResource_Dlg::GraphicsResource_Dlg()
{
	ui.setupUi(this);

	_labelFont = _labelFontBold = ui.label_srv_mostDetailedMip->font();
	_labelFontBold.setBold(true);

	connect(ui.srv_format, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GraphicsResource_Dlg::_onSrvUiUpdated);
	connect(ui.srv_mostDetailedMip, &QLineEdit::editingFinished, this, &GraphicsResource_Dlg::_onSrvUiUpdated);
	connect(ui.srv_mipLevels, &QLineEdit::editingFinished, this, &GraphicsResource_Dlg::_onSrvUiUpdated);
	connect(ui.srv_firstArraySlice, &QLineEdit::editingFinished, this, &GraphicsResource_Dlg::_onSrvUiUpdated);
	connect(ui.srv_arraySize, &QLineEdit::editingFinished, this, &GraphicsResource_Dlg::_onSrvUiUpdated);
	connect(ui.srv_useCubeMap, &QCheckBox::toggled, this, &GraphicsResource_Dlg::_onSrvUiUpdated);
	connect(ui.srv_rawBuffer, &QCheckBox::toggled, this, &GraphicsResource_Dlg::_onSrvUiUpdated);

	connect(ui.rtv_format, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GraphicsResource_Dlg::_onRtvUiUpdated);
	connect(ui.rtv_mipSlice, &QLineEdit::editingFinished, this, &GraphicsResource_Dlg::_onRtvUiUpdated);
	connect(ui.rtv_firstArraySlice, &QLineEdit::editingFinished, this, &GraphicsResource_Dlg::_onRtvUiUpdated);
	connect(ui.rtv_arraySize, &QLineEdit::editingFinished, this, &GraphicsResource_Dlg::_onRtvUiUpdated);

	connect(ui.dsv_format, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GraphicsResource_Dlg::_onDsvUiUpdated);
	connect(ui.dsv_mipSlice, &QLineEdit::editingFinished, this, &GraphicsResource_Dlg::_onDsvUiUpdated);
	connect(ui.dsv_firstArraySlice, &QLineEdit::editingFinished, this, &GraphicsResource_Dlg::_onDsvUiUpdated);
	connect(ui.dsv_arraySize, &QLineEdit::editingFinished, this, &GraphicsResource_Dlg::_onDsvUiUpdated);
	connect(ui.dsv_readOnlyDepth, &QCheckBox::toggled, this, &GraphicsResource_Dlg::_onDsvUiUpdated);
	connect(ui.dsv_readOnlyStencil, &QCheckBox::toggled, this, &GraphicsResource_Dlg::_onDsvUiUpdated);

	connect(ui.uav_format, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GraphicsResource_Dlg::_onUavUiUpdated);
	connect(ui.uav_mipSlice, &QLineEdit::editingFinished, this, &GraphicsResource_Dlg::_onUavUiUpdated);
	connect(ui.uav_firstArraySlice, &QLineEdit::editingFinished, this, &GraphicsResource_Dlg::_onUavUiUpdated);
	connect(ui.uav_arraySize, &QLineEdit::editingFinished, this, &GraphicsResource_Dlg::_onUavUiUpdated);
	connect(ui.uav_raw, &QCheckBox::toggled, this, &GraphicsResource_Dlg::_onUavUiUpdated);
	connect(ui.uav_append, &QCheckBox::toggled, this, &GraphicsResource_Dlg::_onUavUiUpdated);
	connect(ui.uav_counter, &QCheckBox::toggled, this, &GraphicsResource_Dlg::_onUavUiUpdated);
}

GraphicsResource_Dlg::~GraphicsResource_Dlg()
{
}

void GraphicsResource_Dlg::Init()
{
	ID3D12Device *device = GetChip()->device();

	_blockUiUpdates = true;

	ui.srv_format->addItem("Default", DXGI_FORMAT_UNKNOWN);
	ui.rtv_format->addItem("Default", DXGI_FORMAT_UNKNOWN);
	ui.dsv_format->addItem("Default", DXGI_FORMAT_UNKNOWN);
	ui.uav_format->addItem("Default", DXGI_FORMAT_UNKNOWN);
	
	for (DXGI_FORMAT format = dxgiformat::NextFormat(DXGI_FORMAT_UNKNOWN); format != DXGI_FORMAT_UNKNOWN; format = dxgiformat::NextFormat(format)) {
		D3D12_FEATURE_DATA_FORMAT_SUPPORT support { format, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
		HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &support, sizeof(support));
		const Char* fmtStr = dxgiformat::ToString(format);
		const Char* fmtCmtStr = dxgiformat::ToCommentString(format);
		if (!fmtStr)
			continue;
		if (support.Support1 & (D3D12_FORMAT_SUPPORT1_SHADER_LOAD | D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE | D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE_COMPARISON | D3D12_FORMAT_SUPPORT1_MULTISAMPLE_LOAD)) {
			ui.srv_format->addItem(fmtStr, support.Format);
			ui.srv_format->setItemData(ui.srv_format->count() - 1, fmtCmtStr, Qt::ToolTipRole);
		}
		if (support.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET) {
			ui.rtv_format->addItem(fmtStr, support.Format);
			ui.rtv_format->setItemData(ui.rtv_format->count() - 1, fmtCmtStr, Qt::ToolTipRole);
		}
		if (support.Support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL) {
			ui.dsv_format->addItem(fmtStr, support.Format);
			ui.dsv_format->setItemData(ui.dsv_format->count() - 1, fmtCmtStr, Qt::ToolTipRole);
		}
		if (support.Support1 & D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW) {
			ui.uav_format->addItem(fmtStr, support.Format);
			ui.uav_format->setItemData(ui.uav_format->count() - 1, fmtCmtStr, Qt::ToolTipRole);
		}
	} 

	_initSRV = GetChip()->GetSRVDesc();
	for (int i = 0; i < ui.srv_format->count(); i++)
		if ((DXGI_FORMAT)ui.srv_format->itemData(i).toUInt() == _initSRV.format)
			ui.srv_format->setCurrentIndex(i);
	ui.srv_mostDetailedMip->setText(QString::number(_initSRV.mostDetailedMip));
	ui.srv_mipLevels->setText(QString::number((int)_initSRV.mipLevels));
	ui.srv_firstArraySlice->setText(QString::number(_initSRV.firstArraySlice));
	ui.srv_arraySize->setText(QString::number((int)_initSRV.arraySize));
	ui.srv_useCubeMap->setChecked(_initSRV.flags & SRV_USE_CUBEMAP);
	ui.srv_rawBuffer->setChecked(_initSRV.flags & D3D12_BUFFER_SRV_FLAG_RAW);

	_initRTV = GetChip()->GetRTVDesc();
	for (int i = 0; i < ui.rtv_format->count(); i++)
		if ((DXGI_FORMAT)ui.rtv_format->itemData(i).toUInt() == _initRTV.format)
			ui.rtv_format->setCurrentIndex(i);
	ui.rtv_mipSlice->setText(QString::number(_initRTV.mipSlice));
	ui.rtv_firstArraySlice->setText(QString::number(_initRTV.firstArraySlice));
	ui.rtv_arraySize->setText(QString::number((int)_initRTV.arraySize));

	_initDSV = GetChip()->GetDSVDesc();
	for (int i = 0; i < ui.dsv_format->count(); i++)
		if ((DXGI_FORMAT)ui.dsv_format->itemData(i).toUInt() == _initDSV.format)
			ui.dsv_format->setCurrentIndex(i);
	ui.dsv_mipSlice->setText(QString::number(_initDSV.mipSlice));
	ui.dsv_firstArraySlice->setText(QString::number(_initDSV.firstArraySlice));
	ui.dsv_arraySize->setText(QString::number((int)_initDSV.arraySize));
	ui.dsv_readOnlyDepth->setChecked(_initDSV.flags & D3D12_DSV_FLAG_READ_ONLY_DEPTH);
	ui.dsv_readOnlyStencil->setChecked(_initDSV.flags & D3D12_DSV_FLAG_READ_ONLY_STENCIL);

	_initUAV = GetChip()->GetUAVDesc();
	for (int i = 0; i < ui.uav_format->count(); i++)
		if ((DXGI_FORMAT)ui.uav_format->itemData(i).toUInt() == _initUAV.format)
			ui.uav_format->setCurrentIndex(i);
	ui.uav_mipSlice->setText(QString::number(_initUAV.mipSlice));
	ui.uav_firstArraySlice->setText(QString::number(_initUAV.firstArraySlice));
	ui.uav_arraySize->setText(QString::number((int)_initUAV.arraySize));
	ui.uav_raw->setChecked(_initUAV.flags & D3D12_BUFFER_UAV_FLAG_RAW);
//	ui.uav_append->setChecked(_initUAV.flags & D3D11_BUFFER_UAV_FLAG_APPEND);
//	ui.uav_counter->setChecked(_initUAV.flags & D3D11_BUFFER_UAV_FLAG_COUNTER);

	_blockUiUpdates = false;

	_onSrvUiUpdated();
	_onRtvUiUpdated();
	_onDsvUiUpdated();
	_onUavUiUpdated();
}

void GraphicsResource_Dlg::OnOK()
{
	bool ok;

	SRVDesc srvDesc = _getSRVFromUI();
	if (srvDesc != _initSRV) {
		GetChip()->SetSRVDesc(srvDesc);
		SetDirty();
		_initSRV = srvDesc;
	}

	RTVDesc rtvDesc = _getRTVFromUI();
	if (rtvDesc != _initRTV) {
		GetChip()->SetRTVDesc(rtvDesc);
		SetDirty();
		_initRTV = rtvDesc;
	}

	DSVDesc dsvDesc = _getDSVFromUI();
	if (dsvDesc != _initDSV) {
		GetChip()->SetDSVDesc(dsvDesc);
		SetDirty();
		_initDSV = dsvDesc;
	}

	UAVDesc uavDesc = _getUAVFromUI();
	if (uavDesc != _initUAV) {
		GetChip()->SetUAVDesc(uavDesc);
		SetDirty();
		_initUAV = uavDesc;
	}

}

SRVDesc GraphicsResource_Dlg::_getSRVFromUI()
{
	bool ok;
	SRVDesc srvDesc;
	srvDesc.format = (M3D_FORMAT)ui.srv_format->itemData(ui.srv_format->currentIndex()).toUInt(&ok);
	if (!ok) srvDesc.format = _initSRV.format;
	srvDesc.mostDetailedMip = ui.srv_mostDetailedMip->text().toUInt(&ok);
	if (!ok) srvDesc.mostDetailedMip = _initSRV.mostDetailedMip;
	srvDesc.mipLevels = ui.srv_mipLevels->text().toInt(&ok);
	if (!ok) srvDesc.mipLevels = _initSRV.mipLevels;
	srvDesc.firstArraySlice = ui.srv_firstArraySlice->text().toUInt(&ok);
	if (!ok) srvDesc.firstArraySlice = _initSRV.firstArraySlice;
	srvDesc.arraySize = ui.srv_arraySize->text().toInt(&ok);
	if (!ok) srvDesc.arraySize = _initSRV.arraySize;
	srvDesc.flags = (ui.srv_useCubeMap->isChecked() ? SRV_USE_CUBEMAP : 0) | (ui.srv_rawBuffer->isChecked() ? D3D12_BUFFER_SRV_FLAG_RAW : 0);
	return srvDesc;
}

RTVDesc GraphicsResource_Dlg::_getRTVFromUI()
{
	bool ok;
	RTVDesc rtvDesc;
	rtvDesc.format = (M3D_FORMAT)ui.rtv_format->itemData(ui.rtv_format->currentIndex()).toUInt(&ok);
	if (!ok) rtvDesc.format = _initRTV.format;
	rtvDesc.mipSlice = ui.rtv_mipSlice->text().toUInt(&ok);
	if (!ok) rtvDesc.mipSlice = _initRTV.mipSlice;
	rtvDesc.firstArraySlice = ui.rtv_firstArraySlice->text().toUInt(&ok);
	if (!ok) rtvDesc.firstArraySlice = _initRTV.firstArraySlice;
	rtvDesc.arraySize = ui.rtv_arraySize->text().toInt(&ok);
	if (!ok) rtvDesc.arraySize = _initRTV.arraySize;
	return rtvDesc;
}

DSVDesc GraphicsResource_Dlg::_getDSVFromUI()
{
	bool ok;
	DSVDesc dsvDesc;
	dsvDesc.format = (M3D_FORMAT)ui.dsv_format->itemData(ui.dsv_format->currentIndex()).toUInt(&ok);
	if (!ok) dsvDesc.format = _initDSV.format;
	dsvDesc.mipSlice = ui.dsv_mipSlice->text().toUInt(&ok);
	if (!ok) dsvDesc.mipSlice = _initDSV.mipSlice;
	dsvDesc.firstArraySlice = ui.dsv_firstArraySlice->text().toUInt(&ok);
	if (!ok) dsvDesc.firstArraySlice = _initDSV.firstArraySlice;
	dsvDesc.arraySize = ui.dsv_arraySize->text().toInt(&ok);
	if (!ok) dsvDesc.arraySize = _initDSV.arraySize;
	dsvDesc.flags = (ui.dsv_readOnlyDepth->isChecked() ? M3D_DSV_FLAG_READ_ONLY_DEPTH : M3D_DSV_FLAG_NONE) | (ui.dsv_readOnlyStencil->isChecked() ? M3D_DSV_FLAG_READ_ONLY_STENCIL : M3D_DSV_FLAG_NONE);
	return dsvDesc;
}

UAVDesc GraphicsResource_Dlg::_getUAVFromUI()
{
	bool ok;
	UAVDesc uavDesc;
	uavDesc.format = (M3D_FORMAT)ui.uav_format->itemData(ui.uav_format->currentIndex()).toUInt(&ok);
	if (!ok) uavDesc.format = _initUAV.format;
	uavDesc.mipSlice = ui.uav_mipSlice->text().toUInt(&ok);
	if (!ok) uavDesc.mipSlice = _initUAV.mipSlice;
	uavDesc.firstArraySlice = ui.uav_firstArraySlice->text().toUInt(&ok);
	if (!ok) uavDesc.firstArraySlice = _initUAV.firstArraySlice;
	uavDesc.arraySize = ui.uav_arraySize->text().toInt(&ok);
	if (!ok) uavDesc.arraySize = _initUAV.arraySize;
	uavDesc.flags = (ui.uav_raw->isChecked() ? M3D_BUFFER_UAV_FLAG_RAW : M3D_BUFFER_UAV_FLAG_NONE)/* | (ui.uav_append->isChecked() ? D3D11_BUFFER_UAV_FLAG_APPEND : 0) | (ui.uav_counter->isChecked() ? D3D11_BUFFER_UAV_FLAG_COUNTER : 0)*/;
	return uavDesc;
}

void GraphicsResource_Dlg::_onSrvUiUpdated()
{
	if (_blockUiUpdates)
		return;

	static const SRVDesc DEFAULT_SRV;

	const SRVDesc d = _getSRVFromUI();

	ui.label_srv_format->setFont(d.format != DEFAULT_SRV.format ? _labelFontBold : _labelFont);
	ui.label_srv_mostDetailedMip->setFont(d.mostDetailedMip != DEFAULT_SRV.mostDetailedMip ? _labelFontBold : _labelFont);
	ui.label_srv_mipLevels->setFont(d.mipLevels != DEFAULT_SRV.mipLevels ? _labelFontBold : _labelFont);
	ui.label_srv_firstArraySlice->setFont(d.firstArraySlice != DEFAULT_SRV.firstArraySlice ? _labelFontBold : _labelFont);
	ui.label_srv_arraySize->setFont(d.arraySize != DEFAULT_SRV.arraySize ? _labelFontBold : _labelFont);
	ui.srv_useCubeMap->setFont((d.flags & SRV_USE_CUBEMAP) != (DEFAULT_SRV.flags & SRV_USE_CUBEMAP) ? _labelFontBold : _labelFont);
	ui.srv_rawBuffer->setFont((d.flags & D3D12_BUFFER_SRV_FLAG_RAW) != (DEFAULT_SRV.flags & D3D12_BUFFER_SRV_FLAG_RAW) ? _labelFontBold : _labelFont);
}

void GraphicsResource_Dlg::_onRtvUiUpdated()
{
	if (_blockUiUpdates)
		return;

	static const RTVDesc DEFAULT_RTV;

	const RTVDesc d = _getRTVFromUI();

	ui.label_rtv_format->setFont(d.format != DEFAULT_RTV.format ? _labelFontBold : _labelFont);
	ui.label_rtv_mipSlice->setFont(d.mipSlice != DEFAULT_RTV.mipSlice ? _labelFontBold : _labelFont);
	ui.label_rtv_firstArraySlice->setFont(d.firstArraySlice != DEFAULT_RTV.firstArraySlice ? _labelFontBold : _labelFont);
	ui.label_rtv_arraySize->setFont(d.arraySize != DEFAULT_RTV.arraySize ? _labelFontBold : _labelFont);
}

void GraphicsResource_Dlg::_onDsvUiUpdated()
{
	if (_blockUiUpdates)
		return;

	static const DSVDesc DEFAULT_DSV;

	const DSVDesc d = _getDSVFromUI();

	ui.label_dsv_format->setFont(d.format != DEFAULT_DSV.format ? _labelFontBold : _labelFont);
	ui.label_dsv_mipSlice->setFont(d.mipSlice != DEFAULT_DSV.mipSlice ? _labelFontBold : _labelFont);
	ui.label_dsv_firstArraySlice->setFont(d.firstArraySlice != DEFAULT_DSV.firstArraySlice ? _labelFontBold : _labelFont);
	ui.label_dsv_arraySize->setFont(d.arraySize != DEFAULT_DSV.arraySize ? _labelFontBold : _labelFont);
	ui.dsv_readOnlyDepth->setFont((d.flags & D3D12_DSV_FLAG_READ_ONLY_DEPTH) != (DEFAULT_DSV.flags & D3D12_DSV_FLAG_READ_ONLY_DEPTH) ? _labelFontBold : _labelFont);
	ui.dsv_readOnlyStencil->setFont((d.flags & D3D12_DSV_FLAG_READ_ONLY_STENCIL) != (DEFAULT_DSV.flags & D3D12_DSV_FLAG_READ_ONLY_STENCIL) ? _labelFontBold : _labelFont);
}

void GraphicsResource_Dlg::_onUavUiUpdated()
{
	if (_blockUiUpdates)
		return;

	static const UAVDesc DEFAULT_UAV;

	const UAVDesc d = _getUAVFromUI();

	ui.label_uav_format->setFont(d.format != DEFAULT_UAV.format ? _labelFontBold : _labelFont);
	ui.label_uav_mipSlice->setFont(d.mipSlice != DEFAULT_UAV.mipSlice ? _labelFontBold : _labelFont);
	ui.label_uav_firstArraySlice->setFont(d.firstArraySlice != DEFAULT_UAV.firstArraySlice ? _labelFontBold : _labelFont);
	ui.label_uav_arraySize->setFont(d.arraySize != DEFAULT_UAV.arraySize ? _labelFontBold : _labelFont);
	ui.uav_raw->setFont((d.flags & D3D12_BUFFER_UAV_FLAG_RAW) != (DEFAULT_UAV.flags & D3D12_BUFFER_UAV_FLAG_RAW) ? _labelFontBold : _labelFont);
//	ui.uav_append->setFont((d.flags & D3D12_BUFFER_UAV_FLAG_APPEND) != (DEFAULT_UAV.flags & D3D12_BUFFER_UAV_FLAG_APPEND) ? _labelFontBold : _labelFont);
//	ui.uav_counter->setFont((d.flags & D3D12_BUFFER_UAV_FLAG_COUNTER) != (DEFAULT_UAV.flags & D3D12_BUFFER_UAV_FLAG_COUNTER) ? _labelFontBold : _labelFont);
}
