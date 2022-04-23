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


static const Guid CONTAINERCHIP_GUID = { 0x90732a7f, 0xfaf1, 0x46fe, { 0xbb, 0x43, 0x76, 0x9a, 0x1b, 0xa7, 0x48, 0xc7 } };


class STDCHIPS_API ContainerChip : public Chip
{
	CHIPDESC_DECL;
	CHIPMSG(IndexOutOfBoundsException, FATAL, MTEXT("Container index out of bounds!"))
public:
	ContainerChip();
	virtual ~ContainerChip();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual uint32 GetContainerSize() { return 0; }
	virtual void SetContainerSize(uint32 size) {}
	virtual void ClearContainer() {}

	virtual bool IsSaveContent() const { return _saveContent; }
	virtual void SetSaveContent(bool b) { _saveContent = b; }

private:
	bool _saveContent;
};

}