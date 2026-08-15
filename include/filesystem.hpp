#pragma once

#include <filesystem>

namespace filesystem {
	struct Entry {
		bool isDir = false;
		std::filesystem::path filePath;
	};

	extern std::filesystem::path cwd;

	std::vector<Entry> getEntries();

	void sortEntries(std::vector<Entry>& entries);
}