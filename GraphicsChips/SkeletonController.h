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
#include "Skeleton.h"

namespace m3d
{


static const Guid SKELETONCONTROLLER_GUID = { 0xa6cd38c6, 0x47b0, 0x4513, { 0xb9, 0xe9, 0x3f, 0xc8, 0xdd, 0x53, 0xe5, 0x59 } };


class GRAPHICSCHIPS_API SkeletonController : public Chip
{
	CHIPDESC_DECL;
public:
	SkeletonController();
	virtual ~SkeletonController();

	virtual void CallChip() override;

	struct AnimationCmd
	{
		float64 startTime; // Time of first update!
		float64 multiplier; // Time multiplier.
		float64 currentTime; // Current animation time [0->
		float64 fadein; // Fadein time.
		float64 fadeout; // Fadeout time.
		float64 cancelTime; // currentTime at the time of cancel.
		bool justAdded; // true if this animation was just added to command list.
		bool repeat; // true if this is a looped animation.
		bool cancel; // true if asked to cancel this animation.

		AnimationCmd(float64 multiplier = 1.0, float64 fadein = 0.0, float64 fadeout = 0.0, bool repeat = false) : startTime(0.0), multiplier(multiplier), currentTime(0.0), fadein(fadein), fadeout(fadeout), cancelTime(0.0), justAdded(true), repeat(repeat), cancel(false) {}
	};

	// Add Single Animation
	virtual bool AddCommand(String name, float64 multiplier, float64 fadein, float64 fadeout);
	// Add Looped Animation
	virtual bool AddCommand(String name, float64 multiplier, float64 fadein);
	// Remove Animation
	virtual bool RemoveCommand(String name, float64 fadeout);

	virtual const Map<String, AnimationCmd> &GetCommands() const { return _commands; }

protected:
	Map<String, AnimationCmd> _commands;

	void _updateController(float64 t, const Map<String, Skeleton::Animation> &animations, Skeleton::ActiveAnimationSet &activeAnimations);
};



}