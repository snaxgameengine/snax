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
#include "GraphicsChips/Shader.h"
#include "ui_Shader_Dlg.h"
#include <qtextdocument.h>
#include "HLSLHighlighter.h"

namespace m3d
{



class GRAPHICSCHIPS_DLG_EXPORT Shader_Dlg : public StandardDialogPage
{
	Q_OBJECT
	DIALOGDESC_DECL
public:
	Shader_Dlg();
	~Shader_Dlg();

	Shader*GetChip() { return (Shader*)DialogPage::GetChip(); }

	virtual void Init();

	virtual void OnOK();
	virtual void OnCancel();

	virtual QSize preferredSize() const { return QSize(800, 600); }


protected:
	bool eventFilter(QObject *obj, QEvent *event) override;

//	void _getShaderSource(Shaders::ShaderSource &s);

private:
	Ui::Shader_Dlg ui;

	struct 
	{
		ShaderType _st;
		bool _manualMode;
		String _source;
		SID3DBlob _byteCode;
		String _messages;
		UINT _compileFlags;
		ShaderModel _sm;
	} _init;

	QTextDocument *_shaderDocs[2];
	QSyntaxHighlighter *_highlighter[2];
	QTextCursor _cursors[2];
	int _vSliderPositions[2];
	int _hSliderPositions[2];
	int _currentPage;

	UINT _getShaderFlags();

private slots:
	void onCursorPositionChanged();
	void onCompile();
	void onCompileToC();
//	void editorUpdated();
	void onShaderTypeSelected();
	void onManualModeChanged();
	void shaderPageChanged();

};


}