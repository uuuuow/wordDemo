#include "AppraisalPage.h"
#include "AppraiserDialog.h"
#include <QLineEdit>
#include "AppraisalStandardTable.h"
#include "Global.h"
#include "AppraisalStandardDialog.h"

CAppraisalPage::CAppraisalPage(ProjectWizardType type, QWidget *parent)
{
	m_proWizardType = type;
	m_lbAppraiser1 = new QLabel(QString("&第一鉴定人"), this);
	m_cbAppraiser1 = new QComboBox(this);
	m_lbAppraiserNum1 = new QLabel(QString("执业证号"), this);
	m_leAppraiserNum1 = new QLineEdit(this);
	m_leAppraiserNum1->setEnabled(false);
	m_lbAppraiserPhotoText1 = new QLabel(QString("执业照"), this);
	m_lbAppraiserPhotoImage1 = new QLabel(this);
	m_lbAppraiserPhotoImage1->setFixedSize(QSize(300, 200));
	connect(m_cbAppraiser1, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		[=](int index) {

			auto info = CAppraiserDialog::getInfo();
			QString key = m_cbAppraiser1->currentData().toString();
			for (auto& appr : info) {
				if (key == appr.id) {
					m_leAppraiserNum1->setText(appr.id);
					if (!appr.photoPath.isEmpty()) {
						QImage image;
						if (image.load(appr.photoPath)) {
							auto pixmap = QPixmap::fromImage(image);
							auto with = m_lbAppraiserPhotoImage1->width();
							auto height = m_lbAppraiserPhotoImage1->height();
							auto fitpixmap = pixmap.scaled(with, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
							m_lbAppraiserPhotoImage1->setPixmap(fitpixmap);
						}
					}
					break;
				}
			}
		});

	auto info = CAppraiserDialog::getInfo();
	for (auto appr : info) {
		m_cbAppraiser1->addItem(appr.name, appr.id);
	}

	m_lbAppraiser2 = new QLabel(QString("&第二鉴定人"), this);
	m_cbAppraiser2 = new QComboBox(this);
	m_lbAppraiserNum2 = new QLabel(QString("执业证号"), this);
	m_leAppraiserNum2 = new QLineEdit(this);
	m_leAppraiserNum2->setEnabled(false);
	m_lbAppraiserPhotoText2 = new QLabel(QString("执业照"), this);
	m_lbAppraiserPhotoImage2 = new QLabel(this);
	m_lbAppraiserPhotoImage2->setFixedSize(QSize(300, 200));
	connect(m_cbAppraiser2, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		[=](int index) {

			auto info = CAppraiserDialog::getInfo();
			QString key = m_cbAppraiser2->currentData().toString();
			for (auto& appr : info) {
				if (key == appr.id) {
					m_leAppraiserNum2->setText(appr.id);
					if (!appr.photoPath.isEmpty()) {
						QImage image;
						if (image.load(appr.photoPath)) {
							auto pixmap = QPixmap::fromImage(image);
							auto with = m_lbAppraiserPhotoImage2->width();
							auto height = m_lbAppraiserPhotoImage2->height();
							auto fitpixmap = pixmap.scaled(with, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
							m_lbAppraiserPhotoImage2->setPixmap(fitpixmap);
						}
					}
					break;
				}
			}
		});

	for (auto appr : info) {
		m_cbAppraiser2->addItem(appr.name, appr.id);
	}


	m_lbStandard = new QLabel(QString("适应标准"), this);
	m_appraisalStandardTable = new CAppraisalStandardTable(this);
	m_appraisalStandardTable->setFixedHeight(400);
	m_vLayout = new QVBoxLayout;
	m_vLayout->setContentsMargins(10, 2, 10, 2);
	m_vLayout->setSpacing(10);

	setTitle(QString("鉴定过程信息"));

	//鉴定人
	m_lbAppraiser1->setBuddy(m_cbAppraiser1);
	m_lbAppraiser2->setBuddy(m_cbAppraiser2);
	//适应标准
	m_lbStandard->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	for(auto &info : CAppraisalStandardDialog::m_appraiserStandardTable)
		m_appraisalStandardTable->addTable(info);

	m_vLayout->addWidget(m_lbStandard);
	m_vLayout->addWidget(m_appraisalStandardTable);
	/*QHBoxLayout* hbStandard;
	hbStandard = new QHBoxLayout;
	QSpacerItem *spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
	hbStandard->addItem(spacer);
	QPushButton* btn = new QPushButton(QString("新增"), this);
	hbStandard->addWidget(btn);
	btn = new QPushButton(QString("删除"), this);
	hbStandard->addWidget(btn);
	m_vLayout->addItem(hbStandard);*/

	QHBoxLayout* hLayout;
	hLayout = new QHBoxLayout;
	hLayout->setSpacing(10);

	QVBoxLayout* vlAppraiser;
	QHBoxLayout* hbAppraiser;
	//第一鉴定人
	vlAppraiser = new QVBoxLayout;
	vlAppraiser->setContentsMargins(0, 10, 0, 10);
	vlAppraiser->setSpacing(10);

	hbAppraiser = new QHBoxLayout;
	hbAppraiser->addWidget(m_lbAppraiser1);
	hbAppraiser->addWidget(m_cbAppraiser1);
	vlAppraiser->addItem(hbAppraiser);

	hbAppraiser = new QHBoxLayout;
	hbAppraiser->addWidget(m_lbAppraiserNum1);
	hbAppraiser->addWidget(m_leAppraiserNum1);
	vlAppraiser->addItem(hbAppraiser);

	vlAppraiser->addWidget(m_lbAppraiserPhotoText1);
	vlAppraiser->addWidget(m_lbAppraiserPhotoImage1);
	hLayout->addItem(vlAppraiser);

	//第二鉴定人
	vlAppraiser = new QVBoxLayout;
	vlAppraiser->setContentsMargins(0, 10, 0, 10);
	vlAppraiser->setSpacing(10);
	hbAppraiser = new QHBoxLayout;
	hbAppraiser->addWidget(m_lbAppraiser2);
	hbAppraiser->addWidget(m_cbAppraiser2);
	vlAppraiser->addItem(hbAppraiser);

	hbAppraiser = new QHBoxLayout;
	hbAppraiser->addWidget(m_lbAppraiserNum2);
	hbAppraiser->addWidget(m_leAppraiserNum2);
	vlAppraiser->addItem(hbAppraiser);

	vlAppraiser->addWidget(m_lbAppraiserPhotoText2);
	vlAppraiser->addWidget(m_lbAppraiserPhotoImage2);
	hLayout->addItem(vlAppraiser);

	m_vLayout->addItem(hLayout);

	auto spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_vLayout->addItem(spacer);
	setLayout(m_vLayout);
}

void CAppraisalPage::initializePage()
{

}

void CAppraisalPage::setEnabled(bool enable)
{
	m_cbAppraiser1->setEnabled(enable);
	m_cbAppraiser2->setEnabled(enable);
	m_appraisalStandardTable->setEnabled(enable);
}

void CAppraisalPage::getValue(VTProjectInfo & proInfo)
{
	proInfo.appraiser1.name = m_cbAppraiser1->currentText();
	proInfo.appraiser2.name = m_cbAppraiser2->currentText();
	proInfo.appraiser1.id = m_cbAppraiser1->currentData().toString();
	proInfo.appraiser2.id = m_cbAppraiser2->currentData().toString();
	proInfo.appraisalStandards.clear();
	m_appraisalStandardTable->getStandard(proInfo.appraisalStandards);
}

void CAppraisalPage::setValue(VTProjectInfo & proInfo)
{
	m_cbAppraiser1->setCurrentText(proInfo.appraiser1.name);
	m_cbAppraiser2->setCurrentText(proInfo.appraiser2.name);
	m_cbAppraiser1->setCurrentText(proInfo.appraiser1.id);
	m_cbAppraiser2->setCurrentText(proInfo.appraiser2.id);
	m_appraisalStandardTable->setStandard(proInfo.appraisalStandards);
}

void CAppraisalPage::onAdd()
{
	auto dialog = new CAppraisalStandardDialog(this);
	auto ret = dialog->exec();
	if (ret != QDialog::Accepted)
		return;
	auto stantard = dialog->getStantard();
	CAppraisalStandardDialog::deleteSql(stantard);
	CAppraisalStandardDialog::insertSql(stantard);
	m_appraisalStandardTable->addTable(stantard);
}

void CAppraisalPage::onDelete(QString id)
{
	VTAppraisalStandard stantard;
	stantard.id = id;
	CAppraisalStandardDialog::deleteSql(stantard);
	auto iter = CAppraisalStandardDialog::m_appraiserStandardTable.begin();
	for (; iter != CAppraisalStandardDialog::m_appraiserStandardTable.end(); iter++)
	{
		if (iter->id == id)
		{
			CAppraisalStandardDialog::m_appraiserStandardTable.erase(iter);
			break;
		}
	}
}