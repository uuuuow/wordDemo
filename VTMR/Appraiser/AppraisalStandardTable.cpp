#include "AppraisalStandardTable.h"
#include <QToolTip>
#include <QMenu>
#include <QContextMenuEvent>
#include <QMessageBox>
#include <QHeaderView>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QDateTime>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

CAppraisalStandardTable::CAppraisalStandardTable(QWidget *parent)
	: QTableWidget(parent)
	, m_rightClickedItem(NULL)
{
	ui.setupUi(this);

	m_rowHeghtList.clear();
	m_rowWidthList.clear();
	m_fileMap.clear();
	this->setMouseTracking(true);
	
	horizontalHeader()->setDefaultSectionSize(100);
	verticalHeader()->setDefaultSectionSize(30);    //设置默认行高
	m_tableWidth = 100;
	m_tableHeight = 30;

	QFont font = horizontalHeader()->font();    //设置表头字体加粗
	//font.setBold(true);
	font.setFamily("微软雅黑");
	horizontalHeader()->setFont(font);
	horizontalHeader()->setStretchLastSection(true);    //设置充满表宽度
	//horizontalHeader()->setMovable(false);              //表头左右互换
	verticalHeader()->resizeSections(QHeaderView::ResizeToContents);

	setFrameShape(QFrame::NoFrame);      //设置无边框
	//setShowGrid(false);                //设置不显示格子线
	verticalHeader()->setVisible(false); //设置垂直头不可见
	setSelectionMode(QAbstractItemView::ExtendedSelection);  //可多选（Ctrl、Shift、  Ctrl+A都可以）
	setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行为时每次选择一行
	setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑

	//setStyleSheet("selection-background-color:lightblue;");  //设置选中背景色
	//horizontalHeader()->setStyleSheet("QHeaderView::section{background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(134, 245, 99, 255),stop:0.5 rgba(134, 148, 99, 255),stop:1 rgba(115, 87, 128, 255));color:rgb(25, 70, 100);padding-left: 1px;border: 1px solid #FFFF00;}"); //设置表头背景色
	setAlternatingRowColors(true);

	setItemDelegate(new StandardNoFocusDelegate());             //虚线边框去除
	horizontalHeader()->setHighlightSections(false);    //点击表时不对表头行光亮（获取焦点）

	createActions();
	setItemFixed(false);
	

	QStringList header;
	header << QString("标准号") << QString("标准全称") << QString("使用");
	setColumnValue(3, header);

	setHeaderWidth(0, 180);
	setHeaderWidth(1, 300);
	setHeaderWidth(2, 40);
	setHeaderHeight(30);
}

CAppraisalStandardTable::~CAppraisalStandardTable()
{

}

void CAppraisalStandardTable::setColumnValue(const int &columnSum, const QStringList &header)
{
	setColumnCount(columnSum);                //设置列数
	this->setHorizontalHeaderLabels(header);  //设置列的标签
}

void CAppraisalStandardTable::setHeaderWidth(const int &index, const int &width)
{
	horizontalHeader()->resizeSection(index, width);
	if (m_rowWidthList.count() <= index + 1) {
		m_rowWidthList.append(width);
	}
	else {
		m_rowWidthList[index + 1] = width;
	}
	m_tableWidth = 0;
	for (auto index = 0; index < m_rowWidthList.count(); index++)
		m_tableWidth += m_rowWidthList.at(index);
}

void CAppraisalStandardTable::setHeaderHeight(const int &height)
{
	horizontalHeader()->setFixedHeight(height);        //设置表头的高度
	if (m_rowHeghtList.isEmpty()) {
		m_rowHeghtList.append(height);
	}
	else {
		m_rowHeghtList[0] = height;
	}
	m_tableHeight = 0;
	for (auto index = 0; index < m_rowHeghtList.count(); index++)
		m_tableHeight += m_rowHeghtList.at(index);
}

int CAppraisalStandardTable::addRowValue(const int &height, const QStringList &value)
{
	auto rows = rowCount();    //获取表单行数
	insertRow(rows);          //插入新行
	setRowHeight(rows, height);
	for (auto index = 0; index < columnCount(); index++) {
		auto item = new QTableWidgetItem;
		if (index == 0) {
			item->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
			m_fileMap.insert(item, value.at(index));
		}
		else {
			item->setTextAlignment(Qt::AlignCenter);
		}
		item->setText(value.at(index));
		setItem(rows, index, item);
	}
	m_rowHeghtList.append(height);
	m_tableHeight += height;
	return rows;
}

void CAppraisalStandardTable::addRowValue(const int &height, const QStringList &value, const QIcon &fileIcon)
{
	auto rows = rowCount();    //获取表单行数
	insertRow(rows);          //插入新行
	setRowHeight(rows, height);
	for (auto index = 0; index < columnCount(); index++) {
		auto item = new QTableWidgetItem;
		if (index == 0) {
			item->setIcon(fileIcon);
			item->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
			m_fileMap.insert(item, value.at(index));
		}
		else {
			item->setTextAlignment(Qt::AlignCenter);
		}
		item->setText(value.at(index));
		setItem(rows, index, item);
	}
	m_rowHeghtList.append(height);
	m_tableHeight += height;
}

