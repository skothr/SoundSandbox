#ifndef APOLLO_MULTI_LINE_GRAPH_DISPLAY_H
#define APOLLO_MULTI_LINE_GRAPH_DISPLAY_H

#include "GraphDisplay.h"
#include "Interpolation.h"

#include <vector>
#include <algorithm>

#include <GL/glew.h>

/*
template<typename DataType>
struct LineGraphDesc : public GraphDesc
{
	GraphData<DataType> data;

	LineGraphDesc(const GraphData<DataType> &graph_data, GPoint data_min, GPoint data_max, GVec standard_unit_size, GraphProps g_props);
};

template<typename DataType>
LineGraphDesc::LineGraphDesc(const GraphData<DataType> &graph_data, GPoint data_min, GPoint data_max, GVec standard_unit_size, GraphProps g_props)
	: GraphDesc(data_min, data_max, RVec(), standard_unit_size, g_props),
		data(graph_data)
{ }
*/

//Uses 
template<typename XDataType, typename YDataType>
class LineGraphDisplay : public GraphDisplay
{
protected:
	//Gets min/max and graph scaling factor from provided data
	LineGraphDisplay(const GraphData<XDataType, YDataType> *g_data, GVec standard_unit_size, GraphProps g_props);
	//Just gets graph scaling factor from provided data
	LineGraphDisplay(const GraphData<XDataType, YDataType> *g_data, GPoint data_min, GPoint data_max, GVec standard_unit_size, GraphProps g_props);
	//All necessary information provided
	LineGraphDisplay(const GraphData<XDataType, YDataType> *g_data, GPoint data_min, GPoint data_max, RVec absolute_to_graph, GVec standard_unit_size, GraphProps g_props);

	//Pointer to data(array of DataType) for each line (element of vector)
	//std::vector<DataType*> lineData;
	const GraphData<XDataType, YDataType>	*gData;
	GVec									offset;

	void setData(const GraphData<XDataType, YDataType> *g_data);

	virtual void onSizeChanged(AVec d_size) override;

	//Calculate data indices for the given graph values
	unsigned int graphToIndexXP(XDataType g_x);

	virtual void drawGraph(GlInterface &gl) override;

public:
	//LineGraphDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, const LineGraphDesc &desc);
	virtual ~LineGraphDisplay();
	
	//Converts all points to floats before calculating
	//virtual void draw(GlInterface &gl) override;

};


template<typename XDataType, typename YDataType>
LineGraphDisplay<XDataType, YDataType>::LineGraphDisplay(const GraphData<XDataType, YDataType> *g_data, GVec standard_unit_size, GraphProps g_props)
	: GraphDisplay(),
		gData(g_data)
{
	if(gData)
	{
		std::vector<GPoint> data;
		FRange x_range((float)gData->getXRange().start, (float)gData->getXRange().end);

		gData->getData(x_range, 1.0f, data);

		//X range goes from 0 to g_data.size, just need Y range
		//YDataType	min_y = *std::min_element(data.begin(), data.end()),
		//			max_y = *std::max_element(data.begin(), data.end());

		YDataType	min_y = INFINITY,
					max_y = -INFINITY;
		for(auto p : data)
		{
			min_y = std::min(min_y, (YDataType)p.y);
			max_y = std::max(max_y, (YDataType)p.y);
		}

		RVec abs_to_graph(((float)gData->size)/size.x, (float)(max_y - min_y)/size.y);

		setGraph(GPoint(0.0f, (float)min_y), GPoint((float)gData->size, (float)max_y), abs_to_graph, standard_unit_size, g_props);
	}
	else
		setGraph(GPoint(0.0f, 0.0f), GPoint(0.0f, 0.0f), 1.0f, standard_unit_size, g_props);
}
	
template<typename XDataType, typename YDataType>
LineGraphDisplay<XDataType, YDataType>::LineGraphDisplay(const GraphData<XDataType, YDataType> *g_data, GPoint data_min, GPoint data_max, GVec standard_unit_size, GraphProps g_props)
	: GraphDisplay(),
		gData(g_data)
{
	RVec abs_to_graph((data_max.x - data_min.x)/size.x, (data_max.y - data_min.y)/size.y);
	setGraph(data_min, data_max, abs_to_graph, standard_unit_size, g_props);
}

