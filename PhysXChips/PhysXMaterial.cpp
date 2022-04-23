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

#include "pch.h"
#include "PhysXMaterial.h"
#include "PhysXSDK.h"

using namespace m3d;


CHIPDESCV1_DEF(PhysXMaterial, MTEXT("PhysX Material"), PHYSXMATERIAL_GUID, CHIP_GUID);


PhysXMaterial::PhysXMaterial()
{
	_material = nullptr;
	_staticFriction = 0.5f;
	_dynamicFriction = 0.5f;
	_restitution = 0.1f;
	_disableFriction = false;
	_disableStrongFriction = false;
	_frictionCombineMode = PxCombineMode::eAVERAGE;
	_resitutionCombineMode = PxCombineMode::eAVERAGE;
}

PhysXMaterial::~PhysXMaterial()
{
	PX_RELEASE(_material)
}

bool PhysXMaterial::CopyChip(Chip *chip)
{
	PhysXMaterial *c = dynamic_cast<PhysXMaterial*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_staticFriction = c->_staticFriction;
	_dynamicFriction = c->_dynamicFriction;
	_restitution = c->_restitution;
	_disableFriction = c->_disableFriction;
	_disableStrongFriction = c->_disableStrongFriction;
	_frictionCombineMode = c->_frictionCombineMode;
	_resitutionCombineMode = c->_resitutionCombineMode;
	PX_RELEASE(_material)
	return true;
}

bool PhysXMaterial::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	LOADDEF("staticFriction", _staticFriction, 0.5f);
	LOADDEF("dynamicFriction", _dynamicFriction, 0.5f);
	LOADDEF("restitution", _restitution, 0.1f);
	LOADDEF("disableFriction", _disableFriction, false);
	LOADDEF("disableStrongFriction", _disableStrongFriction, false);
	LOADDEF("frictionCombineMode", (uint32&)_frictionCombineMode, PxCombineMode::eAVERAGE);
	LOADDEF("resitutionCombineMode", (uint32&)_resitutionCombineMode, PxCombineMode::eAVERAGE);
	PX_RELEASE(_material)
	return true;
}

bool PhysXMaterial::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVEDEF("staticFriction", _staticFriction, 0.5f);
	SAVEDEF("dynamicFriction", _dynamicFriction, 0.5f);
	SAVEDEF("restitution", _restitution, 0.1f);
	SAVEDEF("disableFriction", _disableFriction, false);
	SAVEDEF("disableStrongFriction", _disableStrongFriction, false);
	SAVEDEF("frictionCombineMode", (uint32)_frictionCombineMode, PxCombineMode::eAVERAGE);
	SAVEDEF("resitutionCombineMode", (uint32)_resitutionCombineMode, PxCombineMode::eAVERAGE);
	return true;
}

PxMaterial *PhysXMaterial::GetMaterial()
{
	if (_material)
		return _material;

	_material = physics()->createMaterial(_staticFriction, _dynamicFriction, _restitution);
	_material->setFlag(PxMaterialFlag::eDISABLE_FRICTION, _disableFriction);
	_material->setFlag(PxMaterialFlag::eDISABLE_STRONG_FRICTION, _disableStrongFriction);
	_material->setFrictionCombineMode(_frictionCombineMode);
	_material->setRestitutionCombineMode(_resitutionCombineMode);
	return _material;
}

float32 PhysXMaterial::GetStaticFriction() const
{
	PxMaterial *m = GetMaterialIfExist();
	if (m)
		return m->getStaticFriction();
	return _staticFriction;
}

float32 PhysXMaterial::GetDynamicFriction() const
{
	PxMaterial *m = GetMaterialIfExist();
	if (m)
		return m->getDynamicFriction();
	return _dynamicFriction;
}

float32 PhysXMaterial::GetRestitution() const
{
	PxMaterial *m = GetMaterialIfExist();
	if (m)
		return m->getRestitution();
	return _restitution;
}

bool PhysXMaterial::IsDisableFriction() const
{
	PxMaterial *m = GetMaterialIfExist();
	if (m)
		return m->getFlags() == PxMaterialFlag::eDISABLE_FRICTION;
	return _disableFriction;
}

bool PhysXMaterial::IsDisableStrongFriction() const
{
	PxMaterial *m = GetMaterialIfExist();
	if (m)
		return m->getFlags() == PxMaterialFlag::eDISABLE_STRONG_FRICTION;
	return _disableStrongFriction;
}

PxCombineMode::Enum PhysXMaterial::GetFrictionCombineMode() const
{
	PxMaterial *m = GetMaterialIfExist();
	if (m)
		return m->getFrictionCombineMode();
	return _frictionCombineMode;
}

PxCombineMode::Enum PhysXMaterial::GetRestitutionCombineMode() const
{
	PxMaterial *m = GetMaterialIfExist();
	if (m)
		return m->getRestitutionCombineMode();
	return _resitutionCombineMode;
}

void PhysXMaterial::SetStaticFriction(float32 v, bool chipSettings)
{
	PxMaterial *m = GetMaterialIfExist();
	if (m)
		m->setStaticFriction(v);
	if (chipSettings)
		_staticFriction = v;
}

void PhysXMaterial::SetDynamicFriction(float32 v, bool chipSettings)
{
	PxMaterial *m = GetMaterialIfExist();
	if (m)
		m->setDynamicFriction(v);
	if (chipSettings)
		_dynamicFriction = v;
}

void PhysXMaterial::SetRestitution(float32 v, bool chipSettings)
{
	PxMaterial *m = GetMaterialIfExist();
	if (m)
		m->setRestitution(v);
	if (chipSettings)
		_restitution = v;
}

void PhysXMaterial::SetDisableFriction(bool v, bool chipSettings)
{
	PxMaterial *m = GetMaterialIfExist();
	if (m)
		m->setFlag(PxMaterialFlag::eDISABLE_FRICTION, v);
	if (chipSettings)
		_disableFriction = v;
}

void PhysXMaterial::SetDisableStrongFriction(bool v, bool chipSettings)
{
	PxMaterial *m = GetMaterialIfExist();
	if (m)
		m->setFlag(PxMaterialFlag::eDISABLE_STRONG_FRICTION, v);
	if (chipSettings)
		_disableStrongFriction = v;
}

void PhysXMaterial::SetFrictionCombineMode(PxCombineMode::Enum v, bool chipSettings)
{
	PxMaterial *m = GetMaterialIfExist();
	if (m)
		m->setFrictionCombineMode(v);
	if (chipSettings)
		_frictionCombineMode = v;
}

void PhysXMaterial::SetRestitutionCombineMode(PxCombineMode::Enum v, bool chipSettings)
{
	PxMaterial *m = GetMaterialIfExist();
	if (m)
		m->setRestitutionCombineMode(v);
	if (chipSettings)
		_resitutionCombineMode = v;
}
