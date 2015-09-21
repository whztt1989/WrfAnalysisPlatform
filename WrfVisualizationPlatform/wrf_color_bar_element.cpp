#include "wrf_color_bar_element.h"

WrfColorBarElement::WrfColorBarElement(ColorMappingType color_type, WrfColorBarType bar_type)
        : type_(bar_type), color_type_(color_type){
}

WrfColorBarElement::~WrfColorBarElement(){

}

void WrfColorBarElement::SetTitle(QString title){
    title_ = title;
}

void WrfColorBarElement::Render(int left, int right, int bottom, int top){
	ColorMappingGenerator::GetInstance()->GetColorIndex(color_type_, color_index_, is_linear_);

    if ( type_ != HORIZONTAL )
        RenderVerticalbar(left, right, bottom, top);
    else
        RenderHorizontalBar(left, right, bottom, top);
}

void WrfColorBarElement::Render(QPainter& painter, int left, int right, int bottom, int top){
	ColorMappingGenerator::GetInstance()->GetColorIndex(color_type_, color_index_, is_linear_);

    if ( type_ != HORIZONTAL ){
        int margin = 10 + left;
        float bar_height = (bottom - top) * 0.8;
        float bar_width = (right - left) * 0.3;
        float bar_bottom = bottom - (bottom - top) * 0.1;

        QFont normal_font;
        normal_font.setFamily("arial");
        normal_font.setBold(false);
        normal_font.setPixelSize(10);

        painter.setFont(normal_font);

        float bar_step =  bar_height / (color_index_.size() - 1);
        float temp_bar_pos = bar_bottom;
        for ( int i = 0; i < color_index_.size(); ++i ){
            if ( !is_linear_ || (is_linear_ && i == 0 || is_linear_ && i == color_index_.size() - 1) ){
                QString value_string = QString("%0").arg(color_index_[i].value_index);
                painter.drawText(QRectF(margin + bar_width + 3, temp_bar_pos - 15, 50, 30), Qt::AlignLeft | Qt::AlignVCenter, value_string);
            }
            temp_bar_pos -= bar_step;
        }

        normal_font.setBold(true);
        normal_font.setPixelSize(12);
        painter.setFont(normal_font);
        if ( title_.length() != 0 ){
            painter.drawText(QRectF(margin, bar_bottom + 10, 40, 60), Qt::AlignLeft | Qt::AlignTop, title_);
        }
    } else {

    }
}

void WrfColorBarElement::RenderHorizontalBar(int left, int right, int bottom, int top){

}

void WrfColorBarElement::RenderVerticalbar(int left, int right, int bottom, int top){
    int margin = 10 + left;
    float bar_height = (top - bottom) * 0.8;
    float bar_width = (right - left) * 0.3;
    if ( bar_width > 20 ) bar_width = 20;

    float bar_step =  bar_height / (color_index_.size() - 1);
    float bar_bottom = (top - bottom) * 0.1;
    float bar_top = (top - bottom) * 0.9;
    float temp_bar_bottom = bar_bottom + bottom;
    for ( int i = 0; i < color_index_.size() - 1; ++i ){
        if ( !is_linear_ ) {
            glColor3f(color_index_[i].color.redF(), color_index_[i].color.greenF(), color_index_[i].color.blueF());
            glBegin(GL_QUADS);
            glVertex3f(margin, temp_bar_bottom, 0);
            glVertex3f(margin, temp_bar_bottom + bar_step, 0);
            glVertex3f(margin + bar_width, temp_bar_bottom + bar_step, 0);
            glVertex3f(margin + bar_width, temp_bar_bottom, 0);
            glEnd();
        } else {
            glBegin(GL_QUADS);
            glColor3f(color_index_[i].color.redF(), color_index_[i].color.greenF(), color_index_[i].color.blueF());
            glVertex3f(margin, temp_bar_bottom, 0);
            glColor3f(color_index_[i + 1].color.redF(), color_index_[i + 1].color.greenF(), color_index_[i + 1].color.blueF());
            glVertex3f(margin, temp_bar_bottom + bar_step, 0);
            glColor3f(color_index_[i + 1].color.redF(), color_index_[i + 1].color.greenF(), color_index_[i + 1].color.blueF());
            glVertex3f(margin + bar_width, temp_bar_bottom + bar_step, 0);
            glColor3f(color_index_[i].color.redF(), color_index_[i].color.greenF(), color_index_[i].color.blueF());
            glVertex3f(margin + bar_width, temp_bar_bottom, 0);
            glEnd();
        }

        if ( !is_linear_ ){
            glColor3f(0.4, 0.4, 0.4);
            glLineWidth(1.0);
            glBegin(GL_LINE_LOOP);
            glVertex3f(margin, temp_bar_bottom, 0);
            glVertex3f(margin, temp_bar_bottom + bar_step, 0);
            glVertex3f(margin + bar_width, temp_bar_bottom + bar_step, 0);
            glVertex3f(margin + bar_width, temp_bar_bottom, 0);
            glEnd();
        }
        
        temp_bar_bottom += bar_step;
    }
}