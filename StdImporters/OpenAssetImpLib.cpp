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
#include "OpenAssetImpLib.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ChipManager.h"
#include "M3DEngine/ClassManager.h"
#include "M3DEngine/DocumentManager.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "M3DEngine/DocumentFileTypes.h"
#include "StdChips/MatrixOperator.h"
#include "StdChips/Value.h"
#include "StdChips/VectorChip.h"
#include "StdChips/Caller.h"
#include "StdChips/Motion.h"
#include "StdChips/MatrixChip.h"
#include "StdChips/Shortcut.h"
#include "StdChips/MatrixArray.h"
#include "StdChips/VectorOperator.h"
#include "StdChips/MatrixOperator.h"
#include "GraphicsChips/Texture.h"
#include "GraphicsChips/Material.h"
#include "GraphicsChips/Renderable.h"
#include "GraphicsChips/StdGeometry.h"
#include "GraphicsChips/3DObject.h"
#include "GraphicsChips/Sampler.h"
#include "GraphicsChips/GraphicsBuffer.h"
#include "GraphicsChips/GraphicsState.h"
#include "GraphicsChips/RootSignature.h"
#include "GraphicsChips/Shaders.h"
#include "GraphicsChips/Shader.h"
#include "GraphicsChips/GraphicsMatrix.h"
#include "GraphicsChips/SkeletonController.h"

#include <assimp\LogStream.hpp>
#include <assimp\DefaultLogger.hpp>
#include <typeinfo>

using namespace m3d;



CHIPDESCV1_DEF_IMPORTER(OpenAssetImpLib, MTEXT("Open Asset Import Library"), OPENASSETIMPLIB_GUID, IMPORTER_GUID, MTEXT("3D;3DS;3MF;AC;AC3D;ACC;AMJ;ASE;ASK;B3D;BLEND;BVH;CMS;COB;DAE;DXF;ENFF;FBX;GLB;glTF;HMB;IFC;STEP;IRR;IRRMESH;LWO;LWS;LXO;MD2;MD3;MD5;MDC;MDL;MESH;MESH.XML;MOT;MS3D;NDO;NFF;OBJ;OFF;OGEX;PLY;PMX;PRJ;Q3O;Q3S;RAW;SCN;SIB;SMD;STP;STL;TER;UC;VTA;X;X3D;XGL;ZGL"));
	
//MTEXT("dae;blend;bvh;3ds;ase;obj;ply;dxf;ifc;nff;smd;vta;mdl;md2;md3;pk3;mdc;md5mesh;md5anim;md5camera;x;q3o;q3s;raw;ac;stl;dxf;irrmesh;irr;xml;off;ter;mdl;hmp;mesh.xml;skeleton.xml;material;ms3d;lwo;lws;lxo;csm;ply;cob;scn"));


OpenAssetImpLib::OpenAssetImpLib()
{
	_importedCG = nullptr;
	_skeletonInit = false;
	_bonesBufferConnected = false;
	_sg.skeleton = nullptr;
	_sg.skeletonController = nullptr;
	_sg.skeletonArray = nullptr;
	_sg.bonesBuffer = nullptr;
	_rootSignature = nullptr;
	_globalCBV = nullptr;

	_postProcessFlags = aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_RemoveRedundantMaterials | aiProcess_GenUVCoords | aiProcess_TransformUVCoords;
	_removeComps = 0;
	_useShortcuts = true;
	_numBones = 0;
}

OpenAssetImpLib::~OpenAssetImpLib()
{
}

void OpenAssetImpLib::_readTextures(const aiScene* scene, const aiMaterial *material, aiTextureType type, List<TexDesc> &textures)
{
	for (uint32 i = 0, j = material->GetTextureCount(type); i < j; i++) {
		TexDesc t;
		aiString p;
		material->GetTexture(type, i, &p, &t.mapping, &t.uvindex, &t.blend, &t.op, t.mapMode);
		if (t.blend == 0.0f)
			t.blend = 1.0f; // Hack!

		const aiTexture* tex = scene->GetEmbeddedTexture(p.C_Str());
		if (tex) {
			t.tex = tex;
			t.path = Path(p.C_Str());
		}
		else {
			t.path = Path(String(p.C_Str()), _filename); // A nonabsolute path if relative to imported document.
			if (!t.path.CheckExistence()) {
				if (t.path.IsAbsolute()) {
					String filename = t.path.GetName();
					Path baseDir = _filename.GetParentDirectory(), q = t.path.GetParentDirectory(), testPath = Path(filename, baseDir);
					while (!testPath.CheckExistence() && !q.IsDrive() && q.IsDirectory()) {
						filename = q.GetName() + MTEXT("\\") + filename;
						testPath = Path(filename, baseDir);
						q = q.GetParentDirectory();
					}
					if (testPath.CheckExistence()) {
						msg(WARN, MTEXT("File not found \'") + t.path.AsString() + MTEXT("\'. Using \'") + testPath.AsString() + MTEXT("\' instead!"));
						t.path = testPath;
					}
				}
				if (!t.path.CheckExistence()) {
					msg(WARN, strUtils::ConstructString(MTEXT("Texture file not found %1.")).arg(t.path.AsString()));
				}
			}
		}
		// TODO: Figure out image format and type.
		textures.push_back(t);
	}
}

Chip *OpenAssetImpLib::_loadTexture(const TexDesc &td, const Array<uint32, AI_MAX_NUMBER_OF_TEXTURECOORDS>& tcMap, M3D_SAMPLER_DESC &sampler, uint32 &cSet)
{
	Path p = td.path;

	Chip *texture = nullptr;

	{
		auto n = _textureMap.find(p); // Texture already loaded?
		if (n == _textureMap.end()) { // No?
			// Create the texture chip.
			Texture *tex = _addChip<Texture>(TEXTURE_GUID);
			if (!tex) {
				msg(WARN, MTEXT("Failed to create Texture"));
				return nullptr;
			}
			else {
				if (td.tex) {
					if (td.tex->mHeight != 0) {
						msg(WARN, MTEXT("Unsupported file format for embedded texture (\'") + p.AsString() + MTEXT("\')."));
					}
					else {
						DataBuffer db((const uint8*)td.tex->pcData, td.tex->mWidth);
						ImageFileFormat ifm = ImageFileFormat::IFF_UNKNOWN;

						if (td.tex->CheckFormat("dds"))			ifm = ImageFileFormat::IFF_DDS;
						else if (td.tex->CheckFormat("bmp"))	ifm = ImageFileFormat::IFF_BMP;
						else if (td.tex->CheckFormat("jpg"))	ifm = ImageFileFormat::IFF_JPG;
						else if (td.tex->CheckFormat("png"))	ifm = ImageFileFormat::IFF_PNG;
						else if (td.tex->CheckFormat("tif"))	ifm = ImageFileFormat::IFF_TIFF;
						else if (td.tex->CheckFormat("gif"))	ifm = ImageFileFormat::IFF_GIF;
						else if (td.tex->CheckFormat("wmp"))	ifm = ImageFileFormat::IFF_WMP;
						else if (td.tex->CheckFormat("tga"))	ifm = ImageFileFormat::IFF_TGA;
						else if (td.tex->CheckFormat("hdr"))	ifm = ImageFileFormat::IFF_HDR;
						else {
							msg(WARN, MTEXT("Unknown file format for embedded texture (\'") + p.AsString() + MTEXT("\')."));
						}
						if (ifm != ImageFileFormat::IFF_UNKNOWN) {
							if (!tex->SetImageData(std::move(db), ImageFileFormat::IFF_JPG)) {
								msg(WARN, MTEXT("Invalid data for embedded texture (\'") + p.AsString() + MTEXT("\')."));
							}
						}
					}
				}
				else {
					String texName = p.GetName();
					tex->SetName(texName);
					if (!tex->LoadImageDataFromFile(p)) {
						msg(WARN, MTEXT("Failed to load texture \'") + p.AsString() + MTEXT("\'."));
					}
				}
			}
			n = _textureMap.insert(std::make_pair(p, tex)).first;

			texture = n->second;
		}
		else if (_useShortcuts) {
			Shortcut *sc = _addChip<Shortcut>(SHORTCUT_GUID);
			if (sc) {
				sc->SetOriginal(n->second);
				texture = sc;
			}
			else
				texture = n->second;
		}
		else
			texture = n->second;
	}

	sampler = M3D_SAMPLER_DESC();

	switch (td.mapMode[0])
	{
	case aiTextureMapMode_Wrap:
		sampler.AddressU = M3D_TEXTURE_ADDRESS_MODE_WRAP;
		break;
	case aiTextureMapMode_Clamp:
		sampler.AddressU = M3D_TEXTURE_ADDRESS_MODE_CLAMP;
		break;
	case aiTextureMapMode_Decal:
		sampler.AddressU = M3D_TEXTURE_ADDRESS_MODE_BORDER;
		(XMFLOAT4&)sampler.BorderColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		break;
	case aiTextureMapMode_Mirror:
		sampler.AddressU = M3D_TEXTURE_ADDRESS_MODE_MIRROR;
		break;
	}

	switch (td.mapMode[1])
	{
	case aiTextureMapMode_Wrap:
		sampler.AddressV = M3D_TEXTURE_ADDRESS_MODE_WRAP;
		break;
	case aiTextureMapMode_Clamp:
		sampler.AddressV = M3D_TEXTURE_ADDRESS_MODE_CLAMP;
		break;
	case aiTextureMapMode_Decal:
		sampler.AddressV = M3D_TEXTURE_ADDRESS_MODE_BORDER;
		(XMFLOAT4&)sampler.BorderColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		break;
	case aiTextureMapMode_Mirror:
		sampler.AddressV = M3D_TEXTURE_ADDRESS_MODE_MIRROR;
		break;
	}

	if (td.uvindex < AI_MAX_NUMBER_OF_TEXTURECOORDS && tcMap[td.uvindex] != -1)
		cSet = tcMap[td.uvindex];
	else { // Texture set does not exist, use first available!
		for (uint32 i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; i++) {
			if (tcMap[i] != -1) {
				cSet = i;
				break;
			}
		}
	}
	
	return texture;
}

