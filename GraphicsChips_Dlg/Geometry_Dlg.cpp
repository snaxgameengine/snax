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
#include "Geometry_Dlg.h"
#include "GraphicsChips/D3D12Formats.h"
#include "GraphicsChips/PipelineStatePool.h"

using namespace m3d;


DIALOGDESC_DEF(Geometry_Dlg, GEOMETRY_GUID);


class NoEditDelegate: public QStyledItemDelegate 
{
public:
	NoEditDelegate(QObject* parent=0): QStyledItemDelegate(parent) {}
	virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const { return 0; }
};


Geometry_Dlg::Geometry_Dlg()
{
	ui.setupUi(this);
	NoEditDelegate *d = new NoEditDelegate(ui.treeWidget_subsets);
	ui.treeWidget_subsets->setItemDelegateForColumn(1, d);
	ui.treeWidget_subsets->setItemDelegateForColumn(2, d);
	ui.treeWidget_subsets->setItemDelegateForColumn(3, d);
	ui.treeWidget_subsets->setItemDelegateForColumn(4, d);
	ui.treeWidget_subsets->setItemDelegateForColumn(5, d);
}

Geometry_Dlg::~Geometry_Dlg()
{
}

QString _toString(D3D_PRIMITIVE_TOPOLOGY pt)
{
	switch (pt)
	{
	case D3D_PRIMITIVE_TOPOLOGY_UNDEFINED: return "Undefined";
	case D3D_PRIMITIVE_TOPOLOGY_POINTLIST: return "Point List";
	case D3D_PRIMITIVE_TOPOLOGY_LINELIST: return "Line List";
	case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP: return "Line Strip";
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST: return "Triangle List";
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: return "Triangle Strip";
	case D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ: return "Line List (Adj)";
	case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ: return "Line Strip (Adj)";
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ: return "Triangle List (Adj)";
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ: return "Triangle Strip (Adj)";
	case D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST: return "1 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST: return "2 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST: return "3 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST: return "4 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST: return "5 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST: return "6 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST: return "7 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST: return "8 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST: return "9 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST: return "10 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST: return "11 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST: return "12 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST: return "13 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST: return "14 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST: return "15 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST: return "16 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST: return "17 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST: return "18 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST: return "19 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST: return "20 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST: return "21 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST: return "22 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST: return "23 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST: return "24 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST: return "25 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST: return "26 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST: return "27 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST: return "28 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST: return "29 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST: return "30 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST: return "31 Control Point Patchlist";
	case D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST: return "32 Control Point Patchlist";
	}
	return "Invalid";
}

QString _toString(DXGI_FORMAT fmt)
{
	return dxgiformat::ToString(fmt);
}

QString _toString(D3D12_INPUT_CLASSIFICATION i)
{
	switch (i)
	{
	case D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA: return "Pr Vertex Data";
	case D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA: return "Pr Instance Data";
	}
	return "Invalid";
}


void Geometry_Dlg::Init()
{
	ui.widget_preview->_geometry = GetChip();
}

void Geometry_Dlg::OnEnter()
{
	// This happens just before the page is visible!

	Geometry *d = GetChip();

	if (!d->IsGPUInit())
		d->CreateDeviceObjects();

	ui.treeWidget_subsets->blockSignals(true);
	ui.treeWidget_subsets->clear();
	ui.listWidget_subsets->clear();
	const GeometrySubsetList &ssl = d->GetSubsets();
	for (size_t i = 0; i < ssl.size(); i++) {
		QStringList sl;
		QString bb;
		if (ssl[i].boundingBox.IsInfinite())
			bb = "Infinite";
		else
			bb = QString("{%1;%2;%3} - {%4;%5;%6}").arg(ssl[i].boundingBox.GetMin().x).arg(ssl[i].boundingBox.GetMin().y).arg(ssl[i].boundingBox.GetMin().z).arg(ssl[i].boundingBox.GetMax().x).arg(ssl[i].boundingBox.GetMax().y).arg(ssl[i].boundingBox.GetMax().z);
		sl << TOQSTRING(ssl[i].name) << _toString((D3D12_PRIMITIVE_TOPOLOGY) ssl[i].pt) << QString::number(ssl[i].count) << QString::number(ssl[i].startLocation) << QString::number(ssl[i].baseVertexLocation) << bb;
		QTreeWidgetItem *itm = new QTreeWidgetItem(ui.treeWidget_subsets, sl);
		itm->setFlags(itm->flags() | Qt::ItemIsEditable);
		itm->setData(0, Qt::UserRole, i);
		QListWidgetItem *itm2 = new QListWidgetItem(TOQSTRING(ssl[i].name));
		itm2->setFlags(itm2->flags() | Qt::ItemIsEditable);
		itm2->setData(Qt::UserRole, i);
		ui.listWidget_subsets->addItem(itm2);
	}
	ui.treeWidget_subsets->blockSignals(false);

	ui.listWidget_subsets->selectAll();

	_updateGPUItems();
}

