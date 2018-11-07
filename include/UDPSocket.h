/*
 * UDPSocket.h
 *
 *  Created on: May 16, 2017
 *      Author: nawrin
 */

#ifndef UDPSOCKET_H_
#define UDPSOCKET_H_

#include <string>
#include <stdint.h>

//using namespace std;

class UDPSocket
{
public:
	UDPSocket();
	UDPSocket(uint16_t port);
	~UDPSocket();
	void send(const void *buf, int length, const std::string &address,
	          uint16_t port);
	int recv(void *buf, int length, std::string &sourceAddress,
	         uint16_t &sourcePort);
private:
	int sockfd;
};


#endif /* UDPSOCKET_H_ */
