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


namespace m3d
{


static const Guid DEMIMPORTER_GUID = { 0x55ab0f53, 0x727e, 0x45de, { 0x9c, 0x50, 0x93, 0xa0, 0x2e, 0xe6, 0xb0, 0x2b } };


class STDIMPORTERS_API DEMImporter : public Importer
{
	CHIPDESC_DECL;
public:
	DEMImporter();
	virtual ~DEMImporter();

	virtual Class *Import(Chip **mainChip = nullptr);

protected:
	struct ARecord
	{
		/// <summary>
		/// The authorized digital cell name followed by a comma, space, and the two-character State designator(s) separated by hyphens.
		/// Abbreviations for other countries, such as Canada and Mexico, shall not be represented in the DEM header.
		/// </summary>
		char file_name[41];
		/// <summary>
		/// Free format descriptor field, contains useful information related to digital process such as digitizing instrument, photo codes, slot widths, etc.
		/// </summary>
		char free_text_format[41];
		/// <summary>
		/// Southing of the southeast geographic corner
		/// SE geographic quadrangle corner ordered as:
		///     x = Longitude = SDDDMMSS.SSSS
		///     y = Latitude = SDDDMMSS.SSSS
		/// (neg sign (S) right justified, no leading zeroes, plus sign (S) implied)
		/// </summary>
		char SE_geographic_corner_S[14];
		/// <summary>
		/// Easting of the southeast geographic corner
		/// SE geographic quadrangle corner ordered as:
		///     x = Longitude = SDDDMMSS.SSSS
		///     y = Latitude = SDDDMMSS.SSSS
		/// (neg sign (S) right justified, no leading zeroes, plus sign (S) implied)
		/// </summary>
		char SE_geographic_corner_E[14];
		/// <summary>
		/// 1=Autocorrelation RESAMPLE Simple bilinear
		/// 2=Manual profile GRIDEM Simple bilinear
		/// 3=DLG/hypsography CTOG 8-direction linear
		/// 4=Interpolation from photogrammetric system contours DCASS 4-direction linear
		/// 5=DLG/hypsography LINETRACE, LT4X Complex linear
		/// 6=DLG/hypsography CPS-3, ANUDEM, GRASS Complex polynomial
		/// 7=Electronic imaging (non-photogrametric), active or passive, sensor systems.
		/// </summary>
		char process_code;
		/// <summary>
		/// Free format Mapping Origin Code. Example: MAC, WMC, MCMC, RMMC, FS, BLM, CONT (contractor), XX (state postal code).
		/// </summary>
		char origin_code[5];
		/// <summary>
		/// Code 1=DEM-1
		/// 2=DEM-2
		/// 3=DEM-3
		/// 4=DEM-4
		/// </summary>
		int32 dem_level_code;
		/// <summary>
		/// 1 = regular
		/// 2 = random
		/// </summary>
		int32 elevation_pattern;
		int32 ground_ref_system;
		int32 ground_ref_zone;
		float64 projection[15];
		int32 ground_unit;
		int32 elevation_unit;
		int32 side_count;
		// pairs of easting-northings
		float64 sw_coord[2];
		float64 nw_coord[2];
		float64 ne_coord[2];
		float64 se_coord[2];
		float64 ccw_angle;
		float64 elevation_min;
		float64 elevation_max;
		int32 elevation_accuracy;
		float32 xyz_resolution[3];
		int32 northings_rows;
		int32 eastings_cols;
		int32 suspect_void;
		int32 percent_void;
	};
	ARecord _aRec;

	void _parseString(char *dest, const char *buffer, uint32 count);
	int32 _parseInt(const char *buffer, uint32 count = 6);
	float64 _parseDouble(const char *buffer, uint32 count = 24);
	float32 _parseFloat(const char *buffer, uint32 count = 12) { return (float32)_parseDouble(buffer, count); }
};

}