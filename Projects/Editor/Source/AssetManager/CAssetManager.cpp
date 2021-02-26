/*
!@
MIT License

CopyRight (c) 2021 Skylicht Technology CO., LTD

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction, including without limitation the Rights to use, copy, modify,
merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRight HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

This file is part of the "Skylicht Engine".
https://github.com/skylicht-lab/skylicht-engine
!#
*/

#include "pch.h"
#include "Version.h"
#include "CAssetManager.h"

#include <filesystem>
#include <chrono>
#include <sstream>
#include <sys/stat.h>

#include "Utils/CPath.h"
#include "Utils/CStringImp.h"
#include "Crypto/sha256.h"

#include "FileWatcher/FileWatcher.h"

#if defined(__APPLE_CC__)
namespace fs = std::__fs::filesystem;
#else
namespace fs = std::filesystem;
#endif

namespace Skylicht
{
	namespace Editor
	{
		CAssetManager::CAssetManager()
		{
			m_workingFolder = getIrrlichtDevice()->getFileSystem()->getWorkingDirectory().c_str();

			m_assetFolder = m_workingFolder + "/../Assets";
			m_assetFolder = CPath::normalizePath(m_assetFolder);

			m_haveAssetFolder = fs::exists(m_assetFolder);
			if (!m_haveAssetFolder)
				os::Printer::log("[CAssetManager] Asset folder is not exists");

		}

		CAssetManager::~CAssetManager()
		{

		}

		void CAssetManager::discoveryAssetFolder()
		{
			m_files.clear();
			m_guidToFile.clear();
			m_pathToFile.clear();

			if (m_haveAssetFolder)
			{
				for (const auto& file : fs::directory_iterator(m_assetFolder))
				{
					std::string path = file.path().generic_u8string();

					if (file.is_directory())
					{
						std::string bundle = CPath::getFileName(path);
						discovery(bundle.c_str(), path.c_str());
					}
				}
			}
		}

		void CAssetManager::discovery(const std::string& bundle, const std::string& folder)
		{
			addFileNode(bundle, folder);

			for (const auto& file : fs::directory_iterator(folder))
			{
				std::string path = file.path().generic_u8string();

				if (file.is_directory())
					discovery(bundle, path);
				else
					addFileNode(bundle, path);
			}
		}

		void CAssetManager::update()
		{

		}

		bool CAssetManager::addFileNode(const std::string& bundle, const std::string& path)
		{
			std::time_t now = std::time(0);
			std::string assetPath = m_assetFolder + "/";
			time_t modifyTime, createTime;

			if (getFileDate(path.c_str(), modifyTime, createTime) == true)
			{
				// get short path
				std::string sortPath = path;
				sortPath.replace(sortPath.find(assetPath.c_str()), assetPath.size(), "");

				// add db
				m_files.push_back(
					SFileNode(
						bundle.c_str(),
						sortPath.c_str(),
						path.c_str(),
						generateHash(bundle.c_str(), path.c_str(), createTime, now).c_str(),
						modifyTime,
						createTime)
				);

				// map guid
				SFileNode& file = m_files.back();
				m_guidToFile[file.Guid] = &file;
				m_pathToFile[sortPath] = &file;

				return true;
			}

			return false;
		}

		bool CAssetManager::getFileDate(const char* path, time_t& modifyTime, time_t& createTime)
		{
			struct stat result;
			if (stat(path, &result) == 0)
			{
				modifyTime = result.st_mtime;
				createTime = result.st_ctime;
				return true;
			}

			return false;
		}

		std::string CAssetManager::generateHash(const char* bundle, const char* path, time_t createTime, time_t now)
		{
			std::string fileName = CPath::getFileName(std::string(path));
			std::string hashString = std::string(bundle) + std::string(":") + fileName;
			hashString += ":";
			hashString += std::to_string(createTime);
			hashString += ":";
			hashString += std::to_string(now);

			BYTE8 buf[SHA256_BLOCK_SIZE];
			SHA256_CTX ctx;

			sha256_init(&ctx);
			sha256_update(&ctx, (const BYTE8*)hashString.c_str(), hashString.length());
			sha256_final(&ctx, buf);

			std::stringstream result;
			for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
				result << std::setfill('0') << std::setw(2) << std::hex << (int)buf[i];

			return result.str();
		}

		void CAssetManager::getRoot(std::vector<SFileInfo>& files)
		{
			files.clear();
			std::string assetPath = m_assetFolder + "/";
			wchar_t name[512];

			for (const auto& file : fs::directory_iterator(m_assetFolder))
			{
				std::string path = file.path().generic_u8string();

				if (file.is_directory())
				{
					files.push_back(SFileInfo());
					SFileInfo& file = files.back();

					file.Name = CPath::getFileName(path);
					file.FullPath = path;
					file.Path = path;
					file.Path.replace(file.Path.find(assetPath.c_str()), assetPath.size(), "");
					file.IsFolder = true;
					file.Type = Folder;
					file.Node = m_pathToFile[file.Path];

					CStringImp::convertUTF8ToUnicode(file.Name.c_str(), name);
					file.NameW = name;
				}
			}
		}

		void CAssetManager::getFolder(const char* folder, std::vector<SFileInfo>& files)
		{
			files.clear();
			std::string assetPath = m_assetFolder + "/";
			wchar_t name[512];

			for (const auto& file : fs::directory_iterator(folder))
			{
				std::string path = file.path().generic_u8string();

				if (file.is_directory())
				{
					files.push_back(SFileInfo());
					SFileInfo& file = files.back();

					file.Name = CPath::getFileName(path);
					file.FullPath = path;
					file.Path = path;
					file.Path.replace(file.Path.find(assetPath.c_str()), assetPath.size(), "");
					file.IsFolder = true;
					file.Type = Folder;
					file.Node = m_pathToFile[file.Path];

					CStringImp::convertUTF8ToUnicode(file.Name.c_str(), name);
					file.NameW = name;
				}
				else
				{
					files.push_back(SFileInfo());
					SFileInfo& file = files.back();

					file.Name = CPath::getFileName(path);
					file.FullPath = path;
					file.Path = path;
					file.Path.replace(file.Path.find(assetPath.c_str()), assetPath.size(), "");
					file.IsFolder = false;
					file.Node = m_pathToFile[file.Path];

					CStringImp::convertUTF8ToUnicode(file.Name.c_str(), name);
					file.NameW = name;

					std::string ext = CPath::getFileNameExt(path);

				}
			}
		}
	}
}