void OpenAssetImpLib::_processTextures(const List<TexDesc> &textures, aiTextureType type, uint32 &counter, const Array<uint32, AI_MAX_NUMBER_OF_TEXTURECOORDS>& tcMap, Material *material, List<TexStage> &ts)
{
	for (size_t i = 0; i < textures.size(); i++) {
		const TexDesc &td = textures[i];
		M3D_SAMPLER_DESC sDesc;
		uint32 cSet = 0;
		Chip *texture = _loadTexture(td, tcMap, sDesc, cSet);
		material->SetChild(texture, 4, counter);

		uint32 sIdx = 0;
		const bool allowDefaultSamplers = true;
		if (allowDefaultSamplers) {
			// We know only wrap-mode is updated _loadTexture(...)
			// Check if it matches one of the default samplers:
			if (sDesc.AddressU == M3D_TEXTURE_ADDRESS_MODE_WRAP && sDesc.AddressV == M3D_TEXTURE_ADDRESS_MODE_WRAP)
				sIdx = DEFAULT_WRAPPED_SAMPLER;
			else if (sDesc.AddressU == M3D_TEXTURE_ADDRESS_MODE_WRAP && sDesc.AddressV == M3D_TEXTURE_ADDRESS_MODE_WRAP)
				sIdx = DEFAULT_SAMPLER;
		}

		if (sIdx == 0) {
			// Find and set sampler.
			uint32 sCount = material->GetSubConnectionCount(5);
			Chip* sampler = nullptr;
			auto n = _samplers.find(sDesc);
			if (n == _samplers.end()) {
				Sampler* s = _addChip<Sampler>(SAMPLER_GUID);
				if (s) {
					s->SetSamplerDesc(sDesc);
					n = _samplers.insert(std::make_pair(sDesc, s)).first;
					sampler = s;
					sIdx = sCount;
				}
			}
			else {
				for (; sIdx < sCount; sIdx++) {
					sampler = material->GetRawChild(5, sIdx);
					if ((sampler->AsShortcut() ? sampler->AsShortcut()->GetOriginal() : sampler) == n->second)
						break;
				}
				if (sIdx == sCount) {
					sampler = n->second;
					if (_useShortcuts) {
						Shortcut* sc = _addChip<Shortcut>(SHORTCUT_GUID);
						if (sc) {
							sc->SetOriginal(sampler);
							sampler = sc;
						}
					}
				}
			}

			if (sampler)
				material->SetChild(sampler, 5, sIdx);
		}

		TexStage s = { type, cSet, sIdx, td.op, td.mapping, td.blend };
		ts.push_back(s);
		counter++;
	}
}

Chip *OpenAssetImpLib::_getVertexShader(const ShaderDesc &sd)
{
	auto n = _vsMap.find(sd.vsd);
	if (n == _vsMap.end()) {
		Shader *vs = _addChip<Shader>(SHADER_GUID);
		vs->SetName(MTEXT("Vertex Shader"));
		n = _vsMap.insert(std::make_pair(sd.vsd, vs)).first;
		_genVS(vs, sd);
	}
	else if (_useShortcuts) {
		Shortcut *sc = _addChip<Shortcut>(SHORTCUT_GUID);
		if (sc) {
			sc->SetOriginal(n->second);
			return sc;
		}
	}
	return n->second;
}

Chip *OpenAssetImpLib::_getShaders(const ShaderDesc &sd)
{
	auto n = _shadersMap.find(sd);
	if (n == _shadersMap.end()) {
		Shaders *shaders = _addChip<Shaders>(SHADERS_GUID);
		n = _shadersMap.insert(std::make_pair(sd, shaders)).first;
		Chip *vs = _getVertexShader(sd);
		shaders->SetChild(vs, 0, 0);

		Shader *ps = _addChip<Shader>(SHADER_GUID);
		ps->SetName(MTEXT("Pixel Shader"));
		shaders->SetChild(ps, 4, 0);
		_genPS(ps, sd);
	}
	else if (_useShortcuts) {
		Shortcut *sc = _addChip<Shortcut>(SHORTCUT_GUID);
		if (sc) {
			sc->SetOriginal(n->second);
			return sc;
		}
	}
	return n->second;
}

Chip *OpenAssetImpLib::_getGraphicsState(const StateDesc &sd)
{
	auto n = _stateMap.find(sd);
	if (n == _stateMap.end()) {
		GraphicsState *gs = _addChip<GraphicsState>(GRAPHICSSTATE_GUID);
		if (!gs)
			return nullptr;
		GraphicsState::StateElementMap elements;
		elements.insert(std::make_pair(GraphicsState::Element::RS_FM, GraphicsState::ElementValue(sd.wireframe != 0 ? M3D_FILL_MODE_WIREFRAME : M3D_FILL_MODE_SOLID))); // M3D_FILL_MODE_SOLID is default.
		elements.insert(std::make_pair(GraphicsState::Element::RS_CM, GraphicsState::ElementValue(sd.twosided != 0 ? M3D_CULL_MODE_NONE : M3D_CULL_MODE_BACK))); // M3D_CULL_MODE_BACK is default.
		elements.insert(std::make_pair(GraphicsState::Element::BS_RT0_BE, GraphicsState::ElementValue(TRUE)));
		elements.insert(std::make_pair(GraphicsState::Element::BS_RT0_SB, GraphicsState::ElementValue(sd.blendMode == aiBlendMode_Default ? M3D_BLEND_SRC_ALPHA : M3D_BLEND_ONE)));
		elements.insert(std::make_pair(GraphicsState::Element::BS_RT0_DB, GraphicsState::ElementValue(sd.blendMode == aiBlendMode_Default ? M3D_BLEND_INV_SRC_ALPHA : M3D_BLEND_ONE)));
		gs->SetStateElements(elements);
		n = _stateMap.insert(std::make_pair(sd, gs)).first;
		if (!_rootSignature) {
			_rootSignature = _addChip<RootSignature>(ROOTSIGNATURE_GUID);
			gs->SetChild(_rootSignature, 0, 0);
		}
		else {
			gs->SetChild(_rootSignature, 0, 0);
			if (_useShortcuts) {
				Shortcut *sc = _addChip<Shortcut>(SHORTCUT_GUID);
				if (sc) {
					sc->SetOriginal(_rootSignature);
					gs->SetChild(sc, 0, 0);
				}
			}
		}
		Chip *shaders = _getShaders(sd.shaderDesc);
		gs->SetChild(shaders, 1, 0);
	}
	else if (_useShortcuts) {
		Shortcut *sc = _addChip<Shortcut>(SHORTCUT_GUID);
		if (sc) {
			sc->SetOriginal(n->second);
			return sc;
		}
	}
	return n->second;
}

