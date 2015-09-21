#ifndef STEP_ITEM_H_
#define STEP_ITEM_H_

#include <QtCore/QObject>
#include <QtGui/QGraphicsItem>

enum StepItemType{
    DATA_VIEW = 0x0,
    PRE_PROCESSING,
    UNCERTAINTY,
    RMS,
    ADJUSTMENT,
    EVENT_RESULT,
    RMS_RESULT,
    COMPARISON
};

class StepItem : public QObject, public QGraphicsItem{
    Q_OBJECT

public:
    StepItem(StepItemType type, QString icon_path, QString step_name);
    ~StepItem();

    void SetExecuted(bool is_executed);

    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

signals:
    void ItemSelected(StepItemType type);
    void ItemDoubleClicked(StepItemType type);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

private:
    StepItemType type_;
    QString icon_path_;
    QString step_name_;

    bool is_executed_;

    QImage image_;
};

#endif