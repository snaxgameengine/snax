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
#include "Renderable.h"
#include "Geometry.h"
#include "DebugGeometry.h"
#include "RenderSEttings.h"
#include "Material.h"
#include "GraphicsException.h"
#include "Graphics.h"
#include "StdChips/MatrixChip.h"
#include "StdChips/Value.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "M3DEngine/Engine.h"

using namespace m3d;


CHIPDESCV1_DEF(Renderable, MTEXT("Renderable"), RENDERABLE_GUID, _3DOBJECT_GUID);



Renderable::Renderable()
{
	ClearConnections();

	CREATE_CHILD(0, GEOMETRY_GUID, false, UP, MTEXT("Geometry"));
	CREATE_CHILD(1, VALUE_GUID, false, UP, MTEXT("Instance Count"));
	CREATE_CHILD(FIRST_MATERIAL_CONNECTION, MATERIAL_GUID, true, UP, MTEXT("Materials"));
}

Renderable::~Renderable()
{
}

bool Renderable::CopyChip(Chip *chip)
{
	Renderable *c = dynamic_cast<Renderable*>(chip);
	B_RETURN(_3DObject::CopyChip(c));
	_wholeObjectCulling = c->_wholeObjectCulling;
	_subsetCulling = c->_subsetCulling;
	_boundingBox = c->_boundingBox;
	if (c->_subsets.empty())
		MakeUniversal();
	else {
		List<std::pair<uint32, String>> s;
		c->GetSpecificSubsets(s);
		MakeSpecific(s);
	}
	return true;
}

bool Renderable::LoadChip(DocumentLoader &loader)
{
	B_RETURN(_3DObject::LoadChip(loader));
	LOAD("useWholeObjectCulling|UseWholeObjectCulling", _wholeObjectCulling);
	LOAD("useSubsetCulling|UseSubsetCulling", _subsetCulling);
	LOAD("boundingBox|BoundingBox", _boundingBox);
	List<std::pair<uint32, String>> subsets;
	LOAD("subsets|Subsets", subsets);
	_subsets.clear();
	if (subsets.empty()) {
		CREATE_CHILD(FIRST_MATERIAL_CONNECTION, MATERIAL_GUID, true, UP, MTEXT("Material"));
		ClearConnections(FIRST_MATERIAL_CONNECTION + 1);
	}
	else {
		for (size_t i = 0; i < subsets.size(); i++) {
			_subsets.push_back(subsets[i].first);
			CREATE_CHILD_KEEP(FIRST_MATERIAL_CONNECTION + (uint32)i, MATERIAL_GUID, false, UP, subsets[i].second);
		}
		ClearConnections(FIRST_MATERIAL_CONNECTION + (uint32)subsets.size());
	}
	
	return true;
}

bool Renderable::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(_3DObject::SaveChip(saver));
	SAVE("useWholeObjectCulling", _wholeObjectCulling);
	SAVE("useSubsetCulling", _subsetCulling);
	SAVE("boundingBox", _boundingBox);
	List<std::pair<uint32, String>> subsets;
	if (!_subsets.empty()) {
		const ChildConnectionList &ccl = GetChildren();
		for (size_t i = 0; i < _subsets.size(); i++) {
			String n;
			if (i + FIRST_MATERIAL_CONNECTION < ccl.size())
				n = ccl[FIRST_MATERIAL_CONNECTION + i]->desc.name;
			subsets.push_back(std::make_pair(_subsets[i], n));
		}
	}
	SAVE("subsets", subsets);
	return true;
}

void Renderable::CallChip()
{
	try 
	{
		// Render without 3DObject using identity world matrix, imm. context and 1 instance.
		Render(MatrixChip::IDENTITY, 1, 0, GetEnableFrustumCulling()); // Throws
	}
	catch (const ChipException &exp) 
	{
		AddException(exp);
	}

	// Clear the lock for any bound resources!
	engine->GetGraphics()->rs()->ClearGraphicsRootDescriptorTables();
}

