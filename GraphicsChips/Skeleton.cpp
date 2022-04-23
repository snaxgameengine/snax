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
#include "Skeleton.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "SkeletonController.h"
#include "StdChips/MatrixChip.h"

using namespace m3d;


CHIPDESCV1_DEF(Skeleton, MTEXT("Skeleton"), SKELETON_GUID, CHIP_GUID);


namespace m3d
{
	bool SerializeDocumentData(DocumentSaver &saver, const Skeleton::Joint &data) 
	{ 
		SAVE("name", data.name);
		SAVE("index", data.index);
		SAVE("jointTransform", data.jointTransform);
		SAVE("inverseBindPose", data.inverseBindPose);
		SAVE("children", data.children);
		return true;	
	}

	bool DeserializeDocumentData(DocumentLoader &loader, Skeleton::Joint &data) 
	{
		LOAD("name", data.name);
		LOAD("index", data.index);
		LOAD("jointTransform", data.jointTransform);
		LOAD("inverseBindPose", data.inverseBindPose)
		LOAD("children", data.children);
		return true;
	}

	bool SerializeDocumentData(DocumentSaver &saver, const Skeleton::Animation &data) 
	{ 
		SAVE("keyframes", data.keyframes);
		SAVE("priority", data.priority);
		SAVE("duration", data.duration);
		SAVE("multiplier", data.multiplier);
		return true;
	}

	bool DeserializeDocumentData(DocumentLoader &loader, Skeleton::Animation &data) 
	{
		LOAD("keyframes", data.keyframes);
		LOAD("priority", data.priority);
		LOADDEF("duration", data.duration, 0.0f);
		LOADDEF("multiplier", data.multiplier, 1.0f);
		return true;
	}

	bool SerializeDocumentData(DocumentSaver &saver, const Skeleton::Transform &data) 
	{ 
		SAVE("position", data.position);
		SAVE("rotation", data.rotation);
		SAVE("scaling", data.scaling);
		return true;	
	}

	bool DeserializeDocumentData(DocumentLoader &loader, Skeleton::Transform &data) 
	{
		LOAD("position", data.position);
		LOAD("rotation", data.rotation);
		LOAD("scaling", data.scaling);
		return true;
	}

	bool SerializeDocumentData(DocumentSaver &saver, const Skeleton::Keyframe &data) 
	{ 
		SAVE("transform", (const Skeleton::Transform&)data);
		SAVE("time", data.time);
		return true;	
	}

	bool DeserializeDocumentData(DocumentLoader &loader, Skeleton::Keyframe &data) 
	{
		LOAD("transform", (Skeleton::Transform&)data);
		LOAD("time", data.time);
		return true;
	}
}


Skeleton::Skeleton()
{
}

Skeleton::~Skeleton()
{
}

bool Skeleton::CopyChip(Chip *chip)
{
	Skeleton *c = dynamic_cast<Skeleton*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_root = c->_root;
	_animations = c->_animations;
	return true;
}

bool Skeleton::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	LOAD("joints", _root);
	LOAD("animations", _animations);
	return true;
}

bool Skeleton::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVE("joints", _root);
	SAVE("animations", _animations);
	return true;
}

Skeleton::Animation *Skeleton::AddAnimation(String name)
{
	if (_animations.find(name) != _animations.end())
		return nullptr;
	return &_animations.insert(std::make_pair(name, Animation())).first->second;
}

bool Skeleton::RemoveAnimation(String name)
{
	return _animations.erase(name) == 1;
}

bool Skeleton::RenameAnimation(String oldName, String newName)
{
	auto n = _animations.find(oldName);
	if (n == _animations.end())
		return false;
	auto m = _animations.find(newName);
	if (m != _animations.end())
		return false;
	Animation &a = _animations.insert(std::make_pair(newName, Animation())).first->second;
	a.duration = n->second.duration;
	a.multiplier = n->second.multiplier;
	a.priority = n->second.priority;
	a.keyframes = std::move(n->second.keyframes);
	_animations.erase(n);
	return true;
}

Skeleton::Animation *Skeleton::GetAnimation(String name)
{
	auto n = _animations.find(name);
	if (n == _animations.end())
		return nullptr;
	return &n->second;
}

float32 Skeleton::CalculateAnimationDuration(Animation* animation)
{
	animation->duration = -std::numeric_limits<float32>::max();
	for (const auto &n : animation->keyframes)
		if (n.second.size())
			animation->duration = std::max(animation->duration, n.second.back().time);

	return animation->duration;
}

void Skeleton::UpdateSkinningMatrices(const ActiveAnimationSet &animations, List<XMFLOAT4X4> &skinningMatrices, uint32 firstIndex, List<XMFLOAT4X4>* worldMatrices)
{
	return _updateSkinningMatrices(_root, XMMatrixIdentity(), animations, skinningMatrices, firstIndex, worldMatrices);
}


struct WeightedTransform : public Skeleton::Transform
{
	float32 weight;
	WeightedTransform(Transform t, float32 weight) : Transform(t), weight(weight) {}
};



Skeleton::Transform _interpolateTransform(const Skeleton::Transform &t0, const Skeleton::Transform &t1, float32 d)
{
	Skeleton::Transform t;
	XMStoreFloat3(&t.position, XMVectorLerp(XMLoadFloat3(&t0.position), XMLoadFloat3(&t1.position), d));
	XMStoreFloat4(&t.rotation, XMQuaternionSlerp(XMLoadFloat4(&t0.rotation), XMLoadFloat4(&t1.rotation), d));
	XMStoreFloat3(&t.scaling, XMVectorLerp(XMLoadFloat3(&t0.scaling), XMLoadFloat3(&t1.scaling), d));
	return t;
}

