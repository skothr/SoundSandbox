#include "FileSystem.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#include <Windows.h>
#include <ShlObj.h>
#include "Shlwapi.h"
#pragma comment( lib, "shlwapi.lib")
#endif

#include <iostream>
#include <fstream>

#include <codecvt>
#include <vector>

#include <algorithm>

#include "Sandbox.h"
#include "Project.h"

#include "Path.h"



/////PATH/////


/////FILE SYSTEM/////

const Path	FileSystem::projectsDir		(L"projects/"),
			FileSystem::ssDir			(L"ss/"),
			FileSystem::configDir		(L"config/");

Path		FileSystem::baseDir;

const int	FileSystem::MAX_FILE_PADDING = 2048;
//std::wstring_convert<FileSystem::ConvertType_utf8, wchar_t> FileSystem::converter_utf8;

void FileSystem::checkRequiredFiles()
{
	if(baseDir != L"")
	{
		Path dir = baseDir + projectsDir;
		if(!dirExists(dir)) makeDir(dir);

		dir = baseDir + ssDir;
		if(!dirExists(dir)) makeDir(dir);
		
		dir = baseDir + configDir;
		if(!dirExists(dir)) makeDir(dir);

		//etc...
	}
	else
		std::cout << "WARNING: Invalid base directory!\n\n";
}

void FileSystem::setDefaultBaseDir()
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(WIN64) ||defined(_WIN64) || defined(__WIN64)
	//Get path to Documents folder
	WCHAR documents_dir[MAX_PATH];
	HRESULT result = SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, documents_dir);

	if(result != S_OK)
		std::cout << "Error: " << result << "\n";

	baseDir = toInternalPath(std::wstring(documents_dir));
	assertTrailingSlash(baseDir);
	
	//Make base ("SoundSandbox") directory 
	baseDir += L"SoundSandbox/";
	if(!dirExists(baseDir))
		makeDir(baseDir);
#endif

//TODO: Other OSes?

	checkRequiredFiles();
}
void FileSystem::setBaseDir(const std::string &base_dir)
{
	baseDir = toInternalPath(toWString(base_dir));
	checkRequiredFiles();
}
void FileSystem::setBaseDir(const std::wstring &base_dir)
{
	baseDir = toInternalPath(base_dir);
	checkRequiredFiles();
}

std::string FileSystem::getBaseDir()
{
	return toString(baseDir.getInternal());
}

///DIRECTORY MANIPULATION FUNCTIONS///

//void FileSystem::makeDir(const std::string &dir_path)
//{
//	makeDir(toWString(dir_path));
//}
void FileSystem::makeDir(const Path &dir_path)
{
	const std::wstring sys_path = dir_path.getSystem();

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
	if(!CreateDirectory(sys_path.c_str(), NULL))
		std::wcout << L"Error creating directory \"" << sys_path << "\"!\n";
#endif
}

//void FileSystem::removeDir(const std::string &dir_path)
//{
//	removeDir(toWString(dir_path));
//}
void FileSystem::removeDir(const Path &dir_path)
{
	const std::wstring sys_path = dir_path.getSystem();

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
	if(!RemoveDirectory(sys_path.c_str()))
		std::wcout << L"Error removing directory \"" << sys_path << "\"!\n";
#endif
}

//bool FileSystem::pathExists(const std::string &path)
//{
//	return pathExists(toWString(path));
//}
bool FileSystem::pathExists(const Path &path)
{
	const std::wstring sys_path = path.getSystem();

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
		return PathFileExists(sys_path.c_str());
#endif
}

bool FileSystem::dirExists(const Path &dir_path)
{
	const std::string sys_path = toString(dir_path.getSystem());

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
	DWORD ftype = GetFileAttributesA(sys_path.c_str());
	return ((ftype != INVALID_FILE_ATTRIBUTES) && (ftype & FILE_ATTRIBUTE_DIRECTORY));
#endif
}
//bool FileSystem::dirExists(const std::wstring &dir_path)
//{
//	return dirExists(toString(dir_path));
//}

