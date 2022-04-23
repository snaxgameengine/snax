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
#include "UserInput.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/InputManager.h"
#include "M3DEngine/RenderWindow.h"
#include "M3DEngine/RenderWindowManager.h"
#include "M3DEngine/Window.h"
#include "GraphicsChips/Graphics.h"
#include "M3DEngine/Joysticks.h"
#include "M3DEngine/Application.h"


using namespace m3d;


CHIPDESCV1_DEF(UserInput, MTEXT("User Input"), USERINPUT_GUID, VALUE_GUID);


UserInput::UserInput()
{
	ClearConnections();

	_dt = DeviceType::NONE;
	_mt = MouseType::NONE;
	_bm = ButtonMode::DOWN;
	_kt = KeyboardType::VIRTUAL_KEY_CODES;
	_keyCode = 0;
	_jt = JoystickType::NONE;
	_tt = TouchType::NONE;
	_joystickNr = 0;
}

UserInput::~UserInput()
{
}

bool UserInput::CopyChip(Chip *chip)
{
	UserInput *c = dynamic_cast<UserInput*>(chip);
	B_RETURN(Value::CopyChip(c));
	_dt = c->_dt;
	_mt = c->_mt;
	_bm = c->_bm;
	_jt = c->_jt;
	_tt = c->_tt;
	_kt = c->_kt;
	_keyCode = c->_keyCode;
	_joystickNr = c->_joystickNr;
	return true;
}

bool UserInput::LoadChip(DocumentLoader &loader) 
{ 
	B_RETURN(Value::LoadChip(loader));
	LOADDEF(MTEXT("deviceType|dt"), _dt, DeviceType::NONE);
	LOADDEF(MTEXT("mouseType|mt"), _mt, MouseType::NONE);
	LOADDEF(MTEXT("buttonMode|bm"), _bm, ButtonMode::DOWN);
	LOADDEF(MTEXT("keyboardType|kt"), _kt, KeyboardType::VIRTUAL_KEY_CODES);
	LOADDEF(MTEXT("joystickType|jt"), _jt, JoystickType::NONE);
	LOADDEF(MTEXT("touchType|tt"), _tt, TouchType::NONE);
	LOADDEF(MTEXT("keyCode|key"), _keyCode, 0);
	LOADDEF(MTEXT("joystickNr"), _joystickNr, 0);
	return true;
}

bool UserInput::SaveChip(DocumentSaver &saver) const 
{
	B_RETURN(Value::SaveChip(saver));
	SAVEDEF(MTEXT("deviceType"), _dt, DeviceType::NONE);
	SAVEDEF(MTEXT("mouseType"), _mt, MouseType::NONE);
	SAVEDEF(MTEXT("buttonMode"), _bm, ButtonMode::DOWN);
	SAVEDEF(MTEXT("keyboardType"), _kt, KeyboardType::VIRTUAL_KEY_CODES);
	SAVEDEF(MTEXT("joystickType"), _jt, JoystickType::NONE);
	SAVEDEF(MTEXT("touchType"), _tt, TouchType::NONE);
	SAVEDEF(MTEXT("keyCode"), _keyCode, 0);
	SAVEDEF(MTEXT("joystickNr"), _joystickNr, 0);
	return true;
}

value UserInput::GetValue()
{
	RefreshT refresh(Refresh);
	if (refresh) {
		if (engine->GetEditMode() == EditMode::EM_EDIT)
			return _value = 0.0;

		switch (_dt) 
		{
		case DeviceType::MOUSE:
			_value = _getMouseInput();
			break;
		case DeviceType::KEYBOARD:
			_value = _getKeyboardInput();
			break;
		case DeviceType::JOYSTICK:
			_value = _getJoystickInput();
			break;
		case DeviceType::TOUCH:
			_value = _getTouchInput();
			break;
		default:
			_value = 0.0f;
			break;
		};
	}
	return _value;
}

