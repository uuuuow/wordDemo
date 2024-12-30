#include "VtmrKeyGenerator.h"
#include <QCryptographicHash>
#include <QByteArray>
#include <QDebug>
#include <QClipboard>
#include <QProcess>
#include <QHostInfo>
#include <QFile>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageBox>
#include <Windows.h>
#include <wincrypt.h>
extern "C" {
#include <openssl/evp.h>
#include <openssl/x509.h>
}

//F02AC97053810BDE10A379A5
static QByteArray vtmrKey1 = "10235970538102723103461710235976";
static QByteArray vtmrIV1 = "5329102572014261";

//E315B94221817C1F239A2801
static QByteArray vtmrKey2 = "53213039431002817539024043593135";
static QByteArray vtmrIV2 = "1592673527904379";

static QString HKeyPath = "HKEY_CURRENT_USER\\Software\\vtmr";
static QString HKeyValueName = "registcode";

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

static bool paserMachineCode(QString machineJson)
{
    QJsonParseError jsonError;
    QJsonDocument doucment = QJsonDocument::fromJson(machineJson.toUtf8(), &jsonError);
    if (doucment.isNull())
        return false;

    if (jsonError.error != QJsonParseError::NoError)
        return false;

    if (!doucment.isObject())
        return false;

    QJsonObject object = doucment.object();
    if (object.contains("hostName")) {
        QJsonValue value = object.value("hostName");
        if (value.isString()) {
            QString tmp = value.toString();
            qDebug() << "hostName: " << QString(QByteArray::fromBase64(tmp.toUtf8()));
        }
    }
    if (object.contains("biosSN")) {
        QJsonValue value = object.value("biosSN");
        if (value.isString()) {
            QString tmp = value.toString();
            qDebug() << "biosSerialnumber: " << QString(QByteArray::fromBase64(tmp.toUtf8()));
        }
    }
    if (object.contains("cpuPId")) {
        QJsonValue value = object.value("cpuPId");
        if (value.isString()) {
            QString tmp = value.toString();
            qDebug() << "cpuProcessorId: " << QString(QByteArray::fromBase64(tmp.toUtf8()));
        }
    }
    return true;
}

VtmrKeyGenerator::VtmrKeyGenerator(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    Qt::WindowFlags windowFlag = Qt::Dialog;
    windowFlag |= Qt::WindowMinimizeButtonHint;
    windowFlag |= Qt::WindowCloseButtonHint;
    setWindowFlags(windowFlag);
    setWindowTitle(QString("注册机"));
    ui.btnCopyRegistCode->setToolTip(QString("将注册码告知VTMR用户，用于注册"));

    //生成注册码
    connect(ui.btnGenRegistCode, &QPushButton::clicked, 
        [=]() {
            int year = 0, month = 0, day = 0;
            QString source = ui.LEMachinecCde->text();
            source = source.trimmed();
            if (source.isEmpty())
                return;
            qDebug() << "Encode machinieCode:" << source;

            QByteArray encryptedBytes = QByteArray::fromBase64(source.toUtf8());
            QByteArray decryptedBytes = decryptAES(encryptedBytes, vtmrKey1, vtmrIV1);
            QString decryptedText = decryptedBytes;
            qDebug() << "machinieCode:" << decryptedText;
            if (!paserMachineCode(decryptedText))
            {
                QString text = "机器码损坏";
                QMessageBox::information(this, QString("注册机"), text);
                return;
            }

            year = m_validityYear;
            month = m_validityMonth;
            day = m_validityDay;

            if (0 == m_validityYear && 0 == m_validityMonth && 0 == m_validityDay)
            {
                QString text = "未设置有效期将生成无限有效期注册码";
                QMessageBox::information(this, QString("注册机"), text);
                year = 99;
            }

            int validity = (year * 365 + month * 30 + day) * 24;
            QJsonObject object;
            object.insert("a", validity);
            QDateTime localTime = QDateTime::currentDateTime();
            qint64 localTimeInSeconds = localTime.toSecsSinceEpoch();
            object.insert("b", localTimeInSeconds);

            QByteArray encodeBytes = encryptAES(decryptedBytes, vtmrKey2, vtmrIV2);
            QByteArray hash = QCryptographicHash::hash(encodeBytes, QCryptographicHash::Md5);
            QString registrationCode = hash.toHex().toBase64();
            object.insert("c", registrationCode);

            QJsonDocument jsonDocument(object);
            QString registJson = jsonDocument.toJson(QJsonDocument::Compact);
            QByteArray registBytes = encryptAES(registJson.toUtf8(), vtmrKey2, vtmrIV2);
            QString registCode = registBytes.toBase64();

            ui.LERegistCode->setText(registCode);
        }
    );
    //拷贝注册码到剪切板
    connect(ui.btnCopyRegistCode, &QPushButton::clicked,
        [=]() {
            QString source = ui.LERegistCode->text();
            QClipboard* clipboard = QApplication::clipboard();
            clipboard->setText(source);
        }
    );

    //有效期
    connect(ui.SBYear, &QSpinBox::valueChanged, [this](int value) {
        m_validityYear = value;
        });
    connect(ui.SBMonth, &QSpinBox::valueChanged, [this](int value) {
        m_validityMonth = value;
        });
    connect(ui.SBDay, &QSpinBox::valueChanged, [this](int value) {
        m_validityDay = value;
        });
    m_validityYear = ui.SBYear->value();
    m_validityMonth = ui.SBMonth->value();
    m_validityDay = ui.SBDay->value();
}

VtmrKeyGenerator::~VtmrKeyGenerator()
{}
