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

#include "StdAfx.h"
#include "HLSLHighlighter.h"



HLSLHighlighter::HLSLHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
	HighlightingRule rule;

	_keywordFormat.setForeground(Qt::blue);
	QStringList keywordPatterns;
	keywordPatterns << 
		// Preprocessor Directives:
		"^[ \t]*#define\\b" << "^[ \t]*#elif\\b" << "^[ \t]*#else\\b" << "^[ \t]*#endif\\b" << "^[ \t]*#error\\b" << "^[ \t]*#if\\b" << 
		"^[ \t]*#ifdef\\b" << "^[ \t]*#ifndef\\b" << "^[ \t]*#include\\b" << "^[ \t]*#line\\b" << "^[ \t]*#pragma\\b" << "^[ \t]*#undef\\b" <<

		// HLSL-Keywords:
		"\\bbool([2-4]|[1-4]x[2-4]|[2-4]x[1-4])?\\b" << "\\bbreak\\b" << "\\bBuffer\\b" << "\\bcbuffer\\b" << "\\bcentroid\\b"  << "\\bclass\\b" << 
		"\\bconst\\b" << "\\bcontinue\\b" << "\\bdiscard\\b" << "\\bdo\\b" << "\\bdouble([2-4]|[1-4]x[2-4]|[2-4]x[1-4])?\\b" << "\\belse\\b" << 
		"\\bextern\\b" << "\\bfalse\\b" << "\\bfloat([2-4]|[1-4]x[2-4]|[2-4]x[1-4])?\\b" << "\\bfor\\b" << "\\bhalf([2-4]|[1-4]x[2-4]|[2-4]x[1-4])?\\b" << 
		"\\bif\\b" << "\\bin\\b" << "\\binline\\b" << "\\binout\\b" << "\\bint([2-4]|[1-4]x[2-4]|[2-4]x[1-4])?\\b" << "\\binterface\\b" << "\\blinear\\b" << 
		"\\bmatrix\\b" << "\\bnamespace\\b" << "\\bnointerpolation\\b" << "\\bnoperspective\\b" << "\\bout\\b" <<  "\\bprecise\\b" << "\\breturn\\b" << 
		"\\bregister\\b" << "\\bsample\\b" <<  "\\bsampler\\b" << "\\bsampler1D\\b" << "\\bsampler2D\\b" << "\\bsampler3D\\b" << "\\bsamplerCUBE\\b" <<
		"\\bshared\\b" << "\\bstatic\\b" << "\\bstring\\b" << "\\bstruct\\b" << "\\bswitch\\b" << "\\btbuffer\\b" << "\\bTexture1D\\b" << "\\bTexture1DArray\\b" << 
		"\\bTexture2D\\b" << "\\bTexture2DArray\\b" << "\\bTexture2DMS\\b" << "\\bTexture2DMSArray\\b" << "\\bTexture3D\\b" << "\\bTextureCube\\b" << 
		"\\bTextureCubeArray\\b" << "\\btrue\\b" << "\\btypedef\\b" << "\\buniform\\b" << "\\bvector\\b" << "\\bvoid\\b" << "\\bvolatile\\b" << "\\bwhile\\b" << 
		"\\bConstantBuffer\\b" << "\\bRWTexture1D\\b" << "\\bRWTexture2D\\b" << "\\bRWTexture3D\\b" << "\\bRWTexture1DArray\\b" << "\\bRWTexture2DArray\\b" <<
		"\\bRWBuffer\\b" << "\\bRWByteAddressBuffer\\b" << "\\bRWStructuredBuffer\\b" <<

		// Semantics:
		"\\bSTATIC_SAMPLER\\d+\\b" << "\\bSAMPLER\\d+\\b" << "\\bTEXTURE\\d+\\b" << "\\bUAV\\d+\\b" << "\\bCBUFFER\\d+\\b" <<
		"\\bView\\b" << "\\bProjection\\b" << "\\bViewInverse\\b" << "\\bViewInverseTranspose\\b" << "\\bProjectionInverse\\b" << "\\bViewProjection\\b" << 
		"\\bViewProjectionInverse\\b" << "\\bWorld\\b" << "\\bWorldInverse\\b" << "\\bWorldInverseTranspose\\b" << "\\bWorldView\\b" << "\\bWorldViewInverse\\b" << 
		"\\bWorldViewProjection\\b" << "\\bWorldViewProjectionInverse\\b" << "\\bShadow\\b" << "\\bWorldShadow\\b" << 
		// OTHER POSSIBLE SEMANTICS:	
		//"ambient", "animationtick", "animationtime", "attenuation", "cameraposition", "color", "constantattenuation", "diffuse", "diffusemap", "direction", "elapsedtime", "emission", "emissive", "environment", "environmentnormal", "envmap", "falloffangle", "falloffexponent", "height", "leftmousedown", "lightposition", "linearattenuation", "mouseposition", "none", "normal", "opacity", "position", "power", "projection", "projectioninverse", "projectioninversetranspose", "projectiontranspose", "quadraticattenuation", "refraction", "rendercolortarget", "renderdepthstenciltarget", "specular", "specularmap", "specularpower", "standardsglobal", "time", "transform", "unitsscale", "unknown", "user", "view", "viewinverse", "viewinversetranspose", "viewportpixelsize", "viewprojection", "viewprojectioninverse", "viewprojectioninversetranspose", "viewprojectiontranspose", "viewtranspose", "world", "worldinverse", "worldinversetranspose", "worldtranspose", "worldview", "worldviewinverse", "worldviewinversetranspose", "worldviewprojection", "worldviewprojectioninverse", "worldviewprojectioninversetranspose", "worldviewprojectiontranspose", "worldviewtranspose"

		// Intrinsic Functions:
		"\\babort\\b" << "\\babs\\b" << "\\bacos\\b" << "\\ball\\b" << "\\bAllMemoryBarrier\\b" << "\\bAllMemoryBarrierWithGroupSync\\b" << "\\bany\\b" << "\\basdouble\\b" << 
		"\\basfloat\\b" << "\\basin\\b" << "\\basint\\b" << "\\basuint\\b" << "\\batan\\b" << "\\batan2\\b" << "\\bceil\\b" << "\\bclamp\\b" << "\\bclip\\b" << "\\bcos\\b" << 
		"\\bcosh\\b" << "\\bcountbits\\b" << "\\bcross\\b" << "\\bD3DCOLORtoUBYTE4\\b" << "\\bddx\\b" << "\\bddx_coarse\\b" << "\\bddx_fine\\b" << "\\bddy\\b" << "\\bddy_coarse\\b" << 
		"\\bddy_fine\\b" << "\\bdegrees\\b" << "\\bdeterminant\\b" << "\\bDeviceMemoryBarrier\\b" << "\\bDeviceMemoryBarrierWithGroupSync\\b" << "\\bdistance\\b" << "\\bdot\\b" << 
		"\\bdst\\b" << "\\berrorf\\b" << "\\bEvaluateAttributeAtCentroid\\b" << "\\bEvaluateAttributeAtSample\\b" << "\\bEvaluateAttributeSnapped\\b" << "\\bexp\\b" << "\\bexp2\\b" << 
		"\\bf16tof32\\b" << "\\bf32tof16\\b" << "\\bfaceforward\\b" << "\\bfirstbithigh\\b" << "\\bfirstbitlow\\b" << "\\bfloor\\b" << "\\bfmod\\b" << "\\bfrac\\b" << "\\bfrexp\\b" << 
		"\\bfwidth\\b" << "\\bGetRenderTargetSampleCount\\b" << "\\bGetRenderTargetSamplePosition\\b" << "\\bGroupMemoryBarrier\\b" << "\\bGroupMemoryBarrierWithGroupSync\\b" << 
		"\\bInterlockedAdd\\b" << "\\bInterlockedAnd\\b" << "\\bInterlockedCompareExchange\\b" << "\\bInterlockedCompareStore\\b" << "\\bInterlockedExchange\\b" << "\\bInterlockedMax\\b" << 
		"\\bInterlockedMin\\b" << "\\bInterlockedOr\\b" << "\\bInterlockedXor\\b" << "\\bisfinite\\b" << "\\bisinf\\b" << "\\bisnan\\b" << "\\bldexp\\b" << "\\blength\\b" << "\\blerp\\b" << 
		"\\blit\\b" << "\\blog\\b" << "\\blog10\\b" << "\\blog2\\b" << "\\bMad\\b" << "\\bmax\\b" << "\\bmin\\b" << "\\bmodf\\b" << "\\bmul\\b" << "\\bnoise\\b" << "\\bnormalize\\b" << 
		"\\bpow\\b" << "\\bprintf\\b" << "\\bProcess2DQuadTessFactorsAvg\\b" << "\\bProcess2DQuadTessFactorsMax\\b" << "\\bProcess2DQuadTessFactorsMin\\b" << "\\bProcessIsolineTessFactors\\b" << 
		"\\bProcessQuadTessFactorsAvg\\b" << "\\bProcessQuadTessFactorsMax\\b" << "\\bProcessQuadTessFactorsMin\\b" << "\\bProcessTriTessFactorsAvg\\b" << "\\bProcessTriTessFactorsMax\\b" << 
		"\\bProcessTriTessFactorsMin\\b" << "\\bradians\\b" << "\\bRcp\\b" << "\\breflect\\b" << "\\brefract\\b" << "\\breversebits\\b" << "\\bround\\b" << "\\brsqrt\\b" << "\\bsaturate\\b" << 
		"\\bsign\\b" << "\\bsin\\b" << "\\bsincos\\b" << "\\bsinh\\b" << "\\bsmoothstep\\b" << "\\bsqrt\\b" << "\\bstep\\b" << "\\btan\\b" << "\\btanh\\b" << "\\btex1D\\b" << "\\btex1Dbias\\b" << 
		"\\btex1Dgrad\\b" << "\\btex1Dlod\\b" << "\\btex1Dproj\\b" << "\\btex2D\\b" << "\\btex2Dbias\\b" << "\\btex2Dgrad\\b" << "\\btex2Dlod\\b" << "\\btex2Dproj\\b" << "\\btex3D\\b" << 
		"\\btex3Dbias\\b" << "\\btex3Dgrad\\b" << "\\btex3Dlod\\b" << "\\btex3Dproj\\b" << "\\btexCUBE\\b" << "\\btexCUBEbias\\b" << "\\btexCUBEgrad\\b" << "\\btexCUBElod\\b" << 
		"\\btexCUBEproj\\b" << "\\btranspose\\b" << "\\btrunc\\b";
		
	foreach (QString pattern, keywordPatterns) {
		rule.pattern = QRegularExpression(pattern);
		rule.format = _keywordFormat;
		_highlightingRules.append(rule);
	}

