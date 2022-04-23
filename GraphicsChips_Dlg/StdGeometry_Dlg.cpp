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
#include "StdGeometry_Dlg.h"


using namespace m3d;


DIALOGDESC_DEF(StdGeometry_Dlg, STDGEOMETRY_GUID);


StdGeometry_Dlg::StdGeometry_Dlg()
{
	ui.setupUi(this);

	_block = true;

	ui.buttonGroup_tsc->setId(ui.radioButton_tsc0, StdGeometry::TSC_NONE);
	ui.buttonGroup_tsc->setId(ui.radioButton_tsc1, StdGeometry::TSC_SNORM16);
	ui.buttonGroup_tsc->setId(ui.radioButton_tsc2, StdGeometry::TSC_10_10_10_2);
	ui.buttonGroup_tsc->setId(ui.radioButton_tsc3, StdGeometry::TSC_SPHEREMAP_TRANSFORM8);
	ui.buttonGroup_tsc->setId(ui.radioButton_tsc4, StdGeometry::TSC_SPHEREMAP_TRANSFORM4);
	ui.buttonGroup_tsc->setId(ui.radioButton_tsc5, StdGeometry::TSC_QUAT16);
	ui.buttonGroup_tsc->setId(ui.radioButton_tsc6, StdGeometry::TSC_QUAT8);
	ui.buttonGroup_tsc->setId(ui.radioButton_tsc7, StdGeometry::TSC_QUAT4);

	ui.comboBox_str_normals->addItem("-", StdGeometry::DISABLE_ELEMENT);
	ui.comboBox_str_tangents->addItem("-", StdGeometry::DISABLE_ELEMENT);
	ui.comboBox_str_bitangents->addItem("-", StdGeometry::DISABLE_ELEMENT);
	ui.comboBox_str_colors->addItem("-", StdGeometry::DISABLE_ELEMENT);
	ui.comboBox_str_blendWeights->addItem("-", StdGeometry::DISABLE_ELEMENT);
	ui.comboBox_str_blendIndices->addItem("-", StdGeometry::DISABLE_ELEMENT);
	ui.comboBox_str_tc0->addItem("-", StdGeometry::DISABLE_ELEMENT);
	ui.comboBox_str_tc1->addItem("-", StdGeometry::DISABLE_ELEMENT);
	ui.comboBox_str_tc2->addItem("-", StdGeometry::DISABLE_ELEMENT);
	ui.comboBox_str_tc3->addItem("-", StdGeometry::DISABLE_ELEMENT);

	for (uint32 i = 0; i < 32/*D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT*/; i++) {
		QString n = QString::number(i);
		ui.comboBox_str_positions->addItem(n, i);
		ui.comboBox_str_normals->addItem(n, i);
		ui.comboBox_str_tangents->addItem(n, i);
		ui.comboBox_str_bitangents->addItem(n, i);
		ui.comboBox_str_colors->addItem(n, i);
		ui.comboBox_str_blendWeights->addItem(n, i);
		ui.comboBox_str_blendIndices->addItem(n, i);
		ui.comboBox_str_tc0->addItem(n, i);
		ui.comboBox_str_tc1->addItem(n, i);
		ui.comboBox_str_tc2->addItem(n, i);
		ui.comboBox_str_tc3->addItem(n, i);
	}

	QComboBox *C[4] = {ui.comboBox_comp_tc0, ui.comboBox_comp_tc1, ui.comboBox_comp_tc2, ui.comboBox_comp_tc3};
	for (uint32 i = 0; i < 4; i++) {
		C[i]->addItem("1 Comp - FLOAT", StdGeometry::U+0); // 1
		C[i]->addItem("1 Comp - UNORM16", StdGeometry::U+1); // 2 
		C[i]->addItem("2 Comp - FLOAT", StdGeometry::UV+1); // 3
		C[i]->addItem("2 Comp - UNORM16", StdGeometry::UV+2); // 4
		C[i]->addItem("3 Comp - FLOAT", StdGeometry::UVW+2); // 5
		C[i]->addItem("3 Comp - UNORM16", StdGeometry::UVW+3); // 6
		C[i]->addItem("4 Comp - FLOAT", StdGeometry::UVWX+3); // 7
		C[i]->addItem("4 Comp - UNORM16", StdGeometry::UVWX+4); // 8
	}

	connect(ui.comboBox_str_positions, &QComboBox::currentIndexChanged, this, &StdGeometry_Dlg::strChanged);
	connect(ui.comboBox_str_normals, &QComboBox::currentIndexChanged, this, &StdGeometry_Dlg::strChanged);
	connect(ui.comboBox_str_tangents, &QComboBox::currentIndexChanged, this, &StdGeometry_Dlg::strChanged);
	connect(ui.comboBox_str_bitangents, &QComboBox::currentIndexChanged, this, &StdGeometry_Dlg::strChanged);
	connect(ui.comboBox_str_colors, &QComboBox::currentIndexChanged, this, &StdGeometry_Dlg::strChanged);
	connect(ui.comboBox_str_blendIndices, &QComboBox::currentIndexChanged, this, &StdGeometry_Dlg::strChanged);
	connect(ui.comboBox_str_blendWeights, &QComboBox::currentIndexChanged, this, &StdGeometry_Dlg::strChanged);
	connect(ui.comboBox_str_tc0, &QComboBox::currentIndexChanged, this, &StdGeometry_Dlg::strChanged);
	connect(ui.comboBox_str_tc1, &QComboBox::currentIndexChanged, this, &StdGeometry_Dlg::strChanged);
	connect(ui.comboBox_str_tc2, &QComboBox::currentIndexChanged, this, &StdGeometry_Dlg::strChanged);
	connect(ui.comboBox_str_tc3, &QComboBox::currentIndexChanged, this, &StdGeometry_Dlg::strChanged);
	connect(ui.comboBox_comp_tc0,&QComboBox::currentIndexChanged, this, &StdGeometry_Dlg::tccChanged);
	connect(ui.comboBox_comp_tc1,&QComboBox::currentIndexChanged, this, &StdGeometry_Dlg::tccChanged);
	connect(ui.comboBox_comp_tc2,&QComboBox::currentIndexChanged, this, &StdGeometry_Dlg::tccChanged);
	connect(ui.comboBox_comp_tc3,&QComboBox::currentIndexChanged, this, &StdGeometry_Dlg::tccChanged);
	connect(ui.buttonGroup_tsc, &QButtonGroup::idClicked, this, &StdGeometry_Dlg::tscChanged);
}

