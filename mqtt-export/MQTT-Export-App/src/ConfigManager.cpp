/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#include "ConfigManager.hpp"

using namespace std;

/** Constructor Initialized CfgManager instance depending on the value of DEV_MODE
 * @param None
 * @return None if successful, else application exits in case if config manager fails to initialize
*/
CfgManager:: CfgManager()
{
	env_config_client = env_config_new();

	if(CCommon::getInstance().isDevMode())
	{
		// create client without certificates
		config_mgr_client = config_mgr_new((char *)"etcd", (char *)"", (char *)"", (char *)"");
	}
	else
	{
		// create client with certificates
		string sCert = "/run/secrets/etcd_" + CCommon::getInstance().getStrAppName() + "_cert";
		string sKey = "/run/secrets/etcd_" + CCommon::getInstance().getStrAppName() + "_key";
		config_mgr_client = config_mgr_new((char *)"etcd", (char *)sCert.c_str(),
				(char *)sKey.c_str(),
				(char *)"/run/secrets/ca_etcd");
	}

	isClientCreated = false;

	if(config_mgr_client == NULL)
	{
		std::cout << "Failed to create configuration manager client" << std::endl;
		exit(-1);
	}
}

/**
 * Maintains single instance of CfgManager class
 * @param  : nothing
 * @return : this instance of CfgManager class
 */
CfgManager& CfgManager::Instance()
{
	static CfgManager _self;
	return _self;
}

/**
 * Check if config manager client is created or not
 * @param None
 * @return true/false - based on success/failure
 */
bool CfgManager::IsClientCreated()
{
	//config_mgr_t* cfgMangr = env_config.get_config_mgr_client();
	bool isClientCreated = (getConfigClient() !=NULL && getEnvClient() !=NULL)?true:false;
	return isClientCreated;
}

/**
 * Maintains single instance of CGlobalConfig class
 * @param  : nothing
 * @return : CGlobalConfig of this class
 */
globalConfig::CGlobalConfig& globalConfig::CGlobalConfig::getInstance()
{
	static globalConfig::CGlobalConfig  _self;
	return _self;
}

/**
 * Constructor Initializes default values of operation instance
 * @param None
 * @return None
 */
globalConfig::COperation::COperation()
{
	m_isRT = false;
	m_operationPriority = 0;
	m_qos = 0;
	m_retries = 0;
}

/**
 * Constructor Initializes default values of OperationInfo instance
 * @param None
 * @return None
 */
globalConfig::COperationInfo::COperationInfo()
{
	m_opType = UNKNOWN_OPERATION;
	m_defaultIsRT = false;
}

/**
 * Display scheduler attributes
 * @param policy : [in] scheduler policy
 * @param param : [in] param to display thread priority
 * @return None
 */
void globalConfig::display_sched_attr(int policy, struct sched_param& param)
{
	string sPolicy = "";
	if(policy == SCHED_FIFO)
	{
		sPolicy = "FIFO";
	}
	else if(policy == SCHED_RR)
	{
		sPolicy = "RR";
	}
	else if(policy == SCHED_OTHER)
	{
		sPolicy = "SCHED_OTHER";
	}
	else
	{
		sPolicy = "UNKNOWN";
	}
	CLogger::getInstance().log(INFO, LOGDETAILS("policy = " + sPolicy +
			" priority = "+ to_string(param.sched_priority)));
}

/**
 * Display thread attributes
 * @param a_sMsg : [in] string msg
 * @return None
 */
void globalConfig::display_thread_sched_attr(const std::string a_sMsg)
{
	int policy, s;
	static int count = 1;
	struct sched_param param;

	s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s != 0)
	{
		handle_error_en(s, "pthread_getschedparam");
	}
	CLogger::getInstance().log(INFO, LOGDETAILS("Thread number :: " + to_string(count++) +" Thread Name :: " + a_sMsg));

	display_sched_attr(policy, param);
}

/**
 * Constructor Fills up values in map with priority values
 * @param None
 * @return None
 */
globalConfig::CPriorityMgr::CPriorityMgr()
{
	//initialize all the default parameters
	m_opPriorityThreadInfo.insert(std::make_pair(6, std::make_pair(60, RR)));
	m_opPriorityThreadInfo.insert(std::make_pair(5, std::make_pair(55, RR)));
	m_opPriorityThreadInfo.insert(std::make_pair(4, std::make_pair(50, RR)));
	m_opPriorityThreadInfo.insert(std::make_pair(3, std::make_pair(15, RR)));
	m_opPriorityThreadInfo.insert(std::make_pair(2, std::make_pair(10, RR)));
	m_opPriorityThreadInfo.insert(std::make_pair(1, std::make_pair(5, RR)));
}

