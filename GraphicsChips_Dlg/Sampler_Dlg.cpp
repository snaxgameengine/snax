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
#include "Sampler_Dlg.h"


using namespace m3d;


DIALOGDESC_DEF(Sampler_Dlg, SAMPLER_GUID);


Sampler_Dlg::Sampler_Dlg()
{
	ui.setupUi(this);

	_labelFont = _labelFontBold = ui.label_filter->font();
	_labelFontBold.setBold(true);

	ui.comboBox_ss_filter->addItem("MIN_MAG_MIP_POINT", M3D_FILTER_MIN_MAG_MIP_POINT);
	ui.comboBox_ss_filter->addItem("MIN_MAG_POINT_MIP_LINEAR", M3D_FILTER_MIN_MAG_POINT_MIP_LINEAR);
	ui.comboBox_ss_filter->addItem("MIN_POINT_MAG_LINEAR_MIP_POINT", M3D_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT);
	ui.comboBox_ss_filter->addItem("MIN_POINT_MAG_MIP_LINEAR", M3D_FILTER_MIN_POINT_MAG_MIP_LINEAR);
	ui.comboBox_ss_filter->addItem("MIN_LINEAR_MAG_MIP_POINT", M3D_FILTER_MIN_LINEAR_MAG_MIP_POINT);
	ui.comboBox_ss_filter->addItem("MIN_LINEAR_MAG_POINT_MIP_LINEAR", M3D_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR);
	ui.comboBox_ss_filter->addItem("MIN_MAG_LINEAR_MIP_POINT", M3D_FILTER_MIN_MAG_LINEAR_MIP_POINT);
	ui.comboBox_ss_filter->addItem("MIN_MAG_MIP_LINEAR", M3D_FILTER_MIN_MAG_MIP_LINEAR);
	ui.comboBox_ss_filter->addItem("ANISOTROPIC", M3D_FILTER_ANISOTROPIC);
	ui.comboBox_ss_filter->addItem("COMPARISON_MIN_MAG_MIP_POINT", M3D_FILTER_COMPARISON_MIN_MAG_MIP_POINT);
	ui.comboBox_ss_filter->addItem("COMPARISON_MIN_MAG_POINT_MIP_LINEAR", M3D_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR);
	ui.comboBox_ss_filter->addItem("COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT", M3D_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT);
	ui.comboBox_ss_filter->addItem("COMPARISON_MIN_POINT_MAG_MIP_LINEAR", M3D_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR);
	ui.comboBox_ss_filter->addItem("COMPARISON_MIN_LINEAR_MAG_MIP_POINT", M3D_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT);
	ui.comboBox_ss_filter->addItem("COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR", M3D_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR);
	ui.comboBox_ss_filter->addItem("COMPARISON_MIN_MAG_LINEAR_MIP_POINT", M3D_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT);
	ui.comboBox_ss_filter->addItem("COMPARISON_MIN_MAG_MIP_LINEAR", M3D_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR);
	ui.comboBox_ss_filter->addItem("COMPARISON_ANISOTROPIC", M3D_FILTER_COMPARISON_ANISOTROPIC);
	ui.comboBox_ss_filter->addItem("MINIMUM_MIN_MAG_MIP_POINT", M3D_FILTER_MINIMUM_MIN_MAG_MIP_POINT);
	ui.comboBox_ss_filter->addItem("MINIMUM_MIN_MAG_POINT_MIP_LINEAR", M3D_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR);
	ui.comboBox_ss_filter->addItem("MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT", M3D_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT);
	ui.comboBox_ss_filter->addItem("MINIMUM_MIN_POINT_MAG_MIP_LINEAR", M3D_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR);
	ui.comboBox_ss_filter->addItem("MINIMUM_MIN_LINEAR_MAG_MIP_POINT", M3D_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT);
	ui.comboBox_ss_filter->addItem("MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR", M3D_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR);
	ui.comboBox_ss_filter->addItem("MINIMUM_MIN_MAG_LINEAR_MIP_POINT", M3D_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT);
	ui.comboBox_ss_filter->addItem("MINIMUM_MIN_MAG_MIP_LINEAR", M3D_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR);
	ui.comboBox_ss_filter->addItem("MINIMUM_ANISOTROPIC", M3D_FILTER_MINIMUM_ANISOTROPIC);
	ui.comboBox_ss_filter->addItem("MAXIMUM_MIN_MAG_MIP_POINT", M3D_FILTER_MAXIMUM_MIN_MAG_MIP_POINT);
	ui.comboBox_ss_filter->addItem("MAXIMUM_MIN_MAG_POINT_MIP_LINEAR", M3D_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR);
	ui.comboBox_ss_filter->addItem("MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT", M3D_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT);
	ui.comboBox_ss_filter->addItem("MAXIMUM_MIN_POINT_MAG_MIP_LINEAR", M3D_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR);
	ui.comboBox_ss_filter->addItem("MAXIMUM_MIN_LINEAR_MAG_MIP_POINT", M3D_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT);
	ui.comboBox_ss_filter->addItem("MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR", M3D_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR);
	ui.comboBox_ss_filter->addItem("MAXIMUM_MIN_MAG_LINEAR_MIP_POINT", M3D_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT);
	ui.comboBox_ss_filter->addItem("MAXIMUM_MIN_MAG_MIP_LINEAR", M3D_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR);
	ui.comboBox_ss_filter->addItem("MAXIMUM_ANISOTROPIC", M3D_FILTER_MAXIMUM_ANISOTROPIC);

	ui.comboBox_ss_addressU->addItem("Wrap", M3D_TEXTURE_ADDRESS_MODE_WRAP);
	ui.comboBox_ss_addressU->addItem("Mirror", M3D_TEXTURE_ADDRESS_MODE_MIRROR);
	ui.comboBox_ss_addressU->addItem("Clamp", M3D_TEXTURE_ADDRESS_MODE_CLAMP);
	ui.comboBox_ss_addressU->addItem("Border Color", M3D_TEXTURE_ADDRESS_MODE_BORDER);
	ui.comboBox_ss_addressU->addItem("Mirror Once", M3D_TEXTURE_ADDRESS_MODE_MIRROR_ONCE);

	ui.comboBox_ss_addressV->addItem("Wrap", M3D_TEXTURE_ADDRESS_MODE_WRAP);
	ui.comboBox_ss_addressV->addItem("Mirror", M3D_TEXTURE_ADDRESS_MODE_MIRROR);
	ui.comboBox_ss_addressV->addItem("Clamp", M3D_TEXTURE_ADDRESS_MODE_CLAMP);
	ui.comboBox_ss_addressV->addItem("Border Color", M3D_TEXTURE_ADDRESS_MODE_BORDER);
	ui.comboBox_ss_addressV->addItem("Mirror Once", M3D_TEXTURE_ADDRESS_MODE_MIRROR_ONCE);

	ui.comboBox_ss_addressW->addItem("Wrap", M3D_TEXTURE_ADDRESS_MODE_WRAP);
	ui.comboBox_ss_addressW->addItem("Mirror", M3D_TEXTURE_ADDRESS_MODE_MIRROR);
	ui.comboBox_ss_addressW->addItem("Clamp", M3D_TEXTURE_ADDRESS_MODE_CLAMP);
	ui.comboBox_ss_addressW->addItem("Border Color", M3D_TEXTURE_ADDRESS_MODE_BORDER);
	ui.comboBox_ss_addressW->addItem("Mirror Once", M3D_TEXTURE_ADDRESS_MODE_MIRROR_ONCE);

	ui.comboBox_ss_compFunc->addItem("Never", M3D_COMPARISON_FUNC_NEVER);
	ui.comboBox_ss_compFunc->addItem("Less", M3D_COMPARISON_FUNC_LESS);
	ui.comboBox_ss_compFunc->addItem("Equal", M3D_COMPARISON_FUNC_EQUAL);
	ui.comboBox_ss_compFunc->addItem("Less or Equal", M3D_COMPARISON_FUNC_LESS_EQUAL);
	ui.comboBox_ss_compFunc->addItem("Greater", M3D_COMPARISON_FUNC_GREATER);
	ui.comboBox_ss_compFunc->addItem("Not Equal", M3D_COMPARISON_FUNC_NOT_EQUAL);
	ui.comboBox_ss_compFunc->addItem("Greater or Equal", M3D_COMPARISON_FUNC_GREATER_EQUAL);
	ui.comboBox_ss_compFunc->addItem("Always", M3D_COMPARISON_FUNC_ALWAYS);

	QMenu *m = new QMenu(this);
	_scTBlack = m->addAction("Transparent Black");
	_scOBlack = m->addAction("Opaque Black");
	_scOWhite = m->addAction("Opaque White");

	ui.toolButton_staticColors->setMenu(m);

	connect(ui.toolButton_staticColors, &QToolButton::clicked, this, &Sampler_Dlg::_onShowColorDialog);
	connect(m, &QMenu::triggered, this, &Sampler_Dlg::_onStaticColorSelected);

}

