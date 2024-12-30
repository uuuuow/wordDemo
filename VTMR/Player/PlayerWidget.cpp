#include "PlayerWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QWidgetAction>
#include <QCheckBox>
#include <QFile>
#include "Global.h"
#include <QDebug>
#include "TakenBG.h"
#include <QButtonGroup>

CPlayerWidget::CPlayerWidget(QWidget *parent)
	: QOpenGLWidget(parent)
{
	ui.setupUi(this);
	setMouseTracking(true);
	installEventFilter(this);
	m_mode = NORMAL;
	m_mousePos = QPoint(0, 0);
	initPopMenu();
}

CPlayerWidget::~CPlayerWidget()
{
}

void CPlayerWidget::initPopMenu()
{
	setContextMenuPolicy(Qt::CustomContextMenu);
	m_popMenu = new QMenu(this);
	auto actionStartImage = new QAction();
	actionStartImage->setText(QString("设定为初始解算帧"));
	m_popMenu->addAction(actionStartImage);
	connect(actionStartImage, &QAction::triggered,
		[=]()
	{
		if (!m_decodeImage)
			return;
		g_proInfo.signPTS = m_decodeImage->pts;
		if (!g_takenBG->actualPointsReady())
		{
			QString tips = QString("设置初始解算帧：\r\n帧号[%1] PTS[%2]\r\n\r\n请输入参照物长度（坐标）：\r\n图像分析  >特征尺寸  >参照物长度输入(参照物坐标输入)")
				.arg(m_decodeImage->index)
				.arg(m_decodeImage->pts);
			VTMessageBoxOk(tips, windowFlags(), QMessageBox::Information, this);
		}
		else
		{
			VTMessageBoxOk(QString("设置初始解算帧：\r\n帧号[%1] PTS[%2]")
				.arg(m_decodeImage->index)
				.arg(m_decodeImage->pts),
				windowFlags(), QMessageBox::Information, this);
		}
	});

	m_popMenu->addSeparator();

	auto menuVideoConfigue = new QMenu(this);
	menuVideoConfigue->setTitle(QString("视频播放设置"));
	m_popMenu->addMenu(menuVideoConfigue);

	auto actionBack = new QAction();
	actionBack->setText(QString("上一帧"));
	menuVideoConfigue->addAction(actionBack);

	menuVideoConfigue->addSeparator();

	auto actionForward = new QAction();
	actionForward->setText(QString("下一帧"));
	menuVideoConfigue->addAction(actionForward);

	menuVideoConfigue->addSeparator();

	auto actionBack5s = new QAction();
	actionBack5s->setText(QString("快退5s"));
	menuVideoConfigue->addAction(actionBack5s);

	menuVideoConfigue->addSeparator();

	auto actionForward5s = new QAction();
	actionForward5s->setText(QString("快进5s"));
	menuVideoConfigue->addAction(actionForward5s);

	m_popMenu->addSeparator();

	auto menuFileInfo = new QMenu(this);
	menuFileInfo->setTitle(QString("文件信息"));
	m_popMenu->addMenu(menuFileInfo);

	auto actionFileName = new QWidgetAction(nullptr);
	m_lbFileName = new QLabel(QString("文件名称："));
	actionFileName->setDefaultWidget(m_lbFileName);
	menuFileInfo->addAction(actionFileName);

	menuFileInfo->addSeparator();

	auto actionDuration = new QWidgetAction(nullptr);
	m_lbDuration = new QLabel(QString("总时长："));
	actionDuration->setDefaultWidget(m_lbDuration);
	menuFileInfo->addAction(actionDuration);

	menuFileInfo->addSeparator();

	auto actionCountFrames = new QWidgetAction(nullptr);
	m_lbCountFrames = new QLabel(QString("总帧数："));
	actionCountFrames->setDefaultWidget(m_lbCountFrames);
	menuFileInfo->addAction(actionCountFrames);

	menuFileInfo->addSeparator();

	auto actionFPS = new QWidgetAction(nullptr);
	m_lbFPS = new QLabel(QString("平均帧率："));
	actionFPS->setDefaultWidget(m_lbFPS);
	menuFileInfo->addAction(actionFPS);

	m_popMenu->addSeparator();

	auto menuHash = new QMenu(this);
	menuHash->setTitle(QString("Hash值校验"));
	m_popMenu->addMenu(menuHash);

	auto actionSHA256 = new QWidgetAction(nullptr);
	m_cbSHA256 = new QCheckBox(QString("SHA256  "));
	actionSHA256->setDefaultWidget(m_cbSHA256);
	menuHash->addAction(actionSHA256);

	menuHash->addSeparator();

	auto actionMD5 = new QWidgetAction(nullptr);
	m_cbMD5 = new QCheckBox(QString("MD5  "));
	actionMD5->setDefaultWidget(m_cbMD5);
	menuHash->addAction(actionMD5);

	menuHash->addSeparator();

	auto actionSHA1 = new QWidgetAction(nullptr);
	m_cbSHA1 = new QCheckBox(QString("SHA1  "));
	actionSHA1->setDefaultWidget(m_cbSHA1);
	menuHash->addAction(actionSHA1);

	menuHash->addSeparator();

	/*auto actionSHA256 = new QWidgetAction(nullptr);
	m_cbSHA256 = new QCheckBox(QString("SHA256  "));
	actionSHA256->setDefaultWidget(m_cbSHA256);
	menuHash->addAction(actionSHA256);

	menuHash->addSeparator();*/

	auto actionSHA512 = new QWidgetAction(nullptr);
	m_cbSHA512 = new QCheckBox(QString("SHA512  "));
	actionSHA512->setDefaultWidget(m_cbSHA512);
	menuHash->addAction(actionSHA512);

	auto group = new QButtonGroup(this);
	group->addButton(m_cbMD5, 1);
	group->addButton(m_cbSHA1, 2);
	group->addButton(m_cbSHA256, 3);
	group->addButton(m_cbSHA512, 4);

	connect(m_cbMD5, &QCheckBox::stateChanged,
		[=](int state)
	{
		if (state == Qt::CheckState::Checked)
			g_proInfo.hashAlgo = QCryptographicHash::Algorithm::Md5;
	});

	connect(m_cbSHA1, &QCheckBox::stateChanged,
		[=](int state)
	{
		if (state == Qt::CheckState::Checked)
			g_proInfo.hashAlgo = QCryptographicHash::Algorithm::Sha1;
	});

	connect(m_cbSHA256, &QCheckBox::stateChanged,
		[=](int state)
	{
		if (state == Qt::CheckState::Checked)
			g_proInfo.hashAlgo = QCryptographicHash::Algorithm::Sha256;
	});

connect(m_cbSHA512, &QCheckBox::stateChanged,
	[=](int state)
{
	if (state == Qt::CheckState::Checked)
		g_proInfo.hashAlgo = QCryptographicHash::Algorithm::Sha512;
});

connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenu(QPoint)));
//上一帧
connect(this, SIGNAL(sigBack()), parent(), SLOT(onBack()));
//下一帧
connect(this, SIGNAL(sigForward()), parent(), SLOT(onForward()));

