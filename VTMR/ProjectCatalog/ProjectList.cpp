#include "ProjectList.h"
#include "ProjectItem.h"
#include <QMenu>
#include "Global.h"
#include <QKeyEvent>
#include <QDebug>

CProjectList::CProjectList(QWidget *parent)
	: QListWidget(parent)
{
	setStyleSheet("text-align:left;");
	installEventFilter(this);
}

CProjectList::~CProjectList()
{
}

bool CProjectList::eventFilter(QObject *obj, QEvent *ev)
{
	QKeyEvent *event = static_cast<QKeyEvent*> (ev);
	if (ev->type() == QEvent::KeyPress) {
		event->ignore();
	}
	else if (ev->type() == QEvent::KeyRelease) {
		QKeyEvent *event = static_cast<QKeyEvent*> (ev);
		event->ignore();
	}

	return QListWidget::eventFilter(obj, ev);
}

void CProjectList::addProject(VTProjectInfo &proInfo)
{
	m_tagProjectInfo = proInfo;

	auto item = new QListWidgetItem;
	auto itemCell = new CProjectItem(proInfo.uuid, proInfo.projectName);
	itemCell->installEventFilter(this);
	connect(itemCell, SIGNAL(sigActiveProject(QString)), this, SLOT(onActiveProject(QString)));
	connect(itemCell, SIGNAL(sigActivePlayer()), this, SLOT(onActivePlayer()));
	connect(itemCell, SIGNAL(sigShowTakenWidge(bool)), (QObject *)g_vtmr, SLOT(onShowTakenWidget(bool)));
	item->setSizeHint(QSize(150, 30));
	addItem(item);          //加载列表项到列表框
	setItemWidget(item, itemCell);
}

void CProjectList::onDelProject(QString uuid)
{
	auto rows = count();
	auto active = false;
	for (auto n = 0; n < rows; n++) {
		auto pitem = item(n);
		auto cell = (CProjectItem *)itemWidget(pitem);
		if (!cell) {
			takeItem(n);
		}
		else if (cell && cell->m_uuid == uuid) {
			CProjectItem *itemCell = (CProjectItem *)itemWidget(pitem);
			active = itemCell->m_activeProject;
			removeItemWidget(pitem);
			//delete itemCell;
			takeItem(n);
			//delete pItem;
		}
	}

	rows = count();
	if (active && rows > 0) {
		auto pitem = item(0);
		auto cell = (CProjectItem *)itemWidget(pitem);
		if(cell)
			cell->setActive(true);
	}
}

void CProjectList::onActiveProject(QString uuid)
{
	auto rows = count();
	for (auto n = 0; n < rows; n++)
	{
		auto pitem = item(n);
		auto cell = (CProjectItem *)itemWidget(pitem);
		if (cell->m_uuid == uuid) {
			cell->setActive(true);
			//m_tagProjectInfo = cell->m_tagProjectInfo;
			emit sigActiveProject(uuid);
		}
		else {
			cell->setActive(false);
		}
	}
}

void CProjectList::onActivePlayer()
{
	emit sigActivePlayer();
}