Sampler_Dlg::~Sampler_Dlg()
{
}

void Sampler_Dlg::Init()
{
	_initDesc = GetChip()->GetSamplerDesc();

	ui.comboBox_ss_filter->setCurrentIndex(ui.comboBox_ss_filter->findData(_initDesc.Filter));
	ui.comboBox_ss_addressU->setCurrentIndex(ui.comboBox_ss_addressU->findData(_initDesc.AddressU));
	ui.comboBox_ss_addressV->setCurrentIndex(ui.comboBox_ss_addressV->findData(_initDesc.AddressV));
	ui.comboBox_ss_addressW->setCurrentIndex(ui.comboBox_ss_addressW->findData(_initDesc.AddressW));
	ui.comboBox_ss_compFunc->setCurrentIndex(ui.comboBox_ss_compFunc->findData(_initDesc.ComparisonFunc));
	ui.lineEdit_ss_borderR->setText(QString::number(_initDesc.BorderColor[0]));
	ui.lineEdit_ss_borderG->setText(QString::number(_initDesc.BorderColor[1]));
	ui.lineEdit_ss_borderB->setText(QString::number(_initDesc.BorderColor[2]));
	ui.lineEdit_ss_borderA->setText(QString::number(_initDesc.BorderColor[3]));
	ui.lineEdit_ss_mipLODBias->setText(QString::number(_initDesc.MipLODBias));
	ui.lineEdit_ss_maxAnisotropy->setText(QString::number(_initDesc.MaxAnisotropy));
	ui.lineEdit_ss_minLOD->setText(QString::number(_initDesc.MinLOD));
	ui.lineEdit_ss_maxLOD->setText(QString::number(_initDesc.MaxLOD));
	_updateUI(_initDesc);
}

