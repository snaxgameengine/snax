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
#include "PhysXCommand.h"
#include "PhysXScene.h"
#include "GraphicsChips/DebugGeometry.h"
#include "PhysXRigidDynamic.h"
#include "StdChips/VectorChip.h"
#include "PhysXJoint.h"


using namespace m3d;


CHIPDESCV1_DEF(PhysXCommand, MTEXT("PhysX Command"), PHYSXCOMMAND_GUID, CHIP_GUID);


PhysXCommand::PhysXCommand()
{
	_cmd = CMD_NONE;
}

PhysXCommand::~PhysXCommand()
{
}

bool PhysXCommand::CopyChip(Chip *chip)
{
	PhysXCommand *c = dynamic_cast<PhysXCommand*>(chip);
	B_RETURN(Chip::CopyChip(c));
	SetCommand(c->_cmd);
	return true;
}

bool PhysXCommand::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	Command cmd;
	LOAD("cmd", (uint32&)cmd);
	SetCommand(cmd);
	return true;
}

bool PhysXCommand::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVE("cmd", (uint32)_cmd);
	return true;
}

void PhysXCommand::CallChip()
{
	switch (_cmd)
	{
	case CMD_SCENE_SIMULATE:
		{
			ChildPtr<PhysXScene> chScene = GetChild(0);
			if (chScene)
				chScene->Simulate();
		}
		break;
	case CMD_SCENE_FETCH_RESULTS:
		{
			ChildPtr<PhysXScene> chScene = GetChild(0);
			if (chScene)
				chScene->FetchResults();
		}
		break;
	case CMD_SCENE_DEBUG_RENDER:
		{
			ChildPtr<PhysXScene> chScene = GetChild(0);
			if (chScene) {
				PxScene *scene = chScene->GetScene();

				DebugGeometry* dg = dynamic_cast<DebugGeometry*>(engine->GetChipManager()->GetGlobalChip(DEBUGGEOMETRY_GUID));

				const PxRenderBuffer &buffer = scene->getRenderBuffer();

				for (uint32 i = 0; i < buffer.getNbPoints(); i++) {
					const PxDebugPoint &p = buffer.getPoints()[i];
					dg->AddPoint(DebugVertex(XMFLOAT3((float32*)&p.pos), XMUBYTE4(p.color)));
				}

				for (uint32 i = 0; i < buffer.getNbLines(); i++) {
					const PxDebugLine &line = buffer.getLines()[i];
					dg->AddLineSegment(DebugVertex(XMFLOAT3((float32*)&line.pos0), XMUBYTE4(line.color0)), DebugVertex(XMFLOAT3((float32*)&line.pos1), XMUBYTE4(line.color1)));
				}

				for (uint32 i = 0; i < buffer.getNbTriangles(); i++) {
					const PxDebugTriangle &triangle = buffer.getTriangles()[i];
					dg->AddTriangle(DebugVertex(XMFLOAT3((float32*)&triangle.pos0), XMUBYTE4(triangle.color0)), DebugVertex(XMFLOAT3((float32*)&triangle.pos1), XMUBYTE4(triangle.color1)), DebugVertex(XMFLOAT3((float32*)&triangle.pos2), XMUBYTE4(triangle.color2)));
				}
			}	
		}
		break;
	case CMD_SCENE_START:
		{
			ChildPtr<PhysXScene> chScene = GetChild(0);
			if (chScene)
				chScene->StartSimulation();
		}
		break;
	case CMD_SCENE_STOP:
		{
			ChildPtr<PhysXScene> chScene = GetChild(0);
			if (chScene)
				chScene->StopSimulation();
		}
		break;
	case CMD_SCENE_DESTROY:
		{
			ChildPtr<PhysXScene> chScene = GetChild(0);
			if (chScene)
				chScene->DestroyScene();
		}
		break;
	case CMD_ACTOR_DESTROY:
		{
			ChildPtr<PhysXActor> chActor = GetChild(0);
			if (chActor)
				chActor->DestroyActor();
		}
		break;
	case CMD_RIGIDDYNAMIC_ADD_GLOBAL_FORCE_AT_GLOBAL_POINT:
	case CMD_RIGIDDYNAMIC_ADD_GLOBAL_FORCE_AT_LOCAL_POINT:
	case CMD_RIGIDDYNAMIC_ADD_LOCAL_FORCE_AT_GLOBAL_POINT:
	case CMD_RIGIDDYNAMIC_ADD_LOCAL_FORCE_AT_LOCAL_POINT:
		{
			ChildPtr<PhysXRigidDynamic> chActor = GetChild(0);
			ChildPtr<VectorChip> chForce = GetChild(1);
			ChildPtr<VectorChip> chPos = GetChild(2);
			if (chActor && chForce) {
				XMFLOAT4 f = chForce->GetVector();
				XMFLOAT4 p = chPos ? chPos->GetVector() : XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
				chActor->AddForce(PhysXForce(
					_cmd == CMD_RIGIDDYNAMIC_ADD_GLOBAL_FORCE_AT_GLOBAL_POINT ? PhysXForce::GLOBAL_AT_GLOBAL_POS : (_cmd == CMD_RIGIDDYNAMIC_ADD_GLOBAL_FORCE_AT_LOCAL_POINT ? PhysXForce::GLOBAL_AT_LOCAL_POS : 
					(_cmd == CMD_RIGIDDYNAMIC_ADD_LOCAL_FORCE_AT_GLOBAL_POINT ? PhysXForce::LOCAL_AT_GLOBAL_POS : PhysXForce::LOCAL_AT_LOCAL_POS)), PxVec3(f.x,f.y,f.z), PxVec3(p.x, p.y, p.z)));
			}
		}
		break;
	case CMD_RIGIDDYNAMIC_ADD_GLOBAL_TORQUE:
	case CMD_RIGIDDYNAMIC_ADD_LOCAL_TORQUE:
		{
			ChildPtr<PhysXRigidDynamic> chActor = GetChild(0);
			ChildPtr<VectorChip> chForce = GetChild(1);
			if (chActor && chForce) {
				XMFLOAT4 f = chForce->GetVector();
				chActor->AddForce(PhysXForce(_cmd == CMD_RIGIDDYNAMIC_ADD_GLOBAL_TORQUE ? PhysXForce::GLOBAL_TORQUE : PhysXForce::LOCAL_TORQUE, PxVec3(f.x, f.y, f.z)));
			}
		}
		break;
	case CMD_RIGIDDYNAMIC_CLEAR_FORCES_AND_TORQUES:
		{
			ChildPtr<PhysXRigidDynamic> chActor = GetChild(0);
			if (chActor)
				chActor->ClearForces();
		}
		break;
	case CMD_JOINT_DESTROY:
		{
			ChildPtr<PhysXJoint> chJoint = GetChild(0);
			if (chJoint)
				chJoint->DestroyJoint();
		}
		break;
	}
}