value UserInput::_getMouseInput()
{
	InputManager *im = engine->GetApplication()->GetInputManager();
	if (!im)
		return 0.0;

	value ret = 0.0f;

	switch (_mt) 
	{
	case MouseType::DMOUSE_X:
		ret = (value)im->GetMouseDeltaPos().x;
		break;
	case MouseType::DMOUSE_Y:
		ret = (value)im->GetMouseDeltaPos().y;
		break;
	case MouseType::LMOUSE:
		ret = _parseButtonState(im->GetMouseButton(InputManager::LBUTTON));
		break;
	case MouseType::LMOUSE_DBLCLICK:
		ret = _parseButtonState(im->GetMouseButtonDblClick(InputManager::LBUTTON));
		break;
	case MouseType::MMOUSE:
		ret = _parseButtonState(im->GetMouseButton(InputManager::MBUTTON));
		break;
	case MouseType::MMOUSE_DBLCLICK:
		ret = _parseButtonState(im->GetMouseButtonDblClick(InputManager::MBUTTON));
		break;
	case MouseType::RMOUSE:
		ret = _parseButtonState(im->GetMouseButton(InputManager::RBUTTON));
		break;
	case MouseType::RMOUSE_DBLCLICK:
		ret = _parseButtonState(im->GetMouseButtonDblClick(InputManager::RBUTTON));
		break;
	case MouseType::MOUSE_WHEEL:
		ret = im->GetMouseWheel();
		break;
	case MouseType::CURSOR_GLOBAL_X:
		ret = im->GetMousePos().x;
		break;
	case MouseType::CURSOR_GLOBAL_Y:
		ret = im->GetMousePos().y;
		break;
	case MouseType::CURSOR_X: 
		{
			POINT mp = {(LONG)im->GetMousePos().x, (LONG)im->GetMousePos().y};
			POINT p = engine->GetGraphics()->GetRenderWindowManager()->GetRenderWindow()->GetWindow()->ScreenToClient(mp);
			ret = (value)p.x;
		}
		break;
	case MouseType::CURSOR_Y:
		{
			POINT mp = {(LONG)im->GetMousePos().x, (LONG)im->GetMousePos().y};
			POINT p = engine->GetGraphics()->GetRenderWindowManager()->GetRenderWindow()->GetWindow()->ScreenToClient(mp);
			ret = (value)p.y;
		}
		break;
	case MouseType::CURSOR_REL_X:
		{
			POINT mp = {(LONG)im->GetMousePos().x, (LONG)im->GetMousePos().y};
			POINT p = engine->GetGraphics()->GetRenderWindowManager()->GetRenderWindow()->GetWindow()->ScreenToClient(mp);
			RECT r =  engine->GetGraphics()->GetRenderWindowManager()->GetRenderWindow()->GetWindow()->GetClientRect();
			ret = (value)p.x / (value)(r.right - r.left);
		}
		break;
	case MouseType::CURSOR_REL_Y:
		{
			POINT mp = {(LONG)im->GetMousePos().x, (LONG)im->GetMousePos().y};
			POINT p = engine->GetGraphics()->GetRenderWindowManager()->GetRenderWindow()->GetWindow()->ScreenToClient(mp);
			RECT r =  engine->GetGraphics()->GetRenderWindowManager()->GetRenderWindow()->GetWindow()->GetClientRect();
			ret = (value)p.y / (value)(r.bottom - r.top);
		}
		break;
	default:
		break;
	};
	return ret;
}

value UserInput::_getKeyboardInput()
{
	InputManager *im = engine->GetApplication()->GetInputManager();
	if (!im)
		return 0.0;
	
	value ret = 0.0;

	switch (_kt) 
	{
	case KeyboardType::VIRTUAL_KEY_CODES:
		if (_keyCode > 0)
			ret = _parseButtonState(im->GetVirtualKeyCode(_keyCode));
		break;
	case KeyboardType::SCAN_CODES:
		if (_keyCode > 0)
			ret = _parseButtonState(im->GetKeyboardScanCode(_keyCode));
		break;
	case KeyboardType::ANY_KEY:
		for (uint32 i = 0; i < 256; i++) {
			if ((ret = _parseButtonState(im->GetVirtualKeyCode(i))) == 1.0f)
				break;
		}
		break;
	default:
		break;
	}
	return ret;
}

