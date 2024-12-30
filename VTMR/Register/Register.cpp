#include "Register.h"
#include <QCryptographicHash>
#include <QByteArray>
#include <QDebug>
#include <QClipboard>
#include <QProcess>
#include <QHostInfo>
#include <QSettings>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSqlQuery>
#include <QSqlError>
#include "SQLManager.h"
#include <Windows.h>
#include <wincrypt.h>
extern "C" {
#include <openssl/evp.h>
#include <openssl/x509.h>
}

static QByteArray vtmrKey1 = "10235970538102723103461710235976";
static QByteArray vtmrIV1 = "5329102572014261";

static QByteArray vtmrKey2 = "53213039431002817539024043593135";
static QByteArray vtmrIV2 = "1592673527904379";


static QString HKeyPath = "HKEY_CURRENT_USER\\SOFTWARE\\VTMR";
static QString HKeyRegCode = "regcode";

QByteArray encryptAES(const QByteArray& plaintext, const QByteArray& key, const QByteArray& iv)
{
    const EVP_CIPHER* cipher = EVP_aes_256_cbc();
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    EVP_EncryptInit_ex(ctx, cipher, nullptr, reinterpret_cast<const unsigned char*>(key.constData()), reinterpret_cast<const unsigned char*>(iv.constData()));
    EVP_CIPHER_CTX_set_padding(ctx, EVP_PADDING_PKCS7);

    int encryptedSize = plaintext.size() + EVP_CIPHER_CTX_block_size(ctx);
    QByteArray encryptedData(encryptedSize, 0);

    int finalSize = 0;
    EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(encryptedData.data()), &finalSize, reinterpret_cast<const unsigned char*>(plaintext.constData()), plaintext.size());

    int finalEncryptedSize = 0;
    EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(encryptedData.data()) + finalSize, &finalEncryptedSize);

    EVP_CIPHER_CTX_free(ctx);

    encryptedData.resize(finalSize + finalEncryptedSize);

    return encryptedData;
}

QByteArray decryptAES(const QByteArray& ciphertext, const QByteArray& key, const QByteArray& iv)
{
    const EVP_CIPHER* cipher = EVP_aes_256_cbc();
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    EVP_DecryptInit_ex(ctx, cipher, nullptr, reinterpret_cast<const unsigned char*>(key.constData()), reinterpret_cast<const unsigned char*>(iv.constData()));

    EVP_CIPHER_CTX_set_padding(ctx, EVP_PADDING_PKCS7);

    int decryptedSize = ciphertext.size();
    QByteArray decryptedData(decryptedSize, 0);

    int finalSize = 0;
    EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(decryptedData.data()), &finalSize, reinterpret_cast<const unsigned char*>(ciphertext.constData()), decryptedSize);

    int finalDecryptedSize = 0;
    EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(decryptedData.data()) + finalSize, &finalDecryptedSize);

    EVP_CIPHER_CTX_free(ctx);

    decryptedData.resize(finalSize + finalDecryptedSize);

    return decryptedData;
}

// 获取WMIC指令结果
static QString getWMICInfo(QString cmd)
{
    // cmd指令
    QProcess process;
    QStringList list = cmd.split(" ");
    process.start("wmic.exe", list);
    process.waitForFinished();

    // 获取输出结果
    QString result = QString::fromLocal8Bit(process.readAllStandardOutput());

    // 删除输出结果中的回车和换行
    result = result.replace("\r", " ");
    result = result.replace("\n", " ");

    // 去除空格操作
    result = result.simplified();

    return result;
}

// 获取BIOS序列号
static QString getBiosSerialnumber()
{
    QString cmd = QString("bios get serialnumber");
    QString info = getWMICInfo(cmd);
    QStringList list = info.split(" ");
    QString key = "serialnumber";
    auto iter = list.begin();
    for (; iter != list.end(); )
    {
        if (!key.compare(*iter, Qt::CaseInsensitive))
        {
            list.erase(iter++);
            continue;
        }
        iter++;
    }
    QString result;
    for (auto str : list)
        result += str;
    qDebug() << "biosSerialnumber:" << result;
    return result;
}

// 获取CPU序列号
static QString getCpuSerialNumber()
{
    QString cmd = QString("cpu get processorId");
    QString info = getWMICInfo(cmd);
    QStringList list = info.split(" ");
    QString key = "processorId";
    auto iter = list.begin();
    for (; iter != list.end(); )
    {
        if (!key.compare(*iter, Qt::CaseInsensitive))
        {
            list.erase(iter++);
            continue;
        }
        iter++;
    }
    QString result;
    for (auto str : list)
        result += str;
    qDebug() << "cpuProcessorId:" << result;
    return result;
}

