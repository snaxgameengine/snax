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
#include "GraphicsChips/Shader.h"

using namespace m3d;


void OpenAssetImpLib::_genVS(Shader *vs, const ShaderDesc &sd)
{
	String src;

	const VertexLayout &vl = sd.vsd.vl;

	const String COMP_STR[5] = { "", "   x", "  xy", " xyz", "xyzw" };
	const String FLOAT_TYPE_STR[5]{ "", "float ", "float2", "float3", "float4" };

	src += "//\n";
	src += "//\n";
	src += "// Open Asset Import Library for SnaX\n";
	src += "//\n";
	src += "// VERTEX SHADER (Shader Model 5.1)\n";
	src += "//\n";
	src += "//\n\n\n";
	src += "// Include common shader defines.\n";
	src += "#include <Std>\n";
	src += "\n";
	if (sd.vsd.skeletonBufferIdx != -1) {
		src += "// This is the tbuffer containing the skinning matrices.\n";
		src += strUtils::ConstructString("tbuffer bonesbuffer : TEXTURE%1\n").arg(sd.vsd.skeletonBufferIdx + 1);
		src += "{\n";
		src += strUtils::ConstructString("    float4x4 bones[%1];\n").arg(_numBones);
		src += "}\n";
		src += "\n";
	}
	src += "// This is the input to the vertex shader. The Geometry connected to your Renderable must supply all these elements!\n";
	src += "struct ShaderInput\n";
	src += "{\n";
	if (vl.hasPositions)	src += "    float3 position     : POSITION;\n";
	if (vl.hasNormals)		src += "    float3 normal       : NORMAL;\n";
	if (vl.hasTangent)		src += "    float3 tangent      : TANGENT;\n";
	if (vl.hasBitangents)	src += "    float3 bitangent    : BITANGENT;\n";
	for (uint32 i = 0; i < 4; i++)
		if (vl.hasTexCoords[i] > 0) src += strUtils::ConstructString("    %1 texcoord%2    : TEXCOORD%2;\n").arg(FLOAT_TYPE_STR[vl.hasTexCoords[i]]).arg(i);
	if (vl.hasColors)		src += "    float4 color        : COLOR;\n";
	if (vl.hasBlendWeigtsAndIndices) {
		src += "    float4 blendWeights : BLENDWEIGHTS;\n";
		src += "    uint4  blendIndices : BLENDINDICES;\n";
	}
	src += "};\n";
	src += "\n";
	src += "// This is the output from the vertex shader. If you modify this you must also update the ShaderInput for the pixel shader accordingly.\n";
	src += "struct ShaderOutput\n";
	src += "{\n";
	src += "    float4 position     : SV_POSITION;\n";
	src += "    float3 worldPos     : POSITION;\n";
	if (vl.hasNormals)		src += "    float3 normal       : NORMAL;\n";
	if (vl.hasTangent || vl.hasBitangents) {
		src += "    float3 tangent      : TANGENT;\n";
		src += "    float3 bitangent    : BITANGENT;\n";
	}
	for (uint32 i = 0; i < 4; i++)
		if (vl.hasTexCoords[i] > 0) src += strUtils::ConstructString("    %1 texcoord%2    : TEXCOORD%2;\n").arg(FLOAT_TYPE_STR[vl.hasTexCoords[i]]).arg(i);
	if (vl.hasColors)		src += "    float4 color        : COLOR;\n";
	src += "};\n";
	src += "\n";
	src += "// Entry point for the vertex shader.\n";
	src += "ShaderOutput main(in ShaderInput IN)\n";
	src += "{\n";
	src += "    ShaderOutput OUT = (ShaderOutput) 0;\n";
	if (sd.vsd.skeletonBufferIdx != -1 && vl.hasBlendWeigtsAndIndices) {
		src += "    float4 p = float4(0.0, 0.0, 0.0, 0.0);\n";
		src += "    float3 n = float3(0.0, 0.0, 0.0), t = float3(0.0, 0.0, 0.0), b = float3(0.0, 0.0, 0.0);\n";
		src += "    for (int i = 0; i < 4; i++) {\n";
		if (vl.hasPositions)	src += "        p +=  mul(float4(IN.position, 1.0), bones[IN.blendIndices[i]]) * IN.blendWeights[i];\n";
		if (vl.hasNormals)		src += "        n +=  mul(IN.normal, (float3x3)bones[IN.blendIndices[i]]) * IN.blendWeights[i];\n";
		if (vl.hasTangent)		src += "        t +=  mul(IN.tangent, (float3x3)bones[IN.blendIndices[i]]) * IN.blendWeights[i];\n";
		if (vl.hasBitangents)	src += "        b +=  mul(IN.bitangent, (float3x3)bones[IN.blendIndices[i]]) * IN.blendWeights[i];\n";
		src += "    }\n";
		if (vl.hasPositions)	src += "    OUT.position = mul(p, worldViewProjection);\n";
		if (vl.hasPositions)	src += "    OUT.worldPos = mul(p, world).xyz;\n";
		if (vl.hasNormals)		src += "    OUT.normal = mul(n, (float3x3)world);\n";
		if (vl.hasTangent)		src += "    OUT.tangent = mul(t, (float3x3)world);\n";
		if (vl.hasBitangents)	src += "    OUT.bitangent = mul(b, (float3x3)world);\n";
	}
	else {
		if (vl.hasPositions)	src += "    OUT.position = mul(float4(IN.position, 1.0), worldViewProjection);\n";
		if (vl.hasPositions)	src += "    OUT.worldPos = mul(float4(IN.position, 1.0), world).xyz;\n";
		if (vl.hasNormals)		src += "    OUT.normal = mul(IN.normal, (float3x3)world);\n";
		if (vl.hasTangent)		src += "    OUT.tangent = mul(IN.tangent, (float3x3)world);\n";
		if (vl.hasBitangents)	src += "    OUT.bitangent = mul(IN.bitangent, (float3x3)world);\n";
	}
	for (uint32 i = 0; i < 4; i++)
		if (vl.hasTexCoords[i] > 0) src += strUtils::ConstructString("    OUT.texcoord%1 = IN.texcoord%1;\n").arg(i);
	if (vl.hasColors) src += "    OUT.color = IN.color;\n";
	if (vl.hasTangent && !vl.hasBitangents)
		src += "    OUT.bitangent = cross(OUT.normal, OUT.tangent);\n";
	else if (!vl.hasTangent && vl.hasBitangents)	
		src += "    OUT.tangent = cross(OUT.normal, OUT.bitangent);\n";
	src += "    return OUT;\n";
	src += "}\n";

	vs->SetManualMode(true);
	vs->SetShaderModel(ShaderModel::SM51);
	vs->SetShaderType(ShaderType::VS);
	vs->SetShaderSource(src);
	vs->Compile();
}

