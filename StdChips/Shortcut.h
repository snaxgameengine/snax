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


static const Guid SHORTCUT_GUID = { 0xeae881a6, 0x83b5, 0x4eef, { 0xae, 0x7e, 0xcb, 0x18, 0x3c, 0x08, 0x39, 0x62 } };


class STDCHIPS_API Shortcut : public Chip, public DestructionObserver
{
	CHIPDESC_DECL;
public:
	Shortcut();
	virtual ~Shortcut();

	virtual const Guid &GetChipType() const override { return _original ? _original->GetChipType() : Chip::GetChipType(); }
	virtual ChipTypeIndex GetChipTypeIndex() const override { return _original ? _original->GetChipTypeIndex() : Chip::GetChipTypeIndex(); }

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual ChipChildPtr GetChip() override;

	virtual Chip *GetOriginal() const { return _original; }
	virtual void SetOriginal(Chip *c);

	virtual Shortcut *AsShortcut() override { return this; }

	// Only for visual feedback in editor!
	virtual String GetName() const override { return _original ? _original->GetName() : Chip::GetName(); }
	virtual bool HasMessages() const override { return _original ? _original->HasMessages() : Chip::HasMessages(); }
	virtual const ChipMessageList *GetMessages() const override { return _original ? _original->GetMessages() : Chip::GetMessages(); }
	virtual uint32 GetMessageHitCount(const ChipMessage &msg) const override { return _original ? _original->GetMessageHitCount(msg) : Chip::GetMessageHitCount(msg); }
	virtual String GetValueAsString() const override { return _original ? _original->GetValueAsString() : Chip::GetValueAsString(); }

	// Function methods.
	virtual bool CanCreateFunction() const override { return false; }//_original ? _original->CanCreateFunction() : false; }
//	virtual Function *CreateFunction() override{ return _original ? _original->CreateFunction() : nullptr; }
//	virtual Function *GetFunction() const override { return _original ? _original->GetFunction() : nullptr; }
//	virtual void RemoveFunction() override { if (_original) _original->RemoveFunction(); }


protected:
	Chip *_original;

	virtual void OnDestruction(DestructionObservable *observable) override;
};

}