//快退5秒
connect(this, SIGNAL(sigBack5s()), parent(), SLOT(onBack5s()));
//快进5秒
connect(this, SIGNAL(sigForward5s()), parent(), SLOT(onForward5s()));
connect(actionBack, &QAction::triggered,
	[=]()
{
	emit sigBack();
});
connect(actionBack, &QAction::triggered,
	[=]()
{
	emit sigForward();
});
connect(actionForward, &QAction::triggered,
	[=]()
{
	emit sigForward();
});
connect(actionBack5s, &QAction::triggered,
	[=]()
{
	emit sigBack5s();
});
connect(actionForward5s, &QAction::triggered,
	[=]()
{
	emit sigForward5s();
});
}

bool CPlayerWidget::initVideo()
{
	return m_initVideo;
}

void CPlayerWidget::setInitVideo(bool init)
{
	m_initVideo = init;
}

void CPlayerWidget::setStatus(VTPalyerStatus status)
{
	m_playerStatus = status;
}

void CPlayerWidget::setMode(OPERATION_MODE mode)
{
	m_mode = mode;
}

void CPlayerWidget::paintVTMR(QPainter& painter)
{
	int w = width();
	int h = height();
	if (w <= 0 || h <= 0)
		return;
	QPixmap pixmap(w, h);
	pixmap.fill(Qt::black);
	QString str = "VTMR";

	QPainter painterBG(&pixmap);
	int fontSize = 14, spacing = 14;
	QFont font("微软雅黑", fontSize, QFont::ExtraLight);
	font.setLetterSpacing(QFont::AbsoluteSpacing, spacing);
	painterBG.setFont(font);
	painterBG.setPen(QColor(150, 150, 150));
	painterBG.translate(width() / 2, -width() / 2);//调整位置
	painterBG.rotate(45);
	int squareEdgeSize = width() * sin(45.0) + height() * sin(45.0);
	//对角线长度
	int count = squareEdgeSize / ((fontSize + spacing) * (str.size() + 1)) + 1;
	int x = squareEdgeSize / count + (fontSize + spacing) * 3;
	int y = x / 2;
	for (int i = 0; i < count; i++) {
		for (int j = 0; j < count * 2; j++)
		{
			painterBG.drawText(x * i, y * j, str);
		}
	}
	painter.drawPixmap(0, 0, w, h, pixmap);
}

