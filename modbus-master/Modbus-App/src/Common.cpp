/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#include "Common.hpp"
#include <sstream>
#include <chrono>

#include "Logger.hpp"
#include <mutex>

namespace
{
	std::map<unsigned short, MbusAPI_t> g_mapRequest;
}

std::mutex __appReqJsonLock;


/**
 * Get time parameters
 * @param a_sTimeStamp	:[in] reference to store time stamp
 * @param a_sUsec		:[in] reference to store time in usec
 */
/*void common_Handler::getTimeParams(std::string &a_sTimeStamp, std::string &a_sUsec)
{
	a_sTimeStamp.clear();
	a_sUsec.clear();

	const auto p1 = std::chrono::system_clock::now();

	std::time_t rawtime = std::chrono::system_clock::to_time_t(p1);
	std::tm* timeinfo = std::gmtime(&rawtime);
	if(NULL == timeinfo)
	{
		return;
	}
	char buffer [80];

	std::strftime(buffer,80,"%Y-%m-%d %H:%M:%S",timeinfo);
	a_sTimeStamp.insert(0, buffer);

	{
		std::stringstream ss;
		ss << std::chrono::duration_cast<std::chrono::microseconds>(p1.time_since_epoch()).count();
		a_sUsec.insert(0, ss.str());
	}
}*/

/**
 * Swap conversion
 * @param vt			:[in] vector
 * @param a_bIsByteSwap	:[in] is byte swap or not
 * @param a_bIsWordSwap	:[in] is word swap or not
 * @return swapped string
 */
std::string common_Handler::swapConversion(std::vector<unsigned char> vt, bool a_bIsByteSwap, bool a_bIsWordSwap)
{
	auto numbytes = vt.size();
	if(0 == numbytes)
	{
		return NULL;
	}

	auto iPosByte1 = 1, iPosByte2 = 0;
	auto iPosWord1 = 0, iPosWord2 = 1;

	if(true == a_bIsByteSwap)
	{
		iPosByte1 = 0; iPosByte2 = 1;
	}
	if(true == a_bIsWordSwap)
	{
		iPosWord1 = 1; iPosWord2 = 0;
	}

	static const char* digits = "0123456789ABCDEF";
	std::string sVal(numbytes*2+2,'0');
	int i = 0;
	sVal[i++] = '0'; sVal[i++] = 'x';
	int iCurPos = 0;
	while(numbytes)
	{
		if(numbytes >= 4)
		{
			auto byte1 = vt[iCurPos + iPosWord1*2 + iPosByte1];
			auto byte2 = vt[iCurPos + iPosWord1*2 + iPosByte2];
			auto byte3 = vt[iCurPos + iPosWord2*2 + iPosByte1];
			auto byte4 = vt[iCurPos + iPosWord2*2 + iPosByte2];
			numbytes = numbytes - 4;
			iCurPos = iCurPos + 4;

			sVal[i] = digits[(byte1 >> 4) & 0x0F];
			sVal[i+1] = digits[byte1 & 0x0F];
			i += 2;

			sVal[i] = digits[(byte2 >> 4) & 0x0F];
			sVal[i+1] = digits[byte2 & 0x0F];
			i += 2;

			sVal[i] = digits[(byte3 >> 4) & 0x0F];
			sVal[i+1] = digits[byte3 & 0x0F];
			i += 2;

			sVal[i] = digits[(byte4 >> 4) & 0x0F];
			sVal[i+1] = digits[byte4 & 0x0F];
			i += 2;
		}
		else if(numbytes >= 2)
		{
			auto byte1 = vt[iCurPos + iPosByte1];
			auto byte2 = vt[iCurPos + iPosByte2];
			numbytes = numbytes - 2;
			iCurPos = iCurPos + 2;

			sVal[i] = digits[(byte1 >> 4) & 0x0F];
			sVal[i+1] = digits[byte1 & 0x0F];
			i += 2;

			sVal[i] = digits[(byte2 >> 4) & 0x0F];
			sVal[i+1] = digits[byte2 & 0x0F];
			i += 2;
		}
		else
		{
			auto byte1 = vt[iCurPos];
			--numbytes;
			++iCurPos;

			sVal[i] = digits[(byte1 >> 4) & 0x0F];
			sVal[i+1] = digits[byte1 & 0x0F];
			i += 2;
		}
	}
	return sVal;
}

