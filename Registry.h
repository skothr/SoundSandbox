#ifndef APOLLO_REGISTRY_H
#define APOLLO_REGISTRY_H

#include <unordered_map>

typedef int RegId;	//Id type for a registry. <0 is invalid.

template<typename T>
class Registry
{
protected:
	std::unordered_map<RegId, T*> regObjs;

	RegId nextId = 0;

public:
	Registry();
	virtual ~Registry();

	RegId registerId(T *obj);		//Returns registered Id

	bool deregisterId(RegId id);	//Deregisters and returns whether given Id is registered
	bool deregisterId(T *obj);		//Deregisters and returns whetehr the given object is registered

	bool validId(RegId id);			//Returns whether the given Id is registered
	bool validId(T *obj);			//Returns whether the given object is registered

	RegId getId(T *obj);			//Returns the id of the given object
	T* getObject(RegId id);			//Returns the object registered to the given id
};

template<typename T>
Registry<T>::Registry()
	: regObjs{}
{ }

template<typename T>
Registry<T>::~Registry()
{
	regObjs.clear();
}

template<typename T>
RegId Registry<T>::registerId(T *obj)
{
	if(!validId(obj))
	{
		regObjs.emplace(nextId, obj);
		return nextId++;
	}
	else
		return -1;
}

template<typename T>
bool Registry<T>::deregisterId(RegId id)
{
	if(validId(id))
	{
		regObjs.erase(id);
		return true;
	}
	else
		return false;
}

template<typename T>
bool Registry<T>::deregisterId(T *obj)
{
	if(validId(obj))
	{
		regObjs.erase(getId(obj));
		return true;
	}
	else
		return false;
}

template<typename T>
bool Registry<T>::validId(RegId id)
{
	return (getObject(id) != nullptr);
}

template<typename T>
bool Registry<T>::validId(T *obj)
{
	return (getId(obj) != -1);
}

template<typename T>
RegId Registry<T>::getId(T *obj)
{
	for(auto o : regObjs)
	{
		if(o.second == obj)
			return o.first;
	}

	return -1;
}


template<typename T>
T* Registry<T>::getObject(RegId id)
{
	auto iter = regObjs.find(id);

	return (iter == regObjs.end() ? nullptr : iter->second);
}

#endif	//APOLLO_REGISTRY_H