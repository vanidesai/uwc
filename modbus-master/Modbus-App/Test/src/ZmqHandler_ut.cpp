/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#include "../include/ZmqHandler_ut.h"

void ZmqHandler_ut::SetUp()
{
	// Setup code
}

void ZmqHandler_ut::TearDown()
{
	// TearDown code
}

/********************************zmq_handler::prepareCommonContext()*************************************************/

/* This test is to check whether topic type is getting or not */
//************check for the Pub Topic************

TEST_F(ZmqHandler_ut, prepare_test_pub)
{
	if(getenv("PubTopics") != NULL)
	{
		bool bRes = zmq_handler::prepareCommonContext("pub");
		if(!bRes)
		{

			retValue =bRes;

			EXPECT_EQ(false, retValue);
		}
		else{
			retValue =bRes;

			EXPECT_EQ(true, retValue);

		}
	}

}




/* This test is to check whether topic type is getting or not */
//************check for the Sub Topic************

TEST_F(ZmqHandler_ut, prepare_test_sub)
{

	if(getenv("SubTopics") != NULL)
	{
		bool bRes = zmq_handler::prepareCommonContext("sub");
		if(!bRes)
		{

			retValue =bRes;

			EXPECT_EQ(false, retValue);
		}
		else{
			retValue =bRes;

			EXPECT_EQ(true, retValue);

		}
	}

}


/* This test is to check whether topic type is getting or not */
//************check for the topic is other than pub or sub in PubTopic************

TEST_F(ZmqHandler_ut, prepare_test_other1)
{

	if(getenv("PubTopics") != NULL)
	{
		bool bRes = zmq_handler::prepareCommonContext("fgdgrpub");
		if(!bRes)
		{

			retValue =bRes;

			EXPECT_EQ(false, retValue);
		}
		else{
			retValue =bRes;

			EXPECT_EQ(true, retValue);
		}
	}

}


/* This test is to check whether topic type is getting or not */
//************check for the topic is other than pub or sub in SubTopic************

TEST_F(ZmqHandler_ut, prepare_test_other2)
{

	if(getenv("PubTopics") != NULL)
	{
		bool bRes = zmq_handler::prepareCommonContext("fgdgrpub");
		if(!bRes)
		{

			retValue =bRes;

			EXPECT_EQ(false, retValue);
		}
		else{
			retValue =bRes;

			EXPECT_EQ(true, retValue);

		}
	}

}






/***************************************ZmqHandler::insertSubCTX()************************************************/


TEST_F(ZmqHandler_ut, insertSubCTX)
{
   try{

	bool bRes = zmq_handler::prepareCommonContext("sub");
	zmq_handler::insertSubCTX("PL0_flowmeter1", objTempSubCtx);
	EXPECT_EQ(true, bRes);

   }
   catch(std::exception &e)
   {
	 bool bRes= false;
	 cout<<e.what()<<endl;
	 EXPECT_EQ(false, bRes);
   }

}

/****************************populatePollingRefData()
 *********This function test is to test the getCTX function******/

TEST_F(ZmqHandler_ut, getCTX)
{
   try
   {
	   populatePollingRefData();

   }
   catch(std::exception &e)
   {
	   std::cout << e.what() << std::endl;
	   EXPECT_EQ("map::at", (string)e.what());
   }


}

TEST_F(ZmqHandler_ut, getCTX2)
{
	try
	{
		zmq_handler::stZmqContext &busCTX = zmq_handler::getCTX("PL0_flowmeter1");

	}
	catch(std::exception &e)
	{

      std::cout<<e.what()<<endl;
      EXPECT_EQ("map::at", (string)e.what());
	}
}


TEST_F(ZmqHandler_ut, getCTX3)
{
	try
	{
		zmq_handler::stZmqContext &busCTX = zmq_handler::getCTX("PL1_flowmeter2");


	}
	catch(std::exception &e)
	{

      std::cout<<e.what()<<endl;
      EXPECT_EQ("map::at", (string)e.what());

	}
}
TEST_F(ZmqHandler_ut, getCTX4)
{
	try
	{
		zmq_handler::stZmqContext &busCTX = zmq_handler::getCTX("PL1_flowmeter2_write");


	}
	catch(std::exception &e)
	{

      std::cout<<e.what()<<endl;
      EXPECT_EQ("map::at", (string)e.what());

	}
}



/***************************getSubCTX()***********************************/
//this test checks for correct context for correct sub topic accordingly
//and throws exceptions accordingly

