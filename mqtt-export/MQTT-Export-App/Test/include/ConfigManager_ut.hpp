/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef CONFIGMANAGER_UT_H_
#define CONFIGMANAGER_UT_H_

#include <gtest/gtest.h>
#include "ConfigManager.hpp"


class ConfigManager_ut : public ::testing::Test {

protected:
	virtual void SetUp();
	virtual void TearDown();

public:
};

#endif /* CONFIGMANAGER_UT_H_ */
