#include <EncfsMounter.hpp>
#include <encfspp_config.h>
#include <thread>
#include <chrono>

using namespace encfspp;

class TestEncfsMounter : public ::testing::Test {
protected:
	void SetUp()
	{
		EncfsMounter::set_readpassphrase(
			[](std::string prompt) -> std::string { return "testpass"; });
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
	EncfsMounter mounter(encfspp_TEST_SAMPLES_DIR + "/encrypted",
	                     encfspp_TEST_SAMPLES_DIR + "/mountpoint",
	                     encfspp_TEST_SAMPLES_DIR + "/config");
	std::ifstream file(encfspp_TEST_SAMPLES_DIR + "/mountpoint/testfile");
	ASSERT_TRUE(file.is_open());
	std::string fdata;
	file >> fdata;
	EXPECT_EQ("test", fdata);
}

TEST_F(TestEncfsMounter, write_read)
{
	{
		EncfsMounter mounter(encfspp_TEST_SAMPLES_DIR + "/encrypted",
							 encfspp_TEST_SAMPLES_DIR + "/mountpoint",
							 encfspp_TEST_SAMPLES_DIR + "/config");
		std::ofstream file(encfspp_TEST_SAMPLES_DIR + "/mountpoint/testfile2");
		ASSERT_TRUE(file.is_open());
		file << "tost";
	}

	std::ifstream file_(encfspp_TEST_SAMPLES_DIR + "/mountpoint/testfile2");
	ASSERT_FALSE(file_.is_open());

	{
		EncfsMounter mounter(encfspp_TEST_SAMPLES_DIR + "/encrypted",
							 encfspp_TEST_SAMPLES_DIR + "/mountpoint",
							 encfspp_TEST_SAMPLES_DIR + "/config");
		std::ifstream file(encfspp_TEST_SAMPLES_DIR + "/mountpoint/testfile2");
		ASSERT_TRUE(file.is_open());
		std::string fdata;
		file >> fdata;
		EXPECT_EQ("tost", fdata);
        unlink((encfspp_TEST_SAMPLES_DIR + "/mountpoint/testfile2").c_str());
	}
}
