#ifndef WRF_STAMP_VIEWER_H_
#define WRF_STAMP_VIEWER_H_

#include <QtGui/QGraphicsView>
#include <QtCore/QStateMachine>
#include <QtCore/QState>
#include <QtCore/QFinalState>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QSignalTransition>
#include <QtCore/QParallelAnimationGroup>
#include <QtCore/QTimer>
#include <vector>

class WrfStamp;

class WrfStampViewer : public QGraphicsView{
    Q_OBJECT
public:
    WrfStampViewer();
    ~WrfStampViewer();

    void SetTitle(QString& str) { title_ = str; }
    void SetStamps(std::vector< WrfStamp* >& stamps);
    void SetSelectedIndex(std::vector< int >& index);
    void Clear();

signals:
    void SelectionChanged();

protected:
    void resizeEvent(QResizeEvent *event);
	void drawBackground(QPainter* painter, const QRectF& rect);

private:
    std::vector< WrfStamp* > viewing_stamps_;

    std::vector< bool > is_selected_;

    QStateMachine* machine_;
    QState* current_state_;
    QParallelAnimationGroup* animation_group;
    QTimer* timer_;

    QState* idle_state_;
    QState* next_state_;

    QString title_;

    void UpdateStampPos();
};

#endif