StdGeometry_Dlg::~StdGeometry_Dlg()
{
}

void StdGeometry_Dlg::Init()
{
	_block = true;

	// TODO: Should I make the Cancel button work?

	StdGeometry *d = GetChip();

	ui.label_vertexCount->setText(QString::number(d->GetPositions().size()));
	if (d->GetAPI() == DRAW)
		ui.label_indexCount->setText("N/A");
	else
		ui.label_indexCount->setText(QString::number(d->GetIndices().size()));

	ui.comboBox_str_positions->setCurrentIndex(ui.comboBox_str_positions->findData(d->GetElementStream(StdGeometry::POSITIONS)));
	ui.comboBox_str_normals->setCurrentIndex(ui.comboBox_str_normals->findData(d->GetElementStream(StdGeometry::NORMALS)));
	ui.comboBox_str_tangents->setCurrentIndex(ui.comboBox_str_tangents->findData(d->GetElementStream(StdGeometry::TANGENTS)));
	ui.comboBox_str_bitangents->setCurrentIndex(ui.comboBox_str_bitangents->findData(d->GetElementStream(StdGeometry::BITANGENTS)));
	ui.comboBox_str_colors->setCurrentIndex(ui.comboBox_str_colors->findData(d->GetElementStream(StdGeometry::COLORS)));
	ui.comboBox_str_blendIndices->setCurrentIndex(ui.comboBox_str_blendIndices->findData(d->GetElementStream(StdGeometry::BLENDINDICES)));
	ui.comboBox_str_blendWeights->setCurrentIndex(ui.comboBox_str_blendWeights->findData(d->GetElementStream(StdGeometry::BLENDWEIGHTS)));
	ui.comboBox_str_tc0->setCurrentIndex(ui.comboBox_str_tc0->findData(d->GetElementStream(StdGeometry::TEXCOORD0)));
	ui.comboBox_str_tc1->setCurrentIndex(ui.comboBox_str_tc1->findData(d->GetElementStream(StdGeometry::TEXCOORD1)));
	ui.comboBox_str_tc2->setCurrentIndex(ui.comboBox_str_tc2->findData(d->GetElementStream(StdGeometry::TEXCOORD2)));
	ui.comboBox_str_tc3->setCurrentIndex(ui.comboBox_str_tc3->findData(d->GetElementStream(StdGeometry::TEXCOORD3)));

	ui.comboBox_str_positions->setEnabled(!d->GetPositions().empty());
	ui.comboBox_str_normals->setEnabled(!d->GetNormals().empty());
	ui.comboBox_str_tangents->setEnabled(!d->GetTangents().empty());
	ui.comboBox_str_bitangents->setEnabled(!d->GetBitangents().empty());
	ui.comboBox_str_colors->setEnabled(!d->GetColors().empty());
	ui.comboBox_str_blendIndices->setEnabled(!d->GetBlendIndices().empty());
	ui.comboBox_str_blendWeights->setEnabled(!d->GetBlendWeights().empty());
	ui.comboBox_str_tc0->setEnabled(d->GetTexCoords(0).size() > 0);
	ui.comboBox_str_tc1->setEnabled(d->GetTexCoords(1).size() > 0);
	ui.comboBox_str_tc2->setEnabled(d->GetTexCoords(2).size() > 0);
	ui.comboBox_str_tc3->setEnabled(d->GetTexCoords(3).size() > 0);

	ui.pushButton_del_normals->setEnabled(!d->GetNormals().empty());
	ui.pushButton_del_tangents->setEnabled(!d->GetTangents().empty());
	ui.pushButton_del_bitangents->setEnabled(!d->GetBitangents().empty());
	ui.pushButton_del_colors->setEnabled(!d->GetColors().empty());
	ui.pushButton_del_blendIndices->setEnabled(!d->GetBlendIndices().empty());
	ui.pushButton_del_blendWeights->setEnabled(!d->GetBlendWeights().empty());
	ui.pushButton_del_tc0->setEnabled(d->GetTexCoords(0).size() > 0);
	ui.pushButton_del_tc1->setEnabled(d->GetTexCoords(1).size() > 0);
	ui.pushButton_del_tc2->setEnabled(d->GetTexCoords(2).size() > 0);
	ui.pushButton_del_tc3->setEnabled(d->GetTexCoords(3).size() > 0);
	
	ui.comboBox_comp_tc0->setCurrentIndex(d->GetTexCoords(0).type * 2 - 2 + d->GetTexCoords(0).fmt);
	ui.comboBox_comp_tc0->setEnabled(d->GetTexCoords(0).size() > 0);

	ui.comboBox_comp_tc1->setCurrentIndex(d->GetTexCoords(1).type * 2 - 2 + d->GetTexCoords(1).fmt);
	ui.comboBox_comp_tc1->setEnabled(d->GetTexCoords(1).size() > 0);

	ui.comboBox_comp_tc2->setCurrentIndex(d->GetTexCoords(2).type * 2 - 2 + d->GetTexCoords(2).fmt);
	ui.comboBox_comp_tc2->setEnabled(d->GetTexCoords(2).size() > 0);

	ui.comboBox_comp_tc3->setCurrentIndex(d->GetTexCoords(3).type * 2 - 2 + d->GetTexCoords(3).fmt);
	ui.comboBox_comp_tc3->setEnabled(d->GetTexCoords(3).size() > 0);

	QAbstractButton *b = ui.buttonGroup_tsc->button(d->GetTangentSpaceCompression());
	if (b)
		b->setChecked(true);

	ui.checkBox_packNormals->setChecked(d->IsPackNormals());
	ui.checkBox_packTextureSets->setChecked(d->IsPackTextureSets());
	_ntbEnable();
	_texEnable();

	ui.pushButton_transform->setEnabled(!d->GetPositions().empty());

	_block = false;
}

