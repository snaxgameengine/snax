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
#include "GraphicsValue.h"
#include "GraphicsResourceChip.h"
#include "ReadbackBuffer.h"
#include "3DObject.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/Application.h"
#include "GraphicsResourceChip.h"
#include "M3DEngine/Engine.h"
#include "GraphicsChips/ReadbackBuffer.h"
#include "GraphicsChips/3DObject.h"
#include "M3DEngine/Application.h"
#include "Utils.h"
#include "RenderSettings.h"

using namespace m3d;


CHIPDESCV1_DEF(GraphicsValue, MTEXT("Graphics Value"), GRAPHICSVALUE_GUID, VALUE_GUID);




GraphicsValue::GraphicsValue()
{
	ClearConnections();
}

GraphicsValue::~GraphicsValue()
{
}

bool GraphicsValue::CopyChip(Chip* chip)
{
	GraphicsValue* c = dynamic_cast<GraphicsValue*>(chip);
	B_RETURN(Value::CopyChip(c));
	SetType(c->_type);
	return true;
}

bool GraphicsValue::LoadChip(DocumentLoader& loader)
{
	B_RETURN(Value::LoadChip(loader));
	OperatorType t;
	LOAD("operatorType|type", t);
	SetType(t);
	return true;
}

bool GraphicsValue::SaveChip(DocumentSaver& saver) const
{
	B_RETURN(Value::SaveChip(saver));
	SAVE("operatorType", _type);
	return true;
}

