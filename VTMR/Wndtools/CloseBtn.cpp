#include "CloseBtn.h"
#include <QPainter>
CCloseBtn::CCloseBtn(QWidget *parent)
	: QPushButton(parent)
{
	ui.setupUi(this);
	m_color = QColor(255, 0, 0);
}

void CCloseBtn::setColor(QColor color)
{
	m_color = color;
}

CCloseBtn::~CCloseBtn()
{
}

void CCloseBtn::paintEvent(QPaintEvent *ev)
{
	QPainter painter(this);
	QPen pen(m_color, 3, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
	painter.setPen(pen);
	painter.drawLine(QPoint(0, 0), QPoint(width() - 1, height() - 1));
	painter.drawLine(QPoint(0, height() - 1), QPoint(width() - 1, 0));
}