void StandardNoFocusDelegate::paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
	QStyleOptionViewItem itemOption(option);
	if (itemOption.state & QStyle::State_HasFocus)
		itemOption.state = itemOption.state ^ QStyle::State_HasFocus;
	QStyledItemDelegate::paint(painter, itemOption, index);
}

void CAppraisalStandardTable::setRowH(const int &index, const int &height)
{
	setRowHeight(index, height);
	if (m_rowHeghtList.count() <= index + 1) {
		m_rowHeghtList.append(height);
	}
	else {
		m_rowHeghtList[index + 1] = height;
	}
	m_tableHeight = 0;
	for (auto index = 0; index < m_rowHeghtList.count(); index++)
		m_tableHeight += m_rowHeghtList.at(index);
}

void CAppraisalStandardTable::createActions()
{
	m_popMenu = new QMenu();
	m_actionAdd = new QAction(this);
	m_actionDelete = new QAction(this);

	m_actionAdd->setText(QString("添加"));
	m_actionDelete->setText(QString("删除"));
	connect(m_actionAdd, SIGNAL(triggered()), parent(), SLOT(onAdd()));
	connect(m_actionDelete, SIGNAL(triggered()), this, SLOT(onDelete()));
	connect(this, SIGNAL(sigDelete(QString)), parent(), SLOT(onDelete(QString)));
}

void CAppraisalStandardTable::contextMenuEvent(QContextMenuEvent *event)
{
	m_popMenu->clear();
	QPoint point = event->pos();
	m_rightClickedItem = this->itemAt(point);
	if (m_rightClickedItem != NULL) {
		m_popMenu->addAction(m_actionAdd);
		m_popMenu->addAction(m_actionDelete);
	}
	else
		m_popMenu->addAction(m_actionAdd);
	m_popMenu->exec(QCursor::pos());
	event->accept();
	QTableWidget::contextMenuEvent(event);
}

void CAppraisalStandardTable::onDelete()
{
	auto row = currentRow();
	auto it = item(row, 0);
	emit sigDelete(it->text());
	removeRow(row);
}

void CAppraisalStandardTable::setItemFixed(bool flag)
{
	if (flag == true)
		horizontalHeader()->resizeSections(QHeaderView::Fixed);
	else
		horizontalHeader()->resizeSections(QHeaderView::Interactive);
}

bool CAppraisalStandardTable::getSelectedRow(QList<int> &rowList)
{
	//多选并获取所选行
	QList<QTableWidgetItem *> items = this->selectedItems();
	int itemCount = items.count();
	if (itemCount <= 0) {
		return false;
	}
	for (auto index = 0; index < itemCount; index++) {
		auto itemRow = this->row(items.at(index));
		rowList.append(itemRow);
	}
	return true;
}

void CAppraisalStandardTable::addTable(VTAppraisalStandard& info, bool check/* = false*/)
{
	auto box = new QCheckBox(this);
	box->setChecked(check);
	auto row = addRowValue(30, QStringList() << info.id << info.name << "");
	setCellWidget(row, 2, box);
}

void CAppraisalStandardTable::getStandard(QVector<VTAppraisalStandard> &standards)
{
	for (int r = 0; r < rowCount(); ++r)
	{
		QTableWidget;
		QTableWidgetItem;
		auto itemId = item(r, 0);
		auto itemName = item(r, 1);
		auto w = cellWidget(r, 2);
		if (!w)
			continue;
		QCheckBox* box = (QCheckBox*)w;
		if (!box->isChecked())
			continue;
		VTAppraisalStandard standard;
		standard.id = itemId->text();
		standard.name = itemName->text();
		standards.push_back(standard);
	}
}

void CAppraisalStandardTable::setStandard(QVector<VTAppraisalStandard>& standards)
{
	for (auto standard : standards) {
		bool find = false;
		for (int r = 0; r < rowCount(); ++r) {
			auto itemId = item(r, 0);
			auto itemName = item(r, 1);
			auto w = cellWidget(r, 2);
			if (!w)
				continue;
			if (itemId->text() != standard.id)
				continue;
			QCheckBox * box = (QCheckBox*)w;
			box->setChecked(true);
			find = true;
			break;
		}
		if (!find) {
			addTable(standard, true);
		}
	}
}

void CAppraisalStandardTable::setEnabled(bool enable)
{
	for (int r = 0; r < rowCount(); ++r) {
		auto w = cellWidget(r, 2);
		if (!w)
			continue;
		QCheckBox* box = (QCheckBox*)w;
		box->setEnabled(enable);
	}
}
