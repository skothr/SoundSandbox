#include "ModData.h"

/////MOD DATA/////

ModData::ModData()
{

}

ModData::~ModData()
{

}

std::vector<ModLevelPoint>* ModData::getData()
{
	return &data;
}

const std::vector<ModLevelPoint>* ModData::getConstData() const
{
	return &data;
}

void ModData::addPoint(const ModLevelPoint &new_point)
{
	for(unsigned int i = 0; i < data.size(); i++)
	{
		ModLevelPoint p = data[i];

		if(new_point.t <= p.t)
		{
			data.insert(data.begin() + i - 1, new_point);
			break;
		}
	}
}

ModLevelPoint* ModData::operator[](Time t)
{
	for(unsigned int i = 0; i < data.size(); i++)
	{
		if((i + 1 < data.size()) && (t <= data[i + 1].t))
		{
			return (t - data[i].t <= data[i + 1].t - t) ? &data[i] : &data[i + 1];
		}
	}

	return nullptr;
}

const ModLevelPoint* ModData::operator[](Time t) const
{
	return (*this)[t];
}