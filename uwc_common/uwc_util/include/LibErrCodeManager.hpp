/********************************************************************************
* Copyright (c) 2021 Intel Corporation.

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*********************************************************************************/

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
