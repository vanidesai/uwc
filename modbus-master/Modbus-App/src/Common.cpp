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
	catch(std::exception &e)
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
	bool bRet = true;
	try
	{
		std::unique_lock<std::mutex> lck(__appReqJsonLock);

		/// insert the data
		g_mapRequest.insert(std::pair <unsigned short, MbusAPI_t> (seqno, reqData));
	}
	catch (std::exception &e)
	{
		DO_LOG_FATAL(e.what());
		bRet = false;
	}

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
	catch (std::exception &e)
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
	std::unique_lock<std::mutex> lck(__appReqJsonLock);
	g_mapRequest.erase(seqno);
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

/**
 * Hex byte to float conversion
 * @param str		:Hex strng
 * @return float: hex string conversion value
 */
float common_Handler::hexBytesToFloat(std::string str)
{
	std::cout<<"common_Handler::hexBytesToFloat" <<std::endl;
	float ffloatData;
	//char *ed;
	unsigned long long lData = stoull(str, NULL, 16);
	std::cout<<"common_Handler::hexBytesToFloat data" << lData << std::endl;
	ffloatData= *reinterpret_cast<float*>(&lData);
	std::cout<<"common_Handler::hexBytesToFloat Fdata" << ffloatData << std::endl;

	return ffloatData;
}

/**
 * Hex byte to unsigned short int conversion
 * @param str		:Hex strng
 * @return UShortInt: hex string conversion value
 */
unsigned short int common_Handler::hexBytesToUShortInt(std::string str)
{
	unsigned short int iUShortIntData;
	iUShortIntData = static_cast<unsigned short int>(stoi(str, 0, 16));
	return iUShortIntData;
}

/**
 * Hex byte to int conversion
 * @param str		:Hex strng
 * @return shortInt: hex string conversion value
 */
short int common_Handler::hexBytesToShortInt(std::string str)
{
	short int  iShortIntData;
	iShortIntData = static_cast<short int>(stoi(str, nullptr, 16));
	return iShortIntData;
}

/**
 * Hex byte to bool conversion
 * @param str		:Hex strng
 * @return bool: hex string conversion value
 */
bool common_Handler::hexBytesToBool(std::string str)
{
	bool bData  = static_cast<bool>(stoi(str, nullptr, 16));
	return bData;
	std::cout<<"Invalid hex bytes";
}

/**
 * Hex byte to std::string conversion
 * @param str		:Hex strng
 * @return string: hex string conversion value
 */
std::string common_Handler::hexBytesToString(std::string str)
{
	std::string sData = "";
	for (size_t i = 2; i < str.length(); i += 2)
	{
		// extract two characters from hex std::string
		std::string part = str.substr(i, 2);

		// change it into base 16 and
		// typecast as the character
		char ch = stoul(part, nullptr, 16);

		// add this char to final ASCII std::string
		sData += ch;
	}
	return sData;
}

/**
 * Hex byte to int conversion
 * @param str		:Hex strng
 * @return intData: hex string conversion value
 */
int common_Handler::hexBytesToInt(std::string str)
{
	int iData;
	iData = static_cast<int>(stoi(str, nullptr, 16));
	return iData;
}

/**
 * Hex byte to unsigned int conversion
 * @param str		:Hex strng
 * @return unsigned int: hex string conversion value
 */
unsigned int common_Handler::hexBytesToUnsignedInt(std::string str)
{
	unsigned int iUIntData;
	iUIntData = static_cast<unsigned int>(stoi(str, nullptr, 16));
	return iUIntData;
}

/**
 * Hex byte to long int conversion
 * @param str		:Hex strng
 * @return long int: hex string conversion value
 */
long int common_Handler::hexBytesToLongInt(std::string str)
{
	long int iLIntData;
	iLIntData = static_cast<long int>(stoul(str, nullptr, 16));
	return iLIntData;
}

/**
 * Hex byte to unsigned long int conversion
 * @param str		:Hex strng
 * @return unsigned long int: hex string conversion value
 */
unsigned long int common_Handler::hexBytesToUnsignedLongInt(std::string str)
{
	unsigned long int iULIntData;
	iULIntData = static_cast<unsigned long int>(stoul(str, nullptr, 16));
	return iULIntData;
}

/**
 * Hex byte to long long int conversion
 * @param str		:Hex strng
 * @return long long int: hex string conversion value
 */
long long int common_Handler::hexBytesToLongLongInt(std::string str)
{
	long long int iLLIntData;
	iLLIntData = static_cast<long long int>(stoull(str, nullptr, 16));
	return iLLIntData;

}

/**
 * Hex byte to unsigned long long int conversion
 * @param str		:Hex strng
 * @return unsigned long long int: hex string conversion value
 */
unsigned long long int common_Handler::hexBytesToUnsignedLongLongInt(std::string str)
{
	unsigned long long int iULLIntData;
	iULLIntData = static_cast<unsigned long long int>(stoull(str, nullptr, 16));
	return iULLIntData;
}

/**
 * Hex byte to double conversion
 * @param str		:Hex strng
 * @return double: hex string conversion value
 */
double common_Handler::hexBytesToDouble(std::string str)
{
	double dData;
	std::cout<<"common_Handler::hexBytesTodouble" <<std::endl;
	unsigned long long lData = stoull(str, NULL, 16);
	std::cout<<"common_Handler::hexBytesTodoubledta" << lData <<std::endl;
	dData= *reinterpret_cast<double*>(&lData);
	std::cout<<"common_Handler::hexBytesTodoubledta casting" << dData <<std::endl;
	return dData;
}

/**
 * Hex byte to long double conversion
 * @param str		:Hex strng
 * @return long double: hex string conversion value
 */
long double common_Handler::hexBytesToLongDouble(std::string str)
{
	long double dLDoubleData;
	unsigned long long lData = stoull(str, NULL, 16);
	dLDoubleData= *reinterpret_cast<long double*>(&lData);
	return dLDoubleData;

}
