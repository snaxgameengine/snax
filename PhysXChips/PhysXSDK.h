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
#include <extensions\PxExtensionsAPI.h>
//#include <task\PxTask.h>

using namespace physx;

namespace m3d
{


static const Guid PHYSXSDK_GUID = { 0x4a56e38e, 0xe6d0, 0x4e25, { 0x95, 0x7c, 0x85, 0x47, 0xe9, 0xa3, 0x33, 0x80 } };

#define PX_RELEASE(x) if (x) { x->release(); x = nullptr; }


class PHYSXCHIPS_API PhysXSDK : public Chip
{
	CHIPDESC_DECL;
public:
	PhysXSDK();
	virtual ~PhysXSDK();

	virtual PxPhysics *GetPhysics() const { return _physics; }
	virtual PxCooking *GetCooking();
	virtual PxCudaContextManager *GetCudaContextManager();

	virtual PxMaterial *GetDefaultMaterial();

	virtual const PxTolerancesScale &GetToleranceScale() const { return _toleranceScale; }

protected:
	PxTolerancesScale _toleranceScale;

	PxFoundation *_foundation;
//	PxProfileZoneManager *_pzm;
	PxPhysics *_physics;
	bool _extensionsOK;
	PxMaterial *_defaultMaterial;
	PxCooking *_cooking;
	PxCudaContextManager *_cudaContextManager;

};

class PHYSXCHIPS_API PhysXUsage
{
public:
	PhysXUsage();
	virtual ~PhysXUsage();

	PhysXSDK *sdk() const;
	PxPhysics *physics() const;

private:
	mutable PhysXSDK *_sdk;
};



}