/**
 * Set thread policy and thread scheduler for current thread depending
 * on the operation it is going to perform
 * @param a_OpsInfo :[in] Operation class reference,
 * @param a_iPriority :[in] optional parameter for priority other than defined operation priority
 * @param a_eSched :[in] optional parameter for scheduler other than defined operation priority
 * @param a_bIsOperation :[in] optional value. (if this flag is set then only a_iPriority & a_eSched will be used)
 * @return None
 */
void globalConfig::set_thread_sched_param(const COperation a_OpsInfo,
		const int a_iPriority,
		const eThreadScheduler a_eSched,
		const bool a_bIsOperation)
{
	int iThreadPriority = 0;
	int operationPriority = 0;
	globalConfig::eThreadScheduler threadPolicy;

	if(!a_bIsOperation)
	{
		operationPriority = a_OpsInfo.getOperationPriority();
		iThreadPriority = CPriorityMgr::getInstance().getThreadPriority(operationPriority);
		threadPolicy = CPriorityMgr::getInstance().getThreadScheduler(operationPriority);
	}
	else
	{
		iThreadPriority = a_iPriority;
		threadPolicy = a_eSched;
	}

	//set priority
	sched_param param;

	if(iThreadPriority == -1 || threadPolicy == UNKNOWN)
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to set thread priority for this thread"));
		CLogger::getInstance().log(ERROR, LOGDETAILS("Invalid operation priority received for thread set priority"));
		return;
	}

	param.sched_priority = iThreadPriority;

	int result;
	result = pthread_setschedparam(pthread_self(), threadPolicy, &param);
	if(0 != result)
	{
		handle_error_en(result, "pthread_setschedparam");
		CLogger::getInstance().log(ERROR, LOGDETAILS("Cannot set thread priority to : " + std::to_string(iThreadPriority)));
		std::cout << __func__ << ":" << __LINE__ << " Cannot set thread priority, result : " << result << std::endl;
	}
	else
	{
		CLogger::getInstance().log(INFO, LOGDETAILS("thread priority to set to: " + std::to_string(iThreadPriority)));
	}
	//end of set priority for current thread
}

/**
 * Set default value in COperationInfo instance depending on the operation type
 * @param a_eOpType :[in] operation type (POLLING,ON_DEMAND_READ, ON_DEMAND_WRITE)
 * @return None
 */
void globalConfig::setDefaultConfig(const enum eOperationType a_eOpType)
{
	// set to default value based on operation type
	YAML::Node defaultNode;
	COperationInfo::buildOperationInfo(defaultNode,
			globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig(),
			a_eOpType);
}

/**
 * Validate given key in YAML file
 * @param baseNode [in] : YAML node to read from
 * @param a_sKey [in] : key to validate
 * @param a_eDataType [in] : data type
 * @return 0 if success, -1 if key is empty or absent, -2 if incorrect data type
 */
int globalConfig::validateParam(const YAML::Node& a_BaseNode,
		const std::string& a_sKey,
		const enum eDataType a_eDataType)
{
	int iRet = 0;
	try
	{
		if(!(a_BaseNode[a_sKey])  || a_BaseNode[a_sKey].as<std::string>() == "")
		{
			CLogger::getInstance().log(WARN, LOGDETAILS(a_sKey + " is not present or empty !!"));
			CLogger::getInstance().log(WARN, LOGDETAILS("setting it to default !!"));
			iRet = -1;
		}
		else
		{
			switch (a_eDataType)
			{
				case DT_BOOL:
				{
					a_BaseNode[a_sKey].as<bool>();
				}
				break;
				case DT_INTEGER:
				{
					a_BaseNode[a_sKey].as<int>();
				}
				break;
				case DT_UNSIGNED_INT:
				{
					a_BaseNode[a_sKey].as<unsigned>();
				}
				break;
				case DT_STRING:
				{
					a_BaseNode[a_sKey].as<std::string>();
				}
				break;
				case DT_MAP:
				{
					a_BaseNode[a_sKey].as<std::map<string,string>>();
				}
				break;
				default:
				{
					CLogger::getInstance().log(ERROR, LOGDETAILS("Invalid data type::" + to_string(a_eDataType)));
					iRet = -3;
				}
				break;
			}
		}
	}
	catch (YAML::Exception &e)
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS( a_sKey +
				" field data type is invalid, setting it to default, exception :: " +	std::string(e.what())));
		iRet = -2;
	}
	return iRet;
}

