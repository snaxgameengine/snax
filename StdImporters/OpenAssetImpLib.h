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
#include "StdChips/Importer.h"
#include "GraphicsChips/Texture.h"
#include "GraphicsChips/Skeleton.h"
#include "GraphicsChips/GraphicsDefines.h"
#include "M3DEngine/Class.h"
#include "M3DEngine/Engine.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <d3d12.h> // TEMPORARY!!


namespace m3d
{

	static bool operator<(const M3D_SAMPLER_DESC& l, const M3D_SAMPLER_DESC& r)
	{
		return std::memcmp(&l, &r, sizeof(M3D_SAMPLER_DESC)) < 0;
	}

class Texture;
class Material;
class Renderable;
class StdGeometry;
class _3DObject;
class Skeleton;
class SkeletonController;
class MatrixArray;
class Sampler;
class GraphicsBuffer;
class GraphicsState;
class RootSignature;
class Shaders;
class Shader;

static const Guid OPENASSETIMPLIB_GUID = { 0x58900f19, 0x99fb, 0x4876, { 0xaf, 0xa9, 0x70, 0xbd, 0x3f, 0x85, 0x3b, 0x2b } };

#define EXTRAFLAG_mask 0xF0000000
#define EXTRAFLAG_force2CompTexcoords 0x10000000

#define DEFAULT_WRAPPED_SAMPLER (uint32)-1
#define DEFAULT_SAMPLER (uint32)-2


class STDIMPORTERS_API OpenAssetImpLib : public Importer
{
	CHIPDESC_DECL;
public:
	OpenAssetImpLib();
	virtual ~OpenAssetImpLib();

	virtual Class *Import(Chip **mainChip = nullptr);

	virtual uint32 GetPostProcessFlags() const { return _postProcessFlags; }
	virtual void SetPostProcessFlags(uint32 flags) { _postProcessFlags = flags; }
	virtual uint32 GetRemoveCompsFlag() const { return _removeComps; }
	virtual void SetRemoveCompsFlag(uint32 flags) { _removeComps = flags; }

protected:
	struct TexDesc
	{
		const aiTexture* tex; // non-null for embedded textures...
		Path path;
		aiTextureMapping mapping;
		uint32 uvindex;
		float32 blend;
		aiTextureOp op;
		aiTextureMapMode mapMode[2];
		TexDesc() : mapping(aiTextureMapping_UV), uvindex(0), blend(0.0f), op(aiTextureOp_Multiply) { mapMode[0] = mapMode[1] = aiTextureMapMode_Wrap; }
	};

	struct ProcessedNode
	{
		aiNode *node;
		List<Renderable*> renderables;
		List<ProcessedNode> children;
	};

	struct VertexLayout
	{
		bool hasPositions;
		bool hasNormals;
		bool hasColors;
		bool hasTangent;
		bool hasBitangents;
		Array<uint32, 4> hasTexCoords;
		bool hasBlendWeigtsAndIndices;

		auto tied() const { return std::tie(hasPositions, hasNormals, hasColors, hasTangent, hasBitangents, hasTexCoords, hasBlendWeigtsAndIndices); }

		bool operator==(const VertexLayout &vl) const
		{
			return tied() == vl.tied();
		}

		std::size_t hash() const
		{
			return hash_combine_simple<Hash>(tied());
		}
	};

	struct VertexShaderDesc
	{
		VertexLayout vl;
		uint32 skeletonBufferIdx;

		auto tied() const { return std::tie(vl, skeletonBufferIdx); }

		bool operator==(const VertexShaderDesc &rhs) const
		{
			return tied() == rhs.tied();
		}

		std::size_t hash() const
		{
			return hash_combine_simple<Hash>(tied());
		}
	};

	struct SkeletonGroup;

	struct MaterialDesc
	{
		const aiMaterial *material;
		Array<uint32, AI_MAX_NUMBER_OF_TEXTURECOORDS> tcMap;
		SkeletonGroup *sg;
		VertexLayout vertexLayout;
		
		auto tied() const { return std::tie(material, tcMap, sg); } // DO NOT INCLUDE vertexLayout!

		bool operator<(const MaterialDesc& rhs) const
		{
			return tied() < rhs.tied();
		}
	};

	struct TexStage
	{
		aiTextureType type;
		uint32 coordset;
		uint32 sampleridx;
		aiTextureOp blendOp;
		aiTextureMapping mapping;
		float32 blendfactor;

		auto tied() const { return std::tie(type, coordset, sampleridx, blendOp, mapping, blendfactor); }

