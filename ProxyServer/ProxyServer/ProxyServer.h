#pragma once

#include "afxsock.h"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

using namespace std;

#define BLACKLIST "blacklist.conf"

class SocketPair
{
public:
	CSocket Client;
	CSocket Proxy;
	SOCKET Socket;
	fstream blackList;

	SocketPair() {};
	~SocketPair() {};
};