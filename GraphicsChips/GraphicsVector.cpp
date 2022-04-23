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
#include "GraphicsVector.h"
#include "ReadbackBuffer.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "M3DEngine/Engine.h"
#include "3DObject.h"
#include "M3DEngine/Application.h"
#include "GraphicsResourceChip.h"
#include "GraphicsChips/ReadbackBuffer.h"
#include "GraphicsChips/3DObject.h"
#include "M3DEngine/Application.h"
#include "Utils.h"
#include "RenderSettings.h"


using namespace m3d;


CHIPDESCV1_DEF(GraphicsVector, MTEXT("Graphics Vector"), GRAPHICSVECTOR_GUID, VECTORCHIP_GUID);

GraphicsVector::GraphicsVector()
{
	ClearConnections();
}

GraphicsVector::~GraphicsVector()
{
}

bool GraphicsVector::CopyChip(Chip* chip)
{
	GraphicsVector* c = dynamic_cast<GraphicsVector*>(chip);
	B_RETURN(VectorChip::CopyChip(c));
	SetType(c->_type);
	return true;
}

bool GraphicsVector::LoadChip(DocumentLoader& loader)
{
	B_RETURN(VectorChip::LoadChip(loader));
	OperatorType t;
	LOAD("operatorType|type", t);
	SetType(t);
	return true;
}

bool GraphicsVector::SaveChip(DocumentSaver& saver) const
{
	B_RETURN(VectorChip::SaveChip(saver));
	SAVE("operatortype", _type);
	return true;
}

void GraphicsVector::SetType(OperatorType type)
{
	if (_type == type)
		return;
	_type = type;
	switch (_type) {
	case OperatorType::DOWNLOADED_PIXEL:
		CREATE_CHILD_KEEP(0, READBACKBUFFER_GUID, false, UP, MTEXT("Readback Buffer"));
		CREATE_CHILD_KEEP(1, VECTORCHIP_GUID, false, UP, MTEXT("Coordinate (XYZ)"));
		ClearConnections(2);
		break;
	default:
		ClearConnections(0);
		break;
	};
}



const XMFLOAT4& GraphicsVector::GetVector()
{
	try
	{
		switch (_type)
		{
		case OperatorType::DOWNLOADED_PIXEL:
		{
			ChildPtr<ReadbackBuffer> ch0 = GetChild(0);
			if (!ch0)
				throw MissingChildException(this, 0);
			ChildPtr<VectorChip> ch1 = GetChild(1);
			if (!ch1)
				throw MissingChildException(this, 1);
			XMFLOAT4 coord = ch1->GetVector();
			if (!ch0->GetPixel(_vector, (UINT)coord.x, (UINT)coord.y, (UINT)coord.z))
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		break;
		default:
			break;
		};
	}
	catch (const ChipException& exc)
	{
		AddException(exc);
		_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	return _vector;
}