void PhysXCommand::SetCommand(Command cmd)
{
	if (_cmd == cmd)
		return;
	
	_cmd = cmd;

	switch (_cmd)
	{
	case CMD_SCENE_SIMULATE:
	case CMD_SCENE_FETCH_RESULTS:
	case CMD_SCENE_DEBUG_RENDER:
	case CMD_SCENE_START:
	case CMD_SCENE_STOP:
	case CMD_SCENE_DESTROY:
		CREATE_CHILD_KEEP(0, PHYSXSCENE_GUID, false, UP, MTEXT("Scene"));
		ClearConnections(1);
		break;
	case CMD_ACTOR_DESTROY:
		CREATE_CHILD_KEEP(0, PHYSXACTOR_GUID, false, DOWN, MTEXT("Actor"));
		ClearConnections(1);
		break;
	case CMD_RIGIDDYNAMIC_ADD_GLOBAL_FORCE_AT_GLOBAL_POINT:
	case CMD_RIGIDDYNAMIC_ADD_GLOBAL_FORCE_AT_LOCAL_POINT:
	case CMD_RIGIDDYNAMIC_ADD_LOCAL_FORCE_AT_GLOBAL_POINT:
	case CMD_RIGIDDYNAMIC_ADD_LOCAL_FORCE_AT_LOCAL_POINT:
		CREATE_CHILD_KEEP(0, PHYSXRIGIDDYNAMIC_GUID, false, DOWN, MTEXT("Rigid Dynamic"));
		CREATE_CHILD_KEEP(1, VECTORCHIP_GUID, false, UP, MTEXT("Force"));
		CREATE_CHILD_KEEP(2, VECTORCHIP_GUID, false, UP, MTEXT("Position"));
		ClearConnections(3);
		break;
	case CMD_RIGIDDYNAMIC_ADD_GLOBAL_TORQUE:
	case CMD_RIGIDDYNAMIC_ADD_LOCAL_TORQUE:
		CREATE_CHILD_KEEP(0, PHYSXRIGIDDYNAMIC_GUID, false, DOWN, MTEXT("Rigid Dynamic"));
		CREATE_CHILD_KEEP(1, VECTORCHIP_GUID, false, UP, MTEXT("Torque"));
		ClearConnections(2);
		break;
	case CMD_RIGIDDYNAMIC_CLEAR_FORCES_AND_TORQUES:
		CREATE_CHILD_KEEP(0, PHYSXRIGIDDYNAMIC_GUID, false, DOWN, MTEXT("Rigid Dynamic"));
		ClearConnections(1);
		break;
	case CMD_JOINT_DESTROY:
		CREATE_CHILD_KEEP(0, PHYSXJOINT_GUID, false, DOWN, MTEXT("Joint"));
		ClearConnections(1);
		break;
	default:
		ClearConnections();
		break;
	}
}