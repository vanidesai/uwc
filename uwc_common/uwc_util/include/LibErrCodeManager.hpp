/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

/***LibErrCodeManager.hpp holds the information for error code of library*/

#ifndef INCLUDE_INC_LIBERRCODE_HPP_
#define INCLUDE_INC_LIBERRCODE_HPP_

/** This enumerator defines modbus app error codes */
enum eUWCLibErrorCode
{
	UWCLIB_SUCCESS = 0,
	UWCLIB_NULL_ARG = 200,
	UWCLIB_EXEPTION = 201,
	UWCLIB_INVLID_TOPIC_TYPE = 202,
	UWCLIB_ERR_APP_DATA_NOT_SET = 300,
	UWCLIB_ERR_CFGMNGR_CLIENT_CR = 301,
	UWCLIB_ERR_CFGMNGRENV_CLIENT_CR = 302,
	UWCLIB_ERR_INV_TOPIC_TYPE = 303,
	UWCLIB_ERR_GET_CTX_ERR = 304,
	UWCLIB_ERR_NEWPUBCTX = 305,
	UWCLIB_ERR_INSRT_CTX_ERR = 306,
	UWCLIB_ERR_REM_CTX_ERR = 307,
	UWCLIB_ERR_PREPARE_CTX_ERR = 308,
	UWCLIB_CTX_NOT_FOUND_IN_MAP = 309,
	UWCLIB_ERR_INVALID_CTX = 310,

};


#endif //INCLUDE_INC_LIBERRCODE_HPP_
