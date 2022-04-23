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
#include "Shader_Dlg.h"
#include "GraphicsChips/RenderSettings.h"

using namespace m3d;


DIALOGDESC_DEF(Shader_Dlg, SHADER_GUID);


Shader_Dlg::Shader_Dlg() : _currentPage(0)
{
	ui.setupUi(this);

	ui.plainTextEdit_msg->viewport()->installEventFilter(this);

	ui.comboBox_shaderType->addItem("Select Shader Type", (uint32)ShaderType::UNDEFINED);
	ui.comboBox_shaderType->addItem("Vertex Shader", (uint32)ShaderType::VS);
	ui.comboBox_shaderType->addItem("Hull Shader", (uint32)ShaderType::HS);
	ui.comboBox_shaderType->addItem("Domain Shader", (uint32)ShaderType::DS);
	ui.comboBox_shaderType->addItem("Geometry Shader", (uint32)ShaderType::GS);
	ui.comboBox_shaderType->addItem("Pixel Shader", (uint32)ShaderType::PS);
	ui.comboBox_shaderType->addItem("Compute Shader", (uint32)ShaderType::CS);

	ui.comboBox_shaderModel->addItem("Shader Model 4.0 9.1", (uint32)ShaderModel::SM40_91);
	ui.comboBox_shaderModel->addItem("Shader Model 4.0 9.3", (uint32)ShaderModel::SM40_93);
	ui.comboBox_shaderModel->addItem("Shader Model 4.0", (uint32)ShaderModel::SM40);
	ui.comboBox_shaderModel->addItem("Shader Model 4.1", (uint32)ShaderModel::SM41);
	ui.comboBox_shaderModel->addItem("Shader Model 5.0", (uint32)ShaderModel::SM50);
	ui.comboBox_shaderModel->addItem("Shader Model 5.1", (uint32)ShaderModel::SM51);

	_currentPage = ui.tabWidget_shaderPages->currentIndex();
	for (uint32 i = 0; i < 2; i++) {
		_shaderDocs[i] = new QTextDocument(this);
		_shaderDocs[i]->setDocumentLayout(new QPlainTextDocumentLayout(_shaderDocs[i]));
		QString dd = ui.plainTextEdit_editor->font().family();
		_shaderDocs[i]->setDefaultFont(ui.plainTextEdit_editor->font());
		_highlighter[i] = new HLSLHighlighter(_shaderDocs[i]);
		ui.plainTextEdit_editor->blockSignals(true);
		ui.plainTextEdit_editor->setDocument(_shaderDocs[i]);
		ui.plainTextEdit_editor->blockSignals(false);
		_cursors[i] = ui.plainTextEdit_editor->textCursor();
		_vSliderPositions[i] = 0;
		_hSliderPositions[i] = 0;
	}
	ui.plainTextEdit_editor->setDocument(_shaderDocs[_currentPage]);

	// For now...
	//ui.checkBox_manualMode->setVisible(false);
}

Shader_Dlg::~Shader_Dlg()
{
}

void Shader_Dlg::Init()
{
	_init._st = GetChip()->GetShaderType();
	_init._manualMode = GetChip()->IsManualMode();
	_init._source = GetChip()->GetShaderSource();
	_init._byteCode = GetChip()->GetByteCode();
	_init._messages = GetChip()->GetCompileMessages();
	_init._compileFlags = GetChip()->GetCompileFlags();
	_init._sm = GetChip()->GetShaderModel();

	_shaderDocs[0]->setPlainText(STD_TEXT);
	_shaderDocs[1]->setPlainText(_init._source.c_str());

	ui.plainTextEdit_msg->setPlainText(_init._messages.c_str());
	ui.comboBox_shaderType->setCurrentIndex(ui.comboBox_shaderType->findData((uint32)_init._st));
	ui.comboBox_shaderModel->setCurrentIndex(ui.comboBox_shaderModel->findData((uint32)_init._sm));
	//ui.checkBox_manualMode->setChecked(_init._manualMode);

	ui.tabWidget_shaderPages->setCurrentIndex(1);

	QTextCursor c = ui.plainTextEdit_editor->textCursor();
	c.movePosition(QTextCursor::Start);
	ui.plainTextEdit_editor->setTextCursor(c);
}

