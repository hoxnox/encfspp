#include <libencfs/EncfsMounter.hpp>
#include <libencfs_config.h>
#include <thread>
#include <chrono>

using namespace libencfs;

class TestEncfsMounter : public ::testing::Test
{
protected:
	void SetUp()
	{
	}
	void TearDown()
	{
	}
};

TEST_F(TestEncfsMounter, general)
{
	EncfsMounter mounter(libencfs_TEST_SAMPLES_DIR + "/encrypted",
	                     libencfs_TEST_SAMPLES_DIR + "/mountpoint",
	                     "testpass",
	                     libencfs_TEST_SAMPLES_DIR + "/config");
}

