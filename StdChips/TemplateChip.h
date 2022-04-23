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
#include "ProxyChip.h"


namespace m3d
{


static const Guid TEMPLATECHIP_GUID = { 0xcad1fece, 0xcc41, 0x483f, { 0x8d, 0x77, 0x48, 0xc1, 0xcf, 0x81, 0x51, 0x33 } };

class STDCHIPS_API TemplateChip : public ProxyChip
{
	CHIPDESC_DECL;
public:
	TemplateChip();
	virtual ~TemplateChip();

	// If the template is set, we can override the ChipType to that of the Template. 
	// This enable us to use templates of type ProxyChip like Switch, FunctionCall etc.
	const Guid &GetChipType() const override { return _template ? _template->GetChipType() : ProxyChip::GetChipType(); }
	ChipTypeIndex GetChipTypeIndex() const override { return _template ? _template->GetChipTypeIndex() : ProxyChip::GetChipTypeIndex(); }

	ChipChildPtr GetChip() override;

	bool CopyChip(Chip *chip) override;
	bool LoadChip(DocumentLoader &loader) override;
	bool SaveChip(DocumentSaver &saver) const override;

	void SetClass(Class* clazz) override;

	// Gets the template for this chip. If it does not exist, CreateTemplate(nullptr) is called to create it.
	virtual Chip *GetTemplate();
	// Creates the template, possibly copies it from the given chip.
	virtual bool CreateTemplate(Chip *copyTemplateFrom = nullptr);
	// Create a new chip based on the template. It is up to the caller to release it. 
	// If template does not exist, CreateTemplate(nullptr) is called to create it.
	// NOTE: CopyChip() is called on the returned chip. No InitChip() required! Child provider is set to _template!
	// Remember to set class and owner!
	virtual Chip *CreateChipFromTemplate();

	virtual void OnDestroyDevice() override;
	virtual void OnReleasingBackBuffer(RenderWindow *rw) override;
	virtual void RestoreChip() override;
	virtual void AddDependencies(ProjectDependencies &deps) override;
	virtual Chip *FindChip(ChipID chipID) override;

	virtual void SetConnection(uint32 index, const ChildConnectionDesc& connection, bool keepChildren = false) override;

private:
	Chip *_template;

protected:
	virtual const ChildConnectionList &ProvideChildren() const;


};



}