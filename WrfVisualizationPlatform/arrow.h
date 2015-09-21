#ifndef ARROW_H_
#define ARROW_H_

#include <QtGui/QGraphicsWidget>
#include <QtCore/QPoint>
#include <QtGui/QColor>

class Arrow : public QGraphicsWidget{
    Q_OBJECT
public:
    Arrow(const char* file_name, QPointF pos);
    ~Arrow();

    //QRectF boundingRect() const;
    //QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void SetExecuted();

    void setGeometry(const QRectF &rect);

signals:
    void AnimationTriggered();

private:
    QImage original_image_;
    QImage image_;
};

#endif