Chip *OpenAssetImpLib::_processMaterial(const MaterialDesc &md, const aiScene* scene)
{
	auto n = _materialMap.find(md); // Check if we have processed this material before.
	if (n != _materialMap.end()) {
		if (_useShortcuts) {
			Shortcut *sc = _addChip<Shortcut>(SHORTCUT_GUID);
			if (sc) {
				sc->SetOriginal(n->second);
				return sc;
			}
		}
		return n->second;
	}

	aiString name;
	if (md.material->Get(AI_MATKEY_NAME, name) != aiReturn_SUCCESS)
		name = "Unnamed material";

	aiShadingMode shadingModel = aiShadingMode_Gouraud;
	if (md.material->Get(AI_MATKEY_SHADING_MODEL, (int32&)shadingModel) != aiReturn_SUCCESS)
		shadingModel = aiShadingMode_Gouraud;
	
	aiColor3D ambientColor(0.f,0.f,0.f);
	if (md.material->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor) != aiReturn_SUCCESS)
		ambientColor = aiColor3D(1.0f, 1.0f, 1.0f);

	// NOTE about Collada: If color not found, a default value is used. Defined in Assimp ColladaHelper, line ~517...
	aiColor3D diffuseColor(0.f,0.f,0.f);
	if (md.material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) != aiReturn_SUCCESS)
		diffuseColor = aiColor3D(1.0f, 1.0f, 1.0f);


	aiColor3D specularColor(0.f,0.f,0.f);
	if (md.material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) != aiReturn_SUCCESS)
		specularColor = aiColor3D(1.0f, 1.0f, 1.0f);

	aiColor3D emissiveColor(0.f,0.f,0.f);
	if (md.material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor) != aiReturn_SUCCESS)
		emissiveColor = aiColor3D(0.0f, 0.0f, 0.0f);

	aiColor3D transparentColor(0.f,0.f,0.f);
	if (md.material->Get(AI_MATKEY_COLOR_TRANSPARENT, transparentColor) != aiReturn_SUCCESS)
		transparentColor = aiColor3D(0.0f, 0.0f, 0.0f);

	aiColor3D reflectiveColor(0.f, 0.f, 0.f);
	if (md.material->Get(AI_MATKEY_COLOR_REFLECTIVE, reflectiveColor) != aiReturn_SUCCESS)
		reflectiveColor = aiColor3D(0.0f, 0.0f, 0.0f);

	float32 opacity = 1.0f;
	if (md.material->Get(AI_MATKEY_OPACITY, opacity) != aiReturn_SUCCESS)
		opacity = 1.0f;
	if (opacity == 0.0f)
		opacity = 1.0f; //  Hack: A fully transparent object is boring!

	float32 shininess = 0.0f;
	if (md.material->Get(AI_MATKEY_SHININESS, shininess) != aiReturn_SUCCESS)
		shininess = 100.0f;
	if (shininess <= 1.0f)
		shininess = shininess * 128.0f;

	float32 shininessStrength = 1.0f;
	if (md.material->Get(AI_MATKEY_SHININESS_STRENGTH, shininessStrength) != aiReturn_SUCCESS)
		shininessStrength = 1.0f;

	float32 bumpScaling = 0.0f;
	if (md.material->Get(AI_MATKEY_BUMPSCALING, bumpScaling) != aiReturn_SUCCESS)
		bumpScaling = 0.0f;

	float32 reflectivity = 0.0f;
	if (md.material->Get(AI_MATKEY_REFLECTIVITY, reflectivity) != aiReturn_SUCCESS)
		reflectivity = 0.0f;

	float32 refraction = 1.0f;
	if (md.material->Get(AI_MATKEY_REFRACTI, refraction) != aiReturn_SUCCESS)
		refraction = 1.0f;
	
	int32 wireframe = 0;
	if (md.material->Get(AI_MATKEY_ENABLE_WIREFRAME, wireframe) != aiReturn_SUCCESS)
		wireframe = 0;

	int32 twosided = 0;
	if (md.material->Get(AI_MATKEY_TWOSIDED, twosided) != aiReturn_SUCCESS)
		twosided = 0;

	int32 blendFunc = aiBlendMode_Default;
	if (md.material->Get(AI_MATKEY_BLEND_FUNC, blendFunc) != aiReturn_SUCCESS)
		blendFunc = aiBlendMode_Default;

	List<TexDesc> ambientTextures;
	_readTextures(scene, md.material, aiTextureType_AMBIENT, ambientTextures);

	List<TexDesc> diffuseTextures;
	_readTextures(scene, md.material, aiTextureType_DIFFUSE, diffuseTextures);

	List<TexDesc> specularTextures;
	_readTextures(scene, md.material, aiTextureType_SPECULAR, specularTextures);

	List<TexDesc> emissiveTextures;
	_readTextures(scene, md.material, aiTextureType_EMISSIVE, emissiveTextures);

	List<TexDesc> shininessTextures;
	_readTextures(scene, md.material, aiTextureType_SHININESS, shininessTextures);

	List<TexDesc> normalTextures;
	_readTextures(scene, md.material, aiTextureType_NORMALS, normalTextures);

	List<TexDesc> displacementTextures;
	_readTextures(scene, md.material, aiTextureType_DISPLACEMENT, displacementTextures);

	List<TexDesc> lightmapTextures;
	_readTextures(scene, md.material, aiTextureType_LIGHTMAP, lightmapTextures);

	List<TexDesc> opacityTextures;
	_readTextures(scene, md.material, aiTextureType_OPACITY, opacityTextures);

	List<TexDesc> reflectionTextures;
	_readTextures(scene, md.material, aiTextureType_REFLECTION, reflectionTextures);

	List<TexDesc> heightTextures;
	_readTextures(scene, md.material, aiTextureType_HEIGHT, heightTextures);

	List<TexDesc> unknownTextures;
	_readTextures(scene, md.material, aiTextureType_UNKNOWN, unknownTextures);

	// Create the material chip.
	Material *mat = _addChip<Material>(MATERIAL_GUID);
	if (!mat) {
		msg(WARN, MTEXT("Failed to create Material"));
		return nullptr;
	}

	// Set name.
	if (name.length)
		mat->SetName(name.C_Str());

	GraphicsBuffer *materialCBV = _addChip<GraphicsBuffer>(GRAPHICSBUFFER_GUID);
	materialCBV->SetName(MTEXT("Material Constants"));
	mat->SetChild(materialCBV, 3, 0);

	Value *valShininess = _addChip<Value>(VALUE_GUID);
	valShininess->SetName(MTEXT("Shininess"));
	valShininess->SetValue(shininess);
	materialCBV->SetChild(valShininess, 0, 0);

	Value *valShininessStrength = _addChip<Value>(VALUE_GUID);
	valShininessStrength->SetName(MTEXT("Shininess Strength"));
	valShininessStrength->SetValue(shininessStrength);
	materialCBV->SetChild(valShininessStrength, 0, 1);

	Value *valOpacity = _addChip<Value>(VALUE_GUID);
	valOpacity->SetName(MTEXT("Opacity"));
	valOpacity->SetValue(opacity);
	materialCBV->SetChild(valOpacity, 0, 2);

	Value *valBumpScaling = _addChip<Value>(VALUE_GUID);
	valBumpScaling->SetName(MTEXT("Bump Scaling"));
	valBumpScaling->SetValue(bumpScaling);
	materialCBV->SetChild(valBumpScaling, 0, 3);
/*	NOTE: These "exotic" properties were deliberately left out to reduce the complextivity of the CBV.
	Value *valReflectivity = _addChip<Value>(VALUE_GUID);
	valReflectivity->SetName(MTEXT("Reflectivity"));
	valReflectivity->SetValue(reflectivity);
	materialCBV->SetChild(valReflectivity, 0, 4);

	Value *valRefraction = _addChip<Value>(VALUE_GUID);
	valRefraction->SetName(MTEXT("Refraction"));
	valRefraction->SetValue(refraction);
	materialCBV->SetChild(valRefraction, 0, 5);
*/
	VectorChip *vecAmbient = _addChip<VectorChip>(VECTORCHIP_GUID);
	vecAmbient->SetName(MTEXT("Ambient Color"));
	vecAmbient->SetVector(XMFLOAT4(ambientColor.r, ambientColor.g, ambientColor.b, 1.0f));
	materialCBV->SetChild(vecAmbient, 0, 4);

	VectorChip *vecDiffuse = _addChip<VectorChip>(VECTORCHIP_GUID);
	vecDiffuse->SetName(MTEXT("Diffuse Color"));
	vecDiffuse->SetVector(XMFLOAT4(diffuseColor.r, diffuseColor.g, diffuseColor.b, 1.0f));
	materialCBV->SetChild(vecDiffuse, 0, 5);

	VectorChip *vecSpecular = _addChip<VectorChip>(VECTORCHIP_GUID);
	vecSpecular->SetName(MTEXT("Specular Color"));
	vecSpecular->SetVector(XMFLOAT4(specularColor.r, specularColor.g, specularColor.b, 1.0f));
	materialCBV->SetChild(vecSpecular, 0, 6);

	VectorChip *vecEmissive = _addChip<VectorChip>(VECTORCHIP_GUID);
	vecEmissive->SetName(MTEXT("Emissive Color"));
	vecEmissive->SetVector(XMFLOAT4(emissiveColor.r, emissiveColor.g, emissiveColor.b, 1.0f));
	materialCBV->SetChild(vecEmissive, 0, 7);
