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
#include "ClassView.h"
#include "DocumentExtSaver.h"
#include "DocumentExtLoader.h"
#include "StdChips/FunctionCall.h"
#include "StdChips/ClassDiagramChip.h"
#include "StdChips/Shortcut.h"
#include "M3DEngine/Function.h"
#include "M3DEngine/InputManager.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ChipManager.h"
#include "M3DEngine/DocumentSaver.h"
#include "M3DEngine/DocumentLoader.h"
#include "M3DEngine/DocumentFileTypes.h"
#include <memory>

using namespace m3d;

void ClassInfoBox::Update(InputManager *im, ClassView *view, float32 left, float32 top)
{
	Vector2 oldMousePos = mousePosVP;
	mousePosVP = Vector2((FLOAT)im->GetMousePos().x-left, (FLOAT)im->GetMousePos().y-top);
	if (mousePosVP != oldMousePos)
		mouseMoveTime = clock();

	if (mousePosVP.x >= 0.0f && mousePosVP.x < view->GetWidth() && mousePosVP.y >= 0.0f && mousePosVP.y < view->GetHeight() && (view->GetDragMode() != DM_DRAGGING && view->GetDragMode() != DM_SIZING) && (view->GetHoverMode() == HM_CHIP || view->GetHoverMode() == HM_TOPCONN || view->GetHoverMode() == HM_CHILDCONN)) {
		if (show && (hm != view->GetHoverMode() || hoverCh != view->GetHoverChip() || (hm == HM_CHILDCONN && hoverConn != view->GetHoverConn())) || !show && ((float32)(clock() - mouseMoveTime) / (float32)CLOCKS_PER_SEC) > 0.25f) {
			show = true;
			mousePosVPInfo = mousePosVP;
			hm = view->GetHoverMode();
			hoverCh = view->GetHoverChip();
			hoverConn = view->GetHoverConn();
		}
	}
	else
		show = false;
}

ClassView::ClassView(ClassExt *cg) : clazz(cg), w(0.0f), h(0.0f), ar(0.0f), mousePos(0.0f, 0.0f), zoom(20.0f), camPos(0.0f, 0.0f), selectionStart(0.0f, 0.0f), folder(MainFolderID),
	hm(HM_NONE), hoverCh(0), hoverConn(0), hoverFolder(0), hoverLink(0), dm(DM_NONE), dragGuid(0), connDragChip(0), dragConn(0), dragSelected(false), connSwap(false), _dScaling(0.0f, 0.0f)
{
} 

ClassView::~ClassView()
{
}

void ClassView::FocusOnChip(ChipID cid, bool centerFocus)
{
	_clearSelection();
	Chip *chip = clazz->GetChip(cid);
	if (chip) {
		sChips.insert(cid);
		if (centerFocus || !_isChipVisible(chip)) { // TODO: may change the calc. to determind if the chip is visible.
			folder = chip->GetChipEditorData()->folder;
			camPos = chip->GetChipEditorData()->pos;
		}
	}
}