//	_classFormat.setFontWeight(QFont::Bold);
//	_classFormat.setForeground(Qt::darkMagenta);
//	rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
//	rule.format = _classFormat;
//	_highlightingRules.append(rule);

	// strings: "a string"
	_quotationFormat.setForeground(Qt::darkRed);
	rule.pattern = QRegularExpression("\".*\"");
	rule.format = _quotationFormat;
	_highlightingRules.append(rule);

	// Comments
	_singleLineCommentFormat.setForeground(Qt::darkGreen);

	// Multiline comments
	_multiLineCommentFormat.setForeground(Qt::darkGreen);


	// any numbers
//	_numberFormat.setFontWeight(QFont::Bold);
//	_numberFormat.setForeground(Qt::darkBlue);
//	rule.pattern = QRegExp("\\b[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?\\b");//\\b[0-9][0-9]*(\\.[0-9][0-9]*f?)?\\b");
//	rule.format = _numberFormat;
//	_highlightingRules.append(rule);


//	_functionFormat.setFontWeight(QFont::Bold);
//	_functionFormat.setForeground(Qt::darkBlue);
//	rule.pattern = QRegExp("\\b[A-Za-z_][A-Za-z0-9_]+(?=\\()");//\\b[A-Za-z0-9_]+(?=\\()");
//	rule.format = _functionFormat;
//	_highlightingRules.append(rule);
	
}

