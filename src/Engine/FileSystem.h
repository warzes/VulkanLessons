#pragma once

struct FileStat
{
	bool   is_file;
	bool   is_directory;
	size_t size;
};

using Path = std::filesystem::path;

class FileSystem
{
public:
	FileSystem(
		Path externalStorageDirectory = std::filesystem::current_path(), 
		Path tempDirectory = std::filesystem::temp_directory_path()) 
		: m_externalStorageDirectory(std::move(externalStorageDirectory))
		, m_tempDirectory(std::move(tempDirectory))
	{}


	FileStat StatFile(const Path& path);
	bool IsFile(const Path& path);
	bool IsDirectory(const Path& path);
	bool Exists(const Path& path);
	bool CreateDir(const Path& path);
	std::vector<uint8_t> ReadChunk(const Path& path, size_t offset, size_t count);
	void WriteFile(const Path& path, const std::vector<uint8_t>& data);
	void Remove(const Path& path);

	const Path& ExternalStorageDirectory() const;
	const Path& TempDirectory() const;

	void WriteFile(const Path& path, const std::string& data);

	// Read the entire file into a string
	std::string ReadFileString(const Path& path);

	// Read the entire file into a vector of bytes
	std::vector<uint8_t> ReadFileBinary(const Path& path);

private:
	Path m_externalStorageDirectory;
	Path m_tempDirectory;
};

using FileSystemPtr = std::shared_ptr<FileSystem>;