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
 * This Function converts hex string to float.
 * @param str: [in] Hex string
 * @return float: [out] converted float value from hex string.
 */
float common_Handler::hexBytesToFloat(std::string str)
{
	float ffloatData = 0;
	try
	{
		long long int lData = stoull(str, NULL, 16);
		hexStrToFlt ohexStrToFlt;
		ohexStrToFlt.hexValue = lData;		
		ffloatData = ohexStrToFlt.actualFltVal;
	}
	catch(const std::out_of_range& oor)
	{
	  DO_LOG_FATAL(oor.what());
	}
	catch(const std::invalid_argument& ia)
	{
      DO_LOG_FATAL(ia.what());
	}
	catch(const std::exception& e)
	{
	  DO_LOG_FATAL(LOGDETAILS(e.what()));
	}
	return ffloatData;
}

/**
 * This function converts hex bytes to unsigned short int. 
 * @param str				  :[in] Hex strng
 * @return unsigned short int :[out]converted unsigned short int from hex string
 */
unsigned short int common_Handler::hexBytesToUShortInt(std::string str)
{
	unsigned short int iUShortIntData = 0;
	try
	{
	  iUShortIntData = static_cast<unsigned short int>(stoi(str, 0, 16));
	}
	catch(const std::out_of_range& oor)
	{
	  DO_LOG_FATAL(oor.what());
	}
	catch(const std::invalid_argument& ia)
	{
      DO_LOG_FATAL(ia.what());
	}
	catch(const std::exception& e)
	{
	  DO_LOG_FATAL(LOGDETAILS(e.what()));
	}
	return iUShortIntData;
}

/**
 * This function converts hex bytes to short int. 
 * @param str		  :[in] Hex string
 * @return short int  :[out]converted short int from hex string
 */
short int common_Handler::hexBytesToShortInt(std::string str)
{
	short int  iShortIntData = 0;
	try
	{
	  iShortIntData = static_cast<short int>(stoi(str, nullptr, 16));
	}
	catch(const std::out_of_range& oor)
	{
	  DO_LOG_FATAL(oor.what());
	}
	catch(const std::invalid_argument& ia)
	{
      DO_LOG_FATAL(ia.what());
	}
	catch(const std::exception& e)
	{
	  DO_LOG_FATAL(LOGDETAILS(e.what()));
	}
	return iShortIntData;
}

/**
 * This function converts hex bytes to boolean. 
 * @param str		  :[in] Hex string
 * @return boolean    :[out]converted boolean from hex string
 */
bool common_Handler::hexBytesToBool(std::string str)
{
	bool bData = 0;
	try
	{
	  bData  = static_cast<bool>(stoi(str, nullptr, 16));
	}
	catch(const std::out_of_range& oor)
	{
	  DO_LOG_FATAL(oor.what());
	}
	catch(const std::invalid_argument& ia)
	{
      DO_LOG_FATAL(ia.what());
	}
	catch(const std::exception& e)
	{
	  DO_LOG_FATAL(LOGDETAILS(e.what()));
	}
	return bData;	
}

/**
 * This function converts hex bytes to string. 
 * @param str		  :[in] Hex string
 * @return string     :[out]converted string from hex string
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
		try
		{
		  char ch = stoul(part, nullptr, 16);
		  sData += ch;
		}
        catch(const std::out_of_range& oor)
        {
          DO_LOG_FATAL(oor.what());
        } 
        catch(const std::invalid_argument& ia)
        {
          DO_LOG_FATAL(ia.what());
        }
        catch(const std::exception& e)
        {
          DO_LOG_FATAL(LOGDETAILS(e.what()));
        }		
	}
	return sData;
}

/**
 * This function converts hex bytes to int. 
 * @param str		  :[in] Hex string
 * @return int        :[out]converted int from hex string
 */
int common_Handler::hexBytesToInt(std::string str)
{
	int iData = 0;
	try
	{
	  iData = static_cast<long int>(stoll(str, nullptr, 16));
	}
	catch(const std::out_of_range& oor)
	{
	  DO_LOG_FATAL(oor.what());
	}
	catch(const std::invalid_argument& ia)
	{
      DO_LOG_FATAL(ia.what());
	}
	catch(const std::exception& e)
	{
	  DO_LOG_FATAL(LOGDETAILS(e.what()));
	}
	return iData;
}

/**
 * This function converts hex bytes to unsigned int. 
 * @param str		  		:[in] Hex string
 * @return unsigned int     :[out]converted unsigned int from hex string
 */
unsigned int common_Handler::hexBytesToUnsignedInt(std::string str)
{
	unsigned int iUIntData = 0;
	try
	{
	  iUIntData = static_cast<unsigned int>(stoul(str, nullptr, 16));
	}
	catch(const std::out_of_range& oor)
	{
	  DO_LOG_FATAL(oor.what());
	}
	catch(const std::invalid_argument& ia)
	{
          DO_LOG_FATAL(ia.what());
	}
	catch(const std::exception& e)
	{
	  DO_LOG_FATAL(LOGDETAILS(e.what()));
	}
	return iUIntData;
}

