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
#include "TextureResampleDialog.h"
#include "M3DEngine/Engine.h"
#include "GraphicsChips/D3D12Formats.h"
#include "GraphicsChips/Graphics.h"

using namespace m3d;



TextureResampleDialog::TextureResampleDialog(QWidget *parent) : QDialog(parent)
{
	_layout = new QFormLayout(this);

	_w = new QLineEdit();
	_layout->addRow("Width:", _w);
	_h = new QLineEdit();
	_layout->addRow("Height:", _h);
	_ar = new QCheckBox();
	_layout->addRow("Keep Aspect Ratio", _ar);
	_d = new QLineEdit();
	_d->setEnabled(false);
	_layout->addRow("Depth:", _d);
	_a = new QLineEdit();
	_a->setEnabled(false);
	_layout->addRow("Array Size:", _a);

	connect(_w, &QLineEdit::textEdited, this, &TextureResampleDialog::onSizeChanged);
	connect(_h, &QLineEdit::textEdited, this, &TextureResampleDialog::onSizeChanged);

	_mips = new QComboBox();
	_layout->addRow("Mip Levels:", _mips);
	_mips->addItem("All", 0);
	_mips->addItem("High", -1);
	_mips->addItem("Medium", -2);
	_mips->addItem("Low", -3);
	_mips->addItem("Single", 1);
	for (uint32 i = 2, j = D3D12_REQ_MIP_LEVELS + 1; i < j; i++)
		_mips->addItem(QString::number(i), i);
	_fmt = new QComboBox();
	_layout->addRow("Formats:", _fmt);
	for (DXGI_FORMAT format = dxgiformat::NextFormat(DXGI_FORMAT_UNKNOWN); format != DXGI_FORMAT_UNKNOWN; format = dxgiformat::NextFormat(format)) {
		_fmt->addItem(dxgiformat::ToString(format), format);
		_fmt->setItemData(_fmt->count() - 1, dxgiformat::ToCommentString(format), Qt::ToolTipRole);
	}

	_srgb = new QCheckBox();
	_layout->addRow("sRGB Data Conversion:", _srgb);

	_rotate = new QComboBox();
	_layout->addRow("Rotate:", _rotate);
	_rotate->addItem("No", (UINT)TEX_FR_ROTATE0);
	_rotate->addItem("90 Degrees CW", (UINT)TEX_FR_ROTATE90);
	_rotate->addItem("180 Degrees", (UINT)TEX_FR_ROTATE180);
	_rotate->addItem("90 Degrees CCW", (UINT)TEX_FR_ROTATE270);

	_flipV = new QCheckBox();
	_layout->addRow("Flip Vertically:", _flipV);
	_flipH = new QCheckBox();
	_layout->addRow("Flip Horizontally:", _flipH);
	
//	QFrame *line = new QFrame();
//	line->setFrameShape(QFrame::HLine);
//	line->setFrameShadow(QFrame::Sunken);
//	_layout->addRow(line);
//	_layout->addRow(new QLabel("Tip: Use 'Single' Mip Levels to avoid internal DDS-conversion\nand increased file size!\nCheck 'Generate Mips' in Texture dialog to generate mips at run-time!"));

	_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	_layout->addWidget(_buttons);
     connect(_buttons, &QDialogButtonBox::accepted, this, &TextureResampleDialog::accept);
     connect(_buttons, &QDialogButtonBox::rejected, this, &TextureResampleDialog::reject);

	 _layout->setSizeConstraint( QLayout::SetFixedSize );

	 _aspectRatio = 1.0f;
}

TextureResampleDialog::~TextureResampleDialog()
{
}