void OpenAssetImpLib::_genPS(Shader *ps, const ShaderDesc &sd)
{
	String src;

	const String COMP_STR[5] = { "", "   x", "  xy", " xyz", "xyzw" };
	const String FLOAT_TYPE_STR[5]{ "", "float ", "float2", "float3", "float4" };

	static const String SHADING_MODELS[] = { "", "Flat", "Gouraud", "Phong", "Blinn", "Toon", "OrenNayar", "Minnaert", "CookTorrance", "NoShading", "Fresnel" };

	static const String TEX_TYPES[] = {
		"none        ",
		"DIFFUSE     ",
		"SPECULAR    ",
		"AMBIENT     ",
		"EMISSIVE    ",
		"HEIGHT      ",
		"NORMAL      ",
		"SHININESS   ",
		"OPACITY     ",
		"DISPLACEMENT",
		"LIGHTMAP    ",
		"REFLECTION  ",
		"UNKNOWN     " };

	static const String TEX_TYPES2[] = {
		"map",
		"diffuseMap",
		"specularMap",
		"ambientMap",
		"emissiveMap",
		"heightMap",
		"normalMap",
		"shininessMap",
		"opacityMap",
		"displacementMap",
		"lightmapMap",
		"reflectionMap",
		"unknownMap" };

	static const String TEX_OPS[] = { 
		"  MULTIPLY", 
		"       ADD", 
		"  SUBTRACT", 
		"    DIVIDE", 
		"SMOOTH_ADD", 
		"SIGNED_ADD" }; // aiTextureOp
	
	static const String TEX_MAPPINGS[] = { 
		"      UV", 
		"  SPHERE", 
		"CYLINDER", 
		"     BOX", 
		"   PLANE", 
		"   OTHER" }; // aiTextureMapping

	const VertexLayout &vl = sd.vsd.vl;

	src += "//\n";
	src += "//\n";
	src += "// Open Asset Import Library for SnaX\n";
	src += "//\n";
	src += "// PIXEL SHADER (Shader Model 5.1)\n";
	src += "//\n";
	src += "//\n";
	src += strUtils::ConstructString("// Shading Hint: %1\n").arg(SHADING_MODELS[sd.shadingMode]);
	src += "//\n";
	src += "// Textures:\n";
	src += "//\n";
	src += "// NBR FUNCTION     INDEX COORDSET SAMPLERIDX  OPERATION  BLEND  MAPPING\n";
	src += "// --- ------------ ----- -------- ---------- ---------- ------ --------\n";
	Map<aiTextureType, List<size_t>> texStack;
	Set<uint32> connectedSamplers;
	Map<uint32, String> samplerName;
	for (size_t i = 0; i < sd.texStages.size(); i++) {
		const TexStage &ts = sd.texStages[i];
		List<size_t> &s = texStack[ts.type];
		s.push_back(i);
		if (ts.sampleridx == DEFAULT_WRAPPED_SAMPLER)
			samplerName[ts.sampleridx] = "defaultWASampler";
		else if (ts.sampleridx == DEFAULT_SAMPLER)
			samplerName[ts.sampleridx] = "defaultASampler";
		else {
			connectedSamplers.insert(ts.sampleridx);
			samplerName[ts.sampleridx] = strUtils::ConstructString("sampler%1").arg(ts.sampleridx + 1);
		}

		src += strUtils::ConstructString("//   %1 %2     %3        %4          %5 %6 %7 %8\n").arg(i).arg(TEX_TYPES[ts.type]).arg(s.size() - 1).arg(ts.coordset).arg((ts.sampleridx == DEFAULT_WRAPPED_SAMPLER || ts.sampleridx == DEFAULT_SAMPLER) ? String("-") : strUtils::fromNum(ts.sampleridx)).arg(TEX_OPS[ts.blendOp]).arg(ts.blendfactor, "%6.1f").arg(TEX_MAPPINGS[ts.mapping]);
	}
	src += "//\n\n\n";
	src += "// Include common shader defines.\n";
	src += "#include <Std>\n";
	src += "\n";
	src += "// Here we define the layout of the material constant buffer.\n";
	src += "struct Material\n";
	src += "{\n";
	src += "    float shininess;\n";
	src += "    float shininess_strength;\n";
	src += "    float opacity;\n";
	src += "    float bump_scaling;\n";
//	src += "    float reflectivity;\n";
//	src += "    float refraction;\n";
	src += "    float4 ambient_color;\n";
	src += "    float4 diffuse_color;\n";
	src += "    float4 specular_color;\n";
	src += "    float4 emissive_color;\n";
//	src += "    float4 transparent_color;\n";
//	src += "    float4 reflective_color;\n";
	src += "};\n";
	src += "\n";
	src += "// Define the material constant buffer as the first buffer connected to the material.\n";
	src += "ConstantBuffer<Material> material : CBUFFER1;\n";
	src += "\n";
	if (sd.shadingMode != aiShadingMode_NoShading && vl.hasNormals) {
		src += "// Here we define the layout of the global constant buffer.\n";
		src += "struct Global\n";
		src += "{\n";
		src += "    float3 light_dir;\n";
		src += "    float3 light_ambient;\n";
		src += "    float3 light_diffuse;\n";
		src += "    float3 light_specular;\n";
		src += "};\n";
		src += "\n";
		src += "// Define the global constant buffer as the second buffer connected to the material.\n";
		src += "ConstantBuffer<Global> global : CBUFFER2;\n";
		src += "\n";
	}
	if (!sd.texStages.empty()) {
		src += "// These are the textures connected to your material.\n";
		Map<uint32, String> tex;
		for (const auto &n : texStack) {
			const List<size_t> &s = n.second;
			for (size_t i = 0; i < s.size(); i++) {
				size_t idx = s[i];
				const TexStage &ts = sd.texStages[idx];
				String str = strUtils::ConstructString("%1<%2> %3 : TEXTURE%4;\n").arg("Texture2D").arg(ts.type == aiTextureType_OPACITY ? "float4" : "float3").arg(TEX_TYPES2[n.first] + (s.size() > 1 ? strUtils::fromNum(i) : "")).arg(idx + 1);
				tex[(uint32)idx] = str;
			}
		}
		for (const auto &n : tex)
			src += n.second;
		src += "\n";
	}
	if (!connectedSamplers.empty()) {
		src += "// These are the sampler(s) connected to the material.\n";
		for (const auto &n : connectedSamplers) {
			src += strUtils::ConstructString("sampler sampler%1 : SAMPLER%1;\n").arg(n + 1);
		}
	}
	src += "\n";
	src += "// This is the output from the vertex shader. It must match (or be a strict subset of) the ShaderOutput struct in the vertex shader.\n";
	src += "struct ShaderInput\n";
	src += "{\n";
	src += "    float4 position     : SV_POSITION;\n";
	src += "    float3 worldPos     : POSITION;\n";
	if (vl.hasNormals)		src += "    float3 normal       : NORMAL;\n";
	if (vl.hasTangent)		src += "    float3 tangent      : TANGENT;\n";
	if (vl.hasBitangents)	src += "    float3 bitangent     : BITANGENT;\n";
	for (uint32 i = 0; i < 4; i++)
		if (vl.hasTexCoords[i] > 0) src += strUtils::ConstructString("    %1 texcoord%2    : TEXCOORD%2;\n").arg(FLOAT_TYPE_STR[vl.hasTexCoords[i]]).arg(i);
	if (vl.hasColors)		src += "    float4 color        : COLOR;\n";
	src += "    bool isFrontFace    : Sv_IsFrontFace;\n";
	src += "};\n";
	src += "\n";
	src += "// Entry point for the pixel shader. The shader returns a color for render target 0.\n";
	src += "float4 main(in ShaderInput IN) : SV_TARGET0\n";
	src += "{\n";
	src += "    float4 OUT = float4(0.0, 0.0, 0.0, 1.0);\n";
	auto texFunc = [&](aiTextureType type, String comp, String fmt = "float3", String cmp = "xyz")
	{
		const List<size_t> &texList  = texStack[type];
		for (uint32 i = 0; i < texList.size(); i++) {
			const TexStage &ts = sd.texStages[texList[i]];
			String texId = strUtils::ConstructString("%1%2").arg(TEX_TYPES2[type]).arg(texList.size() > 1 ? strUtils::fromNum(i) : "");
			if (ts.mapping != aiTextureMapping_UV) {
				src += strUtils::ConstructString("    // %1 does not have an UV-map defined. Skipping it. You should probably make sure you have \'Generate UV-coordinates\' checked in the import-dialog box.\n").arg(texId);
				continue;
			}
			String samplerId = samplerName[ts.sampleridx];
			String texCoord = strUtils::ConstructString("IN.texcoord%1").arg(ts.coordset);
			src += "    {\n";
			src += strUtils::ConstructString("        %5 tex = %1.Sample(%2, %3).%6%4;\n").arg(texId).arg(samplerId).arg(texCoord).arg(ts.blendfactor != 1.0f ? strUtils::ConstructString(" * %1").arg(ts.blendfactor).string : "").arg(fmt).arg(cmp);
			const char *BO[6] = { "%1 * %2", "%1 + %2", "%1 - %2", "%1 / %2", "(%1 + %2) - (%1 * %2)", "%1 + (%2 - 0.5)" };
			src += strUtils::ConstructString("        %1 = %2;\n").arg(comp).arg(strUtils::ConstructString(BO[ts.blendOp]).arg(comp).arg("tex"));
			src += "    }\n";
		}
	};
	src += "    float3 color = float3(0.0, 0.0, 0.0);\n";
	if (sd.shadingMode != aiShadingMode_NoShading && vl.hasNormals) {
		String vColor;
		if (vl.hasColors)
			vColor = " * IN.color.xyz";
		src += "    float3 n = normalize(IN.normal);\n";
		bool hasTBN = vl.hasBitangents || vl.hasTangent;
		bool hasNM = !texStack[aiTextureType_NORMALS].empty();
		bool hasH = !texStack[aiTextureType_HEIGHT].empty();
		if (hasTBN && (hasNM || hasH)) {
			src += "    float3 t = normalize(IN.tangent);\n";
			src += "    float3 b = normalize(IN.bitangent);\n";
			src += "    // t follows u in the uv-map. b follows v in the uv-map.\n";
			src += "    float3x3 tbn = float3x3(t, b, n);\n";
			src += "    float3 tmp_n = float3(1.0, 1.0, 1.0);\n";
			if (hasNM) {
				texFunc(aiTextureType_NORMALS, "tmp_n");
				src += "    tmp_n = tmp_n * 2.0 - float3(1.0, 1.0, 1.0);\n";
			}
			else if (hasH) {
				const TexStage &ts = sd.texStages[texStack[aiTextureType_HEIGHT][0]];
				String texId = strUtils::ConstructString("%1%2").arg(TEX_TYPES2[aiTextureType_HEIGHT]).arg(texStack[aiTextureType_HEIGHT].size() > 1 ? "0" : "");
				if (texStack[aiTextureType_HEIGHT].size() > 1)
					src += "    // NOTE: We are only using the first heightmap. Skipping others.\n";
				if (ts.mapping != aiTextureMapping_UV) {
					src += strUtils::ConstructString("    // %1 does not have an UV-map defined. Skipping it. You should probably make sure you have \'Generate UV-coordinates\' checked in the import-dialog box.\n").arg(texId);
				}
				else {
					String samplerId = samplerName[ts.sampleridx];
					String texCoord = strUtils::ConstructString("IN.texcoord%1").arg(ts.coordset);
					src +=			"    // Just a tip: Generate a normal map from your elevation map instead! Faster and better!\n";
					src +=			"    {\n";
					src +=			"        float2 dxy;\n";
					src += strUtils::ConstructString(	"        %1.GetDimensions(dxy.x, dxy.y);\n").arg(texId);
					src +=			"        dxy = 1.0 / dxy;\n";
					src += strUtils::ConstructString(	"        float tx0 = length(%1.Sample(%2, %3 + float2(-dxy.x, 0.0)));\n").arg(texId).arg(samplerId).arg(texCoord);
					src += strUtils::ConstructString(	"        float tx1 = length(%1.Sample(%2, %3 + float2( dxy.x, 0.0)));\n").arg(texId).arg(samplerId).arg(texCoord);
					src += strUtils::ConstructString(	"        float ty0 = length(%1.Sample(%2, %3 + float2(0.0, -dxy.y)));\n").arg(texId).arg(samplerId).arg(texCoord);
					src += strUtils::ConstructString(	"        float ty1 = length(%1.Sample(%2, %3 + float2(0.0,  dxy.y)));\n").arg(texId).arg(samplerId).arg(texCoord);
					src +=			"        tmp_n = float3((tx0 - tx1) * material.bump_scaling, (ty0 - ty1) * material.bump_scaling, 1.0);\n";
					src +=			"    }\n";
				}
			}
			src += "    n = normalize(mul(tmp_n, tbn));\n";
		}
		src += "    // Flip normal if back-facing primitive.\n";
		src += "    if (!IN.isFrontFace)\n";
		src += "        n = -n;\n";
		src += "    // Add ambient contribution.\n";
		src += strUtils::ConstructString("    float3 ambient = material.ambient_color.xyz%1;\n").arg(vColor);
		texFunc(aiTextureType_AMBIENT, "ambient");
		src += "    color += ambient * global.light_ambient;\n";
		src += "    // Add diffuse contribution.\n";
		src += strUtils::ConstructString("    float3 diffuse = material.diffuse_color.xyz%1;\n").arg(vColor);
		texFunc(aiTextureType_DIFFUSE, "diffuse");
		src += "    float3 light_dir = normalize(global.light_dir);\n";
		src += "    float nDotL = max(dot(n, light_dir), 0.0);\n";
		src += "    color += diffuse * global.light_diffuse * nDotL;\n";
		if (sd.shadingMode != aiShadingMode_Gouraud) {
			src += "    // Add specular contribution.\n";
			src += "    float3 specular = material.specular_color.xyz;\n";
			texFunc(aiTextureType_DIFFUSE, "specular");
			src += "    float3 reflection_dir = reflect(-light_dir, n);\n";
			src += "    float3 eye_dir = normalize(viewInverse[3].xyz - IN.worldPos);\n";
			src += "    float spec = max(dot(eye_dir, reflection_dir), 0.0);\n";
			src += "    color += specular * global.light_specular * pow(spec, material.shininess) * material.shininess_strength;\n";
		}
	}
	else if (vl.hasColors)
		src += "    color = IN.color.xyz;\n";
	if (!texStack[aiTextureType_LIGHTMAP].empty()) {
		src += "    // Apply lightmap.\n";
		src += "    float3 lightMap = float3(1.0, 1.0, 1.0);\n";
		texFunc(aiTextureType_LIGHTMAP, "lightMap");
		src += "    color *= lightmap;\n";
	}
	src += "    // Add emissive lighting.\n";
	src += "    float3 emissive = material.emissive_color.xyz;\n";
	texFunc(aiTextureType_EMISSIVE, "emissive");
	src += "    color += emissive;\n";
	src += "    OUT.xyz = color;\n";
	src += "    // Apply opacity.\n";
	src += "    float alpha = 1.0;\n";
	texFunc(aiTextureType_OPACITY, "alpha", "float", "w");
	src += "    alpha *= material.opacity;\n";
	if (vl.hasColors)
		src += "    alpha *= IN.color.w;\n";
	src += "    OUT = saturate(float4(color, alpha));\n";
	if (sd.shadingMode == aiShadingMode_Toon) {
		src += "    // Create a very simple toon effect by limiting the number of colors to 8^3!\n";
		src += "    OUT = floor(OUT * 8.0) / 8.0;\n";
	}
	src += "    return OUT;\n";
	src += "}\n";

	ps->SetManualMode(true);
	ps->SetShaderModel(ShaderModel::SM51);
	ps->SetShaderType(ShaderType::PS);
	ps->SetShaderSource(src);
	ps->Compile();
}
