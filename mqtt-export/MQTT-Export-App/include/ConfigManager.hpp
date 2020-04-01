/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#ifndef INCLUDE_CONFIGMANAGER_HPP_
#define INCLUDE_CONFIGMANAGER_HPP_

#include<stdio.h>
#include <string.h>
#include<unistd.h>
#include <eis/config_manager/env_config.h>
#include <eis/config_manager/config_manager.h>
#include <iostream>
#include <yaml-cpp/yaml.h>
#include "Logger.hpp"
#include "Common.hpp"

#define GLOBAL_CONFIG_FILE_PATH "/opt/intel/eis/uwc_data/common_config/Global_Config.yml"
#define handle_error_en(en, msg) do { errno = en; perror(msg); } while (0)

/**
 * Config manager class
 */
class CfgManager {
public:

    /** Returns the single instance of this class
     *
     * @param  : nothing
     * @return : object of this class
     */
	static CfgManager& Instance();


    /** Returns the client status of creation
     *
     * @param : Nothing
     * @return: true/false based on status
     */
	bool IsClientCreated();

	/** Returns client from EIS Env library
	 *
	 * @param : Nothing
	 * @return: Configuration object
	 */
	const config_mgr_t* getConfigClient() const {
		return config_mgr_client;
	}

	/** Returns client from EIS Config library
	 *
	 * @param : Nothing
	 * @return: ENV object
	 */
	const env_config_t* getEnvClient() const {
		return env_config_client;
	}

private:

	/// True for success and false for failure
	bool isClientCreated;

	/// Local object for EIS ENV Manager
	env_config_t* env_config_client;

	/// Local object for EIS Config Manager
	config_mgr_t* config_mgr_client;

    /** Constructor
     */
	CfgManager();

    /** copy constructor is private
     */
	CfgManager(CfgManager const&);

    /** assignment operator is private
     */
	CfgManager& operator=(CfgManager const&);
};

/**
 * namespace for global configuration
 */
namespace globalConfig
{

// default values to be used for setting global configuration for all operations
#define DEFAULT_OPERATION_PRIORITY 1
#define DEFAULT_REALTIME false
#define DEFAULT_RETRIES 0
#define DEFAULT_QOS 0

/**
 * Enum of operation types and hierarchy
 */
enum eOperationType
{
	POLLING,       //!< POLLING
	ON_DEMAND_READ,//!< ON_DEMAND_READ
	ON_DEMAND_WRITE//!< ON_DEMAND_WRITE
};

/**
 * Enum of thread policy types and hierarchy
 */
typedef enum eThreadScheduler
{
	OTHER, //!< OTHER
	FIFO,  //!< FIFO
	RR,    //!< RR
	UNKNOWN//!< UNKNOWN
}threadScheduler;

//struct to hold thread info
struct stThreadInfo
{
	int priority;
	eThreadScheduler sched;
};

/**
 * Enum of data types
 */
enum eDataType
{
	DT_BOOL,        //!< DT_BOOL
	DT_INTEGER,     //!< DT_INTEGER
	DT_UNSIGNED_INT,//!< DT_UNSIGNED_INT
	DT_STRING,      //!< DT_STRING
	DT_MAP          //!< DT_MAP
};

/**
 * Class holds information about individual operation
 */
class COperation
{
	bool m_isRT;
	int m_operationPriority;
	int m_qos;
	int m_retries;

public:
	/** Populate COperation data structures
	 *
	 * @param : a_baseNode [in] : YAML node to read from
	 * @param : a_refOpration [in] : data structure to be fill
	 * @param : m_isRT [in] : RT/Noon-RT
	 * @return: Nothing
	 */
	static void build(const YAML::Node& a_baseNode,
			COperation& a_refOpration,
			const bool a_isRT);

	/**
	 * Check if operation is real-time
	 * @return true if operation is real-time
	 * 			false if operation is not real-time
	 */
	bool isRT() const
	{
		return m_isRT;
	}

	/**
	 * Get operation thread priority
	 * @return operation thread priority in int
	 */
	int getOperationPriority() const
	{
		return m_operationPriority;
	}

	/**
	 * Get QOS for operation
	 * @return QOS for operation in int
	 */
	int getQos() const
	{
		return m_qos;
	}

	/**
	 * Get operation retry count
	 * @return operation retry count in int
	 */
	int getRetries() const
	{
		return m_retries;
	}

};

/**
 * Class holds information about operation per operation type
 */
class COperationInfo
{
	eOperationType m_opType;
	bool m_defaultIsRT;

	COperation m_RTConfig;
	COperation m_NonRTConfig;

public:

	/** Populate COperation data structures
	 *
	 * @param : a_baseNode [in] : YAML node to read from
	 * @param : a_refOpInfo [in] : data structure to be fill
	 * @param : a_eOpType [in]: 	operation type (POLLING,ON_DEMAND_READ,	ON_DEMAND_WRITE)
	 * @return: Nothing
	 */
	static void buildOperationInfo(const YAML::Node& a_baseNode,
			COperationInfo& a_refOpInfo,
			const eOperationType a_eOpType);