static QString getMachineCode()
{
    QString hostName = QHostInfo::localHostName();
    qDebug() << "hostName:" << hostName;
    QString biosSerialnumber = getBiosSerialnumber();
    QString cpuSerialNumber = getCpuSerialNumber();
    QString hostNameB64 = hostName.toUtf8().toBase64();
    QString biosSerialnumberB64 = biosSerialnumber.toUtf8().toBase64();
    QString cpuSerialNumberB64 = cpuSerialNumber.toUtf8().toBase64();
    QJsonObject json;
    json.insert("hostName", hostNameB64);
    json.insert("biosSN", biosSerialnumberB64);
    json.insert("cpuPId", cpuSerialNumberB64);
    QJsonDocument document;
    document.setObject(json);
    QByteArray byteArray = document.toJson(QJsonDocument::Compact);
    QString strJson(byteArray);

    return strJson;
}

static QString getRegistryValue(const QString& keyPath, const QString& valueName)
{
    QSettings settings(keyPath, QSettings::NativeFormat);
    return settings.value(valueName).toString();
}

static void setRegistryValue(const QString& keyPath, const QString& valueName, const QVariant& value)
{
    QSettings settings(keyPath, QSettings::NativeFormat);
    settings.setValue(valueName, value);
}

Register::Register(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
    setWindowIcon(QIcon(":/VTMR/images/VTMR/vtmr.ico"));
    Qt::WindowFlags windowFlag = Qt::Dialog;
    windowFlag |= Qt::WindowMinimizeButtonHint;
    windowFlag |= Qt::WindowCloseButtonHint;
    setWindowFlags(windowFlag);
	setWindowTitle(QString("注册"));
    ui.btnCopyMachineCode->setToolTip(QString("将机器码告知我司技术支持人员，获取注册码"));

    //拷贝机器码到剪切板
	connect(ui.btnCopyMachineCode, &QPushButton::clicked, 
		[=]() {
			QString source = ui.LEMachinecCde->text();
            if (source.isEmpty())
                return;
			QClipboard* clipboard = QApplication::clipboard();
			clipboard->setText(source);
		}
	);
	//注册
	connect(ui.btnRegister, &QPushButton::clicked, 
		[=]() {
            QString text = ui.LERegistCode->text();
            if (text.isEmpty())
            {
                setResult(QDialog::Rejected);
                return;
            }

            if (!checkRegistCode(text)) {
                setResult(QDialog::Rejected);
                return;
            }

            setRegistryValue(HKeyPath, HKeyRegCode, text);
            insertSql(text);
            accept();
        }
	);
}

Register::~Register()
{}

static qint64 getCurrentTime()
{
#if 1
    // 获取本地时间
    QDateTime localTime = QDateTime::currentDateTime();
    qint64 localTimeInSeconds = localTime.toSecsSinceEpoch();
#else
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl("http://worldclockapi.com/api/json/utc/now"));
    QNetworkReply* reply = manager.get(request);

    // 获取本地时间
    QDateTime localTime = QDateTime::currentDateTime();
    qint64 localTimeInSeconds = localTime.toSecsSinceEpoch();

    // 计算自1970年以来的秒数
    qint64 networkTimeInSeconds = 0;

    QObject::connect(reply, &QNetworkReply::finished, [&]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QString timeString = QString::fromUtf8(data);
            QDateTime networkTime = QDateTime::fromString(timeString, "yyyy-MM-ddTHH:mm:ssZ");
            networkTimeInSeconds = networkTime.toSecsSinceEpoch();
        }
        else {
            qDebug() << "Network request error:" << reply->errorString();
        }

        });

    QElapsedTimer timer;
    timer.start();
    while (1) {
        if (networkTimeInSeconds > 0)
            break;
        qint64 elapsedTime = timer.elapsed();
        if (elapsedTime > 10000)
        {
            networkTimeInSeconds = localTimeInSeconds;
            qDebug() << "get Network date request timeout,use local date time";
            break;
        }
        QThread::msleep(10);
    }
#endif
    return localTimeInSeconds;
}

