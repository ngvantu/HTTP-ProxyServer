// ProxyServer.cpp : Defines the entry point for the console application.
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
		if (list == test.append(host, host.length() - list.length(), list.length()) && host[host.length() - list.length() - 1] == '.')
			return false;
	}
	return true;
}

DWORD WINAPI UserToProxyThread(LPVOID m_SocketPair)
{
	SocketPair* socketPair = (SocketPair*)m_SocketPair;
	socketPair->Client.Attach(socketPair->Proxy);

	char msg[513];

	socketPair->Client.Receive(msg, 512);

	stringstream s;
	string hostTemp;
	s << msg;
	s >> hostTemp;
	s >> hostTemp;
	string host;
	if (hostTemp.find(HTTP) != -1)
		host.append(hostTemp, 7, hostTemp.find('/', 7) - 7);
	else
		return 1;

	// -----------------Chặn trang
	if (!goodWeb(host, socketPair->blackList))
	{
		string chuoicangui = message_403();
		socketPair->Client.Send(chuoicangui.c_str(), chuoicangui.length()); // gửi thông báo 403 Forbidden
		socketPair->Client.Close();
		socketPair->blackList.close();

		return 1;
	}

	WCHAR *wchar_host = NULL;
	wchar_host = stringToWCHAR(host);
	CSocket client_that;
	client_that.Create();
	client_that.Connect(wchar_host, 80);

	string chuoicangui1(msg);
	string chuoicangui;
	if (hostTemp.find("http://") != -1) {
		chuoicangui.append(chuoicangui1, 0, chuoicangui1.find("HTTP"));
		chuoicangui.erase(4, 7 + host.length());
		//if (chuoicangui == "GET / ") 
		//chuoicangui = "GET ";
		chuoicangui = chuoicangui + "HTTP/1.1\r\nHost: " + host + "\r\nUser-Agent: " + USERAGENT + "\r\n\r\n";
		chuoicangui.push_back(0);
	}


	client_that.Send(chuoicangui.c_str(), chuoicangui.length());

	//Nhan tra ve cua chuoi tu trang web muon truy cap
	while (1)
	{
		char chuoinhan[513];
		memset(chuoinhan, 0, 512);

		int flag = client_that.Receive(chuoinhan, 512);
		if (flag == 0 || flag == -1)
			break;

		socketPair->Client.Send(chuoinhan, flag);
	}
	socketPair->Client.Close();
	client_that.Close();
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
				cout << "Khong the khoi tao Socket Libraray";
				return -1;
			}

			CSocket Server;

			if (Server.Create(8888) == 0)
			{
				cout << "Khoi tao that bai !!!" << endl;
				cout << Server.GetLastError();
				return -1;
			}

			cout << "Server khoi tao thanh cong !!!" << endl;
			if (Server.Listen(1) == FALSE)
			{
				cout << "Khong the lang nghe tren port nay !!!" << endl;
				Server.Close();
				return FALSE;
			}

			vector<SocketPair*> vSocketPair;
			while (1) {
				vSocketPair.push_back(new SocketPair);
				vSocketPair[vSocketPair.size() - 1]->blackList.open(BLACKLIST, ios::in);
				cout << vSocketPair.size(); // ghi ra chơi cho biết 
				Server.Accept(vSocketPair[vSocketPair.size() - 1]->Client);

				vSocketPair[vSocketPair.size() - 1]->Proxy = vSocketPair[vSocketPair.size() - 1]->Client.Detach();
				CreateThread(0, 0, UserToProxyThread, (LPVOID)vSocketPair[vSocketPair.size() - 1], 0, 0);
			}
		}
	}
    return 0;
}

