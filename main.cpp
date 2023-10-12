#include <cstdlib>
#include <glog/logging.h>
#include <iostream>
#include "window_manager.hpp"

using ::std::unique_ptr;

int main(int argc, char** argv) {
	std::cout << "hu";
	::google::InitGoogleLogging(argv[0]);
    std::cout << "he";

	unique_ptr<WindowManager> window_manager(WindowManager::Create());
	if (!window_manager) {
		LOG(ERROR) << "Failed to initialize window manager.";
		return EXIT_FAILURE;
	}

	std::cout << "ha";
	window_manager->Run();
	std::cout << "ho";

	return EXIT_SUCCESS;
}
