/*
 * UDPheader.c
 *
 *  Created on: May 16, 2017
 *      Author: nawrin
 */

#include <stdint.h>
#include <sys/socket.h>

#include "UDPSocket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstdlib>
#include <iostream>

//using namespace std;

//UDPSocket
UDPSocket::UDPSocket() {
	if ((sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		std::cerr << "Unable to create socket" << endl;		      
	}
}

UDPSocket::UDPSocket(uint16_t port) {

	if ((sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		std::cerr << "Unable to create socket" << endl;
	}
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (bind(sockfd, (sockaddr *) &addr, sizeof(sockaddr_in)) < 0) {
		std::cerr << "Unable to bind" << endl;
	}
}

void UDPSocket::send(const void *buf, int length, const string &address, uint16_t port) {
	sockaddr_in destAddr;
	memset(&destAddr, 0, sizeof(destAddr));
	destAddr.sin_family = AF_INET;
	destAddr.sin_port = htons(port);

	hostent *host = gethostbyname(address.c_str());
	if (host == NULL) {
		std::cerr << "Unable to resolve host" << endl;
	}
	destAddr.sin_addr.s_addr = inet_addr(address.c_str());

	if (sendto(sockfd, (void *) buf, length, 0,
	             (sockaddr *) &destAddr, sizeof(destAddr)) != length) {
		std::cerr << "Unable to send" << endl;
	}
}

int UDPSocket::recv(void *buf, int length, string &sourceAddress, uint16_t &sourcePort){
	sockaddr_in clntAddr;
	socklen_t addrLen = sizeof(clntAddr);
	int rtn;
	if ((rtn = recvfrom(sockfd, (void *) buf, length, 0,
	                      (sockaddr *) &clntAddr, (socklen_t *) &addrLen)) < 0) {
		std::cerr << "Unable to receive" << endl;
	}
	sourceAddress = inet_ntoa(clntAddr.sin_addr);
	sourcePort = ntohs(clntAddr.sin_port);

	return rtn;
}

UDPSocket::~UDPSocket() {
	close(sockfd);
}
