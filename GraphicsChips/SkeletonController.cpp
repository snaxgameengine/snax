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

#include "pch.h"
#include "SkeletonController.h"
#include "M3DEngine/Engine.h"
#include "StdChips/MatrixArray.h"
#include "StdChips/Value.h"

using namespace m3d;


CHIPDESCV1_DEF(SkeletonController, MTEXT("Skeleton Controller"), SKELETONCONTROLLER_GUID, CHIP_GUID);


SkeletonController::SkeletonController()
{
	CREATE_CHILD(0, SKELETON_GUID, false, UP, MTEXT("Skeleton"));
	CREATE_CHILD(1, VALUE_GUID, false, UP, MTEXT("Custom Time"));
	CREATE_CHILD(2, MATRIXARRAY_GUID, false, DOWN, MTEXT("Skinning Matrix Array"));
	CREATE_CHILD(3, VALUE_GUID, false, UP, MTEXT("First Index in Skinning Matrix Array"));
	CREATE_CHILD(4, MATRIXARRAY_GUID, false, DOWN, MTEXT("World Matrix Array"));
}

SkeletonController::~SkeletonController()
{
}

void SkeletonController::CallChip()
{
	if (!Refresh)
		return;

	ChildPtr<Skeleton> ch0 = GetChild(0);
	if (!ch0)
		return;

	ChildPtr<Value> ch1 = GetChild(1);
	float64 t = ch1 ? ch1->GetValue() : (float64)engine->GetAppTime() / 1000000.0;

	const Map<String, Skeleton::Animation> &a = ch0->GetAnimations();

	Skeleton::ActiveAnimationSet animations; // Active animations ordered by priority low-high.

	_updateController(t, a, animations);

	ChildPtr<MatrixArray> ch2 = GetChild(2);
	if (!ch2)
		return;

	List<XMFLOAT4X4> &skinningMatrices = ch2->GetArray();

	ChildPtr<Value> ch3 = GetChild(3);
	uint32 firstIndex = ch3 ? (uint32)ch3->GetValue() : 0;
	firstIndex = std::min(firstIndex, 0x00FFFFFFu); // Limit it to some "reasonable" value.

	List<XMFLOAT4X4> *worldMatrices = nullptr;
	ChildPtr<MatrixArray> ch4 = GetChild(4);
	if (ch4)
		worldMatrices = &ch4->GetArray();

	ch0->UpdateSkinningMatrices(animations, skinningMatrices, firstIndex, worldMatrices);
}

bool SkeletonController::AddCommand(String name, float64 multiplier, float64 fadein, float64 fadeout)
{
	AnimationCmd cmd(multiplier, fadein, fadeout, false);
/*	cmd.cancel = false;
	cmd.cancelTime = 0.0;
	cmd.currentTime = 0.0;
	cmd.fadein = fadein;
	cmd.fadeout = fadeout;
	cmd.multiplier = multiplier;
	cmd.repeat = false;
	cmd.startTime = 0;
	cmd.justAdded = true;
*/
	return _commands.insert(std::make_pair(name, cmd)).second;
}

bool SkeletonController::AddCommand(String name, float64 multiplier, float64 fadein)
{
	AnimationCmd cmd(multiplier, fadein, 0.0, true);
	/*cmd.cancel = false;
	cmd.cancelTime = 0.0;
	cmd.currentTime = 0.0;
	cmd.fadein = fadein;
	cmd.fadeout = 0.0;
	cmd.multiplier = multiplier;
	cmd.repeat = true;
	cmd.startTime = 0;
	cmd.justAdded = true;
	*/
	return _commands.insert(std::make_pair(name, cmd)).second;
}

bool SkeletonController::RemoveCommand(String name, float64 fadeout)
{
	auto n = _commands.find(name);
	if (n == _commands.end())
		return false; // No such command!
	if (fadeout <= 0.0)
		_commands.erase(n); // No smooth fadeout!
	else {
		n->second.fadeout = fadeout;
		n->second.cancel = true;
		n->second.cancelTime = n->second.currentTime; // Fadeout from current time!
	}
	return true;
}

void SkeletonController::_updateController(float64 t, const Map<String, Skeleton::Animation> &animations, Skeleton::ActiveAnimationSet &activeAnimations)
{
	activeAnimations.clear();

	List<String> r; // List of commands to be removed.

	for (auto &n : _commands) { // Iterate command list
		if (n.second.justAdded) {
			n.second.startTime = t;
			n.second.justAdded = false;
		}
		n.second.currentTime = (t - n.second.startTime) * n.second.multiplier;

		if (n.second.currentTime < 0.0) { // Invalid time?
			r.push_back(n.first); // Remove command!
			continue;
		}

		auto m = animations.find(n.first);
		if (m == animations.end()) { // Does this animation not exist?
			r.push_back(n.first); // remove it!
			continue;
		}

		float64 inWeight = 1.0, outWeight = 1.0;
		float64 animationDuration = float64(m->second.duration / m->second.multiplier);

		if (n.second.fadein > 0.0 && n.second.currentTime < n.second.fadein)
			inWeight = n.second.currentTime / n.second.fadein;

		if (n.second.cancel) { // Cancel the animation?
			if (n.second.currentTime >= n.second.cancelTime + n.second.fadeout) { // Faded out?
				r.push_back(n.first); // Remove command!
				continue;
			}
			if (n.second.fadeout > 0.0)
				outWeight = 1.0 - (n.second.currentTime - n.second.cancelTime) / n.second.fadeout;
			outWeight = std::min(outWeight, 1.0);
		}

		if (!n.second.repeat) { // Not looped animation?
			if (n.second.currentTime >= animationDuration) { // Animation ended?
				r.push_back(n.first); // Remove it!
				continue;
			}

			float64 c = 1.0;

			if (n.second.fadeout > 0.0 && (animationDuration - n.second.currentTime) < n.second.fadeout)
				c = (animationDuration - n.second.currentTime) / n.second.fadeout;

			outWeight = std::min(outWeight, c);
		}

		float64 g = fmod(n.second.currentTime, animationDuration);
		if (g < 0.0)
			g += animationDuration;

		activeAnimations.insert(Skeleton::ActiveAnimation(&m->second, std::min(inWeight, outWeight), g * m->second.multiplier));
	}

	for (size_t i = 0; i < r.size(); i++) // Iterate commands to be removed!
		_commands.erase(r[i]);
}