/**
 * Populate COperation data structures
 * @param a_baseNode :[in] YAML node to read from
 * @param a_refOpration :[in] data structure to be fill
 * @param m_isRT :[in] RT/Non-RT
 * @return None
 */
void globalConfig::COperation::build(const YAML::Node& a_baseNode,
		COperation& a_refOpration,
		const bool a_isRT)
{

	string ops = "";
	if(!a_isRT)
	{
		cout << "non-realtime parameters: " << endl;
		CLogger::getInstance().log(INFO, LOGDETAILS("non-realtime parameters: "));
		ops = "non-realtime";
	}
	else
	{
		cout << "realtime parameters: " << endl;
		CLogger::getInstance().log(INFO, LOGDETAILS("realtime parameters: "));
		ops = "realtime";
	}

	// validate priority
	if (validateParam(a_baseNode[ops], "operation_priority", DT_INTEGER) != 0)
	{
		// default value
		a_refOpration.m_operationPriority = DEFAULT_OPERATION_PRIORITY;
	}
	else
	{
		if(a_baseNode[ops]["operation_priority"].as<int>() <= 0 ||
				a_baseNode[ops]["operation_priority"].as<int>() > 6)
		{
			CLogger::getInstance().log(WARN, LOGDETAILS(
					"operation_priority parameter is out of range (i.e. expected value must be between 1-6 inclusive ) setting it to default (i.e. 1)"));
			a_refOpration.m_operationPriority = DEFAULT_OPERATION_PRIORITY;
		}
		else
		{
			a_refOpration.m_operationPriority = a_baseNode[ops]["operation_priority"].as<int>();
		}
	}

	// validate retries
	if (validateParam(a_baseNode[ops], "retries", DT_INTEGER) != 0)
	{
		// default value
		a_refOpration.m_retries = DEFAULT_RETRIES;
	}
	else
	{
		if(a_baseNode[ops]["retries"].as<int>() < 0 ||
				a_baseNode[ops]["retries"].as<int>() > 4)
		{
			CLogger::getInstance().log(WARN, LOGDETAILS(
					"retries parameter is out of range (i.e. expected value must be between 0-4 inclusive) setting it to default (i.e. 0)"));
			a_refOpration.m_retries = DEFAULT_RETRIES;
		}
		else
		{
			a_refOpration.m_retries = a_baseNode[ops]["retries"].as<int>();
		}
	}

	// validate qos
	if (validateParam(a_baseNode[ops], "qos", DT_INTEGER) != 0)
	{
		// default value
		a_refOpration.m_qos = DEFAULT_QOS;
	}
	else
	{
		if(a_baseNode[ops]["qos"].as<int>() < 0 ||
				a_baseNode[ops]["qos"].as<int>() > 2)
		{
			CLogger::getInstance().log(WARN, LOGDETAILS(
					"qos parameter is out of range (i.e. expected value must be between 0-2 inclusive) setting it to default (i.e. 0)"));
			a_refOpration.m_qos = DEFAULT_QOS;
		}
		else
		{
			a_refOpration.m_qos = a_baseNode[ops]["qos"].as<int>();
		}
	}

	cout << "	operation priority :: " <<a_refOpration.getOperationPriority()<< endl;
	cout << "	retries :: " <<a_refOpration.getRetries()<< endl;
	CLogger::getInstance().log(INFO, LOGDETAILS("operation priority :: " + to_string(a_refOpration.getOperationPriority())));
	CLogger::getInstance().log(INFO, LOGDETAILS("retries :: " + to_string(a_refOpration.getRetries())));
}

/**
 * Populate COperation data structures
 * @param a_baseNode :[in] YAML node to read from
 * @param a_refOpInfo :[in] data structure to be fill
 * @param a_eOpType :[in] operation type (POLLING,ON_DEMAND_READ, ON_DEMAND_WRITE)
 * @return None
 */