void ClassView::PreRenderProcssing(InputManager *im, float32 width, float32 height, float32 left, float32 top)
{
	vChips.clear();
	vFolders.clear();
	vLinks.clear();
	vILinks.clear();
	vBackgroundItems.clear();
	dragStarts.clear();
	hm = HM_NONE;

	const FolderList &folders = clazz->GetFolderList();
	const ChipPtrByChipIDMap &chipMap = clazz->GetChips();

	bool CTRL = im->GetVirtualKeyCodeState(VK_CONTROL);
	bool SHIFT = im->GetVirtualKeyCodeState(VK_SHIFT);
	bool LALT = im->GetVirtualKeyCodeState(VK_MENU);

	// Check if current folder is valid
	if (!clazz->IsFolderValid(folder)) {
		assert(folder != MainFolderID);
		folder = MainFolderID;
		camPos = Vector2(0.0f, 0.0f);
	}

	w = width;
	h = height;
	ar = w / h;

	// Zoom
	if (!_isCursorOutsideView()) {
		float32 oldZoom = zoom;
		float32 f = zoom - (float32)im->GetMouseWheel() * zoom / WHEEL_SENSITIVITY;

		bool rbuttonZoom = im->GetMouseButton(InputManager::RBUTTON).isActive && LALT && dm == DM_NONE;
		if (rbuttonZoom)
			f += (float32)im->GetMouseDeltaPos().y * zoom * 0.01f;

		zoom = std::min(std::max(f, MIN_ZOOM), MAX_ZOOM);

		float32 m = zoom / oldZoom;

		if (!rbuttonZoom && m != 1.0f) { // Move camera when zooming.
			Vector2 mp = Vector2(((float32)(im->GetMousePos().x - left) / w - 0.5f) * oldZoom * ar, ((1.0f - (float32)(im->GetMousePos().y - top) / h) - 0.5f) * oldZoom) + camPos;
			camPos = mp - (mp - camPos) * m;
		}		
	}

	// Update mouse pos
	mousePos = Vector2(((float32)(im->GetMousePos().x - left) / w - 0.5f) * zoom * ar, ((1.0f - (float32)(im->GetMousePos().y - top) / h) - 0.5f) * zoom) + camPos;

	// Camera Position
	if (im->GetMouseButton(InputManager::MBUTTON).isActive || im->GetMouseButton(InputManager::RBUTTON).isActive) {
		float32 dx = zoom * ar / w * (float32)im->GetMouseDeltaPos().x;
		float32 dy = zoom / h * (float32)im->GetMouseDeltaPos().y;
		camPos += Vector2(-dx, dy);
		camPos.x = std::min(std::max(camPos.x, -MAX_MOVE), MAX_MOVE);
		camPos.y = std::min(std::max(camPos.y, -MAX_MOVE), MAX_MOVE);
	}

	bool allowEdit = engine->GetEditMode() != EditMode::EM_RUN;

	bool N = allowEdit && im->GetVirtualKeyCode('N').onActivate; // Is 'N' being pressed?
	bool P = allowEdit && im->GetVirtualKeyCode('P').onActivate; // Is 'P' being pressed?

	if (!(CTRL || SHIFT) && (N || P)) {
		// Goto next/prev shortcut if ONLY one chip is selected!
		if (sChips.size() == 1) {
			Chip *chip = clazz->GetChip(*sChips.rbegin());
			Shortcut *shortcut = chip->AsShortcut();
			if (shortcut)
				chip = shortcut->GetOriginal();
			if (chip) {
				bool next = shortcut == nullptr;
				const Set<DestructionObserver*> &obs = chip->GetDestructionObservers();
				if (N)
				{
					for (auto n = obs.begin(); n != obs.end(); n++) {
						Shortcut* sc = dynamic_cast<Shortcut*>(*n);
						if (sc) {
							if (next) {
								chip = sc;
								break;
							}
							else if (shortcut && sc == shortcut)
								next = true;
						}
					}
				}
				else
				{
					for (auto n = obs.rbegin(); n != obs.rend(); n++) {
						Shortcut* sc = dynamic_cast<Shortcut*>(*n);
						if (sc) {
							if (next) {
								chip = sc;
								break;
							}
							else if (shortcut && sc == shortcut)
								next = true;
						}
					}
				}
				FocusOnChip(chip->GetID(), false);
			}
		}
	}

	// For each folder, find the nearest visible folder seen from current folder.
	for (FolderID i = 0; i < folders.size(); i++)
		folders[i].nearFolder = clazz->GetClosestFolder((FolderID)i, folder);

	// Find all visible folders and add them to the list to be rendered.
	for (FolderID i = 1; i < folders.size(); i++) { // Start at 1: Do not include main "folder" (0)
		if (folders[i].parentFolder == folder || folder == i) {
			vFolders.push_back(i);
			folders[i].selected = sFolders.find(i) != sFolders.end();
			folders[i].hover = false;
		}
	}

	// Find hovering folder
	{
		size_t hf = InvalidFolderID;
		for (size_t i = vFolders.size(); i > 0; i--) { // Iterate reverse order for z-order to be correct!
			const Folder &f = folders[vFolders[i - 1]];
			Vector2 coord = mousePos - (f.parentFolder == folder ? f.pos : f.inPos);
			// Is mouse over folder?
			if (abs(coord.x) < CHIP_HALF_HEIGHT && abs(coord.y) < CHIP_HALF_HEIGHT) {
				hf = i - 1;
				if (!(f.selected && dm == DM_DRAGGING)) // This trix is to make sure we can drag a folder into any other folders!
					break;
			}
		}
		if (hf != InvalidFolderID) {
			hm = HM_FOLDER;
			hoverFolder = vFolders[hf];
			folders[vFolders[hf]].hover = true;
		}
	}

	// Iterate chips to find visible chips!
	for (const auto &n : chipMap) {
		ChipEditorData *ced = n.second->GetChipEditorData();
		if (ced->folder == folder) { // chip in same folder?
			// Find number of connections and width
			ced->cCount = ced->GetNumberOfChildConnections();
			ced->width = CHIP_HEIGHT + CONNECTION_WIDTH * (float32)ced->cCount;
			bool selected = sChips.find(n.first) != sChips.end();
			// Include if visible or selected: We need to include selected chips, because else we run into trouble when dragging chips out of view (becuase we cull non-visible chips!)
			if (_isChipVisible(n.second) || selected) {
				ced->hm = HM_NONE;
				ced->selected = selected;
				vChips.push_back(n.second);
			}
		}
	}

	// Find hovering chip (which of the visible chips the cursor is currently over)
	// We have to do this in reverse order for z-order to be correct!
	if (hm == HM_NONE) {
		for (size_t i = vChips.size(); i > 0; i--) {
			ChipEditorData *ced = vChips[i - 1]->GetChipEditorData();
			Vector2 chCoord = mousePos - ced->pos;
			// Is mouse over chip?
			if (abs(chCoord.x) < (ced->width * 0.5f) && abs(chCoord.y) < CONNECTION_YPOS) {
				// Over Chip?
				if (abs(chCoord.y) < CHIP_HALF_HEIGHT) {
					hm = ced->hm = HM_CHIP;
					hoverCh = ced->chip->GetID();
					break;
				}
				// Over Top Connection?
				if (chCoord.y > 0.0f && abs(chCoord.x) < (CONNECTION_WIDTH * 0.5f)) {
					hm = ced->hm = HM_TOPCONN;
					hoverCh = ced->chip->GetID();
					break;
				}
				// Over SubConnection Connection?
				if (chCoord.y < 0.0f && abs(chCoord.x) < (ced->width * 0.5f - CHIP_HALF_HEIGHT)) {
					hm = ced->hm = HM_CHILDCONN;
					hoverCh = ced->chip->GetID();
					hoverConn = (unsigned)((float32)ced->cCount * ((2.0f * chCoord.x + ced->width - CHIP_HEIGHT) / (2.0f * ced->width - 2.0f * CHIP_HEIGHT)));
					break;
				}
			}
		}
	}

	Chip *hoverChip = (hm == HM_CHIP || hm == HM_TOPCONN || hm == HM_CHILDCONN) ? clazz->GetChip(hoverCh) : nullptr; // The chip the cursor is currently over

	// Iterate chips to find all visible links.
	for (const auto &m : chipMap) {
		Chip *chip = m.second;

		for (unsigned i = 0, h = 0; i < chip->GetChildren().size(); i++, h++) { // Iterate child connections
			ChildConnection *cc = chip->GetChildren()[i];
			if (!cc) // Empty connection?
				continue;
			for (unsigned j = 0; j < cc->connections.size(); j++, h += (cc->desc.connType == ChildConnectionDesc::GROWING ? 1 : 0)) { // Iterate children
				const SubConnection &sc = cc->connections[j];
				Chip *child = sc.chip;
				if (!child) 
					continue;

				if (child->GetClass() != clazz)
					continue; // Has the chip been linked to a chip in another class? That's not good!

				ChipEditorData *cv = child->GetChipEditorData();
				
				Vector2 a, b;

				// Is the mouse cursor over a chip and we are processing a belonging shortcut or vice versa, we have to draw a (blue) link from the main chip to the current child connection.
				bool drawHoverLink = hm == HM_CHIP && child->AsShortcut() && (hoverChip == child || child->AsShortcut()->GetOriginal() == hoverChip);

				ChipEditorData *ced = chip->GetChipEditorData();

				if (ced->folder == folder) // Our chip is in the same folder? So is all child connections!
					a = ced->pos - Vector2(ced->width * 0.5f - CHIP_HALF_HEIGHT - CONNECTION_WIDTH * 0.5f - CONNECTION_WIDTH * h, CONNECTION_YPOS); // find start pos!
				else {
					if (folders[ced->folder].nearFolder == folders[cv->folder].nearFolder)	// && !drawHoverLink)
						continue; // Connections not visible!								(drawHoverLink overrides it!)?????
					// Find start pos! It is in a different folder!
					if (folders[ced->folder].nearFolder == folders[folder].parentFolder) // Is the chip in a parent folder?
						a = folders[folder].inPos - Vector2(0.0f, CONNECTION_YPOS);
					else // Then it must be in a sub folder!
						a = folders[folders[ced->folder].nearFolder].pos - Vector2(0.0f, CONNECTION_YPOS);
				}
				
				if (cv->folder == folder) // Is child in the same folder?
					b = cv->pos + Vector2(0.0f, CONNECTION_YPOS);
				else if (folders[cv->folder].nearFolder == folders[folder].parentFolder) // Is the chip in a parent folder?
					b = folders[folder].inPos + Vector2(0.0f, CONNECTION_YPOS);
				else // Then it must be in a sub folder!
					b = folders[folders[cv->folder].nearFolder].pos + Vector2(0.0f, CONNECTION_YPOS);

				// CANDO: It may be possible to add a simple culling here!
				Link link = { ChipConnectionID(chip->GetID(), ConnectionID(i, j)), a, b, cc->desc.dataDirection, false, sLinks.find(link.id) != sLinks.end(), sc.lastHit };
				vLinks.push_back(link);

				// TODO: something wrong with blue link from differnet folders... ????

				if (drawHoverLink) {
					Vector2 c;
					ced = hoverChip->GetChipEditorData();
					if (hoverChip->AsShortcut())
						ced = hoverChip->AsShortcut()->GetOriginal()->GetChipEditorData();
					if (ced->folder == folder) // Is child in the same folder?
						c = ced->pos + Vector2(0.0f, CONNECTION_YPOS);
					else if (folders[ced->folder].nearFolder == folders[folder].parentFolder) // Is the chip in a parent folder?
						c = folders[folder].inPos + Vector2(0.0f, CONNECTION_YPOS);
					else // Then it must be in a sub folder!
						c = folders[folders[ced->folder].nearFolder].pos + Vector2(0.0f, CONNECTION_YPOS);
					ILink il = {a, c, cc->desc.dataDirection};
					vILinks.push_back(il);
				}
			}
		}
	}

	// Find hovering link (if we not hover over any other item)
	if (hm == HM_NONE) {
		if (VERTICES_PR_LINK == 2) {
			for (unsigned i = 0; i < vLinks.size(); i++) {
				Link &c = vLinks[i];
				Vector2 ba = c.b - c.a;
				Vector2 pa = mousePos - c.a;
				float32 u = ba.Dot(pa) / ba.Dot(ba);
				if (u < 0.0f || u > 1.0f)
					continue;
				Vector2 x = mousePos - (c.a + u * ba);
				x.x *= w / (zoom * ar);
				x.y *= h / zoom;
				if (x.LengthSquared() < LINK_MOUSE_LIMIT_SQ) {
					hm = HM_LINK;
					hoverLink = i;
					c.hover = true;
					break;
				}
			}
		}
		else {
			static const Matrix H = Matrix(2, -3, 0, 1, -2, 3, 0, 0, 1, -2, 1, 0, 1, -1, 0, 0).Transpose();
			const float32 ws = w / (zoom * ar), hs = h / zoom, lml = sqrt(LINK_MOUSE_LIMIT_SQ);
			for (unsigned i = 0; i < vLinks.size(); i++) {
				Link &c = vLinks[i];
				if ((std::min(c.a.x, c.b.x) - mousePos.x) * ws > lml || (mousePos.x - std::max(c.a.x, c.b.x)) * ws > lml)
					continue;
				float32 yTangent = -(fabs(c.a.y - c.b.y) + log(fabs(c.a.x - c.b.x) + 1.0f));
				Vector2 a = c.a;
				Matrix m = Matrix(c.a.x, c.a.y, 0.0f, 0.0f, c.b.x, c.b.y, 0.0f, 0.0f, 0.0f, yTangent, 0.0f, 0.0f, 0.0f, yTangent, 0.0f, 0.0f);
				for (unsigned j = 0, J = VERTICES_PR_LINK / 2; j < J; j++) {
					float32 t = float32(j) / (J - 1);
					Vector4 b4 = Vector4::Transform(Vector4::Transform(Vector4(t*t*t, t*t, t, 1.0f), H), m);
					Vector2 b = Vector2(b4.x, b4.y);

					Vector2 ba = b - a;
					Vector2 pa = mousePos - a;
					float32 u = ba.Dot(pa) / ba.Dot(ba);
					if (u >= 0.0f && u <= 1.0f) {
						Vector2 x = mousePos - (a + u * ba);
						x.x *= ws;
						x.y *= hs;
						if (x.LengthSquared() < LINK_MOUSE_LIMIT_SQ) {
							hm = HM_LINK;
							hoverLink = i;
							c.hover = true;
							break;
						}
					}
					a = b;
				}
				if (hm == HM_LINK)
					break;
			}
		}
	}

	// Iterate background items to visibles
	for (auto n = clazz->GetBackgroundItems().begin(); n != clazz->GetBackgroundItems().end(); n++) {
		if (n->second.folder == folder) { // chip in same folder?
			bool selected = sBackgroundItems.find(n->first) != sBackgroundItems.end();
			// Include if visible or selected: We need to include selected items, because else we run into trouble when dragging chips out of view (becuase we cull non-visible items!)
			if (selected || (abs(n->second.pos.x - camPos.x) < (zoom * ar * 0.5f + n->second.size.x * 0.5f) && abs(n->second.pos.y - camPos.y) < (zoom * 0.5f + n->second.size.y * 0.5f))) {
				n->second.hover = false;
				n->second.selected = selected;
				vBackgroundItems.push_back(n);
			}
		}
	}

	// Find hovering background item (if we not hover over any other item)
	if (hm == HM_NONE) {
		for (size_t i = vBackgroundItems.size(); i > 0; i--) {
			auto n = vBackgroundItems[i - 1];

			float32 xd = abs(n->second.pos.x + n->second.size.x * 0.5f - BACKGROUNDITEM_HANDLE_SIZE * 0.5f - mousePos.x);
			float32 yd = abs(n->second.pos.y - n->second.size.y * 0.5f + BACKGROUNDITEM_HANDLE_SIZE * 0.5f - mousePos.y);
			
			n->second.showHandles = xd*xd+yd*yd<4.0f*BACKGROUNDITEM_HANDLE_SIZE*BACKGROUNDITEM_HANDLE_SIZE; // Mouse near handle? Then we can display them!
			
			// Is mouse over item scaling handle on lower right corner?
			if (xd < BACKGROUNDITEM_HANDLE_SIZE * 0.5f && yd < BACKGROUNDITEM_HANDLE_SIZE * 0.5f) {
				hm = HM_BGITEM_SCALE;
				n->second.hover = true;
				hoverBackgroundItem = n->first;
				break;
			}
			else {
				xd = abs(n->second.pos.x - n->second.size.x * 0.5f + BACKGROUNDITEM_HANDLE_SIZE * 0.5f - mousePos.x);
				yd = abs(n->second.pos.y + n->second.size.y * 0.5f - BACKGROUNDITEM_HANDLE_SIZE * 0.5f - mousePos.y);
				if (!n->second.showHandles)
					n->second.showHandles = xd*xd+yd*yd<4.0f*BACKGROUNDITEM_HANDLE_SIZE*BACKGROUNDITEM_HANDLE_SIZE; // Mouse near handle? Then we can display them!
				// Is mouse over item handle on top left corner, OR if selected, somewhere else over the item?
				if (n->second.selected && abs(n->second.pos.x - mousePos.x) < n->second.size.x * 0.5f && abs(n->second.pos.y - mousePos.y) < n->second.size.y * 0.5f || xd < BACKGROUNDITEM_HANDLE_SIZE * 0.5f && yd < BACKGROUNDITEM_HANDLE_SIZE * 0.5f) {
					hm = HM_BGITEM;
					n->second.hover = true;
					hoverBackgroundItem = n->first;
					break;
				}
			}
		}
	}

	//===================

	bool L = allowEdit && im->GetVirtualKeyCodeState('L'); // Is 'L' pressed?
	bool K = allowEdit && im->GetVirtualKeyCodeState('K'); // Is 'K' pressed?
	bool D = allowEdit && im->GetVirtualKeyCodeState('D'); // Is 'D' pressed?
	bool A = allowEdit && im->GetVirtualKeyCodeState('A'); // Is 'A' pressed?

	bool pressingLeft = (im->GetMouseButton(InputManager::LBUTTON).onActivate && !im->GetMouseButton(InputManager::RBUTTON).isActive);

	// Pressing Left?
	if (allowEdit && !LALT && pressingLeft) {
		switch (hm) {
			case HM_NONE:
			{
				// Start selecting (rubberbanding)
				dm = DM_SELECT;
				selectionStart = mousePos;

				rChips.clear();
				rLinks.clear();
				rFolders.clear();
				rBackgroundItems.clear();

				if (!CTRL)
					_clearSelection();

				break;
			}
			case HM_CHIP:
			{
				if (!(K || L || D || A || CTRL)) {
					if (hoverChip) {
						_selectChip(hoverChip, false, SHIFT);
						if (hoverChip->GetChipEditorData()->selected) // Only dragging on left button
							dm = DM_DRAGGING;
					}
				}
				break;
			}
			case HM_TOPCONN:
			{
				if (!(K || L || D || A)) {
					// Drag a link from ALL selected chips + the hovered chip!
					if (hoverChip) {
						dm = DM_TOPCONN;
						dragGuid = hoverChip->GetChipTypeIndex();
						connDragChip = hoverCh;
						dragSelected = sChips.find(hoverCh) != sChips.end();
						connSwap = false;
					}
				}
				break;
			}
			case HM_CHILDCONN:
			{
				if (!(K || L || D || A)) {
					if (hoverChip) {
						if (CTRL || SHIFT) {
							// Drag a link from the top connection this child connection is connected to (if it exist!)
							ConnectionID cid = hoverChip->GetChipEditorData()->GetConnectionID(hoverConn);
							ChipID childID = hoverChip->GetChipEditorData()->GetChild(cid);
							Chip *child = clazz->GetChip(childID);
							if (child) {
								dm = DM_TOPCONN;
								dragGuid = child->GetChipTypeIndex();
								connDragChip = childID;
								dragSelected = false;
								connSwap = SHIFT;
								connSwapConnection = ChipConnectionID(hoverCh, cid);
							}
						}
						else {
							// Drag a link from the hovered child connection!
							dm = DM_CHILDCONN;
							dragGuid = -1;
							connDragChip = hoverCh;
							dragConn = hoverConn;
							ConnectionID cid = hoverChip->GetChipEditorData()->GetConnectionID(hoverConn);
							if (cid != InvalidConnectionID) {
								ChildConnection *cc = hoverChip->GetChildren()[cid.first];
								dragGuid = cc ? cc->chipTypeIndex : -1;
							}
						}
					}
				}
				break;
			}
			case HM_LINK:
			{
				if (!(K || L || D || A)) {
					Link &l = vLinks[hoverLink];
					if (l.selected) {
						if (CTRL) {
							l.selected = false;
							sLinks.erase(l.id);
						}
					}
					else {
						if (!CTRL)
							_clearSelection();
						l.selected = true;
						sLinks.insert(l.id);
					}
				}
				break;
			}
			case HM_FOLDER:
			{
				if (!(K || L || D || A || CTRL)) {
					const Folder &f = folders[hoverFolder];
					if (f.selected) {
						dm = DM_DRAGGING;
					}
					else {
						_clearSelection();
						f.selected = true;
						sFolders.insert(hoverFolder);
						dm = DM_DRAGGING;
					}				
				}
				break;
			}
			case HM_BGITEM:
			{
				if (!(K || L || D || A || CTRL)) {
					auto n = clazz->GetBackgroundItems().find(hoverBackgroundItem);
					if (n != clazz->GetBackgroundItems().end()) {
						if (n->second.selected) {
							dm = DM_DRAGGING;
						}
						else {
							_clearSelection();
							n->second.selected = true;
							sBackgroundItems.insert(hoverBackgroundItem);
							dm = DM_DRAGGING;
						}
					}
				}
				break;
			}
			case HM_BGITEM_SCALE:
			{
				if (!(K || L || D || A || CTRL)) {
					auto n = clazz->GetBackgroundItems().find(hoverBackgroundItem);
					if (n != clazz->GetBackgroundItems().end()) {
						_clearSelection();
						n->second.selected = true;
						sBackgroundItems.insert(hoverBackgroundItem);
						dm = DM_SIZING;
						_dScaling = Vector2(mousePos.x - n->second.size.x, mousePos.y + n->second.size.y);
					}
				}
				break;
			}
			default:
				break;
		}
	}

	// Left Pressed
	if (allowEdit && im->GetMouseButton(InputManager::LBUTTON).isActive) {
		if (dm == DM_SELECT) { // Are we in selection mode?
			Vector2 a(std::min(selectionStart.x, mousePos.x), std::min(selectionStart.y, mousePos.y));
			Vector2 b(std::max(selectionStart.x, mousePos.x), std::max(selectionStart.y, mousePos.y));

			// Iterate chips
			for (unsigned i = 0; i < vChips.size(); i++) {
				ChipEditorData *ced = vChips[i]->GetChipEditorData();
				Vector2 p = ced->pos;
				bool inside = p.x > a.x && p.x < b.x && p.y > a.y && p.y < b.y;
				ChipID id = ced->chip->GetID();
				if (inside && rChips.insert(id).second || !inside && rChips.erase(id)) { // Inside AND new OR Outside AND existing?
					if (CTRL && ced->selected || !CTRL && !inside) { // Selected AND inverting OR not inverting AND Outside => Unselect!
						ced->selected = false;
						sChips.erase(id);
					}
					else if (CTRL && !ced->selected || !CTRL && inside) { // Unselected AND inverting OR not inverting AND Inside => Select!
						ced->selected = true;
						sChips.insert(id);
					}
				}
			}

			// Iterate links
			for (unsigned i = 0; i < vLinks.size(); i++) {
				Link &l = vLinks[i];
				Vector2 p = (l.a + l.b) * 0.5f;

				bool inside = p.x > a.x && p.x < b.x && p.y > a.y && p.y < b.y;

				if (inside && rLinks.insert(l.id).second || !inside && rLinks.erase(l.id)) {
					if (CTRL && l.selected || !CTRL && !inside) {
						l.selected = false;
						sLinks.erase(l.id);
					}
					else if (CTRL && !l.selected || !CTRL && inside) {
						l.selected = true;
						sLinks.insert(l.id);
					}
				}
			}

			// Iterate folders
			for (unsigned i = 0; i < vFolders.size(); i++) {
				const Folder &f = folders[vFolders[i]];
				Vector2 p = (f.parentFolder == folder ? f.pos : f.inPos);

				bool inside = p.x > a.x && p.x < b.x && p.y > a.y && p.y < b.y;

				if (inside && rFolders.insert(vFolders[i]).second || !inside && rFolders.erase(vFolders[i])) {
					if (CTRL && f.selected || !CTRL && !inside) {
						f.selected = false;
						sFolders.erase(vFolders[i]);
					}
					else if (CTRL && !f.selected || !CTRL && inside) {
						f.selected = true;
						sFolders.insert(vFolders[i]);
					}
				}
			}
			
			// Iterate background Items
			for (size_t i = 0; i < vBackgroundItems.size(); i++) {
				Vector2 A = vBackgroundItems[i]->second.pos - vBackgroundItems[i]->second.size * 0.5f;
				Vector2 B = vBackgroundItems[i]->second.pos + vBackgroundItems[i]->second.size * 0.5f;
				bool inside = A.x > a.x && B.x < b.x && A.y > a.y && B.y < b.y;
				
				if (inside && rBackgroundItems.insert(vBackgroundItems[i]->first).second || !inside && rBackgroundItems.erase(vBackgroundItems[i]->first)) {
					if (CTRL && vBackgroundItems[i]->second.selected || !CTRL && !inside) {
						vBackgroundItems[i]->second.selected = false;
						sBackgroundItems.erase(vBackgroundItems[i]->first);
					}
					else if (CTRL && !vBackgroundItems[i]->second.selected || !CTRL && inside) {
						vBackgroundItems[i]->second.selected = true;
						sBackgroundItems.insert(vBackgroundItems[i]->first);
					}
				}
			}
		}
	}

	// Left released
	if (allowEdit && im->GetMouseButton(InputManager::LBUTTON).onDeactivate) {
		if (hm == HM_CHIP) { 
			if (!(K || L || D || A) && dm == DM_NONE && CTRL) {
				if (hoverChip)
					_selectChip(hoverChip, true, SHIFT);
			}
		}
		else if (hm == HM_FOLDER) {
			if (dm == DM_NONE && CTRL) {
				const Folder &f = folders[hoverFolder];
				if (f.selected) {
					f.selected = false;
					sFolders.erase(hoverFolder);
				}
				else {
					f.selected = true;
					sFolders.insert(hoverFolder);
				}				
			}
		}
		else if (hm == HM_BGITEM) {
			if (dm == DM_NONE && CTRL) {
				auto n = clazz->GetBackgroundItems().find(hoverBackgroundItem);
				if (n != clazz->GetBackgroundItems().end()) {
					if (n->second.selected) {
						n->second.selected = false;
						sBackgroundItems.erase(hoverBackgroundItem);
					}
					else {
						n->second.selected = true;
						sBackgroundItems.insert(hoverBackgroundItem);
					}
				}				
			}
		}
	}


	//===================

	if (dm == DM_TOPCONN) { // Are we dragging from a chip's top connection
		if (dragSelected) { // Dragging from all selected chips?
			// Drag from ALL selected chips
			for (const auto &n : sChips) {
				Chip *chip = clazz->GetChip(n);
				if (chip)
					dragStarts.push_back(chip->GetChipEditorData()->pos + Vector2(0.0f, CONNECTION_YPOS));
			}
		}
		else {
			// Drag ONLY from the chip we started to drag from.
			Chip *chip = clazz->GetChip(connDragChip);
			if (chip)
				dragStarts.push_back(chip->GetChipEditorData()->pos + Vector2(0.0f, CONNECTION_YPOS));
		}
	}
	else if (dm == DM_CHILDCONN) { // Dragging a link from a child connection?
		Chip *chip = clazz->GetChip(connDragChip);
		if (chip)
			dragStarts.push_back(chip->GetChipEditorData()->pos + Vector2(-chip->GetChipEditorData()->width * 0.5f + CHIP_HALF_HEIGHT + CONNECTION_WIDTH * 0.5f + CONNECTION_WIDTH * dragConn, -CONNECTION_YPOS));
	}

	infoBox.Update(im, this, left, top);
}