void CPlayerWidget::paintEvent(QPaintEvent* ev)
{
	QPainter painter(this);
	if (!m_decodeImage || !m_decodeImage->image) {
		paintVTMR(painter);
		return;
	}
	auto image = m_decodeImage->image;
	if (m_mode == MAGNIFY) {
		// 快速放大
		//QPainter painter(this);
		//double width = this->width() * m_scaleValue;
		//double height = this->height() * m_scaleValue;
		//painter.translate(m_drawPoint.x(), m_drawPoint.y());
		//painter.scale(m_scaleValue, m_scaleValue);
		//m_rectPixmap = QRect(m_drawPoint.x(), m_drawPoint.y(), width, height);  // 图片区
		//painter.drawImage(m_rectPixmap, *image);

		//图片质量较好的放大
		//QPainter painter(this);
		double width = this->width() * m_scaleValue;
		double height = this->height() * m_scaleValue;
		auto scalePixmap = image->scaled(width, height, Qt::KeepAspectRatio, Qt::FastTransformation);
		m_rectPixmap = QRect(m_drawPoint.x(), m_drawPoint.y(), width, height);
		painter.drawImage(m_rectPixmap, *image);
	}
	else if (m_mode == NORMAL) {
		painter.drawImage(rect(), *image, image->rect());
		if (m_playerStatus == PLAYER_STATUS_PAUSE) {
			//画时分秒区域
			manualSelectHMS(painter);
		}
	}
	//qDebug() << "w: " << this->width();
	//qDebug() << "h: " << this->height();
	//画十字线
	painter.drawLine(QPoint(m_mousePos.x(), 0), QPoint(m_mousePos.x(), this->height()));
	painter.drawLine(QPoint(0, m_mousePos.y()), QPoint(this->width(), m_mousePos.y()));
}

void CPlayerWidget::onPlayerImage(std::shared_ptr<VTDecodeImage> decodeImage)
{
	m_decodeImage = decodeImage;
	update();
}

