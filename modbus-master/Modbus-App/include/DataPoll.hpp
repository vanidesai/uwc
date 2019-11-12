/*
 * DataPoll.hpp
 *
 *  Created on: 05-Nov-2019
 *      Author: user
 */

#ifndef INCLUDE_DATAPOLL_HPP_
#define INCLUDE_DATAPOLL_HPP_

#include "Common.hpp"
#include "PeriodicRead.hpp"


class PeriodicDataPoll
{
public:
	PeriodicDataPoll(){};
	~PeriodicDataPoll(){};

	bool initDataPoll();
	eMbusStackErrorCode filljsontoReq(RestRdPeriodicTagPart_t *pstModbusRxPacket);
};


#endif /* INCLUDE_DATAPOLL_HPP_ */
