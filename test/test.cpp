/**@author hoxnox <hoxnox@gmail.com>
 * @date 20160810 15:03:46
 *
 * @brief libencfs test launcher.*/

// Google Testing Framework
#include <gtest/gtest.h>
#include "tEncfsMounter.hpp"

// test cases

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}


