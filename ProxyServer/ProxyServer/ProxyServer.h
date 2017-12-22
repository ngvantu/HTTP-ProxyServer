#pragma once

#include "afxsock.h"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

using namespace std;

#define BLACKLIST "blacklist.conf"

class SocketPair
{
public:
	CSocket Client;
	CSocket Browser;
	SOCKET Socket;
	fstream blackList;

	SocketPair() {};
	~SocketPair() {};
};