		bool operator==(const TexStage &ts) const
		{
			return tied() == ts.tied();
		}

		std::size_t hash() const
		{
			return hash_combine_simple<Hash>(tied());
		}
	};

	struct ShaderDesc
	{
		VertexShaderDesc vsd;
		aiShadingMode shadingMode;
		List<TexStage> texStages;

		auto tied() const { return std::tie(vsd, shadingMode, texStages); }

		bool operator==(const ShaderDesc &ts) const
		{
			return tied() == ts.tied();
		}

		std::size_t hash() const
		{
			return hash_combine_simple<Hash>(tied());
		}
	};

	struct StateDesc
	{
		int32 wireframe;
		int32 twosided;
		aiBlendMode blendMode;
		ShaderDesc shaderDesc;

		auto tied() const { return std::tie(wireframe, twosided, blendMode, shaderDesc); }

		bool operator==(const StateDesc &sd) const
		{
			return tied() == sd.tied(); 
		}

		std::size_t hash() const
		{
			return hash_combine_simple<Hash>(tied());
		}
	};

	typedef Map<M3D_SAMPLER_DESC, Sampler*> SamplerMap;
	SamplerMap _samplers;

	typedef List<aiMesh*> MeshList;
	typedef Map<aiMesh*, Renderable*> MeshMap;
	typedef Map<aiNode*, std::pair<uint16, XMFLOAT4X4>> BoneMap;
	typedef uint32 InputLayout;
	typedef Map<MaterialDesc, Material*> MaterialMap;
	typedef UnorderedMap<StateDesc, GraphicsState*, Hash<StateDesc>> StateMap;
	typedef UnorderedMap<ShaderDesc, Shaders*, Hash<ShaderDesc>> ShadersMap;
	typedef UnorderedMap<VertexShaderDesc, Shader*, Hash<VertexShaderDesc>> VertexShaderMap;

	Map<Path, Texture*> _textureMap;
	Class *_importedCG;
	MaterialMap _materialMap;
	List<uint32> _meshRefCount;
	StateMap _stateMap;
	ShadersMap _shadersMap;
	VertexShaderMap _vsMap;

	uint32 _numBones;

	// Flags corresponding to aiProcess_*
	uint32 _postProcessFlags;
	// Components to remove during load. (aiComponent enum)
	uint32 _removeComps;
	//
	bool _useShortcuts;

	bool _skeletonInit;
	bool _bonesBufferConnected;
	struct SkeletonGroup
	{
		Skeleton *skeleton;
		SkeletonController *skeletonController;
		MatrixArray *skeletonArray;
		GraphicsBuffer *bonesBuffer;
	};
	SkeletonGroup _sg;

	RootSignature *_rootSignature;
	GraphicsBuffer* _globalCBV;

	void _readTextures(const aiScene* scene, const aiMaterial *material, aiTextureType type, List<TexDesc> &textures);
	Chip *_loadTexture(const TexDesc &td, const Array<uint32, AI_MAX_NUMBER_OF_TEXTURECOORDS>& tcMap, M3D_SAMPLER_DESC&sampler, uint32 &cSet);
	Chip *_processMaterial(const MaterialDesc &md, const aiScene* scene);
	Renderable *_processMeshList(const MeshList &meshes, BoneMap &bones, const aiScene *scene);
	void _buildSkeleton(Skeleton *skeleton, Skeleton::Joint &joint, aiNode *node, const Map<aiNode*, bool> &nodes, const BoneMap &skeletonNodes);
	void _processNode(MeshMap &meshes, BoneMap &bones, Set<aiNode*> &riggedNodes, ProcessedNode &p, const aiScene *scene);
	_3DObject *_processProcessedNode(const ProcessedNode &node);
	Chip *_processScene(const aiScene *scene);
	void _processTextures(const List<TexDesc> &textures, aiTextureType type, uint32 &counter, const Array<uint32, AI_MAX_NUMBER_OF_TEXTURECOORDS> &tcMap, Material *material, List<TexStage> &ts);
	void _genVS(Shader *s, const ShaderDesc &sd);
	void _genPS(Shader *s, const ShaderDesc &sd);
	Chip *_getVertexShader(const ShaderDesc &sd);
	Chip *_getShaders(const ShaderDesc &sd);
	Chip *_getGraphicsState(const StateDesc &sd);

	template<typename T>
	T *_addChip(Guid guid)
	{
		return dynamic_cast<T*>(_importedCG->AddChip(engine->GetChipManager()->GetChipTypeIndex(guid)));
	}

};



}