/**
 * This function converts hex bytes to long int. 
 * @param str		  		:[in] Hex string
 * @return long int         :[out]converted long int from hex string
 */
long int common_Handler::hexBytesToLongInt(std::string str)
{
	long int iLIntData = 0;
	try
	{
	  iLIntData = static_cast<long int>(stol(str, nullptr, 16));
	}
	catch(const std::out_of_range& oor)
	{
	  DO_LOG_FATAL(oor.what());
	}
	catch(const std::invalid_argument& ia)
	{
      DO_LOG_FATAL(ia.what());
	}
	catch(const std::exception& e)
	{
	  DO_LOG_FATAL(LOGDETAILS(e.what()));
	}	
	return iLIntData;
}

/**
 * This function converts hex bytes to unsigned long int. 
 * @param str		  		   :[in] Hex string
 * @return unsigned long int   :[out] Converted unsigned long int from hex string
 */
unsigned long int common_Handler::hexBytesToUnsignedLongInt(std::string str)
{
	unsigned long int iULIntData = 0;
	try
	{
	  iULIntData = static_cast<unsigned long int>(stoul(str, nullptr, 16));
	}
	catch(const std::out_of_range& oor)
	{
	  DO_LOG_FATAL(oor.what());
	}
	catch(const std::invalid_argument& ia)
	{
      DO_LOG_FATAL(ia.what());
	}
	catch(const std::exception& e)
	{
	  DO_LOG_FATAL(LOGDETAILS(e.what()));
	}
	return iULIntData;
}

/**
 * This function converts hex bytes to long long int. 
 * @param str		      :[in] Hex string
 * @return long long int  :[out] Converted long long int from hex string
 */
long long int common_Handler::hexBytesToLongLongInt(std::string str)
{
	long long int iLLIntData = 0;
	try
	{
	  iLLIntData = static_cast<long long int>(stoull(str, nullptr, 16));
	}
	catch(const std::out_of_range& oor)
	{
	  DO_LOG_FATAL(oor.what());
	}
	catch(const std::invalid_argument& ia)
	{
      DO_LOG_FATAL(ia.what());
	}
	catch(const std::exception& e)
	{
	  DO_LOG_FATAL(LOGDETAILS(e.what()));
	}
	return iLLIntData;
}

/**
 * This function converts hex bytes to unsigned long long int. 
 * @param str		               :[in] Hex string
 * @return unsigned long long int  :[out] Converted unsigned long long int from hex string
 */
unsigned long long int common_Handler::hexBytesToUnsignedLongLongInt(std::string str)
{
	unsigned long long int iULLIntData = 0;
    try
	{
	  iULLIntData = static_cast<unsigned long long int>(stoull(str, nullptr, 16));
	}
	catch(const std::out_of_range& oor)
	{
	  DO_LOG_FATAL(oor.what());
	}
	catch(const std::invalid_argument& ia)
	{
      DO_LOG_FATAL(ia.what());
	}
	catch(const std::exception& e)
	{
	  DO_LOG_FATAL(LOGDETAILS(e.what()));
	}
	return iULLIntData;
}

/**
 * This function converts hex bytes to double.
 * @param str		  :[in] Hex string
 * @return double     :[out] Converted double from hex string
 */
double common_Handler::hexBytesToDouble(std::string str)
{
	double dData = 0;
	try
	{    
	    unsigned long long int lData = stoull(str, NULL, 16); 	    
		hexStrToDbl ohexStrToDbl;
		ohexStrToDbl.hexValue = lData;		
	    dData = ohexStrToDbl.actualDblVal;	  
	}
	catch(const std::out_of_range& oor)
	{
	  DO_LOG_FATAL(oor.what());
	}
	catch(const std::invalid_argument& ia)
	{
          DO_LOG_FATAL(ia.what());
	}
	catch(const std::exception& e)
	{
	  DO_LOG_FATAL(LOGDETAILS(e.what()));
	}
	return dData;
}

/**
 * This function gets the enumerated datatype
 * @param a_sDataType	:[in]  Datatype received from datapoints.yml
 * @return 	enum datatype.
 */
eYMlDataType common_Handler::getDataType(std::string a_sDataType)
{
	if (! a_sDataType.compare("boolean"))
	{
		return enBOOLEAN;
	}
	else if ((! a_sDataType.compare("uint"))   ||
	         (! a_sDataType.compare("uint16")) ||
	         (! a_sDataType.compare("uint32")) ||
	         (! a_sDataType.compare("uint64")))
	{
		return enUINT;
	}
	else if ((! a_sDataType.compare("int"))   ||
	         (! a_sDataType.compare("int16")) ||
	         (! a_sDataType.compare("int32")) ||
	         (! a_sDataType.compare("int64")))
	{
		return enINT;
	}
	else if (! a_sDataType.compare("float"))
	{
		return enFLOAT;
	}
	else if (! a_sDataType.compare("double"))
	{
		return enDOUBLE;
	}
	else if (! a_sDataType.compare("string"))
	{
		return enSTRING;
	}
	else
	{
		return enUNKNOWN;
	}

}

