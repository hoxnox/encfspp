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
		errlog.str("");
	}
	void TearDown()
	{
		EXPECT_TRUE(errlog.str().empty()) << errlog.str()
			<< "------------- LOGS -----------------" << std::endl
			<< inflog.str()
			<< vrblog.str();
	}
};

TEST_F(TestEncfsMounter, read)
{
	EncfsMounter mounter(libencfs_TEST_SAMPLES_DIR + "/encrypted",
	                     libencfs_TEST_SAMPLES_DIR + "/mountpoint",
	                     "testpass",
	                     libencfs_TEST_SAMPLES_DIR + "/config");
	std::ifstream file(libencfs_TEST_SAMPLES_DIR + "/mountpoint/testfile");
	ASSERT_TRUE(file.is_open());
	std::string fdata;
	file >> fdata;
	EXPECT_EQ("test", fdata);
}

TEST_F(TestEncfsMounter, write_read)
{
	{
		EncfsMounter mounter(libencfs_TEST_SAMPLES_DIR + "/encrypted",
							 libencfs_TEST_SAMPLES_DIR + "/mountpoint",
							 "testpass",
							 libencfs_TEST_SAMPLES_DIR + "/config");
		std::ofstream file(libencfs_TEST_SAMPLES_DIR + "/mountpoint/testfile2");
		ASSERT_TRUE(file.is_open());
		file << "tost";
	}

	std::ifstream file_(libencfs_TEST_SAMPLES_DIR + "/mountpoint/testfile2");
	ASSERT_FALSE(file_.is_open());

	{
		EncfsMounter mounter(libencfs_TEST_SAMPLES_DIR + "/encrypted",
							 libencfs_TEST_SAMPLES_DIR + "/mountpoint",
							 "testpass",
							 libencfs_TEST_SAMPLES_DIR + "/config");
		std::ifstream file(libencfs_TEST_SAMPLES_DIR + "/mountpoint/testfile2");
		ASSERT_TRUE(file.is_open());
		std::string fdata;
		file >> fdata;
		EXPECT_EQ("tost", fdata);
        unlink((libencfs_TEST_SAMPLES_DIR + "/mountpoint/testfile2").c_str());
	}
}