value UserInput::_getJoystickInput()
{
	InputManager *im = engine->GetApplication()->GetInputManager();
	if (!im)
		return 0.0;

	Joysticks *js = im->GetJoysticks();
	if (!js)
		return 0.0;

	value ret = 0.0;

	switch (_jt)
	{
	case JoystickType::AXIS_1: ret = js->GetState(_joystickNr).axes[0]; break;
	case JoystickType::AXIS_2: ret = js->GetState(_joystickNr).axes[1]; break;
	case JoystickType::AXIS_3: ret = js->GetState(_joystickNr).axes[2]; break;
	case JoystickType::AXIS_4: ret = js->GetState(_joystickNr).axes[3]; break;
	case JoystickType::AXIS_5: ret = js->GetState(_joystickNr).axes[4]; break;
	case JoystickType::HAT_1: ret = js->GetState(_joystickNr).hats[0]; break;
	case JoystickType::HAT_2: ret = js->GetState(_joystickNr).hats[1]; break;
	case JoystickType::HAT_3: ret = js->GetState(_joystickNr).hats[2]; break;
	case JoystickType::HAT_4: ret = js->GetState(_joystickNr).hats[3]; break;
	case JoystickType::BUTTON1: ret = _parseButtonState(js->GetState(_joystickNr).buttons[0]); break;
	case JoystickType::BUTTON2: ret = _parseButtonState(js->GetState(_joystickNr).buttons[1]); break;
	case JoystickType::BUTTON3: ret = _parseButtonState(js->GetState(_joystickNr).buttons[2]); break;
	case JoystickType::BUTTON4: ret = _parseButtonState(js->GetState(_joystickNr).buttons[3]); break;
	case JoystickType::BUTTON5: ret = _parseButtonState(js->GetState(_joystickNr).buttons[4]); break;
	case JoystickType::BUTTON6: ret = _parseButtonState(js->GetState(_joystickNr).buttons[5]); break;
	case JoystickType::BUTTON7: ret = _parseButtonState(js->GetState(_joystickNr).buttons[6]); break;
	case JoystickType::BUTTON8: ret = _parseButtonState(js->GetState(_joystickNr).buttons[7]); break;
	case JoystickType::BUTTON9: ret = _parseButtonState(js->GetState(_joystickNr).buttons[8]); break;
	case JoystickType::BUTTON10: ret = _parseButtonState(js->GetState(_joystickNr).buttons[9]); break;
	case JoystickType::BUTTON11: ret = _parseButtonState(js->GetState(_joystickNr).buttons[10]); break;
	case JoystickType::BUTTON12: ret = _parseButtonState(js->GetState(_joystickNr).buttons[11]); break;
	case JoystickType::BUTTON13: ret = _parseButtonState(js->GetState(_joystickNr).buttons[12]); break;
	case JoystickType::BUTTON14: ret = _parseButtonState(js->GetState(_joystickNr).buttons[13]); break;
	case JoystickType::BUTTON15: ret = _parseButtonState(js->GetState(_joystickNr).buttons[14]); break;
	case JoystickType::BUTTON16: ret = _parseButtonState(js->GetState(_joystickNr).buttons[15]); break;
	case JoystickType::BUTTON17: ret = _parseButtonState(js->GetState(_joystickNr).buttons[16]); break;
	case JoystickType::BUTTON18: ret = _parseButtonState(js->GetState(_joystickNr).buttons[17]); break;
	case JoystickType::BUTTON19: ret = _parseButtonState(js->GetState(_joystickNr).buttons[18]); break;
	case JoystickType::BUTTON20: ret = _parseButtonState(js->GetState(_joystickNr).buttons[19]); break;
	case JoystickType::BUTTON21: ret = _parseButtonState(js->GetState(_joystickNr).buttons[20]); break;
	case JoystickType::BUTTON22: ret = _parseButtonState(js->GetState(_joystickNr).buttons[21]); break;
	case JoystickType::BUTTON23: ret = _parseButtonState(js->GetState(_joystickNr).buttons[22]); break;
	case JoystickType::BUTTON24: ret = _parseButtonState(js->GetState(_joystickNr).buttons[23]); break;
	case JoystickType::BUTTON25: ret = _parseButtonState(js->GetState(_joystickNr).buttons[24]); break;
	case JoystickType::BUTTON26: ret = _parseButtonState(js->GetState(_joystickNr).buttons[25]); break;
	case JoystickType::BUTTON27: ret = _parseButtonState(js->GetState(_joystickNr).buttons[26]); break;
	case JoystickType::BUTTON28: ret = _parseButtonState(js->GetState(_joystickNr).buttons[27]); break;
	case JoystickType::BUTTON29: ret = _parseButtonState(js->GetState(_joystickNr).buttons[28]); break;
	case JoystickType::BUTTON30: ret = _parseButtonState(js->GetState(_joystickNr).buttons[29]); break;
	case JoystickType::BUTTON31: ret = _parseButtonState(js->GetState(_joystickNr).buttons[30]); break;
	case JoystickType::BUTTON32: ret = _parseButtonState(js->GetState(_joystickNr).buttons[31]); break;
	default:
		break;
	}
	return ret;
}

