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
#include "GlobalDef.h"
#include "Joysticks.h"
#include "M3DCore/MMath.h"
#include <SDL.h>
#include <SDL_joystick.h>

using namespace m3d;


Joysticks::Joysticks()
{
	SDL_InitSubSystem(SDL_INIT_JOYSTICK);
}

Joysticks::~Joysticks()
{
	SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

const JoystickState& Joysticks::GetState(uint32 index) const 
{ 
	_state[index].requested = true;
	return _state[index]; 
}

void Joysticks::Update()
{
	SDL_JoystickUpdate();

	int32 numJoysticks = SDL_NumJoysticks();

	for (int32 i = 0; i < MAX_JOYSTICKS; i++) {
		if (_state[i].joystick) {
			if (SDL_JoystickGetAttached((SDL_Joystick*)_state[i].joystick) != SDL_TRUE) {
				SDL_JoystickClose((SDL_Joystick*)_state[i].joystick);
				_state[i] = JoystickState();
			}
		}
		if (_state[i].requested && _state[i].joystick == nullptr) {
			if (i < numJoysticks) {
				_state[i].joystick = SDL_JoystickOpen(i);
			}
		}
		if (_state[i].joystick) {
			SDL_Joystick* joystick = (SDL_Joystick*)_state[i].joystick;
			{
				int32 numButtons = SDL_JoystickNumButtons(joystick);
				for (int32 j = 0; j < std::min(numButtons, 32); j++) {
					_state[i].buttons[j] = SDL_JoystickGetButton(joystick, j) != 0;
				}
			}
			{
				int32 numAxes = SDL_JoystickNumAxes(joystick);
				for (int32 j = 0; j < std::min(numAxes, 5); j++) {
					Sint16 v = SDL_JoystickGetAxis(joystick, j);
					_state[i].axes[j] = float32(v) / (v < 0 ? -SDL_JOYSTICK_AXIS_MIN : SDL_JOYSTICK_AXIS_MAX);
				}
			}
			{
				int32 numHats = SDL_JoystickNumHats(joystick);
				for (int32 j = 0; j < std::min(numHats, 4); j++) {
					Uint8 v = SDL_JoystickGetHat(joystick, j);
					uint32 n = 0;
					switch (v)
					{
					case SDL_HAT_CENTERED: n = 0; break;
					case SDL_HAT_UP: n = 1; break;
					case SDL_HAT_RIGHT: n = 3; break;
					case SDL_HAT_DOWN: n = 5; break;
					case SDL_HAT_LEFT: n = 7; break;
					case SDL_HAT_RIGHTUP: n = 2; break;
					case SDL_HAT_RIGHTDOWN: n = 4; break;
					case SDL_HAT_LEFTUP: n = 8; break;
					case SDL_HAT_LEFTDOWN: n = 6; break;
					default: n = 0; break;
					}
					_state[i].hats[j] = n;
				}
			}
		}
	}
}
