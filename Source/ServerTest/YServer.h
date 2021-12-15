// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <thread>

using namespace std;

/**
 * 
 */
class SERVERTEST_API YServer
{
public:
	YServer();
	~YServer();

public:
	void RecvThread();
	void SendThread();
	void SendToServer(FString Data);

	void CreateSocket();
public:
	class AServerTestCharacter* Player;

	class thread Recv;
	class thread Send;
};
