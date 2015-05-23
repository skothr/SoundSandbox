#include "WaveDisplay.h"

#include "Waveform.h"

/////WAVE DISPLAY/////
WaveData::WaveData(const FAudioSample *wave_data, int n_points)
	: GraphData<s_time, FAudioSample>(n_points), 
		waveData(wave_data)
{ }

WaveData::~WaveData()
{ }

void WaveData::update(double dt)
{

}

void WaveData::getData(FRange range, float x_step, std::vector<GPoint> &data_out) const
{
	Range<s_time> s_range((s_time)floor(range.start), (s_time)ceil(range.end));

	int num_data = (int)ceil((float)s_range.length()/x_step);

	data_out.clear();
	data_out.reserve(num_data);

	//if(x_step <= 1.0f)
	//{
		//Interpolate
		//s_time	prev_s = s_range.start,
		//		this_s = (s_time)ceil((float)prev_s + x_step);

		//float last_y = (float)waveData[s_range.start];

		//for(float x = range.start; x <= range.end; x += x_step)
		for(int p = 0; p < num_data; p++)
		{
			float x = range.start + (float)p*x_step;

			s_time	prev_s = (s_time)floor(x),
					next_s = prev_s + 1;

			float	prev_y = (float)waveData[min(prev_s, size - 1)],
					next_y = (float)waveData[min(next_s, size - 1)],
					alpha = x - (float)prev_s;

			data_out.push_back(GPoint(x, lerp(prev_y, next_y, alpha)));
		}
	//}
	/*
	else
	{
		//Estimate (TODO: More options)
		
		for(float s = range.start; s < range.end; s += x_step)
		{
			s_time	prev_s = (s_time)floor(s),
					next_s = (s_time)ceil(s);

			next_s = (prev_s == next_s ? next_s + 1 : next_s);

			float	prev_y = (float)waveData[prev_s],
					next_y = (float)waveData[next_s],
					alpha = s - prev_s;

			data_out.push_back(lerp(prev_y, next_y, alpha));
		}
	}
	*/
}

/*
void WaveData::drawData(Range<s_time> range, GlInterface &gl) const
{
	data_out.clear();
	data_out.reserve(range.length());

	for(s_time s = range.start; s < range.end; s++)
		data_out.push_back(waveData[s]);
}
*/

/////WAVE DISPLAY/////
const GuiPropFlags WaveDisplay::PROP_FLAGS = PFlags::HARD_BACK;

WaveDisplay::WaveDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, Waveform *waveform)
	: GuiElement(parent_, a_pos, a_size, GuiProps(s_flags, PROP_FLAGS)),
		LineGraphDisplay<s_time, FAudioSample>(&waveData,
					GPoint(0.0f, -1.0f), GPoint((float)Waveform::NUM_CACHE_POINTS, 1.0f),
					GVec(512.0f, 0.05f), GraphProps::NONE),
		waveData(WaveData(waveform->getCache(), Waveform::NUM_CACHE_POINTS))
{
	setBgColor(Color(0.75f, 0.75f, 0.75f, 1.0f));
}

WaveDisplay::~WaveDisplay()
{ }

void WaveDisplay::setData(const Waveform *waveform)
{
	waveData.waveData = waveform->getCache();
	waveData.size = Waveform::NUM_CACHE_POINTS;
}

const WaveData* WaveDisplay::getData() const
{
	return &waveData;
}

/*
void WaveDisplay::drawData(GlInterface &gl)
{
	//Only draw if waveform is valid
	if(wave)
	{
		//Get view origin/size in absolute space
		APoint av_pos = gl.viewToAbsolutePoint(APoint(0, 0));
		AVec av_size = gl.viewToAbsoluteVec(gl.getCurrView().size);

		//Cache points per absolute space pixel, and pixels per cache point
		float	cpp = (float)Waveform::NUM_CACHE_POINTS/size.x,
				ppc = 1.0f/cpp;

		//Left-most point, centered vertically (absolute space)
		APoint origin(0, size.y*(1.0f/2.0f));
	
		int start_point = (int)(av_pos.x*cpp),
			num_vis_points = (int)(av_size.x*cpp),
			end_point = start_point + num_vis_points;

		//Make sure end_point is within cache
		end_point = (end_point < Waveform::NUM_CACHE_POINTS) ? end_point :  (Waveform::NUM_CACHE_POINTS - 1);
	
		//Get cache points
		float *cache = wave->getCache();

		std::vector<TVertex> points;
		points.reserve(num_vis_points);

		for(int p = start_point; p < end_point; p++)
			points.push_back(TVertex(APoint((p - start_point)*ppc, origin.y - origin.y*(cache[p])), Color(1.0f, 0.0f, 1.0f, 1.0f)));
	
		//Draw cache
		gl.drawShape(GL_LINE_STRIP, points);

		restoreViewport(gl);
	}
}

void WaveDisplay::draw(GlInterface &gl)
{
	if(visible && isolateViewport(gl))
	{
		drawBackground(gl);
		drawData(gl);

		restoreViewport(gl);
	}
}
*/