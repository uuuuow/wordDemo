#include "docx.h"
#include <windows.h>
#include <QDebug>
#include <QStandardPaths>
#include <QAxBase.h>
#include <QException>
#include "Utils.h"
#include <Windows.h>

CDocx::CDocx(QObject* parent)
    : QObject(parent)
{
}

CDocx::~CDocx()
{
    OleUninitialize();
}

bool CDocx::openNewWord(const bool &show)
{
    HRESULT r = OleInitialize(0);
    if (r != S_OK && r != S_FALSE)
    {
        qCritical() << QStringLiteral("初始化失败");
        return false;
    }

    m_wordApp = new QAxObject();
	bool ret = m_wordApp->setControl("word.Application");
	if (!ret)
	{
		ret = m_wordApp->setControl("kwps.Application");
		if (!ret)
			return false;
	}

	QAxObject *pApplication = m_wordApp->querySubObject("Application");
	QVariant appName = pApplication->property("Name");
	QVariant appVersion = pApplication->property("Version");
	qDebug() << "Name:" << appName.toString();
	qDebug() << "Version:" << appVersion.toString();

    m_wordApp->setProperty("Visible", false);

    QAxObject * pDocuments = m_wordApp->querySubObject("Documents");
    if(!pDocuments){
		qDebug() << QStringLiteral("pDocuments==null");
		return false;
    }
	QString strDesktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	strDesktopPath += QString("/司法鉴定科学研究院司法鉴定意见书.docx");

	//pDocuments->dynamicCall("Add(QString)", strDesktopPath);
	//pDocuments->dynamicCall("Add(QString)", "");
    //m_saveDocument = m_wordApp->querySubObject("ActiveDocument");
	m_saveDocument = pDocuments->querySubObject("Add(QVariant)", QVariant(QString()));
	if(!m_saveDocument){
		qDebug() << QStringLiteral("m_saveDocument==null");
		return false;
    }
	//m_saveDocument->setProperty("Name", QString("司法鉴定科学研究院司法鉴定意见书"));
    m_selection = m_wordApp->querySubObject("Selection");
    QAxObject *paragraphFormat = m_selection->querySubObject("ParagraphFormat");
    paragraphFormat->dynamicCall("SetLineSpacingRule(QVariant)",LineSpace1pt5);//1代表1.5倍行间距
    m_windowActive = m_wordApp->querySubObject("ActiveWindow");
    return true;
}

void CDocx::saveWord(const QString &path)
{
    m_saveDocument->dynamicCall("SaveAs(QString)",path);
}

