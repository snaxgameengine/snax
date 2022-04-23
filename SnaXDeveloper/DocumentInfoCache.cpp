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
#include "DocumentInfoCache.h"
#include "M3DEngine/Function.h"
#include "Paths.h"
#include "FileIconProvider.h"
#include "M3DEngine/DocumentLoader.h"
#include "M3DEngine/DocumentFileTypes.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ChipManager.h"
#include "App.h"

using namespace m3d;

struct FunctionInfo
{
	QString name;
	Guid type;
	QString description;
	m3d::Function::Type ft;
	m3d::Function::Access fa;
};

struct ClassInfo
{
	QString name;
	QString description;
	bool hasStartchip;
	QList<FunctionInfo> functions;
};

struct DocumentInfo
{
	qint64 size;
	QDateTime lastModified;
	QList<ClassInfo> classes;
};

Q_DECLARE_METATYPE(QList<FunctionInfo>)
Q_DECLARE_METATYPE(QList<ClassInfo>)
Q_DECLARE_METATYPE(Guid)
Q_DECLARE_METATYPE(FunctionInfo)
Q_DECLARE_METATYPE(ClassInfo)
Q_DECLARE_METATYPE(DocumentInfo)

QDataStream& operator<<(QDataStream& out, const Guid& rhs) { out.writeRawData((char*)&rhs, sizeof(Guid)); return out; }
QDataStream& operator>>(QDataStream& in, Guid& rhs) { in.readRawData((char*)&rhs, sizeof(Guid)); return in; }
QDataStream& operator<<(QDataStream& out, const FunctionInfo& rhs) { return out << rhs.name << rhs.type << rhs.description << (unsigned)rhs.ft << (unsigned)rhs.fa; }
QDataStream& operator>>(QDataStream& in, FunctionInfo& rhs) { return in >> rhs.name >> rhs.type >> rhs.description >> (unsigned&)rhs.ft >> (unsigned&)rhs.fa; }
QDataStream& operator<<(QDataStream& out, const ClassInfo& rhs) { return out << rhs.name << rhs.description << rhs.hasStartchip << rhs.functions; }
QDataStream& operator>>(QDataStream& in, ClassInfo& rhs) { return in >> rhs.name >> rhs.description >> rhs.hasStartchip >> rhs.functions; }
QDataStream& operator>>(QDataStream& istream, DocumentInfo& fi) { return istream >> fi.size >> fi.lastModified >> fi.classes; }
QDataStream& operator<<(QDataStream& ostream, const DocumentInfo& fi) { return ostream << fi.size << fi.lastModified << fi.classes; }


DocumentInfoCache::DocumentInfoCache(QString dirKey, QString dirDefaultValue, QString cacheKey, QString cacheDefaultFile, bool includeFunctions) : _dirKey(dirKey), _dirDefaultValue(dirDefaultValue), _cacheKey(cacheKey), _cacheDefaultFile(cacheDefaultFile), _includeFunctions(includeFunctions)
{
	UpdateFromSettings();
}

void DocumentInfoCache::UpdateFromSettings()
{
	QSettings s = ((const App*)QApplication::instance())->GetSettings();

	QString tempDirs = s.value(_dirKey, _dirDefaultValue).toString();
	Paths::SplitAndResolve(tempDirs, _templatesPaths);

	Path p;
	if (Paths::Resolve(s.value(_cacheKey, _cacheDefaultFile).toString(), p, false))
		_cacheFileName = TOQSTRING(p.AsString());
}

bool DocumentInfoCache::UpdateAndBuildTree(QTreeWidgetItem* root, bool addOnlyClassesWithStartChip, bool hideDocs, bool useFilePathTooltips) const
{
	QMap<QString, QVariant> cache;

	if (!_cacheFileName.isEmpty())
	{
		QFile cacheFile(_cacheFileName);
		if (cacheFile.exists()) {
			msg(DINFO, strUtils::ConstructString(MTEXT("Opens cache-file \'%1\' for read...")).arg(FROMQSTRING(_cacheFileName)));
			if (cacheFile.open(QIODevice::ReadOnly)) {
				QDataStream in(&cacheFile);
				in >> cache;
			}
			else
				msg(WARN, strUtils::ConstructString(MTEXT("Failed to open cache-file \'%1\' for read.")).arg(FROMQSTRING(_cacheFileName)));
		}
	}

	for (size_t i = 0; i < _templatesPaths.size(); i++) {
		int count = 0;

		_searchDir(root, QDir(TOQSTRING(_templatesPaths[i].AsString())), "Main/", cache, addOnlyClassesWithStartChip, hideDocs, useFilePathTooltips, count);
	}

	if (!_cacheFileName.isEmpty())
	{
		QDir d = QFileInfo(_cacheFileName).absoluteDir();
		if (!d.exists())
			d.mkpath(".");
		QFile cacheFile(_cacheFileName);
		msg(DINFO, strUtils::ConstructString(MTEXT("Opens cache-file \'%1\' for write...")).arg(FROMQSTRING(_cacheFileName)));
		if (cacheFile.open(QIODevice::WriteOnly)) {
			QDataStream out(&cacheFile);
			out << cache;
		}
		else
			msg(WARN, strUtils::ConstructString(MTEXT("Failed to open cache-file \'%1\' for write.")).arg(FROMQSTRING(_cacheFileName)));
	}

	return true;
}

