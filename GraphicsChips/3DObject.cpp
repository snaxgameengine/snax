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
#include "3DObject.h"
#include "Renderable.h"
#include "RenderState.h"
#include "Graphics.h"
#include "StdChips/MatrixChip.h"
#include "StdChips/Value.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "M3DEngine/Engine.h"
#include "RenderSettings.h"


using namespace m3d;


CHIPDESCV1_DEF(_3DObject, MTEXT("3D Object"), _3DOBJECT_GUID, CHIP_GUID);


_3DObject::_3DObject()
{
	CREATE_CHILD(0, MATRIXCHIP_GUID, false, UP, MTEXT("Object Matrix"));
	CREATE_CHILD(1, VALUE_GUID, false, UP, MTEXT("Instance Count"));
	CREATE_CHILD(2, _3DOBJECT_GUID, true, DOWN, MTEXT("Renderables or child objects"));
}

_3DObject::~_3DObject()
{
}

bool _3DObject::CopyChip(Chip *chip)
{
	_3DObject *c = dynamic_cast<_3DObject*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_enableFrustumCulling = c->_enableFrustumCulling;
	return true;
}

bool _3DObject::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	LOADDEF("enableFrustumCulling|EnableFrustumCulling", _enableFrustumCulling, true);
	return true;
}

bool _3DObject::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVEDEF("enableFrustumCulling", _enableFrustumCulling, true);
	return true;
}

void _3DObject::CallChip()
{
	Render(MatrixChip::IDENTITY, 1, 0, _enableFrustumCulling);
}

void _3DObject::Render(const XMFLOAT4X4 &parentMatrix, uint32 instanceCount, uint32 startInstanceLocation, bool enableFrustumCulling)
{
	ChildPtr<Value> chInstanceCount = GetChild(1);
	uint32 instanceCountLocal = 1;
	if (chInstanceCount) {
		instanceCountLocal = (uint32)chInstanceCount->GetValue();
		instanceCountLocal = std::min(instanceCount, 0x00FFFFFFu); // Limit it to some 'reasonable' number (16M)...
	}

	instanceCount *= instanceCountLocal;

	if (instanceCount == 0)
		return; // No instances to render

	ChildPtr<MatrixChip> chObject = GetChild(0);

	XMFLOAT4X4 objMatrix = chObject ? chObject->GetMatrix() : MatrixChip::IDENTITY;
	
	XMFLOAT4X4 combinedMatrix;
	XMStoreFloat4x4(&combinedMatrix, XMMatrixMultiply(XMLoadFloat4x4(&objMatrix), XMLoadFloat4x4(&parentMatrix)));

	for (uint32 i = 0, j = GetSubConnectionCount(2); i < j; i++) {
		ChildPtr<_3DObject> ch = GetChild(2, i);
		if (ch) {
			try
			{
				ch->Render(combinedMatrix, instanceCount, startInstanceLocation, _enableFrustumCulling && enableFrustumCulling); // Throws!
			}
			catch (const ChipException &exp)
			{
				AddException(exp);
			}
		}
	}

	// Clear the lock for any bound resources!
	engine->GetGraphics()->rs()->ClearGraphicsRootDescriptorTables();
}


bool _3DObject::CheckFrustumCulling()
{
	// Note: Ignore _enableFrustumCulling here. If using this api, _enableFrustumCulling could be unset to prevent frustum culling being performed twice (during rendering).
	//	if (!_enableFrustumCulling)
	//		return true;

	ChildPtr<MatrixChip> chWorld = GetChild(0);

	XMFLOAT4X4 world = chWorld ? chWorld->GetMatrix() : MatrixChip::IDENTITY;

	for (uint32 i = 0, j = GetSubConnectionCount(2); i < j; i++) {
		ChildPtr<Renderable> ch = GetChild(2, i);
		if (ch && ch->CheckFrustumCulling(world))
			return true;
	}
	return false;
}