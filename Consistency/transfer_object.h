#ifndef TRANSFER_OBJECT_H_
#define TRANSFER_OBJECT_H_

#include <QtCore/QRectF>

class TransferObject
{
public:
	enum TransferType{
		REGION_SHOW = 0,
		REGION_FADE,
		REGION_SEPARATE,
		REGION_MERGE,
		CENTER_CHANGE,
		SHAPE_CHANGE,
		SIZE_CHANGE,
		MIN_VALUE_CHANGE,
		MAX_VALUE_CHANGE
	};
	
	TransferObject(TransferType type, QRectF& region_one, QRectF& region_two, float evaluation_value);
	~TransferObject();
};

#endif