int TextureResampleDialog::show(TextureConvertInfo &nfo, const DirectX::TexMetadata &meta)
{
	_w->setText(QString::number(nfo.width));
	_h->setText(QString::number(nfo.height));
	_d->setText(QString::number(meta.depth));
	_a->setText(QString::number(meta.arraySize));
	_mips->setCurrentIndex(_mips->findData(nfo.mipLevels));
	_ar->setChecked(true);

	_aspectRatio = float32(nfo.width) / nfo.height;

	_fmt->setCurrentIndex(_fmt->findData(nfo.format));
	_srgb->setChecked(false);

	_rotate->setCurrentIndex(_rotate->findData(uint32(nfo.flipRotate) & 0x3));
	_flipV->setChecked(nfo.flipRotate & TEX_FR_FLIP_VERTICAL);
	_flipH->setChecked(nfo.flipRotate & TEX_FR_FLIP_HORIZONTAL);

	int r = exec();

	if (r == QDialog::Accepted) {
		nfo.width = _w->text().toUInt();
		nfo.height = _h->text().toUInt();
		// TODO: _a and _d!??

		if (_mips->currentIndex() != -1) {
			int i = _mips->itemData(_mips->currentIndex()).toInt();
			if (i < 0) {
				float32 l = (float32)TextureUtil::CalculateMipLevels(meta);
				switch (i) {
					case -1: i = int(l * 0.75f); break; // High
					case -2: i = int(l * 0.5f); break;// Medium
					case -3: i = int(l * 0.25f); break; // Low
					default: i = 0; break;
				}
			}
			nfo.mipLevels = (size_t)i;
			nfo.mipFilter |= TEX_FILTER_FANT;
			nfo.mipFilter |= TEX_FILTER_SEPARATE_ALPHA;
		}

		if (_fmt->currentIndex() != -1) {
			DXGI_FORMAT format = (DXGI_FORMAT)_fmt->itemData(_fmt->currentIndex()).toUInt();
			if (format != nfo.format) {
				if (!_srgb->isChecked()) {
					if (dxgiformat::IsSRGB(nfo.format) && !dxgiformat::IsSRGB(format)) {
						nfo.filter |= TEX_FILTER_SRGB_OUT;
						nfo.compress |= TEX_COMPRESS_SRGB_OUT;
					}
					else if (!dxgiformat::IsSRGB(nfo.format) && dxgiformat::IsSRGB(format)) {
						nfo.filter |= TEX_FILTER_SRGB_IN;
						nfo.compress |= TEX_COMPRESS_SRGB_IN;
					}
				}
				nfo.format = format;
				
			}
		}
	
		nfo.flipRotate = (TEX_FR_FLAGS)(_rotate->itemData(_rotate->currentIndex()).toUInt() | (_flipV->isChecked() ? TEX_FR_FLIP_VERTICAL : 0) | (_flipH->isChecked() ? TEX_FR_FLIP_HORIZONTAL : 0));
	}
	return r;
}

void TextureResampleDialog::accept()
{
	UINT w, h, t;
	bool ok;
	w = _w->text().toUInt(&ok);
	if (!ok || w > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION || w == 0) {
		QMessageBox::critical(this, "Invalid Data", "Invalid texture width.");
		return;
	}
	h = _h->text().toUInt(&ok);
	if (!ok || h > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION || h == 0) {
		QMessageBox::critical(this, "Invalid Data", "Invalid texture height.");
		return;
	}
//D3D11_REQ_TEXTURECUBE_DIMENSION

	if (_fmt->currentIndex() != -1) {
		DXGI_FORMAT format = (DXGI_FORMAT)_fmt->itemData(_fmt->currentIndex()).toUInt();

		if (dxgiformat::IsCompressed(format) && (w % 4 && w != 1 && w != 2) && (h % 4 && h != 1 && h != 2)) {
			QMessageBox::critical(this, "Invalid Data", "When using texture compression, width and height must be multiple of 4.");
			return;
		}

		if (format == DXGI_FORMAT_BC6H_TYPELESS || format == DXGI_FORMAT_BC6H_UF16 ||format == DXGI_FORMAT_BC6H_SF16 ||format == DXGI_FORMAT_BC7_TYPELESS ||format == DXGI_FORMAT_BC7_UNORM ||format == DXGI_FORMAT_BC7_UNORM_SRGB)
			if (QMessageBox::question(this, "Question", "Converting to BC6/BC7 formats can be very slow. Do you want to continue?", QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
				return;
	}
	QDialog::accept();
}

void TextureResampleDialog::onSizeChanged()
{
	bool ok;
	if (!_ar->isChecked())
		return;
	if (sender() == _w) {
		uint32 i = _w->text().toUInt(&ok);
		if (!ok)
			return;
		_h->setText(QString::number(uint32(float32(i) / _aspectRatio)));
	}
	else {
		uint32 i = _h->text().toUInt(&ok);
		if (!ok)
			return;
		_w->setText(QString::number(uint32(float32(i) * _aspectRatio)));
	}
}