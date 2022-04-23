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

#include "stdafx.h"
#include "OpenAssetImpLib_Dlg.h"
#include "ChipDialogs/ChipDialogManager.h"


using namespace m3d;


DIALOGDESC_DEF(OpenAssetImpLib_Dlg, OPENASSETIMPLIB_GUID);


OpenAssetImpLib_Dlg::OpenAssetImpLib_Dlg() : StandardFormDialogPage(false)
{
	delete _layout;
	_gl = new QGridLayout(this);
	_layout = new QFormLayout();
	_gl->addLayout(_layout, 0, 0);
	_layout2 = new QFormLayout();
	_gl->addLayout(_layout2, 0, 1);
}

OpenAssetImpLib_Dlg::~OpenAssetImpLib_Dlg()
{
}

void OpenAssetImpLib_Dlg::Init()
{
	unsigned flags = GetChip()->GetPostProcessFlags();
	unsigned removeComps = GetChip()->GetRemoveCompsFlag();

	_CalcTangentSpace = AddCheckBox("Calculate Tangent Space", flags & aiProcess_CalcTangentSpace); // AI_CONFIG_PP_CT_MAX_SMOOTHING_ANGLE (texcoord set for generation..)
	_CalcTangentSpace->setToolTip("Calculates the tangents and bitangents for the imported meshes. Does nothing if a mesh does not have normals. You might want this post processing step to be executed if you plan to use tangent space calculations such as normal mapping applied to the meshes.");
	_JoinIdenticalVertices = AddCheckBox("Join Identical Vertices", flags & aiProcess_JoinIdenticalVertices);
	_JoinIdenticalVertices->setToolTip("Identifies and joins identical vertex data sets within all imported meshes.");
	_GenNormals = AddCheckBox("Generate Normals", flags & aiProcess_GenNormals);
	_GenNormals->setToolTip("Generates normals for all faces of all meshes. This is ignored if normals are already there at the time this flag is evaluated");
	_GenSmoothNormals = AddCheckBox("Generate Smooth Normals", flags & aiProcess_GenSmoothNormals); // AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE
	_GenSmoothNormals->setToolTip("Generates smooth normals for all vertices in the mesh. This is ignored if normals are already there at the time this flag is evaluated.");
	_SplitLargeMeshes = AddCheckBox("Split Large Meshes", flags & aiProcess_SplitLargeMeshes); // AI_CONFIG_PP_SLM_VERTEX_LIMIT, AI_CONFIG_PP_SLM_TRIANGLE_LIMIT
	_SplitLargeMeshes->setToolTip("Splits large meshes into smaller sub-meshes. This is quite useful for real - time rendering, where the number of triangles which can be maximally processed in a single draw - call is limited by the video driver / hardware.");
	_PreTransformVertices = AddCheckBox("Pretransform Vertices", flags & aiProcess_PreTransformVertices); // (AI_CONFIG_PP_PTV_NORMALIZE)
	_PreTransformVertices->setToolTip("Removes the node graph and pre-transforms all vertices with the local transformation matrices of their nodes.");
	_ImproveCacheLocality = AddCheckBox("Improve Cache Locality", flags & aiProcess_ImproveCacheLocality); // AI_CONFIG_PP_ICL_PTCACHE_SIZE
	_ImproveCacheLocality->setToolTip("Reorders triangles for better vertex cache locality.");
	_RemoveRedundantMaterials = AddCheckBox("Remove Redundant Materials", flags & aiProcess_RemoveRedundantMaterials);
	_RemoveRedundantMaterials->setToolTip("Searches for redundant/unreferenced materials and removes them. This is especially useful in combination with the \'Pretransform Vertices\' and \'Optimize Meshes\' flags.");
	_FixInfacingNormals = AddCheckBox("Fix Infacing Normals", flags & aiProcess_FixInfacingNormals);
	_FixInfacingNormals->setToolTip("This step tries to determine which meshes have normal vectors that are facing inwards and inverts them.");
	_FindDegenerates = AddCheckBox("Find Degenerates", flags & aiProcess_FindDegenerates);
	_FindDegenerates->setToolTip("This step searches all meshes for degenerate primitives and converts them to proper lines or points.");
	_FindInvalidData = AddCheckBox("Find Invalid Data", flags & aiProcess_FindInvalidData);
	_FindInvalidData->setToolTip("This step searches all meshes for invalid data, such as zeroed normal vectors or invalid UV coords and removes / fixes them.This is intended to get rid of some common exporter errors.");
	_GenUVCoords = AddCheckBox("Generate UV-coordinates", flags & aiProcess_GenUVCoords);
	_GenUVCoords->setToolTip("This step converts non-UV mappings (such as spherical or cylindrical mapping) to proper texture coordinate channels.");
	_TransformUVCoords = AddCheckBox("Transform UV-coordinates", flags & aiProcess_TransformUVCoords);
	_TransformUVCoords->setToolTip("This step applies per-texture UV transformations and bakes them into stand - alone vtexture coordinate channels.");
	_FindInstances = AddCheckBox("Find Instances", flags & aiProcess_FindInstances);
	_FindInstances->setToolTip("This step searches for duplicate meshes and replaces them with references to the first mesh.");
	std::swap(_layout, _layout2);
	_OptimizeMeshes = AddCheckBox("Optimize Meshes", flags & aiProcess_OptimizeMeshes);
	_OptimizeMeshes->setToolTip("A postprocessing step to reduce the number of meshes. This will, in fact, reduce the number of draw calls. This is a very effective optimization and is recommended to be used together with \'Optimize Graph\', if possible.");
	_OptimizeGraph = AddCheckBox("Optimize Graph", flags & aiProcess_OptimizeGraph);
	_OptimizeGraph->setToolTip("A postprocessing step to optimize the scene hierarchy. Nodes without animations, bones, lights or cameras assigned are collapsed and joined.");
	_SplitByBoneCount = AddCheckBox("Split by Bone Count", flags & aiProcess_SplitByBoneCount);
	_SplitByBoneCount->setToolTip("This step splits meshes with many bones into sub-meshes so that each submesh has fewer or as many bones as a given limit.");
	_Debone = AddCheckBox("Debone", flags & aiProcess_Debone);
	_Debone->setToolTip("This step removes bones losslessly or according to some threshold.");
	_force2CompTexcoords = AddCheckBox("Force 2 component Texcoords", flags & EXTRAFLAG_force2CompTexcoords);
	_force2CompTexcoords->setToolTip("This will force all texture sets to have two components if the third/forth component are all zero.");
	_removeNormals = AddCheckBox("Remove Normals", removeComps & aiComponent_NORMALS);
	_removeNormals->setToolTip("Remove normal data from vertices.");
	_removeTangentsAndBitangent = AddCheckBox("Remove Tangents and Bitangents", removeComps & aiComponent_TANGENTS_AND_BITANGENTS);
	_removeTangentsAndBitangent->setToolTip("Remove tangents and bitangents from vertices.");
	_removeColors = AddCheckBox("Remove Vertex Colors", removeComps & aiComponent_COLORS);
	_removeColors->setToolTip("Remove colors from vertices.");
	_removeTexcoord0 = AddCheckBox("Remove Texcoord Set 0", removeComps & aiComponent_TEXCOORDSn(0));
	_removeTexcoord0->setToolTip("Remove Texcoord Set 0 from vertices.");
	_removeTexcoord1 = AddCheckBox("Remove Texcoord Set 1", removeComps & aiComponent_TEXCOORDSn(1));
	_removeTexcoord1->setToolTip("Remove Texcoord Set 1 from vertices.");
	_removeTexcoord2 = AddCheckBox("Remove Texcoord Set 2", removeComps & aiComponent_TEXCOORDSn(2));
	_removeTexcoord2->setToolTip("Remove Texcoord Set 2 from vertices.");
	_removeTexcoord3 = AddCheckBox("Remove Texcoord Set 3", removeComps & aiComponent_TEXCOORDSn(3));
	_removeTexcoord3->setToolTip("Remove Texcoord Set 3 from vertices.");
	_removeBoneWeights = AddCheckBox("Remove Bone Weights", removeComps & aiComponent_BONEWEIGHTS);
	_removeBoneWeights->setToolTip("Remove bone weights and indices from vertices.");
	_removeAnimations = AddCheckBox("Remove Animations", removeComps & aiComponent_ANIMATIONS);
	_removeAnimations->setToolTip("Remove animations.");
	_removeMaterials = AddCheckBox("Remove Materials", removeComps & aiComponent_MATERIALS);
	_removeMaterials->setToolTip("Remove material definitions.");
}