bool FileSystem::fileExists(const Path &file_path)
{
	const std::string sys_path = toString(file_path.getSystem());

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
	DWORD ftype = GetFileAttributesA(sys_path.c_str());
	return ((ftype != INVALID_FILE_ATTRIBUTES) && !(ftype & FILE_ATTRIBUTE_DIRECTORY));
#endif
}
//bool FileSystem::fileExists(const std::wstring &file_path)
//{
//	return dirExists(toString(file_path));
//}

///FILE READING/WRITING///
void FileSystem::readProjectFile(const Path &file_path, Project &proj)
{

}
void FileSystem::writeProjectFile(const Path &file_path, const Project &proj)
{

}

void FileSystem::readSandboxFile(const Path &file_path, Sandbox &sb)
{
	std::ifstream i_file(file_path.getSystem(), std::ios::in | std::ios::binary);

	if(i_file.is_open())
	{
		int bytes_read = 0;

		bytes_read += readFile<int>(i_file, sb.sampleRate);

		i_file.close();
	}
	else
		std::cout << "File \"" << file_path << "\" failed to open.\n";
}
void FileSystem::writeSandboxFile(const Path &file_path, const Sandbox &sb)
{
	std::ofstream o_file(file_path.getSystem(), std::ios::out | std::ios::binary);

	if(o_file.is_open())
	{
		int bytes_written = 0;

		bytes_written += writeFile<int>(o_file, sb.sampleRate);

		o_file.close();
	}
	else
		std::cout << "File \"" << file_path << "\" failed to open.\n";

}


///HELPER FUNCTIONS///

void FileSystem::assertTrailingSlash(Path &path)
{
	path += (path.path.back() == L'\\' ? L"" : L"\\");
}


std::string FileSystem::toSystemPath(const std::string &path)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
	std::string sys_path = path;
	std::replace(sys_path.begin(), sys_path.end(), '/', '\\');
	return sys_path;
#else
	return toInternalPath(str);	//Non-Windows --> forward slashes
#endif
}
std::wstring FileSystem::toSystemPath(const std::wstring &path)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
	std::wstring sys_path = path;
	std::replace(sys_path.begin(), sys_path.end(), L'/', L'\\');
	return sys_path;
#else
	return toInternalPath(str);	//Non-Windows --> forward slashes
#endif
}
std::string FileSystem::toInternalPath(const std::string &path)
{
	std::string internal_path = path;
	std::replace(internal_path.begin(), internal_path.end(), '\\', '/');
	return internal_path;
}
std::wstring FileSystem::toInternalPath(const std::wstring &path)
{
	std::wstring internal_path = path;
	std::replace(internal_path.begin(), internal_path.end(), L'\\', L'/');
	return internal_path;
}

std::wstring FileSystem::toWString(const std::string &str)
{
	typedef std::codecvt_utf8<wchar_t> ConvertType_utf8;
	static std::wstring_convert<ConvertType_utf8, wchar_t> converter_utf8;

	return converter_utf8.from_bytes(str);
}
std::string FileSystem::toString(const std::wstring &wstr)
{
	typedef std::codecvt_utf8<wchar_t> ConvertType_utf8;
	static std::wstring_convert<ConvertType_utf8, wchar_t> converter_utf8;

	return converter_utf8.to_bytes(wstr);
}



int FileSystem::writeFileString(std::ofstream &out, const std::string &str)
{
	int n_bytes = str.length() + 1;

	//Write Device name
	out.write(str.c_str(), n_bytes);

	return n_bytes;
}
int FileSystem::readFileString(std::ifstream &in, std::string &str)
{
	int n_bytes = 0;
	char c;

	//Read string until terminated
	str = "";
	in.read(&c, 1);
	n_bytes++;

	while(c != '\0')
	{
		str += c;
		in.read(&c, 1);
		n_bytes++;
	}

	return n_bytes;
}