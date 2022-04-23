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

#include "stdafx.h"
#include "SearchWidget.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ChipManager.h"
#include "M3DEngine/ClassManager.h"
#include "M3DEngineExt/ClassExt.h"
#include "M3DEngine/Chip.h"
#include "M3DEngine/ChipManager.h"
#include <regex>
#include <qmessagebox.h>
#include <qmenu>
#include <qsettings.h>
#include "AppSignals.h"


using namespace m3d;


SearchWidget::SearchWidget(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f)
{
	ui.setupUi(this);

	connect(&AppSignals::instance(), &AppSignals::initChips, this, &SearchWidget::updateChipList);
	connect(&AppSignals::instance(), &AppSignals::updateSettings, this, &SearchWidget::updateSettings);

	_menu = new QMenu(ui.treeWidget);
	_aShowChip = _menu->addAction("Show Chip");
	QFont font = _aShowChip->font();
	font.setBold(true);
	_aShowChip->setFont(font);
	_aShowDialog = _menu->addAction("Show Chip Dialog");
	_aClear = _menu->addAction("Clear Results");

	ui.treeWidget->sortItems(0, Qt::AscendingOrder);
}

SearchWidget::~SearchWidget()
{
}

void SearchWidget::updateSettings(QSettings& s, bool load)
{
	s.beginGroup("mainWindow/" + objectName());
	if (load) {
		ui.lineEdit->setText(s.value("text", QString("")).toString());
		ui.checkBoxCaseSensitive->setChecked(s.value("caseSensitive", false).toBool()); 
		ui.checkBoxSearchComments->setChecked(s.value("searchComments", false).toBool());
		ui.comboBoxMatchType->setCurrentIndex(s.value("matchType", 1).toInt());
		ui.treeWidget->sortItems(s.value("sortColumn", 0).toInt(), (Qt::SortOrder)s.value("sortOrder", (int)Qt::AscendingOrder).toInt());
		QString str = s.value("chipType", QString()).toString();
		Guid g;
		ChipTypeIndex i = 0;
		int j = 0;
		if (str.isEmpty() || !StringToGUID(FROMQSTRING(str), g) || (i = engine->GetChipManager()->GetChipTypeIndex(g)) == 0 || (j = ui.comboBoxType->findData(i)) == -1)
			ui.comboBoxType->setCurrentIndex(0);
		else
			ui.comboBoxType->setCurrentIndex(j);
	}
	else {
		s.setValue("text", ui.lineEdit->text());
		s.setValue("caseSensitive", ui.checkBoxCaseSensitive->isChecked());
		s.setValue("searchComments", ui.checkBoxSearchComments->isChecked());
		s.setValue("matchType", ui.comboBoxMatchType->currentIndex());
		s.setValue("sortColumn", ui.treeWidget->sortColumn());
		s.setValue("sortOrder", (int)ui.treeWidget->header()->sortIndicatorOrder());
		const ChipInfo *nfo = engine->GetChipManager()->GetChipInfo((ChipTypeIndex)ui.comboBoxType->itemData(ui.comboBoxType->currentIndex()).toUInt());
		s.setValue("chipType", nfo ? TOQSTRING(GuidToString(nfo->chipDesc.type)) : QString());
	}
	s.endGroup();
}

bool match(const String &str, bool matchWholeString, const std::regex &pattern)
{
	return matchWholeString ? std::regex_match(str.c_str(), pattern) : std::regex_search(str.c_str(), pattern);
}

