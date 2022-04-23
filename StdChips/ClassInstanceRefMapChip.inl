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



template<typename T>
ClassInstanceRefMapChip<T>::ClassInstanceRefMapChip()
{
}

template<typename T>
ClassInstanceRefMapChip<T>::~ClassInstanceRefMapChip()
{
	_clear();
}

template<typename T>
void ClassInstanceRefMapChip<T>::_clear()
{
	for (const auto &n : this->_map)
		if (n.second.IsOwner())
			n.second->Release();
	this->_map.clear();
	this->_clearCache();
}

template<typename T>
bool ClassInstanceRefMapChip<T>::CopyChip(Chip *chip)
{
	_clear();

	ClassInstanceRefMapChip *c = dynamic_cast<ClassInstanceRefMapChip*>(chip);
	B_RETURN(ContainerChip::CopyChip(c));

	_preload = c->_preload;

	if (this->IsSaveContent()) {
		for (const auto &n : c->_map) {
			if (n.second.IsOwner())
				this->_map.insert(std::make_pair(n.first, n.second->Clone(this)));
			else
				this->_map.insert(std::make_pair(n.first, n.second));
		}
	}
	this->SetUpdateStamp();
	return true;
}

template<typename T>
bool ClassInstanceRefMapChip<T>::LoadChip(DocumentLoader &loader)
{
	_clear();
	B_RETURN(ContainerChip::LoadChip(loader));
	if (loader.GetDocumentVersion() >= Version(1,2,1,0)) // TODO: Remove!
		LOAD(MTEXT("preload"), _preload);
	if (this->IsSaveContent())
		LOAD(MTEXT("map"), this->_map);
	this->SetUpdateStamp();
	return true;
}

template<typename T>
bool ClassInstanceRefMapChip<T>::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(ContainerChip::SaveChip(saver));
	SAVE(MTEXT("preload"), _preload);
	if (this->IsSaveContent())
		SAVE(MTEXT("map"), this->_map);
	return true;
}

template<typename T>
void ClassInstanceRefMapChip<T>::OnDestroyDevice()
{
	for (const auto &n : this->_map)
		if (n.second.IsOwner())
			n.second->OnDestroyDevice();
}

template<typename T>
void ClassInstanceRefMapChip<T>::OnReleasingBackBuffer(RenderWindow *rw)
{
	for (const auto &n : this->_map)
		if (n.second.IsOwner())
			n.second->OnReleasingBackBuffer(rw);
}

template<typename T>
void ClassInstanceRefMapChip<T>::RestoreChip()
{
	for (const auto &n : this->_map) {
		n.second.Prepare(this);
		if (n.second.IsOwner())
			n.second->RestoreChip();
	}
}

template<typename T>
void ClassInstanceRefMapChip<T>::AddDependencies(ProjectDependencies &deps)
{
	for (const auto &n : this->_map)
		if (n.second.IsOwner())
			n.second->AddDependencies(deps);
}

template<typename T>
Chip *ClassInstanceRefMapChip<T>::FindChip(ChipID chipID)
{
	Chip *c = Chip::FindChip(chipID);
	if (c)
		return c;
	for (const auto &n : this->_map)
		if (n.second.IsOwner() && (c = n.second->FindChip(chipID)))
			break;
	return c;
}

template<typename T>
bool ClassInstanceRefMapChip<T>::RemoveElement(T key)
{
	auto n = this->_map.find(key);
	if (n != this->_map.end()) {
		if (n->second.IsOwner())
			n->second->Release();
		this->_map.erase(n);
		this->_clearCache();
		this->SetUpdateStamp();
		return true;
	}
	return false;
}

template<typename T>
bool ClassInstanceRefMapChip<T>::AddElement(T key, ClassInstanceRef val) 
{ 
	SetInstance(key, val, AT_REF); 
	return true;
}

template<typename T>
ClassInstanceRef ClassInstanceRefMapChip<T>::CreateInstance(T key, Class *instanceOf)
{
	RemoveElement(key);
	ClassInstanceRef ref = this->_map.insert(std::make_pair(key, ClassInstance::Create(instanceOf, this))).first->second;
	this->_clearCache();
	this->SetUpdateStamp();
	return ref;
}

template<typename T>
void ClassInstanceRefMapChip<T>::SetInstance(T key, ClassInstanceRef ref, AssignType at)
{
	auto n = this->_map.find(key);
	switch (at)
	{
	case AT_REF:
		if (n != this->_map.end() || n->second != ref) {
			RemoveElement(key);
			this->_map.insert(std::make_pair(key, ClassInstanceRef(ref, false)));
			this->_clearCache();
			this->SetUpdateStamp();
		}
		break;
	case AT_MOVE:
		if (n == this->_map.end() || n->second != ref) {
			RemoveElement(key);
			n = this->_map.insert(std::make_pair(key, ClassInstanceRef(ref, false))).first;
			this->_clearCache();
			this->SetUpdateStamp();
		}
		if (n->second) {
			n->second.SetOwner(true);
			n->second->SetOwner(this);
		}
		break;
	case AT_COPY:
		if (ref)
			ref = ref->Clone(this);
		RemoveElement(key);
		this->_map.insert(std::make_pair(key, ref));
		this->_clearCache();
		this->SetUpdateStamp();
		break;
	}
}

template<typename T>
void ClassInstanceRefMapChip<T>::MakeOwner(T key)
{
	auto n = this->_map.find(key);
	if (n != this->_map.end()) {
		if (!n->second.IsOwner()) {
			n->second->SetOwner(this);
			n->second.SetOwner(true);
			this->SetUpdateStamp();
		}
	}
}


template<typename T>
void ClassInstanceRefMapChip<T>::ClassInstanceRefMapChip::SetClass(Class* clazz)
{
	Chip::SetClass(clazz);
	for (const auto &n : this->_map) {
		auto& a = n.second;
		if (a && a.IsOwner())
			a->SetOwner(this);
	}
}
