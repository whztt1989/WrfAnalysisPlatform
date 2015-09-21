#include "wrf_stamp_generator.h"
#include <fstream>
#include <iostream>
#include "qcolor_bar_controller.h"
#include "wrf_data_manager.h"

WrfStampGenerator* WrfStampGenerator::instance_ = 0;

WrfStampGenerator* WrfStampGenerator::GetInstance(){
	if ( instance_ == 0 ){
		instance_ = new WrfStampGenerator;
	}
	return instance_;
}

bool WrfStampGenerator::DeleteInstance(){
	if ( instance_ != 0 ){
		delete instance_;
		instance_ = 0;
		return true;
	}
	return false;
}

WrfStampGenerator::WrfStampGenerator(){
	LoadMapData();
}

WrfStampGenerator::~WrfStampGenerator(){

}

QPixmap* WrfStampGenerator::GenerateStamp(WrfGridValueMap* bias_map, int w, int h){
	bias_map_ = bias_map;
	WrfDataManager::GetInstance()->GetSiteAlpha(site_alpha_);

	if (glewInit() != GLEW_OK){
		std::cout << "Stamp generator glewInit error!" << std::endl;
		exit(0);
	}

	GLenum err = glGetError();

	start_longitude_ = bias_map->start_longitude;
	end_longitude_ = bias_map->end_longitude;
	start_latitude_ = bias_map->start_latitude;
	end_latitude_ = bias_map->end_latitude;

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();

	glGenRenderbuffers(1, &color_buffer_);
	glBindRenderbuffer(GL_RENDERBUFFER, color_buffer_);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, w, h);

	glGenRenderbuffers(1, &depth_buffer_);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, w, h);

	glGenFramebuffers(1, &frame_buffer_);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color_buffer_);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_);

	glGenBuffers(1, &VBO_);
	glGenBuffers(1, &EBO_);

	err = glGetError();

	glClearColor(0.623f, 0.737f, 0.890f, 1.0f);
	//glClearColor(1.0, 1.0, 1.0, 1.0);

	if ( abs(end_longitude_ - start_longitude_) / w >= abs(end_latitude_ - start_latitude_) / h ){
		float degree_per_pixel = abs(end_longitude_ - start_longitude_) / w;
		left_ = start_longitude_;
		right_ = end_longitude_;
		bottom_ = (end_latitude_ + start_latitude_) / 2 - degree_per_pixel * h / 2;
		top_ = (end_latitude_ + start_latitude_) / 2 + degree_per_pixel * h / 2;
	} else {
		float degree_per_pixel = abs(end_latitude_ - start_latitude_) / h;
		left_ = (end_longitude_ + start_longitude_) / 2 - degree_per_pixel * w / 2;
		right_ = (end_longitude_ + start_longitude_) / 2 + degree_per_pixel * w / 2;
		bottom_ = start_latitude_;
		top_ = end_latitude_;
	}

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(left_, right_, bottom_, top_, 1, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -2);

	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_SMOOTH);

	PaintBackgroundMap();

	PaintBiasMap();

	glDisable(GL_BLEND);

	err = glGetError();

	glBindFramebuffer(GL_READ_FRAMEBUFFER, frame_buffer_);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	temp_pixel_values_.resize(4 * w * h);
	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, temp_pixel_values_.data());

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDeleteBuffers(1, &VBO_);
	glDeleteBuffers(1, &EBO_);
	glDeleteRenderbuffers(1, &color_buffer_);
	glDeleteRenderbuffers(1, &depth_buffer_);
	glDeleteFramebuffers(1, &frame_buffer_);

	glPopMatrix();
	glPopAttrib();

	err = glGetError();
	std::vector< QRgb > pixel_values_;
	pixel_values_.resize(w * h);
	for ( int i = 0; i < h; ++i )
		for ( int j = 0; j < w; ++j ){
			int temp_index1 = i * w * 4 + j * 4;
			int temp_index2 = (h - i - 1) * w + j;

			pixel_values_[temp_index2] = qRgba(temp_pixel_values_[temp_index1], temp_pixel_values_[temp_index1 + 1], temp_pixel_values_[temp_index1 + 2], temp_pixel_values_[temp_index1 + 3]);
		}

	QImage generated_image((uchar*)pixel_values_.data(), w, h, w * 4, QImage::Format_ARGB32);
	QPixmap* map = new QPixmap();
	map->convertFromImage(generated_image);

	return map;
}

