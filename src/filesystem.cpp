#include <filesystem>
#include "filesystem.hpp"

namespace filesystem {
	std::filesystem::path cwd = "C:\\";

	std::vector<Entry> getEntries() {
		std::vector<Entry> entries;
		for (auto& entry : std::filesystem::directory_iterator(cwd)) {
			entries.push_back( Entry{ .isDir = entry.is_directory(), .filePath = entry.path() } );
		}
		return entries;
	}

	void sortEntries(std::vector<Entry>& entries) {
		std::sort(entries.begin(), entries.end(),
			[](const Entry& a, const Entry& b) {
				if (a.isDir != b.isDir)
					return a.isDir > b.isDir;

				return a.filePath < b.filePath;
			});
	}
}