void Shader_Dlg::OnOK()
{
	ShaderType st = (ShaderType)ui.comboBox_shaderType->currentData().toUInt();
	ShaderModel sm = (ShaderModel)ui.comboBox_shaderModel->currentData().toUInt();
	UINT flags = _getShaderFlags();
	bool mm = true;// ui.checkBox_manualMode->isChecked();
	String src = FROMQSTRING(_shaderDocs[1]->toPlainText());

	Shader *c = GetChip();

	c->SetShaderType(st);
	c->SetCompileFlags(flags);
	c->SetShaderModel(sm);
	c->SetManualMode(mm);
	if (mm)
		c->SetShaderSource(src);
}

void Shader_Dlg::OnCancel()
{
	if (IsEdited()) {
		GetChip()->SetShaderType(_init._st);
		GetChip()->SetManualMode(_init._manualMode);
		GetChip()->SetShaderSource(_init._source);
		GetChip()->SetByteCode(_init._byteCode);
		GetChip()->SetCompileMessages(_init._messages);
		GetChip()->SetCompileFlags(_init._compileFlags);
		GetChip()->SetShaderModel(_init._sm);
	}
}

// Position of cursor changed in editor.
void Shader_Dlg::onCursorPositionChanged()
{
	ui.label_line->setText(QString("Ln ") + QString::number(ui.plainTextEdit_editor->textCursor().blockNumber() + 1));
	ui.label_character->setText(QString("Ch ") + QString::number(ui.plainTextEdit_editor->textCursor().columnNumber() + 1));
}

void Shader_Dlg::onShaderTypeSelected()
{
	bool b = /*ui.checkBox_manualMode->isChecked() &&*/ ui.comboBox_shaderType->currentData().toUInt() != (uint32)ShaderType::UNDEFINED;
	ui.pushButton_compile->setEnabled(b);
	ui.pushButton_compileC->setEnabled(b);
}

void Shader_Dlg::onManualModeChanged()
{
	bool b = /*ui.checkBox_manualMode->isChecked() &&*/ ui.comboBox_shaderType->currentData().toUInt() != (uint32)ShaderType::UNDEFINED;
	ui.pushButton_compile->setEnabled(b);
	ui.pushButton_compileC->setEnabled(b);
}

void Shader_Dlg::onCompile()
{
	//if (!ui.checkBox_manualMode->isChecked())
		//return;

	ShaderType st = (ShaderType)ui.comboBox_shaderType->currentData().toUInt();
	if (st == ShaderType::UNDEFINED) {
		QMessageBox::warning(this, M3D_TITLE, "Please select the shader type before compiling.");
		return;
	}
	
	UINT flags = _getShaderFlags();
	ShaderModel sm = (ShaderModel)ui.comboBox_shaderModel->currentData().toUInt();

	String src = FROMQSTRING(_shaderDocs[1]->toPlainText());

	Shader *c = GetChip();

	c->SetShaderType(st);
	c->SetCompileFlags(flags);
	c->SetShaderModel(sm);
	c->SetManualMode(true);
	c->SetShaderSource(src);

	c->Compile();

	String msg = c->GetCompileMessages();
	ui.plainTextEdit_msg->setPlainText(msg.c_str());
	SetDirty();
}

// User changed to another shader page.
void Shader_Dlg::shaderPageChanged()
{
	_cursors[_currentPage] = ui.plainTextEdit_editor->textCursor();
	_vSliderPositions[_currentPage] = ui.plainTextEdit_editor->verticalScrollBar()->sliderPosition();
	_hSliderPositions[_currentPage] = ui.plainTextEdit_editor->horizontalScrollBar()->sliderPosition();
	_currentPage = ui.tabWidget_shaderPages->currentIndex();
	ui.plainTextEdit_editor->blockSignals(true);
	ui.plainTextEdit_editor->setDocument(_shaderDocs[_currentPage]);
	ui.plainTextEdit_editor->blockSignals(false);
	ui.plainTextEdit_editor->setTextCursor(_cursors[_currentPage]);
	ui.plainTextEdit_editor->setTabStopDistance(ui.plainTextEdit_editor->fontMetrics().horizontalAdvance(" ") * 4);
	ui.plainTextEdit_editor->setReadOnly(_currentPage == 0);// || !ui.checkBox_manualMode->isChecked());
	ui.plainTextEdit_editor->verticalScrollBar()->setSliderPosition(_vSliderPositions[_currentPage]);
	ui.plainTextEdit_editor->horizontalScrollBar()->setSliderPosition(_hSliderPositions[_currentPage]);
	ui.pushButton_undo->setEnabled(ui.plainTextEdit_editor->document()->isUndoAvailable());
	ui.pushButton_redo->setEnabled(ui.plainTextEdit_editor->document()->isRedoAvailable());
}

