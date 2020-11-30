```
********************************************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 **********************************************************************************************************************
```

uwc-common API's details and description

# Contents:

1. [API description of CommonDataShare](#Explaination-of-all-the-APIs-in-file-CommonDataShare)
2. [API description of ConfigManager](#Explaination-of-all-the-APIs-in-file-ConfigManager)
3. [API description of EnvironmentVarHandler](#Explaination-of-all-the-APIs-in-file-EnvironmentVarHandler)
4. [API description of Logger](#Explaination-of-all-the-APIs-in-file-Logger)
5. [API description of MQTTPubSubClient](#Explaination-of-all-the-APIs-in-file-MQTTPubSubClient)
6. [API description of NetworkInfo](#Explaination-of-all-the-APIs-in-file-NetworkInfo)
7. [API description of QueueHandler](#Explaination-of-all-the-APIs-in-file-QueueHandler)
8. [API description of YamlUtil](#Explaination-of-all-the-APIs-in-file-YamlUtil)
9. [API description of ZmqHandler](#Explaination-of-all-the-APIs-in-file-ZmqHandler)


# API description of CommonDataShare
Section to describe all the APIs in defined in file `CommonDataShare.cpp`

1. Purpose: To share module related common data to uwc-common library 
2. APIs' details: 
	1. Instance()
			1. Parent class: CcommonEnvManager			
			2. Description: 
			CcommonEnvManager& CcommonEnvManager::Instance()
			Funtion to return the single instance of the class CcommonEnvManager
			
	2. splitString()
			1. Parent class: CcommonEnvManager
			2. Is Singleton class: Yes
			3. Function to create class instance: Instance()
			4. Description:
			void CcommonEnvManager::splitString(const std::string &str, char delim)
			Function to split string with the given delimeter
			Input1: String to split
			Input2: Delometer by which string needs to split
			Return
	3. getTimeParams()
			1. Parent class: CcommonEnvManager
			2. Is Singleton class: Yes
			3. Function to create class instance: Instance()
			4. Description:
			void CcommonEnvManager::getTimeParams(std::string &a_sTimeStamp, std::string &a_sUsec)
			Function to get time parameters
			Input1: Reference to store time stamp
			Input2. Reference to store time in usec
			Return
	4. getAppName():
			1. Parent class: CcommonEnvManager
			2. Is Singleton class: Yes
			3. Function to create class instance: Instance()
			4. Description:
			std::string getAppName()
			Function to get app name
			Return: app name
	5. getDevMode():
			1. Parent class: CcommonEnvManager
			2. Is Singleton class: Yes
			3. Function to create class instance: Instance()
			4. Description:
			bool getDevMode()
			Function to get Dev mode
			Return: Return: Datatype=boolean, true for success, false otherwise
	6. ShareToLibUwcCmnData:
			1. Parent class: CcommonEnvManager
			2. Is Singleton class: Yes
			3. Function to create class instance: Instance()
			4. Description:
			void ShareToLibUwcCmnData(stUWCComnDataVal_t &ImportFromApp_Locobj)
			Function to set stUWCComnDataVal_t structure
			Input: Reference of class stUWCComnDataVal_t's object
	7. getTopicList:
			1. Parent class: CcommonEnvManager
			2. Is Singleton class: Yes
			3. Function to create class instance: Instance()
			4. Description:
			std::vector<std::string> getTopicList()
			Function gets subscribe topic list
			Return: vector containing topic list
	8. addTopicToList:
			1. Parent class: CcommonEnvManager
			2. Is Singleton class: Yes
			3. Function to create class instance: Instance()
			4. Description:
			void addTopicToList(const std::string a_sTopic)
			Input: Subscribe topic to add to topiclist vector

# API description of ConfigManager
Section to describe all the APIs in defined in file `ConfigManager.cpp`

1. Purpose: ConfigManager.cpp is used to handle configuration management using ConfigManager eis library
2. APIs' details: 
	1. Instance():
			1. Parent class: CfgManager
			2. Description:
			CfgManager& CfgManager::Instance()
			Function to returns the single instance of class CfgManager
	2. IsClientCreated():
			1. Parent class: cfManager
			2. Is singleton class: Yes
			3. Function to create class instance: Instance()
			4. Description:
			Function to return the client status of creation	
			Return: Datatype=boolean, true for success, false otherwise
	3. getInstance():
			1. Parent class: CGlobalConfig
			2. Is Singleton class: Yes
			3. Description:
			globalConfig::CGlobalConfig& globalConfig::CGlobalConfig::getInstance()
			Function to return the single instance of class CGlobalConfig
	4. COperation():
			1. Namespace: globalConfig
			2. Parent class: COperation
			3. Description:
			default constructor to initialize default values
	5. COperationInfo():
			1. Namespace: globalConfig
			2. Parent class: COperation
			3. Description:
			default constructor to initialize default values
	6. getInstance():
			1. Parent class: CPriorityMgr
			2. Is Singleton class: Yes
			3. Description:
			static CPriorityMgr& getInstance()
			Function to return the single instance of class CPriorityMgr
	7. getThreadPriority():
			1. Parent class: CPriorityMgr
			2. Is Singleton class: Yes
			3. Function to create class instance: getInstance()
			4. Description:
			int getThreadPriority(int opPriority)
			Function to get thread priority depending on operational priority
			Input: Operational Priority
			Return: thread priority in int
	8. getThreadScheduler():
			1. Parent class: CPriorityMgr
			2. Is Singleton class: Yes
			3. Function to create class instance: getInstance()
			4. Description:
			globalConfig::eThreadScheduler getThreadScheduler(int opPriority)
			Function to get thread scheduling policy depending on operational-priority
			Input: operational priority
			Return: thread scheduling policy
	9. display_sched_attr():
			1. Namespace: globalConfig
			2. Description:
			void globalConfig::display_sched_attr(int policy, struct sched_param& param)
			Function to display scheduler attributes
			Input1: scheduler policy
			Input2: Parameter to display thread priority 
			Return
	10. display_thread_sched_attr():
			1. Namesapce: globalConfig
			2. Description:
			void globalConfig::display_thread_sched_attr(const std::string a_sMsg)
			Function to display thread attributes
			Input1: string msg
			Return
		
	11. set_thread_sched_param():
			1. Namespace: globalConfig
			2. Description:
			void globalConfig::set_thread_sched_param(const COperation a_OpsInfo,
								const int a_iPriority,
								const eThreadScheduler a_eSched,
								const bool a_bIsOperation)
			Function to set thread parameters
			Input1: Operation class reference
			Input2: optional parameter for priority other than defined operation priority
			Input3: optional parameter for scheduler other than defined operation priority
			Input4: optional value. (if this flag is set then only a_iPriority & a_eSched will be used)
			Return
	12. setDefaultConfig():
			1. Namespace: globalConfig
			2. Description:
			void globalConfig::setDefaultConfig(const enum eOperationType a_eOpType)
			Function to set default value based on operation type
			Input: operation type (POLLING,ON_DEMAND_READ,	ON_DEMAND_WRITE)
			Return

	13. validateParam():
			1. Namespace: globalConfig
			2. Description:
			int globalConfig::validateParam(const YAML::Node& a_BaseNode,
							const std::string& a_sKey,
							const enum eDataType a_eDataType)
			Function to validate key in YAML file
			Input1: YAML node to read from
			Input2: key to validate
			Input3: data type
			Return: 0 on success and -1 if key is empty or absent  and -2 if incorrect data type
	14. build():
			1. Namespace: globalConfig
			2. Parent class: COperation
			3. Is Singleton class: No
			4. Description:
			void globalConfig::COperation::build(const YAML::Node& a_baseNode,
							COperation& a_refOpration,
							const bool a_isRT)
			Function to Populate COperation data structures
			Input1: YAML node to read from
			Input2: Reference of class COperation's object
			Input3: DataType=bool, RT/Non-RT 
			Return
	15. buildOperationInfo():
			1. Namespace: globalConfig
			2. Parent class: COperationInfo
			3. Is Singleton class: No
			4. Description:
			void globalConfig::COperationInfo::buildOperationInfo(const YAML::Node& a_baseNode,
									COperationInfo& a_refOpInfo,
									const eOperationType a_eOpType)
			Function to Populate COperationInfo data structures
			Input1: YAML node to read from
			Input2: Reference of class COperationInfo's object
			Input3: operation type (POLLING,ON_DEMAND_READ,	ON_DEMAND_WRITE)
			Return
 	16. loadGlobalConfigurations():
			1. Namespace: globalConfig
			2. Description:
			bool globalConfig::loadGlobalConfigurations()
			Read global configurations from YAML file (Global_Config.yml)
			Global configuration is available in common_config dir from docker volume
			if any configuration is missing/invalid then following configuration will be used as a default one,
				#	default_realtime: false
				#	operation_priority: 1
				#	retries: 0
				#	qos: 0
			Return: true/false based on success/failure
	17. CSparkplugData():
			1. Namespace: globalConfig
			2. Parent class: CSparkplugData
			3. Is Singleton class: No
			4. Description:
			Default contructor
	18. buildSparkPlugInfo():
			1. Namespace: globalConfig
			2. Parent class: CSparkplugData
			3. Is Singleton class: No
			4. Description:	
			void globalConfig::CSparkplugData::buildSparkPlugInfo(const YAML::Node& a_baseNode,
										CSparkplugData& a_refOpration)
			Function to Populate CSparkplugData data structures
			Input1: YAML node to read from
			Input2: Reference of class CSparkplugData's object
			Return

# API description of EnvironmentVarHandler
Section to describe all the APIs in defined in file `EnvironmentVarHandler.cpp`

1. Purpose: EnvironmentVarHandler.cpp is used to handle environment variables of the calling application
2. APIs' details:
	1. readCommonEnvVariables()
			1. Parent class: EnvironmentInfo
			2. Is singleton class: Yes
			3. Function to create class instance: getInstance()
			4. Description:
			`bool readCommonEnvVariables(std::vector<std::string>)`
			function to read environment variables
			Input1: vector of strings, containing all environment variables
			Return: Datatype=boolean, true for success, false otherwise
	2. addDataToEnvMap()
			1. Parent class: EnvironmentInfo
			2. Is singleton class: Yes
			3. Function to create class instance: getInstance()
			4. Description:
			`bool addDataToEnvMap(std::string, std::string)`
			function to add environment variable data to Map
			Input1: key containing the environment variables name
			Input2: value containing the value for each key
			Return: Datatype=boolean, true for success, false otherwise
	3. getDataFromEnvMap()
			1. Parent class: EnvironmentInfo
			2. Is singleton class: Yes
			3. Function to create class instance: getInstance()
			4. Description:
			`std::string getDataFromEnvMap(std::string)`
			Function to read data from map
			Input1: key string used to get data from map
			Input2: value containing the value for each key
			Return: Datatype=string, value of given key

# API description of Logger
Section to describe all the APIs in defined in file `Logger.cpp`

1. Purpose: Logger.cpp is used to log debug and error informations using `log4cpp` library
2. APIs' details:
	1. LogInfo()
			1. Parent class: CLogger
			2. Is singleton class: Yes
			3. Function to create class instance: getInstance()
			4. Description:
			`void LogInfo(std::string msg)`
			Write statement with info level
			Input1: statement to log
	2. LogInfo()
			1. Parent class: CLogger
			2. Is singleton class: Yes
			3. Function to create class instance: getInstance()
			4. Description:
			`void LogDebug(std::string msg)`
			Write statement with debug level
			Input1: statement to log
	3. LogWarn()
			1. Parent class: CLogger
			2. Is singleton class: Yes
			3. Function to create class instance: getInstance()
			4. Description:
			`void LogWarn(std::string msg)`
			Write statement with warn level
			Input1: statement to log
	4. LogError()
			1. Parent class: CLogger
			2. Is singleton class: Yes
			3. Function to create class instance: getInstance()
			4. Description:
			`void LogError(std::string msg)`
			Write statement with warn level
			Input1: statement to log
	5. LogFatal()
			1. Parent class: CLogger
			2. Is singleton class: Yes
			3. Function to create class instance: getInstance()
			4. Description:
			`void LogFatal(std::string msg)`
			Write statement with fatal level
			Input1: statement to log
	6. isLevelSupported()
			1. Parent class: CLogger
			2. Is singleton class: Yes
			3. Function to create class instance: getInstance()
			4. Description:
			`bool isLevelSupported(int priority)`
			Confirms that logger is correctly configured or not. if yes then returns true/false based on logging is enabled for the given priority or not.
			To enable logging for a particular priority(ERROR, WARN, INFO, DEBUG, FATAL), please edit property file `log4cpp.properties`
			Input1: priority(ERROR, WARN, INFO, DEBUG, FATAL)
			Return: Datatype=boolean, true for success, false otherwise
	7. DO_LOG_DEBUG(msg)
		1. Description:
		It is a macro which calls `LogDebug()` function with `msg` message, when logging is enabled for `DEBUG` priority.
	8. DO_LOG_WARN(msg)
		1. Description:
		It is a macro which calls `LogDebug()` function with `msg` message, when logging is enabled for `WARN` priority.
	9. DO_LOG_ERROR(msg)
		1. Description:
		It is a macro which calls `LogDebug()` function with `msg` message, when logging is enabled for `ERROR` priority.
	10. DO_LOG_FATAL(msg)
		1. Description:
		It is a macro which calls `LogDebug()` function with `msg` message, when logging is enabled for `FATAL` priority.

# API description of MQTTPubSubClient
Section to describe all the APIs in defined in file `MQTTPubSubClient.cpp`

1. Purpose: MQTTPubSubClient.cpp is used to handle operations like publishing, subscribing, connection etc to mqtt broker
2. APIs' details:
	1. CMQTTPubSubClient()
		Constructor to set all parameters needed to set a connection with MQTT broker
		Input1: MQTT broker URL
		Input2: Client id to be used to establish a connection
		Input3: QOS level to be used for communication with broker
		Input4: Tells whether TLS connection is needed
		Input5: MQTT CA certificate, needed when TLS = true
		Input6: MQTT client certificate, needed when TLS = true
		Input7: MQTT client private key, needed when TLS = true
		Input8: Action listener name to be used
	2. publishMsg()
		1. Parent class: CMQTTPubSubClient
			2. Is singleton class: No
			4. Description:
			`bool publishMsg(mqtt::message_ptr &a_pubMsg)`
			This function publishes a message on MQTT broker
			Input1: Pointer to message to be published
			Return: true/false status based on success/failure
	3. setWillMsg()
		1. Parent class: CMQTTPubSubClient
			2. Is singleton class: No
			4. Description:
			`bool setWillMsg(mqtt::message_ptr &a_willMsg)`
			Sets will message
			Input1: Pointer to will message
			Return: true/false status based on success/failure
	4. connect()
		1. Parent class: CMQTTPubSubClient
			2. Is singleton class: No
			4. Description:
			`bool connect()`
			This function tries to establish a connection with MQTT broker
			Return: true/false status based on success/failure
	5. disconnect()
		1. Parent class: CMQTTPubSubClient
			2. Is singleton class: No
			4. Description:
			`bool disconnect()`
			This function tries to stop a connection with MQTT broker
			Return: true/false status based on success/failure
	6. isConnected()
		1. Parent class: CMQTTPubSubClient
			2. Is singleton class: No
			4. Description:
			`bool isConnected()`
			This function checks if client is connected to the MQTT broker or not
			Return: true/false status based on connected/not connected
	7. subscribe()
		1. Parent class: CMQTTPubSubClient
			2. Is singleton class: No
			4. Description:
			`void subscribe(const std::string &a_sTopic)`
			This function subscribes to a topic on MQTT broker
			Input1: Topic to be published
	8. setNotificationConnect()
		1. Parent class: CMQTTPubSubClient
			2. Is singleton class: No
			4. Description:
			`void setNotificationConnect(mqtt::async_client::connection_handler a_fcbConnected)`
			Set connect notification
			Input1: Assync client connection handler when client is connected to MQTT broker
	9. setNotificationDisConnect()
		1. Parent class: CMQTTPubSubClient
			2. Is singleton class: No
			4. Description:
			`void setNotificationDisConnect(mqtt::async_client::connection_handler a_fcbDisconnected)`
			Set disconnect notification
			Input1: Assync client connection handler when client is disconnected to MQTT broker
	10. setNotificationMsgRcvd()
		1. Parent class: CMQTTPubSubClient
			2. Is singleton class: No
			4. Description:
			`void setNotificationMsgRcvd(mqtt::async_client::message_handler a_fcbMsgRcvd)`
			Set message recieved notification
			Input1: Assync client connection handler when message is recieved
	11. CMQTTBaseHandler()
		Constructor to set all parameters needed to set a connection with MQTT broker
		Input1: MQTT broker URL
		Input2: Client id to be used to establish a connection
		Input3: QOS level to be used for communication with broker
		Input4: Tells whether TLS connection is needed
		Input5: MQTT CA certificate, needed when TLS = true
		Input6: MQTT client certificate, needed when TLS = true
		Input7: MQTT client private key, needed when TLS = true
		Input8: Action listener name to be used
	12. connected()
		1. Parent class: CMQTTBaseHandler
			2. Is singleton class: No
			4. Description:
			`virtual void connected(const std::string &a_sCause)`
			This is a callback function which gets called when subscriber is connected with MQTT broker
			Input1: reason for connect
	13. disconnected()
		1. Parent class: CMQTTBaseHandler
			2. Is singleton class: No
			4. Description:
			`virtual void disconnected(const std::string &a_sCause)`
			This is a callback function which gets called when subscriber is disconnected with MQTT broker
			Input1: reason for disconnect
	14. msgRcvd()
		1. Parent class: CMQTTBaseHandler
			2. Is singleton class: No
			4. Description:
			`virtual void msgRcvd(mqtt::const_message_ptr a_pMsg)`
			This is a callback function which gets called when a msg is received
			Input1: pointer to received message
	15. isConnected()
		1. Parent class: CMQTTBaseHandler
			2. Is singleton class: No
			4. Description:
			`bool isConnected()`
			Checks if internal MQTT subscriber has been connected with the  MQTT broker
			Return: true/false as per the connection status of the external MQTT subscriber
	16. connect()
		1. Parent class: CMQTTBaseHandler
			2. Is singleton class: No
			4. Description:
			`void connect()`
			Helper function to connect MQTT client to Internal MQTT broker
	17. disconnect()
		1. Parent class: CMQTTBaseHandler
			2. Is singleton class: No
			4. Description:
			`void disconnect()`
			Helper function to disconnect MQTT client from Internal MQTT broker
	18. publishMsg()
		1. Parent class: CMQTTBaseHandler
			2. Is singleton class: No
			4. Description:
			`bool publishMsg(const std::string &a_sMsg, const std::string &a_sTopic)`
			Publish message on MQTT broker for MQTT-Export
			Input1: message to publish
			Input2: topic on which to publish message
			Return: return true/false based on success/failure

# API description of NetworkInfo
Section to describe all the APIs in defined in file `NetworkInfo.cpp`

1. Purpose:To build network info based on network type
2. APIs' details:
	1. populateUniquePointData():
			1. Namespace: Unnamed namespace to define globals
			2. Description:
			void populateUniquePointData(const CWellSiteInfo &a_oWellSite)
			Function to Populate unique point data
			Input: reference of class CWellSiteInfo's object 
			Return 
	2. _getWellSiteList():
			1. Namespace: Unnamed namespace to define globals
			2. Description:
			bool _getWellSiteList(string a_strSiteListFileName)
			Function to Get well site list
			Input: File name for Site list
			Return: true : on success, false : on error
	3. addPoint():
			1. Namespace: network_info
			2. Parent class: CUniqueDataDevice
			3. Is sindletone class: No
			4. Description:
			void network_info::CUniqueDataDevice::addPoint(const CUniqueDataPoint &a_rPoint)
			Function to Add unique data point reference to unique device
			Input: data point reference
			Return
	4. addDataPoint():
			1. Namespace: network_info
			2. Parent class: CDeviceInfo
			3. Is sindletone class: No
			4. Description:
			int network_info::CDeviceInfo::addDataPoint(CDataPoint a_oDataPoint)
			Function adds data point in m_DataPointList
			if DataPoint id is same in one file then it will be ignored
			Input: data point values to be add
			Return: 0 on success, -1 : on error (if point id is duplicate in datapoints file then this point will be ignored)
	5. addDevice():
			1. Namespace: network_info
			2. Parent class: CWellSiteInfo
			3. Is sindletone class: No
			4. Description:
			int network_info::CWellSiteInfo::addDevice(CWellSiteDevInfo a_oDevice)
			Function to add device
			Input: device to add
			Return: 0 : on success,
			-1 : on error - This error will be returned if device ID is common in one site and this device will be ignored
			-2 : if device type does not match with network type.
			E.g. RTU devices will be ignored in TCP container and vice versa 	
	6. build():
			1. Namespace: network_info
			2. Parent class: CWellSiteInfo
			3. Is sindletone class: No
			4. Description: 
			void network_info::CWellSiteInfo::build(const YAML::Node& a_oData, CWellSiteInfo &a_oWellSite)	
			This function build well site info to add all devices in data structures from YAML files
			Input1: a_oData : YAML data node to read from
			Input2: Data structures to store device information	
			Return
	7. validateIpAddress():
			1. Namespace: network_info
			2. Description:
			bool network_info::validateIpAddress(const string &ipAddress)
			Function to validate IP address
			Input: IP address to validate
			Return: true - on success, false - on error
	8. buildRTUNwInfo():
			1. Parent class: 
			2. Is sindletone class: No
			3. Description:
			void CRTUNetworkInfo::buildRTUNwInfo(CRTUNetworkInfo &a_oNwInfo,
								std::string a_fileName)
			Function builds network information for RTU network
			Input1: Reference of class CRTUNetworkInfo's object
			Input2: Filename
			Return
	9. build():
			1. Namespace: network_info
			2. Parent class: CWellSiteDevInfo
			3. Is sindletone class: No
			4. Description:
			void network_info::CWellSiteDevInfo::build(const YAML::Node& a_oData, CWellSiteDevInfo &a_oWellSiteDevInfo)
			This function builds well site device info to store device specific parameters mentioned in YAML file
			Input1: YAML data node to read from
			Input2: Reference of class CWellSiteDevInfo's object
			Return 
	10. build():
			1. Namespace: network_info
			2. Parent class: CDeviceInfo
			3. Is sindletone class: No
			4. Description:
			void network_info::CDeviceInfo::build(const YAML::Node& a_oData, CDeviceInfo &a_oCDeviceInfo )
			This function is used to read data points YML files and store it in CDeviceInfo data struct
			Input1: YAML data node to read from
			Input2: Reference of class CDeviceInfo's object
			Return
	11. getWellSiteList():
			1. Namespace: network_info
			2. Description:
			const std::map<std::string, CWellSiteInfo>& network_info::getWellSiteList()
			Function gets well site list
			Return: global well site map (e.g. ("PL0", CWellSiteInfo))
	12. getUniquePointList():
			1. Namespace: network_info
			2. Description:
			const std::map<std::string, CUniqueDataPoint>& network_info::getUniquePointList()
			Function gets unique point list
			Return: map of unique points
	13. getUniqueDeviceList():
			1. Namespace: network_info
			2. Description:
			const std::map<std::string, CUniqueDataDevice>& network_info::getUniqueDeviceList()
			Function gets unique device list
			Return: map of unique device
	14. getPointType():
			1. Namespace: network_info
			2. Parent class: CDataPoint
			3. Is sindletone class: No
			4. Description:
			eEndPointType network_info::CDataPoint::getPointType(const std::string& a_type)
			Function gets point type
			Return: point type enum based on string
	15. isNumber():
			1. Namespace: network_info
			2. Description:
			bool network_info:: isNumber(string s)
			Function checks if input is number
			Input: string might be containing number
			Return: true : on success, false : on error
	16. build():
			1. Namespace: network_info
			2. Parent class: CDataPoint
			3. Is sindletone class: No
			4. Description:
			void network_info::CDataPoint::build(const YAML::Node& a_oData, CDataPoint &a_oCDataPoint, bool a_bDefaultRealTime)
			This function is used to store each point information in CDataPoint class		
			Input1: YAML node to read from
			Input2: Reference of class CDataPoint's object
			Input3: bool value for RT or Non-RT
			Return
	17. printWellSite():
			1. Description: 
			void printWellSite(CWellSiteInfo a_oWellSite)
			Function to Print well site info
			Input: well site
			Return 
	18. buildNetworkInfo():
			1. Namespace: network_info
			2. Decription:
			Build network info based on network type
			if network type is TCP then this function will read all TCP devices and store it in associated data structures and 				vice versa for RTU
			void network_info::buildNetworkInfo(string a_strNetworkType, string a_strSiteListFileName, string a_strAppId)
			Input1: Network type
			Input2: File name for the site list
			Input3: App Id
			Return
	19. CUniqueDataPoint():
			1. Parent class: CUniqueDataPoint
			2. Description: Constructor
			CUniqueDataPoint::CUniqueDataPoint(std::string a_sId, const CWellSiteInfo &a_rWellSite,
				const CWellSiteDevInfo &a_rWellSiteDev, const CDataPoint &a_rPoint)
			Input1: a_sId = site id
			Input2: a_rWellSite = well site
			Input3: a_rWellSiteDev = well site device
			Input4: a_rPoint = data points
	20. CUniqueDataPoint():
			1. Parent class: CUniqueDataPoint
			2. Description: Constructor
			CUniqueDataPoint::CUniqueDataPoint(const CUniqueDataPoint&)
			Input: reference CUniqueDataPoint object for copy constructor
	21. isIsAwaitResp():
			1. Parent class: CUniqueDataPoint
			2. Is singleton class: No
			3. Description: 
			bool CUniqueDataPoint::isIsAwaitResp()
			Function Checks if response is received or not for specific point
			Return: true : on success, false : on error
	22. setIsAwaitResp():
			1. Parent class: CUniqueDataPoint
			2. Is singleton class: No
			3. Description: 
			void CUniqueDataPoint::setIsAwaitResp(bool isAwaitResp)
			Function Sets the response status for point
			Input: isAwaitResp = true/false based on response received or not

# API description of QueueHandler
Section to describe all the APIs in defined in file `QueueHandler.cpp`

1. Purpose: QueueHandler.cpp is used to handle mqtt message queue.
2. APIs' details:
	1. CMessageObject()
		Constructor, used to set TS to calendar time based in time base BASE.
	2. CMessageObject(const std::string &a_sTopic, const std::string &a_sMsg)
		Constructor, used to
		set TS to calendar time based in time base BASE,
		and initialise mqtt message based on input topic `a_sTopic` and message payload `a_sMsg`
	3. CMessageObject(mqtt::const_message_ptr a_mqttMsg)
		Constructor, used to
		set TS to calendar time based in time base BASE,
		and set mqtt message to `m_mqttMsg` to given input `a_mqttMsg`
	4. CMessageObject(const CMessageObject& a_obj)
		Constructor, used to initialise `m_mqttMsg` and `m_stTs` with another instance of class `CMessageObject`
	5. getTopic()
		1. Parent class: CMessageObject
			2. Is singleton class: No
			4. Description:
			`std::string getTopic()`
			Gets the topic for the message `m_mqttMsg`
			Return: Empty string if  `m_mqttMsg` is not initialised. Otherwise valid topic string
	6. getStrMsg()
		1. Parent class: CMessageObject
			2. Is singleton class: No
			4. Description:
			`std::string getStrMsg()`
			Gets the payload from `m_mqttMsg`
			Return: Empty string if  `m_mqttMsg` is not initialised. Otherwise valid message payload
	7. getMqttMsg()
		1. Parent class: CMessageObject
			2. Is singleton class: No
			4. Description:
			`mqtt::const_message_ptr& getMqttMsg()`
			Gets mqtt message
			Return: mqtt message
	8. getTimestamp()
		1. Parent class: CMessageObject
			2. Is singleton class: No
			4. Description:
			`struct timespec getTimestamp()`
			Gets structure for a time value
			Return: Structure for a time value
	9. CQueueHandler()
		1. Parent class: CQueueHandler()
			2. Is singleton class: No
			4. Description:
			`CQueueHandler()`
			Constructor, used to initialize semaphore `m_semaphore`
	10. pushMsg()
		1. Parent class: CQueueHandler
			2. Is singleton class: No
			4. Description:
			Push message in operational queue
			`bool pushMsg(CMessageObject msg)`
			Input1: MQTT message to push in message queue
			Return: Datatype=boolean, true for success, false otherwise
	11. isMsgArrived()
		1. Parent class: CQueueHandler
			2. Is singleton class: No
			4. Description:
			`bool isMsgArrived(CMessageObject& msg)`
			Checks if a new message has arrived and retrieves the message
			Input1: reference to message Object class which handles mqtt message, time & topic related operations
			Return: Datatype=boolean, true/false based on success/failure
	12. getSubMsgFromQ()
		1. Parent class: CQueueHandler
			2. Is singleton class: No
			4. Description:
			`bool getSubMsgFromQ(CMessageObject& msg)`
			Retrieve non-RT read message from message queue to publish on EIS
			Input1: reference to message to retrieve from queue
			Return: Datatype=boolean, true/false based on success/failure
	13. breakWaitOnQ()
		1. Parent class: CQueueHandler
			2. Is singleton class: No
			4. Description:
			`bool CQueueHandler::breakWaitOnQ()`
			Breaks wait on queue by posting semaphore `m_semaphore`
			Return: Datatype=boolean, true/false based on success/failure
	14. cleanup()
		1. Parent class: CQueueHandler
			2. Is singleton class: No
			4. Description:
			`void cleanup()`
			Clean up, destroy semaphores, disables callback, disconnect from MQTT broker
	15. clear()
		1. Parent class: CQueueHandler
			2. Is singleton class: No
			4. Description:
			`void clear()`
			Clears queue

# API description of YamlUtil
Section to describe all the APIs in defined in file `YamlUtil.cpp`

1. Purpose:To parse yaml file and save the data
2. APIs' details:
	1. loadYamlFile():
			1. Namespace: CommonUtils
			2. Description: 
			YAML::Node loadYamlFile(const std::string& filename)
			Function is used to load YAML file from local files
			Input: Actual filename to load
			Return: YAML node
	2. convertYamlToList():
			1. Namespace: CommonUtils
			2. Description:
			bool convertYamlToList(YAML::Node &node, std::vector<std::string>& a_slist)
			Function is used to store YAML to list
			Input1: Reference of YAML node
			Input2: Vector to store values from YAML
			Return: true/false based on success or error
	3. ConvertIPStringToCharArray():
			1. Namespace: CommonUtils
			2. Description:
			void ConvertIPStringToCharArray(string strIPaddr, unsigned char *ptrIpAddr)
			Function converts string to char array
			Input1: string to convert
			Input2: char array to store token key
			Return
	4. readEnvVariable():
			1. Namespace: CommonUtils
			2. Description:
			bool readEnvVariable(const char *pEnvVarName, string &storeVal)
			Function reads the Env variables
			Input1: Env variable name ptr 
			Input2: string to store value of Env variable
			Return: true/false based on success or error

# API description of ZmqHandler
Section to describe all the APIs in defined in file `ZmqHandler.cpp`

1. Purpose: To prepare and manage context related information for ZMQ communication
2. APIs' details:
	1. prepareContext():
			1. Namespace: zmq_handler
			2. Description: 
			bool zmq_handler::prepareContext(bool a_bIsPub,
							void* msgbus_ctx,
							std::string a_sTopic,
							config_t *config)
			Prepare pub or sub context for ZMQ communication
			Input1: flag to check for Pub or Sub
			Input2: Common message bus context used for zmq communication
			Input3: Topic for which pub or sub context needs to be created
			Input4: Config instance used for zmq library
			Return: true : on success, false : on error
	2. prepareCommonContext():
			1. Namespace: zmq_handler
			2. Description:
			bool zmq_handler::prepareCommonContext(std::string topicType)
			Prepare all EIS contexts for zmq communications based on topic configured in SubTopics or PubTopics section from 				docker-compose.yml file 
			Input: opic type to create context for, value is either "sub" or "pub"
			Return: true : on success, false : on error
	3. getSubCTX():
			1. Namespace: zmq_handler
			2. Description:
			stZmqSubContext& zmq_handler::getSubCTX(std::string a_sTopic)
			Function gets sub context for topic
			Input: topic to get sub context for
			Return: structure containing EIS contexts

	4. insertSubCTX():
			1. Namespace: zmq_handler
			2. Description:
			void zmq_handler::insertSubCTX(std::string a_sTopic, stZmqSubContext ctxRef)
			Function to Insert sub context
			Input1: topic to insert context for
			Input2: reference to context
			Return
	5. removeSubCTX():
			1. Namespace: zmq_handler
			2. Description:
			void zmq_handler::removeSubCTX(std::string a_sTopic)
			Function to Remove sub context
			Input1: topic to remove sub context for
			Return
	6. getCTX():
			1. Namespace: zmq_handler
			2. Description:
			stZmqContext& zmq_handler::getCTX(std::string a_sTopic)
			Function to get msgbus context for topic
			Input: topic for msgbus context
			Return: Reference to structure containing contexts
	7. insertCTX():
			1. Namespace: zmq_handler
			2. Description:
			void zmq_handler::insertCTX(std::string a_sTopic, stZmqContext& ctxRef)
			Function to Insert msgbus context
			Input1: topic to insert msgbus context for
			Input2: Reference of stZmqContext structure
			Return
	8. removeCTX():
			1. Namespace: zmq_handler
			2. Description:
			void zmq_handler::removeCTX(std::string a_sTopic)
			Function to  remove msgbus context
			Input: topic to remove msgbus context for
			Return
	9. getPubCTX():
			1. Namespace: zmq_handler
			2. Description:
			stZmqPubContext& zmq_handler::getPubCTX(std::string a_sTopic)
			Function to get pub context
			Input: topic for which to get pub context
			Return: Reference to structure containing EIS contexts
	10. insertPubCTX():
			1. Namespace: zmq_handler
			2. Description:
			bool zmq_handler::insertPubCTX(std::string a_sTopic, stZmqPubContext ctxRef)
			Function to insert pub contexts
			Input1: Toppic to insert pub context for
			Input2: context
			Return: true : on success, false : on error
	11. removePubCTX():
			1. Namespace: zmq_handler
			2. Description:
			void zmq_handler::removePubCTX(std::string a_sTopic)
			Function to Remove pub context
			Input: Topic to remove context for
			Return 
	12. publishJson(): 
			1. Namespace: zmq_handler
			2. Description:
			bool zmq_handler::publishJson(std::string &a_sUsec, msg_envelope_t* msg, const std::string &a_sTopic, std::string 								a_sPubTimeField)
			Function to Publish json
			Input1: USEC timestamp value at which a message is published
			Input2: message to publish
			Input3: topic on which to publish
			Input4: Time field to publish
			Return: true : on success, false : on error
