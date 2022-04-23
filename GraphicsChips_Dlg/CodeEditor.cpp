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

#include "StdAfx.h"
#include "CodeEditor.h"
#include "HLSLHighlighter.h"
#include <qpainter.h>

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
	_lineNumberArea = new LineNumberArea(this);
	_highlighter = new HLSLHighlighter(document());

	connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
	connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
	connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

	QFont font("Consolas");
	font.setPointSize(10);
	setFont(font);
	QString fdf = this->font().family();
	
	setTabStopDistance(fontMetrics().horizontalAdvance(" ") * 4);

	setWordWrapMode(QTextOption::NoWrap);

	updateLineNumberAreaWidth(0);
	highlightCurrentLine();
}


int CodeEditor::lineNumberAreaWidth()
{
	int digits = 1;
	int max = qMax(1, blockCount());
	while (max >= 10) {
		max /= 10;
		++digits;
	}

	int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

	return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
	if (dy)
		_lineNumberArea->scroll(0, dy);
	else
		_lineNumberArea->update(0, rect.y(), _lineNumberArea->width(), rect.height());

	if (rect.contains(viewport()->rect()))
		updateLineNumberAreaWidth(0);
}


void CodeEditor::resizeEvent(QResizeEvent *e)
{
	QPlainTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}


void CodeEditor::highlightCurrentLine()
{
	QList<QTextEdit::ExtraSelection> extraSelections;

	if (!isReadOnly()) {
		QTextEdit::ExtraSelection selection;

		QColor lineColor = QColor(Qt::yellow).lighter(160);

		selection.format.setBackground(lineColor);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
	}

	setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
	QPainter painter(_lineNumberArea);
	painter.fillRect(event->rect(), Qt::lightGray);

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int) blockBoundingRect(block).height();

	while (block.isValid() && top <= event->rect().bottom()) {
		if (block.isVisible() && bottom >= event->rect().top()) {
			QString number = QString::number(blockNumber + 1);
			painter.setPen(Qt::black);
			painter.drawText(0, top, _lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);
		}

		block = block.next();
		top = bottom;
		bottom = top + (int) blockBoundingRect(block).height();
		++blockNumber;
	}
}

void CodeEditor::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Tab) {
		if (textCursor().hasSelection() && textCursor().selectedText().contains(QChar::ParagraphSeparator)) {
			// multiline
			textCursor().beginEditBlock();
			QTextCursor cursor = textCursor();
			int a = cursor.anchor();
			int b = cursor.position();
			cursor.setPosition(std::min(a, b));
			cursor.movePosition(QTextCursor::StartOfBlock);
			while (cursor.position() < std::max(a, b)) {
				if (cursor.block().length() > 1) {
					setTextCursor(cursor);
					int c = cursor.position();
					cursor.insertText("\t");
					if (a > c)
						a++;
					if (b > c)
						b++;
					cursor = textCursor();
					cursor.movePosition(QTextCursor::StartOfBlock);
				}
				if (!cursor.movePosition(QTextCursor::Down))
					break;
			}
			cursor.setPosition(a);
			cursor.setPosition(b, QTextCursor::KeepAnchor);
			setTextCursor(cursor);
			textCursor().endEditBlock();
		}
		else
			QPlainTextEdit::keyPressEvent(e);
	}
	else if (e->key() == Qt::Key_Backtab) {
		textCursor().beginEditBlock();
		if (textCursor().hasSelection() && textCursor().selectedText().contains(QChar::ParagraphSeparator)) {
			// multiline..
			QTextCursor cursor = textCursor();
			int a = cursor.anchor();
			int b = cursor.position();
			cursor.setPosition(std::min(a, b));
			cursor.movePosition(QTextCursor::StartOfBlock);
			while (cursor.position() < std::max(a, b)) {
				setTextCursor(cursor);
				int c = cursor.position();
				int r = 0;
				while (r < 4) {
					QChar ch = document()->characterAt(textCursor().position());
					if (ch == '\t') {
						textCursor().deleteChar();
						r++;
						break;
					}
					else if (ch == ' ') {
						textCursor().deleteChar();
						r++;
					}
					else
						break;
				}
				if (a > c)
					a = std::max(c, a - r);
				if (b > c)
					b = std::max(c, b - r);
				cursor = textCursor();
				if (!cursor.movePosition(QTextCursor::Down))
					break;
			}
			cursor.setPosition(a);
			cursor.setPosition(b, QTextCursor::KeepAnchor);
			setTextCursor(cursor);
		}
		else {
			QTextCursor cursor = textCursor();
			int a = cursor.anchor();
			int b = cursor.position();
			cursor.setPosition(std::min(a, b));
			setTextCursor(cursor);
			if (textCursor().position() > 0) {
				int c = ((_getCursorColumn() - 1) / 4) * 4;
				while (textCursor().position() > 0 && _getCursorColumn() > c && (document()->characterAt(textCursor().position() - 1) == ' ' ||  document()->characterAt(textCursor().position() - 1) == '\t'))
					textCursor().deletePreviousChar();
			}
			cursor = textCursor();
			int c = std::min(a, b) - cursor.position();
			cursor.setPosition(a - c);
			cursor.setPosition(b - c, QTextCursor::KeepAnchor);
			setTextCursor(cursor);
		}
		textCursor().endEditBlock();
	}
	else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
		QPlainTextEdit::keyPressEvent(e);
		int i = _getPrevLineIndentation();
		if (i > 0)
			textCursor().insertText(_createIndentation(i));

		
		// Add intentation similar to previous line
		// Add 1 extra intentation if previous line:
		// - not a comment and ended with an ) or {
		// Remove 1 intentatioin if to do so!!!!!!
	}
	else if (e->key() == Qt::Key_BracketRight) {
		QPlainTextEdit::keyPressEvent(e);
		// Match { indentation?
	}
	else
		QPlainTextEdit::keyPressEvent(e);
}

int CodeEditor::_getCursorColumn()
{
	QTextCursor cursor = textCursor();
	int b = cursor.position();
	cursor.movePosition(QTextCursor::StartOfBlock);
	int a = cursor.position();
	int c = 0;
	while (a < b) {
		if (document()->characterAt(a++) == '\t')
			c = ((c + 4) / 4) * 4;
		else
			c++;
	}
	return c;
}

int CodeEditor::_getPrevLineIndentation()
{
	QTextCursor cursor = textCursor();
	cursor.movePosition(QTextCursor::StartOfLine);
	if (!cursor.movePosition(QTextCursor::Up))
		return 0;
	int a = cursor.position();
	int b = 0;
	while (true) {
		QChar c = document()->characterAt(a++);
		if (c == '\t')
			b += 4;
		else if (c == ' ')
			b += 1;
		else
			break;
	}
	return b;
}

QString CodeEditor::_createIndentation(int i)
{
	QString s;
	for (;i > 3; i -= 4)
		s += '\t';
	for (; i > 0; i--)
		s += ' ';
	return s;
}