void StdGeometry_Dlg::delClicked()
{
	if (sender() == ui.pushButton_del_normals) {
		GetChip()->ClearNormals();
		ui.pushButton_del_normals->setEnabled(false);
		ui.comboBox_str_normals->setEnabled(false);
		ui.comboBox_str_normals->setCurrentIndex(-1);
		_ntbEnable();
	}
	else if (sender() == ui.pushButton_del_tangents) {
		GetChip()->ClearTangents();
		ui.pushButton_del_tangents->setEnabled(false);
		ui.comboBox_str_tangents->setEnabled(false);
		ui.comboBox_str_tangents->setCurrentIndex(-1);
		_ntbEnable();
	}
	else if (sender() == ui.pushButton_del_bitangents) {
		GetChip()->ClearBitangents();
		ui.pushButton_del_bitangents->setEnabled(false);
		ui.comboBox_str_bitangents->setEnabled(false);
		ui.comboBox_str_bitangents->setCurrentIndex(-1);
		_ntbEnable();
	}
	else if (sender() == ui.pushButton_del_colors) {
		GetChip()->ClearColors();
		ui.pushButton_del_colors->setEnabled(false);
		ui.comboBox_str_colors->setEnabled(false);
		ui.comboBox_str_colors->setCurrentIndex(-1);
	}
	else if (sender() == ui.pushButton_del_blendIndices) {
		GetChip()->ClearBlendIndices();
		ui.pushButton_del_blendIndices->setEnabled(false);
		ui.comboBox_str_blendIndices->setEnabled(false);
		ui.comboBox_str_blendIndices->setCurrentIndex(-1);
	}
	else if (sender() == ui.pushButton_del_blendWeights) {
		GetChip()->ClearBlendWeights();
		ui.pushButton_del_blendWeights->setEnabled(false);
		ui.comboBox_str_blendWeights->setEnabled(false);
		ui.comboBox_str_blendWeights->setCurrentIndex(-1);
	}
	else if (sender() == ui.pushButton_del_tc0) {
		GetChip()->ClearTexcoords(0);
		ui.pushButton_del_tc0->setEnabled(false);
		ui.comboBox_str_tc0->setEnabled(false);
		ui.comboBox_str_tc0->setCurrentIndex(-1);
		ui.comboBox_comp_tc0->setEnabled(false);
		ui.comboBox_comp_tc0->setCurrentIndex(-1);
		_texEnable();
	}
	else if (sender() == ui.pushButton_del_tc1) {
		GetChip()->ClearTexcoords(1);
		ui.pushButton_del_tc1->setEnabled(false);
		ui.comboBox_str_tc1->setEnabled(false);
		ui.comboBox_str_tc1->setCurrentIndex(-1);
		ui.comboBox_comp_tc1->setEnabled(false);
		ui.comboBox_comp_tc1->setCurrentIndex(-1);
		_texEnable();
	}
	else if (sender() == ui.pushButton_del_tc2) {
		GetChip()->ClearTexcoords(2);
		ui.pushButton_del_tc2->setEnabled(false);
		ui.comboBox_str_tc2->setEnabled(false);
		ui.comboBox_str_tc2->setCurrentIndex(-1);
		ui.comboBox_comp_tc2->setEnabled(false);
		ui.comboBox_comp_tc2->setCurrentIndex(-1);
		_texEnable();
	}
	else if (sender() == ui.pushButton_del_tc3) {
		GetChip()->ClearTexcoords(3);
		ui.pushButton_del_tc3->setEnabled(false);
		ui.comboBox_str_tc3->setEnabled(false);
		ui.comboBox_str_tc3->setCurrentIndex(-1);
		ui.comboBox_comp_tc3->setEnabled(false);
		ui.comboBox_comp_tc3->setCurrentIndex(-1);
		_texEnable();
	}
	GetChip()->DestroyDeviceObjects();
	SetDirty();
}

