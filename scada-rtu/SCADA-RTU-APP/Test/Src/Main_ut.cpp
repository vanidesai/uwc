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

#if 0 //In progress
/* updateDataPoints: ? */
TEST_F(Main_ut, updateDataPoints_001)
{
	if(-1 == sem_init(&( QMgr::getMqtt().m_semaphore ), 0, 0))
	{
		cout<<endl<<"[ UT ]:     semaphore initialization failed!"<<endl;
		exit(0);
	}
	else
	{
		cout<<endl<<"[ UT ]:     semaphore initialization is successful"<<endl;
	}

	std::thread TestTarget( updateDataPoints, std::ref(QMgr::getMqtt()) );
	std::thread TestHelper(Set_g_shouldStop);

	TestTarget.join();
	TestHelper.join();

	// Setting the value of "g_shouldStop back" to default value
	g_shouldStop = false;
}
#endif

/* postMsgsToSCADA: success */
TEST_F(Main_ut, postMsgsToSCADA_Success)
{
	Bool_Res = postMsgsToSCADA(QMgr::getMqtt());

	EXPECT_EQ(true, Bool_Res);
}

/* isTaskComplete: When status = ready */
TEST_F(Main_ut, isTaskComplete_StatusReady)
{
	stTask.m_bStatus = future_status::ready;

	Bool_Res = isTaskComplete(stTask);

	EXPECT_EQ(true, Bool_Res);
}

/* isTaskComplete: When status = timeout */
TEST_F(Main_ut, isTaskComplete_StatusTimeout)
{
	stTask.m_bStatus = future_status::timeout;

	Bool_Res = isTaskComplete(stTask);

	EXPECT_EQ(false, Bool_Res);
}

/* isTaskComplete: When status = deferred */
TEST_F(Main_ut, isTaskComplete_StatusDeferred)
{
	stTask.m_bStatus = future_status::deferred;

	Bool_Res = isTaskComplete(stTask);

	EXPECT_EQ(false, Bool_Res);
}

/* About this test */
TEST_F(Main_ut, TimerThread_001)
{
	std::thread TestTarget(timerThread, 10 );
	std::thread TestHelper(Set_g_shouldStop);

	TestHelper.join();
	TestTarget.join();

	// Setting the value of "g_shouldStop back" to default value
	g_shouldStop = false;

	//EXPECT_EQ(false, Bool_Res);
}
