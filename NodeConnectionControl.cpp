#include "NodeConnectionControl.h"

#include "Node.h"
#include "NodeConnection.h"
#include "NodeGraphControl.h"
#include "NodeControl.h"

#include "Interpolation.h"

#include <GL/glew.h>

/////NODE CONNECTION CONTROL/////

const GuiPropFlags	NodeConnectionControl::PROP_FLAGS = PFlags::NONE;
const double		NodeConnectionControl::TRANSITION_TIME = 0.25f;
const float			NodeConnectionControl::ARROW_LENGTH = 9.0f,
					NodeConnectionControl::SIN_WAVELENGTH = 2.0f*M_PI,
					NodeConnectionControl::SIN_VELOCITY = -2.0f,
					NodeConnectionControl::SIN_FREQUENCY = SIN_VELOCITY/SIN_WAVELENGTH,
					NodeConnectionControl::SIN_K = 2.0f*M_PI/SIN_WAVELENGTH,
					NodeConnectionControl::SIN_OMEGA = 2.0f*M_PI*SIN_FREQUENCY;

const Color			NodeConnectionControl::INACTIVE_COLOR = Color(1.0f, 1.0f, 1.0f, 1.0f),				//Color of the connection with inactive
					NodeConnectionControl::FORWARD_ACTIVE_COLOR = Color(0.0f, 1.5f, 0.0f, 1.0f),		//Color of the connection when data is being PUSHED
					NodeConnectionControl::BACKWARD_ACTIVE_COLOR = Color(1.5f, 0.0f, 1.5f, 1.0f),		//Color of the connection when data is being PULLED
					NodeConnectionControl::ARROW_COLOR = Color(1.5f, 1.5f, 0.0f, 1.0f);				//Color of the connection arrow when active


NodeConnectionControl::NodeConnectionControl(NodeGraphControl *parent_, GuiStateFlags s_flags, NodeConnection *nc)
	: GuiElement(parent_, APoint(), AVec(), GuiProps(s_flags, PROP_FLAGS)),
		Control(GuiProps(s_flags, PROP_FLAGS)),
		ngc_parent(parent_), NC(nc)
		
		//fromNodeControl(from_nc), toNodeControl(to_nc),
		//fromNc(NodeConnector::getNodeConnector(from_id)), toNc(NodeConnector::getNodeConnector(to_id))
{
	setBgStateColor(Color(0.5f, 0.5f, 0.5f, 1.0f), CS::NONE);
	setBgStateColor(Color(0.9f, 0.9f, 0.9f, 1.0f), CS::HOVERING);
	setBgStateColor(Color(1.0f, 1.0f, 1.0f, 1.0f), CS::CLICKING);
	setBgStateColor(Color(1.0f, 1.0f, 1.0f, 1.0f), CS::DRAGGING);
	setBgStateColor(Color(1.0f, 1.0f, 1.0f, 1.0f), CS::DROP_HOVERING);
	//setAllBgStateColors(Color(1.0f, 0.0f, 1.0f, 1.0f));

	fromNodeControl = (NC->fromIsConnected() ? &ngc_parent->getNodeControl(NC->fromNode->getId()) : nullptr);
	toNodeControl = (NC->toIsConnected() ? &ngc_parent->getNodeControl(NC->toNode->getId()) : nullptr);

	fromPoint = (NC->fromIsConnected() ? fromNodeControl->getConnectorPoint(NC->fromNc->ioType) : APoint(0.0f, 0.0f));
	toPoint = (NC->toIsConnected() ? toNodeControl->getConnectorPoint(NC->toNc->ioType) : fromPoint);
	fromPoint = (NC->fromIsConnected() ? fromPoint : toPoint);

	adjustPos();
}

NodeConnectionControl::~NodeConnectionControl()
{

}

void NodeConnectionControl::onDrag(APoint m_pos, AVec d_pos, bool direct)
{
	APoint a_m_pos = m_pos + pos;//ngc_parent->transform.absoluteToVirtualPoint(m_pos);
	float dist = distanceToLine(m_pos);

	if(NC->isConnected() && direct && clickSelect)
	{
		if(dist > disconnectDist || !pointInside(a_m_pos))
		{
			AVec	d_p_from = fromPoint - a_m_pos,
					d_p_to = toPoint - a_m_pos;
			float	from_dist_2 = d_p_from.x*d_p_from.x + d_p_from.y*d_p_from.y,
					to_dist_2 = d_p_to.x*d_p_to.x + d_p_to.y*d_p_to.y;

			//bool d = NC->disconnectHalf((to_dist_2 < from_dist_2) ? NC->toId : NC->fromId);
			bool d = NC->disconnectFrom((to_dist_2 < from_dist_2) ? NC->toId : NC->fromId);

			if(d)
				std::cout << "SUCCESSFULLY DISCONNECTED NODES!\n";
			else
				std::cout << "FAILED TO DISCONNECT NODES!\n";

			ngc_parent->movingConnection = this;

			//std::cout << to_dist_2 << ", " << from_dist_2 << "\n";

			//if(to_dist_2 < from_dist_2)
			//{
			//	NC->toId = -1;
			//	NC->toNode = nullptr;
			//	NC->toNc = nullptr;
			//	toNodeControl = nullptr;
			//}
			//else
			//{
			//	NC->fromId = -1;
			//	NC->fromNode = nullptr;
			//	NC->fromNc = nullptr;
			//	fromNodeControl = nullptr;
			//}

			//ngc_parent->updateGraph();
		}
	}
	
	setHangingPos(a_m_pos);
}

