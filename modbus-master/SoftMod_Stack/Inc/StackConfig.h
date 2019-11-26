/*************************************************************************
*                   Copyright (c) by Softdel Systems              
*                                                                       
*   This software is copyrighted by and is the sole property of Softdel
*   Systems. All rights, title, ownership, or other interests in the
*   software remain the property of Softdel Systems. This software
*   may only be used in accordance with the corresponding license
*   agreement. Any unauthorized use, duplication, transmission,
*   distribution, or disclosure of this software is expressly forbidden. 
*                                                                       
*   This Copyright notice may not be removed or modified without prior   
*   written consent of Softdel Systems.                               
*                                                                       
*   Softdel Systems reserves the right to modify this software       
*   without notice.                                                      
*************************************************************************/

#ifndef STACKCONFIG_H_
#define STACKCONFIG_H_

#include "osalLinux.h"

#define TCP_MODBUS_ADU_LENGTH 260
#define FILE_RECORD_REFERENCE_TYPE 6
#define MODBUS_DATA_LENGTH (260)
#define MAX_ALLOWED_SLAVES	(247)
#define MODBUS_TCP_PORT 	502
#define MAXIMUM_TCP_CONNECTION 	32

/// Starting Addresses for coils and registers
#define SERIES_COIL_STATUS_ADDR		(00001)
#define SERIES_READ_INPUT_ADDR		(10001)
#define SERIES_INPUT_REG_ADDR		(30001)
#define SERIES_HOLDING_REG_ADDR		(40001)

#define APP_DATA_TIMEOUT	(250)

/// Validation limits for stack
#define MIN_VALUE			(0x0000)
#define MIN_COILS			(1)
#define MAX_COILS			(2000)
#define MAX_COIL_REGISTER	(65535)

#define MIN_INPUT_REGISTER	(1)
#define MAX_INPUT_REGISTER	(125)
#define MIN_MULTI_REGISTER	(1)
#define MAX_MULTI_REGISTER	(123)

#define MIN_MULTI_COIL		(1)
#define MAX_MULTI_COIL		(1968)
#define MAX_STARTING_ADDRESS (65535)
#define COIL_ON			(0xFF00)
#define COIL_OFF		(0x0000)
#define MAX_VALUE		(65535)
#define MAX_REGISTERS	(200)

#define MIN_HOLDING_REGISTERS (1)
#define MAX_HOLDING_REGISTERS (125)
#define MIN_INPUT			  (1)
#define MAX_INPUT			(65535)

#define MIN_MUL_WRITE_REG	(1)
#define MAX_MUL_WRITE_REG	(121)

#define MIN_FILE_BYTE_COUNT		(7)
#define MAX_FILE_BYTE_COUNT	(245)

/// Application Address
/// To Do : Need to update as per application requirements
#define MAX_COIL_ADDR			(65535)
#define MAX_INPUT_REGISTER_ADDR (65535)

#define MAX_BITS		(8)
#define VALUE_ZERO		(0x00)
#define MAX_BYTE	(0xFF)
#define MEI_TYPE	(14)

/**
 @struct MbusTXData_t
 @brief
    This structure defines Modbus transmit data
*/
typedef struct
{
	/// Holds the data field
	uint8_t m_au8DataFields[ MODBUS_DATA_LENGTH ];
	/// Holds the received length of packet
	uint16_t m_u16Length;
}MbusTXData_t;

/**
 @struct MbusRXData_t
 @brief
    This structure defines Modbus receive data
*/
typedef struct
{
	/// Holds the data field
	uint8_t m_au8DataFields[ MODBUS_DATA_LENGTH ];
	/// Holds the received length of packet
	uint8_t m_u8Length;

	void* m_pvAdditionalData;
}MbusRXData_t;

/**
*
* @struct  - stMbusPacketVariables_t
*
* DESCRIPTION
* This structure defines the Receive queue along with block
* size, fill & process index. It also defines the mutex
* lock required to synchronize the queue.
*
*/
typedef struct _stMbusPacketVariables
{
	/** Holds the received transaction ID*/
	uint16_t m_u16TransactionID;
	/** Holds the unit id  */
	uint8_t  m_u8UnitID;
	/** Holds Ip address of salve/server device */
	uint8_t m_u8IpAddr[4];
	/** Holds the unit id  */
	uint8_t m_u8FunctionCode;
	/** Holds Data to be send to server */
	MbusTXData_t m_stMbusTxData;
	/** Holds Data received from server */
	MbusRXData_t m_stMbusRxData;
	/** Holds Status of command success or failure
	 * after ack received from server or timeout */
	uint8_t m_u8CommandStatus;
	/** Holds the start address  */
	uint16_t  m_u16StartAdd;
	/** Holds the Quantity  */
	uint16_t  m_u16Quantity;
	void *pFunc;

}stMbusPacketVariables_t;

/**
 @struct IP_address
 @brief
    This structure defines IP address parameters
*/
typedef struct IP_address
{
	union
	{
		struct{
			unsigned char IP_1;
			unsigned char IP_2;
			unsigned char IP_3;
			unsigned char IP_4;
		}s_un_b;
		uint32_t s_addr;
	}s_un;
}IP_address_t;

/**
 @union uByteOrder
 @brief
    This structure defines byte structure
*/
typedef union uByteOrder
{
	struct
	{
		unsigned char u8ByteOne;
		unsigned char u8ByteTwo;
	}TwoByte;
	uint16_t u16Word;
}uByteOrder_t;

/**
 @union stEndianess
 @brief
    This structure defines endian ness structure
*/
typedef union stEndianess
{
	struct
	{
		uint8_t u8FirstByte;
		uint8_t u8SecondByte;
	}stByteOrder;
	uint16_t u16word;
}stEndianess_t;

/**
 @enum eMbusIndex_enum
 @brief
    This structure defines modbus index
*/
typedef enum
{
	MBUS_INDEX_0,
	MBUS_INDEX_1,
	MBUS_INDEX_2,
	MBUS_INDEX_3,
	MBUS_INDEX_4,
	MBUS_INDEX_5,
	MBUS_INDEX_6,
	MBUS_INDEX_7,
	MBUS_INDEX_8,
	MBUS_INDEX_9,
	MBUS_INDEX_10
}eMbusIndex_enum;

/// Application callback handler
void ApplicationCallBackHandler(stMbusPacketVariables_t *pstMBusRequesPacket,
		eStackErrorCode eMbusStackErr);

/// Function to send packet on network
uint8_t Modbus_SendPacket(stMbusPacketVariables_t *pstMBusRequesPacket,
		int32_t* pi32sockfd);


#endif /* STACKCONFIG_H_ */
