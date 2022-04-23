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
#include "PhysX.h"


namespace m3d
{


static const Guid PHYSXARTICULATIONBASE_GUID = { 0x6ae4523d, 0xc066, 0x46b9,{ 0x95, 0xbd, 0x3c, 0x83, 0xe2, 0x60, 0x1f, 0xfc } };



class PHYSXCHIPS_API PhysXArticulationBase : public Chip, public PhysXSceneObject
{
	CHIPDESC_DECL;
public:
	PhysXArticulationBase();
	~PhysXArticulationBase();

	bool CopyChip(Chip *chip) override;
	bool LoadChip(DocumentLoader &loader) override;
	bool SaveChip(DocumentSaver &saver) const override;

	void OnSceneDestroyed() override { DestroyArticulation(); }

	virtual bool Exist() const { return false; }
	virtual PxArticulationBase *GetArticulation() const { return nullptr; }
	virtual void DestroyArticulation() {}

	virtual float32 GetSleepThreshold();
	virtual void GetMinSolverIterations(uint32 &pos, uint32 &vel);

	virtual void SetSleepThreshold(float32 f);
	virtual void SetMinSolverIterations(uint32 pos, uint32 vel);

protected:
	float32 _sleepThreshold;
	uint32 _minPositionIters;
	uint32 _minVelocityIters;

	void OnRelease() override { DestroyArticulation(); }
	
};

}