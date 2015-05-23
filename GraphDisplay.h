#ifndef APOLLO_GRAPH_DISPLAY_H
#define APOLLO_GRAPH_DISPLAY_H

#include "Display.h"
#include "Range.h"

#include "GraphData.h"

#include <vector>


typedef Rect GRect;

namespace GProps
{
	enum Props
	{
		NONE			= 0x00,
		
		FIXED_WIDTH		= 0x01,		//The graph's width scale is determined by its set width
		FIXED_HEIGHT	= 0x02,		//The graph's height scale is determined by its set height

		FLIP_X			= 0x04,		//Y-axis is flipped -- UP   => -y,	DOWN  => +y
		FLIP_Y			= 0x08,		//X-axis is flipped -- LEFT => +x,	RIGHT => -x
	};
};
typedef GProps::Props GraphProps;

/*
struct GraphDesc
{
	GPoint	dataMin = GPoint(0.0f, 0.0f),
			dataMax = GPoint(0.0f, 0.0f);
	RVec	absoluteToGraph = RVec(-1.0f, -1.0f);
	GVec	standardUnitSize = GVec(1.0f, 1.0f);

	GraphProps props = GraphProps::NONE;

	GraphDesc(GPoint data_min, GPoint data_max, RVec absolute_to_graph, GVec standard_unit_size, GraphProps g_props);
};
*/

//Used as a base class for displays that show graphical data.
//	- Easy conversion from graph coordinates to screen coordinates for drawing data
//	- P --> Point data type ; V --> Vec data type
class GraphDisplay : public Display
{
protected:
	//Uses element's set size to calculate conversion multiplier
	//GraphDisplay(AVec data_min, AVec data_max);

	//Creates invalid GraphDisplay --> need to call setGraph()
	GraphDisplay();
	//Creates already valid GraphDisplay
	GraphDisplay(GPoint data_min, GPoint data_max, RVec absolute_to_graph, GVec standard_unit_size, GraphProps g_props);

	GPoint		minData,
				maxData;
	GVec		standardUnitSize;	//Standard/base number of units in a graph-unit measurement
									//	-- Determines how the background scale is drawn

	GraphProps gProps;

	//Whether each dimension's multiplier is fixed -->
	//	- TRUE: Size is adjusted to fit more data (keeping the same multiplier)
	//	- FALSE: Multiplier is calculated from size
	bool		fixedWidth = false,
				fixedHeight = false;

	//Whether to flip the X/Y Axes
	//	Y:
	//	- Not flipped: Up = +y, Down = -y
	//	- Flipped: Up = -y, Down = +y
	//
	//	X:
	//	- Not flipped: Left = -x, Right = +x
	//	- Flipped: Left = +x, Right = -x
	bool		flipX = false,
				flipY = false;	

	RVec		absoluteToGraphUnits,	//Dimension multiplier --> Converts absolute units (relative to this element)
				graphToAbsoluteUnits;	//							into the graph units being used (or vice versa)
	
	virtual void onSizeChanged(AVec d_size) override;
	
	void setGraph(GPoint data_min, GPoint data_max, RVec absolute_to_graph, GVec standard_unit_size, GraphProps g_props);
	
	//Returns the origin/dimensions of the graph (in graph units) that is visible
	GPoint getVisibleOrigin(GlInterface &gl) const;
	GVec getVisibleSize(GlInterface &gl) const;
	GRect getVisibleRect(GlInterface &gl) const;

	float graphToAbsoluteXP(float g_x) const;
	float graphToAbsoluteYP(float g_y) const;
	APoint graphToAbsolutePoint(GPoint g_point) const;
	float graphToAbsoluteXV(float g_x, bool is_size = true) const;
	float graphToAbsoluteYV(float g_y, bool is_size = true) const;		//if is_size is false, flipY will flip the sign of the Y vector part.
	AVec graphToAbsoluteVec(GVec g_vec, bool is_size = true) const;

	float absoluteToGraphXP(float a_x) const;
	float absoluteToGraphYP(float a_y) const;
	GPoint absoluteToGraphPoint(APoint a_point) const;
	float absoluteToGraphXV(float a_x, bool is_size = true) const;
	float absoluteToGraphYV(float a_y, bool is_size = true) const;
	GVec absoluteToGraphVec(AVec a_vec, bool is_size = true) const;

	virtual void drawBackground(GlInterface &gl) override;

	//Should be overridden to return a pointer to the graph data that is currently visible within the provided 
	//virtual const GraphData* getData(GRect visible_area) = 0;

	virtual void drawGraph(GlInterface &gl) = 0;

public:
	virtual ~GraphDisplay();

	//virtual void drawSegment(GlInterface &gl, const GraphSegment &gs) = 0;

	//Draws each
	virtual void draw(GlInterface &gl) override;

};

/*
//Draws a Function, where each segment is contained within separate x values
template<typename DataType>
class LineGraphDisplay : public MultiLineGraphDisplay<DataType, 1>
{
protected:
	
	//Data values that will be drawn to the display.
	//	- The index represents the x value, and the stored value at that location represents the y value.
	LineData	*data;

	virtual const GraphData* getData(GRect visible_area);

public:
	LineGraphDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags);
	virtual ~LineGraphDisplay();
	
	static const GuiPropFlags PROP_FLAGS;

	//virtual void draw(GlInterface &gl) override;
};
*/

#endif	//APOLLO_GRAPH_DISPLAY_H