bool NodeConnectionControl::respondToMouse(APoint m_pos)
{
	return (cState == CS::CLICKING || cState == CS::DRAGGING || ((distanceToLine(m_pos) <= activeDist)));
}

bool NodeConnectionControl::respondToClick(APoint m_pos, MouseButton b)
{
	return (cState == CS::CLICKING || cState == CS::DRAGGING || ((distanceToLine(m_pos) <= activeDist)));
}


void NodeConnectionControl::adjustPos()
{

	setRect(Rect(APoint(min(fromPoint.x, toPoint.x), min(fromPoint.y, toPoint.y)) - AVec(baseHeight, baseHeight),
				  AVec(abs(fromPoint.x - toPoint.x), abs(fromPoint.y - toPoint.y)) + AVec(baseHeight, baseHeight)*2.0f));
	/*
	setX((fromPoint.x > toPoint.x ? toPoint.x : fromPoint.x) - baseHeight);
	setWidth((fromPoint.x > toPoint.x ? (fromPoint.x - toPoint.x) : (toPoint.x - fromPoint.x)) + 2.0f*baseHeight);

	setY((fromPoint.y > toPoint.y ? toPoint.y : fromPoint.y) - baseHeight);
	setHeight((fromPoint.y > toPoint.y ? (fromPoint.y - toPoint.y) : (toPoint.y - fromPoint.y)));
	*/
}

float NodeConnectionControl::distanceToLine(APoint p)
{
	APoint	to_p = toPoint,//ngc_parent->transform.virtualToAbsolutePoint(toPoint),
			from_p = fromPoint;//ngc_parent->transform.virtualToAbsolutePoint(fromPoint);

	AVec d_pos = to_p - from_p;

	return NC->isConnected() ? abs(d_pos.y*p.x + to_p.x*from_p.y - (d_pos.x*p.y + from_p.x*to_p.y))/sqrt(d_pos.x*d_pos.x + d_pos.y*d_pos.y)
								: activeDist + 10000.0f;
}


void NodeConnectionControl::setHangingNode(NodeConnector *other_nc)
{
	setHangingNode(other_nc->getId());
}

void NodeConnectionControl::setHangingNode(NCID other_id)
{
	/*
	if(NC->toIsConnected() && !NC->fromIsConnected())
	{
		toNodeControl = ngc_parent->getNodeControl(NC->toNode);
		toPoint = toNodeControl->getConnectorPoint(NC->toNc->ioType);
	}
	else if(!NC->toIsConnected() && NC->fromIsConnected())
	{
		fromNodeControl = ngc_parent->getNodeControl(NC->fromNode);
		fromPoint = fromNodeControl->getConnectorPoint(NC->fromNc->ioType);
	}
	else
		std::cout << "ERROR: Node control set other node!\n";
		
	
	fromNodeControl = (NC->fromIsConnected() ? ngc_parent->getNodeControl(NC->fromNode) : nullptr);
	toNodeControl = (NC->toIsConnected() ? ngc_parent->getNodeControl(NC->toNode) : nullptr);
	*/

	if(NC->isHanging())	//If one is connected and the other isnt
	{
		NC->connectTo(other_id);
		
		fromNodeControl = (NC->fromIsConnected() ? &ngc_parent->getNodeControl(NC->fromNode->getId()) : nullptr);
		toNodeControl = (NC->toIsConnected() ? &ngc_parent->getNodeControl(NC->toNode->getId()) : nullptr);

		fromPoint = fromNodeControl->getConnectorPoint(NC->fromNc->ioType);
		toPoint = toNodeControl->getConnectorPoint(NC->toNc->ioType);
	}

	adjustPos();
}


void NodeConnectionControl::setHangingPos(APoint h_pos)
{
	if(!NC->toIsConnected() && NC->fromIsConnected())
		toPoint = h_pos;
	else if(NC->toIsConnected() && !NC->fromIsConnected())
		fromPoint = h_pos;

	adjustPos();
}

NodeConnector* NodeConnectionControl::getHangingNode()
{
	if(!NC->toIsConnected() && NC->fromIsConnected())
		return NC->fromNc;
	else if(NC->toIsConnected() && !NC->fromIsConnected())
		return NC->toNc;
}


bool NodeConnectionControl::fromConnected() const
{
	return NC->fromIsConnected();
}

