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
#include "Skeleton_Dlg.h"
#include "ChipDialogs/ChipDialogManager.h"

using namespace m3d;


DIALOGDESC_DEF(Skeleton_Dlg, SKELETON_GUID);


class AnimationItemDelegate : public QStyledItemDelegate
{
public:
	AnimationItemDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}
	virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		if (index.column() == 3)
			return nullptr;
		return QStyledItemDelegate::createEditor(parent, option, index);
	}

	virtual void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
	{
		QLineEdit *edt = dynamic_cast<QLineEdit*>(editor);
		if (!edt)
			return;
		QString txt = edt->text().trimmed();

		Skeleton *skeleton = ((Skeleton_Dlg*)parent())->GetChip();
		if (!skeleton)
			return;

		QModelIndex i = index.sibling(index.row(), 0);
		QString name = i.data().toString();

		Skeleton::Animation *a = skeleton->GetAnimation(FROMQSTRING(name));
		if (!a)
			return;

		if (index.column() == 0) {
			if (txt == name || txt.isEmpty())
				return;
			if (skeleton->GetAnimation(FROMQSTRING(txt)) != nullptr)
				return;
			if (!skeleton->RenameAnimation(FROMQSTRING(name), FROMQSTRING(txt)))
				return;

		}
		else if (index.column() == 1) {
			bool ok = false;
			uint32 p = txt.toUInt(&ok);
			if (p == a->priority)
				return;
			a->priority = p;
		}
		else if (index.column() == 2) {
			bool ok = false;
			float32 p = txt.toFloat(&ok);
			if (p == a->multiplier || p < 0.00001f)
				return;
			a->multiplier = p;
		}
		((Skeleton_Dlg*)parent())->SetDirty();
		return QStyledItemDelegate::setModelData(editor, model, index);
	}
};

class BoneAnimationItemDelegate : public QStyledItemDelegate
{
public:
	BoneAnimationItemDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}
	virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		if (index.column() != 0)
			return nullptr;
		return QStyledItemDelegate::createEditor(parent, option, index);
	}

	virtual void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
	{
		QLineEdit *edt = dynamic_cast<QLineEdit*>(editor);
		if (!edt)
			return;
		QString txt = edt->text().trimmed();

		Skeleton *skeleton = ((Skeleton_Dlg*)parent())->GetChip();
		if (!skeleton)
			return;

		QModelIndex i = index.sibling(index.row(), 0);
		QString anim = i.data(Qt::UserRole).toString();
		QString oldName = i.data().toString();

		Skeleton::Animation *a = skeleton->GetAnimation(FROMQSTRING(anim));
		if (!a)
			return;

		if (index.column() == 0) {
			if (txt == oldName || txt.isEmpty())
				return;
			if (a->keyframes.find(FROMQSTRING(txt)) != a->keyframes.end())
				return;
			auto itr = a->keyframes.find(FROMQSTRING(oldName));
			if (itr == a->keyframes.end())
				return;
			a->keyframes.insert(std::make_pair(FROMQSTRING(txt), itr->second));
			a->keyframes.erase(itr);
			model->setData(index, txt, 0);
			QMap<QString, QTreeWidgetItem*> boneItems; // List of all bones in our skeleton!
			for (QTreeWidgetItemIterator it(((Skeleton_Dlg*)parent())->GetBonesWidget()); *it; it++) {
				boneItems.insert((*it)->text(0), *it);
			}
			auto jtr = boneItems.find(oldName);
			if (jtr != boneItems.end())
				jtr.value()->setData(0, Qt::ForegroundRole, QColor(255, 0, 0));
			jtr = boneItems.find(txt);
			if (jtr != boneItems.end()) {
				jtr.value()->setData(0, Qt::ForegroundRole, QColor(0, 0, 0));
				model->setData(i, QColor(0, 0, 0), Qt::ForegroundRole);
			}
			else {
				model->setData(i, QColor(255, 0, 0), Qt::ForegroundRole);
			}
		}
		((Skeleton_Dlg*)parent())->SetDirty();
		return QStyledItemDelegate::setModelData(editor, model, index);
	}
};