Skeleton::Transform _interpolateTransforms(const List<WeightedTransform> &transforms, size_t i)
{
	if (transforms[i].weight > 0.999f || i == 0)
		return transforms[i];
	return _interpolateTransform(_interpolateTransforms(transforms, i - 1), transforms[i], transforms[i].weight);
};

Skeleton::Transform _interpolateKeyframes(const Skeleton::KeyframeList &kf, float32 t)
{
	if (kf.empty())
		return Skeleton::Transform(); // should not happen!
	
	if (kf.size() == 1)
		return kf.front();

	size_t i = 0;
	for (; i < kf.size(); i++)
		if (kf[i].time > t)
			break;

	if (i == kf.size())
		return kf.back();

	if (i == 0) {
		float32 d = t / kf.front().time;
		return _interpolateTransform(kf.back(), kf.front(), d);
	}

	float32 d = (t - kf[i - 1].time) / (kf[i].time - kf[i - 1].time);

	return _interpolateTransform(kf[i - 1], kf[i], d);
}

uint32 __findMaxIndex(const Skeleton::Joint& joint)
{
	uint32 n = joint.index != -1 ? joint.index : 0;
	for (size_t i = 0; i < joint.children.size(); i++) {
		uint32 nc = __findMaxIndex(joint.children[i]);
		n = std::max(n, nc);
	}
	return n;
}

void Skeleton::_updateSkinningMatrices(Joint &j, CXMMATRIX parentWorldMatrix, const ActiveAnimationSet &animations, List<XMFLOAT4X4> &skinningMatrices, uint32 firstIndex, List<XMFLOAT4X4>* worldMatrices)
{
	Transform finalAnimatedTransform = j.jointTransform;

	struct WeightedAndPrioritizedTransform : public WeightedTransform
	{
		uint32 priority;
		WeightedAndPrioritizedTransform(WeightedTransform t, uint32 priority) : WeightedTransform(t), priority(priority) {}
	};

	Transform t = { {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f} };

	XMMATRIX worldMatrix;

	if (animations.size()) {
		List<WeightedAndPrioritizedTransform> transforms; // This contains list of transforms (one for each animation) after keyframe interpolation.

		for (const auto &m : animations) { // Iterate all animations
			auto n = m.animation->keyframes.find(j.name);
			if (n == m.animation->keyframes.end())
				continue; // This animation does not affect this joint!
			if (n->second.empty())
				continue; // No keyframes!
			transforms.push_back(WeightedAndPrioritizedTransform(WeightedTransform(_interpolateKeyframes(n->second, (float32)m.time), (float32)m.weight), m.animation->priority));
		}

		List<WeightedTransform> vv; // This contains a transform for each priority.
		vv.push_back(WeightedTransform(finalAnimatedTransform, 1.0f)); // <= This is the default fallback!

		for (uint32 i = 0, j = 0; i < transforms.size(); i = j) { // Iterate transforms. NOTE: these are ordered by priority!
			for (j = i + 1; j < transforms.size(); j++)
				if (transforms[j].priority != transforms[i].priority)
					break;
			if (j - i > 1) { // More than one transform with this priority?
				float32 sum = 0.0f;
				float32 L = 0.0f;
				List<WeightedTransform> v; // This list contains transforms of same priority ready to be interpolated based on their weight.
				for (uint32 k = i; k < j; k++) {
					sum += transforms[k].weight;
					L = std::max(L, transforms[k].weight); // The highest weight will be used as the weight for this priority group.
//					L += transforms[k].weight * transforms[k].weight;
					v.push_back(WeightedTransform(transforms[k], transforms[k].weight / sum));
				}
				vv.push_back(WeightedTransform(_interpolateTransforms(v, v.size() - 1), L)); // L will weight this priority group with the rest.
			}
			else {
				vv.push_back(transforms[i]);
			}
		}

		// Calculate the final transform of all the animations. There is only one transform for each priority.
		// If there are no animations, the default joint transform is used. (First index in vv)
		t = _interpolateTransforms(vv, vv.size() - 1);

		// Calculate matrix from animated transform.
		worldMatrix = XMMatrixAffineTransformation(XMLoadFloat3(&t.scaling), XMVectorZero(), XMLoadFloat4(&t.rotation), XMLoadFloat3(&t.position));
		worldMatrix *= parentWorldMatrix;
	}
	else {
		worldMatrix = XMMatrixInverse(nullptr, XMLoadFloat4x4(&j.inverseBindPose));
	}

	if (j.index != -1) { // Do we have an index into the matrix array?
		XMFLOAT4X4 skinningMatrix;
		XMStoreFloat4x4(&skinningMatrix, XMLoadFloat4x4(&j.inverseBindPose) * worldMatrix);

		if (skinningMatrices.size() <= (j.index + firstIndex)) {
			uint32 n = __findMaxIndex(_root);
			skinningMatrices.resize(firstIndex + n + 1u);
		}
		if (worldMatrices) {
			if (worldMatrices->size() <= j.index) {
				uint32 n = __findMaxIndex(_root);
				worldMatrices->resize(n + 1u);
			}
			XMStoreFloat4x4(&worldMatrices->at(j.index), worldMatrix);
		}

		skinningMatrices[j.index] = skinningMatrix;
	}
	for (size_t i = 0; i < j.children.size(); i++) // Iterate all subjoints
		_updateSkinningMatrices(j.children[i], worldMatrix, animations, skinningMatrices, firstIndex, worldMatrices);  // Recursive!
}
