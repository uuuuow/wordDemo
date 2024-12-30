#include "MeasuringMethodPage.h"
#include <QWizard>
#include <QVariant>

CMeasuringMethodPage::CMeasuringMethodPage(ProjectWizardType type, QWidget *parent)
	: QWizardPage(parent)
{
	m_proWizardType = type;
	m_lb1 = new QLabel(QString("固定摄像头"), this);
	m_lb2 = new QLabel(QString("车载摄像头"), this);

	//固定摄像头
	m_rb1 = new QRadioButton(QString("&车身自标定"), this);
	m_rb2 = new QRadioButton(QString("&地面参照物"), this);
	m_rb3 = new QRadioButton(QString("&一维线性"), this);
	m_rb4 = new QRadioButton(QString("&二维线性"), this);

	m_rb5 = new QRadioButton(QString("&空间标定"), this);
	m_rb6 = new QRadioButton(QString("&二维本车"), this);
	m_rb7 = new QRadioButton(QString("&二维目标车"), this);
	m_gl = new QGridLayout(this);
	m_gb = new QGroupBox(this);
	m_hLayout = new QHBoxLayout;

	setTitle(QString("请选择标定方式"));
	

	m_rb1->setChecked(true);

	m_gl->addWidget(m_lb1, 0, 0, 1, 1);
	m_gl->addWidget(m_rb1, 1, 0, 1, 1);
	m_gl->addWidget(m_rb2, 2, 0, 1, 1);
	m_gl->addWidget(m_rb3, 3, 0, 1, 1);
	m_gl->addWidget(m_rb4, 4, 0, 1, 1);

	m_gl->addWidget(m_lb2, 0, 1, 1, 1);
	m_gl->addWidget(m_rb5, 1, 1, 1, 1);
	m_gl->addWidget(m_rb6, 2, 1, 1, 1);
	m_gl->addWidget(m_rb7, 3, 1, 1, 1);
	m_gb->setLayout(m_gl);

	m_hLayout->addWidget(m_gb);
	setLayout(m_hLayout);

	registerField("FIXED_CAM_ONESELF", m_rb1);
	registerField("FIXED_CAM_GROUND_REFERENCE", m_rb2);
	registerField("FIXED_CAM_1D", m_rb3);
	registerField("FIXED_CAM_2D", m_rb4);

	registerField("MOVING_CAM_SPACE", m_rb5);
	registerField("MOVING_CAM_2D_ONESELF", m_rb6);
	registerField("MOVING_CAM_2D_TARGET_VEHICLE", m_rb7);
}

void CMeasuringMethodPage::setEnabled(bool enable)
{
	m_rb1->setEnabled(enable);
	m_rb2->setEnabled(enable);
	m_rb3->setEnabled(enable);
	m_rb4->setEnabled(enable);
	m_rb5->setEnabled(enable);
	m_rb6->setEnabled(enable);
	m_rb7->setEnabled(enable);
}

void CMeasuringMethodPage::getValue(VTProjectInfo & proInfo)
{
	VTCalibration mode;
	if (field("FIXED_CAM_ONESELF").toBool()) {
		//固定摄像头,车身自标定
		mode = FIXED_CAM_ONESELF;
	}
	else if (field("FIXED_CAM_GROUND_REFERENCE").toBool())
	{
		//固定摄像头，地面参照物
		mode = FIXED_CAM_GROUND_REFERENCE;
	}
	else if (field("FIXED_CAM_1D").toBool()) {
		//固定摄像头,一维线性
		mode = FIXED_CAM_1D;
	}
	else if (field("FIXED_CAM_2D").toBool()) {
		//固定摄像头,二维线性
		mode = FIXED_CAM_2D;
	}
	else if (field("MOVING_CAM_SPACE").toBool()) {
		//车载摄像头,空间标定
		mode = MOVING_CAM_SPACE;
	}
	else if (field("MOVING_CAM_2D_ONESELF").toBool()) {
		//车载摄像头,二维本车
		mode = MOVING_CAM_2D_ONESELF;
	}
	else if (field("MOVING_CAM_2D_TARGET_VEHICLE").toBool()) {
		//车载摄像头，二维目标车
		mode = MOVING_CAM_2D_TARGET_VEHICLE;
	}
	proInfo.mode = mode;
}

void CMeasuringMethodPage::setValue(VTProjectInfo & proInfo)
{
	if (proInfo.mode == FIXED_CAM_ONESELF) {
		//固定摄像头,车身自标定
		m_rb1->setChecked(true);
	}
	else if (proInfo.mode == FIXED_CAM_GROUND_REFERENCE)
	{
		//固定摄像头，地面参照物
		m_rb2->setChecked(true);
	}
	else if (proInfo.mode == FIXED_CAM_1D) {
		//固定摄像头,一维线性
		m_rb3->setChecked(true);
	}
	else if (proInfo.mode == FIXED_CAM_2D) {
		//固定摄像头,二维线性
		m_rb4->setChecked(true);
	}
	else if (proInfo.mode == MOVING_CAM_SPACE) {
		//车载摄像头,空间标定
		m_rb5->setChecked(true);
	}
	else if (proInfo.mode == MOVING_CAM_2D_ONESELF) {
		//车载摄像头,二维本车
		m_rb6->setChecked(true);
	}
	else if (proInfo.mode == MOVING_CAM_2D_TARGET_VEHICLE) {
		//车载摄像头，二维目标车
		m_rb7->setChecked(true);
	}
}

void CMeasuringMethodPage::initializePage()
{

}