void ClassView::PostRenderProcssing(InputManager *im, ClassViewCallback *cb)
{
	// [X] Hold L and press Left on a chip: Link all selected chips as children.
	// [X] Hold K and press Left on a chip: Do a RemoveEmptyChildConnections() on the chip
	// [X] Hold D and press Left on a chip/link: Remove the chip/link
	// [X] Hold A and press Left on a chip: Ack. message(s).
	// [X] Press N with 1 selected chip: goto next shortcut
	// [X] Press P with 1 selected chip: goto prev shortcut
	// [X] Press S: create shortcut of all selected chips. 
	// [X] Press Space: Create a new folder with all selected items in it OR expand folder.
	// [X] Press Delete: Remove all selected items. If SHIFT not pressed, show a confirmation message.
	// [X] Press Ctrl+R: Toggle "recycling" mode for all selected chips.
	// [X] Press Ctrl+C: Copy all selected items.
	// [X] Press Ctrl+X: Cut all selected items. Show confirmation message.
	// [X] Press Ctrl+V: Paste.

	const FolderList &folders = clazz->GetFolderList();
	const ChipPtrByChipIDMap &chipMap = clazz->GetChips();

	bool allowEdit = engine->GetEditMode() != EditMode::EM_RUN;

	bool SHIFT = im->GetVirtualKeyCodeState(VK_SHIFT);
	bool CTRL = im->GetVirtualKeyCodeState(VK_CONTROL);
	bool LALT = im->GetVirtualKeyCodeState(VK_MENU);
	bool L = allowEdit && im->GetVirtualKeyCodeState('L'); // Is 'L' pressed?
	bool K = allowEdit && im->GetVirtualKeyCodeState('K'); // Is 'K' pressed?
	bool D = allowEdit && im->GetVirtualKeyCodeState('D'); // Is 'D' pressed?
	bool A = allowEdit && im->GetVirtualKeyCodeState('A'); // Is 'A' pressed?
	bool S = allowEdit && im->GetVirtualKeyCode('S').onActivate; // Is 'S' being pressed?
	bool R = allowEdit && im->GetVirtualKeyCode('R').onActivate; // Is 'R' being pressed?
	bool SPACE = allowEdit && im->GetVirtualKeyCode(VK_SPACE).onActivate; // Is 'space' being pressed?

	Chip *hoverChip = (hm == HM_CHIP || hm == HM_TOPCONN || hm == HM_CHILDCONN) ? clazz->GetChip(hoverCh) : nullptr;

	ClassViewCallback *callback = dynamic_cast<ClassViewCallback*>(cb);

	// NOTE: It is NOT safe to access vChips in this function.

	// holding down left button? (This cannot be part of PreRenderProcessing!!)
	if (allowEdit && im->GetMouseButton(InputManager::LBUTTON).isActive) {
		if (dm == DM_DRAGGING) { // Dragging selected items. This only happens when left button is pressed!
			Vector2 d((float32)im->GetMouseDeltaPos().x * zoom * ar / w, (float32)-im->GetMouseDeltaPos().y * zoom / h);
			if (d.LengthSquared() > 0.0f) {
				// Drag chips
				if (sChips.size() > 0) {
					for (unsigned i = 0; i < vChips.size(); i++) {
						if (vChips[i]->GetChipEditorData()->selected)
							vChips[i]->GetChipEditorData()->pos += d;
					}
				}
				// Drag folders
				if (sFolders.size() > 0) {
					for (unsigned i = 0; i < vFolders.size(); i++) {
						const Folder &f = folders[vFolders[i]];
						if (f.selected) {
							if (f.parentFolder == folder)
								clazz->SetFolderPos(vFolders[i], f.pos + d);
							else
								clazz->SetFolderInPos(vFolders[i], f.inPos + d);
						}
					}
				}
				// Drag background items
				if (sBackgroundItems.size() > 0) {
					for (size_t i = 0; i < vBackgroundItems.size(); i++) {
						if (vBackgroundItems[i]->second.selected) {
							auto n = vBackgroundItems[i];
							clazz->UpdateBackgroundItem(n->first, n->second.pos + d, n->second.size);
						}
					}
				}
				clazz->SetDirty();
			}
		}
		if (dm == DM_SIZING) {
			Vector2 d((float32)im->GetMouseDeltaPos().x * zoom * ar / w, (float32)-im->GetMouseDeltaPos().y * zoom / h);
			if (d.LengthSquared() > 0.0f) {
				if (sBackgroundItems.size() == 1) {
					auto n = clazz->GetBackgroundItems().find(*sBackgroundItems.begin());
					if (n != clazz->GetBackgroundItems().end()) {
						Vector2 s = Vector2(std::max(mousePos.x - _dScaling.x, BACKGROUNDITEM_MIN_SIZE), std::max(_dScaling.y - mousePos.y, BACKGROUNDITEM_MIN_SIZE));

						if (SHIFT) {
							float32 ar = 1.0f;
							if (n->second.type == CGBackgroundItem::IMAGE) {
								if (n->second.imageSRV) {
									UINT w = 1, h = 1;
									// Assume texture2d.
									SID3D11Resource res;
									n->second.imageSRV->GetResource(&res);
									D3D11_TEXTURE2D_DESC desc;
									((ID3D11Texture2D*)res.get())->GetDesc(&desc);
									ar = float32(desc.Width) / desc.Height;
								}
							}
							if (s.y * ar < s.x / ar) {
								s.x = std::max(s.y * ar, BACKGROUNDITEM_MIN_SIZE);
								s.y = s.x / ar;
								_dScaling.x = mousePos.x - s.x;
							}
							else {
								s.y = std::max(s.x / ar, BACKGROUNDITEM_MIN_SIZE);
								s.x = s.y * ar;
								_dScaling.y = mousePos.y + s.y;
							}
						}

						Vector2 p = n->second.pos;
						p.x += (s.x - n->second.size.x) * 0.5f;
						p.y += (n->second.size.y - s.y) * 0.5f;
						clazz->UpdateBackgroundItem(n->first, p, s);
					}
				}
			}
		}
	}


	// Releasing left mouse button?
	if (allowEdit && im->GetMouseButton(InputManager::LBUTTON).onDeactivate) {
		if (hm == HM_CHIP) { // Mouse is over chip?
			if (dm == DM_TOPCONN)
				_createLinkFromTopConnections(false); // Link(s) dragged from a top connection onto a chip. Try create the link(s).
			else if (dm == DM_CHILDCONN)
				_createLinkFromChildConnection(); // Link dragged from a child connection onto a chip. Try create the link.
			else if (dm == DM_SELECT) {} // N/A
			else if (dm == DM_DRAGGING) {} // N/A
			else if (L) 
				_createLinkFromTopConnections(false); // Link selected chips as child to the hovered chip
			else if (K) {
				if (hoverChip) { 
					hoverChip->RemoveEmptyConnections();
					unsigned c = hoverChip->GetChipEditorData()->GetNumberOfChildConnections();
					hoverChip->GetChipEditorData()->pos.x -= CONNECTION_WIDTH * 0.5f * (float32)(hoverChip->GetChipEditorData()->cCount - c);
					if (hoverChip->GetChipEditorData()->cCount - c != 0)
						clazz->SetDirty();
				}
			}
			else if (D) { // Remove this single chip
				if (hoverChip) {
					clazz->RemoveChip(hoverChip);
					clazz->SetDirty();
				}
				
			}
			else if (A) {
				if (hoverChip) {
					hoverChip->ClearMessages();
				}
			}
		}
		else if (hm == HM_TOPCONN) { // Is mouse over top connection?
			if (dm == DM_TOPCONN) {} // N/A
			else if (dm == DM_CHILDCONN)
				_createLinkFromChildConnection(); // Link dragged from a child connection onto a top connection. Try create the link.
			else if (dm == DM_SELECT) {} // N/A
			else if (dm == DM_DRAGGING) {} // N/A
			else if (L) {} // N/A
			else if (K) {} // N/A
			else if (D) {} // N/A
		}
		else if (hm == HM_CHILDCONN) { // Is mouse over child connection?
			if (dm == DM_TOPCONN)
				_createLinkFromTopConnections(SHIFT); // Link(s) dragged from a top connection onto a child connection. Try create the link(s).
			else if (dm == DM_CHILDCONN) {} // N/A
			else if (dm == DM_SELECT) {} // N/A
			else if (dm == DM_DRAGGING) {} // N/A
			else if (L)
				_createLinkFromTopConnections(false); // Link selected chips as child to the hovered chip at the hovered link.
			else if (K) {} // N/A
			else if (D) {} // N/A
		}
		else if (hm == HM_LINK) { // Is mouse over link?
			if (dm == DM_TOPCONN) {} // N/A
			else if (dm == DM_CHILDCONN) { // Dragging a child connector to an empty space => Show context menu to create&connect new chip! (Allow it when releasing on links as well!)
				hm = HM_ADDCHILD;
				if (callback)
					callback->ShowContextMenu();
			}
			else if (dm == DM_SELECT) {} // N/A
			else if (dm == DM_DRAGGING) {} // N/A
			else if (L) {} // N/A
			else if (K) {} // N/A
			else if (D) {
				// Remove this single link
				Link &l = vLinks[hoverLink];
				auto n = chipMap.find(l.id.first);
				if (n != chipMap.end()) {
					n->second->SetChild(0, l.id.second.first, l.id.second.second);
					clazz->SetDirty();
				}
			}
		}
		else if (hm == HM_FOLDER) { // Is mouse over folder?
			if (dm == DM_DRAGGING) {
				// Move all selected items to this folder
				if (sFolders.find(hoverFolder) == sFolders.end()) {
					FolderID f = hoverFolder == folder ? folders[folder].parentFolder : hoverFolder;
					clazz->MoveChipsToFolder(f, sChips);
					clazz->MoveFoldersToFolder(f, sFolders);
					clazz->MoveBackgroundItemsToFolder(f, sBackgroundItems);
					_clearSelection();
					clazz->SetDirty();
				}
			}
		}
		else if (hm == HM_BGITEM) {
			if (dm == DM_TOPCONN) {} // N/A
			else if (dm == DM_CHILDCONN) {} // N/A
			else if (dm == DM_SELECT) {} // N/A
			else if (dm == DM_DRAGGING) {} // N/A
			else if (L) {} // N/A
			else if (K) {} // N/A
			else if (D) { // Remove this single item
				if (clazz->RemoveBackgroundItem(hoverBackgroundItem))
					clazz->SetDirty();
			}
		}
		else if (dm == DM_CHILDCONN) { // Dragging a child connector to an empty space => Show context menu to create&connect new chip!
			hm = HM_ADDCHILD;
			if (callback)
				callback->ShowContextMenu();
		}
		dm = DM_NONE;
	}

	if (!(K || L || D) && im->GetMouseButtonDblClick(InputManager::LBUTTON).isActive) { // Doubleclicking left button?
		if (hm == HM_CHIP) { 
			if (hoverChip && callback) {
				if (hoverChip->AsFunctionCall() && hoverChip->AsFunctionCall()->GetFunction())
					callback->FocusOnChip((ClassExt*)hoverChip->AsFunctionCall()->GetFunction()->GetChip()->GetClass(), hoverChip->AsFunctionCall()->GetFunction()->GetChip()); // Doubleclick on function call => goto function!
				else if (dynamic_cast<ClassDiagramChip*>(hoverChip)) { // Class Diagram Chip? Go to CG!
					callback->FocusOnChip((ClassExt*)dynamic_cast<ClassDiagramChip*>(hoverChip)->GetCG(), 0);
				} 
				else
					callback->ShowChipDialog(hoverChip); // Show chip dialog
			}
		}
		else if (hm == HM_FOLDER) {
			// Enter this folder
			if (callback)
				callback->EnterFolder(clazz, hoverFolder, hoverFolder != folder);
		}
		else if (hm == HM_BGITEM) {
			if (callback) {
				callback->UpdateBackgroundItem(hoverBackgroundItem);
				dm = DM_NONE; // NOTE: we have to cancel drag mode, because we are not getting an release event in this case!
			}
		}
	}

	if (!(CTRL || SHIFT) && S) { // Is S being pressed this frame?
		// Create shortcuts of all selected chips.
		// Center it at cursor position.
		unsigned i = 0;
		Vector2 centerPos(0.0f, 0.0f);
		for (const auto& n : sChips) {
			Chip *c = clazz->GetChip(n);
			if (!c)
				continue;
			centerPos += c->GetChipEditorData()->pos;
			i++;
		}
		if (i > 0) {
			centerPos /= (float32)i;
			Set<ChipID> s;
			for (const auto &n : sChips) {
				Chip *c = clazz->GetChip(n);
				if (!c)
					continue;
				Shortcut *shortcut = clazz->CreateShortcut(c, folder, mousePos + (c->GetChipEditorData()->pos - centerPos));
				if (shortcut)
					s.insert(shortcut->GetID());
			}
			_clearSelection();
			sChips = s; // Select the new shortcuts!
			clazz->SetDirty();
		}
	}
	else if (CTRL && R) {
		RefreshManager::RefreshMode rm;
		bool b = false;
		for (const auto &n : sChips) {
			Chip *cv = clazz->GetChip(n);
			if (!cv)
				continue;
			if (!b) {
				rm = cv->GetRefreshManager().GetRefreshMode();
				if (rm == RefreshManager::RefreshMode::Once)
					rm = RefreshManager::RefreshMode::OncePerFrame;
				else if (rm == RefreshManager::RefreshMode::OncePerFrame)
					rm = RefreshManager::RefreshMode::OncePerFunctionCall;
				else if (rm == RefreshManager::RefreshMode::OncePerFunctionCall)
					rm = RefreshManager::RefreshMode::Always;
				else
					rm = RefreshManager::RefreshMode::Once;
				b = true;
			}
			cv->GetRefreshManager().SetRefreshMode(rm);
			clazz->SetDirty();
		}
	}
	else if (SPACE) {
		if (sFolders.size() == 1 && sChips.size() == 0 && sBackgroundItems.size() == 0) {
			UnpackFolder(*sFolders.rbegin());
		}
		else if (sChips.size() > 0 || sBackgroundItems.size() > 0) {
			// Create a new folder with ALL selected items!
			AddSelectionToNewFolder();
		}
	}
}

