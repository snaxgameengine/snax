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
#include "M3DEngine/Engine.h"
#include "M3DEngine/ChipManager.h"
#include "M3DEngine/Chip.h"
#include "StdChips/Shortcut.h"
#include "ChipDialogManager.h"
#include "ChipDialog.h"

using namespace m3d;

// Note: This function is a copy of QDialog::adjustPosition(QWidget* w) (internal and private). 
// I use it to position 'widget' with a possible size 'size' in the center of 'w'.
void adjustPosition(QWidget* widget, QSize size, QWidget* w)
{
//	if (const QPlatformTheme* theme = QGuiApplicationPrivate::platformTheme())
//		if (theme->themeHint(QPlatformTheme::WindowAutoPlacement).toBool())
//			return;
	QPoint p(0, 0);
	int extraw = 0, extrah = 0;
	const QWindow* parentWindow = nullptr;
	if (w) {
		w = w->window();
	}
	else {
		//parentWindow = widget->d->transientParentWindow();
	}
	QRect desk;
	QScreen* scrn = nullptr;
	if (w)
		scrn = w->screen();
	else if (parentWindow)
		scrn = parentWindow->screen();
	else if (QGuiApplication::primaryScreen()->virtualSiblings().size() > 1)
		scrn = QGuiApplication::screenAt(QCursor::pos());
	else
		scrn = widget->screen();
	if (scrn)
		desk = scrn->availableGeometry();

	QWidgetList list = QApplication::topLevelWidgets();
	for (int i = 0; (extraw == 0 || extrah == 0) && i < list.size(); ++i) {
		QWidget* current = list.at(i);
		if (current->isVisible()) {
			int framew = current->geometry().x() - current->x();
			int frameh = current->geometry().y() - current->y();

			extraw = qMax(extraw, framew);
			extrah = qMax(extrah, frameh);
		}
	}

	// sanity check for decoration frames. With embedding, we
	// might get extraordinary values
	if (extraw == 0 || extrah == 0 || extraw >= 10 || extrah >= 40) {
		extrah = 40;
		extraw = 10;
	}


	if (w) {
		// Use pos() if the widget is embedded into a native window
		QPoint pp;
		if (w->windowHandle() && qvariant_cast<WId>(w->windowHandle()->property("_q_embedded_native_parent_handle")))
			pp = w->pos();
		else
			pp = w->mapToGlobal(QPoint(0, 0));
		p = QPoint(pp.x() + w->width() / 2,
			pp.y() + w->height() / 2);
	}
	else if (parentWindow) {
		// QTBUG-63406: Widget-based dialog in QML, which has no Widget parent
		// but a transient parent window.
		QPoint pp = parentWindow->mapToGlobal(QPoint(0, 0));
		p = QPoint(pp.x() + parentWindow->width() / 2, pp.y() + parentWindow->height() / 2);
	}
	else {
		// p = middle of the desktop
		p = QPoint(desk.x() + desk.width() / 2, desk.y() + desk.height() / 2);
	}

	// p = origin of this
	p = QPoint(p.x() - size.width() / 2 - extraw,
		p.y() - size.height() / 2 - extrah);


	if (p.x() + extraw + size.width() > desk.x() + desk.width())
		p.setX(desk.x() + desk.width() - size.width() - extraw);
	if (p.x() < desk.x())
		p.setX(desk.x());

	if (p.y() + extrah + size.height() > desk.y() + desk.height())
		p.setY(desk.y() + desk.height() - size.height() - extrah);
	if (p.y() < desk.y())
		p.setY(desk.y());

	// QTBUG-52735: Manually set the correct target screen since scaling in a
	// subsequent call to QWindow::resize() may otherwise use the wrong factor
	// if the screen changed notification is still in an event queue.
	if (scrn) {
		if (QWindow* window = widget->windowHandle())
			window->setScreen(scrn);
	}

	widget->move(p);
}





namespace m3d
{

	static bool operator==(const DialogType &a, const DialogType &b)
	{
		return a.chipType == b.chipType && a.factoryFunc == b.factoryFunc;
	}

}


