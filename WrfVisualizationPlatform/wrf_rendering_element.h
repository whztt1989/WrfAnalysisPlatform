#ifndef WRF_RENDERING_ELEMENT_H_
#define WRF_RENDERING_ELEMENT_H_

#include "gl/glew.h"
#include <QtCore/QObject>
#include <QtGui/QPainter>

class WrfRenderingElement : public QObject
{
    Q_OBJECT

public:
    WrfRenderingElement() { is_visible_ = true; name_ = "Element"; };
    ~WrfRenderingElement() {}

	void SetVisible(bool is_visible) { is_visible_ = is_visible; }
	void SetName(std::string& name) { name_ = name; }
	const char* name() { return name_.c_str(); }
    virtual void Render(int left = 0, int right = 0, int bottom = 0, int top = 0) = 0;
    virtual void Render(QPainter& painter, int left = 0, int right = 0, int bottom = 0, int top = 0){}

protected:
	bool is_visible_;
	std::string name_;

signals:
    void ElementChanged();
};

#endif