bool NodeConnectionControl::toConnected() const
{
	return NC->toIsConnected();
}

void NodeConnectionControl::update(const Time &dt)
{
	Control::update(dt);

	if(NC->isConnected())
	{
		NCDir	active_dir = NC->getActiveDir();
		bool	forward = valid(active_dir & NCDir::FORWARD),
				backward = valid(active_dir & NCDir::BACKWARD);

		f_trans_time += (forward ? 1.0 : -1.0)*dt;
		f_trans_time = min(f_trans_time, TRANSITION_TIME);
		f_trans_time = max(f_trans_time, 0.0);
		
		b_trans_time += (backward ? dt : -dt);
		b_trans_time = min(b_trans_time, TRANSITION_TIME);
		b_trans_time = max(b_trans_time, 0.0);

		//Calculate base connection color
		connectionColor = (FORWARD_ACTIVE_COLOR*f_trans_time + BACKWARD_ACTIVE_COLOR*b_trans_time)*(1.0f/TRANSITION_TIME);
		connectionColor.a = 1.0f;
		
		//Calculate connection amplitude
		amplitude = smootherInterp(0.0f, 1.0f, max(f_trans_time, b_trans_time)*(1.0f/TRANSITION_TIME));
		
		pushing = NC->isPushing();
		pulling = NC->isPulling();

		//NC->resetConnectionStates();
	}

}

void NodeConnectionControl::draw(GlInterface &gl)
{
	if(visible)
	{
		fromPoint = (NC->fromIsConnected() && fromNodeControl ? fromNodeControl->getConnectorPoint(NC->fromNc->ioType) : fromPoint);
		toPoint = (NC->toIsConnected() && toNodeControl ? toNodeControl->getConnectorPoint(NC->toNc->ioType) : toPoint);

		APoint	f_p = fromPoint - pos,//ngc_parent->transform.absoluteToVirtualPoint(fromPoint),
				t_p = toPoint - pos;//ngc_parent->transform.absoluteToVirtualPoint(toPoint);

		adjustPos();

		AVec		diff = t_p - f_p;
		const float length = diff.length(),
					angle = atan2(diff.y, diff.x),
					cos_a = cos(angle),
					sin_a = sin(angle);

		APoint	start = fromPoint;
		AVec	step(cos_a, sin_a),
				perp(sin_a, -cos_a);

		step.normalize();
		perp.normalize();

		std::vector<TVertex> t_points;
		t_points.reserve((int)ceil((length + ARROW_LENGTH)*(2.0f/lineSpacing)));

		Color	color_mult = bgStateColors[cState],
				inactive_col(INACTIVE_COLOR.r*color_mult.r, INACTIVE_COLOR.g*color_mult.g, INACTIVE_COLOR.b*color_mult.b, INACTIVE_COLOR.a*color_mult.a),
				connection_col(connectionColor.r*color_mult.r, connectionColor.g*color_mult.g, connectionColor.b*color_mult.b, connectionColor.a*color_mult.a),
				arrow_col(ARROW_COLOR.r*color_mult.r, ARROW_COLOR.g*color_mult.g, ARROW_COLOR.b*color_mult.b, ARROW_COLOR.a*color_mult.a),

				col;


		//Add rotating line vertices
		for(float r = 0.0f; r < length - ARROW_LENGTH; r += lineSpacing)
		{
			APoint	p = start + step*r;
			float	w_t = SIN_OMEGA*Clock::getGlobalTime(),
					phi = -SIN_K*r,

					height = amplitude*sin(w_t + phi),
					offset = 0.0f,
					tilt = 2.0f*amplitude*cos(-w_t + phi);


			col = lerp(inactive_col, connection_col, -abs(height)*(1.0f + 2.0f*max(0.0f, 8.0f*amplitude*(cos(-w_t*2.0f + phi*0.01f) - 0.9f))));

			AVec	height_vec = perp*(baseHeight + sin_diff*height),
					tilt_vec = step*tilt,
					offset_vec = perp*offset;

			t_points.push_back(TVertex(p - height_vec - tilt_vec + offset_vec, col));
			t_points.push_back(TVertex(p + height_vec + tilt_vec + offset_vec, col));
		}

		gl.drawShape(GL_LINES, t_points);
		t_points.clear();
		t_points.reserve(3);
		
		//Draw pushing/pulling lines
		gl.setColor(Color(1.0f, 0.0f, 0.0f, 1.0f));
		APoint half = (fromPoint + toPoint)*0.5f;
		if(pushing)
			gl.drawLine(fromPoint, half);
		if(pulling)
			gl.drawLine(half, toPoint);

		t_points.push_back(TVertex(start + step*(length - ARROW_LENGTH) - perp*baseHeight*2.0f, col));
		t_points.push_back(TVertex(start + step*(length - ARROW_LENGTH) + perp*baseHeight*2.0f, col));
		t_points.push_back(TVertex(start + step*length, col));

		gl.drawShape(GL_TRIANGLES, t_points);
	}
}