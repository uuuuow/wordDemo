#include "WaterMarkWnd.h"
#include <QPainter>
#include "Utils.h"

CWaterMarkWnd::CWaterMarkWnd(QColor color, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	m_color = color;
	setWindowFlags(Qt::FramelessWindowHint);
	ui.BtnClose->setColor(color);
	
	ui.BtnClose->setMaximumSize(QSize(24, 24));
	ui.BtnClose->setMinimumSize(QSize(24, 24));

	connect(ui.BtnClose, &QPushButton::clicked, [=]() {
		emit sigClose(this);
	});
	connect(this, SIGNAL(sigClose(QWidget*)), parent, SLOT(onWaterMarkWndClose(QWidget*)));
}

CWaterMarkWnd::~CWaterMarkWnd()
{
}

void CWaterMarkWnd::setRect(QRect wm, QRect image)
{
	m_waterMarkRect = wm;
	m_imageRect = image;
}

void CWaterMarkWnd::moveWnd(QRect r)
{
	QRect dst(0, 0, 0, 0);
	if (m_waterMarkRect != QRect(0, 0, 0, 0))
	{
		VTTransRect(m_waterMarkRect, m_imageRect, r, dst);
		if (dst != QRect(0, 0, 0, 0))
		{
			setMaximumSize(dst.width(), dst.height());
			setMinimumSize(dst.width(), dst.height());
			this->move(dst.x(), dst.y());
		}
	}
}

void CWaterMarkWnd::paintEvent(QPaintEvent *ev)
{
	QPainter painter(this);
	QPen pen(m_color, 3, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
	painter.setPen(pen);
	painter.drawLine(QPoint(0, 0), QPoint(width() - 1, 0));
	painter.drawLine(QPoint(0, height() - 1), QPoint(width() - 1, height() - 1));
	painter.drawLine(QPoint(0, 0), QPoint(0, height() - 1));
	painter.drawLine(QPoint(width() - 1, 0), QPoint(width() - 1, height() - 1));
}

void CWaterMarkWnd::resizeEvent(QResizeEvent *ev)
{
	int x = 0;
	int y = 0;
	if (width() >= 80)
		x = width() - 40;
	if (height() >= 20)
		y = 12;
	ui.BtnClose->move(x, y);
}