void CDocx::quitWord()
{
    if (!m_wordApp)
        return;
    QPoint pos = VTDeskPosition();
    QAxObject * ActiveWindow = m_saveDocument->querySubObject("ActiveWindow");
    HWND wordWindowHandle = (HWND)m_windowActive->property("Hwnd").toInt();
    SetWindowPos(wordWindowHandle, HWND_TOPMOST, pos.x(), pos.y(), 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    m_wordApp->setProperty("DisplayAlerts", false);
    
    m_saveDocument->dynamicCall("Close(bool)", true);
    m_wordApp->dynamicCall("Quit()");

    deleteObject();
}
void CDocx::wdStyleTitle()
{
	m_selection->dynamicCall("setStyle(WdBuiltinStyle)", "wdStyleTitle");//设置标题
}

void CDocx::wdStyleSubtitle()
{
	m_selection->dynamicCall("setStyle(WdBuiltinStyle)", "wdStyleSubtitle");//设置副标题
}

void CDocx::wdStyleHeading1()
{
	m_selection->dynamicCall("setStyle(WdBuiltinStyle)", "wdStyleHeading1");//设置标题1
}

void CDocx::newLine(int line)
{
    if(line <= 0)
        return;

    for(int i = 0; i < line;++i){
        m_selection->dynamicCall("TypeParagraph(void)");
    }

}

void CDocx::indent()
{
	m_selection->dynamicCall("setStyle(WdBuiltinStyle)", "wdStyleBodyTextFirstIndent2");
}

void CDocx::Tab()
{
	QAxObject *paragraphFormat = m_selection->querySubObject("ParagraphFormat");
	paragraphFormat->dynamicCall("Tab");
}

void CDocx::changePage()
{
    m_selection->dynamicCall("InsertBreak(QVariant)", 7);
}

void CDocx::setTitleText(const QString &title,
                         CDocx::TitleLevel level)
{
	m_selection->dynamicCall("SetStyle(QVariant)",(const QVariant &)(level));
	m_selection->dynamicCall("TypeText(QString)",title);

    newLine();

    iniParagraphText();
}

void CDocx::setCenterTitleText(const QString &title)
{
    auto paragraphFormat = m_selection->querySubObject("ParagraphFormat");
    paragraphFormat->dynamicCall("SetAlignment(QVariant)",
                                  CDocx::AliginCenter);
    m_selection->dynamicCall("TypeText(QString)",title);
}

void CDocx::iniParagraphText()
{
    //设置段落格式为正文
    auto paragraphFormat = m_selection->querySubObject("ParagraphFormat");
    paragraphFormat->dynamicCall("SetStyle(QVariant)",-67);
}

void CDocx::setLineSpace(CDocx::LineSpacing space)
{
    auto paragraphFormat = m_selection->querySubObject("ParagraphFormat");
    paragraphFormat->dynamicCall("SetLineSpacingRule(QVariant)",space);//1代表1.5倍行间距
}

void CDocx::insertPageHead(const QString &text)
{
    m_windowActive = m_wordApp->querySubObject("ActiveWindow");
    m_pane = m_windowActive->querySubObject("ActivePane");
    m_viewActive = m_pane->querySubObject("View");
    m_viewActive->dynamicCall("SetSeekView(QVariant)",9);

    addText(text);

    auto paragraphFormat = m_selection->querySubObject("ParagraphFormat");
    paragraphFormat->dynamicCall("SetAlignment(QVariant)",
                                  CDocx::AliginCenter);

    m_viewActive->dynamicCall("SetSeekView(QVariant)",0);
}

void CDocx::insertPageNumber()
{
    auto fields = m_selection->querySubObject("Fields");
    m_windowActive = m_wordApp->querySubObject("ActiveWindow");
    m_pane = m_windowActive->querySubObject("ActivePane");
    m_viewActive = m_pane->querySubObject("View");
    m_viewActive->dynamicCall("SetSeekView(QVariant)",10);

    auto paragraphFormat = m_selection->querySubObject("ParagraphFormat");
    paragraphFormat->dynamicCall("SetAlignment(QVariant)",
                                  CDocx::AliginCenter);
    addText("- ");
    fields->dynamicCall("Add(QVariant,QVariant,QVariant,QVariant)",
                         m_selection->querySubObject("Range")->asVariant(),
                         33,"PAGE  ",1);
    addText(" -");
    m_selection->dynamicCall("TypeParagraph()");
    m_viewActive->dynamicCall("SetSeekView(QVariant)",0);

}

void CDocx::insertPageNumberV2(int pages)
{
	auto fields = m_selection->querySubObject("Fields");
	m_windowActive = m_wordApp->querySubObject("ActiveWindow");
	m_pane = m_windowActive->querySubObject("ActivePane");
	m_viewActive = m_pane->querySubObject("View");
	m_viewActive->dynamicCall("SetSeekView(QVariant)", 10);

	auto paragraphFormat = m_selection->querySubObject("ParagraphFormat");
    paragraphFormat->dynamicCall("SetAlignment(QVariant)",
		CDocx::AliginRight);
	setFontName(QString("宋体"));
	setFontStyle(12);
	addText(QString("第"));
    fields->dynamicCall("Add(QVariant,QVariant,QVariant,QVariant)",
		m_selection->querySubObject("Range")->asVariant(), 33, "PAGE", 1);
	QString text = QString("页，共%1页").arg(pages);
	addText(text);
	m_selection->dynamicCall("TypeParagraph()");
	m_viewActive->dynamicCall("SetSeekView(QVariant)", 0);
}

void CDocx::insertCatalogue()
{
    setFontStyle(18,true); //正文字体
    addText(QStringLiteral("                    目 录\n\n"));
    m_tablesOfContents = m_saveDocument->querySubObject("TablesOfContents");

    auto range=m_selection->querySubObject("Range");
    m_tablesOfContents->dynamicCall("Add(QVariant)", range->asVariant());
}

void CDocx::updateCatalogue()
{
    int count = m_tablesOfContents->property("Count").toInt();
    if (count > 0){
        auto tableOfContent = m_tablesOfContents->querySubObject("Item(int)" ,count);
        tableOfContent->dynamicCall("UpdatePageNumbers(void)");
    }
}

void CDocx::addText(const QString &text)
{
    m_selection->dynamicCall("TypeText(QString)",text);
}

void CDocx::addNumber_E(const double &value)
{
    QString text = QString::number(value,'e');
    m_selection->dynamicCall("TypeText(QString)",text);
}

void CDocx::addNumber_Int(const int &num)
{
    QString text = QString::number(num);
    m_selection->dynamicCall("TypeText(QString)",text);
}

void CDocx::addNumber_Float(const float &value)
{
    QString text = QString::number(value,'f');
    m_selection->dynamicCall("TypeText(QString)",text);
}

void CDocx::addPic(const QString path, float width, float height)
{
	if (!m_selection)
	{
		return;
	}
    auto paragraphFormat = m_selection->querySubObject("ParagraphFormat");
	if (!paragraphFormat)
	{
		return;
	}
    paragraphFormat->dynamicCall("SetLineSpacingRule(QVariant)",0);
    paragraphFormat->dynamicCall("SetAlignment(QVariant)",
                                  CDocx::AliginCenter);
	

	auto inlineShapes = m_selection->querySubObject("InlineShapes");
	if(!inlineShapes)
	{
		return;
	}
    inlineShapes->dynamicCall("AddPicture(const &QString)",path);

    inlineShapes = m_saveDocument->querySubObject("InlineShapes");

	int count = inlineShapes->property("Count").toInt();
	auto inlineShape = inlineShapes->querySubObject("Item(int)", count);
	if (!inlineShape)
		return;
	if(width > 0)
        inlineShape->setProperty("Width", width);
	if(height > 0)
        inlineShape->setProperty("Height", height);
    inlineShape->setProperty("LockAspectRatio", "msoTrue");
}

void CDocx::setFontStyle(const float &size,
                         const bool &bold,
                         bool italic,
                         bool underLine)
{
    if(m_selection->isNull()){
        qWarning() << QStringLiteral("Selection为空，字体设置失败!");
        return;
    }
    m_font = m_selection->querySubObject("Font");
    m_font->dynamicCall("SetSize(float)",size);
}

void CDocx::setTextAlign(CDocx::TextAlign textAlign)
{
    auto paragraphFormat = m_selection->querySubObject("ParagraphFormat");
    paragraphFormat->dynamicCall("SetAlignment(int)",static_cast<int>(textAlign));
}

void CDocx::setTextColor(const QColor &color)
{
    auto font = m_selection->querySubObject("Font");
    font->dynamicCall("SetColor(QVariant)",color);//设置颜色参数使用QVariant即可
}

void CDocx::setFontName(const QString &name)
{
    auto font = m_selection->querySubObject("Font");
    font->dynamicCall("SetName(QString)",name);
}

void CDocx::addTable(const int &row,
                     const int &col,
                     CDocx::TableFitBehavior autoFit)
{
    auto tables = m_saveDocument->querySubObject("Tables");
    auto range = m_selection->querySubObject("Range");

    tables->dynamicCall("Add(QVariant,int,int,QVariant,QVariant)",
        range->asVariant(),row,col, 1,static_cast<int>(autoFit));
}

void CDocx::setTableWidth(const int &tableIndex, const float &width)
{
    auto tables = m_saveDocument->querySubObject("Tables");
    auto table = tables->querySubObject("Item(int)",tableIndex);
    table->setProperty("PreferredWidthType",2);
    table->setProperty("PreferredWidth",width);
}

void CDocx::setTableColWidth(const int &tableIndex,
                             const int &col,
                             const float &width)
{
    auto tables = m_saveDocument->querySubObject("Tables");
    auto table = tables->querySubObject("Item(int)",tableIndex);
    auto columns = table->querySubObject("Columns");
    auto column = columns->querySubObject("Item(int)",col);
    column->setProperty("PreferredWidthType",2);
    column->setProperty("PreferredWidth",width);
}

void CDocx::setTableRowHeight(const int tableIndex,
                              const int &row,
                              const float &height)
{
    auto tables = m_saveDocument->querySubObject("Tables");
    auto table = tables->querySubObject("Item(int)",tableIndex);
    auto rows = table->querySubObject("Rows");
    auto rowObj = rows->querySubObject("Item(int)",row);
    rowObj->setProperty("Height",height);
}

void CDocx::setCellsBorderStyle(const int &tableIndex,
                                const int &startRow,
                                const int &startCol,
                                const int &endRow,
                                const int &endCol,
                                const CDocx::LineStyle &top,
                                const CDocx::LineStyle &bottom,
                                const CDocx::LineStyle &left,
                                const CDocx::LineStyle &right)
{
    auto tables = m_saveDocument->querySubObject("Tables");
    auto table = tables->querySubObject("Item(int)",tableIndex);
    auto rows = table->querySubObject("Rows");
    auto colums = table->querySubObject("Columns");

    int rowCount = rows->property("Count").toInt();
    int colCount = colums->property("Count").toInt();

    table->dynamicCall("Select(void)");
    auto range = m_selection->querySubObject("Range");

    //行
    range->dynamicCall("MoveStart(QVariant,QVariant)",10,startRow - 1);
    range->dynamicCall("MoveEnd(QVariant,QVariant)",10,endRow - rowCount);

    //列
    range->dynamicCall("MoveStart(QVariant,QVariant)",12,startCol - 1);
    range->dynamicCall("MoveEnd(QVariant,QVariant)",12,endCol - colCount);
    range->dynamicCall("Select(void)");

    auto borders = m_selection->querySubObject("Borders");
    borders->querySubObject("Item(int)",1)
            ->dynamicCall("SetLineStyle(int)",static_cast<int>(top));
    borders->querySubObject("Item(int)",2)
            ->dynamicCall("SetLineStyle(int)",static_cast<int>(left));
    borders->querySubObject("Item(int)",3)
            ->dynamicCall("SetLineStyle(int)",static_cast<int>(bottom));
    borders->querySubObject("Item(int)",4)
            ->dynamicCall("SetLineStyle(int)",static_cast<int>(right));

}

void CDocx::setCellBorderStyle(const int &tableIndex,
                               const int &row,
                               const int &col,
                               const CDocx::LineStyle &top,
                               const CDocx::LineStyle &bottom,
                               const CDocx::LineStyle &left,
                               const CDocx::LineStyle &right)
{
    auto tables = m_saveDocument->querySubObject("Tables");
    auto table = tables->querySubObject("Item(int)",tableIndex);
    auto cell = table->querySubObject("Cell(int,int)",row,col);
    cell->dynamicCall("Select(void)");

    auto borders = cell->querySubObject("Borders");
    borders->querySubObject("Item(int)",1)
            ->dynamicCall("SetLineStyle(int)",static_cast<int>(top));
    borders->querySubObject("Item(int)",2)
            ->dynamicCall("SetLineStyle(int)",static_cast<int>(left));
    borders->querySubObject("Item(int)",3)
            ->dynamicCall("SetLineStyle(int)",static_cast<int>(bottom));
    borders->querySubObject("Item(int)",4)
            ->dynamicCall("SetLineStyle(int)",static_cast<int>(right));
}

void CDocx::setCellsColor(const int &tableIndex,
                          const int &startRow,
                          const int &startCol,
                          const int &endRow,
                          const int &endCol,
                          const QColor &color)
{
    auto tables = m_saveDocument->querySubObject("Tables");
    auto table = tables->querySubObject("Item(int)",tableIndex);
    auto rows = table->querySubObject("Rows");
    auto columns = table->querySubObject("Columns");

    int rowCount = rows->property("Count").toInt();
    int colCount = columns->property("Count").toInt();

    table->dynamicCall("Select(void)");
    auto range = m_selection->querySubObject("Range");

    //行
    range->dynamicCall("MoveStart(QVariant,QVariant)",10,startRow - 1);
    range->dynamicCall("MoveEnd(QVariant,QVariant)",10,endRow - rowCount);

    //列
    range->dynamicCall("MoveStart(QVariant,QVariant)",12,startCol - 1);
    range->dynamicCall("MoveEnd(QVariant,QVariant)",12,endCol - colCount);

    range->dynamicCall("Select(void)");

    auto shading = m_selection->querySubObject("Shading");
    shading->dynamicCall("SetTexture(int)",1000);
    shading->dynamicCall("SetBackgroundPatternColor(QVariant)",QColor(255,255,255));
    shading->dynamicCall("SetForegroundPatternColor(QVariant)",color);
}

void CDocx::setCellColor(const int &tableIndex,
                         const int &row,
                         const int &col,
                         const QColor &color)
{
    auto tables = m_saveDocument->querySubObject("Tables");
    auto table = tables->querySubObject("Item(int)",tableIndex);
    auto cell = table->querySubObject("Cell(int,int)",row,col);
    cell->dynamicCall("Select(void)");

    auto shading = m_selection->querySubObject("Shading");
    shading->dynamicCall("SetTexture(int)",1000);
    shading->dynamicCall("SetBackgroundPatternColor(QVariant)",QColor(255,255,255));
    shading->dynamicCall("SetForegroundPatternColor(QVariant)",color);
}

void CDocx::selectTable(const int &tableIndex)
{
    auto tables = m_saveDocument->querySubObject("Tables");
    auto table = tables->querySubObject("Item(int)",tableIndex);
    table->dynamicCall("Select(void)");
}

void CDocx::moveToTableEnd(const int &tableIndex)
{
    auto tables = m_saveDocument->querySubObject("Tables");
    auto table = tables->querySubObject("Item(int)",tableIndex);
    table->dynamicCall("Select(void)");

    m_selection->dynamicCall("MoveRight(QVariant,QVariant,QVariant)",1,1,0);
}

void CDocx::spanCells(const int &tableIndex,
                      const int &startRow,
                      const int &startCol,
                      const int &endRow,
                      const int &endCol)
{
    auto tables = m_saveDocument->querySubObject("Tables");
    auto table = tables->querySubObject("Item(int)",tableIndex);
    auto cell1 = table->querySubObject("Cell(int,int)",startRow,startCol);
    auto cell2 = table->querySubObject("Cell(int,int)",endRow,endCol);

    cell1->dynamicCall("Merge(QVariant)", cell2->asVariant());
    cell1->dynamicCall("Select(void)");
    cell1->dynamicCall("SetVerticalAlignment(QVariant)",CDocx::AliginCenter);

    auto paragraphFormat = m_selection->querySubObject("ParagraphFormat");
    paragraphFormat->dynamicCall("SetAlignment(QVariant)",CDocx::AliginLeft);
}

void CDocx::setCellText(const int &tableIndex,
                        const int &row,
                        const int &col,
                        const QString &text)
{
    auto tables = m_saveDocument->querySubObject("Tables");
    auto table = tables->querySubObject("Item(int)",tableIndex);
    auto cell = table->querySubObject("Cell(int,int)",row,col);

    cell->dynamicCall("Select(void)");
    addText(text);
}

void CDocx::setCellTextAlign(const int &tableIndex,
                             const int &row,
                             const int &col,
                             CDocx::TextAlign align)
{
    auto tables = m_saveDocument->querySubObject("Tables");
    auto table = tables->querySubObject("Item(int)",tableIndex);
    auto cell = table->querySubObject("Cell(int,int)",row,col);

    cell->dynamicCall("Select(void)");
    setTextAlign(align);
}

void CDocx::setCellTextColor(const int &tableIndex,
                             const int &row,
                             const int &col,
                             const QColor &color)
{
    auto tables = m_saveDocument->querySubObject("Tables");
    auto table = tables->querySubObject("Item(int)",tableIndex);
    auto cell = table->querySubObject("Cell(int,int)",row,col);
    cell->dynamicCall("Select(void)");

    setTextColor(color);
}

void CDocx::setCellFont(const int &tableIndex,
                        const int &row,
                        const int &col,
                        QString fontName,
                        float fontSize,
                        bool bold,
                        bool italic,
                        bool underLine)
{
    auto tables = m_saveDocument->querySubObject("Tables");
    auto table = tables->querySubObject("Item(int)",tableIndex);
    auto cell = table->querySubObject("Cell(int,int)",row,col);
    cell->dynamicCall("Select(void)");

    QAxObject *font = m_selection->querySubObject("Font");
    font->dynamicCall("SetName(QString)",fontName);
    font->dynamicCall("SetSize(float)",fontSize);
    font->dynamicCall("SetBold(bool)",bold);
    font->dynamicCall("SetItalic(bool)",italic);
    font->dynamicCall("SetUnderline(bool)",underLine);

    m_selection->dynamicCall("SetFont(QVariant)",font->asVariant());
}

void CDocx::setTableFont(const int &tableIndex,
                         QString fontName,
                         float fontSize,
                         bool bold,
                         bool italic,
                         bool underLine)
{
    QAxObject *tables = m_saveDocument->querySubObject("Tables");
    QAxObject *table = tables->querySubObject("Item(int)",tableIndex);
    table->dynamicCall("Select(void)");

    QAxObject *font = m_selection->querySubObject("Font");
    font->dynamicCall("SetName(QString)",fontName);
    font->dynamicCall("SetSize(float)",fontSize);
    font->dynamicCall("SetBold(bool)",bold);
    font->dynamicCall("SetItalic(bool)",italic);
    font->dynamicCall("SetUnderline(bool)",underLine);

    m_selection->dynamicCall("SetFont(QVariant)",font->asVariant());
}

void CDocx::setTableTextAlign(const int &tableIndex,
                              CDocx::TextAlign aligin)
{
    QAxObject *tables = m_saveDocument->querySubObject("Tables");
    QAxObject *table = tables->querySubObject("Item(int)",tableIndex);

    table->dynamicCall("Select(void)");

    QAxObject *pCells = m_selection->querySubObject("Cells");
    pCells->dynamicCall("SetVerticalAlignment(QVariant)",aligin);
}

void CDocx::setCellPicture(const int &tableIndex,
                           const int &row,
                           const int &col,
                           const QString &path)
{
    QAxObject *tables = m_saveDocument->querySubObject("Tables");
    QAxObject *table = tables->querySubObject("Item(int)",tableIndex);
    QAxObject *cell = table->querySubObject("Cell(int,int)",row,col);

    cell->dynamicCall("Select(void)");
    cell->dynamicCall("SetVerticalAlignment(QVariant)",CDocx::AliginCenter);
    cell->dynamicCall("Select(void)");

    QAxObject *paragraphFormat = m_selection->querySubObject("ParagraphFormat");
    paragraphFormat->dynamicCall("SetAlignment(QVariant)",CDocx::AliginCenter);

    QAxObject *pRange = cell->querySubObject("Range");
    QAxObject *pInlineShapes = pRange->querySubObject("InlineShapes");
    pInlineShapes->dynamicCall("AddPicture(QString)",path);
}

void CDocx::releaseDispatch(QAxObject *object)
{
    object->dynamicCall("ReleaseDispatch()");
}

QAxObject *CDocx::getTable(const int &tableIndex)
{
    QAxObject *tables = m_saveDocument->querySubObject("Tables");
    return tables->querySubObject("Item(int)",tableIndex);
}

void CDocx::deleteObject()
{
    if(m_wordApp){
        m_wordApp->deleteLater();
        m_wordApp = nullptr;
    }
    if(m_selection){
        m_selection->deleteLater();
        m_selection = nullptr;
    }
    if(m_font){
        m_font->deleteLater();
        m_font = nullptr;
    }
    if(m_saveDocument){
        m_saveDocument->deleteLater();
        m_font = nullptr;
    }

    if(m_windowActive){
        m_windowActive->deleteLater();
        m_windowActive = nullptr;
    }
    if(m_viewActive){
        m_viewActive->deleteLater();
        m_viewActive = nullptr;
    }
    if(m_pane){
        m_pane->deleteLater();
        m_pane = nullptr;
    }
}
void CDocx::test(int num)
{
	//设置段落格式为正文
	newLine(1);
	setTextStyle(num);
	QString strT = QString("%1").arg(num);
	addText(strT);
}

void CDocx::setTextStyle(int num)
{
	//设置段落格式为正文
	QAxObject *style = m_saveDocument->querySubObject("Styles(const QVariant&)", QString("正文"));
	m_selection->setProperty("Style", style->asVariant());
}

void CDocx::setContext(const QString &context,
	const QString &fontName,
	const QColor &color,
	const float &size,
	const bool &bold,
	bool italic,
	bool underLine)
{
	setFontName(fontName);
	setFontStyle(size, bold, italic, underLine);
	setTextColor(color);
	if(context != "")
		addText(context);
}

void CDocx::slots_exception(int a, QString b, QString c, QString d)
{
	qDebug() << "slots_exception a=" << a << " b=" << b << " c=" << c << " d" << d;
}

static double CentimetersToPoints(double cm) {
	// 将厘米转换为英寸
	double inch = cm / 2.54;
	// 将英寸转换为磅（1英寸 = 72磅）
	return inch * 72;
}

void CDocx::ApplyListTemplate(int index, int seq)
{
	m_selection->querySubObject("Font")->setProperty("Name", QVariant(QString("黑体")));
	m_selection->querySubObject("Font")->setProperty("Size", QVariant(14));

	QAxObject *listGalleries = m_wordApp->querySubObject("ListGalleries");
	QAxObject *wdNumberGallery = listGalleries->querySubObject("Item(wdNumberGallery)");
	QAxObject *listTemplates = wdNumberGallery->querySubObject("ListTemplates");
	//QAxObject *listTemplate = listTemplates->querySubObject("Item(1)");
	QAxObject *listTemplate = listTemplates->querySubObject("Item(int)", index);
	QAxObject *listLevels = listTemplate->querySubObject("ListLevels");
	QAxObject *listLevel = listLevels->querySubObject("Item(1)");

	if(index == 1)
		listLevel->setProperty("NumberFormat", QVariant(QString("%1、").arg(VTNumToChinese(QString::number(seq)))));
	else if(index == 2)
		listLevel->setProperty("NumberFormat", QVariant(QString("(%1)").arg(VTNumToChinese(QString::number(seq)))));
	listLevel->setProperty("TrailingCharacter", QVariant((int)0));//wdTrailingTab
	listLevel->setProperty("NumberStyle", QVariant((int)39));//wdListNumberStyleSimpChinNum3
	listLevel->setProperty("NumberPosition", QVariant(0));
	listLevel->setProperty("Alignment", QVariant((int)0));//wdListLevelAlignLeft
	listLevel->setProperty("TextPosition", QVariant(0.74));
	listLevel->setProperty("TabPosition", QVariant((int)0));//wdUndefined
	listLevel->setProperty("ResetOnHigher", QVariant(0));
	listLevel->setProperty("StartAt", QVariant(1));

	QAxObject *font = listLevel->querySubObject("Font");
	font->setProperty("Bold", QVariant((int)0));
	font->setProperty("Italic", QVariant((int)0));
	font->setProperty("StrikeThrough", QVariant((int)0));
	font->setProperty("Subscript", QVariant((int)0));
	font->setProperty("Superscript", QVariant((int)0));
	font->setProperty("Shadow", QVariant((int)0));
	font->setProperty("Outline", QVariant((int)0));
	font->setProperty("Emboss", QVariant((int)0));
	font->setProperty("Engrave", QVariant((int)0));
	font->setProperty("AllCaps", QVariant((int)0));
	font->setProperty("Hidden", QVariant((int)0));
	font->setProperty("Underline", QVariant((int)0));
	font->setProperty("Color", QVariant((int)0));
	font->setProperty("Size", QVariant((int)0));
	font->setProperty("Animation", QVariant((int)0));
	font->setProperty("DoubleStrikeThrough", QVariant((int)0));
	font->setProperty("Name", QVariant(""));

	listLevel->setProperty("LinkedStyle", QVariant(""));

	listTemplate->setProperty("Name", QVariant(""));

	QAxObject *listFormat = m_selection->querySubObject("Range")->querySubObject("ListFormat");
	listFormat->dynamicCall("ApplyListTemplate(QAxObject*, QVariant, QVariant, QVariant)", listTemplate->asVariant(), QVariant((int)false), QVariant((int)0/*wdListApplyToWholeList*/), QVariant((int)0/*wdWord10ListBehavior*/));
}

void CDocx::Indent()
{
	//Paragraphs.Item(1).Indent()
	QAxObject *Range = m_selection->querySubObject("Range");
	QAxObject *Paragraphs = Range->querySubObject("Paragraphs");
	QAxObject *Paragraph = Paragraphs->querySubObject("Item(int)", 1);
	Paragraph->dynamicCall("Indent()");
}

void CDocx::ParagraphFormat(
	float CharacterUnitLeftIndent, //缩进，文本之前N字符
	float CharacterUnitRightIndent, //文本之后N字符
	float CharacterUnitFirstLineIndent, //悬挂缩进
	int Alignment //对齐方式
)
{
	QAxObject *ParagraphFormat = m_selection->querySubObject("ParagraphFormat");
	ParagraphFormat->setProperty("Alignment ", Alignment);
	ParagraphFormat->setProperty("CharacterUnitRightIndent", CharacterUnitRightIndent);
	ParagraphFormat->setProperty("CharacterUnitLeftIndent", CharacterUnitLeftIndent);
	ParagraphFormat->setProperty("CharacterUnitFirstLineIndent", CharacterUnitFirstLineIndent);
	ParagraphFormat->setProperty("FirstLineIndent", "wdLineSpaceExactly");
	ParagraphFormat->setProperty("LineSpacing", 26);
	ParagraphFormat->setProperty("FirstLineIndent", 0);
	ParagraphFormat->setProperty("LineSpacingRule", "wdLineSpaceExactly");
	ParagraphFormat->setProperty("DisableLineHeightGrid", 0);
	ParagraphFormat->setProperty("ReadingOrder", "wdReadingOrderLtr");
	ParagraphFormat->setProperty("AutoAdjustRightIndent", -1);
	ParagraphFormat->setProperty("WidowControl", 0);
	ParagraphFormat->setProperty("KeepWithNext", 0);
	ParagraphFormat->setProperty("KeepTogether", 0);
	ParagraphFormat->setProperty("PageBreakBefore", 0);
	ParagraphFormat->setProperty("FarEastLineBreakControl", -1);
	ParagraphFormat->setProperty("WordWrap", -1);
	ParagraphFormat->setProperty("HangingPunctuation", -1);
	ParagraphFormat->setProperty("HalfWidthPunctuationOnTopOfLine", 0);
	ParagraphFormat->setProperty("AddSpaceBetweenFarEastAndAlpha", -1);
	ParagraphFormat->setProperty("AddSpaceBetweenFarEastAndDigit", -1);
	ParagraphFormat->setProperty("BaseLineAlignment", "wdBaselineAlignAuto");
	ParagraphFormat->setProperty("LeftIndent", CharacterUnitLeftIndent);
	ParagraphFormat->setProperty("RightIndent", CharacterUnitRightIndent);
}

void CDocx::setHeaderFooter(QString strFontName, int fontSize, QString text)
{
	QAxObject *Section = m_saveDocument->querySubObject("Sections(int)", 1);

	QAxObject *Headers = Section->querySubObject("Headers(int)", 1);
	QAxObject *Footers = Section->querySubObject("Footers(int)", 1);

	//QAxObject *HeadersRange = Headers->querySubObject("Range");
	//HeadersRange->setProperty("Text", text);
	//QAxObject *ParagraphFormat = HeadersRange->querySubObject("ParagraphFormat");
	//ParagraphFormat->setProperty("Alignment", "wdAlignParagraphCenter");


	//QAxObject *HeadersFont = HeadersRange->querySubObject("Font");
	//HeadersFont->setProperty("Name", strFontName);
	//HeadersFont->setProperty("Size", fontSize);
	//HeadersFont->setProperty("Bold", "True");


	QAxObject *FootersRange = Footers->querySubObject("Range");
	//FootersRange->setProperty("Text", QString("第 X 页 共 Y 页"));
	QAxObject *FootersParagraphFormat = FootersRange->querySubObject("ParagraphFormat");
	FootersParagraphFormat->setProperty("Alignment", "wdAlignParagraphRight");


	QAxObject *FootersFont = FootersRange->querySubObject("Font");
	FootersFont->setProperty("Name", QString("宋体"));
	FootersFont->setProperty("Size", 12);

	int count = 1;
	QAxObject *ActiveWindow = m_saveDocument->querySubObject("ActiveWindow");
	QAxObject *ActivePane = ActiveWindow->querySubObject("ActivePane");
	QAxObject *Pages = ActivePane->querySubObject("Pages");
	do {
		QAxObject *Page = Pages->querySubObject("Item(int)", count);
		if (!Page)
		{
			count--;
			break;
		}
		count++;
	} while (true);

	// 获取文档总页数
	int pageCount = Pages->property("Count").toInt();
	qDebug() << "====count=" << count;
	qDebug() << "====pageCount=" << pageCount;
	if (pageCount > 0)
		count = pageCount;
	insertPageNumberV2(count);
}