void ClassView::UnpackFolder(FolderID f)
{
	if (!clazz->IsFolderValid(f))
		return;

	const FolderList &folders = clazz->GetFolderList();
	const ChipPtrByChipIDMap &chipMap = clazz->GetChips();

	if (f == folder) { // Are we expanding the folder we are currently inside?
		// Move view up one folder!
		folder = folders[f].parentFolder;
		camPos = folders[f].pos;
	}

	_clearSelection();

	// Select all content inside folder!
	// Iterate chips and shortcuts.
	for (const auto &n : chipMap) {
		ChipEditorData *ced = n.second->GetChipEditorData();
		if (ced->folder == f)
			sChips.insert(n.first);
	}
	// Iterate all folders
	for (FolderID i = 1; i < folders.size(); i++) { // Start at 1: Do not include main "folder" (0)
		if (folders[i].parentFolder == f)
			sFolders.insert(i);
	}
	// Iterate background items
	for (const auto &n : clazz->GetBackgroundItems()) {
		if (n.second.folder == f)
			sBackgroundItems.insert(n.first);
	}

	clazz->MoveChipsToFolder(folder, sChips);
	clazz->MoveFoldersToFolder(folder, sFolders);
	clazz->MoveBackgroundItemsToFolder(folder, sBackgroundItems);
	clazz->RemoveFolder(f);
	clazz->SetDirty();
}

