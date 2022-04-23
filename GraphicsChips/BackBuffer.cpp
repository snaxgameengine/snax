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
#include "BackBuffer.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "D3D12RenderWindow.h"
#include "M3DEngine/RenderWindowManager.h"


using namespace m3d;

CHIPDESCV1_DEF(BackBuffer, MTEXT("Back Buffer"), BACKBUFFER_GUID, GRAPHICSRESOURCECHIP_GUID);


BackBuffer::BackBuffer()
{
	ClearConnections();
}

bool BackBuffer::CopyChip(Chip* chip)
{
	BackBuffer* c = dynamic_cast<BackBuffer*>(chip); // skip resource!
	B_RETURN(Chip::CopyChip(c));
	_useSrgb = c->_useSrgb;
	return true;
}

bool BackBuffer::LoadChip(DocumentLoader& loader)
{
	B_RETURN(Chip::LoadChip(loader)); // skip resource!
	LOADDEF("useSRGB|srgb", _useSrgb, true);
	return true;
}

bool BackBuffer::SaveChip(DocumentSaver& saver) const
{
	B_RETURN(Chip::SaveChip(saver)); // skip resource!
	SAVEDEF("useSRGB", _useSrgb, true);
	return true;
}

ID3D12Resource* BackBuffer::GetResource()
{
	D3D12RenderWindow* rw = GetRenderWindow();
	if (!rw)
		throw GraphicsException(this, MTEXT("No render window!"));
	return rw->GetBackBuffer();
}

const Descriptor& BackBuffer::GetRenderTargetViewDescriptor()
{
	D3D12RenderWindow* rw = GetRenderWindow();
	if (!rw)
		throw GraphicsException(this, MTEXT("No render window!"));
	return rw->GetDescriptor(isSRGB());
}

D3D12RenderWindow* BackBuffer::GetRenderWindow()
{
	return dynamic_cast<D3D12RenderWindow*>(graphics()->GetRenderWindowManager()->GetRenderWindow());
}