/**
 * Get request data from map
 * @param seqno		:[in] sequence no
 * @param reqData	:[out] reference to store request data
 * @return 	true : on success,
 * 			false : on error
 */
bool common_Handler::getReqData(unsigned short seqno, MbusAPI_t& reqData)
{
	DO_LOG_DEBUG("Start: " + std::to_string(seqno));
	bool bRet = true;
	try
	{
		std::unique_lock<std::mutex> lck(__appReqJsonLock);

		/// return the context
		reqData = g_mapRequest.at(seqno);
	}
	catch(exception &e)
	{
		DO_LOG_FATAL(LOGDETAILS(e.what()));
		bRet = false;
	}
	DO_LOG_DEBUG("End: ");

	return bRet;
}

/**
 * Insert request data into the map
 * @param seqno		:[in] sequence no
 * @param reqData	:[in] request data
 * @return 	true : on success,
 * 			false : on error
 */
bool common_Handler::insertReqData(unsigned short seqno, MbusAPI_t& reqData)
{
	//DO_LOG_DEBUG("Start: ");
	bool bRet = true;
	try
	{
		std::unique_lock<std::mutex> lck(__appReqJsonLock);

		/// insert the data
		g_mapRequest.insert(std::pair <unsigned short, MbusAPI_t> (seqno, reqData));
	}
	catch (exception &e)
	{
		DO_LOG_FATAL(e.what());
		bRet = false;
	}
	//DO_LOG_DEBUG("End: ");

	return bRet;
}

/**
 * Update map value at specified key
 * @param seqno		:[in] sequence no
 * @param reqData	:[in] request data
 * @return 	true : on success,
 * 			false : on error
 */
bool common_Handler::updateReqData(unsigned short seqno, MbusAPI_t& reqData)
{
	DO_LOG_DEBUG("Start: ");
	bool bRet = true;
	try
	{
		std::unique_lock<std::mutex> lck(__appReqJsonLock);

		/// insert the data
		g_mapRequest[seqno] = reqData;
	}
	catch (exception &e)
	{
		DO_LOG_FATAL(e.what());
		bRet = false;
	}
	DO_LOG_DEBUG("End: ");

	return bRet;
}

/**
 * Remove request data from map
 * @param seqno	:[in] sequence no of request to remove
 */
void common_Handler::removeReqData(unsigned short seqno)
{
	//DO_LOG_DEBUG("Start: " + std::to_string(seqno));
	std::unique_lock<std::mutex> lck(__appReqJsonLock);
	g_mapRequest.erase(seqno);
	//DO_LOG_DEBUG("End: ");
}

/**
 * get request priority from global configuration depending on the operation priority
 * @param a_OpsInfo		:[in] global config for which to retrieve operation priority
 * @return [long]		:[out] request priority to be sent to stack.(lower is the value higher is the priority)
 */
long common_Handler::getReqPriority(const globalConfig::COperation a_Ops)
{
	long iReqPriority = 0;

	// get the request priority based on operation priority
	switch (a_Ops.getOperationPriority())
	{
		case 1:
		{
			iReqPriority = 6000;
		}
		break;
		case 2:
		{
			iReqPriority = 5000;
		}
		break;
		case 3:
		{
			iReqPriority = 4000;
		}
		break;
		case 4:
		{
			iReqPriority = 3000;
		}
		break;
		case 5:
		{
			iReqPriority = 2000;
		}
		break;
		case 6:
		{
			iReqPriority = 1000;
		}
		break;
		default:
		{
			DO_LOG_ERROR("Invalid operation priority received..setting it to 6000 default");
			iReqPriority = 6000;
		}
		break;
	}

	return iReqPriority;
}
