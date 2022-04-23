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
#include "ChipDialogs/StandardFormDialogPage.h"
#include "StdImporters/OpenAssetImpLib.h"
#include <qgridlayout.h>

namespace m3d
{


class STDIMPORTERS_DLG_API OpenAssetImpLib_Dlg : public StandardFormDialogPage
{
	Q_OBJECT
	DIALOGDESC_DECL
public:
	OpenAssetImpLib_Dlg();
	~OpenAssetImpLib_Dlg();

	OpenAssetImpLib *GetChip() { return (OpenAssetImpLib*)DialogPage::GetChip(); }

	virtual void Init();

protected:
	QGridLayout *_gl;
	QFormLayout *_layout2;

	QCheckBox *_CalcTangentSpace;
	QCheckBox *_JoinIdenticalVertices;
	QCheckBox *_GenNormals;
	QCheckBox *_GenSmoothNormals;
	QCheckBox *_SplitLargeMeshes;
	QCheckBox *_PreTransformVertices;
	QCheckBox *_ImproveCacheLocality;
	QCheckBox *_RemoveRedundantMaterials;
	QCheckBox *_FixInfacingNormals;
	QCheckBox *_FindDegenerates;
	QCheckBox *_FindInvalidData;
	QCheckBox *_GenUVCoords;
	QCheckBox *_TransformUVCoords;
	QCheckBox *_FindInstances;
	QCheckBox *_OptimizeMeshes;
	QCheckBox *_OptimizeGraph;
	QCheckBox *_SplitByBoneCount;
	QCheckBox *_Debone;
	QCheckBox *_force2CompTexcoords;

	QCheckBox *_removeNormals;
	QCheckBox *_removeTangentsAndBitangent;
	QCheckBox *_removeColors;
	QCheckBox *_removeTexcoord0;
	QCheckBox *_removeTexcoord1;
	QCheckBox *_removeTexcoord2;
	QCheckBox *_removeTexcoord3;
	QCheckBox *_removeBoneWeights;
	QCheckBox *_removeAnimations;
	QCheckBox *_removeMaterials;

//	
//	aiComponent_MESHES


	virtual void OnOK();

	virtual void CheckBoxUpdated(QCheckBox *widget, bool value);
};


}