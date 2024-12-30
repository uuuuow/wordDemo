#include "P0Trace.h"
#include <QLineSeries>
#include <QValueAxis>
#include "Utils.h"
#include <QGraphicsSimpleTextItem>
#include <QCategoryAxis>
#include <QScatterSeries>

class MultilineLabelItem : public QGraphicsTextItem {
public:
    MultilineLabelItem(const QString& text, QGraphicsItem* parent = nullptr) : QGraphicsTextItem(parent) {
        setTextInteractionFlags(Qt::NoTextInteraction);
        setZValue(1000);
        setTextWidth(100); // Set the width to wrap the text
        setPlainText(text); // Set the text for the QGraphicsTextItem
    }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override {
        if (change == ItemPositionChange && scene()) {
            QPointF newPos = value.toPointF();
            newPos.setY(newPos.y() - boundingRect().height());
            return newPos;
        }
        return QGraphicsItem::itemChange(change, value);
    }
};

CP0Trace::CP0Trace(QWidget *parent)
	: QChartView(parent)
{

    QChart* chart = this->chart();
    chart->legend()->hide();
    chart->setTitle("特征点变化");
}

CP0Trace::~CP0Trace()
{
}

void CP0Trace::setValue(QVector<QPointF> &points, QVector<int64_t> &ptss, int w, int h, bool scale/* = true*/)
{
    QChart* chart = this->chart();
    m_points = points;
    m_ptss = ptss;
    m_scale = scale;
    m_w = w;
    m_h = h;
    int rangeMinW = 0;
    int rangeMaxW = 0;
    int rangeMinH = 0;
    int rangeMaxH = 0;

    if (points.size() == 0)
        return;

    for (auto &axis : chart->axes()) {
        chart->removeAxis(axis);
    }
    chart->removeAllSeries();
    update();

    if (m_scale) {
        int minX = 0;
        int maxX = 0;
        int minY = 0;
        int maxY = 0;
        for (auto& p : points) {
            if (minX == 0)
                minX = p.x();
            else if (p.x() < minX)
                minX = p.x();

            if (maxX == 0)
                maxX = p.x();
            else if (p.x() > maxX)
                maxX = p.x();

            if (minY == 0)
                minY = p.y();
            else if (p.y() < minY)
                minY = p.y();

            if (maxY == 0)
                maxY = p.y();
            else if (p.y() > maxY)
                maxY = p.y();
        }

        rangeMinW = minX;
        rangeMaxW = maxX;
        if ((rangeMinW -= 20) < 0)
            rangeMinW = 20;

        if ((rangeMaxW += 20) > w)
            rangeMaxW = w;

        rangeMinH = minY;
        rangeMaxH = maxY;

        if ((rangeMinH -= 20) < 0)
            rangeMinH = 20;

        if ((rangeMaxH += 20) > h)
            rangeMaxH = h;
    }
    else {
        rangeMaxW = w;
        rangeMaxH = h;
    }
    

    QLineSeries* series = new QLineSeries();
    series->setPointsVisible(true);

    //QVector<QPointF> points = { QPointF(100, 200), QPointF(110, 210), QPointF(120, 220), QPointF(130, 230) };
    //QStringList titles = { "00:00:01.333", "00:00:02.333", "00:00:03.333", "00:00:03.333" };

    // Create custom QScatterSeries for displaying multiline point labels
    QScatterSeries* scatterSeries = new QScatterSeries();
    for (int i = 0; i < points.size(); ++i) {
        QString VTMillisecondsToTimeFormat(int milliseconds);
        QString labelText = QString("(%1, %2)\n%3").arg(points[i].x()).arg(points[i].y()).arg(VTMillisecondsToTimeFormat(ptss[i]));
        scatterSeries->append(points[i]);
        scatterSeries->setPointLabelsVisible(true);
        scatterSeries->setPointLabelsFormat(labelText);
    }

    chart->addSeries(scatterSeries);

    // Set up X and Y axis
    QValueAxis* xAxis = new QValueAxis();
    xAxis->setLabelFormat("%i");
    xAxis->setTitleText("X轴(像素)");
    chart->addAxis(xAxis, Qt::AlignBottom);
    scatterSeries->attachAxis(xAxis);

    QValueAxis* yAxis = new QValueAxis();
    yAxis->setLabelFormat("%i");
    yAxis->setTitleText("Y轴(像素)");
    chart->addAxis(yAxis, Qt::AlignLeft);
    scatterSeries->attachAxis(yAxis);

    // Customize Y-axis with a QCategoryAxis to display multiline labels
    QCategoryAxis* categoryAxis = new QCategoryAxis();
    categoryAxis->setStartValue(0);
    categoryAxis->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    categoryAxis->setLinePenColor(Qt::transparent);
    chart->addAxis(categoryAxis, Qt::AlignLeft);
    scatterSeries->attachAxis(categoryAxis);

    // Set custom labels for the Y-axis
    for (int i = 0; i < points.size(); ++i) {
        QString labelText = QString("(%1, %2)").arg(points[i].x()).arg(points[i].y());
        categoryAxis->append(labelText, i);
    }

    // Adjust the range of the axes to ensure all points are visible
    xAxis->setRange(rangeMinW, rangeMaxW);  // Adjust the X-axis range as needed
    yAxis->setRange(rangeMinH, rangeMaxH); // Adjust the Y-axis range as needed

    // Set background color to follow the parent window
    //chart->setBackgroundBrush(QBrush(Qt::white)); // Replace 'Qt::white' with your desired color

    // Set line color to red and point color to green
    //QPen pen(Qt::blue);  // Set the line color to red
    //series->setPen(pen);

    //QBrush pointBrush(Qt::green);  // Set the point color to green
    //series->setPointLabelsColor(pointBrush.color());
    update();
}

void CP0Trace::wheelEvent(QWheelEvent* ev)
{
    if (ev->angleDelta().y() > 0) {
        if (!m_scale)
            setValue(m_points, m_ptss, m_w, m_h, true);
    }
    else {
        if (m_scale)
            setValue(m_points, m_ptss, m_w, m_h, false);
    }
    ev->ignore();
}