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
#include "PhysXSDK.h"


namespace m3d
{


static const Guid PHYSXMATERIAL_GUID = { 0xfd80fcef, 0x652f, 0x4ac7, { 0xbe, 0x54, 0x18, 0x0d, 0xc4, 0xfa, 0xcd, 0x2e } };



class PHYSXCHIPS_API PhysXMaterial : public Chip, public PhysXUsage
{
	CHIPDESC_DECL;
public:
	PhysXMaterial();
	virtual ~PhysXMaterial();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual PxMaterial *GetMaterial();
	virtual PxMaterial *GetMaterialIfExist() const { return _material; }

	virtual float32 GetStaticFriction() const;
	virtual float32 GetDynamicFriction() const;
	virtual float32 GetRestitution() const;
	virtual bool IsDisableFriction() const;
	virtual bool IsDisableStrongFriction() const;
	virtual PxCombineMode::Enum GetFrictionCombineMode() const;
	virtual PxCombineMode::Enum GetRestitutionCombineMode() const;

	virtual void SetStaticFriction(float32 v, bool chipSettings = true);
	virtual void SetDynamicFriction(float32 v, bool chipSettings = true);
	virtual void SetRestitution(float32 v, bool chipSettings = true);
	virtual void SetDisableFriction(bool v, bool chipSettings = true);
	virtual void SetDisableStrongFriction(bool v, bool chipSettings = true);
	virtual void SetFrictionCombineMode(PxCombineMode::Enum v, bool chipSettings = true);
	virtual void SetRestitutionCombineMode(PxCombineMode::Enum v, bool chipSettings = true);

protected:
	PxMaterial *_material;

	float32 _staticFriction;
	float32 _dynamicFriction;
	float32 _restitution;
	bool _disableFriction;
	bool _disableStrongFriction;
	PxCombineMode::Enum _frictionCombineMode;
	PxCombineMode::Enum _resitutionCombineMode;
};



}