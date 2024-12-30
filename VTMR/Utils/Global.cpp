#include "Global.h"

CVTMR *g_vtmr = nullptr;
CPlayerBG *g_playerBG = nullptr;
CTakenBG *g_takenBG = nullptr;
VTCurrentWidget g_currentWidget = PLAYER_WIDGET;
VTProjectInfo g_proInfo;
QVector<VTProjectInfo *> g_proInfoList;
QQueue<QString> g_uuidQue;
QString g_appPath;
QString g_proDataPath;
QTextBrowser *g_logBrowser;
bool g_checkKeyTrace = false;
int g_brightness = 0; //亮度
int g_contrast = 0; //对比度
int g_saturation = 0; //饱和度
QString g_authDate;
QString g_authTime;
//委托人历史记录
QStringList g_clientHistory;