/*	NOTE: These "exotic" properties were deliberately left out to reduce the complextivity of the CBV.
	VectorChip *vecTransparent = _addChip<VectorChip>(VECTORCHIP_GUID);
	vecTransparent->SetName(MTEXT("Transparent Color"));
	vecTransparent->SetVector(XMFLOAT4(transparentColor.r, transparentColor.g, transparentColor.b, 1.0f));
	materialCBV->SetChild(vecTransparent, 0, 8);

	VectorChip *vecReflective = _addChip<VectorChip>(VECTORCHIP_GUID);
	vecReflective->SetName(MTEXT("Reflective Color"));
	vecReflective->SetVector(XMFLOAT4(reflectiveColor.r, reflectiveColor.g, reflectiveColor.b, 1.0f));
	materialCBV->SetChild(vecReflective, 0, 9);
*/

	if (shadingModel != aiShadingMode_NoShading && md.vertexLayout.hasNormals) {
		if (_globalCBV)
		{
			if (_useShortcuts) {
				Shortcut* sc = _addChip<Shortcut>(SHORTCUT_GUID);
				if (sc) {
					sc->SetOriginal(_globalCBV);
					mat->SetChild(sc, 3, 1);
				}
				else
					mat->SetChild(_globalCBV, 3, 1);
			}
			else
				mat->SetChild(_globalCBV, 3, 1);
		}
		else {
			_globalCBV = _addChip<GraphicsBuffer>(GRAPHICSBUFFER_GUID);
			_globalCBV->SetName(MTEXT("Global Constants"));
			VectorOperator* vecLightDir = _addChip<VectorOperator>(VECTOROPERATOR_GUID);
			vecLightDir->SetOperatorType(VectorOperator::OperatorType::TRANSFORM_NORMAL);
			vecLightDir->SetName(MTEXT("Light Dir"));
			VectorChip* vecLightDirC = _addChip<VectorChip>(VECTORCHIP_GUID);
			vecLightDirC->SetVector(XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f));
			vecLightDir->SetChild(vecLightDirC, 0, 0);
			MatrixOperator* mOp = _addChip<MatrixOperator>(MATRIXOPERATOR_GUID);
			mOp->SetOperatorType(MatrixOperator::OperatorType::INVERSE);
			vecLightDir->SetChild(mOp, 1, 0);
			GraphicsMatrix* gMatrix = _addChip<GraphicsMatrix>(GRAPHICSMATRIX_GUID);
			gMatrix->SetOperatorType(GraphicsMatrix::OperatorType::VIEW_MATRIX);
			gMatrix->SetName(MTEXT("Get/Set View Matrix"));
			mOp->SetChild(gMatrix, 0, 0);

			_globalCBV->SetChild(vecLightDir, 0, 0);

			VectorChip* vecLightAmbient = _addChip<VectorChip>(VECTORCHIP_GUID);
			vecLightAmbient->SetName(MTEXT("Light Ambient"));
			vecLightAmbient->SetVector(XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));
			_globalCBV->SetChild(vecLightAmbient, 0, 1);

			VectorChip* vecLightDiffuse = _addChip<VectorChip>(VECTORCHIP_GUID);
			vecLightDiffuse->SetName(MTEXT("Light Diffuse"));
			vecLightDiffuse->SetVector(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
			_globalCBV->SetChild(vecLightDiffuse, 0, 2);

			VectorChip* vecLightSpecular = _addChip<VectorChip>(VECTORCHIP_GUID);
			vecLightSpecular->SetName(MTEXT("Light Specular"));
			vecLightSpecular->SetVector(XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));
			_globalCBV->SetChild(vecLightSpecular, 0, 3);

			mat->SetChild(_globalCBV, 3, 1);
		}
	}

	StateDesc sd;
	sd.wireframe = wireframe;
	sd.twosided = twosided;
	sd.blendMode = (aiBlendMode)blendFunc;
	sd.shaderDesc.shadingMode = shadingModel;
	sd.shaderDesc.vsd.vl = md.vertexLayout;
	sd.shaderDesc.vsd.skeletonBufferIdx = -1;

	uint32 textureCount = 0;

	_processTextures(ambientTextures, aiTextureType_AMBIENT, textureCount, md.tcMap, mat, sd.shaderDesc.texStages);
	_processTextures(lightmapTextures, aiTextureType_LIGHTMAP, textureCount, md.tcMap, mat, sd.shaderDesc.texStages);
	_processTextures(diffuseTextures, aiTextureType_DIFFUSE, textureCount, md.tcMap, mat, sd.shaderDesc.texStages);
	_processTextures(specularTextures, aiTextureType_SPECULAR, textureCount, md.tcMap, mat, sd.shaderDesc.texStages);
	_processTextures(emissiveTextures, aiTextureType_EMISSIVE, textureCount, md.tcMap, mat, sd.shaderDesc.texStages);
	_processTextures(shininessTextures, aiTextureType_SHININESS, textureCount, md.tcMap, mat, sd.shaderDesc.texStages);
	_processTextures(normalTextures, aiTextureType_NORMALS, textureCount, md.tcMap, mat, sd.shaderDesc.texStages);
	_processTextures(displacementTextures, aiTextureType_DISPLACEMENT, textureCount, md.tcMap, mat, sd.shaderDesc.texStages);
	_processTextures(opacityTextures, aiTextureType_OPACITY, textureCount, md.tcMap, mat, sd.shaderDesc.texStages);
	_processTextures(reflectionTextures, aiTextureType_REFLECTION, textureCount, md.tcMap, mat, sd.shaderDesc.texStages);
	_processTextures(heightTextures, aiTextureType_HEIGHT, textureCount, md.tcMap, mat, sd.shaderDesc.texStages);
	_processTextures(unknownTextures, aiTextureType_UNKNOWN, textureCount, md.tcMap, mat, sd.shaderDesc.texStages);

	if (md.sg) {
		sd.shaderDesc.vsd.skeletonBufferIdx = textureCount;
		Chip *c = md.sg->bonesBuffer;
		if (_useShortcuts && _bonesBufferConnected) {
			Shortcut *sc = _addChip<Shortcut>(SHORTCUT_GUID);
			if (sc) {
				sc->SetOriginal(md.sg->bonesBuffer);
				c = sc;
			}
		}
		mat->SetChild(c, 4, textureCount++);
		_bonesBufferConnected = true;
	}

	Chip *graphicsState = _getGraphicsState(sd);
	mat->SetChild(graphicsState, 0, 0);

	_materialMap.insert(std::make_pair(md, mat));

	return mat;
}


XMFLOAT4X4 toXMMatrix(aiMatrix4x4 m) { return XMFLOAT4X4(m.a1, m.b1, m.c1, m.d1, m.a2, m.b2, m.c2, m.d2, m.a3, m.b3, m.c3, m.d3, m.a4, m.b4, m.c4, m.d4); }


