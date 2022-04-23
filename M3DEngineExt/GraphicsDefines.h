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


#define CHIP_HEIGHT 1.0f
#define CONNECTION_WIDTH 0.35f
#define CONNECTION_SPACE 0.05f
#define CONNECTION_HEIGHT 0.2f
#define CONNECTION_YPAD 0.005f
#define CONNECTION_YPOS (CHIP_HEIGHT * 0.5f + CONNECTION_HEIGHT)
#define CHIP_HALF_HEIGHT (CHIP_HEIGHT * 0.5f)
#define CHIPTEXT_CUTOFF 20.0f
#define BACKGROUNDITEM_MIN_SIZE 2.0f
#define BACKGROUNDITEM_HANDLE_SIZE 0.5f
#define MAX_NAME_LENGTH 40
#define DATAPARTICLE_SPEED 3.0f
#define LINK_ARROW_SIZE 0.15f
#define DATA_PARTICLE_WIDTH 0.15f
#define DATA_PARTICLE_LENGTH 0.3f


namespace m3d
{

struct ColorScheme
{
	enum ColorIndex { CI_BACKGROUND, CI_FOREGROUND = 3, CI_CHIP = 15, CI_PARAMETER, CI_FUNC_CALL, CI_FUNC_DATA, CI_INSTANCE_DATA, CI_THIS_CHIP, CI_RG_CHIP, CI_BREAKPOINT, CI_SHADERIO, CI_SHADERUNIFORM };

	Color background; // clear color
	Color backgroundClassDiagram; // clear color for class diagram
	Color backgroundLibrary; // Clear color for library classes
	Color foreground; // text color
	Color selected; // Color indication selected item
	Color folder; // color for folders
	Color lines; // background +, rubber band
	Color connectors; // Default color for connectors
	Color growingConnectors; // Color for growing connector
	Color multiConnector; // Color for multi-connector
	Color canLink; // Connector color when linking can be done
	Color cantLink; // Connector color when linking can't be done
	Color links; // Color of links
	Color iLinks; // Color of links used to indicate shortcuts
	Color active; // Color for active items
	Color chip; // default chip background
	Color parameter; // Fill color of a parameter chip.
	Color funcCall;
	Color funcData;
	Color instanceData;
	Color thisChip;
	Color rgChip;
	Color breakPoint;
	Color shaderIO;
	Color shaderUniform;
	Color staticFunc;
	Color virtualFunc;
	Color nonvirtualFunc;
	Color infoBox;
	Color templateExtent;
	Color hoverFactor;
	Color chipShortcutFactor;
};


extern const ColorScheme StdColors;

}