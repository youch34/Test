// Fill out your copyright notice in the Description page of Project Settings.


#include "JoinServer.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Networking.h"
#include <thread>

using namespace std;

// Sets default values for this component's properties
UJoinServer::UJoinServer()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}

void UJoinServer::EndPlay(const EEndPlayReason::Type EndPlayReson)
{
	Super::EndPlay(EndPlayReson);
	if (sock != nullptr)
		sock->Close();
}

// Called when the game starts
void UJoinServer::BeginPlay()
{
	Super::BeginPlay();

	sock = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("Test"), NAME_None);
	
	FString addr = TEXT("127.0.0.1");
	int32 port = 6969;
	
	FIPv4Address ip;
	FIPv4Address::Parse(addr, ip);

	TSharedPtr<FInternetAddr> Addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	Addr->SetIp(ip.Value);
	Addr->SetPort(port);
	sock->GetConnectionState();
	bool connected = sock->Connect(*Addr);
	//sock->SetNonBlocking(true);
	if (sock->GetConnectionState() == ESocketConnectionState::SCS_Connected)
	{
		//SendThread = thread(&UJoinServer::SendMsg,this);
		//SendThread.detach();
		RecvThread = thread(&UJoinServer::RecvMsg, this);
		RecvThread.detach();
	}
}



// Called every frame
void UJoinServer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// ...
}

void UJoinServer::SendMsg()
{
	while (sock != nullptr) 
	{
		FString msg = "UE4";
		TCHAR * serializedMsg = msg.GetCharArray().GetData();
		int32 sent = 0;
		sock->Send((uint8*)(TCHAR_TO_UTF8(serializedMsg)), FCString::Strlen(serializedMsg), sent);
	}
}

void UJoinServer::RecvMsg()
{
	uint32 DataSize = 0;
	TArray<uint8> Data;
	int32 Read = 1;
	static float r = 0;
	while (sock != nullptr)
	{
		if (sock->Recv(Data.GetData(), 1024, Read, ESocketReceiveFlags::None))
		{
			if (Data.GetData() != nullptr)
			{
				GEngine->AddOnScreenDebugMessage(1, 1, FColor(r, r, r), TEXT("Recieve Message"));
				r += 0.1;
			}
			return;
		}
	}
}