bool Register::checkRegistCode(QString text)
{    int validity = 0;
    qint64 authorizationDate = 0;
    QString regMachine;
    QString regCode;
    if (text.isEmpty()) {
        regCode = getRegistryValue(HKeyPath, HKeyRegCode);
        if (regCode.isEmpty())
            regCode = loadRegCode();
        if (regCode.isEmpty())
            return false;
    }
    else {
        regCode = text;
    }


    QJsonParseError jsonError;
    QByteArray encryptedBytes = QByteArray::fromBase64(regCode.toUtf8());
    QByteArray decodeBytes = decryptAES(encryptedBytes, vtmrKey2, vtmrIV2);
    QString decodelog = decodeBytes;

    QJsonDocument doucment = QJsonDocument::fromJson(decodeBytes, &jsonError);
    if (doucment.isNull()) {
        qDebug() << "regist code parse json failed";
        return false;
    }

    if (jsonError.error != QJsonParseError::NoError) {
        qDebug() << "regist code parse json failed:" << jsonError.error;
        return false;
    }

    if (!doucment.isObject()) {
        qDebug() << "regist code parse json failed:not object";
        return false;
    }

    QJsonObject object = doucment.object();
    if (object.contains("a")) {//有效期时长单位小时
        QJsonValue value = object.value("a");
        if (value.isDouble()) {
            validity = value.toInt();
        }
        g_authTime = QString::number(validity);
    }
    else {
        qDebug() << "regist code parse json failed:no validity";
        return false;
    }

    if (object.contains("b")) {//授权日期
        QJsonValue value = object.value("b");
        if (value.isDouble()) {
            authorizationDate = value.toInt();
        }
        g_authDate = QString::number(authorizationDate);
    }
    else {
        qDebug() << "regist code parse json failed:no regTime";
        return false;
    }

    if (0 == validity || 0 == authorizationDate)
        return false;

    qint64 now = getCurrentTime();
    qint64 reg = authorizationDate + validity * 3600;
    if (now > reg) {
        QDateTime nowDateTime = QDateTime::fromSecsSinceEpoch(reg);
        QDateTime regDateTime = QDateTime::fromSecsSinceEpoch(reg);
        qDebug() << "有效期：" << nowDateTime.toString("yyyy-MM-dd hh:mm:ss") << " 当前时间：" << regDateTime.toString("yyyy-MM-dd hh:mm:ss");
        return false;

    }
    if (object.contains("c")) {
        QJsonValue value = object.value("c");
        if (value.isString()) {
            regMachine = value.toString();
        }
    }
    else {
        qDebug() << "regist code parse json failed:no regCode";
        return false;
    }

    QByteArray encodeBytes = encryptAES(m_machineCode.toUtf8(), vtmrKey2, vtmrIV2);
    QByteArray hash = QCryptographicHash::hash(encodeBytes, QCryptographicHash::Md5);
    QString code = hash.toHex().toBase64();

    return (code == regMachine);
}

void Register::generateMachineCode()
{
    m_machineCode = getMachineCode();
    qDebug() << "machineCode:" << m_machineCode;
    QByteArray encodeBytes = encryptAES(m_machineCode.toUtf8(), vtmrKey1, vtmrIV1);
    QString encryptedText = encodeBytes.toBase64();
    ui.LEMachinecCde->setText(encryptedText);
}

QString Register::loadRegCode()
{
    QString regCode;
    QSqlQuery query;
    QString sql = QString("SELECT * FROM  main.%1").arg(SQL_APP_INFO_TABLE);
    query.exec(sql);
    if (!query.exec())
        qDebug() << QString("%1 执行失败：%2").arg(sql).arg(query.lastError().text());
    else {
        QVector<QString> keys;
        while (query.next()) {
            regCode = query.value(SQL_APP_INFO_REGISTER_CODE).toString();
            qDebug() << QString("load regCode:%1").arg(regCode);
        }
    }
    return regCode;
}

void Register::insertSql(QString regCode)
{
    //创建表格
    QSqlQuery query;
    QString sql;
    sql = QString("CREATE TABLE IF NOT EXISTS main.%1(%2 text)")
        .arg(SQL_APP_INFO_TABLE).arg(SQL_APP_INFO_REGISTER_CODE);
    if (!query.exec(sql))
        qDebug() << QString("%1 执行失败：%2").arg(sql).arg(query.lastError().text());
    else
        CSQLManager::commit(query);

    //插入数据
    sql = QString("INSERT INTO main.%1(%2) VALUES('%3')")
        .arg(SQL_APP_INFO_TABLE)
        .arg(SQL_APP_INFO_REGISTER_CODE)
        .arg(regCode);
    if (!query.exec(sql))
        qDebug() << QString("%1 执行失败：%2").arg(sql).arg(query.lastError().text());
    else
        CSQLManager::commit(query);
}