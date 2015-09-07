/***************************************************************************
 *   Copyright (C) 2008,2009 by Volker Lanz <vl@fidra.de>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#if !defined(TREELOG__H)

#define TREELOG__H

#include "util/libpartitionmanagerexport.h"

#include "ui_treelogbase.h"

#include "util/globallog.h"

#include <QWidget>

class QTreeWidget;

/** A tree for formatted log output.
	@author Volker Lanz <vl@fidra.de>
*/
class LIBPARTITIONMANAGERPRIVATE_EXPORT TreeLog: public QWidget, public Ui::TreeLogBase
{
	Q_OBJECT
	Q_DISABLE_COPY(TreeLog)

	public:
		TreeLog(QWidget* parent = NULL);
		~TreeLog();

	Q_SIGNALS:
		void contextMenuRequested(const QPoint&);

	public:
		void init();

	protected Q_SLOTS:
		void onNewLogMessage(Log::Level logLevel, const QString& s);
		void onHeaderContextMenu(const QPoint& pos);
		void onClearLog();
		void onSaveLog();
		void on_m_TreeLog_customContextMenuRequested(const QPoint& pos);

	protected:
		QTreeWidget& treeLog() { Q_ASSERT(m_TreeLog); return *m_TreeLog; }
		const QTreeWidget& treeLog() const { Q_ASSERT(m_TreeLog); return *m_TreeLog; }

		void loadConfig();
		void saveConfig() const;

	private:
};

#endif

