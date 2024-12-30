#pragma once

#include <QOpenGLWidget>
#include "ui_PlayerWidget.h"
#include "Utils.h"
#include <QMenu>
#include <QPoint>
#include <QLabel>
#include <QResizeEvent>
#include <QVector>
#include "WaterMarkWnd.h"

enum WATER_MARK_WND_TYPE
{
	WATER_MARK_WND_RED,
	WATER_MARK_WND_GREEN
};
enum OPERATION_MODE {
	// 操作模式
	MAGNIFY, // 鼠标滑轮放大缩小模式
	NORMAL // 正常模式，有十字线
};
/*
*播放器渲染界面
*/
class CPlayerWidget : public QOpenGLWidget
{
	Q_OBJECT


public:
	CPlayerWidget(QWidget *parent = Q_NULLPTR);
	~CPlayerWidget();
	void setStatus(VTPalyerStatus status);
	void setMode(OPERATION_MODE mode);
	VTPalyerStatus status() { return m_playerStatus; }
	void initVideoInfo(QString file, int fps, int64_t duration, int64_t countFrames);
	void setInitVideo(bool init);
	bool initVideo();
private:
	void drawRect(QPainter &painter, QRect rc, QColor cl);
	void drawDotRect(QPainter &painter, QPoint from, int nDX, int nDY);
	void manualSelectHMS(QPainter &painter);
	void removeHMS(QColor color, QRect rt);
	void onKeyPressEvent(QKeyEvent *ev);
	void onKeyReleaseEvent(QKeyEvent *ev);
	void paintVTMR(QPainter& painter);
public slots:
	void onPlayerImage(std::shared_ptr<VTDecodeImage> decodeImage);
	void onContextMenu(QPoint pos);
	void onWaterMarkWndClose(QWidget *w);
	void onWaterMarkWndHide();
	void onWaterMarkWndShow();
	void onUpdateFirstImage();

signals:
	//上一帧
	void sigBack();
	//下一帧
	void sigForward();
	//快退5秒
	void sigBack5s();
	//快进5秒
	void sigForward5s();
	
	void sigPosChange();
	void sigSelectChange(bool doOCR);
	void sigSelectChange(QRect selectRect);
	void sigOCR(QImage *, QRect rect);
	void sigSelectedRect(QRect selectRect);
	void sigUpdateFirstImage();

protected:
	virtual bool eventFilter(QObject *obj, QEvent *ev);
	virtual void paintEvent(QPaintEvent *ev);
	virtual void mouseMoveEvent(QMouseEvent *ev);
	virtual void mousePressEvent(QMouseEvent *ev);
	virtual void mouseReleaseEvent(QMouseEvent *ev);
	virtual void wheelEvent(QWheelEvent* event);
	void changeWheelValue(QPointF event, int numSteps);

private:
	void initPopMenu();
private:
	Ui::PlayerWidget ui;

	VTPalyerStatus m_playerStatus = PLAYER_STATUS_PAUSE;
	QPoint m_mousePos;

	std::shared_ptr<VTDecodeImage>m_decodeImage = nullptr;

	bool m_initVideo = false;
	QMenu *m_popMenu = nullptr;

	QCheckBox *m_cbMD5 = nullptr;
	QCheckBox *m_cbSHA1 = nullptr;
	QCheckBox *m_cbSHA256 = nullptr;
	QCheckBox *m_cbSHA512 = nullptr;

	QLabel *m_lbFileName = nullptr;
	QLabel *m_lbDuration = nullptr;
	QLabel *m_lbCountFrames = nullptr;
	QLabel *m_lbFPS = nullptr;

	//OCR
	bool m_mousePress = false;
	QPoint m_formalPos = QPoint(0, 0);
	QPoint m_mousePosBegin = QPoint(0, 0);
	QPoint m_mousePosEnd = QPoint(0, 0);

	bool m_pressShiftKey = false;
	bool m_pressCtrlKey = false;
	bool m_pressAltKey = false;
	QVector<CWaterMarkWnd *> m_waterMarkWnds;
	WATER_MARK_WND_TYPE m_waterMarkWndType = WATER_MARK_WND_RED;

	// 正常模式
	OPERATION_MODE m_mode = NORMAL;
	// 画面随鼠标滑轮放大缩小所需的参数
	qreal m_scaleValue = 1.0;
	int m_XPtInterval = 0;    //平移X轴的值
	int m_YPtInterval = 0;    //平移Y轴的值
	QPointF m_drawPoint;
	QRect m_rectPixmap = QRect();
	QPoint m_mousePoint;
	int SCALE_MAX_VALUE = 5;
	int SCALE_MIN_VALUE = 1;
	bool m_waterMarkVisible = true;
};