void ClassView::AddSelectionToNewFolder()
{
	FolderID f = clazz->CreateFolder(folder, mousePos, Vector2(0.0f, 4.0f), MTEXT("New Folder"));
	if (f == InvalidFolderID)
		return;
	clazz->MoveChipsToFolder(f, sChips);
	clazz->MoveFoldersToFolder(f, sFolders);
	clazz->MoveBackgroundItemsToFolder(f, sBackgroundItems);
	_clearSelection();
	clazz->SetDirty();
}

void ClassView::SetFolder(FolderID f, bool inside)
{
	if (!clazz->IsFolderValid(f))
		return;
	const FolderList &folders = clazz->GetFolderList();
	camPos = inside ? folders[f].inPos : folders[f].pos;
	folder = inside ? f : folders[f].parentFolder;
	_clearSelection();
}

bool ClassView::CanCopy()
{
	return sChips.size() > 0 || sBackgroundItems.size() > 0 || (sFolders.size() > 1 || (sFolders.size() == 1 && sFolders.find(folder) == sFolders.end()));
}

bool ClassView::CanDelete()
{
	return CanCopy() || sLinks.size() > 0;
}

void ClassView::Copy(DataBuffer &db)
{
	if (!CanCopy())
		return;
	std::unique_ptr<DocumentSaver> saver(DocumentFileTypes::CreateSaver(DocumentFileTypes::FileType::JSON));
	saver->Initialize();
	DocumentExtSaver::CopySelection(clazz, *saver, folder, sChips, sBackgroundItems, sFolders);
	saver->SaveToMemory(db);
}