void Renderable::Render(const XMFLOAT4X4 &parentMatrix, uint32 instanceCount, uint32 startInstanceLocation, bool enableFrustumCulling)
{
	ChildPtr<Geometry> chGeometry = GetChild(0);
	if (!chGeometry)
		return;

	ChildPtr<Value> chInstanceCount = GetChild(1);
	if (chInstanceCount) {
		uint32 ic = (uint32)chInstanceCount->GetValue();
		ic = std::min(ic, 0x00FFFFFFu); // limit to some reasonable number (16M)....
		instanceCount *= ic;
	}
	if (instanceCount == 0)
		return; // No instances to render

	Graphics* g = engine->GetGraphics();
	RenderState *rs = g->rs();

	rs->SetWorldMatrix(parentMatrix);

	XMMATRIX world = XMLoadFloat4x4(&rs->GetWorldMatrix());

	Frustum::Intersection objIntersection = Frustum::INTERSECT;

	enableFrustumCulling = enableFrustumCulling && GetEnableFrustumCulling();

	if (enableFrustumCulling && _wholeObjectCulling) {
		AxisAlignedBox aabb = _boundingBox * world;
		objIntersection = rs->GetFrustum(0).Test(aabb, &_cullingPlane);

		if (objIntersection == Frustum::OUTSIDE)
			return; // Outside view frustum

		if (g->IsRenderWorldSpaceAABB())
			g->dg()->AddBox(XMMatrixIdentity(), aabb.GetMin(), aabb.GetMax(), WHITE);
		if (g->IsRenderLocalAABB())
			g->dg()->AddBox(world, _boundingBox.GetMin(), _boundingBox.GetMax(), YELLOW);
	}

	chGeometry->Update();

	chGeometry->Prepare(); // Throws!

	bool subsetCulling = enableFrustumCulling && _subsetCulling && objIntersection == Frustum::INTERSECT;
	bool specific = _subsets.size() > 0;

	for (uint32 i = 0, j = specific ? (uint32)_subsets.size() : (uint32)chGeometry->GetSubsets().size(); i < j; i++) {
		uint32 index = specific ? _subsets[i] : i;
		if (index >= chGeometry->GetSubsets().size())
			continue; // Invalid subset index

		ChildPtr<Material> chMaterial = specific ? GetChild(FIRST_MATERIAL_CONNECTION + index) : GetChild(FIRST_MATERIAL_CONNECTION, index);
		if (!chMaterial) {
			AddMessage(MissingChildException(specific ? FIRST_MATERIAL_CONNECTION + index : FIRST_MATERIAL_CONNECTION));
			continue; // No material for this subset..
		}

		const GeometrySubset &ss = chGeometry->GetSubsets()[index];

		if (subsetCulling) { // We also do culling by subset...
			const AxisAlignedBox &localAABB = ss.boundingBox;
			AxisAlignedBox aabb = localAABB * world;
			if (rs->GetFrustum(0).Test(aabb, &(m3d::Frustum::PlaneId&)ss._cullingPlane) == Frustum::OUTSIDE)
				continue; // This subset is outside the view frustum
			if (g->IsRenderWorldSpaceAABB())
				g->dg()->AddBox(XMMatrixIdentity(), aabb.GetMin(), aabb.GetMax(), WHITE);
			if (g->IsRenderLocalAABB())
				g->dg()->AddBox(world, localAABB.GetMin(), localAABB.GetMax(), YELLOW);
		}

		// Store current pipeline state (could have been set using GraphicsState::CallChip())
		struct KeepPSO
		{
			RenderState* rs;
			KeepPSO(RenderState* rs) : rs(rs) { rs->PushState(); }
			~KeepPSO() { rs->PopState(); }
		} keepPSO(rs);

		try 
		{
			chMaterial->UpdateChip();
			chMaterial->SetGraphicsStates();
		}
		catch (ChipException &exp) 
		{
			AddException(exp); // Report and continue!
			continue;
		}

		rs->IASetPrimitiveTopology(ss.pt);
		rs->PrepareDraw();
		rs->CommitResourceBarriers();

		if (chGeometry->GetAPI() == DRAW)
			rs->DrawInstanced(ss.count, instanceCount, ss.startLocation + ss.baseVertexLocation, startInstanceLocation);
		else
			rs->DrawIndexedInstanced(ss.count, instanceCount, ss.startLocation, ss.baseVertexLocation, startInstanceLocation);
	}
}

bool Renderable::CheckFrustumCulling(const XMFLOAT4X4 &worldMatrix)
{
	if (_wholeObjectCulling) {
		XMMATRIX world = XMLoadFloat4x4(&worldMatrix);
		AxisAlignedBox aabb = _boundingBox * world;
		Frustum::Intersection objIntersection = engine->GetGraphics()->rs()->GetFrustum(0).Test(aabb, &_cullingPlane);
		if (objIntersection == Frustum::OUTSIDE)
			return false;
	}
	return true; // TODO: may want to do subset culling!
/*
	ChildPtr<Geometry> chGeometry = GetChild(0);
	if (!chGeometry)
	return false;
	return objIntersection != Frustum::OUTSIDE;
*/
}

