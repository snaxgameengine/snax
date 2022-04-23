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
#include "MatrixChip.h"
#include "M3DEngine/DocumentSaver.h"
#include "M3DEngine/DocumentLoader.h"

using namespace m3d;


CHIPDESCV1_DEF(MatrixChip, MTEXT("Matrix"), MATRIXCHIP_GUID, NUMERIC_GUID);


const XMFLOAT4X4 MatrixChip::IDENTITY = XMFLOAT4X4(1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f);


MatrixChip::MatrixChip()
{
}

MatrixChip::~MatrixChip()
{
}

bool MatrixChip::InitChip()
{
	B_RETURN(Numeric::InitChip());
	_matrix = IDENTITY;
	return true;
}

bool MatrixChip::CopyChip(Chip *chip)
{
	MatrixChip *c = dynamic_cast<MatrixChip*>(chip);
	B_RETURN(Numeric::CopyChip(c));
	_matrix = c->_matrix;
	return true;
}

bool MatrixChip::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Numeric::LoadChip(loader));
	LOADARRAY(MTEXT("matrix"), (float32*)&_matrix, 16);
	return true;
}

bool MatrixChip::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Numeric::SaveChip(saver));
	SAVEARRAY(MTEXT("matrix"), (float32*)&_matrix, 16);
	return true;
}

void MatrixChip::CallChip()
{
	GetMatrix();
}

const XMFLOAT4X4 &MatrixChip::GetMatrix()
{
	return _matrix;
}

void MatrixChip::SetMatrix(const XMFLOAT4X4 &m)
{
	_matrix = m;
}

bool MatrixChip::GetInverseMatrix(XMFLOAT4X4 &invertedMatrix)
{
	const XMFLOAT4X4 m = GetMatrix();
	XMVECTOR v;
	XMStoreFloat4x4(&invertedMatrix, XMMatrixInverse(&v, XMLoadFloat4x4(&m)));
	return true;
}

bool MatrixChip::GetDecomposedMatrix(XMFLOAT3 &translation, XMFLOAT4 &rotation, XMFLOAT3 &scaling)
{
	XMVECTOR t, r, s;
	const XMFLOAT4X4 m = GetMatrix();
	if (XMMatrixDecompose(&s, &r, &t, XMLoadFloat4x4(&m)) == FALSE)
		return false;
	XMStoreFloat3(&translation, t);
	XMStoreFloat4(&rotation, r);
	XMStoreFloat3(&scaling, s);
	return true;
}

