#include <string>
#include "dispatch.hpp"
#include "input.hpp"
#include "render.hpp"

namespace dispatch {
	int dispatchCMD() {
		std::wstring cmd = input::commandBar;
		input::commandBar.clear();
		
		if (cmd == L"quit") {
			return -1;
		}

		return 0;
	}
}