#include "ReadIni.h"


QReadIni* QReadIni::instance = NULL;


QReadIni* QReadIni::getInstance()
{
	if (instance == NULL)
	{
		instance = new QReadIni();
	}
	return instance;
}


QReadIni::QReadIni()
{
	this->readIni();
}


void QReadIni::readIni()
{
	QSettings* configIniRead = new QSettings("config.ini", QSettings::IniFormat);//初始化读取Ini文件对象
	iniConfig.region = configIniRead->value("region/region").toString();                          //IP地址


	delete configIniRead;
}


const IniConfig& QReadIni::getIniConfig()
{
	return iniConfig;
}