Skeleton_Dlg::Skeleton_Dlg()
{
	ui.setupUi(this);

	ui.treeWidget_animations->setItemDelegate(new AnimationItemDelegate(this));
	ui.treeWidget_animationBones->setItemDelegate(new BoneAnimationItemDelegate(this));

	connect(ui.treeWidget_animations, &QTreeWidget::currentItemChanged, this, &Skeleton_Dlg::animationSelected);
	connect(ui.treeWidget_animations, &QTreeWidget::currentItemChanged, this, &Skeleton_Dlg::updateActions);
	connect(ui.treeWidget_animationBones, &QTreeWidget::currentItemChanged, this, &Skeleton_Dlg::updateActions);

	_actAnimCut = new QAction("Cut", ui.treeWidget_animations);
	_actAnimCopy = new QAction("Copy", ui.treeWidget_animations);
	_actAnimPaste = new QAction("Paste", ui.treeWidget_animations);
	_actAnimDelete = new QAction("Delete", ui.treeWidget_animations);
	_actAnimCut->setShortcut(QKeySequence::Cut);
	_actAnimCopy->setShortcut(QKeySequence::Copy);
	_actAnimPaste->setShortcut(QKeySequence::Paste);
	_actAnimDelete->setShortcut(QKeySequence::Delete);
	ui.treeWidget_animations->addAction(_actAnimCut);
	ui.treeWidget_animations->addAction(_actAnimCopy);
	ui.treeWidget_animations->addAction(_actAnimPaste);
	ui.treeWidget_animations->addAction(_actAnimDelete);
	connect(_actAnimCut, &QAction::triggered, this, &Skeleton_Dlg::onAnimCut);
	connect(_actAnimCopy, &QAction::triggered, this, &Skeleton_Dlg::onAnimCopy);
	connect(_actAnimPaste, &QAction::triggered, this, &Skeleton_Dlg::onAnimPaste);
	connect(_actAnimDelete, &QAction::triggered, this, &Skeleton_Dlg::onAnimDelete);
	_actAnimBonesDelete = new QAction("Delete", ui.treeWidget_animationBones);
	_actAnimBonesReplace = new QAction("Replace in Name(s)...", ui.treeWidget_animationBones);
	_actAnimBonesDelete->setShortcut(QKeySequence::Delete);
	ui.treeWidget_animationBones->addAction(_actAnimBonesDelete);
	ui.treeWidget_animationBones->addAction(_actAnimBonesReplace);
	connect(_actAnimBonesDelete, &QAction::triggered, this, &Skeleton_Dlg::onBoneAnimDelete);
	connect(_actAnimBonesReplace, &QAction::triggered, this, &Skeleton_Dlg::onBoneAnimReplace);
}

Skeleton_Dlg::~Skeleton_Dlg()
{
}

void _buildBoneTree(QTreeWidgetItem *p, const Skeleton::Joint &j)
{
	QStringList lst;
	lst << TOQSTRING(j.name) << (j.index == -1 ? "" : QString::number(j.index));
	QTreeWidgetItem *itm = new QTreeWidgetItem(p, lst);
	for (size_t i = 0; i < j.children.size(); i++)
		_buildBoneTree(itm, j.children[i]);
}

QTreeWidgetItem *_addAnimationItem(QTreeWidget *w, String name, const Skeleton::Animation &a)
{
	QStringList lst;
	lst << TOQSTRING(name) << QString::number(a.priority) << QString::number(a.multiplier) << QString::number(a.duration);
	QTreeWidgetItem *itm = new QTreeWidgetItem(lst);
	itm->setFlags(itm->flags() | Qt::ItemIsEditable);
	itm->setData(0, Qt::UserRole, TOQSTRING(name));
	w->invisibleRootItem()->addChild(itm);
	return itm;
}



void Skeleton_Dlg::Init()
{
	Skeleton *s = GetChip();

	_root = s->GetRootJoint();
	_animations = s->GetAnimations();

	_buildBoneTree(ui.treeWidget_bones->invisibleRootItem(), s->GetRootJoint());
	ui.treeWidget_bones->expandAll();

	const Map<String, Skeleton::Animation> &a = s->GetAnimations();
	for (const auto &n : a)
		_addAnimationItem(ui.treeWidget_animations, n.first, n.second);

}

void Skeleton_Dlg::OnCancel()
{
	Skeleton *s = GetChip();
	s->SetRootJoint(_root);
	s->SetAnimations(_animations);
}

void Skeleton_Dlg::AfterApply()
{
	Skeleton *s = GetChip();
	_root = s->GetRootJoint();
	_animations = s->GetAnimations();
}

