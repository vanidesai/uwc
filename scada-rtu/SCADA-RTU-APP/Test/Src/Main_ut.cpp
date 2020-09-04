/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../Inc/Main_ut.hpp"

void Main_ut::SetUp()
{
	// Setup code
}

void Main_ut::TearDown()
{
	// TearDown code
}


/****************************************************************/
/* Test helper function:
 * Sets global variable g_shouldStop to true after 1 sec,
 * So that function timerThread doesnt run infinitely.
 */
static void Set_g_shouldStop()
{
	std::this_thread::sleep_for(std::chrono::seconds(1));
	g_shouldStop = true;
}
/****************************************************************/


/* updateDataPoints: ? */
TEST_F(Main_ut, updateDataPoints_001)
{
	/*if(-1 == sem_init(&( QMgr::getMqtt().m_semaphore ), 0, 0))
	{
		cout<<endl<<"[ UT ]:     semaphore initialization failed!"<<endl;
		exit(0);
	}
	else
	{
		cout<<endl<<"[ UT ]:     semaphore initialization is successful"<<endl;
	}*/

	std::thread TestTarget( updateDataPoints );
	std::thread TestHelper(Set_g_shouldStop);

	TestTarget.join();
	TestHelper.join();

	// Setting the value of "g_shouldStop back" to default value
	g_shouldStop = false;
}
