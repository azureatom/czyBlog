/*
 * DRoundProgressBar - a circular progress bar Qt widget.
 *
 * Sintegrial Technologies (c) 2015-now
 *
 * The software is freeware and is distributed "as is" with the complete source codes.
 * Anybody is free to use it in any software projects, either commercial or non-commercial.
 * Please do not remove this copyright message and remain the name of the author unchanged.
 *
 * It is very appreciated if you produce some feedback to us case you are going to use
 * the software.
 *
 * Please send your questions, suggestions, and information about found issues to the
 *
 * sintegrial@gmail.com
 *
 */


#include "QRoundProgressBar.h"

#include <QtGui/QPainter>
#include <QPointF>

QRoundProgressBar::QRoundProgressBar(QWidget *parent) :
    QWidget(parent),
    m_min(0), m_max(100),
    m_value(25),
    m_innerOuterRate(0.75),
    m_nullPosition(PositionTop),
    m_barStyle(StyleDonut),
    m_outlinePenWidth(1),
    m_dataPenWidth(1),
    m_rebuildBrush(false),
    m_format("%p%"),
    m_decimals(1),
    m_updateFlags(UF_PERCENT)
{
}

void QRoundProgressBar::setRange(double min, double max)
{
    m_min = min;
    m_max = max;

    if (m_max < m_min)
        qSwap(m_max, m_min);

    if (m_value < m_min)
        m_value = m_min;
    else if (m_value > m_max)
        m_value = m_max;

    if (!m_gradientData.isEmpty())
        m_rebuildBrush = true;

    update();
}

void QRoundProgressBar::setMinimum(double min)
{
    setRange(min, m_max);
}

void QRoundProgressBar::setMaximum(double max)
{
    setRange(m_min, max);
}

void QRoundProgressBar::setValue(double val)
{
    if (m_value != val)
    {
        if (val < m_min)
            m_value = m_min;
        else if (val > m_max)
            m_value = m_max;
        else
            m_value = val;

        update();
    }
}

void QRoundProgressBar::setValue(int val)
{
    setValue(double(val));
}

void QRoundProgressBar::setNullPosition(double position)
{
    if (position != m_nullPosition)
    {
        m_nullPosition = position;

        if (!m_gradientData.isEmpty())
            m_rebuildBrush = true;

        update();
    }
}

void QRoundProgressBar::setBarStyle(QRoundProgressBar::BarStyle style)
{
    if (style != m_barStyle)
    {
        m_barStyle = style;

        update();
    }
}

void QRoundProgressBar::setOutlinePenWidth(double penWidth)
{
    if (penWidth != m_outlinePenWidth)
    {
        m_outlinePenWidth = penWidth;

        update();
    }
}

void QRoundProgressBar::setDataPenWidth(double penWidth)
{
    if (penWidth != m_dataPenWidth)
    {
        m_dataPenWidth = penWidth;

        update();
    }
}

void QRoundProgressBar::setDataColors(const QGradientStops &stopPoints)
{
    if (stopPoints != m_gradientData)
    {
        m_gradientData = stopPoints;
        m_rebuildBrush = true;

        update();
    }
}

void QRoundProgressBar::setFormat(const QString &format)
{
    if (format != m_format)
    {
        m_format = format;

        valueFormatChanged();
    }
}

void QRoundProgressBar::resetFormat()
{
    m_format = QString::null;

    valueFormatChanged();
}

void QRoundProgressBar::setDecimals(int count)
{
    if (count >= 0 && count != m_decimals)
    {
        m_decimals = count;

        valueFormatChanged();
    }
}

void QRoundProgressBar::paintEvent(QPaintEvent* /*event*/)
{
    double outerRadius = qMin(width(), height());
    QRectF baseRect(1, 1, outerRadius-2, outerRadius-2);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // data brush
    rebuildDataBrushIfNeeded();

    // background
    drawBackground(p, rect());
    double innerRadius(0);
    QRectF innerRect;
    calculateInnerRect(baseRect, outerRadius, innerRect, innerRadius);
    double arcStep = 360.0 / (m_max - m_min) * m_value;
    // base circle
    drawBase(p, baseRect,innerRect);

    // data circle

    drawValue(p, baseRect, m_value, arcStep,innerRect, innerRadius);

    // center circle

    drawInnerBackground(p, innerRect);

    // text
    drawText(p, innerRect, innerRadius, m_value);

    // finally draw the bar
    p.end();
}

void QRoundProgressBar::drawBackground(QPainter &p, const QRectF &baseRect)
{
    p.fillRect(baseRect, palette().background());
}

void QRoundProgressBar::drawBase(QPainter &p, const QRectF &baseRect,const QRectF &innerRect)
{
    switch (m_barStyle)
    {
    case StyleDonut:
    {
        QPainterPath dataPath;
        dataPath.setFillRule(Qt::OddEvenFill);
        dataPath.addEllipse(baseRect);
        dataPath.addEllipse(innerRect);
        p.setPen(QPen(palette().shadow().color(), m_outlinePenWidth));
        p.setBrush(palette().base());
        p.drawPath(dataPath);
        break;
    }
    case StylePie:
        p.setPen(QPen(palette().base().color(), m_outlinePenWidth));
        p.setBrush(palette().base());
        p.drawEllipse(baseRect);
        break;

    case StyleLine:
        p.setPen(QPen(palette().base().color(), m_outlinePenWidth));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(baseRect.adjusted(m_outlinePenWidth/2, m_outlinePenWidth/2, -m_outlinePenWidth/2, -m_outlinePenWidth/2));
        break;

    default:;
    }
}