void Sampler_Dlg::OnOK()
{
}

void Sampler_Dlg::OnCancel()
{
	if (std::memcmp(&_initDesc, &GetChip()->GetSamplerDesc(), sizeof(M3D_SAMPLER_DESC)) != 0) {
		GetChip()->SetSamplerDesc(_initDesc);
		SetDirty();
	}
}


void Sampler_Dlg::_onStaticColorSelected(QAction *a) 
{
	if (a == _scTBlack) {
		ui.lineEdit_ss_borderR->setText("0");
		ui.lineEdit_ss_borderG->setText("0");
		ui.lineEdit_ss_borderB->setText("0");
		ui.lineEdit_ss_borderA->setText("0");
	}
	else if (a == _scOBlack) {
		ui.lineEdit_ss_borderR->setText("0");
		ui.lineEdit_ss_borderG->setText("0");
		ui.lineEdit_ss_borderB->setText("0");
		ui.lineEdit_ss_borderA->setText("1");
	}
	else {
		ui.lineEdit_ss_borderR->setText("1");
		ui.lineEdit_ss_borderG->setText("1");
		ui.lineEdit_ss_borderB->setText("1");
		ui.lineEdit_ss_borderA->setText("1");
	}

	onUpdate();
}

void Sampler_Dlg::_onShowColorDialog()
{
	QColor c(int32(ui.lineEdit_ss_borderR->text().toFloat() * 255.0f), int32(ui.lineEdit_ss_borderG->text().toFloat() * 255.0f), int32(ui.lineEdit_ss_borderB->text().toFloat() * 255.0f), int32(ui.lineEdit_ss_borderA->text().toFloat() * 255.0f));

	c = QColorDialog::getColor(c, this, "Select Border Color", QColorDialog::ShowAlphaChannel);
	if (!c.isValid())
		return;

	ui.lineEdit_ss_borderR->setText(QString::number(c.redF()));
	ui.lineEdit_ss_borderG->setText(QString::number(c.greenF()));
	ui.lineEdit_ss_borderB->setText(QString::number(c.blueF()));
	ui.lineEdit_ss_borderA->setText(QString::number(c.alphaF()));

	onUpdate();
}

