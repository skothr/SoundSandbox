#include "GraphDisplay.h"

#include "Interpolation.h"

/////GRAPH DESC/////

/*
GraphDesc::GraphDesc(GPoint data_min, GPoint data_max, RVec absolute_to_graph, GVec standard_unit_size, GraphProps g_props)
	: dataMin(data_min), dataMax(data_max),
		absoluteToGraph(absolute_to_graph), standardUnitSize(standard_unit_size),
		props(g_props)
{ }
*/


/////GRAPH DISPLAY/////

/*
GraphDisplay::GraphDisplay(AVec min_data, AVec max_data)
	: Display(),
		minData(min_data), maxData(max_data), absoluteToGraphUnits(maxData - minData)
{
	if(absoluteToGraphUnits.x <= 0.0f)
		absoluteToGraphUnits.x = (maxData.x - minData.x)/size.x;
	if(absoluteToGraphUnits.y <= 0.0f)
		absoluteToGraphUnits.x = (maxData.x - minData.x)/size.x;

	graphToAbsoluteUnits = AVec(1.0f/absoluteToGraphUnits.x, 1.0f/absoluteToGraphUnits.y);
}
*/

GraphDisplay::GraphDisplay()
	: Display(),
		minData(), maxData(), absoluteToGraphUnits(), graphToAbsoluteUnits(),
		gProps(GraphProps::NONE), fixedWidth(false), fixedHeight(false), flipX(false), flipY(false),
		standardUnitSize()
{
	setBgColor(Color(0.75f, 0.75f, 0.75f, 1.0f));
}

GraphDisplay::GraphDisplay(GPoint data_min, GPoint data_max, RVec absolute_to_graph, GVec standard_unit_size, GraphProps g_props)
	: Display(),
		minData(data_min), maxData(data_max),
		absoluteToGraphUnits(absolute_to_graph), graphToAbsoluteUnits(1.0f/absolute_to_graph.x, 1.0f/absolute_to_graph.y),
		gProps(g_props), fixedWidth(g_props & GraphProps::FIXED_WIDTH), fixedHeight(g_props & GraphProps::FIXED_HEIGHT),
		flipX(g_props & GraphProps::FLIP_X), flipY(g_props & GraphProps::FLIP_Y),
		standardUnitSize(standard_unit_size)
{
	setBgColor(Color(0.75f, 0.75f, 0.75f, 1.0f));
	onSizeChanged(AVec());
}

GraphDisplay::~GraphDisplay()
{ }

void GraphDisplay::onSizeChanged(AVec d_size)
{
	if(fixedWidth)
	{
		size.x = (maxData.x - minData.x)*graphToAbsoluteUnits.x;
		resolveAttachments();
	}
	else
	{
		absoluteToGraphUnits.x = (maxData.x - minData.x)/size.x;
		graphToAbsoluteUnits.x = (float)(1.0/(double)absoluteToGraphUnits.x);
	}

	if(fixedHeight)
	{
		size.y = (maxData.y - minData.y)*graphToAbsoluteUnits.y;
		resolveAttachments();
	}
	else
	{
		absoluteToGraphUnits.y = (maxData.y - minData.y)/size.y;
		graphToAbsoluteUnits.y = (float)(1.0/(double)absoluteToGraphUnits.y);
	}
}


void GraphDisplay::setGraph(GPoint data_min, GPoint data_max, RVec absolute_to_graph, GVec standard_unit_size, GraphProps g_props)
{
	minData = data_min;
	maxData = data_max;
	absoluteToGraphUnits = absolute_to_graph;
	graphToAbsoluteUnits = RVec(1.0f/absoluteToGraphUnits.x, 1.0f/absoluteToGraphUnits.y);
	gProps = g_props;
	fixedWidth = gProps & GraphProps::FIXED_WIDTH;
	fixedHeight = gProps & GraphProps::FIXED_HEIGHT;
	flipX = gProps & GraphProps::FLIP_X;
	flipY = gProps & GraphProps::FLIP_Y;
	standardUnitSize = standard_unit_size;
	
	onSizeChanged(AVec());
}

GPoint GraphDisplay::getVisibleOrigin(GlInterface &gl) const
{
	//Get view origin in graph units
	APoint a_origin = gl.viewToAbsolutePoint(APoint());
	return absoluteToGraphPoint(a_origin);
}

GVec GraphDisplay::getVisibleSize(GlInterface &gl) const
{
	//Get view size in graph units
	AVec a_size = gl.viewToAbsoluteVec(gl.getCurrView().size);
	return absoluteToGraphVec(a_size, true);
}