void WrfStampGenerator::PaintBackgroundMap(){
	glColor3f(0.0, 0.0, 0.0);

	for ( int i = 0; i < map_data_.size(); ++i ){
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, (float*)map_data_[i].data());
		glDrawArrays(GL_LINE_STRIP, 0, map_data_[i].size() / 2);
		glDisableClientState(GL_VERTEX_ARRAY);
	}
}

void WrfStampGenerator::PaintBiasMap(){
    std::vector< float > vertex_pos;
    vertex_pos.resize(bias_map_->latitude_grid_number * bias_map_->longitude_grid_number * 2);
    std::vector< float > vertex_color;
    vertex_color.resize(bias_map_->latitude_grid_number * bias_map_->longitude_grid_number * 4);
    std::vector< int > tri_index;
    tri_index.resize((bias_map_->longitude_grid_number - 1) * (bias_map_->latitude_grid_number - 1) * 6);

    float temp_latitude, temp_longitude;
    int vertex_index;
    temp_latitude = bias_map_->start_latitude;
    vertex_index = 0;
    for ( int i = 0; i < bias_map_->latitude_grid_number; ++i ){
        temp_longitude = bias_map_->start_longitude;
        for ( int j = 0; j < bias_map_->longitude_grid_number; ++j ){
            vertex_pos[vertex_index * 2] = temp_longitude;
            vertex_pos[vertex_index * 2 + 1] = temp_latitude;

            QColor color = QColorBarController::GetInstance(METEO_COLOR_MAP)->GetColor(bias_map_->min_value, bias_map_->max_value, bias_map_->values[vertex_index]);
            vertex_color[vertex_index * 4] = color.redF();
            vertex_color[vertex_index * 4 + 1] = color.greenF();
            vertex_color[vertex_index * 4 + 2] = color.blueF();
            vertex_color[vertex_index * 4 + 3] = site_alpha_[vertex_index] * 0.8;

            vertex_index++;
            temp_longitude += bias_map_->longitude_grid_space;

            if ( i == 0 || j == 0 ) continue;
            int left_bottom = (i - 1) * bias_map_->longitude_grid_number + j - 1;
            int left_top = i * bias_map_->longitude_grid_number + j - 1;
            int right_bottom = (i - 1) * bias_map_->longitude_grid_number + j;
            int right_top = i * bias_map_->longitude_grid_number + j;
            int temp_index = ((i - 1) * (bias_map_->longitude_grid_number - 1) + j - 1) * 6;
            tri_index[temp_index] = left_top;
            tri_index[temp_index + 1] = left_bottom;
            tri_index[temp_index + 2] = right_bottom;
            tri_index[temp_index + 3] = left_top;
            tri_index[temp_index + 4] = right_bottom;
            tri_index[temp_index + 5] = right_top;
        }
        temp_latitude += bias_map_->latitude_grid_space;
    }


    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, (float*)vertex_pos.data());
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_FLOAT, 0, (float*)vertex_color.data());
    glDrawElements(GL_TRIANGLES, tri_index.size(), GL_UNSIGNED_INT, (GLvoid*)tri_index.data());

	GLenum err = glGetError();

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void WrfStampGenerator::LoadMapData(){
	std::ifstream map_input1("./MapData/China_line1.dat");

	if ( map_input1.good() ){
		std::string header_info;
		getline(map_input1, header_info);

		int point_number;
		int para[6];
		while ( !map_input1.eof() ){
			map_input1 >> point_number >> para[0] >> para[1] >> para[2] >> para[3] >> para[4] >> para[5];
			std::vector< float > point_vec;
			point_vec.resize(point_number * 2);
			for ( int i = 0; i < point_number; ++i )
				map_input1 >> point_vec[2 * i] >> point_vec[2 * i + 1];
			map_data_.push_back(point_vec);
		}
		map_input1.close();
	} else {
		std::cout << "Load map data error!" << std::endl;
	}

	std::ifstream map_input("./MapData/new_map.dat");

	if ( map_input.good() ){
		std::string header_info;
		getline(map_input, header_info);
		std::string test;
		getline(map_input, test);

		int point_number;
		int para[8];
		while ( !map_input.eof() ){
			map_input >> point_number >> para[0] >> para[1] >> para[2] >> para[3] >> para[4] >> para[5] >> para[6];
			std::vector< float > point_vec;
			point_vec.resize(point_number * 2);
			for ( int i = 0; i < point_number; ++i ){
				map_input >> point_vec[2 * i] >> point_vec[2 * i + 1];
			}
			if ( para[0] > 2 ) continue;
			map_data_.push_back(point_vec);
		}
		map_input.close();
	} else {
		std::cout << "Load map data error!" << std::endl;
	}
}