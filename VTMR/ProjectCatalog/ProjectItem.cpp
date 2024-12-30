#include "ProjectItem.h"
#include <QMouseEvent>
#include <QDebug>
#include <QMenu>
#include "Global.h"

CProjectItem::CProjectItem(QString uuid, QString proName, QWidget *parent)
	: QPushButton(parent)
{
	m_uuid = uuid;
	setText(proName);

	m_styleSheet = styleSheet();

	setContextMenuPolicy(Qt::CustomContextMenu);

	m_popMenu = new QMenu(this);
	auto actionStartUp = new QAction();
	auto actionProjectInfo = new QAction();
	m_actionPlayer = new QAction();
	m_actionTakenArea = new QAction();
	auto actionDelInfo = new QAction();
	actionStartUp->setText(QString("设为活动工程"));
	actionProjectInfo->setText(QString("工程详情"));
	m_actionPlayer->setText(QString("原始视频"));
	m_actionTakenArea->setText(QString("分帧图片"));
	actionDelInfo->setText(QString("删除"));
	m_popMenu->addAction(actionStartUp);
	m_popMenu->addAction(actionProjectInfo);
	m_popMenu->addAction(m_actionPlayer);
	m_popMenu->addAction(m_actionTakenArea);
	m_popMenu->addAction(actionDelInfo);
	connect(this, SIGNAL(sigProjectInfo(QString)), (QWidget*)g_vtmr, SLOT(onProjectInfo(QString)));
	connect(this, SIGNAL(sigDelProject(QString)), (QWidget*)g_vtmr, SLOT(onDelProject(QString)));
	connect(actionStartUp, &QAction::triggered,
		[=]() {
		emit sigActiveProject(m_uuid);
		if (!m_activeProject)
		{
			m_actionPlayer->setDisabled(false);
			m_actionTakenArea->setDisabled(false);
			m_activeProject = true;
		}
	});

	connect(actionProjectInfo, &QAction::triggered,
		[=]() {
		emit sigProjectInfo(m_uuid);
	});

	connect(m_actionPlayer, &QAction::triggered,
		[=]() {
		emit sigActivePlayer();
	});

	connect(m_actionTakenArea, &QAction::triggered,
		[=]() {
		emit sigShowTakenWidge(false);
	});

	connect(actionDelInfo, &QAction::triggered,
		[=]() {
		emit sigDelProject(m_uuid);
	});

	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenu(QPoint)));

	if (!m_activeProject) {
		//actionDelInfo->setDisabled(true);//[MD]
		m_actionPlayer->setDisabled(true);
		m_actionTakenArea->setDisabled(true);
	}
}

CProjectItem::~CProjectItem()
{
}

void CProjectItem::onContextMenu(QPoint pos)
{
	m_popMenu->exec(QCursor::pos()); // 菜单出现的位置为当前鼠标的位置
}

void CProjectItem::setActive(bool status)
{
	if (m_activeProject == status)return;
	m_activeProject = status;
	if (m_activeProject) {
		//QString strStyle = "background-color:#34434D;border: 1px solid 	#4682B4;";
		auto style = "background-color:#34434D;border: 1px solid 	#FFFFFF;";
		setStyleSheet(style);
		m_actionPlayer->setDisabled(false);
		m_actionTakenArea->setDisabled(false);
	}
	else {
		setStyleSheet(m_styleSheet);
		m_actionPlayer->setDisabled(true);
		m_actionTakenArea->setDisabled(true);
	}
}