void Renderable::MakeUniversal()
{
	if (_subsets.empty())
		return;

	Map<uint32, Chip*> m; // Keep connected materials!
	for (size_t i = 0; i < _subsets.size(); i++)
		m.insert(std::make_pair(_subsets[i], GetRawChild(FIRST_MATERIAL_CONNECTION + (uint32)i)));

	_subsets.clear();
	ClearConnections(FIRST_MATERIAL_CONNECTION);
	CREATE_CHILD(FIRST_MATERIAL_CONNECTION, MATERIAL_GUID, true, UP, MTEXT("Materials"));
	for (const auto &n : m)
		SetChild(n.second, FIRST_MATERIAL_CONNECTION, n.first);
}

void Renderable::MakeSpecific(const List<std::pair<uint32, String>> &subsets)
{
	if (subsets.empty())
		return MakeUniversal();
	List<std::pair<uint32, Chip*>> m; // Keep connected materials!
	if (_subsets.size()) {
		for (size_t i = 0; i < _subsets.size(); i++)
			m.push_back(std::make_pair(_subsets[i], GetRawChild(FIRST_MATERIAL_CONNECTION + (uint32)i)));
	}
	else {
		for (size_t i = 0; i < GetSubConnectionCount(FIRST_MATERIAL_CONNECTION); i++)
			m.push_back(std::make_pair((uint32)i, GetRawChild(FIRST_MATERIAL_CONNECTION, (uint32)i)));
	}
	_subsets.clear();
	for (size_t i = 0; i < subsets.size(); i++) {
		_subsets.push_back(subsets[i].first);
		CREATE_CHILD_KEEP(FIRST_MATERIAL_CONNECTION + (uint32)i, MATERIAL_GUID, false, UP, subsets[i].second);
		for (size_t j = 0; j < m.size(); j++) {
			if (m[j].first == subsets[i].first) {
				SetChild(m[j].second, FIRST_MATERIAL_CONNECTION + (uint32)i, 0);
				m[j].first = -1;
			}
		}
	}
	ClearConnections(FIRST_MATERIAL_CONNECTION + (uint32)subsets.size());
}

void Renderable::UpdateToGeometry()
{
	Geometry *d = dynamic_cast<Geometry*>(GetRawChild(0)); // Drawable must be linked *directly* to us!

	if (!d || d->GetSubsets().empty())
		return MakeUniversal();

	List<std::pair<uint32, String>> subsets;

	const GeometrySubsetList &ssl = d->GetSubsets();

	for (size_t i = 0; i < ssl.size(); i++)
		subsets.push_back(std::make_pair((uint32)i, MTEXT("Subset ") + strUtils::fromNum(i) + MTEXT(": ") + (ssl[i].name.empty() ? MTEXT("Unnamed") : ssl[i].name)));

	MakeSpecific(subsets);
}

void Renderable::GetSpecificSubsets(List<std::pair<uint32, String>> &subsets)
{
	subsets.clear();
	const ChildConnectionList &ccl = GetChildren();
	for (size_t i = 0; i < _subsets.size(); i++) {
		String n;
		if (i + FIRST_MATERIAL_CONNECTION < ccl.size())
			n = ccl[FIRST_MATERIAL_CONNECTION + i]->desc.name;
		subsets.push_back(std::make_pair(_subsets[i], n));
	}
}

void Renderable::CalculateBoundingBox()
{
	Geometry*d = dynamic_cast<Geometry*>(GetRawChild(0)); // Drawable must be linked *directly* to us!

	if (!d) {
		_boundingBox.SetInfinite();
		return;
	}

	_boundingBox.SetNull();

	const GeometrySubsetList &ssl = d->GetSubsets();

	if (_subsets.empty()) {
		for (size_t i = 0; i < ssl.size(); i++)
			_boundingBox += ssl[i].boundingBox;
	}
	else {
		for (size_t i = 0; i < _subsets.size(); i++) {
			if (_subsets[i] < ssl.size())
				_boundingBox += ssl[_subsets[i]].boundingBox;
		}
	}
}