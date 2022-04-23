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
#include "M3DEngine/Chip.h"

namespace m3d
{

static const Guid SKELETON_GUID = { 0x3c9366d1, 0x695b, 0x4e81, { 0x9a, 0x4, 0x91, 0x28, 0x99, 0x31, 0x9e, 0xda } };


class GRAPHICSCHIPS_API Skeleton : public Chip
{
	CHIPDESC_DECL;
public:
	Skeleton();
	virtual ~Skeleton();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	struct Transform
	{
		XMFLOAT3 position;
		XMFLOAT4 rotation;
		XMFLOAT3 scaling;
	};

	struct Keyframe : public Transform
	{
		float32 time;
	};

	typedef List<Keyframe> KeyframeList;

	struct Animation
	{
		uint32 priority; // Priority of the animation. Higher priority goes before lower priority animations.
		float32 multiplier; // Time multiplier for this animation.
		float32 duration; // Should be calculated from the keyframes as max Keyframe.time.
		Map<String, KeyframeList> keyframes; // Each (uniquly) named joint can have a series of keyframes in an animation!

		Animation() : priority(0), multiplier(1.0f), duration(0.0f) {}
	};

	struct ActiveAnimation
	{
		const Animation *animation;
		float64 weight;
		float64 time;

		ActiveAnimation(const Animation *animation = nullptr, float64 weight = 0.0, float64 time = 0.0) : animation(animation), weight(weight), time(time) {}

		bool operator<(const ActiveAnimation &rhs) const { return animation->priority < rhs.animation->priority; }
	};

	typedef MultiSet<ActiveAnimation> ActiveAnimationSet;


	struct Joint
	{
		String name; // The name of the joint. Should be unique in the joint hierarky.
		uint32 index; // The index of the joint. This is the index to be used in the matrix array of joint transforms. Can be -1 if not in array.
		Transform jointTransform;
		XMFLOAT4X4 inverseBindPose;
		List<Joint> children;
	};
	
	// Returns the root joint which can be used to build the skeleton.
	virtual Joint &GetRootJoint() { return _root; }
	// Returns the map containing all animations ordered by name.
	virtual const Map<String, Animation> &GetAnimations() const { return _animations; }
	// Add an animation with the given name, and returns it. Returns null is exist.
	virtual Animation *AddAnimation(String name);
	// Removes animation with given name, and returns true if success.
	virtual bool RemoveAnimation(String name);
	// Renames animation. Returns true on success.
	virtual bool RenameAnimation(String oldName, String newName);
	// Returns animation with given name. nullptr if not found.
	virtual Animation *GetAnimation(String name);
	// Calculates the duration field using the keyframes for all joints.
	virtual float32 CalculateAnimationDuration(Animation *animation);
	// Updates the array of skinning matrices. NOTE: animations must be ordered by priority low->high.
	virtual void UpdateSkinningMatrices(const ActiveAnimationSet &animations, List<XMFLOAT4X4> &skinningMatrices, uint32 firstIndex = 0, List<XMFLOAT4X4>* worldMatrices = nullptr);
	// To be used by dialog's cancel function.
	virtual void SetRootJoint(const Joint &j) { _root = j; }
	virtual void SetAnimations(const Map<String, Animation> &a) { _animations = a; }
protected:
	Joint _root;
	Map<String, Animation> _animations;

	// Updates recursivly. NOTE: animations must be ordered by priority low->high.
	void _updateSkinningMatrices(Joint &j, CXMMATRIX parentWorldMatrix, const ActiveAnimationSet &animations, List<XMFLOAT4X4> &skinningMatrices, uint32 firstIndex, List<XMFLOAT4X4>* worldMatrices);
};



}