GRect GraphDisplay::getVisibleRect(GlInterface &gl) const
{
	return GRect(getVisibleOrigin(gl), getVisibleSize(gl));
}

//#define GRAPH_TOP_OFFSET minData.y//(flipY ? 0.0f : minData.y)//(flipY ? (maxData.y - size.y*absoluteToGraphUnits.y) : minData.y)

//G --> A (POINT)
float GraphDisplay::graphToAbsoluteXP(float g_x) const
{
	float a_x = (g_x - minData.x)*graphToAbsoluteUnits.x;
	return (flipX ? size.x - a_x : a_x);
}

float GraphDisplay::graphToAbsoluteYP(float g_y) const
{
	float a_y = (g_y - minData.y)*graphToAbsoluteUnits.y;
	return (flipY ? a_y : size.y - a_y);
}

APoint GraphDisplay::graphToAbsolutePoint(GPoint g_point) const
{
	//Translate, then scale
	return APoint(graphToAbsoluteXP(g_point.x), graphToAbsoluteYP(g_point.y));
}


//G --> A (VEC)
float GraphDisplay::graphToAbsoluteXV(float g_x, bool is_size) const
{
	return g_x*graphToAbsoluteUnits.x*(flipX && !is_size ? -1.0f : 1.0f);
}

float GraphDisplay::graphToAbsoluteYV(float g_y, bool is_size) const
{
	return g_y*graphToAbsoluteUnits.y*(flipY || is_size ? 1.0f : -1.0f);
}

AVec GraphDisplay::graphToAbsoluteVec(GVec g_vec, bool is_size) const
{
	//Just scale
	return AVec(graphToAbsoluteXV(g_vec.x, is_size), graphToAbsoluteYV(g_vec.y, is_size));
}


//A --> G (POINT)
float GraphDisplay::absoluteToGraphXP(float a_x) const
{
	return (flipX ? size.x - a_x : a_x)*absoluteToGraphUnits.x + minData.x;
}

float GraphDisplay::absoluteToGraphYP(float a_y) const
{
	return (flipY ? a_y : size.y - a_y)*absoluteToGraphUnits.y + minData.y;
}

GPoint GraphDisplay::absoluteToGraphPoint(APoint a_point) const
{
	//Scale, then translate
	return GPoint(absoluteToGraphXP(a_point.x), absoluteToGraphYP(a_point.y));
}

//A --> G (VEC)
float GraphDisplay::absoluteToGraphXV(float a_x, bool is_size) const
{
	return a_x*absoluteToGraphUnits.x*(flipX && !is_size ? -1.0f : 1.0f);
}

float GraphDisplay::absoluteToGraphYV(float a_y, bool is_size) const
{
	return a_y*absoluteToGraphUnits.y*(flipY || is_size ? 1.0f : -1.0f);
}

GVec GraphDisplay::absoluteToGraphVec(AVec a_vec, bool is_size) const
{
	//Just scale
	return GVec(absoluteToGraphXV(a_vec.x, is_size), absoluteToGraphYV(a_vec.y, is_size));
}



void GraphDisplay::drawBackground(GlInterface &gl)
{
	Display::drawBackground(gl);

	//Draw standard scale as the background
	GPoint v_origin = getVisibleOrigin(gl);
	GVec v_size = getVisibleSize(gl);

	GPoint	prev_lines(	floor(v_origin.x/standardUnitSize.x)*standardUnitSize.x,
						floor(v_origin.y/standardUnitSize.y)*standardUnitSize.y);
	APoint	start_lines = graphToAbsolutePoint(prev_lines + standardUnitSize);
	AVec	line_step = graphToAbsoluteVec(standardUnitSize);

	//Vertical lines (x dimension) --> GREY
	gl.setColor(Color(0.4f, 0.4f, 0.4f, 1.0f));
	for(float a_x = start_lines.x; a_x < size.x; a_x += line_step.x)
		gl.drawLine(APoint(a_x, 0.0f), APoint(a_x, size.y));
	
	//Horizontal lines (y dimension) --> GREY
	gl.setColor(Color(0.4f, 0.4f, 0.4f, 1.0f));
	for(float a_y = start_lines.y; a_y < size.y; a_y += line_step.y)
		gl.drawLine(APoint(0.0f, a_y), APoint(size.x, a_y));
}

void GraphDisplay::draw(GlInterface &gl)
{
	if(isolateViewport(gl))
	{
		drawBackground(gl);
		drawGraph(gl);

		restoreViewport(gl);
	}
}