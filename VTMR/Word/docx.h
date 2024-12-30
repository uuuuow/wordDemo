#ifndef DOCX_H
#define DOCX_H

#include <QAxObject>
#include <QAxWidget>
#include <QObject>
#include <QColor>

class CDocx : public QObject
{
	Q_OBJECT


public:
    enum TextAlign{AliginLeft = 0,AliginCenter = 1,AliginRight = 2, AliginLeftAndRight = 3};    //字体对齐方式
    enum TitleLevel{TitleNine = -10,TitleEight = -9,TitleSeven = -8,
                    TitleSix = -7,TitleFive = -6,TitleFour = -5,
                    TitleThree = -4,TitleTwo = -3,TitleOne = -2};       //标题分级
    enum TableFitBehavior{TableFitFixed = 0,TableFitContent = 1,
                         TableFitWindow = 2};                           //表格拟合方式
    enum LineStyle{LineStyleSingle = 1,LineStyleDouble = 7,
                   LineStyleDot = 2};                                   //线风格

    enum LineSpacing{LineSpace1pt5 = 1,LineSpaceAtLeast = 3,
                    LineSpaceDouble = 2,LineSpaceExactly = 4,
                    LineSpaceMultiple = 5,LineSpaceSingle = 0};//行距

public slots:
	void slots_exception(int a, QString b, QString c, QString d);

public:
    CDocx(QObject* parent);
    ~CDocx();

    /************功能相关************/
    bool openNewWord(const bool &show = bool(true));      //打开一个新文档
    void saveWord(const QString &path);                     //保存文档
    void quitWord();                                        //退出word

	void wdStyleTitle();
	void wdStyleSubtitle();
	void wdStyleHeading1();

    void newLine(int line = 1);                            //回车换行，默认为1行
    void changePage();                                      //换页
	void indent();											//正文首行缩进
	void Tab();												//缩进
    void setTitleText(const QString &title,
                      CDocx::TitleLevel level);             //设置标题及大小
    void setCenterTitleText(const QString &title);          //设置居中标题
    void iniParagraphText();                                //初始化段落文字格式为正文
    void setLineSpace(CDocx::LineSpacing space);            //设置行距

    /************页眉页脚************/
    void insertPageHead(const QString &text);               //插入页眉
	void insertPageNumber();                                //插入页码
	void insertPageNumberV2(int pages);                                //插入页码
    /************目录相关************/
    void insertCatalogue();                                 //插入目录
    void updateCatalogue();                                 //更新目录

    /************添加内容************/
    void addText(const QString &text);                      //添加一段文字
    void addNumber_E(const double &value);                   //以科学计数法方式添加一个数字
    void addNumber_Int(const int &num);                    //添加整数
    void addNumber_Float(const float &value);                //添加小数
	void addPic(const QString path, float width, float height);                       //添加图片

    /************字体相关************/
    void setFontStyle(const float &size = float(12),
                      const bool &bold = bool(false),
                      bool italic = false,
                      bool underLine = false);
    void setTextAlign(CDocx::TextAlign textAlign);
    void setTextColor(const QColor &color = QColor(0,0,0));
    void setFontName(const QString &name);

    /************表格相关************/
    void addTable(const int &row,
                  const int &col,
                  CDocx::TableFitBehavior autoFit = CDocx::TableFitFixed);//新增表格
    void setTableWidth(const int &tableIndex,
                       const float &width);
    void setTableColWidth(const int &tableIndex,
                          const int &col,
                          const float &width);             //设置表格列宽
    void setTableRowHeight(const int tableIndex,
                           const int &row,
                           const float &height);           //设置表格行高
    void setCellsBorderStyle(const int &tableIndex,
                             const int &startRow,
                             const int &startCol,
                             const int &endRow,
                             const int &endCol,
                             const CDocx::LineStyle &top,
                             const CDocx::LineStyle &bottom,
                             const CDocx::LineStyle &left,
                             const CDocx::LineStyle &right);//设置批量单元格风格
    void setCellBorderStyle(const int &tableIndex,
                            const int &row,
                            const int &col,
                            const CDocx::LineStyle &top,
                            const CDocx::LineStyle &bottom,
                            const CDocx::LineStyle &left,
                            const CDocx::LineStyle &right); //设置单个单元格风格
    void setCellsColor(const int &tableIndex,
                       const int &startRow,
                       const int &startCol,
                       const int &endRow,
                       const int &endCol,
                       const QColor &color);                //批量设置单元格颜色
    void setCellColor(const int &tableIndex,
                       const int &row,
                       const int &col,
                       const QColor &color);                //设置单个单元格颜色
    void selectTable(const int &tableIndex);               //选择一个表格
    void moveToTableEnd(const int &tableIndex);            //光标移动到表格下面
    void spanCells(const int &tableIndex,
                   const int &startRow,
                   const int &startCol,
                   const int &endRow,
                   const int &endCol);                     //合并单元格
    void setCellText(const int &tableIndex,
                     const int &row,
                     const int &col,
                     const QString &text);                  //设置单元格字符串内容
    void setCellTextAlign(const int &tableIndex,
                          const int &row,
                          const int &col,
                          CDocx::TextAlign align);          //设置单元格对齐方式
    void setCellTextColor(const int &tableIndex,
                          const int &row,
                          const int &col,
                          const QColor &color);             //设置单元格中字符串的颜色
    void setCellFont(const int &tableIndex,
                     const int &row,
                     const int &col,
                     QString fontName = QStringLiteral("仿宋"),
                     float fontSize = 9,
                     bool bold = false,
                     bool italic = false,
                     bool underLine = false);             //设置单元格字体
    void setTableFont(const int &tableIndex,
                      QString fontName = QStringLiteral("仿宋"),
                      float fontSize = 9,
                      bool bold = false,
                      bool italic = false,
                      bool underLine = false);            //设置表格字体
    void setTableTextAlign(const int &tableIndex,
                           CDocx::TextAlign aligin);        //设置表格内容对齐方式
    void setCellPicture(const int &tableIndex,
                        const int &row,
                        const int &col,
                        const QString &path);               //向单元格内插入图片
	void test(int num);
	void setTextStyle(int num);
	void setContext(const QString &context,
		const QString &fontName,
		const QColor &color = QColor(0,0,0),
		const float &size = float(12),
		const bool &bold = false,
		bool italic = false,
		bool underLine = false);
	void ApplyListTemplate(int index, int seq);
	void Indent();
	void ParagraphFormat(
		float CharacterUnitLeftIndent, //缩进，文本之前N字符
		float CharacterUnitRightIndent,//文本之后N字符
		float CharacterUnitFirstLineIndent, //悬挂缩进
		int Alignment //对齐方式
	);
	void setHeaderFooter(QString strFontName, int fontSize, QString text);


protected:
    void releaseDispatch(QAxObject *object);
    QAxObject* getTable(const int &tableIndex);
    void deleteObject();                                    //释放所有对象
private:
    QAxObject *m_wordApp = nullptr;
    QAxObject *m_selection = nullptr;
    QAxObject *m_font = nullptr;
    QAxObject *m_saveDocument = nullptr;

    QAxObject *m_windowActive = nullptr;
    QAxObject *m_viewActive = nullptr;
    QAxObject *m_pane = nullptr;
    QAxObject *m_tablesOfContents = nullptr;
public:
	int m_picCount = 0;
	int m_numFormatSeq1;
	int m_numFormatSeq2;
    QTimer *m_timer;
};

#endif // DOCX_H
