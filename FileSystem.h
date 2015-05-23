#ifndef APOLLO_FILE_SYSTEM_H
#define APOLLO_FILE_SYSTEM_H

#include <string>

class Project;
class Sandbox;
class SystemPath;
class FileSystem;

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

class FileSystem
{
private:
	//To convert from wstring to string and vice versa
	//typedef std::codecvt_utf8<wchar_t> ConvertType_utf8;
	//static std::wstring_convert<ConvertType_utf8, wchar_t> converter_utf8;

	//Default directories (relative to baseDir)
	static const Path	projectsDir,
						ssDir,
						configDir;

	//Base directory --> default
	static Path			baseDir;

	static const int MAX_FILE_PADDING;

	//Makes sure the required files and directories exist in the current baseDir.
	static void checkRequiredFiles();

	//(Static class -- shouldn't have instances)
	FileSystem() { }
	~FileSystem() = default;

public:

	static void setDefaultBaseDir();
	static void setBaseDir(const std::string &base_dir);
	static void setBaseDir(const std::wstring &base_dir);

	static std::string getBaseDir();

	///DIRECTORY MANIPULATION FUNCTIONS///
	static void makeDir(const Path &dir_path);
	//static void makeDir(const std::wstring &dir_path);
	
	static void removeDir(const Path &dir_path);
	//static void removeDir(const std::wstring &dir_path);

	static bool pathExists(const Path &path);
	//static bool pathExists(const std::wstring &path);

	static bool dirExists(const Path &dir_path);
	//static bool dirExists(const std::wstring &dir_path);
	
	static bool fileExists(const Path &file_path);
	//static bool fileExists(const std::wstring &file_path);

	///FILE READING/WRITING///

	//static void writeMidiFile();
	//static void writeAudioFile();
	static void readProjectFile(const Path &file_path, Project &proj);
	static void writeProjectFile(const Path &file_path, const Project &proj);
	
	static void readSandboxFile(const Path &file_path, Sandbox &proj);
	static void writeSandboxFile(const Path &file_path, const Sandbox &sb);

	///HELPER FUNCTIONS///

	//Makes sure the given string's last character is a slash
	static void assertTrailingSlash(Path &path);

	static std::string toSystemPath(const std::string &path);	//Back slashes
	static std::wstring toSystemPath(const std::wstring &path);
	static std::string toInternalPath(const std::string &path);	//Forward slashes
	static std::wstring toInternalPath(const std::wstring &path);

	static std::wstring toWString(const std::string &str);
	static std::string toString(const std::wstring &wstr);
	
	//These functions read/write num_bytes bytes from/to the given file.
	//	- Returns number of bytes read/written
	//	- Do NOT use for strings/anything else that relies on pointers!!
	////	- If num_bytes < 0, reads/writes the size of the type
	////	- If num_bytes is different from sizeof(T), file data is padded with zeros (AFTER DATA)
	////	- num_bytes MUST be at least the size of the data (NO CHECKS DONE)
	template<typename T>
	static int writeFile(std::ofstream &out, const T &num);//, int num_bytes = -1);
	template<typename T>
	static int readFile(std::ifstream &in, T &num);//, int num_bytes = -1);

	static int writeFileString(std::ofstream &out, const std::string &str);
	static int readFileString(std::ifstream &in, std::string &str);

};

typedef FileSystem FileSys;


template<typename T>
int FileSystem::writeFile(std::ofstream &out, const T &num)//, int num_bytes)
{
	//static const char padding[MAX_FILE_PADDING] = {0};
	static const int t_size = sizeof(T);

	//int data_bytes = (num_bytes < 0 ? t_size : num_bytes),
	//	pad_bytes = data_bytes - t_size;

	//Write given data
	out.write((char*)&num, t_size);
	//Pad to specified number of bytes
	//out.write(padding, pad_bytes);

	return t_size;
}

template<typename T>
int FileSystem::readFile(std::ifstream &in, T &num)//, int num_bytes)
{
	//static const char padding[MAX_FILE_PADDING] = {0};
	static const int t_size = sizeof(T);

	//int read_bytes = (num_bytes < 0 ? sizeof(num) : num_bytes),

	in.read((char*)&num, t_size);

	return t_size;
}

#endif	//APOLLO_FILE_SYSTEM_H