template<typename XDataType, typename YDataType>
LineGraphDisplay<XDataType, YDataType>::LineGraphDisplay(const GraphData<XDataType, YDataType> *g_data, GPoint data_min, GPoint data_max, RVec absolute_to_graph, GVec standard_unit_size, GraphProps g_props)
	: GraphDisplay(data_min, data_max, absolute_to_graph, standard_unit_size, g_props),
		gData(g_data)
{

}

/*
template <typename DataType>
LineGraphDisplay<DataType>::LineGraphDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, const LineGraphDesc &desc)
	: GuiElement(parent_, a_pos, a_size, GuiProps(s_flags, PROP_FLAGS)),
		GraphDisplay(desc),
		data(desc.data)
{
	if(fixedX
}
*/

template<typename XDataType, typename YDataType>
LineGraphDisplay<XDataType, YDataType>::~LineGraphDisplay()
{

}

template<typename XDataType, typename YDataType>
void LineGraphDisplay<XDataType, YDataType>::onSizeChanged(AVec d_size)
{
	GraphDisplay::onSizeChanged(d_size);
}

template<typename XDataType, typename YDataType>
void LineGraphDisplay<XDataType, YDataType>::setData(const GraphData<XDataType, YDataType> *g_data)
{
	gData = g_data;
}

template<typename XDataType, typename YDataType>
unsigned int LineGraphDisplay<XDataType, YDataType>::graphToIndexXP(XDataType g_x)
{
	return (unsigned int)g_x;//(unsigned int)(g_x - minData.x);
}

template<typename XDataType, typename YDataType>
void LineGraphDisplay<XDataType, YDataType>::drawGraph(GlInterface &gl)
{
	GPoint	v_origin = getVisibleOrigin(gl);
	GVec	v_size = getVisibleSize(gl);
	GlVec	gl_size = gl.getCurrView().size;

	std::vector<GPoint> p_data;
	FRange x_range = flipX ? FRange(maxData.x - v_origin.x - v_size.x, maxData.x - v_origin.x)
							: FRange(v_origin.x, v_origin.x + v_size.x);
	float x_step = x_range.length()/gl_size.x;

	if(gData)
		gData->getData(x_range, x_step, p_data);
	else
		return;

	if(p_data.size() == 0)
		return;
		
	gl.setColor(Color(0.0f, 0.0f, 1.0f, 1.0f));

	std::vector<TVertex> vertices;
	vertices.reserve(p_data.size());
	//vertices.push_back(TVertex(prev_point, Color(0.0f, 0.0f, 1.0f, 1.0f)));

	//float ax = graphToAbsoluteXP(v_origin.x);

	for(auto gp : p_data)
	{
		//float ay = graphToAbsoluteYP(gy);
		vertices.push_back(TVertex(graphToAbsolutePoint(gp), Color(0.0f, 0.0f, 1.0f, 1.0f)));

		//ax += 1.0f;
	}

	/*
	for(XDataType g_x = first_g_x; g_x <= max_g_x; g_x++)
	{
		//(x is in graph units)
		XDataType	low_g_x = (XDataType)floor(g_x),
					high_g_x = (XDataType)ceil(g_x);

		//Fix in case 
		high_g_x += (low_g_x == high_g_x ? 1 : 0);
		
		if(high_g_x - first_g_x >= y_data.size())
			break;

		GPoint	low_point((float)low_g_x, (float)y_data[low_g_x - first_g_x]),
				high_point((float)high_g_x, (float)y_data[high_g_x - first_g_x]);

		APoint this_point = graphToAbsolutePoint(APoint((float)g_x, (float)lerp(g_x, low_point, high_point)));
		//gl.drawLine(prev_point, this_point);
		vertices.push_back(TVertex(this_point, Color(0.0f, 0.0f, 1.0f, 1.0f)));
		prev_point = this_point;
	}
	*/
	gl.drawShape(GL_LINE_STRIP, vertices);

	gl.drawLine(graphToAbsolutePoint(GPoint((float)gData->size/2.0f, (float)maxData.y)), graphToAbsolutePoint(GPoint((float)gData->size/2.0f, (float)minData.y)));
}


#endif	//APOLLO_MULTI_LINE_GRAPH_DISPLAY_H