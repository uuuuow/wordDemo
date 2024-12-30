#pragma once

#include <QtWidgets/QDialog>
#include "ui_VtmrKeyGenerator.h"

class VtmrKeyGenerator : public QDialog
{
    Q_OBJECT

public:
    VtmrKeyGenerator(QWidget *parent = nullptr);
    ~VtmrKeyGenerator();

private:
    Ui::VtmrKeyGeneratorClass ui;
    int m_validityYear = 0;
    int m_validityMonth = 0;
    int m_validityDay = 0;
};


// 获取主板序列号
QString GetBaseboardSerialnumber();

// 获取BIOS序列号
QString GetBIOSSerialnumber();