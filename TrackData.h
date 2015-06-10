#ifndef APOLLO_TRACK_DATA_H
#define APOLLO_TRACK_DATA_H

#include <vector>

//Base class for data --> AudioData, MidiData
template<typename UnitType>
class TrackData
{
//private:
//	void deleteData(std::true_type);
//	void deleteData(std::false_type);

protected:
	std::vector<UnitType> data;

public:
	TrackData(unsigned int num_units, UnitType default_value);
	virtual ~TrackData();

	const std::vector<UnitType>* getData() const;
	std::vector<UnitType>* getDataRef();

	//virtual UnitType& operator[](unsigned int i) = 0;
	//virtual const UnitType& operator[](unsigned int i) = 0;
};

template<typename UnitType>
TrackData::TrackData(unsigned int num_units, UnitType default_value)
	: data(num_units, default_value)
{ }

template<typename UnitType>
TrackData::~TrackData()
{ }

template<typename UnitType>
const std::vector<UnitType>* TrackData::getData() const
{
	return &data;
}

template<typename UnitType>
std::vector<UnitType>* TrackData::getDataRef()
{
	return &data;
}

/*
template<typename UnitType>
UnitType& TrackData::operator[](unsigned int i)
{
	return data[i];
}

template<typename UnitType>
const UnitType& TrackData::operator[](unsigned int i)
{
	return data[i];
}
*/

#endif	//APOLLO_TRACK_DATA_H