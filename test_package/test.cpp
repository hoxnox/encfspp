#include <EncfsMounter.hpp>
#include <encfspp_config.h>

using namespace encfspp;

int main(int argc, char* argv[])
{
	EncfsMounter::set_readpassphrase(
		[](std::string prompt) -> std::string { return "testpass"; });
	EncfsMounter mounter(encfspp_TEST_SAMPLES_DIR + "/encrypted",
	                     encfspp_TEST_SAMPLES_DIR + "/mountpoint",
	                     encfspp_TEST_SAMPLES_DIR + "/config");
	return 0;
}