void CPlayerWidget::manualSelectHMS(QPainter &painter)
{
	QColor r = QColor(255, 0, 0, 100);
	QColor g = QColor(0, 255, 0, 100);

	//画虚线
	int w = abs(m_mousePosBegin.x() - m_mousePosEnd.x());
	int h = abs(m_mousePosBegin.y() - m_mousePosEnd.y());

	if (m_mousePress)
		drawDotRect(painter, m_mousePosBegin, w, h);
	// 没按下 且宽大于40 高大于4
	if (!m_mousePress && !(w <= 40 || h <= 4)) {
		QRect src;
		src.setX(m_mousePosBegin.x());
		src.setY(m_mousePosBegin.y());
		src.setWidth(w);
		src.setHeight(h);

		if (g_proInfo.firstImage) {
			QRect dst = QRect(0, 0, 0, 0);
			VTTransRect(src, rect(), g_proInfo.firstImage->image->rect(), dst);
			VTWaterMark wm;
			wm.rect = dst;
			if (dst != QRect(0, 0, 0, 0)) {
				bool find = false;
				if(m_waterMarkWndType == WATER_MARK_WND_RED)
					g_proInfo.firstImage->waterMarkHMS.rect = dst;
				else
				{
					for (auto & waterMark : g_proInfo.firstImage->waterMarks) {
						QRect rectWaterMark = waterMark.rect;
						if (dst == rectWaterMark) {
							find = true;
							break;
						}
					}
				}
				if (!find)
					g_proInfo.firstImage->waterMarks.push_back(wm);
				emit sigUpdateFirstImage();
			}
		}
		m_mousePosBegin = QPoint(0, 0);
		m_mousePosEnd = QPoint(0, 0);
	}

	//画水印框
	for (auto w : m_waterMarkWnds)
	{
		w->moveWnd(rect());
	}
}

void CPlayerWidget::removeHMS(QColor color, QRect rt)
{
	QColor r = QColor(255, 0, 0, 100);
	QColor g = QColor(0, 255, 0, 100);

	std::shared_ptr<VTDecodeImage> decodeImage;
	if (!g_proInfo.firstImage)
		return;
	decodeImage = g_proInfo.firstImage;
	if (!decodeImage || !decodeImage->image)
		return;

	QVector<VTWaterMark>::iterator iter = decodeImage->waterMarks.begin();
	for (; iter != decodeImage->waterMarks.end();)
	{
		QRect rectWaterMark = (*iter).rect;
		if (g != color)
		{
			iter++;
			continue;
		}

		if (rectWaterMark == rt)
		{
			(*iter).rect = QRect(0, 0, 0, 0);
			(*iter).text = "";
			iter = decodeImage->waterMarks.erase(iter);
			continue;
		}
		iter++;
	}

	if (decodeImage->waterMarkHMS.rect != QRect(0, 0, 0, 0) && r == color)
	{
		if (decodeImage->waterMarkHMS.rect == rt)
		{
			decodeImage->waterMarkHMS.rect = QRect(0, 0, 0, 0);
			decodeImage->waterMarkHMS.text = "";
		}
	}
}

