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
#include "PhysXSDK.h"
#include <thread>

namespace m3d
{


static const Guid PHYSXSCENE_GUID = { 0x7bad6997, 0x690d, 0x48d1, { 0xa4, 0x53, 0x1f, 0x60, 0x30, 0xc3, 0x65, 0x15 } };


class PhysXSceneObject;

class PHYSXCHIPS_API PhysXScene : public Chip
{
	CHIPDESC_DECL;
public:
	PhysXScene();
	virtual ~PhysXScene();

	bool CopyChip(Chip *chip) override;
	bool LoadChip(DocumentLoader &loader) override;
	bool SaveChip(DocumentSaver &saver) const override;

	void CallChip() override;

	virtual bool Exist() const { return _scene != nullptr; }
	virtual bool IsSimulating() const { return _isSimulating; }
	virtual bool IsRunning() const { return _isRunning; }
	virtual PxScene *GetScene();
	virtual void DestroyScene();
	virtual void StartSimulation();
	virtual void StopSimulation();

	virtual float64 GetSimulationRate() const { return _stepSize; }
	virtual float64 GetMaxSimulationTime() const { return _maxSimulationTime; }
	virtual PxSolverType::Enum GetSolverType() const { return _solverType; }
	virtual PxBroadPhaseType::Enum GetBroadPhaseAlgorithm() const { return _broadPhaseType; }
	virtual PxFrictionType::Enum GetFrictionType() const { return _frictionType; }
	virtual PxSceneFlags GetSceneFlags() const { return _flags; }
	virtual float64 GetRealTimeIndex() const { return _realTimeIndex; }
	virtual float64 GetSimulationIndex() const { return _simIndex; }
	virtual float64 GetSimulationWaitingTime() const { return _simulationWait; }

	virtual void SetSimulationRate(float64 stepSize) { _stepSize = stepSize; }
	virtual void SetMaxSimulationTime(float64 timeInMS) { _maxSimulationTime = timeInMS; }
	virtual void SetSolverType(PxSolverType::Enum solverType) { _solverType = solverType; }
	virtual void SetBroadPhaseAlgorithm(PxBroadPhaseType::Enum broadPhase) { _broadPhaseType = broadPhase; }
	virtual void SetFrictionType(PxFrictionType::Enum frictionType) { _frictionType = frictionType; }
	virtual void SetSceneFlags(PxSceneFlags flags) { _flags = flags; }


	virtual void Simulate(bool sync = false);
	virtual bool FetchResults(bool block = false);

	virtual void RegisterSceneObject(PhysXSceneObject *obj) { _sceneObjects.insert(obj); }
	virtual void UnregisterSceneObject(PhysXSceneObject *obj) { _sceneObjects.erase(obj); }

	uint32 __threadEnter();

protected:
	PxScene *_scene;
	PxDefaultCpuDispatcher *_cpuDispatcher;

	bool _isSimulating;
	bool _isRunning;

	std::thread _simulationThread;
	HANDLE _threadHandle;
	HANDLE _simulateEvent;
	HANDLE _simulateDoneEvent;
	bool _terminate;

	bool __isSimulating() const { return _isSimulating; }

	bool _started;
	float64 _accum;

	float64 _stepSize;
	float64 _maxSimulationTime;
	float64 _realTimeIndex;
	float64 _simIndex;
	float64 _simulationWait;
	PxSolverType::Enum _solverType;
	PxBroadPhaseType::Enum _broadPhaseType;
	PxFrictionType::Enum _frictionType;
	PxSceneFlags _flags;

	Set<PxActor*> _updatedActors;
	Set<PhysXSceneObject*> _sceneObjects;

	void _simulate(float64 &realTimeIndex, float64 &simIndex);
	virtual void OnRelease() { DestroyScene(); }
};



}