ChipDialogManager::ChipDialogManager(ChipDialogManagerCallback *callback) : _callback(callback), _embedByDefault(0), _commentW(-1), _commentH(-1)
{
}

ChipDialogManager::~ChipDialogManager()
{
	m3d::msg(DINFO, MTEXT("ChipDialogManager::~ChipDialogManager() called."));

	assert(_dialogs.empty()); // All dialogs should have been closed!
	assert(_settings.empty()); // All chips should have been removed!

	for (const auto &n : _dialogInfos)
		delete n.second;
	_dialogInfos.clear();
	for (uint32 i = 0; i < _dlgPackets.size(); i++) {
		if (_dlgPackets[i]->module) {
			ONDLGPACKETUNLOAD onDlgPacketUnloadFunc = (ONDLGPACKETUNLOAD)GetProcAddress(_dlgPackets[i]->module, MTEXT("OnDlgPacketUnload"));
			if (onDlgPacketUnloadFunc)
				(*onDlgPacketUnloadFunc)();
			FreeLibrary(_dlgPackets[i]->module);
		}
		delete _dlgPackets[i];
	}
	_dlgPackets.clear();
}

bool ChipDialogManager::FindDialogs(Path folder)
{
	if (!folder.IsDirectory())
		return false;

	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = FindFirstFileA((folder.AsString() + MTEXT("*.dll")).c_str(), &FindFileData);

	// Process all DLLs in the specified directory
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			_processDLL(Path(FindFileData.cFileName, folder));
		}
		while (FindNextFileA(hFind, &FindFileData) != 0);
		FindClose(hFind);
	}

	return true;
}

String GetLastErrorAsString()
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return String();

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	String message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}

void ChipDialogManager::_processDLL(Path fileName)
{
	msg(DINFO, String(MTEXT("Searching for dialogs in \'")) + fileName.AsString() + MTEXT("\'..."));
	HMODULE hDLL = LoadLibraryA(fileName.AsString().c_str());
	if (!hDLL) {
		msg(WARN, strUtils::ConstructString(MTEXT("Unable to load \'%1\' to search for dialogs: %2")).arg(fileName.AsString()).arg(GetLastErrorAsString()));
		return;
	}

	ONDLGPACKETQUERY onDlgPacketQueryFunc = (ONDLGPACKETQUERY)GetProcAddress(hDLL, MTEXT("OnDlgPacketQuery"));

	if (!onDlgPacketQueryFunc || (*onDlgPacketQueryFunc)()) {
		GETDIALOGCOUNT getDialogCountFunc = (GETDIALOGCOUNT)GetProcAddress(hDLL, MTEXT("GetDialogCount"));
		GETDIALOGTYPE getDialogTypeFunc = (GETDIALOGTYPE)GetProcAddress(hDLL, MTEXT("GetDialogType"));
		RELEASEMEMORY deleteMemoryFunc = (RELEASEMEMORY)GetProcAddress(hDLL, MTEXT("ReleaseMemory"));
		if (getDialogCountFunc && getDialogTypeFunc && deleteMemoryFunc) {
			DlgPacket *packet = new DlgPacket();
			packet->filename = fileName;
			packet->dlgCount = (*getDialogCountFunc)();
			packet->deleteMemFunc = nullptr; // We set this later!
			uint32 dlgCount = 0;
			for (uint32 i = 0; i < packet->dlgCount; i++) {
				const DialogType &dt = (*getDialogTypeFunc)(i);
				const ChipInfo *ci = engine->GetChipManager()->GetChipInfo(dt.chipType);
				if (!ci) {
					msg(WARN, String(MTEXT("No chip type with Guid ")) + GuidToString(dt.chipType) + MTEXT(" found. Skipping dialog."));
					continue;
				}
				if (_dialogInfos.find(dt.chipType) != _dialogInfos.end()) {
					msg(WARN, String(MTEXT("Dialog for chip \'")) + ci->chipDesc.name + MTEXT("\' already exist. Skipping this one."));
					continue;
				}
				if (dt.factoryFunc) {
					DIALOGFACTORY fac = (DIALOGFACTORY)GetProcAddress(hDLL, dt.factoryFunc);
					if (!fac) {
						msg(WARN, String(MTEXT("No factory function found for dialog for chip \'")) + ci->chipDesc.name + MTEXT("\'."));
						continue;
					}
				}
				else
					msg(DINFO, String(MTEXT("No factory function specified for dialog for chip \'")) + ci->chipDesc.name + MTEXT("\'. This is OK."));

				ChipDlgInfo *nfo = new ChipDlgInfo();
				nfo->dialogDesc.chipType = dt.chipType;
				nfo->dialogDesc.chipTypeNP = dt.chipTypeNP;
				nfo->dialogDesc.factoryFunc = dt.factoryFunc ? dt.factoryFunc : "";
				nfo->packet = packet;
				nfo->dialogFactoryFunc = 0; // We set this later!
				nfo->index = i;
				_dialogInfos.insert(std::make_pair(dt.chipType, nfo));
				ChipDlgInfoList fdd;
				fdd.push_back(nfo);
				packet->dialogs.push_back(nfo);
				dlgCount++;
			}
			msg(DINFO, String(MTEXT("Found ")) + strUtils::fromNum(dlgCount) + MTEXT(" dialogs in \'") + fileName.AsString() + MTEXT("\'."));
			if (packet->dlgCount)
				_dlgPackets.push_back(packet);
			else
				delete packet;
		}
		else {
			msg(WARN, String(MTEXT("Skipping search for dialogs in \'")) + fileName.AsString() + MTEXT("\'. It does not appear to be a valid dialog dll."));
		}
	}
	else
		msg(WARN, String(MTEXT("Querying dialog DLL \'")) + fileName.AsString() + MTEXT("\' returned false. Skipping file."));

	msg(DINFO, String(MTEXT("Unloading \'")) + fileName.AsString() + MTEXT("\'..."));

	FreeLibrary(hDLL);
}