void Geometry_Dlg::_updateGPUItems()
{
	Geometry *d = GetChip();
	ui.treeWidget_inputLayout->clear();
	if (d->IsGPUInit()) {
		uint32 vertexSize = 0;
		const PipelineInputStateDesc *descs = d->graphics()->GetPipelineStatePool()->GetPipelineInputStateDesc(d->GetPipelineInputStateDescID());
		if (descs) {
			for (UINT i = 0; i < descs->InputLayout.NumElements; i++) {
				const D3D12_INPUT_ELEMENT_DESC &ied = descs->InputLayout.pInputElementDescs[i];
				vertexSize += dxgiformat::BitsPerPixel(ied.Format);
				QStringList sl;
				sl << (ied.SemanticName ? ied.SemanticName : "") << QString::number(ied.SemanticIndex) << _toString(ied.Format) << QString::number((int)ied.InputSlot) << QString::number((int)ied.AlignedByteOffset) << _toString(ied.InputSlotClass) << QString::number((int)ied.InstanceDataStepRate);
				QTreeWidgetItem *itm = new QTreeWidgetItem(ui.treeWidget_inputLayout, sl);
			}
			ui.label_vertexSize->setText(QString::number(vertexSize / 8) + " Bytes");
			ui.groupBox_inputLayout->setEnabled(true);
		}
	}
	else {
		ui.label_vertexSize->setText("N/A");
		ui.groupBox_inputLayout->setEnabled(false);
	}
	ui.label_drawAPI->setText(d->GetAPI() == DRAW ? "Draw" : "Draw Indexed");
	ui.label_indexFormat->setText(d->GetAPI() == DRAW ? "N/A" : (d->GetIndexBufferView().Format == DXGI_FORMAT_UNKNOWN ? "Unknown" : (QString::number(dxgiformat::BitsPerPixel(d->GetIndexBufferView().Format)) + "-Bits")));
}

void Geometry_Dlg::ssItemChanged(QTreeWidgetItem *itm,int c)
{
	if (c != 0)
		return;
	QString s = itm->text(0);
	uint32 i = itm->data(0, Qt::UserRole).toUInt();
	if (i < GetChip()->GetSubsets().size()) {
		GeometrySubset ss = GetChip()->GetSubsets()[i];
		ss.name = FROMQSTRING(s);
		GetChip()->SetSubset(ss, i);
		SetDirty();
		ui.listWidget_subsets->blockSignals(true);
		ui.listWidget_subsets->item(i)->setText(s);
		ui.listWidget_subsets->blockSignals(false);
	}
}

void Geometry_Dlg::tabChanged()
{
	if (ui.tabWidget->currentIndex() == 0) {
	}
	else {
	}
}

void Geometry_Dlg::Update()
{
	ui.widget_preview->invalidateGraphics();
}

void Geometry_Dlg::ss2SelectionChanged()
{
	ui.widget_preview->_subsets.clear();
	for (int i = 0; i < ui.listWidget_subsets->count(); i++)
		if (ui.listWidget_subsets->item(i)->isSelected())
			ui.widget_preview->_subsets.insert(i);
}

void Geometry_Dlg::ss2ItemChanged(QListWidgetItem*itm)
{
	QTreeWidgetItem *itm2 = ui.treeWidget_subsets->topLevelItem(itm->data(Qt::UserRole).toUInt());
	if (itm2)
		itm2->setText(0, itm->text()); // This will trigger event for setting subset text!
}