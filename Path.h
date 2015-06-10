#ifndef APOLLO_PATH_H
#define APOLLO_PATH_H

#include <string>

class Path
{
	friend class FileSystem;

private:
	//The path this class represents (soted as internal format)
	std::wstring path;

public:
	Path();
	Path(std::wstring p);
	Path(std::string p);
	//Path(const std::wstring &p);
	//Path(const std::string &p);
	//Path(const char* &p);
	//Path(const wchar_t* &p);
	//Path(const char[] p);
	Path(const Path &p);

	//TODO: Task in const references for each possible type of string, instead of using string values

	Path& operator=(const Path &other);
	Path& operator=(std::wstring p);
	Path& operator=(std::string p);
	//Path& operator=(const char* &p);
	//Path& operator=(const wchar_t* &p);

	Path& operator+=(const Path &rhs);
	Path& operator+=(std::wstring rhs);
	Path& operator+=(std::string rhs);
	//Path& operator+=(const char* &p);
	//Path& operator+=(const wchar_t* &p);
	
	Path operator+(const Path &rhs) const;
	Path operator+(std::wstring rhs) const;
	Path operator+(std::string rhs) const;
	//Path operator+(const char* &p) const;
	//Path operator+(const wchar_t* &p) const;

	bool operator==(const Path &other) const;
	bool operator==(std::wstring rhs) const;
	bool operator==(std::string rhs) const;
	//bool operator==(const char* &rhs) const;
	//bool operator==(const wchar_t* &rhs) const;

	bool operator!=(const Path &other) const;
	bool operator!=(std::wstring rhs) const;
	bool operator!=(std::string rhs) const;
	//bool operator!=(const char* &rhs) const;
	//bool operator!=(const wchar_t* &rhs) const;
	
	friend std::ostream& operator<<(std::ostream &os, const Path &p);

	//Conversion to wstring --> returns system path
	//operator std::wstring() const;
	//explicit operator std::wstring() const;

	//Returns path in internal format
	std::wstring getInternal() const;
	//Returns path in system format
	std::wstring getSystem() const;
};


#endif	//APOLLO_PATH_H