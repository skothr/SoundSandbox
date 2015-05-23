#ifndef APOLLO_DATA_STATUS_H
#define APOLLO_DATA_STATUS_H

#include <vector>

#include "Timing.h"

//Represents the status of some (track)data -- clean/dirty/(etc?)
enum class DataStatus
{
	INVALID = -1,
	CLEAN = 0,
	DIRTY
};

//Used to divide track data into sections with different statuses
struct DataStatusDivider
{
	c_time		start = 0;
	DataStatus	status = DataStatus::DIRTY;

	DataStatusDivider()
	{ }
	DataStatusDivider(c_time c_start, DataStatus stat)
		: start(c_start), status(stat)
	{ }

};

typedef std::vector<DataStatusDivider> StatusList;


#endif	//APOLLO_DATA_STATUS_H