	/**
	 * Get default RT configuration for this operation
	 * @return true if default RT
	 * 			false if not
	 */
	bool getDefaultRTConfig()
	{
		return m_defaultIsRT;
	}

	/**
	 * Get non-RT configuration of this operation
	 * @return reference of non-RT operation configuration
	 */
	COperation& getNonRTConfig()
	{
		return m_NonRTConfig;
	}

	/**
	 * Get RT configuration of this operation
	 * @return reference of RT operation configuration
	 */
	COperation& getRTConfig()
	{
		return m_RTConfig;
	}

};

/**
 * Class holds global configuration for all operations
 */
class CGlobalConfig
{
	COperationInfo m_OpPollingConfig;
	COperationInfo m_OpOnDemandReadConfig;
	COperationInfo m_OpOnDemandWriteConfig;

	// Private constructor so that no objects can be created.
	CGlobalConfig(){};
	CGlobalConfig(const CGlobalConfig & obj)=delete;
	CGlobalConfig& operator=(CGlobalConfig const&)=delete;

public:
	/**
	 * Get this instance of class
	 * @return reference of this class instance
	 */
	static CGlobalConfig& getInstance();

	/**
	 * Get configuration of polling operation
	 * @return reference to instance of polling operation class
	 */
	COperationInfo& getOpPollingOpConfig()
	{
		return m_OpPollingConfig;
	}

	/**
	 * Get configuration of on-demand read operation
	 * @return reference to instance of on-demand read operation class
	 */
	COperationInfo& getOpOnDemandReadConfig()
	{
		return m_OpOnDemandReadConfig;
	}

	/**
	 * Get configuration of on-demand write operation
	 * @return reference to instance of on-demand write operation class
	 */
	COperationInfo& getOpOnDemandWriteConfig()
	{
		return m_OpOnDemandWriteConfig;
	}
};

/**
 * Class holds information about thread priority for each operation
 */
class CPriorityMgr
{
	//map holds operational priority wise information about thread scheduler and thread priority
	std::map<int, std::pair<int, threadScheduler>> m_opPriorityThreadInfo;

	// Private constructor so that no objects can be created.
	CPriorityMgr();
	CPriorityMgr(const CPriorityMgr & obj)=delete;
	CPriorityMgr& operator=(CPriorityMgr const&)=delete;

public:
	/**
	 * Get only this instance of class
	 * @return this instance of class
	 */
	static CPriorityMgr& getInstance()
	{
		static CPriorityMgr _self;
			return _self;
	}

	/**
	 * Get thread priority depending on operational priority
	 * @param opPriority :[in] operational priority
	 * @return thread priority in int
	 */
	int getThreadPriority(int opPriority)
	{
		if(m_opPriorityThreadInfo.find(opPriority) != m_opPriorityThreadInfo.end())
		{
			return m_opPriorityThreadInfo.at(opPriority).first;
		}
		else
		{
			return -1;
		}
	}

	/**
	 * Get thread scheduling policy depending on operational-priority
	 * @param opPriority : [in]operational priority
	 * @return thread scheduling policy
	 */
	globalConfig::eThreadScheduler getThreadScheduler(int opPriority)
	{
		if(m_opPriorityThreadInfo.find(opPriority) != m_opPriorityThreadInfo.end())
		{
			return m_opPriorityThreadInfo.at(opPriority).second;
		}
		else
		{
			return UNKNOWN;
		}
	}
};

/** Read global configurations from YAML file and store it in data structures
 *
 * @return: true/false - based on success/failure
 */
bool loadGlobalConfigurations();

/** Function to validate key in YAML file
 *
 * @param : baseNode [in] : YAML node to read from
 * @param : a_sKey [in] : key to validate
 * @param : a_eDataType [in] : data type
 * @return: [int] : 0- success & -1 - if key is empty or absent & -2 if incorrect data type
 */
int validateParam(const YAML::Node& a_BaseNode,
		const std::string& a_sKey,
		const enum eDataType a_eDT);

/** Function to set default value
 ** @param : a_eOpType [in]: 	operation type (POLLING,ON_DEMAND_READ,	ON_DEMAND_WRITE)
 * @return: Nothing
 */
void setDefaultConfig(const enum eOperationType a_eOpType);

/** Function to set thread parameters
 ** @param : a_OpsInfo [in]: Operation class reference,
 ** @param : a_iPriority [in]: optional parameter for priority other than defined operation priority
 ** @param : a_eSched [in]: optional parameter for scheduler other than defined operation priority
 ** param : a_bIsOperation [in]: optional value. (if this flag is set then only a_iPriority & a_eSched will be used)
 * @return: Nothing
 */
void set_thread_sched_param(const COperation a_OpsInfo,
		const int a_iPriority = 0,
		const eThreadScheduler a_eSched = OTHER,
		const bool a_bIsOperation = false);

// functions added for displaying thread attr
void display_thread_sched_attr(const std::string a_sMsg);
void display_sched_attr(int policy, struct sched_param& param);

}  // namespace globalConfig

#endif /* INCLUDE_CONFIGMANAGER_HPP_ */