void StdGeometry_Dlg::strChanged(int32 i)
{
	if (i == -1 || _block)
		return;
	if (sender() == ui.comboBox_str_positions) GetChip()->SetElementStream(StdGeometry::POSITIONS, ui.comboBox_str_positions->itemData(i).toUInt());
	else if (sender() == ui.comboBox_str_normals) { GetChip()->SetElementStream(StdGeometry::NORMALS, ui.comboBox_str_normals->itemData(i).toUInt()); _ntbEnable(); }
	else if (sender() == ui.comboBox_str_tangents) { GetChip()->SetElementStream(StdGeometry::TANGENTS, ui.comboBox_str_tangents->itemData(i).toUInt()); _ntbEnable(); }
	else if (sender() == ui.comboBox_str_bitangents) { GetChip()->SetElementStream(StdGeometry::BITANGENTS, ui.comboBox_str_bitangents->itemData(i).toUInt()); _ntbEnable(); }
	else if (sender() == ui.comboBox_str_colors) GetChip()->SetElementStream(StdGeometry::COLORS, ui.comboBox_str_colors->itemData(i).toUInt());
	else if (sender() == ui.comboBox_str_blendIndices) GetChip()->SetElementStream(StdGeometry::BLENDINDICES, ui.comboBox_str_blendIndices->itemData(i).toUInt());
	else if (sender() == ui.comboBox_str_blendWeights) GetChip()->SetElementStream(StdGeometry::BLENDWEIGHTS, ui.comboBox_str_blendWeights->itemData(i).toUInt());
	else if (sender() == ui.comboBox_str_tc0) { GetChip()->SetElementStream(StdGeometry::TEXCOORD0, ui.comboBox_str_tc0->itemData(i).toUInt()); _texEnable(); }
	else if (sender() == ui.comboBox_str_tc1) { GetChip()->SetElementStream(StdGeometry::TEXCOORD1, ui.comboBox_str_tc1->itemData(i).toUInt()); _texEnable(); }
	else if (sender() == ui.comboBox_str_tc2) { GetChip()->SetElementStream(StdGeometry::TEXCOORD2, ui.comboBox_str_tc2->itemData(i).toUInt()); _texEnable(); }
	else if (sender() == ui.comboBox_str_tc3) { GetChip()->SetElementStream(StdGeometry::TEXCOORD3, ui.comboBox_str_tc3->itemData(i).toUInt()); _texEnable(); }
	GetChip()->DestroyDeviceObjects();
	SetDirty();
}