void ClassView::Paste(DataBuffer &&db)
{
	_clearSelection();
	std::unique_ptr<DocumentLoader> loader(DocumentFileTypes::CreateLoader(DocumentFileTypes::FileType::JSON));
	bool b = loader->OpenMemory(std::move(db));
	if (DocumentExtLoader::Paste(clazz, *loader, folder, _isCursorOutsideView() ? camPos : mousePos, sChips, sBackgroundItems, sFolders)) {
		clazz->SetDirty();
		for (const auto &n : sChips) {
			Chip *ch = clazz->GetChip(n);
			if (ch)
				ch->RestoreChip();
		}
	}
}

void ClassView::Delete()
{
	if (!CanDelete())
		return;
	for (const auto &n : sChips) {
		Chip *ch = clazz->GetChip(n);
		if (ch)
			clazz->RemoveChip(ch);
	}
	for (const auto &n : sLinks) {
		Chip *ch = clazz->GetChip(n.first);
		if (ch)
			ch->SetChild(0, n.second.first, n.second.second);
	}
	for (const auto &n : sFolders) {
		if (n != folder) // Can't remove current folder!
			clazz->RemoveFolder(n);
	}
	for (const auto &n : sBackgroundItems) {
		clazz->RemoveBackgroundItem(n);
	}
	_clearSelection();
	clazz->SetDirty();
}

