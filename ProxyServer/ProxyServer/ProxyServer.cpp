﻿// ProxyServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ProxyServer.h"

WCHAR* stringToWCHAR(string s) {
	WCHAR* a = new WCHAR[s.length()];
	for (int i = 0; i < s.length(); i++)
		a[i] = s[i];
	a[s.length()] = '\0';
	return a;
}

string message_403() {
	string body;
	body = body +
		"<html>\r\n" +
		"<head><title>403 Forbidden</title><head>\r\n"
		"<body>\r\n" +
		"<h1> 403 Forbidden</h1>\r\n" +
		"<p>You don't have permission to access / on this server.</p>\r\n" +
		"</body>\r\n" +
		"</html>\r\n";

	stringstream convert;
	convert << body.length();
	string strlen_body = convert.str();

	string header;
	header = header +
		"HTTP/1.0 403 Forbidden\r\n" +
		"Content-type: text/html\r\n" +
		"Connection: close\r\n" +
		"Content-Length: ";

	string message = header + strlen_body + "\r\n\r\n" + body;
	return message;
}

bool goodWeb(string host, fstream &blackList) {
	string list;
	blackList.seekg(0);

	while (!blackList.eof()) {
		getline(blackList, list);

		if (list == host)
			return false;
		string test;
		if (host.length() > list.length() && list == test.append(host, host.length() - list.length(), list.length()) && host[host.length() - list.length() - 1] == '.')
			return false;
	}
	return true;
}

DWORD WINAPI UserToProxyThread(LPVOID m_SocketPair)
{
	SocketPair* socketPair = (SocketPair*)m_SocketPair;
	socketPair->Proxy.Attach(socketPair->Socket);

	char message[1000];

	socketPair->Proxy.Receive(message, 1000);

	stringstream s;
	string hostTemp;
	s << message;
	s >> hostTemp;
	s >> hostTemp;
	string host;
	if (hostTemp.find("http://") != -1)
		host.append(hostTemp, 7, hostTemp.find('/', 7) - 7);
	else {
		socketPair->Proxy.Close();
		socketPair->Client.Close();
		socketPair->blackList.close();
		delete socketPair;
		return 1;
	}
		
	// -----------------Chặn trang
	if (!goodWeb(host, socketPair->blackList))
	{
		string error403 = message_403();
		socketPair->Proxy.Send(error403.c_str(), error403.length());
		socketPair->Proxy.Close();
		socketPair->Client.Close();
		socketPair->blackList.close();
		delete socketPair;
		return 1;
	}

	socketPair->Client.Create();
	socketPair->Client.Connect(stringToWCHAR(host), 80);

	string bufferSender(message);

	bufferSender.erase(bufferSender.begin() + 4, bufferSender.begin() + 4 + 7 + host.length());
	//bufferSender = bufferSender.substr(0, bufferSender.find(" (")) + "\r\n\r\n";
	socketPair->Client.Send(bufferSender.c_str(), bufferSender.length());

	while (1)
	{
		char bufferReceiver[513];
		memset(bufferReceiver, 0, 513);

		int flag = socketPair->Client.Receive(bufferReceiver, 513);
		if (flag == 0 || flag == -1)
			break;

		socketPair->Proxy.Send(bufferReceiver, flag);
	}
	socketPair->Proxy.Close();
	socketPair->Client.Close();
	socketPair->blackList.close();
	delete socketPair;
	return 1;
}

int main()
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
			if (AfxSocketInit() == FALSE)
			{
				cout << "Socket Library initialization failed" << endl;
				return -1;
			}

			CSocket Server;

			if (Server.Create(8888) == 0)
			{
				cout << "Server initialization failed !!!" << endl;
				cout << Server.GetLastError();
				return -1;
			}

			cout << "Server initialization successful !!!" << endl;
			if (Server.Listen(1) == FALSE)
			{
				cout << "Server cannot listen !!!" << endl;
				Server.Close();
				return FALSE;
			}

			while (1) {
				SocketPair* socketPair = new SocketPair;
				socketPair->blackList.open(BLACKLIST, ios::in);
				if (!socketPair->blackList.good()) return 0;
				Server.Accept(socketPair->Proxy);

				socketPair->Socket = socketPair->Proxy.Detach();
				CreateThread(0, 0, UserToProxyThread, (LPVOID)socketPair, 0, 0);
			}
			
			Server.Close();
		}
	}
    return 0;
}