void StdGeometry_Dlg::tccChanged(int32 i)
{
	if (i == -1 || _block)
		return;
	int32 s = 0;
	if (sender() == ui.comboBox_comp_tc1)
		s = 1;
	else if (sender() == ui.comboBox_comp_tc2)
		s = 2;
	else if (sender() == ui.comboBox_comp_tc3)
		s = 3;
	StdGeometry::TexCoordSetType t = StdGeometry::TexCoordSetType(i/2+1);
	StdGeometry::TexCoordSetFormat f = StdGeometry::TexCoordSetFormat(i%2);
	if (t != GetChip()->GetTexCoords(s).type)
		GetChip()->ChangeTexCoordSetType(s, t);
	if (f != GetChip()->GetTexCoords(s).fmt)
		GetChip()->SetTexCoordSetFormat(s, f);
	GetChip()->DestroyDeviceObjects();
	SetDirty();
}


void StdGeometry_Dlg::swapTexSetClicked()
{
	QDialog d(this);
	QGridLayout *l = new QGridLayout(&d);
	l->addWidget(new QLabel("Swap"), 0, 0);
	QComboBox *c0 = new QComboBox();
	l->addWidget(c0, 0, 1);
	c0->addItem("1st Set");
	c0->addItem("2st Set");
	c0->addItem("3st Set");
	c0->addItem("4st Set");
	c0->setCurrentIndex(0);
	l->addWidget(new QLabel("with"), 0, 2);
	QComboBox *c1 = new QComboBox();
	l->addWidget(c1, 0, 3);
	c1->addItem("1st Set");
	c1->addItem("2st Set");
	c1->addItem("3st Set");
	c1->addItem("4st Set");
	c1->setCurrentIndex(1);
	l->addWidget(new QLabel("?"), 0, 4);
	QHBoxLayout *bl = new QHBoxLayout();
	QPushButton *okBtn = new QPushButton("OK");
	okBtn->setDefault(true);
	QPushButton *cancelBtn = new QPushButton("Cancel");
	bl->addWidget(okBtn);
	bl->addWidget(cancelBtn);
	l->addLayout(bl, 1, 0, 1, 4);
	d.connect(okBtn, &QPushButton::clicked, &d, &QDialog::accept);
	d.connect(cancelBtn, &QPushButton::clicked, &d, &QDialog::reject);
	if (d.exec() != QDialog::Accepted)
		return;
	int32 a = c0->currentIndex();
	int32 b = c1->currentIndex();
	if (a == b)
		return;
	GetChip()->SwapTexcoords(a, b);
	QPushButton *A[4] = {ui.pushButton_del_tc0, ui.pushButton_del_tc1, ui.pushButton_del_tc2, ui.pushButton_del_tc3};
	QComboBox *B[4] = {ui.comboBox_str_tc0, ui.comboBox_str_tc1, ui.comboBox_str_tc2, ui.comboBox_str_tc3};
	QComboBox *C[4] = {ui.comboBox_comp_tc0, ui.comboBox_comp_tc1, ui.comboBox_comp_tc2, ui.comboBox_comp_tc3};
	bool t = A[a]->isEnabled();
	A[a]->setEnabled(A[b]->isEnabled());
	B[a]->setEnabled(B[b]->isEnabled());
	C[a]->setEnabled(C[b]->isEnabled());
	A[b]->setEnabled(t);
	B[b]->setEnabled(t);
	C[b]->setEnabled(t);
	int32 i = B[a]->currentIndex();
	B[a]->setCurrentIndex(B[b]->currentIndex());
	B[b]->setCurrentIndex(i);
	i = C[a]->currentIndex();
	C[a]->setCurrentIndex(C[b]->currentIndex());
	C[b]->setCurrentIndex(i);
	GetChip()->DestroyDeviceObjects();
	SetDirty();
}

