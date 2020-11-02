#ifdef UNIT_TEST

#include <gtest/gtest.h>
#include "Logger.hpp"
#include "EnvironmentVarHandler.hpp"

std::vector<std::string> m_vecEnv{"DEV_MODE"};

int main(int argc, char* argv[])
{
	EnvironmentInfo::getInstance().readCommonEnvVariables(m_vecEnv);

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

#endif