void Skeleton_Dlg::animationSelected()
{
	QMap<QString, QTreeWidgetItem*> boneItems; // List of all bones in our skeleton!
	for (QTreeWidgetItemIterator it(ui.treeWidget_bones); *it; it++) {
		boneItems.insert((*it)->text(0), *it);
		(*it)->setData(0, Qt::ForegroundRole, QColor(0,0,0)); // Clear text color.
	}

	QTreeWidgetItem *itm = ui.treeWidget_animations->currentItem();
	ui.treeWidget_animationBones->clear();
	if (!itm)
		return;

	String name = FROMQSTRING(itm->text(0));
	Skeleton::Animation *a = GetChip()->GetAnimation(name);
	if (!a)
		return;

	for (const auto &m : a->keyframes) {
		QStringList lst;
		lst << TOQSTRING(m.first) << QString::number(m.second.size()) << (m.second.size() ? QString::number(m.second.back().time) : "");
		QTreeWidgetItem *jtm = new QTreeWidgetItem(ui.treeWidget_animationBones, lst);
		jtm->setData(0, Qt::UserRole, itm->text(0)); // Name of anmimation..
		jtm->setFlags(jtm->flags() | Qt::ItemIsEditable);

		QMap<QString, QTreeWidgetItem*>::iterator itr = boneItems.find(TOQSTRING(m.first));
		// Mark animation bones with a red color if they are unmatched in the main bones tree.
		if (itr == boneItems.end())
			jtm->setData(0, Qt::ForegroundRole, QColor(255,0,0));
		else
			boneItems.erase(itr);
	}

	for (QMap<QString, QTreeWidgetItem*>::ConstIterator itr = boneItems.cbegin(); itr != boneItems.cend(); itr++)
		(*itr)->setData(0, Qt::ForegroundRole, QColor(255,0,0)); // Set text color for bones that does NOT have a matching animation-bone.
}

void Skeleton_Dlg::updateActions()
{
	bool a = ui.treeWidget_animations->currentItem() != nullptr;
	bool b = ui.treeWidget_animationBones->currentItem() != nullptr;

	_actAnimCut->setEnabled(a);
	_actAnimCopy->setEnabled(a);
	_actAnimPaste->setEnabled(a);
	_actAnimDelete->setEnabled(a);

	_actAnimBonesDelete->setEnabled(b);
	_actAnimBonesReplace->setEnabled(b);
}

void Skeleton_Dlg::onAnimCut()
{
	onAnimCopy();
	onAnimDelete();
}

void Skeleton_Dlg::onAnimCopy()
{
	QTreeWidgetItem *itm = ui.treeWidget_animations->currentItem();
	if (!itm)
		return;

	String name = FROMQSTRING(itm->text(0));

	Skeleton::Animation *a = GetChip()->GetAnimation(name);
	if (!a)
		return;

	QByteArray ba;
	QDataStream str(&ba, QIODevice::WriteOnly);
	str << TOQSTRING(name);
	str << a->multiplier;
	str << a->priority;
	str << a->keyframes.size();
	for (const auto &n : a->keyframes) {
		str << TOQSTRING(n.first);
		str << n.second.size();
		if (n.second.size())
			str.writeRawData((const char*)&n.second.front(), sizeof(Skeleton::Keyframe) * n.second.size());
	}
	QMimeData *data = GetDialogManager()->CreateMimeData();
	data->setData("SKEL_ANI", ba);
	QApplication::clipboard()->setMimeData(data);
}

void Skeleton_Dlg::onAnimPaste()
{
	QByteArray ba = QApplication::clipboard()->mimeData()->data("SKEL_ANI");
	if (ba.isEmpty())
		return;
	QDataStream str(&ba, QIODevice::ReadOnly);
	QString qname;
	str >> qname;
	String name = FROMQSTRING(qname), n2 = MTEXT("");
	Skeleton::Animation *ani = nullptr;
	for (uint32 i = 1; (ani = GetChip()->AddAnimation(name + n2)) == nullptr; n2 = MTEXT(" ") + strUtils::fromNum(++i));
	name += n2;
	str >> ani->multiplier;
	str >> ani->priority;
	size_t count = 0;
	str >> count;
	for (size_t i = 0; i < count; i++) {
		str >> qname;
		Skeleton::KeyframeList &kf = ani->keyframes.insert(std::make_pair(FROMQSTRING(qname), Skeleton::KeyframeList())).first->second;
		size_t c = 0;
		str >> c;
		if (c) {
			kf.resize(c);
			str.readRawData((char*)&kf.front(), sizeof(Skeleton::Keyframe) * c);
		}
	}
	GetChip()->CalculateAnimationDuration(ani);
	QTreeWidgetItem *itm = _addAnimationItem(ui.treeWidget_animations, name, *ani);
	ui.treeWidget_animations->clearSelection();
	if (itm) {
		ui.treeWidget_animations->scrollToItem(itm);
		itm->setSelected(true);
	}
	SetDirty();
}

