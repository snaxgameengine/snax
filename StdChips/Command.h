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
#include "M3DEngine/Chip.h"

namespace m3d
{


static const Guid COMMAND_GUID = { 0x03a46761, 0xca4d, 0x42be, { 0x9d, 0xa0, 0x4f, 0xb3, 0xf6, 0x69, 0xc1, 0x93 } };

class STDCHIPS_API Command : public Chip
{
	CHIPDESC_DECL;
public:
	Command();
	virtual ~Command();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual void CallChip() override;

	enum class OperatorType 
	{ 
		NONE, 
		SLEEP,
		SET_WINDOW_PROPS,
		POST_MESSAGE,
		SET_APP_NAME,
		QUIT,
		CREATE_INSTANCE = 100,
		CLEAR_INSTANCE_REF,
		COPY_INSTANCE_REF,
		MAKE_OWNER,
		VALUEARRAY_SORT = 150,
		INSTANCEARRAY_SORT,
		RESET_REFRESH_MANAGER = 200,
		COPY_CHIP,
		ENVELOPE_ADD_STEP_CP = 300,
		ENVELOPE_ADD_LINEAR_CP,
		ENVELOPE_ADD_TCB_CP,
		ENVELOPE_REMOVE_CP,
		ENVELOPE_CLEAR,
		CONTAINER_CLEAR = 400,
		VALUESET_REMOVE_KEY,
		TEXTSET_REMOVE_KEY
	};

	virtual OperatorType GetOperatorType() const { return _ot; }
	virtual void SetOperatorType(OperatorType ot);

protected:
	OperatorType _ot;

};



}