void Sampler_Dlg::onUpdate()
{
	M3D_SAMPLER_DESC desc;
	desc.Filter = (M3D_FILTER)ui.comboBox_ss_filter->currentData().toUInt();
	desc.AddressU = (M3D_TEXTURE_ADDRESS_MODE)ui.comboBox_ss_addressU->currentData().toUInt();
	desc.AddressV = (M3D_TEXTURE_ADDRESS_MODE)ui.comboBox_ss_addressV->currentData().toUInt();
	desc.AddressW = (M3D_TEXTURE_ADDRESS_MODE)ui.comboBox_ss_addressW->currentData().toUInt();
	desc.MipLODBias = ui.lineEdit_ss_mipLODBias->text().toFloat();
	desc.MaxAnisotropy = ui.lineEdit_ss_maxAnisotropy->text().toFloat();
	desc.ComparisonFunc = (M3D_COMPARISON_FUNC)ui.comboBox_ss_compFunc->currentData().toUInt();
	desc.BorderColor[0] = ui.lineEdit_ss_borderR->text().toFloat();
	desc.BorderColor[1] = ui.lineEdit_ss_borderG->text().toFloat();
	desc.BorderColor[2] = ui.lineEdit_ss_borderB->text().toFloat();
	desc.BorderColor[3] = ui.lineEdit_ss_borderA->text().toFloat();
	desc.MinLOD = ui.lineEdit_ss_minLOD->text().toFloat();
	desc.MaxLOD = ui.lineEdit_ss_maxLOD->text().toFloat();
	if (std::memcmp(&desc, &GetChip()->GetSamplerDesc(), sizeof(M3D_SAMPLER_DESC)) != 0) {
		GetChip()->SetSamplerDesc(desc);
		SetDirty();
	}
	_updateUI(desc);
}

void Sampler_Dlg::_updateUI(const M3D_SAMPLER_DESC &samplerDesc)
{
	static const M3D_SAMPLER_DESC def = M3D_SAMPLER_DESC();

	ui.label_filter->setFont(samplerDesc.Filter == def.Filter ? _labelFont : _labelFontBold);
	ui.label_addressU->setFont(samplerDesc.AddressU == def.AddressU ? _labelFont : _labelFontBold);
	ui.label_addressV->setFont(samplerDesc.AddressV == def.AddressV ? _labelFont : _labelFontBold);
	ui.label_addressW->setFont(samplerDesc.AddressW == def.AddressW ? _labelFont : _labelFontBold);
	ui.label_compFunc->setFont(samplerDesc.ComparisonFunc == def.ComparisonFunc ? _labelFont : _labelFontBold);
	ui.label_borderColor->setFont(m3d::flteql(samplerDesc.BorderColor[0], def.BorderColor[0]) && m3d::flteql(samplerDesc.BorderColor[1], def.BorderColor[1]) && m3d::flteql(samplerDesc.BorderColor[2], def.BorderColor[2]) && m3d::flteql(samplerDesc.BorderColor[3], def.BorderColor[3])  ? _labelFont : _labelFontBold);
	ui.label_mipLodBias->setFont(m3d::flteql(samplerDesc.MipLODBias, def.MipLODBias) ? _labelFont : _labelFontBold);
	ui.label_maxAnis->setFont(m3d::flteql(samplerDesc.MaxAnisotropy, def.MaxAnisotropy) ? _labelFont : _labelFontBold);
	ui.label_minLod->setFont(m3d::flteql(samplerDesc.MinLOD, def.MinLOD) ? _labelFont : _labelFontBold);
	ui.label_maxLod->setFont(m3d::flteql(samplerDesc.MaxLOD, def.MaxLOD) ? _labelFont : _labelFontBold);
}