void globalConfig::COperationInfo::buildOperationInfo(const YAML::Node& a_baseNode,
		COperationInfo& a_refOpInfo,
		const eOperationType a_eOpType)
{
	// fill out operationType and default real-time
	a_refOpInfo.m_opType = a_eOpType;
	if (validateParam(a_baseNode, "default_realtime", DT_BOOL) != 0)
	{
		a_refOpInfo.m_defaultIsRT = DEFAULT_REALTIME;
	}
	else
	{
		a_refOpInfo.m_defaultIsRT = a_baseNode["default_realtime"].as<bool>();
	}
	cout << " default RT :: " << a_refOpInfo.m_defaultIsRT << endl;
	CLogger::getInstance().log(INFO, LOGDETAILS(" default RT :: " + to_string(a_refOpInfo.m_defaultIsRT)));

	COperation Obj;
	// realtime
	Obj.build(a_baseNode, a_refOpInfo.getRTConfig(), true);

	// non-realtime
	Obj.build(a_baseNode, a_refOpInfo.getNonRTConfig(), false);
}


/**
 * Read global configurations from YAML file
 * @param None
 * @return: true/false - based on success/failure
 */
bool globalConfig::loadGlobalConfigurations()
{
	bool bRetVal = false;
	bool isPollingExist = false;
	bool isOdReadExist = false;
	bool isOdWriteExist = false;

	try
	{
		YAML::Node config = YAML::LoadFile(GLOBAL_CONFIG_FILE_PATH);
		for (auto it : config)
		{
			if(it.first.as<std::string>() == "Global")
			{
				YAML::Node ops = it.second;
				YAML::Node listOps = ops["Operations"];
				for (auto key : listOps)
				{
					YAML::Node node;
					if(key["Polling"])
					{
						node = key["Polling"];

						cout << "********************************************************************";
						CLogger::getInstance().log(INFO, LOGDETAILS("\nFollowing Global configurations is available inside container"));
						cout << "\nFollowing Global configurations is available inside container \n";
						cout << "********************************************************************\n";
						CLogger::getInstance().log(INFO, LOGDETAILS("For Polling >>>"));
						cout << "For Polling >>>\n";
						COperationInfo::buildOperationInfo(node,
								globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig(),
								POLLING);
						isPollingExist = true;
						continue;
					}
					else if (key["on-demand-read"])
					{
						node = key["on-demand-read"];
						cout << "For On-demand read >>>\n";
						CLogger::getInstance().log(INFO, LOGDETAILS("For On-demand read >>>"));
						COperationInfo::buildOperationInfo(node,
								globalConfig::CGlobalConfig::getInstance().getOpOnDemandReadConfig(),
								ON_DEMAND_READ);
						isOdReadExist= true;
						continue;
					}
					else if (key["on-demand-write"])
					{
						node = key["on-demand-write"];
						cout << "For On-demand write >>>\n";
						CLogger::getInstance().log(INFO, LOGDETAILS("For On-demand write >>>"));
						COperationInfo::buildOperationInfo(node,
								globalConfig::CGlobalConfig::getInstance().getOpOnDemandWriteConfig(),
								ON_DEMAND_WRITE);
						isOdWriteExist= true;
						continue;
					}
				}
			}
		}
	}
	catch (YAML::Exception& e)
	{
		cout << "Error while loading global configurations :: " << e.what()<< "\n";
		CLogger::getInstance().log(ERROR, LOGDETAILS("Error while loading global configurations::" + std::string(e.what())));
		bRetVal = false;
	}

	if(isPollingExist && isOdReadExist && isOdWriteExist)
	{
		// success case
		bRetVal = true;
	}

	// check for errors
	if(!isPollingExist)
	{
		CLogger::getInstance().log(INFO, LOGDETAILS("Setting default config for Polling >>>"));
		cout << "Setting default config for Polling >>>\n";
		bRetVal = false;
		setDefaultConfig(POLLING);
	}
	if (!isOdReadExist)
	{
		CLogger::getInstance().log(INFO, LOGDETAILS("Setting default config for On-Demand-Read >>>"));
		cout << "Setting default config for On-Demand-Read >>>\n";
		bRetVal = false;
		setDefaultConfig(ON_DEMAND_READ);
	}
	if (!isOdWriteExist)
	{
		CLogger::getInstance().log(INFO, LOGDETAILS("Setting default config for On-Demand-Write >>>"));
		cout << "Setting default config for On-Demand-Write >>>\n";
		bRetVal = false;
		setDefaultConfig(ON_DEMAND_WRITE);
	}
	return bRetVal;
}
