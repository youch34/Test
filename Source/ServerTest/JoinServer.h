// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <thread>
#include "JoinServer.generated.h"

using namespace std;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SERVERTEST_API UJoinServer : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UJoinServer();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReson) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override; \

	UFUNCTION(BlueprintCallable, Category = "Network")
		void SendMsg();

	UFUNCTION(BlueprintCallable, Category = "Network")
		void RecvMsg();
	 
	class FSocket* sock;
	class thread RecvThread;
	class thread SendThread;
};
