#include "PointItem.h"
#include <QPainter>
#include <QStyleOption>

CPointItem::CPointItem(int px, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	if (px == 0)
		ui.labelP->setText("P0");
	else if (px == 1)
		ui.labelP->setText("P1");
	else if (px == 2)
		ui.labelP->setText("P2");
	else if (px == 3)
		ui.labelP->setText("P3");
	else if (px == 4)
		ui.labelP->setText("P4");

	ui.lineEditX->setAttribute(Qt::WA_InputMethodEnabled, false);
	ui.lineEditY->setAttribute(Qt::WA_InputMethodEnabled, false);

	connect(ui.lineEditX, &QLineEdit::textChanged,
		[=](const QString &text) {
		float fX = 0; 
		float fY = 0;
		fX = text.toFloat();
		fY = ui.lineEditY->text().toFloat();
		emit onValueFChanged(QPointF(fX, fY));
	});

	connect(ui.lineEditY, &QLineEdit::textChanged,
		[=](const QString &text) {
		float fX = 0;
		float fY = 0;
		fX = ui.lineEditX->text().toFloat();
		fY = text.toFloat();
		emit onValueFChanged(QPointF(fX, fY));
	});
}

CPointItem::~CPointItem()
{
}

void CPointItem::setEnableXY(bool enableX, bool enableY)
{
	ui.lineEditX->setEnabled(enableX);
	ui.lineEditY->setEnabled(enableY);
}

QPoint CPointItem::getVelue()
{
	QString textX = ui.lineEditX->text();
	QString textY = ui.lineEditY->text();
	int x = textX.toInt();
	int y = textY.toInt();
	return QPoint(x, y);
}

void CPointItem::setVelue(QPoint point)
{
	QString textX = QString("%1").arg(point.x());
	QString textY = QString("%1").arg(point.x());
	ui.lineEditX->setText(textX);
	ui.lineEditY->setText(textY);
}

QPointF CPointItem::getVeluF()
{
	QString textX = ui.lineEditX->text();
	QString textY = ui.lineEditY->text();
	float x = textX.toFloat();
	float y = textY.toFloat();
	return QPointF(x, y);
}

void CPointItem::setVelueF(QPointF point)
{
	QString textX = QString("%1").arg(point.x());
	QString textY = QString("%1").arg(point.x());
	ui.lineEditX->setText(textX);
	ui.lineEditY->setText(textY);
}

void CPointItem::setVelueFX(float x)
{
	QString textX = QString("%1").arg(x);
	ui.lineEditX->setText(textX);
}

void CPointItem::setVelueFY(float y)
{
	QString textY = QString("%1").arg(y);
	ui.lineEditY->setText(textY);
}

void CPointItem::paintEvent(QPaintEvent *e) {
	QStyleOption opt;    
	opt.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);    
	QWidget::paintEvent(e); 
}