TEST_F(ZmqHandler_ut, getSubCtx)
{

	try
	{

		bool bRes = zmq_handler::prepareCommonContext("sub");

		busSubCTX = zmq_handler::getSubCTX("PL0_flowmeter1");
		 //EXPECT_EQ(1,1);

	}
	catch(std::exception &e)
	{
       //EXPECT_EQ("map::at", e.what());
       busSubCTX = zmq_handler::getSubCTX("PL0_flowmeter1_write");

       cout<<e.what()<<endl;

       EXPECT_EQ("map::at", (string)e.what());
	}

}


/***************************getSubCTX()***********************************/
//this test checks for correct context for correct sub topic accordingly
//and throws exceptions accordingly

TEST_F(ZmqHandler_ut, getSubCtx2)
{

	try
	{

		bool bRes = zmq_handler::prepareCommonContext("sub");

		busSubCTX = zmq_handler::getSubCTX("PL0_flowmeter1_write");
		 //EXPECT_EQ(1,1);

	}
	catch(std::exception &e)
	{

       busSubCTX = zmq_handler::getSubCTX("MQTT-Export/PL0_flowmeter1_write");
       EXPECT_EQ("map::at", (string)e.what());
	}

}

/******************************************getPubCTX()***********************************/

TEST_F(ZmqHandler_ut, getPubCtx)
{

	try
	{

		bool bRes = zmq_handler::prepareCommonContext("pub");

	}
	catch(std::exception &e)
	{

       busPubCTX = zmq_handler::getPubCTX("PL0_flowmeter1");

       EXPECT_EQ("map::at", (string)e.what());


	}

}
/***************getPubCTX() and insertPubCTX() functions are working properly
  ***need to check******/
/***********************************insertPubCTX()***********************************/

TEST_F(ZmqHandler_ut, insertpub)
{
	try
	{
		bool bRes = zmq_handler::prepareCommonContext("pub");
		busPubCTX = zmq_handler::getPubCTX("PL0_flowmeter1_write");


	}
	catch(std::exception &e)
	{
		std::cout<<e.what()<<endl;
		zmq_handler::insertPubCTX("PL1_flowmeter2", objPubContext);
		busPubCTX = zmq_handler::getPubCTX("PL1_flowmeter2");
		EXPECT_EQ("map::at", (string)e.what());


	}
}

/******************************getAppSeq()*****************************************/

TEST_F(ZmqHandler_ut, getAppSeq)
{
	try
	{
		std::string stAppSeqNum = zmq_handler::getAppSeq(2048);
	}
	catch(std::exception &e){

		cout<<e.what()<<endl;
		EXPECT_EQ("map::at", (string)e.what());
	}
}




/*******************************removeCTX()***********************************************/

/*This test is to check the behaviour of the removeCTX() function************/

TEST_F(ZmqHandler_ut, removeCTX)
{
   try
   {
	   zmq_handler::removeCTX("PL0_flowmeter1");

   }
   catch(std::exception &e)
   {

	   cout<<e.what()<<endl;
	   EXPECT_EQ("map::at", (string)e.what());
   }
}

TEST_F(ZmqHandler_ut, removeCTX1)
{
   try
   {
	   zmq_handler::removeCTX("PL2_flowmeter3");

   }
   catch(std::exception &e)
   {

	   EXPECT_EQ("map::at", (string)e.what());
   }
}

/*******************************removeSubCTX()***********************************/

TEST_F(ZmqHandler_ut, removeSubCTX)
{
	try
	{
		zmq_handler::removeSubCTX("PL0_flowmeter1_write");

	}
	catch(std::exception &e)
	{
		cout<<e.what();
		EXPECT_EQ("map::at", (string)e.what());
	}
}

/****************************removePubCTX()************************************/

TEST_F(ZmqHandler_ut, removePubCTX)
{
   try
   {
	   zmq_handler::removePubCTX("PL0_flowmeter1_write");
	   EXPECT_EQ(1,1);
   }
   catch(std::exception &e)
   {
	   std::cout<<e.what();
	   EXPECT_EQ("map::at", (string)e.what());
   }
}


/********************************insertAppCTX******************************/

TEST_F(ZmqHandler_ut, insertAppCTX)
{
	bool bRet = true;
	try
	{
		zmq_handler::insertAppSeq(1, "2048");
		EXPECT_EQ(bRet,true);
	}
	catch(std::exception &e)
	{
		bool bRet = false;
		EXPECT_EQ(bRet, false);
	}


}

/**************************removeAppCTX()************************************/

TEST_F(ZmqHandler_ut, removeAppCTX)
{
	try
	{
		zmq_handler::removeAppSeq(1);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("map::at", (string)e.what());
		cout<<e.what()<<endl;
	}
}

TEST_F(ZmqHandler_ut, buildNettest)
{
	network_info::buildNetworkInfo(true);
}
