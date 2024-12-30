#include "InputWidget.h"
#include <QListWidgetItem>
#include "PointItem.h"
#include <QPainter>
#include <QStyleOption>
#include "PointsManage.h"
#include <QDebug>

CInputWidget::CInputWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setMaximumSize(250, 260);
	setMinimumSize(250, 260);
	for (int n = 0; n < 5; ++n)
		m_itemList[n] = nullptr;
	connect(this, SIGNAL(sigValueFChanged(int, QPointF)), this, SLOT(onValueFChanged(int, QPointF)));

	ui.listWidget->setSpacing(0);
	ui.listWidget->setResizeMode(QListView::Fixed);
	ui.listWidget->setMovement(QListView::Static);//static 避免新增item， 导致原有item样式布局位置geometry问题
	ui.listWidget->setWordWrap(true);
	ui.listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
}

void CInputWidget::onValueFChanged(int index, QPointF point)
{
	m_actualPoints[index] = point;
}

CInputWidget::~CInputWidget()
{
}

void CInputWidget::addItem(int index) 
{
	auto item = new QListWidgetItem;
	auto pointItem = new CPointItem(index);
	m_itemList[index] = pointItem;

	if (INVALID_VALUE_FLOAT != m_actualPoints[index].x())
		pointItem->setVelueFX(m_actualPoints[index].x());
	if (INVALID_VALUE_FLOAT != m_actualPoints[index].y())
		pointItem->setVelueFY(m_actualPoints[index].y());

	item->setSizeHint(pointItem->size());
	getListWidget()->addItem(item);
	getListWidget()->setItemWidget(item, pointItem);
	connect(pointItem, &CPointItem::onValueFChanged,
		[=](QPointF point) {
			emit onValueFChanged(index, point);
		}
	);
};

void CInputWidget::paintEvent(QPaintEvent *e) {
	QStyleOption opt;
	opt.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
	QWidget::paintEvent(e);
}