void ClassView::AddChildToDraggedConnection(Chip *c, bool allowRename)
{
	if (!c)
		return;
	Chip *parent = clazz->GetChip(connDragChip);
	if (!parent)
		return;
	ConnectionID cid = parent->GetChipEditorData()->GetConnectionID(dragConn);
	if (cid == InvalidConnectionID)
		return;
	Vector2 p = mousePos;
	p.y += -(CHIP_HALF_HEIGHT + CONNECTION_HEIGHT);
	c->GetChipEditorData()->pos = p;
	c->GetChipEditorData()->folder = folder;
	if (c->AsProxyChip() && !c->AsProxyChip()->IsChipTypeSet()) {
		const ChipInfo *nfo = engine->GetChipManager()->GetChipInfo(dragGuid);
		if (nfo)
			c->AsProxyChip()->SetChipType(nfo->chipDesc.type);
	}
	parent->SetChild(c, cid.first, cid.second);
	if (allowRename) {
		auto a = parent->GetChildren()[cid.first];
		if (a && a->desc.connType == ChildConnectionDesc::SINGLE)
			c->SetName(a->desc.name);
	}
}

void ClassView::InsertTemplate(DocumentLoader &loader, String name)
{
	_clearSelection();
	if (DocumentExtLoader::InsertTemplate(clazz, loader, name, folder, _isCursorOutsideView() ? camPos : mousePos, false, sChips, sBackgroundItems, sFolders))
		clazz->SetDirty();
}

