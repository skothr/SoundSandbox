#ifndef APOLLO_GRAPH_DATA_H
#define APOLLO_GRAPH_DATA_H

#include "Range.h"
#include "Screen.h"
#include <vector>

class GlInterface;

//TYPEDEFS (To make units clearer)
typedef APoint GPoint;	//To denote graph units
typedef AVec GVec;

typedef Range<float> FRange;

template<typename XDataType, typename YDataType>
struct GraphData
{
	XDataType			size;

	GraphData(XDataType data_size);
	virtual ~GraphData();

	Range<XDataType> getXRange() const;
	//Range<YDataType> getYRange() const;

	//YDataType operator[](XDataType index) const;
	//YDataType operator[](Range<XDataType> range) const;
	
	//virtual YDataType getData(XDataType index) const = 0;
	//virtual void getData(Range<XDataType> range, YDataType *data_out) const = 0;

	virtual void update(double dt) = 0;

	virtual void getData(FRange x_range, float x_step, std::vector<GPoint> &out_data) const = 0;
	//virtual void drawData(Range<XDataType> x_range, GlInterface &gl) const = 0;
};


template<typename XDataType, typename YDataType>
GraphData<XDataType, YDataType>::GraphData(XDataType data_size)
	: size(data_size)
{ }

template<typename XDataType, typename YDataType>
GraphData<XDataType, YDataType>::~GraphData()
{ }


template<typename XDataType, typename YDataType>
Range<XDataType> GraphData<XDataType, YDataType>::getXRange() const
{
	return Range<XDataType>(0, size);
}


/*
template<typename XDataType, typename YDataType>
YDataType GraphData<XDataType, YDataType>::operator[](XDataType index) const
{
	return getData(index);
}
*/

/*
template<typename XDataType, typename YDataType>
void GraphData<XDataType, YDataType>::updateData(Range<XDataType> x_range)
{

}
*/

/*
template<typename XDataType, typename YDataType>
YDataType* GraphData<XDataType, YDataType>::getData(Range<XDataType> x_range, float x_step)
{
	updateData(x_range, x_step);
	return data;
}
*/

/*
struct FloatGraphData : public GraphData<float>
{
	FloatGraphData(const float *data_, unsigned int data_size);
	virtual ~FloatGraphData();
};
*/

#endif	//APOLLO_GRAPH_DATA_H