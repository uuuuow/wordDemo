#include "ProjectInfoPage.h"
#include <QPushButton>
#include <QFileDialog>

CProjectInfoPage::CProjectInfoPage(ProjectWizardType type, QWidget *parent)
	: QWizardPage(parent)
{
	m_proWizardType = type;
	m_lbProjectName = new QLabel(QString("&工程名称"), this);
	m_leProjectName = new QLineEdit(this);
	m_lbVideoPath = new QLabel(QString("&视频路径"), this);
	m_leVideoPath = new QLineEdit(this);
	m_btnVideoPath = new QPushButton(QString("&浏览..."), this);

	m_vLayout = new QVBoxLayout;

	setTitle(QString("请填写工程详情"));
	//工程名称
	m_lbProjectName->setBuddy(m_leProjectName);
	//视频路径
	m_leVideoPath->setReadOnly(true);
	m_lbVideoPath->setBuddy(m_leVideoPath);

	m_vLayout->setContentsMargins(10, 10, 10, 10);
	m_vLayout->setSpacing(10); //控件之间的边距

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->setSpacing(10);
	hLayout->addWidget(m_lbProjectName);
	hLayout->addWidget(m_leProjectName);
	m_vLayout->addItem(hLayout);

	hLayout = new QHBoxLayout;
	hLayout->setSpacing(10);
	hLayout->addWidget(m_lbVideoPath);
	hLayout->addWidget(m_leVideoPath);
	hLayout->addWidget(m_btnVideoPath);
	m_vLayout->addItem(hLayout);

	auto spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_vLayout->addItem(spacer);
	setLayout(m_vLayout);

	connect(m_btnVideoPath, &QPushButton::clicked,
		[=]() {
		auto name = QFileDialog::getOpenFileName(this, QString("选择视频文件"));
		if (!name.isEmpty())
			m_leVideoPath->setText(name);
	});

	registerField("project_name*", m_leProjectName);
	registerField("video_path*", m_leVideoPath);
}

void CProjectInfoPage::initializePage()
{

}

void CProjectInfoPage::setEnabled(bool enable)
{
	m_leProjectName->setEnabled(enable);
	m_btnVideoPath->setEnabled(enable);
}

void CProjectInfoPage::getValue(VTProjectInfo & proInfo)
{
	//项目名称
	proInfo.projectName = field("project_name").toString();
	//视频路径
	proInfo.videoPath = field("video_path").toString();
}

void CProjectInfoPage::setValue(VTProjectInfo & proInfo)
{
	//项目名称
	setField("project_name", proInfo.projectName);
	//视频路径
	setField("video_path", proInfo.videoPath);
}
