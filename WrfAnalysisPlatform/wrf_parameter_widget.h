#ifndef WRF_PARAMETER_WIDGET_H_
#define WRF_PARAMETER_WIDGET_H_

#include <QtGui/QWidget>
#include <QtGui/QLabel>

class WrfParameterWidget : public QWidget
{
	Q_OBJECT

public:
	WrfParameterWidget();
	~WrfParameterWidget();

	public slots:
		void OnWindVarianceThreshChanged(int);
		void OnHistoricalWeightChanged(int);
		void OnCurrentWeightChanged(int);
		void OnHistoryLengthChanged(int);

signals:
	void WindVarThresholdChanged();
	void BiasWeightChanged();

signals:
	void StartLongitudeChanged(QString);
	void EndLongitudeChanged(QString);
	void StartLatitudeChanged(QString);
	void EndLatitudeChanged(QString);
	void PatchSizeChanged(QString);
	void HistoryLengthChanged(int);

private:
	QLabel* his_length_view_label;

	void InitWidget();
		
};

#endif