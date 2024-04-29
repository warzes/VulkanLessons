#include "stdafx.h"
#include "FileSystem.h"

FileStat FileSystem::StatFile(const Path& path)
{
	std::error_code ec;

	auto fs_stat = std::filesystem::status(path, ec);
	if (ec)
	{
		return FileStat{
			false,
			false,
			0,
		};
	}

	auto size = std::filesystem::file_size(path, ec);
	if (ec)
	{
		size = 0;
	}

	return FileStat{
		fs_stat.type() == std::filesystem::file_type::regular,
		fs_stat.type() == std::filesystem::file_type::directory,
		size,
	};
}

bool FileSystem::IsFile(const Path& path)
{
	auto stat = StatFile(path);
	return stat.is_file;
}

bool FileSystem::IsDirectory(const Path& path)
{
	auto stat = StatFile(path);
	return stat.is_directory;
}

bool FileSystem::Exists(const Path& path)
{
	auto stat = StatFile(path);
	return stat.is_file || stat.is_directory;
}

bool FileSystem::CreateDir(const Path& path)
{
	std::error_code ec;

	std::filesystem::create_directory(path, ec);

	if (ec)
	{
		throw std::runtime_error("Failed to create directory");
	}

	return !ec;
}

std::vector<uint8_t> FileSystem::ReadChunk(const Path& path, size_t offset, size_t count)
{
	std::ifstream file{ path, std::ios::binary | std::ios::ate };

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file for reading");
	}

	auto size = StatFile(path).size;

	if (offset + count > size)
	{
		return {};
	}

	// read file contents
	file.seekg(offset, std::ios::beg);
	std::vector<uint8_t> data(count);
	file.read(reinterpret_cast<char*>(data.data()), count);

	return data;
}

void FileSystem::WriteFile(const Path& path, const std::vector<uint8_t>& data)
{
	// create directory if it doesn't exist
	auto parent = path.parent_path();
	if (!std::filesystem::exists(parent))
	{
		create_directory(parent);
	}

	std::ofstream file{ path, std::ios::binary | std::ios::trunc };

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file for writing");
	}

	file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

void FileSystem::Remove(const Path& path)
{
	std::error_code ec;

	std::filesystem::remove(path, ec);

	if (ec)
	{
		throw std::runtime_error("Failed to remove file");
	}
}

const Path& FileSystem::ExternalStorageDirectory() const
{
	return m_externalStorageDirectory;
}

const Path& FileSystem::TempDirectory() const
{
	return m_tempDirectory;
}

void FileSystem::WriteFile(const Path& path, const std::string& data)
{
	WriteFile(path, std::vector<uint8_t>(data.begin(), data.end()));
}

std::string FileSystem::ReadFileString(const Path& path)
{
	auto bin = ReadFileBinary(path);
	return { bin.begin(), bin.end() };
}

std::vector<uint8_t> FileSystem::ReadFileBinary(const Path& path)
{
	auto stat = StatFile(path);
	return ReadChunk(path, 0, stat.size);
}