DialogPage *ChipDialogManager::CreateDialogPage(const Guid &guid)
{
	auto itr = _dialogInfos.find(guid);
	if (itr == _dialogInfos.end())
		return nullptr; // Some chips may not have a dialog!

	ChipDlgInfo* nfo = itr->second;

	const ChipInfo *ci = engine->GetChipManager()->GetChipInfo(guid);
	assert(ci);

	if (nfo->failedToLoad)
		return 0; // Previously failed!

	if (nfo->packet->failedToLoad) {
		msg(FATAL, String(MTEXT("Dialog DLL \'")) + nfo->packet->filename.AsString() + MTEXT("\' containing dialog for chip \'") + ci->chipDesc.name + MTEXT("\' has previously failed to load. No retry!"));
		nfo->failedToLoad = true; // To make sure no echo!
		return 0;
	}

	if (!nfo->packet->module) { // not loaded?
		// load dll
		msg(DINFO, String(MTEXT("Trying to load dialog DLL \'")) + nfo->packet->filename.AsString() + MTEXT("\'..."));
		HMODULE hDLL = LoadLibraryA(nfo->packet->filename.AsString().c_str());
		if (!hDLL) {
			msg(FATAL, String(MTEXT("Unable to load dialog DLL \'")) + nfo->packet->filename.AsString() + MTEXT("\'."));
			nfo->packet->failedToLoad = true;
			return 0; // dll not found anymore?
		}

		GETDIALOGCOUNT getDialogCountFunc = (GETDIALOGCOUNT)GetProcAddress(hDLL, MTEXT("GetDialogCount"));
		GETDIALOGTYPE getDialogTypeFunc = (GETDIALOGTYPE)GetProcAddress(hDLL, MTEXT("GetDialogType"));
		RELEASEMEMORY deleteMemoryFunc = (RELEASEMEMORY)GetProcAddress(hDLL, MTEXT("ReleaseMemory"));

		if (getDialogCountFunc && getDialogTypeFunc && deleteMemoryFunc) {
			bool ok = false;
			if ((*getDialogCountFunc)() == nfo->packet->dlgCount) {
				ok = true;
				for (uint32 i = 0; i < nfo->packet->dialogs.size(); i++) {
					const DialogType &dt = (*getDialogTypeFunc)(nfo->packet->dialogs[i]->index);
					if (!(ok = 
						(dt.chipType == nfo->packet->dialogs[i]->dialogDesc.chipType && 
						 dt.chipTypeNP == nfo->packet->dialogs[i]->dialogDesc.chipTypeNP && 
						 String(dt.factoryFunc ? dt.factoryFunc : "") == nfo->packet->dialogs[i]->dialogDesc.factoryFunc)))
						break;
					if (!nfo->packet->dialogs[i]->dialogDesc.factoryFunc.empty())
						nfo->packet->dialogs[i]->dialogFactoryFunc = (DIALOGFACTORY)GetProcAddress(hDLL, nfo->packet->dialogs[i]->dialogDesc.factoryFunc.c_str());
				}
			}
			if (ok) {
				ONDLGPACKETLOAD onDlgPacketLoadFunc = (ONDLGPACKETLOAD)GetProcAddress(hDLL, MTEXT("OnDlgPacketLoad"));
				if (!onDlgPacketLoadFunc || (*onDlgPacketLoadFunc)()) {
					// Packet loaded!!
					nfo->packet->module = hDLL;
					nfo->packet->deleteMemFunc = deleteMemoryFunc;
					msg(DINFO, String(MTEXT("Successfully loaded dialog DLL \'")) + nfo->packet->filename.AsString() + MTEXT("\'."));
				}
				else
					msg(FATAL, String(MTEXT("Dialog DLL \'")) + nfo->packet->filename.AsString() + MTEXT("\' refused to load."));
			}
			else
				msg(FATAL, String(MTEXT("Dialog DLL \'")) + nfo->packet->filename.AsString() + MTEXT("\' appears to be different from when first loaded."));
		}
		else
			msg(FATAL, String(MTEXT("\'")) + nfo->packet->filename.AsString() + MTEXT("\' does not appear to be a valid dialog DLL."));

		if (!nfo->packet->module) {
			msg(DINFO, String(MTEXT("Unloading \'")) + nfo->packet->filename.AsString() + MTEXT("\'..."));
			FreeLibrary(hDLL);
			nfo->packet->failedToLoad = true;
			return 0; // Invalid dll?
		}
	}

	if (!nfo->dialogFactoryFunc) { // No factory func? 
		if (!nfo->dialogDesc.factoryFunc.empty()) { // (NOTE: Empty factory function name implies no dialog page and is OK!)
			msg(FATAL, String(MTEXT("Unable to find factory function for dialog for chip \'")) + ci->chipDesc.name + MTEXT("\'."));
			nfo->failedToLoad = true; // Avoid echo!
		}
		return nullptr;
	}

	DialogPage *d = nullptr;
	
	try {
		d = (*nfo->dialogFactoryFunc)();
	}
	catch (...) {
		d = nullptr;
		msg(FATAL, String(MTEXT("The dialog factory function \'") + nfo->dialogDesc.factoryFunc + MTEXT("\' throwed an exception.")));
	}

	if (!d) {
		msg(FATAL, String(MTEXT("Unable to create dialog for chip \'")) + ci->chipDesc.name + MTEXT("\'."));
		nfo->failedToLoad = true;
		return nullptr;
	}

	d->SetDealloc(nfo->packet->deleteMemFunc);

	return d;
}