void StdGeometry_Dlg::sphereMapFuncClicked()
{
	QApplication::clipboard()->setText("function...");
	QMessageBox::information(this, "Standard Drawable", "Spheremap transform decoding function is copied to the clipboard.\nPaste it into your shader code.");
}

void StdGeometry_Dlg::quatFuncClicked()
{
	QApplication::clipboard()->setText("function...");
	QMessageBox::information(this, "Standard Drawable", "Quaternion decoding function is copied to the clipboard.\nPaste it into your shader code.");
}

void StdGeometry_Dlg::packNormalsClicked()
{
	GetChip()->SetPackNormals(ui.checkBox_packNormals->isChecked());
	GetChip()->DestroyDeviceObjects();
	SetDirty();
}

void StdGeometry_Dlg::packTexSetsClicked()
{
	GetChip()->SetPackTextureSets(ui.checkBox_packTextureSets->isChecked());
	GetChip()->DestroyDeviceObjects();
	SetDirty();
}

void StdGeometry_Dlg::tscChanged()
{
	GetChip()->SetTangentSpaceCompression((StdGeometry::TangentSpaceCompression)ui.buttonGroup_tsc->checkedId());
	GetChip()->DestroyDeviceObjects();
	SetDirty();
}

void StdGeometry_Dlg::transformClicked()
{
	QDialog *d = new QDialog(this,Qt::MSWindowsFixedSizeDialogHint);
	d->setWindowTitle("Transform Drawable");
	QGridLayout *l = new QGridLayout(d);
	l->setSizeConstraint(QLayout::SetFixedSize);
	l->addWidget(new QLabel("Translate:"),1,0,1,1);
	l->addWidget(new QLabel("Rotate:"),2,0,1,1);
	l->addWidget(new QLabel("Scale:"),3,0,1,1);
	l->addWidget(new QLabel("x:"),0,1,1,1, Qt::AlignHCenter);
	l->addWidget(new QLabel("y:"),0,2,1,1, Qt::AlignHCenter);
	l->addWidget(new QLabel("z:"),0,3,1,1, Qt::AlignHCenter);
	QLineEdit *t[9];
	l->addWidget(t[0] = new QLineEdit("0.0"),1,1,1,1);
	l->addWidget(t[1] = new QLineEdit("0.0"),1,2,1,1);
	l->addWidget(t[2] = new QLineEdit("0.0"),1,3,1,1);
	l->addWidget(t[3] = new QLineEdit("0.0"),2,1,1,1);
	l->addWidget(t[4] = new QLineEdit("0.0"),2,2,1,1);
	l->addWidget(t[5] = new QLineEdit("0.0"),2,3,1,1);
	l->addWidget(t[6] = new QLineEdit("1.0"),3,1,1,1);
	l->addWidget(t[7] = new QLineEdit("1.0"),3,2,1,1);
	l->addWidget(t[8] = new QLineEdit("1.0"),3,3,1,1);
	for (int32 i = 0; i < 9; i++) t[i]->setAlignment(Qt::AlignRight);
	QDialogButtonBox *btns = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	l->addWidget(btns, 4, 0, 1, 4, Qt::AlignRight);
	connect(btns, &QDialogButtonBox::accepted, d, &QDialog::accept);
	connect(btns, &QDialogButtonBox::rejected, d, &QDialog::reject);
	t[0]->selectAll();
	while (true) {
		if (d->exec() == QDialog::Accepted) {
			float32 f[9];
			bool ok;
			for (int32 i = 0; i < 9; i++) {
				f[i] = t[i]->text().toFloat(&ok);
				if (!ok) {
					t[i]->setFocus();
					t[i]->selectAll();
					QMessageBox::warning(this, "Invalid Input", "Invalid number entered.");
					break;
				}
			}
			if (!ok)
				continue;
			XMMATRIX m = XMMatrixAffineTransformation(XMVectorSet(f[6],f[7],f[8],1.0f), XMVectorZero(), XMQuaternionRotationRollPitchYaw(f[3]*XM_PI/180.0f,f[4]*XM_PI/180.0f,f[5]*XM_PI/180.0f), XMVectorSet(f[0],f[1],f[2],1.0f));
			GetChip()->TransformPositions(m);
			GetChip()->TransformNormals(m);
			GetChip()->DestroyDeviceObjects();
			SetDirty();
		}
		break;
	}
	delete d;
}

