// Copyright Epic Games, Inc. All Rights Reserved.

#include "ServerTestCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "YServer.h"
#include <cstdlib>

//////////////////////////////////////////////////////////////////////////
// AServerTestCharacter

FString GetTransformString(FTransform Trans) 
{
	FString Location = FString::Printf(TEXT("%f,%f,%f"), Trans.GetLocation().X, Trans.GetLocation().Y, Trans.GetLocation().Z);
	FString Rotation = FString::Printf(TEXT("%f,%f,%f"), Trans.GetRotation().X, Trans.GetRotation().Y, Trans.GetRotation().Z);
	FString Scale = FString::Printf(TEXT("%f,%f,%f"), Trans.GetScale3D().X, Trans.GetScale3D().Y, Trans.GetScale3D().Z);
	FString Result = Location + Rotation + Scale;
	return Result;
}

FTransform GetTransformFromString(FString String) 
{
	FTransform result;
	TArray<FString> Strings = {};
	String.ParseIntoArray(Strings, TEXT("/"));
	TArray<FString> Temp = {};
	FVector temp;
	Strings[0].ParseIntoArray(Temp, TEXT(","));
	temp.X = FCString::Atof(*Temp[0]);
	temp.Y = FCString::Atof(*Temp[1]);
	temp.Z = FCString::Atof(*Temp[2]);
	result.SetLocation(temp);
	Temp.Empty();
	Strings[1].ParseIntoArray(Temp, TEXT(","));
	temp.X = FCString::Atof(*Temp[0]);
	temp.Y = FCString::Atof(*Temp[1]);
	temp.Z = FCString::Atof(*Temp[2]);
	result.SetRotation(temp.ToOrientationQuat());
	Temp.Empty();
	Strings[2].ParseIntoArray(Temp, TEXT(","));
	temp.X = FCString::Atof(*Temp[0]);
	temp.Y = FCString::Atof(*Temp[1]);
	temp.Z = FCString::Atof(*Temp[2]);
	result.SetScale3D(temp);
	return result;
}


AServerTestCharacter::AServerTestCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void AServerTestCharacter::BeginPlay()
{
	Super::BeginPlay();
	SM = new YServer();
	SM->Player = this;
	SM->CreateSocket();
	SM->SendToServer("JoinServer:"+ GetTransformString(GetActorTransform()));
}

void AServerTestCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	delete SM;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AServerTestCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	//PlayerInputComponent->BindAction("Move", IE_Pressed, this, &AServerTestCharacter::PressMoveInput);
	//PlayerInputComponent->BindAction("Move", IE_Released, this, &AServerTestCharacter::ReleaseMoveInput);
	PlayerInputComponent->BindAction("SpaceBar", IE_Pressed, this, &AServerTestCharacter::PressMoveInput);



	PlayerInputComponent->BindAxis("MoveForward", this, &AServerTestCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AServerTestCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AServerTestCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AServerTestCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AServerTestCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AServerTestCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AServerTestCharacter::OnResetVR);
}

void AServerTestCharacter::AddPlayerEvent(FTransform Transform)
{
	GetWorld()->SpawnActor<AServerTestCharacter>(AServerTestCharacter::StaticClass(), Transform);
}

void AServerTestCharacter::RecieveServerMsg(FString Msg)
{
	if (Msg.Len() <= 0)
		return;
	if (Msg == "Jump")
		Jump();
	char* context;
	char* Head = strtok_s(TCHAR_TO_ANSI(*Msg), ":",&context);
	if (strcmp(Head, "JoinServer") == 0)
	{
		FString TransString = context; 
		AddPlayerEvent(GetTransformFromString(TransString));
	}

}

void AServerTestCharacter::PressMoveInput()
{
	FTransform trans;
	GetWorld()->SpawnActor<AServerTestCharacter>(AServerTestCharacter::StaticClass(), trans);

	//SM->SendToServer(FString("StartMove"));
}

void AServerTestCharacter::ReleaseMoveInput()
{
	SM->SendToServer(FString("EndMove"));
}


void AServerTestCharacter::OnResetVR()
{
	// If ServerTest is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in ServerTest.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AServerTestCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AServerTestCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AServerTestCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AServerTestCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AServerTestCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AServerTestCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
