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
#include "SimpleDialogPage.h"


using namespace m3d;

RVariant::RVariant() : _ptr(new QVariant()) {}
RVariant::RVariant(const RVariant &rhs) : _ptr(new QVariant(*static_cast<const QVariant*>(rhs._ptr))) {}
RVariant::RVariant(RPtr ptr) : _ptr(new QVariant(*static_cast<QVariant*>(ptr))) {}
RVariant::RVariant(bool d) : _ptr(new QVariant(d)) {}
RVariant::RVariant(int32 d) : _ptr(new QVariant(d)) {}
RVariant::RVariant(uint32 d) : _ptr(new QVariant(d)) {}
RVariant::RVariant(int64 d) : _ptr(new QVariant(d)) {}
RVariant::RVariant(uint64 d) : _ptr(new QVariant(d)) {}
RVariant::RVariant(float32 d) : _ptr(new QVariant(d)) {}
RVariant::RVariant(float64 d) : _ptr(new QVariant(d)) {}
RVariant::RVariant(String d) : _ptr(new QVariant(TOQSTRING(d))) {}
RVariant::~RVariant() { delete static_cast<QVariant*>(_ptr); }
RVariant& RVariant::operator=(const RVariant& rhs)
{
	if (this != &rhs) {
		delete static_cast<QVariant*>(_ptr);
		_ptr = new QVariant(*static_cast<QVariant*>(rhs._ptr));
	}
	return *this;
}
bool RVariant::operator==(const RVariant& rhs) const { return *static_cast<QVariant*>(_ptr) == *static_cast<QVariant*>(rhs._ptr); }
bool RVariant::operator!=(const RVariant& rhs) const { return *static_cast<QVariant*>(_ptr) != *static_cast<QVariant*>(rhs._ptr); }
bool RVariant::IsNull() const { return static_cast<QVariant*>(_ptr)->isNull(); }
bool RVariant::IsValid() const { return static_cast<QVariant*>(_ptr)->isValid(); }
bool RVariant::ToBool() const { return static_cast<QVariant*>(_ptr)->toBool(); }
int32 RVariant::ToInt() const { return static_cast<QVariant*>(_ptr)->toInt(); }
uint32 RVariant::ToUInt() const { return static_cast<QVariant*>(_ptr)->toUInt(); }
int64 RVariant::ToInt64() const { return static_cast<QVariant*>(_ptr)->toLongLong(); }
uint64 RVariant::ToUInt64() const { return static_cast<QVariant*>(_ptr)->toULongLong(); }
float32 RVariant::ToFloat() const { return static_cast<QVariant*>(_ptr)->toFloat(); }
float64 RVariant::ToDouble() const { return static_cast<QVariant*>(_ptr)->toDouble(); }
String RVariant::ToString() const { return FROMQSTRING(static_cast<QVariant*>(_ptr)->toString()); }


class SimpleDialogPageWidget : public QWidget
{
public:
	SimpleDialogPageWidget() : _preferredWidth(0), _preferredHeight(0) {}
	~SimpleDialogPageWidget() {}

	QSize sizeHint() const override { return QSize(_preferredWidth, _preferredHeight); }

	int32 _preferredWidth;
	int32 _preferredHeight;
};


SimpleDialogPage::SimpleDialogPage()
{
	_ptr = new SimpleDialogPageWidget();
}

QWidget* SimpleDialogPage::GetWidget()
{
	return static_cast<QWidget*>(_ptr);
}

SimpleDialogPage::~SimpleDialogPage()
{
	delete static_cast<QWidget*>(_ptr);
}

void SimpleDialogPage::setPreferredSize(int32 w, int32 h)
{
	static_cast<SimpleDialogPageWidget*>(_ptr)->_preferredWidth = w;
	static_cast<SimpleDialogPageWidget*>(_ptr)->_preferredHeight = h;
}