int32 StdGeometry_Dlg::_getNumberOfActiveTexSets() const
{
	int32 n = 0;
	n += ui.comboBox_comp_tc0->currentIndex() > 0 && ui.comboBox_comp_tc0->isEnabled() ? 1 : 0;
	n += ui.comboBox_comp_tc1->currentIndex() > 0 && ui.comboBox_comp_tc1->isEnabled() ? 1 : 0;
	n += ui.comboBox_comp_tc2->currentIndex() > 0 && ui.comboBox_comp_tc2->isEnabled() ? 1 : 0;
	n += ui.comboBox_comp_tc3->currentIndex() > 0 && ui.comboBox_comp_tc3->isEnabled() ? 1 : 0;
	return n;
}

int32 StdGeometry_Dlg::_getNumberOfActiveNTB() const
{
	int32 n = 0;
	n += ui.comboBox_str_normals->currentIndex() > 0 && ui.comboBox_str_normals->isEnabled() ? 1 : 0;
	n += ui.comboBox_str_tangents->currentIndex() > 0 && ui.comboBox_str_tangents->isEnabled() ? 1 : 0;
	n += ui.comboBox_str_bitangents->currentIndex() > 0 && ui.comboBox_str_bitangents->isEnabled() ? 1 : 0;
	return n;
}

void StdGeometry_Dlg::_ntbEnable()
{
	int32 n = _getNumberOfActiveNTB();
	ui.checkBox_packNormals->setEnabled(n > 1 && (ui.radioButton_tsc3->isChecked() || ui.radioButton_tsc4->isChecked()));
	ui.groupBox_tsc->setEnabled(n > 0);
	ui.radioButton_tsc5->setEnabled(n > 1);
	ui.radioButton_tsc6->setEnabled(n > 1);
	ui.radioButton_tsc7->setEnabled(n > 1);
	ui.pushButton_quatFunc->setEnabled(n > 1);
}

void StdGeometry_Dlg::_texEnable()
{
	ui.checkBox_packTextureSets->setEnabled(_getNumberOfActiveTexSets() > 1);
	ui.pushButton_swapTexSet->setEnabled(ui.pushButton_del_tc0->isEnabled() || ui.pushButton_del_tc1->isEnabled() || ui.pushButton_del_tc2->isEnabled() || ui.pushButton_del_tc3->isEnabled());
}
