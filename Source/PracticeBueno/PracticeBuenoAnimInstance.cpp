// Fill out your copyright notice in the Description page of Project Settings.


#include "PracticeBuenoAnimInstance.h"

UPracticeBuenoAnimInstance::UPracticeBuenoAnimInstance()
{
	IsInAir = false;
}

void UPracticeBuenoAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	AActor* OwningActor = GetOwningActor();

	if (OwningActor != nullptr)
	{
		if(OwningActor->GetVelocity().Z == 0.0)
		{
			IsInAir = false;
		}
		else 
		{
			IsInAir = true;
		}
		if (OwningActor->GetVelocity().Size() == 0.0f)
		{
			Speed = false;
		}
		else
		{
			Speed = true;
			
		}
	}
}