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

#include <qlineedit.h>


class KeyboardListenerLineEdit : public QLineEdit
{
	Q_OBJECT
public:
	KeyboardListenerLineEdit(QWidget * parent = 0) : QLineEdit(parent) {}

protected:
	virtual void keyPressEvent (QKeyEvent *event);

	virtual void focusInEvent (QFocusEvent * event);
	virtual void focusOutEvent (QFocusEvent * event);

signals:
	void keyPressed(QKeyEvent *event);
};

const char *const VIRTUALKEYCODENAMES[255] = {
"...",
"LBUTTON",
"RBUTTON",
"CANCEL",
"MBUTTON",
"XBUTTON1",
"XBUTTON2",
"...",
"BACKSPACE",
"TAB",
"...",
"...",
"CLEAR",
"RETURN",
"...",
"...",
"SHIFT",
"CONTROL",
"ALT",
"PAUSE",
"CAPITAL",
"KANA",
"...",
"JUNJA",
"FINAL",
"HANJA",
"...",
"ESCAPE",
"CONVERT",
"NONCONVERT",
"ACCEPT",
"MODECHANGE",
"SPACEBAR",
"PAGE UP",
"PAGE DOWN",
"END",
"HOME",
"LEFT",
"UP",
"RIGHT",
"DOWN",
"SELECT",
"PRINT",
"EXECUTE",
"SNAPSHOT",
"INSERT",
"DELETE",
"HELP",
"0",
"1",
"2",
"3",
"4",
"5",
"6",
"7",
"8",
"9",
"...",
"...",
"...",
"...",
"...",
"...",
"...",
"A",
"B",
"C",
"D",
"E",
"F",
"G",
"H",
"I",
"J",
"K",
"L",
"M",
"N",
"O",
"P",
"Q",
"R",
"S",
"T",
"U",
"V",
"W",
"X",
"Y",
"Z",
"LEFT WIN",
"RIGHT WIN",
"APPS",
"...",
"SLEEP",
"NUMPAD 0",
"NUMPAD 1",
"NUMPAD 2",
"NUMPAD 3",
"NUMPAD 4",
"NUMPAD 5",
"NUMPAD 6",
"NUMPAD 7",
"NUMPAD 8",
"NUMPAD 9",
"NUMPAD *",
"NUMPAD +",
"SEPARATOR",
"NUMPAD -",
"NUMPAD ,",
"NUMPAD /",
"F1",
"F2",
"F3",
"F4",
"F5",
"F6",
"F7",
"F8",
"F9",
"F10",
"F11",
"F12",
"F13",
"F14",
"F15",
"F16",
"F17",
"F18",
"F19",
"F20",
"F21",
"F22",
"F23",
"F24",
"...", // Unassigned
"...", // Unassigned
"...", // Unassigned
"...", // Unassigned
"...", // Unassigned
"...", // Unassigned
"...", // Unassigned
"...", // Unassigned
"NUM LOCK",
"SCROLL LOCK",
"OEM_NEC_EQUAL", // OEM specific
"OEM_FJ_MASSHOU", // OEM specific
"OEM_FJ_TOUROKU", // OEM specific
"OEM_FJ_LOYA", // OEM specific
"OEM_FJ_ROYA", // OEM specific
"...", // Unassigned
"...", // Unassigned
"...", // Unassigned
"...", // Unassigned
"...", // Unassigned
"...", // Unassigned
"...", // Unassigned
"...", // Unassigned
"...", // Unassigned
"LEFT SHIFT",
"RIGHT SHIFT",
"LEFT CONTROL",
"RIGHT CONTROL",
"LMENU",
"RMENU",
"BROWSER BACK",
"BROWSER FORWARD",
"BROWSER REFRESH",
"BROWSER STOP",
"BROWSER SEARCH",
"BROWSER FAVORITES",
"BROWSER HOME",
"VOLUME MUTE",
"VOLUME DOWN",
"VOLUME UP",
"MEDIA NEXT TRACK",
"MEDIA PREV TRACK",
"MEDIA STOP",
"MEDIA PLAY PAUSE",
"LAUNCH MAIL",
"LAUNCH MEDIA SELECT",
"LAUNCH APP 1",
"LAUNCH APP 2",
"...", // Reserved
"...", // Reserved
u8"OEM 1: ;:\x00A8", // "OEM_1", // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ';:' key
"+", //"OEM_PLUS", // For any country/region, the '+' key
",", //"OEM_COMMA", // For any country/region, the ',' key
"-", //"OEM_MINUS", // For any country/region, the '-' key
".", //"OEM_PERIOD", // For any country/region, the '.' key
"OEM 2: /\'", //"OEM_2", // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '/?' key
u8"OEM 3: ~\x00D8", //"OEM_3", // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '`~' key
"...", // Reserved
"...", // Reserved
"...", // Reserved
"...", // Reserved
"...", // Reserved
"...", // Reserved
"...", // Reserved
"...", // Reserved
"...", // Reserved
"...", // Reserved
"...", // Reserved
"...", // Reserved
"...", // Reserved
"...", // Reserved
"...", // Reserved
"...", // Reserved
"...", // Reserved
"...", // Reserved
"...", // Reserved
"...", // Reserved
"...", // Reserved
"...", // Reserved
"...", // Unassigned
"...", // Unassigned
"...", // Unassigned
"OEM 4: [{", // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '[{' key
"OEM 5: \\|", //"OEM_5", // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '\|' key
"OEM 6: ]}|", //"OEM_6", // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ']}' key
u8"OEM 7: \'\"\x00c5",//"OEM_7", // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the 'single-quote/double-quote' key
u8"OEM 8: \x00c6", //"OEM_8", // Used for miscellaneous characters; it can vary by keyboard.
"...", // Reserved
"OEM_AX", // OEM specific
"OEM_102", // Either the angle bracket key or the backslash key on the RT 102-key keyboard
"<\\|", // "ICO_HELP", // OEM specific
"ICO_00", // OEM specific
"PROCESSKEY",
"ICO_CLEAR",
"PACKET",
"...",
"OEM_RESET",
"OEM_JUMP",
"OEM_PA1",
"OEM_PA2",
"OEM_PA3",
"OEM_WSCTRL",
"OEM_CUSEL",
"OEM_ATTN",
"OEM_FINISH",
"OEM_COPY",
"OEM_AUTO",
"OEM_ENLW",
"OEM_BACKTAB",
"ATTN",
"CRSEL",
"EXSEL",
"EREOF",
"PLAY",
"ZOOM",
"NONAME",
"PA1",
"OEM_CLEAR",
"..."};