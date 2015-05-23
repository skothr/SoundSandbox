#ifndef APOLLO_NODE_CONNECTION_CONTROL_H
#define APOLLO_NODE_CONNECTION_CONTROL_H

#include "Control.h"

#include "Node.h"

class NodeGraphControl;
class NodeControl;

class NodeConnectionControl : public Control
{
protected:
	NodeGraphControl	*ngc_parent = nullptr;
	COwnedPtr			NC;
	
	NodeControl			*fromNodeControl = nullptr,	//No arrow side
						*toNodeControl = nullptr;	//Arrow side

	APoint				fromPoint,
						toPoint;

	float				sin_diff = 5.0f,						//Max height difference of sine wave
						sin_diff_inv = 1.0f/sin_diff,
						baseHeight = 3.0f,						//Base height of the lines
						lineSpacing = 3.0f,						//Spacing between the lines
						activeDist = baseHeight*(4.0f/3.0f),	//Distance from center line that control will respond to mouse
						disconnectDist = baseHeight*2.0f;		//Distance away from center line where dragging will disconnect a side

	float				amplitude	= 0.0f,
						aVel = 0.0f;
	double				f_trans_time = 0.0,
						b_trans_time = 0.0;

	Color				connectionColor;

	virtual void onDrag(APoint m_pos, AVec d_pos, bool direct) override;
	
	virtual bool respondToMouse(APoint m_pos) override;
	virtual bool respondToClick(APoint m_pos, MouseButton b) override;

	void adjustPos();
	float distanceToLine(APoint p);	//Returns the distance of the given point from the main connection line

public:
	NodeConnectionControl(NodeGraphControl *parent_, GuiStateFlags s_flags, COwnedPtr nc);
	virtual ~NodeConnectionControl();

	static const GuiPropFlags PROP_FLAGS;

	static const double		TRANSITION_TIME;
	static const float		ARROW_LENGTH,
							SIN_WAVELENGTH,
							SIN_VELOCITY,
							SIN_FREQUENCY,
							SIN_K,
							SIN_OMEGA;

	static const Color		INACTIVE_COLOR,				//Color of the connection with inactive
							FORWARD_ACTIVE_COLOR,		//Color of the connection when data is being PUSHED
							BACKWARD_ACTIVE_COLOR,		//Color of the connection when data is being PULLED
							ARROW_COLOR;				//Color of the connection arrow when active

	void setHangingNode(NCID other_id);
	void setHangingNode(NodeConnector *other_nc);

	void setHangingPos(APoint h_pos);

	bool fromConnected() const;
	bool toConnected() const;

	virtual void update(double dt) override;
	virtual void draw(GlInterface &gl) override;

	friend class NodeGraphControl;
};

#endif	//APOLLO_NODE_CONNECTION_CONTROL_H