#pragma once
#include "Utils.h"
#include <vector>
#include <QMap>
#include <QQueue>
#include <QTextBrowser>
/*
*全局变量
*/
using std::vector;
class CVTMR;
class CPlayerBG;
class CTakenBG;

//主界面
extern CVTMR *g_vtmr;
//播放器窗口
extern CPlayerBG *g_playerBG;
//解算编辑窗口
extern CTakenBG *g_takenBG;
//当前窗口
extern VTCurrentWidget g_currentWidget;
// 当前工程信息
extern VTProjectInfo g_proInfo;
// 所有工程信息
extern QVector<VTProjectInfo *> g_proInfoList;
// 按时间顺序存储的UID
extern QQueue<QString> g_uuidQue;
// 程序运行路径
extern QString g_appPath;
// 数据保存路径
extern QString g_proDataPath;
// 日志显示窗口
extern QTextBrowser *g_logBrowser;
// 是否选用点跟踪算法
extern bool g_checkKeyTrace;
extern int g_brightness; //亮度
extern int g_contrast; //对比度
extern int g_saturation; //饱和度
//委托人历史记录
extern QStringList g_clientHistory;
// 授权日期
extern QString g_authDate;
// 授权持续时间
extern QString g_authTime;
//鉴定标准
static VTAppraisalStandard g_appraisalStandardTable[] = {
	{"GB/T 33195-2016", "《道路交通事故车辆速度鉴定》"},
	{"GA/T 1133-2014", "《基于视频图像的车辆行驶速度技术鉴定》"},
	{"GA/T 1087-2013", "《道路交通事故痕迹鉴定》"},
	{"GA/T 41-2019", "《道路交通事故现场痕迹物证勘查》"},
	{"SF/T 0072-2020", "《道路交通事故痕迹物证鉴定通用规范》"},
};
