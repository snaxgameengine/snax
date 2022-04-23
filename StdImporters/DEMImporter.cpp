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
#include "DEMImporter.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ClassManager.h"
#include "M3DEngine/ChipManager.h"
#include "M3DEngine/DocumentManager.h"
#include "StdChips/Text.h"
#include "GraphicsChips/Texture.h"

#include <ppl.h>
#include <fstream>
#include <sstream>
#include <DirectXTex.h>
#include <DirectXTK12/SimpleMath.h>

using namespace m3d;
using namespace DirectX::SimpleMath;


CHIPDESCV1_DEF_IMPORTER(DEMImporter, MTEXT("DEM Importer"), DEMIMPORTER_GUID, IMPORTER_GUID, MTEXT("dem"));



DEMImporter::DEMImporter()
{
}

DEMImporter::~DEMImporter()
{
}

Class *DEMImporter::Import(Chip **mainChip)
{
	std::ifstream demFile(GetFilename().AsString().c_str(), std::ios::ate);
	if (!demFile.is_open())
		return nullptr;

	memset(&_aRec, 0, sizeof(_aRec));
	char block[1024]; // standard block size.

	std::ifstream::pos_type demFileByteSize = demFile.tellg();

	demFile.seekg(0);

	demFile.read(block, 1024); // Read A-block (file header).

	// Parse A-block
	_parseString(_aRec.file_name, block + 0, 40);
	_parseString(_aRec.free_text_format, block + 40, 40);
	_parseString(_aRec.SE_geographic_corner_S, block + 109, 13);
	_parseString(_aRec.SE_geographic_corner_E, block + 122, 13);
	_aRec.process_code = block[135];
	_parseString(_aRec.origin_code, block + 140, 4);
	_aRec.dem_level_code = _parseInt(block + 144);
	_aRec.elevation_pattern = _parseInt(block + 150);
	_aRec.ground_ref_system = _parseInt(block + 156);
	_aRec.ground_ref_zone = _parseInt(block + 162);
	for (uint32 i = 0; i < 15; i++)
		_aRec.projection[i] = _parseDouble(block + 168 + i * 24);
	_aRec.ground_unit = _parseInt(block + 528);
	_aRec.elevation_unit = _parseInt(block + 534);
	_aRec.side_count = _parseInt(block + 540);
	_aRec.sw_coord[0] = _parseDouble(block + 546); // UTM grid (measured in meters)
	_aRec.sw_coord[1] = _parseDouble(block + 570);
	_aRec.nw_coord[0] = _parseDouble(block + 594);
	_aRec.nw_coord[1] = _parseDouble(block + 618);
	_aRec.ne_coord[0] = _parseDouble(block + 642);
	_aRec.ne_coord[1] = _parseDouble(block + 666);
	_aRec.se_coord[0] = _parseDouble(block + 690);
	_aRec.se_coord[1] = _parseDouble(block + 714);
	_aRec.elevation_min = _parseDouble(block + 738);
	_aRec.elevation_max = _parseDouble(block + 762);
	_aRec.ccw_angle = _parseDouble(block + 786);
	_aRec.elevation_accuracy = _parseInt(block + 810);
	_aRec.xyz_resolution[0] = _parseFloat(block + 816);
	_aRec.xyz_resolution[1] = _parseFloat(block + 828);
	_aRec.xyz_resolution[2] = _parseFloat(block + 840);
	_aRec.northings_rows = _parseInt(block + 852);
	_aRec.eastings_cols = _parseInt(block + 858);
	_aRec.suspect_void = _parseInt(block + 886, 2);
	_aRec.percent_void = _parseInt(block + 896);
	
	_aRec.northings_rows = _aRec.eastings_cols; // WARNING. SHOULD NOT USE THIS VALUE. BUT WE ASSUME IS THE SAME WITH eastings SINCE IT IS ALWAYS THE CASE.

	// Elevation points. South->North for each profile West->East.
	DataBuffer db(_aRec.eastings_cols * _aRec.northings_rows * sizeof(float32));
	float32 *points = (float32*)db.getBuffer();

	int32 _format = 1; // 1=16-bit uint32, 0=32-bit floating point.
	float32 _minAlt = 0.0f; // for _format=1.
	float32 _maxAlt = 0.0f; // for _format=1.
	bool _generateNormalMap = true; 

	// Parse profiles. Each profile is a row of points going from south to north.
	for (int32 j = 0; j < _aRec.eastings_cols; j++) { // Iterate profiles. West->East.
		demFile.read(block, 1024); // Read first block (there may be many). First block contains header of profile (B-Block).
		int32 cn = _parseInt(block + 0);
		int32 ce = _parseInt(block + 6);
		int32 rows = _parseInt(block + 12);
		int32 columns = _parseInt(block + 18);
		float64 n = _parseDouble(block + 24);
		float64 e = _parseDouble(block + 48);
		float64 z = _parseDouble(block + 72);
		float64 l = _parseDouble(block + 96);
		float64 h = _parseDouble(block + 120);
		for (int32 i = 0, o = 144; i < rows; o = 0) { // Iterate blocks to get all points in profile.
			for (; o < 1020 && i < rows; o += 6, i++) // Iterate points in current block. South->North.
				points[j * rows + i] = float32(_aRec.xyz_resolution[2] * _parseInt(block + o) + z); // TODO: value can also -32767, which means it is a false value.
			if (i < rows)
				demFile.read(block, 1024); // Need to read a new block for current profile.
		}
	}
	// Done reading file. It may contain an additional C-Block, but we don't need that.

	demFile.close(); // close file manually and free up resources for rest of the process!

	// Calculate min/max elevation.
	float32 minElevation = std::numeric_limits<float32>::max(), maxElevation = -std::numeric_limits<float32>::max();
	for (int32 i = 0; i < _aRec.eastings_cols * _aRec.northings_rows; i++) {
		minElevation = std::min(minElevation, points[i]);
		maxElevation = std::max(maxElevation, points[i]);
	}

	// xml: UTM-zone, four corners, xres, yres, mine, maxe

	// Create the class
	Document *doc = engine->GetDocumentManager()->CreateDocument();
	if (!doc) {
		msg(WARN, MTEXT("Import failed"));
		return nullptr;
	}
	Class *importedCG = engine->GetClassManager()->CreateClass(_filename.GetFileNameWithoutExtention(), doc);
	if (!importedCG) {
		msg(WARN, MTEXT("Import failed"));
		return nullptr; // TODO: what about doc?
	}

	// Generate heightmap texture.
	{
		ScratchImage img;
		if (_format == 0) {
			img.Initialize2D(DXGI_FORMAT_R32_FLOAT, _aRec.eastings_cols, _aRec.northings_rows, 1, 1);
			const Image *data = img.GetImage(0, 0, 0);
			for (int32 j = 0; j < _aRec.northings_rows; j++) {
				for (int32 i = 0; i < _aRec.eastings_cols; i++) {
					((float32*)(data->pixels + data->rowPitch * j))[i] = points[_aRec.northings_rows - j - 1 + _aRec.eastings_cols * i];
				}
			}
		}
		else if (_format == 1) {
			float32 a = minElevation, b = maxElevation - minElevation;
			if (_minAlt != 0.0f || _maxAlt != 0.0f) { // Custom range?
				a = _minAlt;
				b = _maxAlt - _minAlt;
			}
			img.Initialize2D(DXGI_FORMAT_R16_UNORM, _aRec.eastings_cols, _aRec.northings_rows, 1, 1);
			const Image *data = img.GetImage(0, 0, 0);
			concurrency::parallel_for(0, _aRec.northings_rows, [=](int32 j) {
//			for (int32 j = 0; j < _aRec.northings_rows; j++) {
				float32 c = 0.0f;
				for (int32 i = 0; i < _aRec.eastings_cols; i++) {
					c = (points[_aRec.northings_rows - j - 1 + _aRec.eastings_cols * i] - a) / b;
					c = std::min(std::max(c, 0.0f), 1.0f);
					((uint16*)(data->pixels + data->rowPitch * j))[i] = uint16(c * 65535.0f);
				}
			});
		}
		//HRESULT gr = SaveToDDSFile(*data, DDS_FLAGS_NONE, MTEXT("c:\\M3D\\jalla2.dds"));
		Blob blob;
		HRESULT hr = SaveToDDSMemory(*img.GetImage(0, 0, 0), DDS_FLAGS_NONE, blob);
		DataBuffer heightmapDDS;
		heightmapDDS.setBufferData((const uint8*)blob.GetBufferPointer(), blob.GetBufferSize());

		Texture *heightmap = dynamic_cast<Texture*>(importedCG->AddChip(engine->GetChipManager()->GetChipTypeIndex(TEXTURE_GUID)));
		heightmap->SetName(MTEXT("Heightmap"));
		heightmap->SetImageData(std::move(heightmapDDS), IFF_DDS);
	}

	// Generate normal map texture.
	if (_generateNormalMap) {
		ScratchImage img;
		img.Initialize2D(DXGI_FORMAT_R8G8_SNORM, _aRec.eastings_cols, _aRec.northings_rows, 1, 1);
		const Image *data = img.GetImage(0, 0, 0);
		concurrency::parallel_for(1, _aRec.northings_rows - 1, [=](int32 j) {
		//for (int32 j = 1; j < _aRec.northings_rows - 1; j++) {
			for (int32 i = 1; i < _aRec.eastings_cols - 1; i++) {
				float32 eN = points[_aRec.northings_rows - j - 0 + _aRec.eastings_cols * (i + 0)];
				float32 eE = points[_aRec.northings_rows - j - 1 + _aRec.eastings_cols * (i + 1)];
				float32 eS = points[_aRec.northings_rows - j - 2 + _aRec.eastings_cols * (i + 0)];
				float32 eW = points[_aRec.northings_rows - j - 1 + _aRec.eastings_cols * (i - 1)];
				XMVECTOR n = XMVector3Normalize(XMVectorSet(eW - eE, eS - eN, 2.0f * _aRec.xyz_resolution[0] + 2.0f * _aRec.xyz_resolution[1], 0.0f)) * 127.0f;
				XMStoreByte2((XMBYTE2*)(data->pixels + data->rowPitch * j) + i, n);
			}
		});
		Blob blob;
		HRESULT hr = SaveToDDSMemory(*data, DDS_FLAGS_NONE, blob);
		DataBuffer normalmapDDS;
		normalmapDDS.setBufferData((const uint8*)blob.GetBufferPointer(), blob.GetBufferSize());

		Texture *normalmap = dynamic_cast<Texture*>(importedCG->AddChip(engine->GetChipManager()->GetChipTypeIndex(TEXTURE_GUID)));
		normalmap->SetName(MTEXT("Normalmap"));
		normalmap->SetImageData(std::move(normalmapDDS), IFF_DDS);
	}

	// Write xml-description
	{
		String units[4] = {MTEXT("radians"), MTEXT("feet"), MTEXT("meters"), MTEXT("arc-seconds")};

		String d = strUtils::ConstructString(MTEXT("<?xml version=\"1.0\"?>\n<dem file=\"%1\" zone=\"%2\" ground_unit=\"%3\" elev_unit=\"%4\" %5 %6 elev_min=\"%7\" elev_max=\"%8\"/>\n"))
			.arg(_aRec.file_name)
			.arg(strUtils::fromNum(_aRec.ground_ref_zone))
			.arg((_aRec.ground_unit >= 0 && _aRec.ground_unit < 4) ? units[_aRec.ground_unit] : MTEXT("unknown"))
			.arg((_aRec.elevation_unit >= 0 && _aRec.elevation_unit < 4) ? units[_aRec.elevation_unit] : MTEXT("unknown"))
			.arg(strUtils::ConstructString(MTEXT("sw_x=\"%1\" sw_y=\"%2\" nw_x=\"%3\" nw_y=\"%4\" ne_x=\"%5\" ne_y=\"%6\" se_x=\"%7\" se_y=\"%8\""))
				.arg(strUtils::fromNum(_aRec.sw_coord[0])).arg(strUtils::fromNum(_aRec.sw_coord[1]))
				.arg(strUtils::fromNum(_aRec.nw_coord[0])).arg(strUtils::fromNum(_aRec.nw_coord[1]))
				.arg(strUtils::fromNum(_aRec.ne_coord[0])).arg(strUtils::fromNum(_aRec.ne_coord[1]))
				.arg(strUtils::fromNum(_aRec.se_coord[0])).arg(strUtils::fromNum(_aRec.se_coord[1])))
			.arg(strUtils::ConstructString(MTEXT("dx=\"%1\" dy=\"%2\" nx=\"%3\" ny=\"%4\""))
				.arg(strUtils::fromNum(_aRec.xyz_resolution[0])).arg(strUtils::fromNum(_aRec.xyz_resolution[1]))
				.arg(strUtils::fromNum(_aRec.eastings_cols)).arg(strUtils::fromNum(_aRec.northings_rows)))
			.arg(strUtils::fromNum(minElevation))
			.arg(strUtils::fromNum(maxElevation));

		Text *xmlDesc = (Text*)importedCG->AddChip(engine->GetChipManager()->GetChipTypeIndex(TEXT_GUID));
		xmlDesc->SetName(MTEXT("Description"));
		xmlDesc->SetText(d);
	}

	return importedCG;
}

void DEMImporter::_parseString(char *dest, const char *buffer, uint32 count)
{
	std::memcpy(dest, buffer, count); // count is without terminating 0.
	for (; count > 0; count--)
		if (dest[count - 1] != ' ')
			break;
	dest[count] = '\0';
}

int32 DEMImporter::_parseInt(const char *buffer, uint32 count)
{
	char tmp[64];
	_parseString(tmp, buffer, count);
	for (char *c = tmp; *c; c++) if (*c == 'D') *c = 'E';
	return atoi(tmp);
}

float64 DEMImporter::_parseDouble(const char *buffer, uint32 count)
{
	char tmp[64];
	_parseString(tmp, buffer, count);
	for (char *c = tmp; *c; c++) if (*c == 'D') *c = 'E';
	return atof(tmp);
}