void ChipDialogManager::OpenChipDialog(Chip *chip, bool showComment, uint32 embeddedID, int32 *modalResult)
{
	if (chip && chip->AsShortcut())
		chip = chip->AsShortcut()->GetOriginal();

	if (!chip)
		return;


	auto n = _dialogs.find(chip);
	if (n != _dialogs.end()) {
		n->second->Activate();
		if (embeddedID == 0 || n->second->GetEmbeddedID() != embeddedID) // Do only flash if NOT opened embedded in the requested editor.
			n->second->Flash();
		n->second->GetChipDialog()->SetFocusToFirstItem();
		if (showComment)
			n->second->GetChipDialog()->OpenCommentPage();
		if (modalResult)
			*modalResult = -1; // modal not available!
		return;
	}

	auto m = _settings.find(chip);

	ChipDialogContainer *container = nullptr;
	
	if (modalResult == nullptr) { // Embedded containers can not be used when modal dialog is requested!
		if (embeddedID != 0) 
			container = _callback->GetEmbeddedContainer(embeddedID); // This will create an embedded container in the given editor.
		else if (m != _settings.end() && m->second.embedded)
			container = _callback->GetEmbeddedContainer(m->second.embeddedID); // This will try to create an embedded container in the same editor as last time. Else, the current editor is used.
		else {
			ChipTypeIndex idx = engine->GetChipManager()->GetChipTypeIndex(chip->GetChipDesc().type);
			auto n = _embedTypeByDefault.find(idx);
			if (n != _embedTypeByDefault.end() && m == _settings.end()) { // Keep embed-settings for this chip-dialog
				embeddedID = n->second;
			}
			else {
				embeddedID = _embedByDefault;
			}
			if (embeddedID != 0)
				container = _callback->GetEmbeddedContainer(embeddedID); // This will create an embedded container in the given editor.
		}
	}
	
	if (container == nullptr) // Still no container? Create a nonembedded one!
		container = new ChipDialogDialog(_callback->GetParentWidget());

	ChipDialog *cd = new ChipDialog();
	cd->SetChip(chip);
	cd->SetDialogManager(this);
	container->SetChipDialog(cd);
	
	for (Guid type = chip->GetChipDesc().type, nextPage = type; true; ) {
		const ChipInfo *ci = engine->GetChipManager()->GetChipInfo(type);
		assert(ci);

		if (type == nextPage) {
			DialogPage *dp = CreateDialogPage(type);
			if (dp) { // Some chips may not have a dialog!
				int32 w = -1, h = -1;
				PageData::MaximizedState maximized = PageData::MAX_UNSET;
				ChipTypeIndex cti = engine->GetChipManager()->GetChipTypeIndex(type);
				if (m != _settings.end()) { // Try to get page size for this particular chip...
					auto o = m->second.pageSizes.find(cti);
					if (o != m->second.pageSizes.end()) {
						w = o->second.w;
						h = o->second.h;
						maximized = o->second.maximized;
					}
				}
				if (w == -1) { // try to get page size for this chip type...
					auto o = _defaultPageSizes.find(cti);
					if (o != _defaultPageSizes.end()) {
						w = o->second.w;
						h = o->second.h;
						maximized = o->second.maximized;
					}
				}
				dp->SetChipTypeIndex(cti);
				cd->AddPage(dp, ci->chipDesc.name, QSize(w, h), maximized);
			}

			auto itr = _dialogInfos.find(type);
			nextPage = itr == _dialogInfos.end() || itr->second->dialogDesc.chipTypeNP == type ? ci->chipDesc.basetype : itr->second->dialogDesc.chipTypeNP;
		}

		if (type == CHIP_GUID)
			break;
		type = ci->chipDesc.basetype;
	}

	n = _dialogs.insert(std::make_pair(chip, container)).first;

	chip->AddDestructionObserver(this);

	int32 commentW = _commentW, commentH = _commentH;
	if (m != _settings.end() && m->second.commentW != -1) {
		commentW = m->second.commentW;
		commentH = m->second.commentH;
	}

	QSize sizeHint;
	cd->Init(QSize(commentW, commentH), &sizeHint);

	if (container->AsQDialog()) {
		if (m != _settings.end()) {
			container->AsQDialog()->move(m->second.x, m->second.y);
		}
		else {
			adjustPosition(container->AsQDialog(), sizeHint, _callback->GetParentWidget());
		}
	}

	if (!_callback->HasComments(chip))
		cd->HideCommentPage(true);

	if (showComment)
		cd->OpenCommentPage();

	if (modalResult) {
		*modalResult = container->AsQDialog()->exec();
		CloseChipDialog(chip); // We have do manually close the dialog here to avoid a race condition where the dialog is deleted before the return of exec()!
	}
	else {
		container->OpenDialog();
		cd->SetFocusToFirstItem();
	}
}

