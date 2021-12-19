// Copyright Epic Games, Inc. All Rights Reserved.

#include "PracticeBuenoCharacter.h"
#include "GameFramework/Actor.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

//////////////////////////////////////////////////////////////////////////
// APracticeBuenoCharacter

APracticeBuenoCharacter::APracticeBuenoCharacter()
{
	//PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);

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
	GetCharacterMovement()->JumpZVelocity = 450.f;
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
	
	Golpear = true;
	Chocando = false;
	
	CajaColision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	CajaColision->SetCollisionProfileName("Overlap");
	CajaColision->SetBoxExtent(FVector(50.0f, 50.0f, 30.0f));
	CajaColision->SetupAttachment(RootComponent);
	CajaColision->SetGenerateOverlapEvents(true);
	
	CajaColision->OnComponentBeginOverlap.AddDynamic(this, &APracticeBuenoCharacter::OnOverlapBegin);
	CajaColision->OnComponentEndOverlap.AddDynamic(this, &APracticeBuenoCharacter::OnOverlapEnd);
}

void APracticeBuenoCharacter::BeginPlay()
{
	Super::BeginPlay();

}
void APracticeBuenoCharacter::Tick(float DeltaTime) 
{
	Super::Tick(DeltaTime);
}

//////////////////////////////////////////////////////////////////////////
// Input

void APracticeBuenoCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &APracticeBuenoCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APracticeBuenoCharacter::MoveRight);

	PlayerInputComponent->BindAction("Punch", IE_Pressed, this, &APracticeBuenoCharacter::PunchEventServer);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &APracticeBuenoCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APracticeBuenoCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &APracticeBuenoCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &APracticeBuenoCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &APracticeBuenoCharacter::OnResetVR);
}


void APracticeBuenoCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void APracticeBuenoCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void APracticeBuenoCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void APracticeBuenoCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void APracticeBuenoCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void APracticeBuenoCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);

		
	}
}

void APracticeBuenoCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
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

void APracticeBuenoCharacter::PunchEventServer_Implementation()
{
	PunchEventMulti();
}

void APracticeBuenoCharacter::PunchEventMulti_Implementation()
{
	if (PunchMontage && Golpear)
	{
		Golpear = false;
		PlayAnimMontage(PunchMontage, 1, NAME_None);

		if (Player2 != nullptr)
		{
			Player2->HitEvent();
		}

		FTimerHandle TimerGolpe;
		GetWorld()->GetTimerManager().SetTimer(TimerGolpe, this, &APracticeBuenoCharacter::CambiarGolpe, 1.0f, false);
	}
}

void APracticeBuenoCharacter::CambiarGolpe()
{
	Golpear = true;
}

void APracticeBuenoCharacter::HitEvent()
{
	if (HitMontage)
	{
		PlayAnimMontage(HitMontage, 1, NAME_None);
	}
}

void APracticeBuenoCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APracticeBuenoCharacter* OtroJugador = Cast<APracticeBuenoCharacter>(OtherActor);

	if (OtroJugador && OtroJugador!= this)
	{
		Player2 = OtroJugador;
		Chocando = true;
	}
}

void APracticeBuenoCharacter::OnOverlapEnd(UPrimitiveComponent* OverlappedComp,  AActor* OtherActor,  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APracticeBuenoCharacter*  OtroJugador = Cast<APracticeBuenoCharacter>(OtherActor);

	if (OtroJugador && OtroJugador == Player2)
	{
		Player2 = nullptr;
		Chocando = false;
	}
}