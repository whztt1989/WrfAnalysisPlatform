#ifndef WRF_STAMP_GENERATOR_H_
#define WRF_STAMP_GENERATOR_H_

#include "gl/glew.h"
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include "wrf_data_stamp.h"

class WrfStampGenerator
{
public:
	static WrfStampGenerator* GetInstance();
	static bool DeleteInstance();

	QPixmap* GenerateStamp(WrfGridValueMap* bias_map, int w, int h);

protected:
	static WrfStampGenerator* instance_;

	WrfStampGenerator();
	~WrfStampGenerator();

private:
	std::vector< unsigned char > pixel_values_, temp_pixel_values_;

	std::vector< std::vector< float > > map_data_;
	std::vector< float > site_alpha_;

	float start_longitude_, end_longitude_, start_latitude_, end_latitude_;
	float left_, right_, bottom_, top_;

	WrfGridValueMap* bias_map_;

	GLuint frame_buffer_, color_buffer_, depth_buffer_;
	GLuint VBO_, EBO_;

	void LoadMapData();
	void PaintBiasMap();
	void PaintBackgroundMap();
};

#endif