value UserInput::_getTouchInput()
{
	InputManager *im = engine->GetApplication()->GetInputManager();
	if (!im)
		return 0.0;

	value ret = 0.0;

	switch (_tt) 
	{
	case TouchType::COUNT: ret = im->GetTouchCount(); break;
	case TouchType::TOUCH0_STATE: ret = _parseButtonState(im->GetTouchPoint(0).state); break;
	case TouchType::TOUCH0_ID: ret = im->GetTouchPoint(0).uniqueID; break;
	case TouchType::TOUCH0_X: ret = im->GetTouchPoint(0).pos.x; break;
	case TouchType::TOUCH0_Y: ret = im->GetTouchPoint(0).pos.y; break; 
	case TouchType::TOUCH0_DX: ret = im->GetTouchPoint(0).dpos.x; break;
	case TouchType::TOUCH0_DY: ret = im->GetTouchPoint(0).dpos.y; break;
	case TouchType::TOUCH1_STATE: ret = _parseButtonState(im->GetTouchPoint(1).state); break;
	case TouchType::TOUCH1_ID: ret = im->GetTouchPoint(1).uniqueID; break;
	case TouchType::TOUCH1_X: ret = im->GetTouchPoint(1).pos.x; break;
	case TouchType::TOUCH1_Y: ret = im->GetTouchPoint(1).pos.y; break; 
	case TouchType::TOUCH1_DX: ret = im->GetTouchPoint(1).dpos.x; break;
	case TouchType::TOUCH1_DY: ret = im->GetTouchPoint(1).dpos.y; break;
	case TouchType::TOUCH2_STATE: ret = _parseButtonState(im->GetTouchPoint(2).state); break;
	case TouchType::TOUCH2_ID: ret = im->GetTouchPoint(2).uniqueID; break;
	case TouchType::TOUCH2_X: ret = im->GetTouchPoint(2).pos.x; break;
	case TouchType::TOUCH2_Y: ret = im->GetTouchPoint(2).pos.y; break; 
	case TouchType::TOUCH2_DX: ret = im->GetTouchPoint(2).dpos.x; break;
	case TouchType::TOUCH2_DY: ret = im->GetTouchPoint(2).dpos.y; break;
	case TouchType::TOUCH3_STATE: ret = _parseButtonState(im->GetTouchPoint(3).state); break;
	case TouchType::TOUCH3_ID: ret = im->GetTouchPoint(3).uniqueID; break;
	case TouchType::TOUCH3_X: ret = im->GetTouchPoint(3).pos.x; break;
	case TouchType::TOUCH3_Y: ret = im->GetTouchPoint(3).pos.y; break; 
	case TouchType::TOUCH3_DX: ret = im->GetTouchPoint(3).dpos.x; break;
	case TouchType::TOUCH3_DY: ret = im->GetTouchPoint(3).dpos.y; break;
	default:
		break;
	};

	return ret;
}

value UserInput::_parseButtonState(TriState s)
{
	switch (_bm)
	{
	case ButtonMode::UP:
		return s.isActive ? 0.0f : 1.0f;
	case ButtonMode::PRESSING:
		return s.onActivate ? 1.0f : 0.0f;
	case ButtonMode::DOWN:
		return s.isActive ? 1.0f : 0.0f;
	case ButtonMode::RELEASING:
		return s.onDeactivate ? 1.0f : 0.0f;
	default:;
	}
	return 0.0f;
}