void OpenAssetImpLib_Dlg::OnOK()
{
	unsigned flags = 0, removeComps = 0;
	flags |= _CalcTangentSpace->isChecked() ? aiProcess_CalcTangentSpace : 0;
	flags |= _JoinIdenticalVertices->isChecked() ? aiProcess_JoinIdenticalVertices : 0;
	flags |= _GenNormals->isChecked() ? aiProcess_GenNormals : 0;
	flags |= _GenSmoothNormals->isChecked() ? aiProcess_GenSmoothNormals : 0;
	flags |= _SplitLargeMeshes->isChecked() ? aiProcess_SplitLargeMeshes : 0;
	flags |= _PreTransformVertices->isChecked() ? aiProcess_PreTransformVertices : 0;
	flags |= _ImproveCacheLocality->isChecked() ? aiProcess_ImproveCacheLocality : 0;
	flags |= _RemoveRedundantMaterials->isChecked() ? aiProcess_RemoveRedundantMaterials : 0;
	flags |= _FixInfacingNormals->isChecked() ? aiProcess_FixInfacingNormals : 0;
	flags |= _FindDegenerates->isChecked() ? aiProcess_FindDegenerates : 0;
	flags |= _FindInvalidData->isChecked() ? aiProcess_FindInvalidData : 0;
	flags |= _GenUVCoords->isChecked() ? aiProcess_GenUVCoords : 0;
	flags |= _TransformUVCoords->isChecked() ? aiProcess_TransformUVCoords : 0;
	flags |= _FindInstances->isChecked() ? aiProcess_FindInstances : 0;
	flags |= _OptimizeMeshes->isChecked() ? aiProcess_OptimizeMeshes : 0;
	flags |= _OptimizeGraph->isChecked() ? aiProcess_OptimizeGraph : 0;
	flags |= _SplitByBoneCount->isChecked() ? aiProcess_SplitByBoneCount : 0;
	flags |= _Debone->isChecked() ? aiProcess_Debone : 0;
	flags |= _force2CompTexcoords->isChecked() ? EXTRAFLAG_force2CompTexcoords : 0;
	removeComps |= _removeNormals->isChecked() ? aiComponent_NORMALS : 0;
	removeComps |= _removeTangentsAndBitangent->isChecked() ? aiComponent_TANGENTS_AND_BITANGENTS : 0;
	removeComps |= _removeColors->isChecked() ? aiComponent_COLORS : 0;
	removeComps |= _removeTexcoord0->isChecked() ? aiComponent_TEXCOORDSn(0) : 0;
	removeComps |= _removeTexcoord1->isChecked() ? aiComponent_TEXCOORDSn(1) : 0;
	removeComps |= _removeTexcoord2->isChecked() ? aiComponent_TEXCOORDSn(2) : 0;
	removeComps |= _removeTexcoord3->isChecked() ? aiComponent_TEXCOORDSn(3) : 0;
	removeComps |= _removeBoneWeights->isChecked() ? aiComponent_BONEWEIGHTS : 0;
	removeComps |= _removeAnimations->isChecked() ? aiComponent_ANIMATIONS : 0;
	removeComps |= _removeMaterials->isChecked() ? aiComponent_MATERIALS : 0;
	GetChip()->SetPostProcessFlags(flags);
	GetChip()->SetRemoveCompsFlag(removeComps);
}

void OpenAssetImpLib_Dlg::CheckBoxUpdated(QCheckBox *widget, bool value)
{
	if (widget == _GenNormals) {
		_GenSmoothNormals->setEnabled(!value);
		if (value)
			_GenSmoothNormals->setChecked(false);
	}
	if (widget == _GenSmoothNormals) {
		_GenNormals->setEnabled(!value);
		if (value)
			_GenNormals->setChecked(false);
	}
	if (widget == _PreTransformVertices) {
		_OptimizeGraph->setEnabled(!value);
		if (value)
			_OptimizeGraph->setChecked(false);
	}
	if (widget == _OptimizeGraph) {
		_PreTransformVertices->setEnabled(!value);
		if (value)
			_PreTransformVertices->setChecked(false);
	}

}