void SearchWidget::search()
{
	String searchString = FROMQSTRING(ui.lineEdit->text());
	bool caseSensitive = ui.checkBoxCaseSensitive->isChecked();
	bool searchComments = ui.checkBoxSearchComments->isChecked();
	unsigned matchType = ui.comboBoxMatchType->currentIndex();
	ChipTypeIndex typeIndex = (ChipTypeIndex)ui.comboBoxType->itemData(ui.comboBoxType->currentIndex()).toUInt();

	ui.treeWidget->clear();
	try {
		std::regex pattern;
		bool matchWholeString = false;
		if (matchType == 0 || matchType == 1) { // regular or wildcard search.
			searchString = strUtils::replace(searchString, MTEXT("\\"), MTEXT("\\\\"));
			searchString = strUtils::replace(searchString, MTEXT("^"), MTEXT("\\^"));
			searchString = strUtils::replace(searchString, MTEXT("."), MTEXT("\\."));
			searchString = strUtils::replace(searchString, MTEXT("$"), MTEXT("\\$"));
			searchString = strUtils::replace(searchString, MTEXT("|"), MTEXT("\\|"));
			searchString = strUtils::replace(searchString, MTEXT("("), MTEXT("\\("));
			searchString = strUtils::replace(searchString, MTEXT(")"), MTEXT("\\)"));
			searchString = strUtils::replace(searchString, MTEXT("["), MTEXT("\\["));
			searchString = strUtils::replace(searchString, MTEXT("]"), MTEXT("\\]"));
			searchString = strUtils::replace(searchString, MTEXT("*"), MTEXT("\\*"));
			searchString = strUtils::replace(searchString, MTEXT("+"), MTEXT("\\+"));
			searchString = strUtils::replace(searchString, MTEXT("?"), MTEXT("\\?"));
			searchString = strUtils::replace(searchString, MTEXT("/"), MTEXT("\\/"));
		}
		if (matchType == 1) { // wildcard search.
			matchWholeString = true;
			searchString = MTEXT("^") + searchString + MTEXT("$");
			searchString = strUtils::replace(searchString, MTEXT("\\?"), MTEXT("."));
			searchString = strUtils::replace(searchString, MTEXT("\\*"), MTEXT(".*"));
		}
		pattern = std::regex(searchString.c_str(), (caseSensitive ? (std::regex_constants::syntax_option_type)0 : std::regex_constants::icase));

		const ClassPtrByStringMap &cgMap = engine->GetClassManager()->GetClasssByName();
		for (const auto &n : cgMap) {
			const ChipPtrByChipIDMap &chipMap = n.second->GetChips();
			for (const auto &m : chipMap) {
				bool isMatch = false;
				if (typeIndex != -1 && m.second->GetChipTypeIndex() != typeIndex)
					continue;

				if (match(m.second->GetName(), matchWholeString, pattern))
					isMatch = true;

				if (!isMatch && searchComments) {
					ChipEditorData *data = m.second->GetChipEditorData();
					if (!data || data->comment.empty())
						continue;
					if (match(data->comment, matchWholeString, pattern))
						isMatch = true;
				}
				if (isMatch) {
					QStringList sl;
					const ChipInfo *ci = engine->GetChipManager()->GetChipInfo(m.second->GetChipTypeIndex());
					sl << TOQSTRING(m.second->GetName()) << TOQSTRING(n.second->GetName()) << TOQSTRING(ci->chipDesc.name);
					QTreeWidgetItem *chItem = new QTreeWidgetItem(ui.treeWidget, sl);
					chItem->setData(0, Qt::UserRole, m.first);
					chItem->setData(1, Qt::UserRole, n.second->GetID());
				}
			}
		}
	}
	catch( std::regex_error err)
	{
		String e(MTEXT("Regular Expression Error: "));
		switch (err.code()) {
		case std::regex_constants::error_collate:
			e += MTEXT("An invalid collating element was specified in a [[.name.]] block.");
			break;
		case std::regex_constants::error_ctype:
			e += MTEXT("An invalid character class name was specified in a [[:name:]] block.");
			break;
		case std::regex_constants::error_escape:
			e += MTEXT("An invalid or trailing escape was encountered.");
			break;
		case std::regex_constants::error_backref:
			e += MTEXT("A back-reference to a non-existant marked sub-expression was encountered.");
			break;
		case std::regex_constants::error_brack:
			e += MTEXT("An invalid character set [...] was encountered.");
			break;
		case std::regex_constants::error_paren:
			e += MTEXT("Mismatched \'(\' and \')\'.");
			break;
		case std::regex_constants::error_brace:
			e += MTEXT("Mismatched \'{\' and \'}\'.");
			break;
		case std::regex_constants::error_badbrace:
			e += MTEXT("Invalid contents of a {...} block.");
			break;
		case std::regex_constants::error_range:
			e += MTEXT("A character range was invalid, for example [d-a].");
			break;
		case std::regex_constants::error_space:
			e += MTEXT("Out of memory.");
			break;
		case std::regex_constants::error_badrepeat:
			e += MTEXT("An attempt to repeat something that can not be repeated - for example a*+");
			break;
		case std::regex_constants::error_complexity:
			e += MTEXT("The expression became too complex to handle.");
			break;
		case std::regex_constants::error_stack:
			e += MTEXT("Out of program stack space.");
			break;
		case std::regex_constants::error_parse:
			e += MTEXT("Parsing error."); // !!
			break;
		case std::regex_constants::error_syntax:
			e += MTEXT("Syntax error."); // !!
			break;
		default:
			e += MTEXT("Unknown error.");
		}
		QMessageBox::critical(this, "Error", TOQSTRING(e));
	}
}

void SearchWidget::updateChipList()
{
	ui.comboBoxType->clear();
	ui.comboBoxType->addItem("All Types", -1);

	const PacketPtrByStringMap &pm = engine->GetChipManager()->GetPacketMap();

	Set<std::pair<String, unsigned>> s;
	for (const auto &n : pm) {
		for (const auto &m : n.second->chips) {
			if (m.second->chipDesc.usage == ChipDesc::HIDDEN)
				continue;
			s.insert(std::make_pair(m.second->chipDesc.name,  (unsigned)m.second->chipTypeIndex));
		}
	}
	for (const auto &n : s)
		ui.comboBoxType->addItem(TOQSTRING(n.first), n.second);
	ui.comboBoxType->setCurrentIndex(0);
}

void SearchWidget::clearResults()
{
	ui.treeWidget->clear();
}

void SearchWidget::_show(QTreeWidgetItem *item, bool showDialog)
{
	if (!item)
		return;
	ChipID cid = (ChipID)item->data(0, Qt::UserRole).toUInt();
	ClassID cgid = (ClassID)item->data(1, Qt::UserRole).toUInt();
	ClassExt *cg = dynamic_cast<ClassExt*>(engine->GetClassManager()->GetClass(cgid));
	if (!cg)
		return;
	Chip *ch = cg->GetChip(cid);
	if (!ch)
		return;
	if (showDialog)
		AppSignals::instance().openChipDialog(ch);
	else
		AppSignals::instance().openClass(cg, ch);
}

void SearchWidget::resultDblClick(QTreeWidgetItem * item, int column)
{
	_show(item, false);
}

void SearchWidget::resultMenuRequested(const QPoint &pos)
{
	QTreeWidgetItem *item = ui.treeWidget->itemAt(pos);
	_aShowChip->setEnabled(item != nullptr);
	_aShowDialog->setEnabled(item != nullptr);
	_aClear->setEnabled(ui.treeWidget->topLevelItemCount() != 0);
	QAction *a = _menu->exec(ui.treeWidget->viewport()->mapToGlobal(pos));
	if (a == _aShowChip)
		_show(item, false);
	else if (a == _aShowDialog)
		_show(item, true);
	else if (a == _aClear)
		clearResults();
}
