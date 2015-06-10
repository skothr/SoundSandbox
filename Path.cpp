#include "Path.h"

#include "FileSystem.h"

/////PATH/////

Path::Path()
	: path(L"")
{ }
Path::Path(std::wstring p)
	: path(FileSystem::toInternalPath(p))
{ }
Path::Path(std::string p)
	: Path(FileSystem::toWString(p))
{ }
/*
Path::Path(const char* &p)
	: Path(std::string(p))
{ }
Path::Path(const wchar_t* &p)
	: Path(std::wstring(p))
{ }
*/
Path::Path(const Path &p)
	: path(p.path)
{ }

Path& Path::operator=(const Path &p)
{
	path = p.path;
	return *this;
}
Path& Path::operator=(std::wstring p)
{
	path = FileSystem::toInternalPath(p);
	return *this;
}
Path& Path::operator=(std::string p)
{ return (*this = FileSystem::toWString(p)); }
/*
Path& Path::operator=(const char* &p)
{ return (*this = std::string(p)); }
Path& Path::operator=(const wchar_t* &p)
{ return (*this = std::wstring(p)); }
*/

Path& Path::operator+=(const Path &rhs)
{
	path += rhs.path;
	return *this;
}
Path& Path::operator+=(std::wstring rhs)
{
	path += FileSystem::toInternalPath(rhs);
	return *this;
}
Path& Path::operator+=(std::string rhs)
{ return (*this += FileSystem::toWString(rhs)); }
/*
Path& Path::operator+=(const char* &rhs)
{ return (*this += std::string(rhs)); }
Path& Path::operator+=(const wchar_t* &rhs)
{ return (*this += std::wstring(rhs)); }
*/
	
Path Path::operator+(const Path &rhs) const
{ return (Path(*this) += rhs); }
Path Path::operator+(std::wstring rhs) const
{ return (Path(*this) += rhs); }
Path Path::operator+(std::string rhs) const
{ return (Path(*this) += rhs); }
/*
Path Path::operator+(const char* &rhs) const
{ return (Path(*this) += rhs); }
Path Path::operator+(const wchar_t* &rhs) const
{ return (Path(*this) += rhs); }
*/
bool Path::operator==(const Path &other) const
{
	return (path == other.path);
}
bool Path::operator==(std::wstring rhs) const
{
	return (path == rhs);
}
bool Path::operator==(std::string rhs) const
{ return (*this == FileSystem::toWString(rhs)); }
/*
bool Path::operator==(const char* &rhs) const
{ return (*this == std::string(rhs)); }
bool Path::operator==(const wchar_t* &rhs) const
{ return (*this == std::wstring(rhs)); }
*/

bool Path::operator!=(const Path &other) const
{ return !(*this == other); }
bool Path::operator!=(std::wstring rhs) const
{ return !(*this == rhs); }
bool Path::operator!=(std::string rhs) const
{ return !(*this == rhs); }
/*
bool Path::operator!=(const char* &rhs) const
{ return !(*this == rhs); }
bool Path::operator!=(const wchar_t* &rhs) const
{ return !(*this == rhs); }
*/

std::ostream& operator<<(std::ostream &os, const Path &p)
{
	return (os << FileSystem::toString(p.path));
}

std::wstring Path::getInternal() const
{
	return path;
}
std::wstring Path::getSystem() const
{
	return FileSystem::toSystemPath(path);
}
