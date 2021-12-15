// Fill out your copyright notice in the Description page of Project Settings.


#include "YServer.h"
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <shellapi.h>
#include <Windows/MinWindows.h>
#include "ServerTestCharacter.h"


#pragma comment(lib, "ws2_32")

using namespace std;

#define PACKET_SIZE 1024

SOCKET MySocket;

YServer::YServer()
{
}

YServer::~YServer()
{
	closesocket(MySocket);
	if(Recv.joinable())
		Recv.join();
	if(Recv.joinable())
		Send.join();
	WSACleanup();
}

void YServer::RecvThread()
{
	char buffer[PACKET_SIZE] = {};
	FString Data;
	while (!WSAGetLastError())
	{
		ZeroMemory(&buffer, PACKET_SIZE);
		recv(MySocket, buffer, PACKET_SIZE, 0);
		Data = buffer;
		if (Data == "OpenGoogle")
		{
			ShellExecute(0, 0, L"http://www.google.com", 0, 0, SW_SHOW);
			GEngine->AddOnScreenDebugMessage(1, 5, FColor(1, 1, 1), TEXT("Open Google!!!"));
		}
		else
			Player->RecieveServerMsg(Data);
	}
}

void YServer::SendThread()
{
	char buffer[PACKET_SIZE] = { 0 };
	while (!WSAGetLastError())
	{
		cin >> buffer;
		if (strcmp(buffer, "Quit") == 0)
		{
			break;
		}
		send(MySocket, buffer, strlen(buffer), 0);
	}
}

void YServer::SendToServer(FString Data)
{
	char buffer[PACKET_SIZE] = { 0 };
	WideCharToMultiByte(CP_ACP, 0, *Data, PACKET_SIZE, buffer, PACKET_SIZE, NULL, NULL);
	send(MySocket, buffer, strlen(buffer), 0);
}

void YServer::CreateSocket()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		return;
	}

	//MySocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	MySocket = WSASocketW(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	int option;
	option = 1;
	setsockopt(MySocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&option), sizeof(option));
	if (MySocket < 0)
	{
		return;
	}

	SOCKADDR_IN MyAddr = {};
	MyAddr.sin_family = PF_INET;
	MyAddr.sin_port = htons(6969);
	inet_pton(PF_INET, "192.168.1.87", &MyAddr.sin_addr.s_addr);

	if (connect(MySocket, reinterpret_cast<SOCKADDR*>(&MyAddr), sizeof(MyAddr)) == 0)
	{
		GEngine->AddOnScreenDebugMessage(1, 5, FColor(1, 1, 1), TEXT("Connect Success"));
	}

	Recv = thread(&YServer::RecvThread,this);
	Send = thread(&YServer::SendThread,this);
	Recv.detach();
	Send.detach();
}
