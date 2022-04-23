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
#include "DestructionObserver.h"


using namespace m3d;


DestructionObserver::DestructionObserver()
{
}

DestructionObserver::~DestructionObserver()
{
	while (_observed.rbegin() != _observed.rend())
		(*_observed.rbegin())->RemoveDestructionObserver(this);
}


DestructionObservable::DestructionObservable()
{
}

DestructionObservable::~DestructionObservable()
{
	// NotifyDestuction should have been called now, 
	// but erase any observers if not! (they will not be notified!)
	while (_observers.rbegin() != _observers.rend()) {
		DestructionObserver *d = *_observers.rbegin();
		_observers.erase(*_observers.rbegin());
		d->_observed.erase(this);
	}
}

void DestructionObservable::NotifyDestuction() 
{
	while (_observers.rbegin() != _observers.rend()) {
		DestructionObserver *d = *_observers.rbegin();
		_observers.erase(*_observers.rbegin());
		d->_observed.erase(this);
		d->OnDestruction(this);
	}
}

void DestructionObservable::AddDestructionObserver(DestructionObserver *obs)
{
	if (obs) {
		_observers.insert(obs);
		obs->_observed.insert(this);
	}
}

void DestructionObservable::RemoveDestructionObserver(DestructionObserver *obs)
{
	if (obs) {
		_observers.erase(obs);
		obs->_observed.erase(this);
	}
}