UINT Shader_Dlg::_getShaderFlags()
{
	UINT flags = 0;
	if (ui.checkBox_debug->isChecked())
		flags |= D3DCOMPILE_DEBUG;
	if (ui.checkBox_forceHalf->isChecked())
		flags |= D3DCOMPILE_PARTIAL_PRECISION;
	if (ui.comboBox_flowControl->currentIndex() == 1)
		flags |= D3DCOMPILE_PREFER_FLOW_CONTROL;
	else if (ui.comboBox_flowControl->currentIndex() == 2)
		flags |= D3DCOMPILE_AVOID_FLOW_CONTROL;
	if (ui.comboBox_optimization->currentIndex() == 0)
		flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	else if (ui.comboBox_optimization->currentIndex() == 1)
		flags |= D3DCOMPILE_OPTIMIZATION_LEVEL0;
	else if (ui.comboBox_optimization->currentIndex() == 2)
		flags |= D3DCOMPILE_OPTIMIZATION_LEVEL1;
	else if (ui.comboBox_optimization->currentIndex() == 3)
		flags |= D3DCOMPILE_OPTIMIZATION_LEVEL2;
	else if (ui.comboBox_optimization->currentIndex() == 4)
		flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;

	return flags;
}

bool Shader_Dlg::eventFilter(QObject *obj, QEvent *event)
{
	// Move cursor to error.
	if (obj == ui.plainTextEdit_msg->viewport() && event->type() == QEvent::MouseButtonDblClick) {
		QTextCursor c = ui.plainTextEdit_msg->cursorForPosition(((QMouseEvent*)event)->pos());
		QString s = c.block().text();
		static QRegularExpression r("^([_a-zA-Z0-9]+)[ ]*\\((\\d+),(\\d+)(-(\\d+))?\\)"); // capture: Page(1,2), Page (1,2-3)
		QRegularExpressionMatch m = r.match(s);
		if (m.hasMatch() && m.lastCapturedIndex() >= 3) {
			QString page = m.captured(1);
			int s1 = m.captured(2).toInt();
			int s2 = m.captured(3).toInt();
			int s3 = m.lastCapturedIndex() == 5 ? m.captured(5).toInt() : s2;
			static QString pages[2] = { "Std", "Main" };
			for (uint32 i = 0; i < 2; i++) {
				if (pages[i] == page) {
					ui.tabWidget_shaderPages->setCurrentIndex(i);
					ui.plainTextEdit_editor->setFocus();
					QTextCursor c = ui.plainTextEdit_editor->textCursor();
					c.movePosition(QTextCursor::Start);
					c.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, s1 - 1);
					c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, s2 - 1);
					if (s3 > s2)
						c.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, s3 - s2 + 1);
					ui.plainTextEdit_editor->setTextCursor(c);
				}
			}
		}
	}
	return false;
}

void Shader_Dlg::onCompileToC()
{
	//if (!ui.checkBox_manualMode->isChecked())
//		return;

	ShaderType st = (ShaderType)ui.comboBox_shaderType->currentData().toUInt();
	if (st == ShaderType::UNDEFINED) {
		QMessageBox::warning(this, M3D_TITLE, "Please select the shader type before compiling.");
		return;
	}

	UINT flags = _getShaderFlags();
	ShaderModel sm = (ShaderModel)ui.comboBox_shaderModel->currentData().toUInt();

	String src = FROMQSTRING(_shaderDocs[1]->toPlainText());

	String msg;

	SID3DBlob bc;

	HRESULT hr = Shader::Compile(&bc, st, sm, flags, src, msg);

	if (SUCCEEDED(hr)) {
		QString s = QString("const uint8 HLSL_BYTECODE[%1] =\n{\n\t").arg((uint32)bc->GetBufferSize());
		String t;
		const uint8 *C = (const uint8*)bc->GetBufferPointer();
		for (size_t i = 0, j = bc->GetBufferSize(); i < j;) {
			for (size_t k = std::min(i + 20, j); i < k; i++) {
				s += QString("0x%1, ").arg((uint32)C[i], 2, 16, QChar('0'));
			}
			if (i < j - 1)
				s += "\n\t";
		}
		s.remove(s.size() - 2, 2);
		s += "\n};";

		QApplication::clipboard()->setText(s);

		msg += String("\nHLSL Byte Code successfully copied to clipboard as a C-array string.\n\n");
	}
	else {
		msg += "\nCompile FAILED\n\n";
	}

	ui.plainTextEdit_msg->setPlainText(msg.c_str());
}