void ChipDialogManager::CloseChipDialog(Chip *chip)
{
	auto n = _dialogs.find(chip);
	if (n != _dialogs.end()) {
		_keepDialogSettings(n->second);
		n->second->DeleteLater();
		_dialogs.erase(n);
	}
	int32 gg = 5;
}

void ChipDialogManager::_keepDialogSettings(ChipDialogContainer *container)
{
	Chip *chip = container->GetChipDialog()->GetChip();

	ChipDialogSettings &s = _settings.insert(std::make_pair(chip, ChipDialogSettings())).first->second;
	if (container->AsQDialog()) { // Remember dialog position and page sizes if not embedded!
		s.embedded = false; // We're not embedded, but keep the embeddedID (if any) for later!
		s.x = container->AsQDialog()->pos().x();
		s.y = container->AsQDialog()->pos().y();
	}
	else {
		s.embedded = true;
		s.embeddedID = container->GetEmbeddedID();
	}

	QSize cs = container->GetChipDialog()->GetCommentPageSize();
	if (cs.width() != -1) _commentW = s.commentW = cs.width();
	if (cs.height() != -1) _commentH = s.commentH = cs.height();

	for (Guid type = chip->GetChipDesc().type; true; ) {
		const ChipInfo *ci = engine->GetChipManager()->GetChipInfo(type);
		assert(ci);
		ChipTypeIndex cti = engine->GetChipManager()->GetChipTypeIndex(type);
		PageData pd = container->GetChipDialog()->GetPageData(cti);
		if (pd.size.width() != -1) {
			auto m = _defaultPageSizes.insert(std::make_pair(cti, ChipPageSettings{ 0, 0, PageData::MAX_UNSET })).first;
			auto o = s.pageSizes.insert(std::make_pair(cti, ChipPageSettings{ 0, 0, PageData::MAX_UNSET })).first;
			m->second.w = o->second.w = pd.size.width();
			m->second.h = o->second.h = pd.size.height();
			m->second.maximized = o->second.maximized = pd.maximized;
		}
		if (type == CHIP_GUID)
			break;
		type = ci->chipDesc.basetype;
	}
}

