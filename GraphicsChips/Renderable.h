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
#include "M3DCore/Frustum.h"
#include "M3DCore/AxisAlignedBox.h"
#include "3DObject.h"

namespace m3d
{


static const Guid RENDERABLE_GUID = { 0x5446a662, 0x18a8, 0x48e4, { 0xa2, 0x10, 0x61, 0x9a, 0xf4, 0x5c, 0xea, 0x8b } };

#define FIRST_MATERIAL_CONNECTION 2


class GRAPHICSCHIPS_API Renderable : public _3DObject
{
	CHIPDESC_DECL;
public:
	Renderable();
	~Renderable();

	bool CopyChip(Chip *chip) override;
	bool LoadChip(DocumentLoader &loader) override;
	bool SaveChip(DocumentSaver &saver) const override;

	// Renders object at identity world matrix.
	void CallChip() override;
	// Renders using given parameters. Frustum culling is performed here.
	void Render(const XMFLOAT4X4 &parentMatrix, uint32 instanceCount = 1, uint32 startInstanceLocation = 0, bool enableFrustumCulling =true) override;
	// Check if this object passes view frustum culling.
	bool CheckFrustumCulling(const XMFLOAT4X4 &worldMatrix);

	// Get/Set use frustum culling.
	bool IsUsingWholeObjectCulling() const { return _wholeObjectCulling; }
	void SetUseWholeObjectCulling(bool b) { _wholeObjectCulling = b; }

	// Get/Set use frustum culling for individual subsets.
	bool IsUsingSubsetCulling() const { return _subsetCulling; }
	void SetUseSubsetCulling(bool b) { _subsetCulling = b; }

	// Makes the renderable "universal" with growing material connections.
	void MakeUniversal();
	// Makes the renderable "specific" with the given subsets to draw. The string is the name of the subsets (only for convinience).
	void MakeSpecific(const List<std::pair<uint32, String>> &subsets);
	// Makes specific to *directly* connected geometry. Includes ALL subsets in default order.
	void UpdateToGeometry();
	// returns true if this renderable is "universal".
	bool IsUniversal() const { return _subsets.empty(); }
	// Gets the specific subsets to draw.
	void GetSpecificSubsets(List<std::pair<uint32, String>> &subsets);

	// Return our bounding box.
	const AxisAlignedBox &GetBoundingBox() const { return _boundingBox; }
	// Sets the bounding box.
	void SetBoundingBox(const AxisAlignedBox &aabb) { _boundingBox = aabb; }
	// Calculate boundingbox from geometry.
	void CalculateBoundingBox();

private:
	// This should be the bounding box for the subsets to draw. Can be calculated or set manually.
	AxisAlignedBox _boundingBox; 
	// true if we are to do initial view frustum culling using _boundingBox.
	bool _wholeObjectCulling = false;
	// true if we are to cull each subset in the geometry.
	bool _subsetCulling = true;

	// Subsets to draw. If empty, we have generic growable material connections and should draw all subsets in the connected geometry.
	// If not empty, each entry should have a material connection. The subsets are drawn in the order they appear in the list.
	List<uint32> _subsets; 

	Frustum::PlaneId _cullingPlane = Frustum::LEFTP; // This MAY improve performance in some cases by testing the frustum plane we failed last frame first.. Works good in theory :)
};


}