Renderable *OpenAssetImpLib::_processMeshList(const MeshList &meshes, BoneMap &bones, const aiScene *scene)
{
	// Create the renderable and geometry to represent meshes.
	Renderable *renderable = _addChip<Renderable>(RENDERABLE_GUID);
	if (!renderable) {
		msg(WARN, MTEXT("Failed to create StdGeometry."));
		return nullptr;
	}
	StdGeometry *geometry = _addChip<StdGeometry>(STDGEOMETRY_GUID);
	if (!geometry) {
		msg(WARN, MTEXT("Failed to create StdGeometry."));
		return nullptr;
	}
	renderable->SetChild(geometry, 0, 0);

	uint32 baseIndex = 0;

	// Iterate meshes in the list.
	for (size_t i = 0; i < meshes.size(); i++) {
		aiMesh *mesh = meshes[i];

		VertexLayout vl = { 0 };

		if (mesh->mPrimitiveTypes != aiPrimitiveType_POINT && mesh->mPrimitiveTypes != aiPrimitiveType_LINE && mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
			return nullptr; // Skip it, polygons not supported, neighter are mixed types within a single subset! (We have set the flag aiProcess_Triangulate to take care of n-sided polygons)

		if (vl.hasPositions = mesh->HasPositions())
			for (uint32 j = 0; j < mesh->mNumVertices; j++)
				geometry->AddPosition((const XMFLOAT3&)mesh->mVertices[j]);

		if (vl.hasNormals = mesh->HasNormals())
			for (uint32 j = 0; j < mesh->mNumVertices; j++)
				geometry->AddNormal((const XMFLOAT3&)mesh->mNormals[j]);

		bool colorFound = false;
		for (uint32 k = 0; k < mesh->GetNumColorChannels(); k++) {
			if (mesh->HasVertexColors(k)) {
				if (colorFound) {
					msg(WARN, MTEXT("Skipping color set. Only one color set is supported!"));
				}
				else {
					for (uint32 j = 0; j < mesh->mNumVertices; j++)
						geometry->AddColor(XMCOLOR((const float32*)&mesh->mColors[k][j]));
					colorFound = vl.hasColors = true;
				}
			}
		}

		if (mesh->HasTangentsAndBitangents()) {
			// NOTE: Sometimes, if the model contains triangles with two equal vertices/texcoord, mTangents/mBitangents will contain -1.#IND.
			// Check CalcTangentsProcess.cpp (ca line 200) in assimp. Is this a bug/issue in that lib that can be fixed somehow?
			// For now, I check for #IND in (bi)tangents, and just set to normal if found. (degenerates not visible anyway)...

			vl.hasBitangents = vl.hasTangent = true;

			for (uint32 j = 0; j < mesh->mNumVertices; j++) {
				const XMFLOAT3 &t = (const XMFLOAT3&)mesh->mTangents[j];
				const XMFLOAT3 &b = (const XMFLOAT3&)mesh->mBitangents[j];
				if (t.x != t.x || b.y != b.y) { // Checks only x. Should be enough! (Checks for -1.#IND)
					if (mesh->HasNormals()) {
						geometry->AddTangent((const XMFLOAT3&)mesh->mNormals[j]);
						geometry->AddBitangent((const XMFLOAT3&)mesh->mNormals[j]);
					}
					else {
						geometry->AddTangent(XMFLOAT3(1.0f, 0.0f, 0.0f));
						geometry->AddBitangent(XMFLOAT3(0.0f, 1.0f, 0.0f));
					}
					continue;
				}
				geometry->AddTangent(t);
				geometry->AddBitangent(b);
			}
		} 
		
		uint32 tcCount = 0;

		MaterialDesc md;
		md.material = scene->mMaterials[mesh->mMaterialIndex];
		md.sg = nullptr;

		for (uint32 k = 0; k < AI_MAX_NUMBER_OF_TEXTURECOORDS; k++) {
			if (mesh->HasTextureCoords(k)) {
				if (tcCount < StdGeometry::MAX_TEXCOORD_SETS) { // Within max number of coord-sets?
					switch ((_postProcessFlags & EXTRAFLAG_force2CompTexcoords) ? 2 : mesh->mNumUVComponents[k])
					{
					case 1:
						for (uint32 j = 0; j < mesh->mNumVertices; j++)
							geometry->AddTexCoord((const float32&)mesh->mTextureCoords[k][j], tcCount);
						vl.hasTexCoords[tcCount] = 1;
						break;
					case 2: 
						for (uint32 j = 0; j < mesh->mNumVertices; j++)
							geometry->AddTexCoord((const XMFLOAT2&)mesh->mTextureCoords[k][j], tcCount);
						vl.hasTexCoords[tcCount] = 2;
						break;
					case 3:
						for (uint32 j = 0; j < mesh->mNumVertices; j++)
							geometry->AddTexCoord((const XMFLOAT3&)mesh->mTextureCoords[k][j], tcCount);
						vl.hasTexCoords[tcCount] = 3;
						break;
					case 4: // Not supported by assimp!
					default: break; // Invalid!
					}
					md.tcMap[k] = tcCount++; // Mapping from assimp-coordset to us.
					continue;
				}
				msg(WARN, strUtils::format(MTEXT("Skipping texture set %i for Geometry because of too many texture sets."), k), geometry);
			}
			md.tcMap[k] = -1; // Set k does not map!
		}

		// Process bones, if any.
		if (mesh->HasBones()) {
			List<std::pair<XMUSHORT4, XMFLOAT4>> blends; // BlendIndices (bonenr), BlendWeights for each vertex (weights should add up to 1.0)
			blends.resize(mesh->mNumVertices, std::make_pair(XMUSHORT4(0.0f, 0.0f, 0.0f, 0.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
			
			for (uint32 j = 0; j < mesh->mNumBones; j++) { // Iterate bones
				aiBone *bone = mesh->mBones[j];

				aiNode *boneNode = scene->mRootNode->FindNode(bone->mName); // Find the node representing this bone.

				// Create the mapping from the boneNode to the boneIndex and jointMatrix (offsetMatrix). 
				// NOTE: what if different bones refering the same node have different mOffsetMatrix (inverse bind-pose)?!? Weakness in assimplib?!
				// boneNode may already exist, leaving our mOffsetMatrix unused and possible different from the existing one. Dunno if this is a real world problem, though....
				uint16 boneIndex = bones.insert(std::make_pair(boneNode, std::make_pair((uint16)bones.size(), toXMMatrix(bone->mOffsetMatrix)))).first->second.first;

				for (uint32 k = 0; k < bone->mNumWeights; k++) { // Iterate weights for each bone. These should be limited to 4 by using the aiProcess_LimitBoneWeights-flag (set at init)!
					const aiVertexWeight &w = bone->mWeights[k];
					std::pair<XMUSHORT4, XMFLOAT4> &vertex = blends[w.mVertexId];
					for (uint32 p = 0; p < 4; p++) {
						if (((const float32*)&vertex.second.x)[p] == 0.0f) {
							((uint16*)&vertex.first.x)[p] = boneIndex;
							((float32*)&vertex.second.x)[p] = w.mWeight;
							break;
						} 
					}
				}
			}

			vl.hasBlendWeigtsAndIndices = true;

			// Add blend indices and weights to geometry.
			for (size_t j = 0; j < blends.size(); j++) {
				geometry->AddBlendIndices(blends[j].first);
				geometry->AddBlendWeights(XMUBYTEN4(&blends[j].second.x));
			}

			// Create the skeleton, if it does not exist.
			if (!_skeletonInit) {
				_skeletonInit = true;
				_sg.skeleton = _addChip<Skeleton>(SKELETON_GUID);
				_sg.skeletonController = _addChip<SkeletonController>(SKELETONCONTROLLER_GUID);
				_sg.skeletonArray = _addChip<MatrixArray>(MATRIXARRAY_GUID);
				_sg.skeletonController->SetChild(_sg.skeleton, 0, 0);
				_sg.skeletonController->SetChild(_sg.skeletonArray, 2, 0);
				_sg.bonesBuffer = _addChip<GraphicsBuffer>(GRAPHICSBUFFER_GUID);
				Chip *c = _sg.skeletonArray;
				if (_useShortcuts) {
					Shortcut *sc = _addChip<Shortcut>(SHORTCUT_GUID);
					if (sc) {
						sc->SetOriginal(_sg.skeletonArray);
						c = sc;
					}
				}
				_sg.bonesBuffer->SetChild(c, 0, 0);
			}
			md.sg = &_sg;
		}

		if (mesh->HasFaces()) { // Add the indices! Should ALWAYS be present!
			for (uint32 j = 0; j < mesh->mNumFaces; j++) {
				const aiFace &face = mesh->mFaces[j];
				for (uint32 k = 0; k < face.mNumIndices; k++)
					geometry->AddIndex(baseIndex + face.mIndices[k]);
			}
		}

		String subsetName = mesh->mName.C_Str();

		switch (mesh->mPrimitiveTypes) {
		case aiPrimitiveType_POINT:
			geometry->CommitSubset(M3D_PRIMITIVE_TOPOLOGY_POINTLIST, subsetName);
			break;
		case aiPrimitiveType_LINE:
			geometry->CommitSubset(M3D_PRIMITIVE_TOPOLOGY_LINELIST, subsetName);
			break;
		case aiPrimitiveType_TRIANGLE:
			geometry->CommitSubset(M3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, subsetName);
			break;
		default:
			break;
		}

		baseIndex += mesh->mNumVertices;

		md.vertexLayout =vl;

		// We can create&link a skeleton but not fill it.
		// Process&link material here
		Chip *material = _processMaterial(md, scene);
		renderable->SetChild(material, 2, (uint32)i);
	}

	renderable->UpdateToGeometry();
	renderable->CalculateBoundingBox();

	return renderable;
}

void OpenAssetImpLib::_processNode(MeshMap &meshes, BoneMap &bones, Set<aiNode*> &riggedNodes, ProcessedNode &p, const aiScene *scene)
{
	if (p.node->mNumMeshes) {
		Map<InputLayout, MeshList> groupedByInputLayout;
		for (uint32 i = 0; i < p.node->mNumMeshes; i++) { // Iterate meshes in node
			aiMesh *mesh = scene->mMeshes[p.node->mMeshes[i]];
			if (_meshRefCount[p.node->mMeshes[i]] > 1) { // If more than one node is referring a mesh, we can't combine the mesh with other meshes. Therefore, process it separately.
				auto n = meshes.find(mesh);
				if (n == meshes.end()) {
					MeshList meshList;
					meshList.push_back(mesh);
					Renderable *renderable = _processMeshList(meshList, bones, scene);
					n = meshes.insert(std::make_pair(mesh, renderable)).first; // Add the mesh to a map, so we can resue it by other nodes!
				}
				if (n->second)
					p.renderables.push_back(n->second);
			}
			else { // Group meshes bases on input layout so that they can be combinded in a single Geometry/Renderable. (Remember, each mesh can have a different material!)
				InputLayout inputLayout = 0;
				inputLayout += mesh->HasPositions() ? 0x1 : 0; // bit0
				inputLayout += mesh->HasNormals() ? 0x2 : 0; // bit1
				inputLayout += mesh->GetNumColorChannels() << 2; // bit2-4 (AI_MAX_NUMBER_OF_COLOR_SETS is 8 -> 3 bits)
				// ch0: [5-6], ch1: [7-8] ... ch7[19-20] (AI_MAX_NUMBER_OF_TEXTURECOORDS is 8, therefore ch0-7. 2 bits pr channel because we can have 1,2 or 3 (U, UV, UVW) texcoord components.)
				for (uint32 p = 0; p < mesh->GetNumUVChannels(); p++) 
					inputLayout += ((_postProcessFlags & EXTRAFLAG_force2CompTexcoords) ? 2 : mesh->mNumUVComponents[p]) << (5 + p * 2);
				//inputLayout += mesh->GetNumUVChips() << 5; // bit5-7
				inputLayout += mesh->HasTangentsAndBitangents() ? 0x200000 : 0; // bit21
				inputLayout += mesh->HasBones() ? 0x400000 : 0; // bit22
				// Note: We can't combine meshes of different primitive types.
				inputLayout += mesh->mPrimitiveTypes << 23; // bit 23-26
				groupedByInputLayout.insert(std::make_pair(inputLayout, MeshList())).first->second.push_back(mesh);
			}
			if (mesh->HasBones()) // Mesh has bones? 
				riggedNodes.insert(p.node); // Mark current node as "rigged" to help us find root node in bone hierarchy.
		}

		// Process all grouped meshes into one single Geometry/Renderable.
		for (const auto &n : groupedByInputLayout) {
			Renderable *renderable = _processMeshList(n.second, bones, scene);
			if (renderable)
				p.renderables.push_back(renderable);
		}
	}

	// Process children!
	for (uint32 i = 0; i < p.node->mNumChildren; i++) {
		ProcessedNode n = { p.node->mChildren[i] };
		_processNode(meshes, bones, riggedNodes, n, scene);
		if (n.renderables.size() || n.children.size())
			p.children.push_back(std::move(n)); // Only include children which contains meshes, directly or further down the hierarchy.
	}
}

_3DObject *OpenAssetImpLib::_processProcessedNode(const ProcessedNode &node)
{
	List<_3DObject*> children;

	for (size_t i = 0; i < node.children.size(); i++) {
		_3DObject *child = _processProcessedNode(node.children[i]);
		if (child)
			children.push_back(child);
	}

	if (node.renderables.size()) { // insert renderables...
		for (size_t i = 0; i < node.renderables.size(); i++) {
			children.push_back(node.renderables[i]);
		}
	}

	if (children.empty())
		return nullptr; // Nothing to do!

	_3DObject *object = _addChip<_3DObject>(_3DOBJECT_GUID);
	if (!object) {
		msg(WARN, MTEXT("Failed to create 3DObject."));
		return nullptr;
	}

	if (node.node->mName.length)
		object->SetName(node.node->mName.C_Str());

	MatrixChip *transformChip = nullptr;
	XMFLOAT4X4 transform = toXMMatrix(node.node->mTransformation);
	XMVECTOR translate, rotate, scale;
	if (XMMatrixDecompose(&scale, &rotate, &translate, XMLoadFloat4x4(&transform))) {
		XMFLOAT4 f0, f1, f2;
		XMStoreFloat4(&f0, translate);
		XMStoreFloat4(&f1, rotate);
		XMStoreFloat4(&f2, scale);
		f0.w = f2.w = 0.0f;
		Motion *motion = _addChip<Motion>(MOTION_GUID);
		if (node.node->mName.length)
			motion->SetName(String(node.node->mName.C_Str()) + MTEXT(" Transform"));
		VectorChip *v0 = _addChip<VectorChip>(VECTORCHIP_GUID);
		VectorChip *v1 = _addChip<VectorChip>(VECTORCHIP_GUID);
		VectorChip *v2 = _addChip<VectorChip>(VECTORCHIP_GUID);
		motion->SetChild(v0, 0, 0);
		motion->SetChild(v1, 1, 0);
		motion->SetChild(v2, 2, 0);
		motion->SetRoationQuaternion(true);
		v0->SetName(MTEXT("Translation"));
		v1->SetName(MTEXT("Rotation"));
		v2->SetName(MTEXT("Scaling"));
		v0->SetVector(f0);
		v1->SetVector(f1);
		v2->SetVector(f2);
		//motion->SetChild(parentTransform, 3, 0);
		transformChip = motion;
	}
	else {
		transformChip = _addChip<MatrixChip>(MATRIXCHIP_GUID);
		if (node.node->mName.length)
			transformChip->SetName(String(node.node->mName.C_Str()) + MTEXT(" Transform"));
		transformChip->SetMatrix(transform);
		/*if (parentTransform) { 
			MatrixOperator *mo = (MatrixOperator*)_importedCG->AddChip(engine->GetChipManager()->GetChipTypeIndex(MATRIXOPERATOR_GUID));
			mo->SetOperatorType(MatrixOperator::OT_MULTIPLY);
			mo->SetChild(parentTransform, 0, 0);
			mo->SetChild(transformChip, 1, 0);
			transformChip = mo;
		}*/
	}

	object->SetChild(transformChip, 0, 0);

	for (size_t i = 0; i < children.size(); i++) {
		object->SetChild(children[i], 2, (uint32)i);
	}

	return object;
}

void OpenAssetImpLib::_buildSkeleton(Skeleton *skeleton, Skeleton::Joint &joint, aiNode *node, const Map<aiNode*, bool> &nodes, const BoneMap &skeletonNodes)
{
	auto k = skeletonNodes.find(node);

	// Add this bone to the skeleton
	if (k == skeletonNodes.end()) {
		joint.index = -1;
		XMStoreFloat4x4(&joint.inverseBindPose, XMMatrixIdentity());
	}
	else {
		joint.index = k->second.first;
		joint.inverseBindPose = k->second.second;
	}
	XMFLOAT4X4 t = toXMMatrix(node->mTransformation);
	XMVECTOR a, b, c;
	if (XMMatrixDecompose(&a, &b, &c, XMLoadFloat4x4(&t))) {
		XMStoreFloat3(&joint.jointTransform.position, c);
		XMStoreFloat4(&joint.jointTransform.rotation, b);
		XMStoreFloat3(&joint.jointTransform.scaling, a);
	}
	else {
		joint.jointTransform.position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		joint.jointTransform.rotation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		joint.jointTransform.scaling = XMFLOAT3(1.0f, 1.0f, 1.0f);
		msg(WARN, MTEXT("Decompose failed.."));
	}
	joint.name = node->mName.C_Str();

	for (uint32 j = 0; j < node->mNumChildren; j++) {
		auto n = nodes.find(node->mChildren[j]);
		if (n == nodes.end())
			continue;
		joint.children.push_back(Skeleton::Joint());
		_buildSkeleton(skeleton, joint.children.back(), node->mChildren[j], nodes, skeletonNodes);		
	}
}

void _countBones(const aiScene *scene, aiNode *n, Set<aiNode*> &bones)
{
	for (uint32 i = 0; i < n->mNumMeshes; i++) { // Iterate meshes in node
		aiMesh *mesh = scene->mMeshes[n->mMeshes[i]];
		if (mesh->HasBones()) {
			for (uint32 j = 0; j < mesh->mNumBones; j++) { // Iterate bones
				aiBone *bone = mesh->mBones[j];
				aiNode *boneNode = scene->mRootNode->FindNode(bone->mName);
				if (boneNode)
					bones.insert(boneNode);
			}
		}
	}
	for (uint32 i = 0; i < n->mNumChildren; i++)
		_countBones(scene, n->mChildren[i], bones);
}

// Called from Import()
Chip *OpenAssetImpLib::_processScene(const aiScene *scene)
{
	Map<InputLayout, StdGeometry*> groupedMeshes;
	MeshMap meshes;
	BoneMap bones;
	Set<aiNode*> riggedNodes;

	// Find the number of nodes refering each mesh and store the result in _meshRefCount.
	_meshRefCount.resize(scene->mNumMeshes);
	List<aiNode*> stack;
	stack.push_back(scene->mRootNode);
	while (stack.size()) {
		aiNode *n = stack.back();
		stack.pop_back();
		for (uint32 i = 0; i < n->mNumMeshes; i++)
			_meshRefCount[n->mMeshes[i]]++;
		for (uint32 i = 0; i < n->mNumChildren; i++)
			stack.push_back(n->mChildren[i]);
	}

	List<_3DObject*> objects;

	if (_postProcessFlags & aiProcess_PreTransformVertices) { // Do we use the aiProcess_PreTransformVertices flag?
		// All transforms should be identity using this flag and all meshes are in nodes directly below the root.
		// Collect all meshes into a new node, and process this node instead of root as we usually do!
		aiNode n;
		n.mName = scene->mRootNode->mName;
		List<uint32> m;
		if (scene->mRootNode->mNumMeshes) { // I've found sometimes, the meshes are in the rootnode, even if aiProcess_PreTransformVertices is specified.
			for (uint32 j = 0; j < scene->mRootNode->mNumMeshes; j++)
				m.push_back(scene->mRootNode->mMeshes[j]);
		}
		else {
			for (uint32 i = 0; i < scene->mRootNode->mNumChildren; i++)
				for (uint32 j = 0; j < scene->mRootNode->mChildren[i]->mNumMeshes; j++)
					m.push_back(scene->mRootNode->mChildren[i]->mMeshes[j]);
		}

		if (m.size()) {
			n.mNumMeshes = (uint32)m.size();
			n.mMeshes = new uint32[n.mNumMeshes];
			std::memcpy(n.mMeshes, &m.front(), sizeof(uint32)*n.mNumMeshes);
		}

		ProcessedNode node = { &n };

		Set<aiNode*> b;
		_countBones(scene, node.node, b);
		_numBones = (uint32)b.size();

		// Get list of geometries!
		_processNode(meshes, bones, riggedNodes, node, scene);

		// Build the only 3DObject that should exist!
		_3DObject *obj = _processProcessedNode(node);
		if (obj)
			objects.push_back(obj);
	}
	else {
		ProcessedNode node = { scene->mRootNode };

		Set<aiNode*> b;
		_countBones(scene, node.node, b);
		_numBones = (uint32)b.size();

		// Get hierarchy of Geometries.
		_processNode(meshes, bones, riggedNodes, node, scene);

		// Build 3DObject(s)
		_3DObject *obj = _processProcessedNode(node);
		if (obj)
			objects.push_back(obj);
	}

	if (!bones.empty()) { // Do we have bones to process?

		// Find the root node of the skeleton! Normally this is a child or sibling of the bone-node highest in the hierarchy.
		Set<aiNode*> tmp;
		for (const auto &n : riggedNodes)
			for (aiNode *m = n; m != nullptr; m = m->mParent)
				tmp.insert(m);

		aiNode *skeletonRoot = nullptr;
		Map<aiNode*, bool> boneNodes;

		// Iterate bones
		for (const auto &b : bones) {
			boneNodes.insert(std::make_pair(b.first, true)).first->second = true;
			for (aiNode *n = b.first; n; n = n->mParent)
				boneNodes.insert(std::make_pair(n, false));
		}

		skeletonRoot = scene->mRootNode;

		while (true) {
			if (boneNodes.find(skeletonRoot)->second) // bone?
				break;

			aiNode *n = nullptr;
			for (uint32 j = 0; j < skeletonRoot->mNumChildren; j++) {
				if (boneNodes.find(skeletonRoot->mChildren[j]) != boneNodes.end()) {
					if (n != nullptr) {
						n = nullptr;
						break;
					}
					else
						n = skeletonRoot->mChildren[j];
				}
			}

			if (n == nullptr)
				break;

			if (tmp.find(skeletonRoot) == tmp.end())
				break;

			skeletonRoot = n;
		}

		// Note: If I am to add option to remove all transforms, I should transform all non-rigged meshes.
		// The rigged meshes should be left untouched, and instead I should add a dummy root node to the
		// skeleton with the final transform (world-transform) of the mesh.

		// Now, build the skeleton. The Skeleton-chip is already created! (because it needs to be linked to materials)
		_buildSkeleton(_sg.skeleton, _sg.skeleton->GetRootJoint(), skeletonRoot, boneNodes, bones);
	}

	if (objects.empty())
		return nullptr; // Nothing to return.
//	if (objects.size() == 1)
//		return objects.front(); // Only one 3DObject found!

	uint32 cc = 0;
	// More than one 3DObject found. Link them all to a ChipCaller!
	Caller *ret = _addChip<Caller>(CALLER_GUID);
	if (!ret)
		return nullptr; // Shouldn't happend!

	if (_sg.skeletonController) {
		ret->SetChild(_sg.skeletonController, 0, cc++);
	}

	ret->SetName(MTEXT("Import"));
	Function *func = ret->CreateFunction();
	if (func)
		func->Set(MTEXT("Import"), Function::Type::Static, Function::Access::Public, ParameterConnectionSet());

	for (size_t i = 0; i < objects.size(); i++)
		ret->SetChild(objects[i], 0, (uint32)i + cc++);

	return ret;
}


Class *OpenAssetImpLib::Import(Chip **mainChip)
{
	class LogStream : public Assimp::LogStream
	{
	public:
		MessageSeverity severity;
		LogStream(MessageSeverity severity) : severity(severity) {}

		void write(const char* message)
		{
			String mess = message;
			strUtils::replaceChar(mess, L'\n', L' ');
			msg(severity, mess); 
		}
	};

	struct Logger
	{
		Logger() { Assimp::DefaultLogger::create(""); }
		~Logger() { Assimp::DefaultLogger::kill(); }

		Assimp::Logger *get() { return Assimp::DefaultLogger::get(); }
	} logger;

	// Logger takes ownership of LogStream!
	logger.get()->attachStream(new LogStream(DINFO), Assimp::Logger::Debugging);
	logger.get()->attachStream(new LogStream(INFO), Assimp::Logger::Info);
	logger.get()->attachStream(new LogStream(WARN), Assimp::Logger::Warn);
	logger.get()->attachStream(new LogStream(FATAL), Assimp::Logger::Err);

	// Create an instance of the Importer class
	Assimp::Importer importer;

	uint32 flags = aiProcess_ConvertToLeftHanded | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_LimitBoneWeights | aiProcess_RemoveComponent;
	
	flags |= _postProcessFlags;

	flags &= ~EXTRAFLAG_mask;

	// TODO: if OptimizeGraph we should multiply the only transform with all verticies and normals!
	// DONE: If PretransformVerticies, we should combine all nodes into one!

	// *****************************************************************************************************************
	// NOTE: I've changed aiComponent_COLORSn from 20 to 16 and and aiComponent_TEXCOORDSn from 25 to 24 in assimp config.h.
	// This is because both AI_MAX_NUMBER_OF_COLOR_SETS and AI_MAX_NUMBER_OF_TEXTURECOORDS are 8. 
	// Without those changes, removing components will collide.
	// *****************************************************************************************************************

	uint32 removeComps = _removeComps;
	// Remove unsupported texcomps and colors.
	removeComps |= aiComponent_TEXCOORDSn(4) | aiComponent_TEXCOORDSn(5) | aiComponent_TEXCOORDSn(6) | aiComponent_TEXCOORDSn(7);
	removeComps |= aiComponent_COLORSn(1) | aiComponent_COLORSn(2) | aiComponent_COLORSn(3) | aiComponent_COLORSn(4);
	removeComps |= aiComponent_COLORSn(5) | aiComponent_COLORSn(6) | aiComponent_COLORSn(7);
	removeComps |= aiComponent_LIGHTS | aiComponent_CAMERAS; // No need to import lights and cameras!
	
	if (removeComps) 
		importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, removeComps);

/*
	aiComponent_ANIMATIONS
	aiComponent_TEXTURES
	aiComponent_MESHES
	aiComponent_MATERIALS
*/

	const aiScene* scene = importer.ReadFile( _filename.AsString().c_str(), flags);
	
	// If the import failed, report it
	if( !scene) {
		msg(WARN, importer.GetErrorString());
		return nullptr;
	}

	// Create the class
	Document *doc = engine->GetDocumentManager()->CreateDocument();
	if (!doc) {
		msg(WARN, MTEXT("Import failed"));
		return nullptr;
	}
	_importedCG = engine->GetClassManager()->CreateClass(_filename.GetFileNameWithoutExtention(), doc);
	if (!_importedCG) {
		msg(WARN, MTEXT("Import failed"));
		return nullptr; // TODO: what about doc?
	}

	// Process the scene!
	Chip *rootChip = _processScene(scene);

	// Now, process *skeletal* animations if any!
	if (scene->HasAnimations()) {
		
		// If we did not process any skeleton yet, create one for the animations... (for importing animations that can be pasted to other models)
		if (!_sg.skeleton) {
			_sg.skeleton = _addChip<Skeleton>(SKELETON_GUID);
		}
		if (_sg.skeleton)
		{
			for (uint32 i = 0; i < scene->mNumAnimations; i++) { // Iterate animations
				aiAnimation* animation = scene->mAnimations[i];

				String name = strUtils::trim(animation->mName.C_Str()), ext = MTEXT("");
				if (name.empty())
					name = MTEXT("Unnamed");

				Skeleton::Animation* a = nullptr;

				for (uint32 i = 1; a == nullptr; i++) { // Animations need to have a unique name!
					if (i > 1)
						ext = MTEXT(" ") + strUtils::fromNum(i);
					a = _sg.skeleton->AddAnimation(name + ext); // Addanimation will return nullptr if name exist!
				}

				name += ext;

				for (uint32 j = 0; j < animation->mNumChannels; j++) { // Iterate all animation chips (ie animated nodes)
					aiNodeAnim* na = animation->mChannels[j];
					aiNode* node = scene->mRootNode->FindNode(na->mNodeName);

					XMFLOAT3 dt(0.0f, 0.0f, 0.0f), ds(1.0f, 1.0f, 1.0f);
					XMFLOAT4 dr(0.0f, 0.0f, 0.0f, 1.0f);
					XMVECTOR t1, t2, t3;
					XMFLOAT4X4 m = toXMMatrix(node->mTransformation);
					if (XMMatrixDecompose(&t3, &t2, &t1, XMLoadFloat4x4(&m))) {
						XMStoreFloat3(&dt, t1);
						XMStoreFloat4(&dr, t2);
						XMStoreFloat3(&ds, t3);
					}

					Skeleton::KeyframeList& keyframes = a->keyframes.insert(std::make_pair(na->mNodeName.C_Str(), Skeleton::KeyframeList())).first->second;

					Set<float64> keys;
					for (uint32 k = 0; k < na->mNumPositionKeys; k++)
						keys.insert(na->mPositionKeys[k].mTime);
					for (uint32 k = 0; k < na->mNumRotationKeys; k++)
						keys.insert(na->mRotationKeys[k].mTime);
					for (uint32 k = 0; k < na->mNumScalingKeys; k++)
						keys.insert(na->mScalingKeys[k].mTime);

					uint32 ka = 0, kb = 0, kc = 0;
					for (const auto &n : keys) {
						Skeleton::Keyframe keyframe;
						keyframe.time = (float32)n;
						if (na->mNumPositionKeys) {
							if (ka < na->mNumPositionKeys && na->mPositionKeys[ka].mTime < n)
								ka++;
							if (ka == 0)
								keyframe.position = (XMFLOAT3&)na->mPositionKeys[0].mValue;
							else if (ka == na->mNumPositionKeys)
								keyframe.position = (XMFLOAT3&)na->mPositionKeys[na->mNumPositionKeys - 1].mValue;
							else if (na->mPositionKeys[ka].mTime == n)
								keyframe.position = (XMFLOAT3&)na->mPositionKeys[ka].mValue;
							else
								XMStoreFloat3(&keyframe.position, XMVectorLerp(XMLoadFloat3((XMFLOAT3*)&na->mPositionKeys[ka - 1].mValue), XMLoadFloat3((XMFLOAT3*)&na->mPositionKeys[ka].mValue), float32((n - na->mPositionKeys[ka - 1].mTime) / (na->mPositionKeys[ka].mTime / na->mPositionKeys[ka - 1].mTime))));
						}
						else
							keyframe.position = dt;
						if (na->mNumRotationKeys) {
							if (kb < na->mNumRotationKeys && na->mRotationKeys[kb].mTime < n)
								kb++;
							if (kb == 0)
								keyframe.rotation = (XMFLOAT4&)na->mRotationKeys[0].mValue;
							else if (kb == na->mNumRotationKeys)
								keyframe.rotation = (XMFLOAT4&)na->mRotationKeys[na->mNumRotationKeys - 1].mValue;
							else if (na->mRotationKeys[kb].mTime == n)
								keyframe.rotation = (XMFLOAT4&)na->mRotationKeys[kb].mValue;
							else
								XMStoreFloat4(&keyframe.rotation, XMQuaternionSlerp(XMLoadFloat4((XMFLOAT4*)&na->mRotationKeys[kb - 1].mValue), XMLoadFloat4((XMFLOAT4*)&na->mRotationKeys[kb].mValue), float32((n - na->mRotationKeys[kb - 1].mTime) / (na->mRotationKeys[kb].mTime / na->mRotationKeys[kb - 1].mTime))));
							XMFLOAT4 tmp = keyframe.rotation;
							keyframe.rotation = XMFLOAT4(tmp.y, tmp.z, tmp.w, tmp.x);
						}
						else
							keyframe.rotation = dr;
						if (na->mNumScalingKeys) {
							if (kc < na->mNumScalingKeys && na->mScalingKeys[kc].mTime < n)
								kc++;
							if (kc == 0)
								keyframe.scaling = (XMFLOAT3&)na->mScalingKeys[0].mValue;
							else if (kc == na->mNumScalingKeys)
								keyframe.scaling = (XMFLOAT3&)na->mScalingKeys[na->mNumScalingKeys - 1].mValue;
							else if (na->mScalingKeys[kc].mTime == n)
								keyframe.scaling = (XMFLOAT3&)na->mScalingKeys[kc].mValue;
							else
								XMStoreFloat3(&keyframe.scaling, XMVectorLerp(XMLoadFloat3((XMFLOAT3*)&na->mScalingKeys[kc - 1].mValue), XMLoadFloat3((XMFLOAT3*)&na->mScalingKeys[kc].mValue), float32((n - na->mScalingKeys[kc - 1].mTime) / (na->mScalingKeys[kc].mTime / na->mScalingKeys[kc - 1].mTime))));
						}
						else
							keyframe.scaling = ds;
						keyframes.push_back(keyframe);
					}
				}
				_sg.skeleton->CalculateAnimationDuration(a);
			}
		}
		else
		{
			msg(WARN, MTEXT("This import contains animations, but no skeleton. Only skeletal animations are currently supported!"));
		}
	}
	
	if (mainChip)
		*mainChip = rootChip;

	if (scene->mMetaData) {
		msg(INFO, MTEXT("---- Scene Metadata ----"));
		for (uint32 i = 0; i < scene->mMetaData->mNumProperties; i++) {
			aiString str = scene->mMetaData->mKeys[i];
			String key = str.C_Str(), value;
			aiMetadataEntry data = scene->mMetaData->mValues[i];
			if (!data.mData)
				continue;
			switch (data.mType)
			{
			case AI_BOOL: value = *((bool*)data.mData) ? MTEXT("true") : MTEXT("false"); break;
			case AI_INT32: value = strUtils::fromNum(*((int32*)data.mData)); break;
			case AI_UINT64: value = strUtils::fromNum(*((uint64*)data.mData)); break;
			case AI_FLOAT: value = strUtils::fromNum(*((float32*)data.mData)); break;
			case AI_DOUBLE: value = strUtils::fromNum(*((float64*)data.mData)); break;
			case AI_AISTRING: value = ((aiString*)data.mData)->C_Str(); break;
			case AI_AIVECTOR3D: value = strUtils::ConstructString(MTEXT("(%1, %2, %3)")).arg(((aiVector3D*)data.mData)->x).arg(((aiVector3D*)data.mData)->y).arg(((aiVector3D*)data.mData)->z); break;
			default: continue;
			}
			msg(INFO, key + MTEXT(": ") + value);
		}
		msg(INFO, MTEXT("------------------------"));
	}

	msg(INFO, MTEXT("Import SUCCEEDED!"));

	return _importedCG;
}

