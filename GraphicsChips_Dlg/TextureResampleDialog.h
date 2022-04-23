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


#include <QDialog>
#include "GraphicsChips/D3D12Include.h"
#include <qformlayout.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qdialogbuttonbox.h>
#include "TextureUtil.h"


namespace m3d
{


class TextureResampleDialog : public QDialog
{
	Q_OBJECT
public:
	TextureResampleDialog(QWidget *parent = 0);
	~TextureResampleDialog();

	int show(TextureConvertInfo &nfo, const DirectX::TexMetadata &meta);

	virtual void accept();

private:
	void onSizeChanged();

private:
	QFormLayout *_layout;
	QLineEdit *_w;
	QLineEdit *_h;
	QCheckBox *_ar;
	QLineEdit *_d;
	QLineEdit *_a;
	QComboBox *_mips;
	QComboBox *_fmt;
	QCheckBox *_srgb;
	QComboBox *_rotate;
	QCheckBox *_flipV;
	QCheckBox *_flipH;

	float32 _aspectRatio;

	QDialogButtonBox *_buttons;

};

}