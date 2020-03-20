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
	std::map<unsigned short, stOnDemandRequest> g_mapAppSeq;
}

std::mutex __appSeqMapLock;


/**
 * Get time parameters
 * @param a_sTimeStamp	:[in] reference to store time stamp
 * @param a_sUsec		:[in] reference to store time in usec
 */
void common_Handler::getTimeParams(std::string &a_sTimeStamp, std::string &a_sUsec)
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
}

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
 * Get on-demand request data
 * @param seqno		:[in] sequence no
 * @param reqData	:[out] reference to store request data
 * @return 	true : on success,
 * 			false : on error
 */
bool common_Handler::getOnDemandReqData(unsigned short seqno, stOnDemandRequest& reqData)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start: " + std::to_string(seqno)));
	bool bRet = true;
	try
	{
		std::unique_lock<std::mutex> lck(__appSeqMapLock);

		/// return the context
		reqData = g_mapAppSeq.at(seqno);
	}
	catch(exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		bRet = false;
	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End: "));

	return bRet;
}

/**
 * Insert on-demand request data
 * @param seqno		:[in] sequence no
 * @param reqData	:[in] request data
 * @return 	true : on success,
 * 			false : on error
 */
bool common_Handler::insertOnDemandReqData(unsigned short seqno, stOnDemandRequest reqData)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start: "));
	bool bRet = true;
	try
	{
		std::unique_lock<std::mutex> lck(__appSeqMapLock);

		/// insert the data
		g_mapAppSeq.insert(std::pair <unsigned short, stOnDemandRequest> (seqno, reqData));
	}
	catch (exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		bRet = false;
	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End: "));

	return bRet;
}

/**
 * Remove on-demand request data
 * @param seqno	:[in] sequence no of request to remove
 */
void common_Handler::removeOnDemandReqData(unsigned short seqno)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start: " + std::to_string(seqno)));
	std::unique_lock<std::mutex> lck(__appSeqMapLock);
	g_mapAppSeq.erase(seqno);
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End: "));
}



