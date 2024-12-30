#include "TipsDialog.h"
#include <QLabel>
#include <QVBoxLayout>

CTipsDialog::CTipsDialog(QString strTitle, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
	m_title = new QLabel(strTitle, this);
	QVBoxLayout *vLayout = new QVBoxLayout(this);
	vLayout->addWidget(m_title);
	setLayout(vLayout);
	resize(m_title->width(), m_title->height());

	connect(this, SIGNAL(sigClose()), this, SLOT(onClose()));
}

CTipsDialog::~CTipsDialog()
{
}

void CTipsDialog::onClose()
{
	QDialog::accept();
}