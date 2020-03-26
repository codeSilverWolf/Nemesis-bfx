#ifndef BEHAVIORLISTMODEL_H_
#define BEHAVIORLISTMODEL_H_

#include <QObject>
#include <QDataStream>
#include <QMimeData>
#include <QIODevice>
#include <QAbstractItemModel>

#include <boost/date_time/posix_time/posix_time.hpp>

#include "ui/BehaviorInfo.h"

class BehaviorListModel : public QAbstractItemModel
{
	Q_OBJECT

public:

	enum EColumn {
		COL_NAME,
		COL_AUTHOR,
		COL_PRIORITY,

		COL_LASTCOL = COL_PRIORITY
	};

	BehaviorListModel(const QList<BehaviorInfo>& string, QObject* parent = nullptr) : QAbstractItemModel(parent), behaviorList(string) {}

	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	int columnCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex& index, int role) const;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex& index) const;
	Qt::DropActions supportedDropActions() const;
	QStringList mimeTypes() const;
	QMimeData* mimeData(const QModelIndexList& indexes) const;
	bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);

	Qt::ItemFlags flags(const QModelIndex& index) const;
	bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
	bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());

public slots:
	void dropModeUpdate(bool dropOnItems);
	void goToUrl(const QModelIndex& index);

private:
	QList<BehaviorInfo> behaviorList;
	Qt::CheckState tempCheck;
	boost::posix_time::ptime click_time = boost::posix_time::microsec_clock::local_time();
	bool m_DropOnItems;
	int draggedIndex = 0;
};

#endif