void Skeleton_Dlg::onAnimDelete()
{
	QTreeWidgetItem *itm = ui.treeWidget_animations->currentItem();
	if (!itm)
		return;

	String name = FROMQSTRING(itm->text(0));

	if (GetChip()->RemoveAnimation(name))
		delete itm;
	SetDirty();
}

void Skeleton_Dlg::onBoneAnimDelete()
{
	QList<QTreeWidgetItem*> itms = ui.treeWidget_animationBones->selectedItems();
	if (itms.empty())
		return;

	QTreeWidgetItem *itm = ui.treeWidget_animations->currentItem();
	if (!itm)
		return;
	String anim = FROMQSTRING(itm->text(0));
	Skeleton::Animation *an = GetChip()->GetAnimation(anim);
	if (!an)
		return;

	size_t n = 0;
	for (int32 i = 0; i < itms.size(); i++) {
		String name = FROMQSTRING(itms[i]->text(0));
		n += an->keyframes.erase(name);
	}

	if (n > 0)
		SetDirty();
}

void Skeleton_Dlg::onBoneAnimReplace()
{
	QList<QTreeWidgetItem*> itms = ui.treeWidget_animationBones->selectedItems();
	if (itms.empty())
		return;

	QTreeWidgetItem *itm = ui.treeWidget_animations->currentItem();
	if (!itm)
		return;
	String anim = FROMQSTRING(itm->text(0));
	Skeleton::Animation *an = GetChip()->GetAnimation(anim);
	if (!an)
		return;

	QDialog *d = new QDialog(this);
	QGridLayout *l = new QGridLayout(d);
	QLineEdit *a = new QLineEdit(d);
	QLineEdit *b = new QLineEdit(d);
	QCheckBox *c = new QCheckBox("Use Regular Expression", d);
	l->addWidget(new QLabel("Find in Name:", d), 0, 0, 1, 2);
	l->addWidget(a, 1, 0, 1, 2);
	l->addWidget(new QLabel("Replace With:", d), 2, 0, 1, 2);
	l->addWidget(b, 3, 0, 1, 2);
	l->addWidget(c, 4, 0, 1, 1);
	QDialogButtonBox *dbb = new QDialogButtonBox(d);
	QPushButton *bOk = dbb->addButton(QDialogButtonBox::Ok);
	QPushButton *bCancel = dbb->addButton(QDialogButtonBox::Cancel);
	connect(bOk, &QPushButton::clicked, d, &QDialog::accept);
	connect(bCancel, &QPushButton::clicked, d, &QDialog::reject);
	l->addWidget(dbb, 5, 1, 1, 1);
	if (d->exec() == QDialog::Accepted) {
		QString before = a->text(), after = b->text();
		List<std::pair<String, String>> lst;
		Set<String> m;
		for (const auto &a : an->keyframes)
			m.insert(a.first);
		for (int32 i = 0; i < itms.size(); i++) {
			QString name = itms[i]->text(0);
			m.erase(FROMQSTRING(name));
			QString newName;
			if (c->isChecked())
				newName = QString(name).replace(QRegularExpression(before), after).trimmed();
			else
				newName = QString(name).replace(before, after).trimmed(); // TODO: RegExp...
			if (name == newName || newName.isEmpty())
				continue;
			if (m.find(FROMQSTRING(newName)) != m.end()) {
				QMessageBox::warning(this, M3D_TITLE, "Duplicate name.");
				lst.clear();
				break;
			}
			lst.push_back(std::make_pair(FROMQSTRING(name), FROMQSTRING(newName)));
		}

		if (!lst.empty()) {
			for (size_t i = 0; i < lst.size(); i++) {
				auto a = an->keyframes.find(lst[i].first);
				if (a == an->keyframes.end())
					continue;
				an->keyframes.insert(std::make_pair(lst[i].second, a->second));
			}

			animationSelected();
			SetDirty();
		}
	}

	delete d;
}