void CPlayerWidget::drawRect(QPainter &painter, QRect rc, QColor cl)
{
	QPen oldPen = painter.pen();
	QPen pen(cl, 2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
	painter.setPen(pen);
	painter.drawRect(rc);
	painter.setPen(oldPen);
}

void CPlayerWidget::drawDotRect(QPainter &painter, QPoint from, int nDX, int nDY)
{
	QPen oldPen = painter.pen();
	QPen pen(QColor(255, 255, 255, 100), 2, Qt::DotLine, Qt::FlatCap, Qt::MPenJoinStyle);
	painter.setPen(pen);
	painter.drawRect(from.x(), from.y(), nDX, nDY);
	painter.setPen(oldPen);
}

void CPlayerWidget::mouseMoveEvent(QMouseEvent *ev)
{
	if (m_mode == NORMAL) {
		if (m_playerStatus != PLAYER_STATUS_PAUSE)
		{
			QWidget::mouseMoveEvent(ev);
			return;
		}

		if (ev->buttons() & Qt::LeftButton)
		{
			m_mousePosEnd = ev->pos();
		}
		m_mousePos = ev->pos();
		update();
		ev->ignore();
	}
	else if (m_mode == MAGNIFY) {
		if (m_mousePress) {
			int x = ev->pos().x() - m_mousePoint.x();
			int y = ev->pos().y() - m_mousePoint.y();
			m_mousePoint = ev->pos();
			m_drawPoint = QPointF(m_drawPoint.x() + x, m_drawPoint.y() + y);

			if (m_drawPoint.x() > 0) m_drawPoint.setX(0);
			if (m_drawPoint.y() > 0) m_drawPoint.setY(0);
			double width = this->width() * m_scaleValue;
			double height = this->height() * m_scaleValue;
			if (m_drawPoint.y() + height <= this->height()) m_drawPoint.setY(this->height() - height);
			if (m_drawPoint.x() + width <= this->width()) m_drawPoint.setX(this->width() - width);
		}
		m_mousePos = ev->pos();
		update();
	}

}

bool CPlayerWidget::eventFilter(QObject *obj, QEvent *ev)
{
	QKeyEvent *event = nullptr;
	if (ev->type() == QEvent::KeyPress)
	{
		event = static_cast<QKeyEvent*> (ev);
		onKeyPressEvent(event);
	}
	else if (ev->type() == QEvent::KeyRelease)
	{
		event = static_cast<QKeyEvent*> (ev);
		onKeyReleaseEvent(event);
	}

	return QOpenGLWidget::eventFilter(obj, ev);
}

void CPlayerWidget::onKeyPressEvent(QKeyEvent *ev)
{
	if (m_playerStatus != PLAYER_STATUS_PAUSE)
	{
		return;
	}
	if (ev->key() == Qt::Key_Shift)
	{
		m_pressShiftKey = true;
	}
	else if (ev->key() == Qt::Key_Control)
	{
		m_pressCtrlKey = true;
	}
}

void CPlayerWidget::onKeyReleaseEvent(QKeyEvent *ev)
{
	if (m_playerStatus != PLAYER_STATUS_PAUSE)
	{
		return;
	}
	int x = QCursor::pos().x();
	int y = QCursor::pos().y();

	if (ev->key() == Qt::Key_Left) {
		if (m_pressShiftKey)
		{
			--x;
			QCursor::setPos(QPoint(x, y));
		}
		else if (m_pressCtrlKey)
		{
			emit sigBack5s();
		}
		else
		{
			emit sigBack();
		}
	}
	else if (ev->key() == Qt::Key_Right) {
		if (m_pressShiftKey)
		{
			++x;
			QCursor::setPos(QPoint(x, y));
		}
		else if (m_pressCtrlKey)
		{
			emit sigForward5s();
		}
		else
		{
			emit sigForward();
		}
	}
	else if (ev->key() == Qt::Key_Up) {
		if (m_pressShiftKey)
		{
			--y;
			QCursor::setPos(QPoint(x, y));
		}
	}
	else if (ev->key() == Qt::Key_Down) {
		if (m_pressShiftKey)
		{
			++y;
			QCursor::setPos(QPoint(x, y));
		}
	}
	else if (ev->key() == Qt::Key_Shift)
	{
		m_pressShiftKey = false;
	}
	else if (ev->key() == Qt::Key_Control)
	{
		m_pressCtrlKey = false;
	}
}

void CPlayerWidget::mousePressEvent(QMouseEvent* ev)
{
	m_mousePress = true;
	if (m_playerStatus != PLAYER_STATUS_PAUSE && m_mode == NORMAL)
	{
		QWidget::mouseMoveEvent(ev);
		return;
	}

	if (ev->button() == Qt::LeftButton)
	{
		m_mousePoint = ev->pos();
		if (m_mode == NORMAL) {
			m_mousePosEnd = m_mousePosBegin = ev->pos();
			if (m_pressCtrlKey)
				m_waterMarkWndType = WATER_MARK_WND_GREEN;
			else
				m_waterMarkWndType = WATER_MARK_WND_RED;

		}
	}

}

void CPlayerWidget::mouseReleaseEvent(QMouseEvent *ev)
{
	m_mousePress = false;
	//ev->ignore();
}

void CPlayerWidget::wheelEvent(QWheelEvent* event)
{
	if (m_mode == NORMAL) {
		m_mode = MAGNIFY;
	}
	changeWheelValue(event->position(), event->angleDelta().y());
	event->accept();
}
void CPlayerWidget::changeWheelValue(QPointF event, int numSteps) {
	double oldScale = m_scaleValue;
	if (numSteps > 0)
	{
		m_scaleValue *= 1.1;
		if (m_waterMarkVisible) {
			onWaterMarkWndHide();
			m_waterMarkVisible = false;
		}
	}
	else
	{
		m_scaleValue *= 0.9;
	}
	if (m_scaleValue > (SCALE_MAX_VALUE))
	{
		m_scaleValue = SCALE_MAX_VALUE;
	}
	if (m_scaleValue < (SCALE_MIN_VALUE))
	{
		m_scaleValue = SCALE_MIN_VALUE;
		if (!m_waterMarkVisible) {
			onWaterMarkWndShow();
			m_waterMarkVisible = true;
		}

	}
	QPoint tmp;
	tmp.setX(event.x());
	tmp.setY(event.y());
	if (m_rectPixmap.contains(tmp))
	{
		double x = m_drawPoint.x() - (event.x() - m_drawPoint.x()) / m_rectPixmap.width() * (this->width() * (m_scaleValue - oldScale));
		double y = m_drawPoint.y() - (event.y() - m_drawPoint.y()) / m_rectPixmap.height() * (this->height() * (m_scaleValue - oldScale));
		m_drawPoint = QPointF(x, y);
	}
	else
	{
		double x = m_drawPoint.x() - (this->width() * (m_scaleValue - oldScale)) / 2;
		double y = m_drawPoint.y() - (this->height() * (m_scaleValue - oldScale)) / 2;
		m_drawPoint = QPointF(x, y);
	}
	if (m_drawPoint.x() > 0) m_drawPoint.setX(0);
	if (m_drawPoint.y() > 0) m_drawPoint.setY(0);
	double width = this->width() * m_scaleValue;
	double height = this->height() * m_scaleValue;
	if (m_drawPoint.y() + height <= this->height()) m_drawPoint.setY(this->height() - height);
	if (m_drawPoint.x() + width <= this->width()) m_drawPoint.setX(this->width() - width);
	update();
}
void CPlayerWidget::onContextMenu(QPoint pos)
{
	m_popMenu->exec(QCursor::pos());
}

void CPlayerWidget::initVideoInfo(QString file, int fps, int64_t duration, int64_t countFrames)
{
	int index = file.lastIndexOf('/');
	QString strName = "";
	
	if (index == -1)
	{
		index = file.lastIndexOf('\\');
	}
	if (index != -1 && index + 1 <= file.size())
	{
		strName = file.mid(index + 1);
	}
	
	int hours, minutes, seconds;
	duration /= 1000000;
	hours = duration / 3600;
	minutes = (duration - hours * 3600) / 60;
	seconds = duration % 60;
	QString time = QString("%1:%2:%3").arg(hours, 2, 10, QLatin1Char('0'))
		.arg(minutes, 2, 10, QLatin1Char('0'))
		.arg(seconds, 2, 10, QLatin1Char('0'));

	m_lbFileName->setText(QString("文件名称：%1").arg(strName));
	m_lbDuration->setText(QString("总时长：%1").arg(time));
	m_lbCountFrames->setText(QString("总帧数：%1").arg(countFrames));
	m_lbFPS->setText(QString("平均帧率：%1").arg(fps));

	if (g_proInfo.hashAlgo == QCryptographicHash::Algorithm::Md5) {
		m_cbMD5->setCheckState(Qt::CheckState::Checked);
	}
	else if (g_proInfo.hashAlgo == QCryptographicHash::Algorithm::Sha1) {
		m_cbSHA1->setCheckState(Qt::CheckState::Checked);
	}
	else if (g_proInfo.hashAlgo == QCryptographicHash::Algorithm::Sha256) {
		m_cbSHA256->setCheckState(Qt::CheckState::Checked);
	}
	else if (g_proInfo.hashAlgo == QCryptographicHash::Algorithm::Sha512) {
		m_cbSHA512->setCheckState(Qt::CheckState::Checked);
	}
	else {
		g_proInfo.hashAlgo = QCryptographicHash::Algorithm::Md5;
		m_cbMD5->setCheckState(Qt::CheckState::Checked);
	}

	m_decodeImage.reset();
}

void CPlayerWidget::onWaterMarkWndClose(QWidget *w)
{
	CWaterMarkWnd* pWaterMarkWnd;
	QVector<CWaterMarkWnd *>::iterator iter = m_waterMarkWnds.begin();
	for (; iter != m_waterMarkWnds.end();)
	{
		if (w == *iter)
		{
			pWaterMarkWnd = (CWaterMarkWnd *)w;
			pWaterMarkWnd->close();
			iter = m_waterMarkWnds.erase(iter);
			removeHMS(pWaterMarkWnd->getColor(), pWaterMarkWnd->getRect());
		}
		else
		{
			iter++;
		}
	}
}
void CPlayerWidget::onWaterMarkWndHide() {
	for (QVector<CWaterMarkWnd*>::iterator iter = m_waterMarkWnds.begin(); iter != m_waterMarkWnds.end(); iter++)
	{
		if ((*iter)->isVisible()) {
			(*iter)->hide();
		}
	}
}
void CPlayerWidget::onWaterMarkWndShow() {
	for (QVector<CWaterMarkWnd*>::iterator iter = m_waterMarkWnds.begin(); iter != m_waterMarkWnds.end(); iter++)
	{
		if (!(*iter)->isVisible()) {
			(*iter)->show();
		}
	}
}
void CPlayerWidget::onUpdateFirstImage()
{
	QColor r = QColor(255, 0, 0, 100);
	QColor g = QColor(0, 255, 0, 100);

	//画水印框
	for (auto w : m_waterMarkWnds)
		w->hide();
	m_waterMarkWnds.clear();

	std::shared_ptr<VTDecodeImage> decodeImage;
	if (!g_proInfo.firstImage)
		return;
	decodeImage = g_proInfo.firstImage;
	if (!decodeImage || !decodeImage->image)
		return;

	if (decodeImage->waterMarkHMS.rect != QRect(0, 0, 0, 0))
	{
		auto waterMarkWnd = new CWaterMarkWnd(r, this);
		waterMarkWnd->setRect(decodeImage->waterMarkHMS.rect, decodeImage->image->rect());
		waterMarkWnd->show();
		m_waterMarkWnds.push_back(waterMarkWnd);
	}

	QVector<VTWaterMark>::iterator iter = decodeImage->waterMarks.begin();
	for (; iter != decodeImage->waterMarks.end();)
	{
		QRect rectWaterMark = (*iter).rect;
		if (rectWaterMark == QRect(0, 0, 0, 0))
			continue;
		if (decodeImage->waterMarkHMS.rect == rectWaterMark)
		{
			iter = decodeImage->waterMarks.erase(iter);
			continue;
		}

		auto waterMarkWnd = new CWaterMarkWnd(g, this);
		waterMarkWnd->setRect(rectWaterMark, decodeImage->image->rect());
		waterMarkWnd->show();
		m_waterMarkWnds.push_back(waterMarkWnd);
		++iter;
	}
	update();
}