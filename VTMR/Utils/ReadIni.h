#ifndef QREADINI_H
#define QREADINI_H
#include<QSettings>


#define DATACONFIG  QReadIni::getInstance()->getIniConfig()


typedef struct IniConfig
{
    QString region;  // ��ǰ�ĵ���
}IniConfig;


class QReadIni
{
public:
    static QReadIni* getInstance();


    void readIni();


    const IniConfig& getIniConfig();


private:
    QReadIni();


    IniConfig iniConfig;


    static QReadIni* instance;
};


#endif // QREADINI_H