void ClassView::AddRectangle(Vector4 color, Vector2 pos)
{
	CGBackgroundItemID id = clazz->AddRectangle(folder, pos, Vector2(5.0f, 5.0f), color);
	_clearSelection();
	sBackgroundItems.insert(id);
}

void ClassView::AddText(String text, Vector2 s, float32 size, Vector4 color, Vector2 pos)
{
	CGBackgroundItemID id = clazz->AddText(folder, pos, s, color, text, size);
	_clearSelection();
	sBackgroundItems.insert(id);
}

void ClassView::AddImage(Vector2 s, const DataBuffer &db, Vector2 pos, ID3D11ShaderResourceView *srv)
{
	CGBackgroundItemID id = clazz->AddImage(folder, pos, s, db, srv);
	_clearSelection();
	sBackgroundItems.insert(id);
}

void ClassView::AddFolder(String name, Vector2 pos)
{
	FolderID fid = clazz->CreateFolder(folder, pos, Vector2(0.0f, 4.0f), name);
	_clearSelection();
	sFolders.insert(fid);
}

bool ClassView::_isCursorOutsideView()
{
	return mousePos.x < (camPos.x - zoom * ar * 0.5f) || mousePos.x > (camPos.x + zoom * ar * 0.5f) || mousePos.y < (camPos.y - zoom * 0.5f) || mousePos.y > (camPos.y + zoom * 0.5f);
}

bool ClassView::_isChipVisible(Chip *ch) const
{
	ChipEditorData *ced = ch->GetChipEditorData();
	return folder == ced->folder && (abs(ced->pos.x - camPos.x) < (zoom * ar * 0.5f + ced->width * 0.5f) && abs(ced->pos.y - camPos.y) < (zoom * 0.5f + CONNECTION_YPOS));
}

void ClassView::_clearSelection()
{
	sChips.clear();
	sLinks.clear();
	sFolders.clear();
	sBackgroundItems.clear();
	// NOTE: I should probably set selected=false in all vChips....
}

void ClassView::_selectChip(Chip *ch, bool keep, bool selectChildren)
{
	// If CTRL: Keep selection. Invert hovered.
	// If !CTRL: Clear selection if hovered is not selected, then select hovered.
	// If SHIFT: Select all children.

	ChipEditorData *ced = ch->GetChipEditorData();

	if (keep) {
		if (ced->selected) {
			ced->selected = false;
			sChips.erase(ch->GetID());
		}
		else {
			ced->selected = true;
			sChips.insert(ch->GetID());
		}
	}
	else if (!ced->selected) {
		_clearSelection();
		ced->selected = true;
		sChips.insert(ch->GetID());
	}

	if (!selectChildren)
		return;
	
	// Select all children!

	const FolderList &folders = clazz->GetFolderList();

	Set<Chip*> s;

	ced->FindAllChildren(s); // Find all children to the clicked chip

	for (const auto &m : s) {
		ChipEditorData *cgi = m->GetChipEditorData();
		if (cgi->folder == folder) { // Same folder?
			cgi->selected = true;
			sChips.insert(m->GetID());
		}
		else {
			FolderID f = clazz->GetClosestFolder(cgi->folder, folder);
			folders[f].selected = true;
			sFolders.insert(f);
		}
	}
}

void ClassView::_createLinkFromChildConnection()
{
	Chip *ch = clazz->GetChip(connDragChip);
	if (!ch)
		return;
	Chip *hoverChip = clazz->GetChip(hoverCh);
	if (!hoverChip) 
		return;
	if (clazz->CreateLink(ch, hoverChip, ch->GetChipEditorData()->GetConnectionID(dragConn), false))
		clazz->SetDirty();
}

void ClassView::_createLinkFromTopConnections(bool insert)
{
	Chip *chip = clazz->GetChip(hoverCh);
	if (!chip) 
		return;
	ChipEditorData *cv = chip->GetChipEditorData();

	Chip *cvc = 0;
	MultiMap<FLOAT, Chip*> chMap;

	ConnectionID cid = InvalidConnectionID;
	if (hm == HM_CHILDCONN) // Did we click/drop on a child connection and not the chip itself?
		cid = cv->GetConnectionID(hoverConn);

	if (dm == DM_TOPCONN) { // Are we dragging from a top connection? (opposite to pressing 'L' and clicking a chip)
		cvc = clazz->GetChip(connDragChip);
		if (!cvc)
			return;
		if (hm == HM_CHILDCONN) { // Hovers over a child connection and not the chip itself. Link the chip we are dragging from to this particular connection.
			// Dropping link at child connection (When releasing button!):
			//		If SHIFT is pressed AND linking to a growing connection, insert the child at the selected connection, and move the other children to the right.
			//		Else always unlink the previous child, if any, from the connection.
			// Drag from child connection:
			//		If CTRL is pressed when starting dragging from a linked connection, start to drag a new link from the child's topconnection instead.
			//			The above role still applys when linking to a child connection!
			//		Else If SHIFT is pressed when starting dragging from a linked connection, set a flag to indicate that this is a SWAP operation, and drag from topconnection.
			//			When linking to a child connection, set the connection, if any, that was there before, to the child connection we started dragging from, if type allow!
			//			Exception: If SHIFT is still pressed when linking, and we're linking to the same row of growing child connections, make this a MOVE operation!

			if (connSwap) {
				bool move = insert && chip->GetID() == connSwapConnection.first && cid.first == connSwapConnection.second.first;
				if (move) {
					if (chip->MoveChild(cid.first, connSwapConnection.second.second, cid.second))
						clazz->SetDirty();
				}
				else {
					Chip *childToSwap = clazz->GetChip(cv->GetChild(cid));
					if (clazz->CreateLink(chip, cvc, cid, false)) {
						clazz->SetDirty();
						Chip *oldChip = clazz->GetChip(connSwapConnection.first);
						if (oldChip) {
							clazz->CreateLink(oldChip, childToSwap, connSwapConnection.second, false);
						}
					}
				}
			}
			else {
				if (clazz->CreateLink(chip, cvc, cid, insert))
					clazz->SetDirty();
			}
		}
		else
			chMap.insert(std::make_pair(cvc->GetChipEditorData()->pos.x, cvc));
	}
		
	if (dm != DM_TOPCONN || dm == DM_TOPCONN && dragSelected) { // Do only include selected chips if 'L'-click OR we want to include selected!
		// Sort selected chips to be connected by x-pos.
		for (const auto &n : sChips) {
			Chip *c = clazz->GetChip(n);
			if (c && c != cvc) // c != cvc: Not insert the chip dragged from (it has already been inserted OR (tried) linked!)
				chMap.insert(std::make_pair(c->GetChipEditorData()->pos.x, c));
		}
	}

	unsigned a = cid.first != InvalidConnectionID.first ? cid.first : 0;
	unsigned b = cid.first != InvalidConnectionID.first ? (a + 1) : (unsigned)chip->GetChildren().size();
	for (const auto &n : chMap) {
		Chip *cvc = n.second;
		for (unsigned i = a, j = 0; i < b; i++) {
			ChildConnection *cc = chip->GetChildren()[i];
			if (!cc)
				continue;
			for (j = 0; j < cc->connections.size() && cc->connections[j].chip; j++);
//			if (!chip->GetChild(i, j)) { // changed 16/12/13. Should not call this function here...
			if (!chip->GetRawChild(i, j)) {
				if (clazz->CreateLink(chip, cvc, ConnectionID(i, j), false)) { // try to create the link
					clazz->SetDirty();
					break;
				}
			}
		}
	}
}

