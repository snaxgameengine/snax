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
#include "GraphicsMatrix.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "M3DEngine/Engine.h"
#include "Graphics.h"
#include "RenderSettings.h"

using namespace m3d;


CHIPDESCV1_DEF(GraphicsMatrix, MTEXT("Graphics Matrix"), GRAPHICSMATRIX_GUID, MATRIXCHIP_GUID);


bool GraphicsMatrix::CopyChip(Chip *chip)
{
	GraphicsMatrix *c = dynamic_cast<GraphicsMatrix*>(chip);
	B_RETURN(MatrixChip::CopyChip(c));
	SetOperatorType(c->_ot);
	return true;
}

bool GraphicsMatrix::LoadChip(DocumentLoader &loader)
{
	B_RETURN(MatrixChip::LoadChip(loader));
	OperatorType ot;
	LOAD("operatorType|ot", ot);
	SetOperatorType(ot);
	return true;
}

bool GraphicsMatrix::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(MatrixChip::SaveChip(saver));
	SAVE("operatorType", _ot);
	return true;
}

const XMFLOAT4X4 &GraphicsMatrix::GetMatrix()
{
	RenderSettings *rs = engine->GetGraphics()->rs();

	switch (_ot) 
	{
	case OperatorType::WORLD_MATRIX:
		_matrix = rs->GetWorldMatrix();
		break;
	case OperatorType::VIEW_MATRIX:
		_matrix = rs->GetViewMatrix();
		break;
	case OperatorType::PROJECTION_MATRIX:
		_matrix = rs->GetProjectionMatrix();
		break;
	case OperatorType::CULLING_MATRIX:
		_matrix = rs->GetCullingMatrix();
		break;
	case OperatorType::SHADOW_MATRIX:
		_matrix = rs->GetShadowMatrix();
		break;
	default:
		_matrix = IDENTITY;
	}

	return _matrix;
}

void GraphicsMatrix::SetOperatorType(OperatorType ot)
{
	if (_ot == ot)
		return;
	_ot = ot;
	ClearConnections();
}