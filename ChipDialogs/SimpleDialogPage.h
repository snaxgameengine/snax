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
#include "DialogPage.h"



namespace m3d
{

typedef void* RPtr;


enum RSortOrder
{
	AscendingOrder,
	DescendingOrder
};

enum RCheckState 
{
	Unchecked,
	PartiallyChecked,
	Checked
};

typedef uint32 RData;


class CHIPDIALOGS_EXPORT RVariant
{
public:
	RVariant();
	RVariant(const RVariant& rhs);
	RVariant(RPtr ptr);
	RVariant(bool d);
	RVariant(int32 d);
	RVariant(uint32 d);
	RVariant(int64 d);
	RVariant(uint64 d);
	RVariant(float32 d);
	RVariant(float64 d);
	RVariant(String d);
	~RVariant();

	RVariant& operator=(const RVariant& rhs);
	bool operator==(const RVariant& rhs) const;
	bool operator!=(const RVariant& rhs) const;

	bool IsNull() const;
	bool IsValid() const;

	bool ToBool() const;
	int32 ToInt() const;
	uint32 ToUInt() const;
	int64 ToInt64() const;
	uint64 ToUInt64() const;
	float32 ToFloat() const;
	float64 ToDouble() const;
	String ToString() const;

	RPtr ptr() const { return _ptr; }
private:
	RPtr _ptr;
};

class CHIPDIALOGS_EXPORT SimpleDialogPage : public DialogPage
{
protected:
	SimpleDialogPage();
	~SimpleDialogPage();

	QWidget* GetWidget() override;

	void setPreferredSize(int32 w, int32 h);

	RPtr _ptr;
};

}