void QRoundProgressBar::drawValue(QPainter &p
                                  , const QRectF &baseRect
                                  , double value
                                  , double arcLength
                                  , const QRectF & innerRect
                                  , double innerRadius)
{
    // nothing to draw
    if (value == m_min)
        return;

    switch(m_barStyle)
    {
    case StyleLine:
    {
        p.save();
        p.setPen(QPen(palette().highlight().color(), m_dataPenWidth));
        p.setBrush(Qt::NoBrush);
        p.drawArc(baseRect.adjusted(m_outlinePenWidth/2, m_outlinePenWidth/2, -m_outlinePenWidth/2, -m_outlinePenWidth/2),
                  m_nullPosition * 16,
                  -arcLength * 16);
        p.restore();
        break;
    }
    case StylePie:
    {
        QPainterPath dataPath;
        dataPath.setFillRule(Qt::WindingFill);
        dataPath.moveTo(baseRect.center());
        dataPath.arcTo(baseRect, m_nullPosition, -arcLength);
        dataPath.lineTo(baseRect.center());

        p.save();
        p.setPen(QPen(palette().shadow().color(), m_dataPenWidth));
        p.setBrush(palette().highlight());
        p.drawPath(dataPath);
        p.restore();
        break;
    }
    case StyleDonut:
    {
        const float circleWidth = (baseRect.width() - innerRect.width()) / 2;
        QRectF arcRect = baseRect;
        arcRect.adjust(circleWidth / 2, circleWidth / 2, -circleWidth / 2, -circleWidth / 2);

        p.save();
        p.setPen(QPen(palette().highlight(), circleWidth, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin));
        p.drawArc(arcRect, 16 * m_nullPosition, -16 * arcLength);
        p.restore();
        break;
    }
    default:
        break;
    }
}

void QRoundProgressBar::calculateInnerRect(const QRectF &/*baseRect*/, double outerRadius, QRectF &innerRect, double &innerRadius)
{
    // for Line style
    if (m_barStyle == StyleLine)
    {
        innerRadius = outerRadius - m_outlinePenWidth;
    }
    else    // for Pie and Donut styles
    {
        innerRadius = outerRadius * m_innerOuterRate;
    }

    double delta = (outerRadius - innerRadius) / 2;
    innerRect = QRectF(delta, delta, innerRadius, innerRadius);
}

void QRoundProgressBar::drawInnerBackground(QPainter &p, const QRectF &innerRect)
{
    if (m_barStyle == StyleDonut)
    {
        p.setBrush(palette().alternateBase());
        p.drawEllipse(innerRect);
    }
}

void QRoundProgressBar::drawText(QPainter &p, const QRectF &innerRect, double innerRadius, double value)
{
    if (m_format.isEmpty())
        return;

    QFont f(font());
    int minPixelSize = innerRadius * qMax(0.05, (0.35 - (double)m_decimals * 0.08));
    if(f.pixelSize() < minPixelSize)
    {
        //revise font size
        f.setPixelSize(minPixelSize);
    }
    p.setFont(f);

    QRectF textRect(innerRect.x(), innerRect.y() + innerRadius * 0.3, innerRadius, innerRadius * 0.3);
    p.setPen(palette().text().color());
    p.drawText(textRect, Qt::AlignCenter, valueToText(value));

    QRectF percentageTextRect(textRect);
    percentageTextRect.adjust(0, textRect.height(), 0, textRect.height());
    percentageTextRect.setHeight(innerRadius * 0.2);
    p.setPen(QColor(0x9C9F9E));
    f.setPixelSize(18);
    p.setFont(f);
    p.drawText(percentageTextRect, Qt::AlignCenter, "%");
}

QString QRoundProgressBar::valueToText(double value) const
{
    QString textToDraw(m_format);

    if (m_updateFlags & UF_VALUE)
        textToDraw.replace("%v", QString::number(value, 'f', m_decimals));

    if (m_updateFlags & UF_PERCENT)
    {
        double procent = (value - m_min) / (m_max - m_min) * 100.0;
        textToDraw.replace("%p", QString::number(procent, 'f', m_decimals));
    }

    if (m_updateFlags & UF_MAX)
        textToDraw.replace("%m", QString::number(m_max - m_min + 1, 'f', m_decimals));

    return textToDraw;
}

void QRoundProgressBar::valueFormatChanged()
{
    m_updateFlags = 0;

    if (m_format.contains("%v"))
        m_updateFlags |= UF_VALUE;

    if (m_format.contains("%p"))
        m_updateFlags |= UF_PERCENT;

    if (m_format.contains("%m"))
        m_updateFlags |= UF_MAX;

    update();
}

void QRoundProgressBar::rebuildDataBrushIfNeeded()
{
    if (m_rebuildBrush)
    {
        m_rebuildBrush = false;

        QConicalGradient dataBrush;
        dataBrush.setCenter(0.5,0.5);
        dataBrush.setCoordinateMode(QGradient::StretchToDeviceMode);

        // invert colors
        for (int i = 0; i < m_gradientData.count(); i++)
        {
            dataBrush.setColorAt(1.0 - m_gradientData.at(i).first, m_gradientData.at(i).second);
        }

        // angle
        dataBrush.setAngle(m_nullPosition);

        QPalette p(palette());
        p.setBrush(QPalette::Highlight, dataBrush);
        setPalette(p);
    }
}
float QRoundProgressBar::innerOuterRate() const
{
    return m_innerOuterRate;
}

void QRoundProgressBar::setInnerOuterRate(float innerOuterRate)
{
    m_innerOuterRate = innerOuterRate;
}




