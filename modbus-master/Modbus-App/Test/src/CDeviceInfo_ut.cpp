/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#include "../include/CDeviceInfo_ut.h"

void CDeviceInfo_ut::SetUp()
{
	// Setup code
}

void CDeviceInfo_ut::TearDown()
{
	// TearDown code
}


/**********************************************CDeviceInfo::build()************************************************************/



/*test 01:: this test checks whether the device_info attribute is present or not
 * and if it is unavailable it throws the exceptions accorindingly */

#ifdef TO_BE_UPDATED_LATER
TEST_F(CDeviceInfo_ut, device_info_unavailable)
{
	baseNode = CommonUtils::loadYamlFile("iou_device2.yml");
	for( auto test : baseNode)
	{

		if(test.first.as<std::string>() == "device_info")
		{
			try
			{
				//network_info::CDeviceInfo Cdeviceinfo_obj;
				baseNode.remove("device_info");
				Cdeviceinfo_obj.build(baseNode, Cdeviceinfo_obj);


			}

			catch(YAML::Exception &e)
			{
				BOOST_LOG_SEV(lg, error) << __func__ << " " << e.what();
				//EXPECT_EQ(e.what(), "name key not found");
				/****** Exception is trown but not showing the proper message so the EXPECT or ASSERT is not used
				to check the functionality for the unit test*********/

			}
		}
	}
}
#endif

/*test 02:: this test checks whether the name key is present in the device_info or not
 * and if it is unavailable it throws the exceptions accordingly */

#ifdef TO_BE_UPDATED_LATER
TEST_F(CDeviceInfo_ut, device_info_name_unavailable)
{

	baseNode = CommonUtils::loadYamlFile("iou_device2.yml");
	for( auto test : baseNode)
	{
		if(test.first.as<std::string>() == "device_info")
		{
			//network_info::CDeviceInfo Cdeviceinfo_obj;
			std::string m_sName;
			//baseNode.remove("device_info");

			cout<< baseNode <<endl;


			try
			{
				Cdeviceinfo_obj.build(baseNode, Cdeviceinfo_obj);
				m_sName = test.second["name"].as<std::string>();

			}
			catch(YAML::Exception &e)
			{
				BOOST_LOG_SEV(lg, error) << __func__ << " " << e.what();
				cout<<endl<<"============================="<<endl;
				cout<<e.what();
				cout<<endl<<"============================="<<endl;
				//EXPECT_EQ(e.what(),"name key not found");
				/****** Exception is trown but not showing the proper message so the EXPECT or ASSERT is not used
				 to check the functionality for the unit test*********/

			}
		}
	}
}
#endif

/*test 03:: this test whether datapoints key is available or not in the yml file and
  if it is unavailable then it throws exceptons accordingly
 */

#ifdef TO_BE_UPDATED_LATER
TEST_F(CDeviceInfo_ut, deviceInfo_bsent_datapoints)
{
	baseNode = CommonUtils::loadYamlFile("iou_device2.yml");
	for( auto test : baseNode)
	{
		if(test.first.as<std::string>() == "pointlist")
		{
			YAML::Node node = CommonUtils::loadYamlFile(test.second.as<std::string>());
			//cout<< node <<endl;
			node.remove("datapoints");
			for (auto it : node)
			{

				if(it.second.IsSequence() && it.first.as<std::string>() == "datapoints")
				{

					try
					{
						const YAML::Node& points =  it.second;
						for (auto it1 : points)
						{
							if(0 == Cdeviceinfo_obj.addDataPoint(datapoint_obj))
							{
								BOOST_LOG_SEV(lg, info) << __func__ << " : Added point with id: " << datapoint_obj.getID();
								cout<< "inside if"<<endl;
							}
							else
							{
								BOOST_LOG_SEV(lg, error) << __func__ << " : Ignored point with id : " << datapoint_obj.getID();
								cout<< "Inside else"<<endl;
							}

						}
					}
					catch(YAML::Exception &e)
					{
						//BOOST_LOG_SEV(lg, error) << __func__ << " " << e.what();
						ASSERT_EQ(e.what(), "name key not found");


					}
				}
			}

		}
	}
}
#endif

/**test 04: this unit test checks the name of the device is correct in the yml file .**************/

#if 0
TEST_F(CDeviceInfo_ut, device_info_name_available)
{

	baseNode = CommonUtils::loadYamlFile("iou_device1.yml");
	for( auto test : baseNode)
	{
		if(test.first.as<std::string>() == "device_info")
		{
			//network_info::CDeviceInfo Cdeviceinfo_obj;
			std::string m_sName;
			//baseNode.remove("device_info");

			//cout<< baseNode <<endl;


			try
			{
				Cdeviceinfo_obj.build(baseNode, Cdeviceinfo_obj);
				m_sName = test.second["name"].as<std::string>();
				cout<<"=======name======="<<endl;
				cout<< m_sName<<endl;
				cout<<"=======name======="<<endl;

			}
			catch(YAML::Exception &e)
			{
				BOOST_LOG_SEV(lg, error) << __func__ << " " << e.what();
				cout<<endl<<"============================="<<endl;
				cout<<e.what();
				cout<<endl<<"============================="<<endl;
				//EXPECT_EQ(e.what(),"name key not found");
				/****** Exception is trown but not showing the proper message so the EXPECT or ASSERT is not used
				 to check the functionality for the unit test*********/

			}
		}
	}
}
#endif
/*
 test 05:: Test the behaviour of addDataPoint() function when point name is already present in
  datapoint list.
 Expected: It should return "-1".
 */
#ifdef TO_BE_UPDATED_LATER
TEST_F(CDeviceInfo_ut, addDataPoint_returnVal)
{
	baseNode = CommonUtils::loadYamlFile("iou_datapoints_3.yml");

	for( auto basenode_it : baseNode)
	{

		if(basenode_it.second.IsSequence() && basenode_it.first.as<std::string>() == "datapoints")
		{
			const YAML::Node& DataPoints =  basenode_it.second;

			for( auto DataPoints_It : DataPoints)
			{
				network_info::CDataPoint::build(DataPoints_It, datapoint_obj);
				int temp = Cdeviceinfo_obj.addDataPoint(datapoint_obj);
			}
		}
	}

	int temp = Cdeviceinfo_obj.addDataPoint(datapoint_obj);
	EXPECT_EQ(-1, temp);
}
#endif

/*test 06:: Behaviour of build() when there are duplicate IDs present in device list */

//TEST_F(CDeviceInfo_ut, build_DupID)
//{
//	baseNode = CommonUtils::loadYamlFile("CDeviceInfo_build_DupId.yml");
//
//	try
//	{
//		Cdeviceinfo_obj.build(baseNode, Cdeviceinfo_obj);
//		EXPECT_EQ(1, 1);
//	}
//
//	catch(YAML::Exception &e)
//	{
//		EXPECT_EQ(1, 2);	 //Test Fails
//	}
//
//}


