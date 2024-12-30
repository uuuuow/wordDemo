#include "RulerWidget.h"
#include <QKeyEvent>

CRulerWidget::CRulerWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
	setWindowOpacity(0.8);
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);

	m_actOrn = new QAction(QString("垂直"), this);
	connect(m_actOrn, SIGNAL(triggered()), SLOT(onOrn()));
	m_actAlign = new QAction(QString("对齐"), this);
	connect(m_actAlign, SIGNAL(triggered()), SLOT(onAlign()));
	m_actClose = new QAction(QString("关闭"), this);
	connect(m_actClose, SIGNAL(triggered()), SLOT(onClose()));
	addAction(m_actOrn);
	addAction(m_actAlign);
	addAction(m_actClose);

	setContextMenuPolicy(Qt::ActionsContextMenu);

	ui.ruler->setOrientation(Qt::Horizontal);
	resize(ui.ruler->sizeHint());

	connect(ui.ruler, SIGNAL(moved(QPoint)), SLOT(onMove(QPoint)));
}

CRulerWidget::~CRulerWidget()
{
}

void CRulerWidget::onMove(QPoint p)
{
	move(pos() + p);
}

void CRulerWidget::onOrn()
{
	if (m_actOrn->text() == QString("垂直")) {
		ui.ruler->setOrientation(Qt::Vertical);
		m_actOrn->setText(QString("水平"));
	}
	else {
		ui.ruler->setOrientation(Qt::Horizontal);
		m_actOrn->setText(QString("垂直"));
	}

	resize(ui.ruler->sizeHint());
	update();
}

void CRulerWidget::onAlign()
{
	m_actOrn->text() == QString("垂直") ? move(0, pos().y()) : move(pos().x(), 0);
}

void CRulerWidget::onClose()
{
	close();
}

void CRulerWidget::keyReleaseEvent(QKeyEvent *e)
{
	QPoint p = pos();
	if (e->key() == Qt::Key_Left) p += QPoint(-1, 0);
	else if (e->key() == Qt::Key_Right) p += QPoint(1, 0);
	else if (e->key() == Qt::Key_Up) p += QPoint(0, -1);
	else if (e->key() == Qt::Key_Down) p += QPoint(0, 1);
	move(p);
	e->ignore();
}