void ChipDialogManager::CloseAllChipDialogs()
{
	while (_dialogs.size())
		CloseChipDialog(_dialogs.begin()->first);
}

void ChipDialogManager::OnDestruction(DestructionObservable *observable)
{
	// chip destroyed!
	Chip *chip = dynamic_cast<Chip*>(observable);
	if (chip) {
		CloseChipDialog(chip); // Need to destroy the dialog!
		_settings.erase(chip);
	}

}

void ChipDialogManager::UpdateChipDialogs()
{
	for (const auto &n : _dialogs)
		n.second->GetChipDialog()->Update();
}

void ChipDialogManager::ToggleEmbedding(ChipDialog *cd)
{
	Chip *chip = cd->GetChip();
	auto n = _dialogs.find(chip);

	_keepDialogSettings(n->second);

	auto m = _settings.find(chip);

	ChipDialogContainer *container = nullptr;

	if (n->second->AsQDialog()) {
		if (m != _settings.end() && m->second.embeddedID != 0)
			container = _callback->GetEmbeddedContainer(m->second.embeddedID);
		else
			container = _callback->GetEmbeddedContainer(0);
	}
	else {
		container = new ChipDialogDialog(_callback->GetParentWidget());
	}

	container->SetChipDialog(cd);
	container->OnPageChanged();
	container->OpenDialog();
	n->second->ReleaseChipDialog();
	if (n->second->AsQDialog())
		n->second->AsQDialog()->close();
	n->second->DeleteLater();
	n->second = container;

}

void ChipDialogManager::SetDefaultEmbedding(Guid chipType, uint32 embeddedID)
{
	ChipTypeIndex idx = engine->GetChipManager()->GetChipTypeIndex(chipType);
//	_embedTypeByDefault[idx] = _embedByDefault = embeddedID;
	_embedTypeByDefault[idx] = embeddedID;
}

void ChipDialogManager::DestroyDeviceObject()
{
	for (const auto &n : _dialogs)
		n.second->GetChipDialog()->DestroyDeviceObject();
}

QMimeData* ChipDialogManager::CreateMimeData() const
{
	return new QMimeData();
}