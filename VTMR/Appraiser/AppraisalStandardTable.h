#pragma once

#include <QTableWidget>
#include "ui_AppraisalStandardTable.h"
#include <QStyledItemDelegate>
#include "Utils.h"

class CAppraisalStandardTable : public QTableWidget
{
	Q_OBJECT
public:
	CAppraisalStandardTable(QWidget *parent = Q_NULLPTR);
	~CAppraisalStandardTable();
	void setColumnValue(const int &columnSum, const QStringList &header);   //set header value
	void setHeaderWidth(const int &index, const int &width);    //set header and column widhth for each index
	void setHeaderHeight(const int &height);                    //set header height

	int addRowValue(const int &height, const QStringList &value);
	void addRowValue(const int &height, const QStringList &value, const QIcon &fileIcon);
	void setRowH(const int &index, const int &height);
	void setItemFixed(bool flag);
	bool getSelectedRow(QList<int> &rowList);
	void addTable(VTAppraisalStandard& info, bool check = false);
	void getStandard(QVector<VTAppraisalStandard>& standards);
	void setStandard(QVector<VTAppraisalStandard>& standards);
	void setEnabled(bool enable);

protected:
	void contextMenuEvent(QContextMenuEvent *event);

private:
	void createActions();

signals:
	void sigDelete(QString id);
private slots:
	void onDelete();
private:
	int m_tableWidth;
	int m_tableHeight;
	QList<int> m_rowHeghtList;
	QList<int> m_rowWidthList;

	QMenu *m_popMenu;
	QAction *m_actionAdd;
	QAction *m_actionDelete;
	QTableWidgetItem *m_rightClickedItem;
	QMap<QTableWidgetItem *, QString> m_fileMap;

private:
	Ui::StandardTable ui;
};

// custom item delegate class
class StandardNoFocusDelegate : public QStyledItemDelegate
{
protected:
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};