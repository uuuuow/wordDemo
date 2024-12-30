#include "AddAppraiserDialog.h"
#include <QFileDialog>
#include <QResizeEvent>

CAddAppraiserDialog::CAddAppraiserDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.LEAppraiser->installEventFilter(this);
	ui.LEAppraiserID->installEventFilter(this);
	setMaximumSize(QSize(360, 420));

	connect(ui.LEAppraiser, &QLineEdit::textChanged,
		[=](const QString &text) {
		m_info.name = text;
	});

	connect(ui.LEAppraiserID, &QLineEdit::textChanged,
		[=](const QString &text) {
		m_info.id = text;
	});

	connect(ui.BtnAppraiserPhoto, &QPushButton::clicked,
		[=]() {
		int h = ui.LBAppraiser->height() + ui.LEAppraiser->height() + ui.LBAppraiserID->height() + ui.LEAppraiserID->height() + ui.LBAppraiserPhoto->height();
		m_info.photoPath = QFileDialog::getOpenFileName(this, "加载鉴定人执业照"); 
		if (!m_info.photoPath.isEmpty()) {
			if (m_photo.load(m_info.photoPath)) {
				auto pixmap = QPixmap::fromImage(m_photo);
				h = height() - h;
				auto fitpixmap = pixmap.scaled(ui.LBPhoto->width() - 10, h - 10, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
				ui.LBPhoto->setPixmap(fitpixmap);
			}
		}
	});

	connect(ui.BtnOK, &QPushButton::clicked,
		[=]() {
		QDialog::accept();
	});
}

CAddAppraiserDialog::~CAddAppraiserDialog()
{
	ui.LEAppraiser->removeEventFilter(this);
	ui.LEAppraiserID->removeEventFilter(this);
}

VTAppraiserInfo CAddAppraiserDialog::getInfo()
{
	return m_info;
}

bool CAddAppraiserDialog::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == ui.LEAppraiser) {
		if (event->type() == QEvent::FocusIn) {
		}
		else if (event->type() == QEvent::FocusOut) {
		}
	}
	else if (watched == ui.LEAppraiserID) {
		if (event->type() == QEvent::FocusIn) {
		}
		else if (event->type() == QEvent::FocusOut) {
		}
	}
	return QWidget::eventFilter(watched, event);
}
