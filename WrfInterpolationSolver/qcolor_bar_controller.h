#ifndef QCOLOR_BAR_CONTROLLER_H_
#define	QCOLOR_BAR_CONTROLLER_H_

#include <QtGui/QColor>

enum ColorBarType{
	HEAT_MAP = 0,
	METEO_COLOR_MAP,
    GRAY_MAP
};

typedef struct{
	float value_index;
	QColor color;
} ColorIndex;

class QColorBarController
{
public:
	static QColorBarController* GetInstance(ColorBarType bar_type);
	static bool DeleteInstance();
	
	QColor GetColor(float min_value, float max_value, float current_value);
	void SetBarType(ColorBarType bar_type);
	const std::list< ColorIndex >* GetColorList() { return &color_index_list_; }

protected:
	QColorBarController();
	~QColorBarController();

private:
	static QColorBarController* controller_;

	std::list< ColorIndex > color_index_list_;
	ColorBarType color_bar_type_;

	void ConstructColorList();
};

#endif