void GraphicsValue::SetType(OperatorType type)
{
	if (_type == type)
		return;
	_type = type;
	switch (_type) {
	case OperatorType::RESOURCE_WIDTH:
	case OperatorType::RESOURCE_HEIGHT:
	case OperatorType::RESOURCE_DEPTH:
	case OperatorType::RESOURCE_MIP_LEVELS:
	case OperatorType::RESOURCE_ARRAY_SIZE:
	case OperatorType::RESOURCE_MS_COUNT:
	case OperatorType::RESOURCE_MS_QUALITY:
	case OperatorType::RTV_WIDTH:
	case OperatorType::RTV_HEIGHT:
	case OperatorType::DSV_WIDTH:
	case OperatorType::DSV_HEIGHT:
	case OperatorType::UAV_WIDTH:
	case OperatorType::UAV_HEIGHT:
	case OperatorType::RESOURCE_EXIST:
		CREATE_CHILD_KEEP(0, GRAPHICSRESOURCECHIP_GUID, false, UP, MTEXT("Resource"));
		ClearConnections(1);
		break;
	case OperatorType::IS_DOWNLOAD_COMPLETE:
		CREATE_CHILD_KEEP(0, READBACKBUFFER_GUID, false, UP, MTEXT("Readback Buffer"));
		ClearConnections(1);
		break;
	case OperatorType::SRV_WIDTH:
	case OperatorType::SRV_HEIGHT:
		CREATE_CHILD_KEEP(0, GRAPHICSRESOURCECHIP_GUID, false, UP, MTEXT("Resource"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Mip Level"));
		ClearConnections(2);
		break;
	case OperatorType::RESOURCE_CALC_SUBRESOURCE:
		CREATE_CHILD_KEEP(0, VALUE_GUID, false, UP, MTEXT("Mip Slice"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Array Slice"));
		CREATE_CHILD_KEEP(2, VALUE_GUID, false, UP, MTEXT("Plane Slice"));
		CREATE_CHILD_KEEP(3, VALUE_GUID, false, UP, MTEXT("Mip Levels"));
		CREATE_CHILD_KEEP(4, VALUE_GUID, false, UP, MTEXT("Array Size"));
		ClearConnections(5);
		break;
	case OperatorType::DEVICE_EXIST:
	case OperatorType::NUM_ADAPTERS:
		break;
	case OperatorType::CHECK_FRUSTUM_CULLING:
		CREATE_CHILD_KEEP(0, _3DOBJECT_GUID, false, UP, MTEXT("3D Object"));
		ClearConnections(1);
		break;
	default:
		ClearConnections(0);
		break;
	};
}

value GraphicsValue::GetValue()
{
	try
	{
		switch (_type)
		{
		case OperatorType::RESOURCE_WIDTH:
		case OperatorType::RESOURCE_HEIGHT:
		case OperatorType::RESOURCE_DEPTH:
		case OperatorType::RESOURCE_MIP_LEVELS:
		case OperatorType::RESOURCE_ARRAY_SIZE:
		case OperatorType::RESOURCE_MS_COUNT:
		case OperatorType::RESOURCE_MS_QUALITY:
		{
			ChildPtr<GraphicsResourceChip> ch0 = GetChild(0);
			if (!ch0)
				throw MissingChildException(this, 0);
			ch0->UpdateChip();
			ID3D12Resource* res = ch0->GetResource();
			D3D12_RESOURCE_DESC desc = res->GetDesc();
			switch (_type)
			{
			case OperatorType::RESOURCE_WIDTH: _value = (value)desc.Width; break;
			case OperatorType::RESOURCE_HEIGHT: _value = (value)desc.Height; break;
			case OperatorType::RESOURCE_DEPTH: _value = desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? (value)desc.DepthOrArraySize : 0.0f; break;
			case OperatorType::RESOURCE_MIP_LEVELS: _value = (value)desc.MipLevels; break;
			case OperatorType::RESOURCE_ARRAY_SIZE: _value = desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D ? (value)desc.DepthOrArraySize : 0.0f; break;
			case OperatorType::RESOURCE_MS_COUNT: _value = (value)desc.SampleDesc.Count; break;
			case OperatorType::RESOURCE_MS_QUALITY: _value = (value)desc.SampleDesc.Quality; break;
			}
		}
		break;
		case OperatorType::RESOURCE_CALC_SUBRESOURCE:
		{
			ChildPtr<Value> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			ChildPtr<Value> ch2 = GetChild(2);
			ChildPtr<Value> ch3 = GetChild(3);
			ChildPtr<Value> ch4 = GetChild(4);
			if (!ch0)
				throw MissingChildException(0);
			if (!ch1)
				throw MissingChildException(1);
			if (!ch2)
				throw MissingChildException(2);
			if (!ch3)
				throw MissingChildException(3);
			if (!ch4)
				throw MissingChildException(4);
			UINT mipslice = (UINT)ch0->GetValue();
			UINT arrslice = (UINT)ch1->GetValue();
			UINT pslice = (UINT)ch2->GetValue();
			UINT miplevels = (UINT)ch3->GetValue();
			UINT arrSize = (UINT)ch4->GetValue();
			_value = D3D12CalcSubresource(mipslice, arrslice, pslice, miplevels, arrSize);
		}
		break;
		case OperatorType::RESOURCE_EXIST:
		{
			ChildPtr<GraphicsResourceChip> ch0 = GetChild(0);
			if (!ch0)
				throw MissingChildException(this, 0);
			_value = ch0->HasResource() ? 1.0f : 0.0f;
		}
		break;
		case OperatorType::IS_DOWNLOAD_COMPLETE:
		{
			ChildPtr<ReadbackBuffer> ch0 = GetChild(0);
			_value = ch0 && ch0->IsDownloadComplete() ? 1.0f : 0.0f;
		}
		break;
		case OperatorType::RTV_WIDTH:
		case OperatorType::RTV_HEIGHT:
		{
			ChildPtr<GraphicsResourceChip> ch0 = GetChild(0);
			UINT w = 0, h = 0;
			if (!ch0)
				throw MissingChildException(this, 0);
			ch0->UpdateChip();
			const Descriptor& d = ch0->GetRenderTargetViewDescriptor(); // Throws!
			CalculateRenderTargetSize(w, h, d.resource, d.rtv);
			_value = _type == OperatorType::RTV_WIDTH ? (value)w : (value)h;
		}
		break;
		case OperatorType::DSV_WIDTH:
		case OperatorType::DSV_HEIGHT:
		{
			ChildPtr<GraphicsResourceChip> ch0 = GetChild(0);
			UINT w = 0, h = 0;
			if (!ch0)
				throw MissingChildException(this, 0);
			ch0->UpdateChip();
			const Descriptor& d = ch0->GetDepthStencilViewDescriptor(); // Throws!
			CalculateDepthStencilSize(w, h, d.resource, d.dsv);
			_value = _type == OperatorType::DSV_WIDTH ? (value)w : (value)h;
		}
		break;
		case OperatorType::UAV_WIDTH:
		case OperatorType::UAV_HEIGHT:
		{
			ChildPtr<GraphicsResourceChip> ch0 = GetChild(0);
			UINT w = 0, h = 0;
			if (!ch0)
				throw MissingChildException(this, 0);
			ch0->UpdateChip();
			ID3D12Resource* res = ch0->GetResource();
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			ch0->GetUnorderedAccessViewDesc(uavDesc);
			CalculateUnorderedAccessSize(w, h, res, &uavDesc);
			_value = _type == OperatorType::UAV_WIDTH ? (value)w : (value)h;
		}
		break;
		case OperatorType::SRV_WIDTH:
		case OperatorType::SRV_HEIGHT:
		{
			ChildPtr<GraphicsResourceChip> ch0 = GetChild(0);
			UINT w = 0, h = 0, d = 0;
			if (!ch0)
				throw MissingChildException(this, 0);
			ChildPtr<Value> ch1 = GetChild(1);
			UINT mipLevel = ch1 ? (UINT)ch1->GetValue() : 0;
			ch0->UpdateChip();
			ID3D12Resource* res = ch0->GetResource();
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ch0->GetShaderResourceViewDesc(srvDesc);
			CalculateShaderResourceSize(w, h, d, mipLevel, res, &srvDesc);
			_value = _type == OperatorType::SRV_WIDTH ? (value)w : (value)h;
		}
		break;
		case OperatorType::CURRENT_RTV_WIDTH:
		case OperatorType::CURRENT_RTV_HEIGHT:
		{
			auto w = rs()->GetRenderTargetWidth();
			auto h = rs()->GetRenderTargetHeight();
			if (w == 0 || h == 0)
				throw GraphicsException(this, MTEXT("No render target has been set."), FATAL);
			_value = (value)(_type == OperatorType::CURRENT_RTV_WIDTH ? w : h);
			break;
		}
		case OperatorType::DEVICE_EXIST:
		{
			_value = graphics()->HasDevice() ? 1.0f : 0.0f;
		}
		break;
		case OperatorType::NUM_ADAPTERS:
		{
			_value = (value)graphics()->GetAdaptersAndOutputs().size();
		}
		break;
		case OperatorType::DISPLAY_ORIENTATION:
		{
			return engine->GetApplication()->GetDisplayOrientation();
		}
		break;
		case OperatorType::CHECK_FRUSTUM_CULLING:
		{
			ChildPtr<_3DObject> ch0 = GetChild(0);
			_value = ch0 && ch0->CheckFrustumCulling() ? 1.0 : 0.0;
		}
		default:
			break;
		};
	}
	catch (const ChipException& exc)
	{
		AddException(exc);
		_value = 0.0f;
	}
	return _value;
}

