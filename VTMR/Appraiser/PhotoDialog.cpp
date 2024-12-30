#include "PhotoDialog.h"

CPhotoDialog::CPhotoDialog(QString title, QString path, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle(title);
	if (m_photo.load(path)) {
		auto pixmap = QPixmap::fromImage(m_photo);
		auto sizeImage = pixmap.size();
		auto thumbnailWidth = 600;
		auto thumbnailHeight = 600;
		if (sizeImage.width() > thumbnailWidth || sizeImage.height() > thumbnailHeight) {
			if (sizeImage.width() > sizeImage.height())
				pixmap = pixmap.scaledToWidth(thumbnailWidth, Qt::SmoothTransformation);
			else
				pixmap = pixmap.scaledToHeight(thumbnailHeight, Qt::SmoothTransformation);
			setFixedSize(pixmap.size());
		}
		else
			setFixedSize(sizeImage);

		ui.LBPhoto->setPixmap(pixmap);
	}
}

CPhotoDialog::~CPhotoDialog()
{
}
