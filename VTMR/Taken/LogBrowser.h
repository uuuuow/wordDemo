#pragma once

#include <QTextBrowser>
class CLogBrowser  : public QTextBrowser
{
	Q_OBJECT

public:
	CLogBrowser(QWidget *parent);
	~CLogBrowser();
	void keyPressEvent(QKeyEvent* event) override;
};
