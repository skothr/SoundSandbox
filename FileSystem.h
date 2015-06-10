#ifndef APOLLO_FILE_SYSTEM_H
#define APOLLO_FILE_SYSTEM_H

#include <string>

class Project;
class Sandbox;
class SystemPath;
class Path;

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

	//int data_bytes = (num_bytes < 0 ? t_size : num_bytes),
	//	pad_bytes = data_bytes - t_size;

	//Write given data
	//out.write((char*)&num, sizeof(T));
	out.write((char*)&num, sizeof(T));

	//Pad to specified number of bytes
	//out.write(padding, pad_bytes);

	return sizeof(T);
}

template<typename T>
int FileSystem::readFile(std::ifstream &in, T &num)//, int num_bytes)
{
	//static const char padding[MAX_FILE_PADDING] = {0};

	//int read_bytes = (num_bytes < 0 ? sizeof(num) : num_bytes),

	in.read((char*)&num, sizeof(T));

	return sizeof(T);
}

#endif	//APOLLO_FILE_SYSTEM_H