void DocumentInfoCache::_searchDir(QTreeWidgetItem* treeItem, QDir dir, QString rPath, QMap<QString, QVariant>& templateCache, bool addOnlyClassesWithStartChip, bool hideDocs, bool useFilePathTooltips, int& count) const
{
	static const QIcon classIcon = QIcon(":/EditorApp/Resources/graph.png");
	static const QIcon ficons[3] = { QIcon(":/EditorApp/Resources/static-func.png"), QIcon(":/EditorApp/Resources/nonvirt-func.png"), QIcon(":/EditorApp/Resources/virt-func.png") };

	QFileInfoList lst = dir.entryInfoList(QStringList() << "*.m3j" << "*.m3b" << "*.m3x", QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
	for (const QFileInfo& nfo : lst) {
		if (nfo.isDir()) {
			QString f = nfo.fileName();
			QTreeWidgetItem* item = new DocumentInfoCacheTreeWidgetItem(treeItem, QStringList(nfo.fileName()));
			_searchDir(item, nfo.absoluteFilePath(), rPath + nfo.fileName() + "/", templateCache, addOnlyClassesWithStartChip, hideDocs, useFilePathTooltips, count);
			if (item->childCount() == 0)
				delete item;
			else {
				item->setIcon(0, FileIconProvider::icon(nfo));
				item->setData(DATA_ITEM_TYPE, IT_FOLDER); 
				item->setData(DATA_ITEM_PATH, nfo.absoluteFilePath());
				item->setToolTip(0, nfo.absoluteFilePath());
			}
		}
		else {
			QMap<QString, QVariant>::iterator itr = templateCache.find(nfo.absoluteFilePath());

			if (nfo.isHidden() || !nfo.exists()) {
				if (itr != templateCache.end())
					templateCache.erase(itr);
				continue;
			}

			DocumentInfo fileInfo;

			if (itr != templateCache.end()) { // Found file in cache?
				fileInfo = itr.value().value<DocumentInfo>();
				if (fileInfo.lastModified != nfo.lastModified() || fileInfo.size != nfo.size()) { // check for changes...
					msg(DINFO, MTEXT("Updating template cache for \'") + FROMQSTRING(nfo.absoluteFilePath()) + MTEXT("\'."));
					templateCache.erase(itr); // cache out of date!
					itr = templateCache.end();
				}
			}

			Path p = FROMQSTRING(nfo.absoluteFilePath());

			if (itr == templateCache.end()) { // file not in cache?
				List<ClassMeta> cgList;
				const DocumentFileTypes::FileTypeDesc* ft = DocumentFileTypes::GetFileType(p);
				if (ft) {
					DocumentLoader* loader = (*ft->loader)();
					if (loader) {
						if (loader->OpenFile(p)) {
							if (!loader->SearchForClasses(cgList, _includeFunctions)) {
								msg(WARN, MTEXT("Failed to search template document \'") + p.AsString() + MTEXT("\'."));
								cgList.clear();
							}
						}
						else
							msg(WARN, MTEXT("Failed to open template document \'") + p.AsString() + MTEXT("\'."));
						DocumentFileTypes::Free(loader);
					}
					else
						msg(WARN, MTEXT("Failed to create template loader for \'") + p.AsString() + MTEXT("\'."));
				}
				else
					msg(WARN, MTEXT("Could not find template loader for \'") + p.AsString() + MTEXT("\'."));
				fileInfo.size = nfo.size();
				fileInfo.lastModified = nfo.lastModified();
				fileInfo.classes.clear();
				for (size_t i = 0; i < cgList.size(); i++) {
					ClassInfo ci;
					ci.name = TOQSTRING(cgList[i].name);
					ci.description = TOQSTRING(cgList[i].description);
					ci.hasStartchip = cgList[i].hasStartchip;
					for (size_t j = 0; j < cgList[i].functions.size(); j++) {
						FunctionInfo fi;
						fi.name = TOQSTRING(cgList[i].functions[j].name);
						fi.type = cgList[i].functions[j].type;
						fi.description = TOQSTRING(cgList[i].functions[j].comment);
						fi.ft = cgList[i].functions[j].ft;
						fi.fa = cgList[i].functions[j].fa;
						ci.functions.push_back(fi);
					}
					fileInfo.classes.push_back(ci);
				}
				itr = templateCache.insert(nfo.absoluteFilePath(), QVariant::fromValue(fileInfo));
			}

			count += fileInfo.classes.size();

			if (itr != templateCache.end()) {
				// Insert from cache.
				bool hasTemplates = fileInfo.classes.size() > 0;
				if (addOnlyClassesWithStartChip) {
					hasTemplates = false;
					for (int i = 0; i < fileInfo.classes.size(); i++) {
						if (fileInfo.classes[i].hasStartchip) {
							hasTemplates = true;
							break;
						}
					}
				}
				if (hasTemplates) {
					QTreeWidgetItem* item = treeItem;
					QString fn = nfo.baseName();
					if (!hideDocs) {
						item = new DocumentInfoCacheTreeWidgetItem(treeItem, QStringList(fn));
						const DocumentFileTypes::FileTypeDesc* d = DocumentFileTypes::GetFileType(p);
						QIcon icon = FileIconProvider::icon(nfo);
						item->setIcon(0, icon);
						item->setToolTip(0, nfo.absoluteFilePath());
						item->setData(DATA_ITEM_TYPE, IT_DOCUMENT);
						item->setData(DATA_ITEM_PATH, nfo.absoluteFilePath());
					}
					for (const ClassInfo &ti : fileInfo.classes) {
						if (addOnlyClassesWithStartChip && !ti.hasStartchip)
							continue;
						QTreeWidgetItem* jtem = new DocumentInfoCacheTreeWidgetItem(item, QStringList(ti.name));
						QString tempPath = rPath + fn + ":" + ti.name;
						jtem->setIcon(0, classIcon);
						jtem->setData(DATA_ITEM_TYPE, IT_CLASS);
						jtem->setData(DATA_ITEM_PATH, nfo.absoluteFilePath());
						jtem->setData(DATA_ITEM_NAME, ti.name);
						jtem->setData(DATA_ITEM_TEMPPATH, tempPath);
						jtem->setData(DATA_ITEM_DESC, ti.description);
						if (useFilePathTooltips) {
							jtem->setToolTip(0, nfo.absoluteFilePath());
						}
						else {
							QString tt = ti.description.isEmpty() ? ti.name : ti.description;
							if (tt.length() > 250)
								tt = tt.left(247) + "...";
							if (tt.isEmpty())
								jtem->setToolTip(0, QString());
							else
								jtem->setToolTip(0, "<font>" + tt.toHtmlEscaped() + "</font>");
						}
						if (_includeFunctions) {
							for (const FunctionInfo& fi : ti.functions) {
								if (fi.fa == Function::Access::Private)
									continue; // Skip private functions!
								const ChipInfo* nfo = engine->GetChipManager()->GetChipInfo(fi.type);

								QTreeWidgetItem* fitm = new DocumentInfoCacheTreeWidgetItem(jtem, QStringList() << fi.name << ("    " + TOQSTRING(nfo->chipDesc.name)));
								QString tempPath = rPath + fn + ":" + ti.name;
								fitm->setIcon(0, ficons[(uint32)fi.ft < 3 ? (uint32)fi.ft : 0]);
								fitm->setData(DATA_ITEM_TYPE, IT_FUNCTION);
								//jtem->setData(DATA_ITEM_PATH, nfo.absoluteFilePath());
								fitm->setData(DATA_ITEM_NAME, fi.name);
								//jtem->setData(DATA_ITEM_TEMPPATH, tempPath);
								fitm->setData(DATA_ITEM_DESC, fi.description);
								{
									QString tt = fi.description.isEmpty() ? fi.name : fi.description;
									if (tt.length() > 250)
										tt = tt.left(247) + "...";
									if (tt.isEmpty())
										fitm->setToolTip(0, QString());
									else
										fitm->setToolTip(0, "<font>" + tt.toHtmlEscaped() + "</font>");
								}
							}
						}
					}
				}
			}
		}
	}
}