void HLSLHighlighter::highlightBlock(const QString &text)
{
	static const QRegularExpression singlelineCommentStartExpression = QRegularExpression("//[^\n]*");
	static const QRegularExpression multilineCommentStartExpression = QRegularExpression("/\\*");
	static const QRegularExpression multilineCommentEndExpression = QRegularExpression("\\*/");

	// Iterate standard rules
	foreach (HighlightingRule rule, _highlightingRules) {
		QRegularExpressionMatchIterator itr = rule.pattern.globalMatch(text);
		while (itr.hasNext()) {
			QRegularExpressionMatch m = itr.next();
			int length = m.capturedLength(), index = m.capturedStart();
			setFormat(index, length, rule.format);
		}
	}

	// Find single line comment
	int commentStart = -1;
	QRegularExpressionMatch m = singlelineCommentStartExpression.match(text);
	if (m.hasMatch() && format(m.capturedStart()) != _quotationFormat)  { // Not in text string?
		commentStart = m.capturedStart();
		int length = m.capturedLength();
		setFormat(commentStart, length, _singleLineCommentFormat);
	}

	// Clear multiline comment flag
	setCurrentBlockState(0); 

	int startIndex = 0;
	if (previousBlockState() != 1) { // Previous block not in multiline comment..
		m = multilineCommentStartExpression.match(text);
		startIndex = m.capturedStart();
		if (commentStart != -1 && m.capturedStart() > commentStart)
			startIndex = -1;
	}

	while (startIndex >= 0) {
		int endIndex = text.indexOf(multilineCommentEndExpression, startIndex);
		m = multilineCommentEndExpression.match(text, startIndex);
		int commentLength;
		if (!m.hasMatch()) {
			setCurrentBlockState(1);
			commentLength = text.length() - startIndex;
		} 
		else {
			commentLength = endIndex - startIndex + m.capturedLength();
		}
		setFormat(startIndex, commentLength, _multiLineCommentFormat);
		startIndex = text.indexOf(multilineCommentStartExpression, startIndex + commentLength);
	}
}