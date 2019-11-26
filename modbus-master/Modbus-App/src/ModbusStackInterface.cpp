/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#include "BoostLogger.hpp"
#include "Common.hpp"

extern "C" {
	#include <safe_lib.h>
}

extern src::severity_logger< severity_level > lg;

/**
 * Description
 * function to call Modbus stack APIs
 *
 * @param u8FunCode 		[in] function code
 * @param pstMbusApiPram 	[in] Input request packet
 * @param vpCallBackFun		[in] callback function pointer
 *
 * @return uint8_t			[out] return 0 on success
 *
 */
uint8_t Modbus_Stack_API_Call(unsigned char u8FunCode, MbusAPI_t *pstMbusApiPram,
					void* vpCallBackFun)
{
	BOOST_LOG_SEV(lg, debug) << __func__ << "Start: Function Code: " << u8FunCode;
	uint8_t u8ReturnType = MBUS_JSON_APP_ERROR_NULL_POINTER;

	if(pstMbusApiPram != NULL)
	{
		switch ((eModbusFuncCode_enum)u8FunCode)
		{
			case READ_COIL_STATUS:
				BOOST_LOG_SEV(lg, info) << __func__ << "Read Coil Request Received";
				u8ReturnType = Modbus_Read_Coils(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					vpCallBackFun);
			break;
			case READ_INPUT_STATUS:
				BOOST_LOG_SEV(lg, info) << __func__ << "Read Input Status Request Received";
				u8ReturnType = Modbus_Read_Discrete_Inputs(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					vpCallBackFun);
			break;
			case READ_HOLDING_REG:
				BOOST_LOG_SEV(lg, info) << __func__ << "Read Holding Register Request Received";
				u8ReturnType = Modbus_Read_Holding_Registers(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					vpCallBackFun);
			break;
			case READ_INPUT_REG:
				BOOST_LOG_SEV(lg, info) << __func__ << "Read Input Register Request Received";
				u8ReturnType = Modbus_Read_Input_Registers(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					vpCallBackFun);
			break;
			case WRITE_SINGLE_COIL:
			{
				BOOST_LOG_SEV(lg, info) << __func__ << "Write Single Coil Request Received";
				uint16_t *pu16OutData = (uint16_t *)pstMbusApiPram->m_pu8Data;
				u8ReturnType = Modbus_Write_Single_Coil(pstMbusApiPram->m_u16StartAddr,
						*pu16OutData,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					vpCallBackFun);
			}
			break;

			case WRITE_SINGLE_REG:
			{
				BOOST_LOG_SEV(lg, info) << __func__ << "Write Single Register Request Received";
				uint16_t u16OutData = 0;
				memcpy_s(&u16OutData,sizeof(uint16_t),pstMbusApiPram->m_pu8Data,sizeof(uint16_t));
				u8ReturnType = Modbus_Write_Single_Register(pstMbusApiPram->m_u16StartAddr,
					*(uint16_t*)pstMbusApiPram->m_pu8Data,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					vpCallBackFun);
			}
			break;
			case WRITE_MULTIPLE_COILS:
				BOOST_LOG_SEV(lg, info) << __func__ << "Write Multiple Coils Request Received";
				u8ReturnType = Modbus_Write_Multiple_Coils(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_pu8Data,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					vpCallBackFun);
			break;
			case WRITE_MULTIPLE_REG:
				BOOST_LOG_SEV(lg, info) << __func__ << "Write Multiple Register Request Received";
				u8ReturnType = Modbus_Write_Multiple_Register(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_pu8Data,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					vpCallBackFun);
			break;

			default:
				BOOST_LOG_SEV(lg, error) << __func__ << "Invalid Request Received";
				u8ReturnType = MBUS_STACK_ERROR_INVALID_INPUT_PARAMETER;
			break;
		}
	}
	BOOST_LOG_SEV(lg, debug) << __func__ << "End: Return: " << u8ReturnType;
	return u8ReturnType;
}
