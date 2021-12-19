// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Animation/AnimInstance.h"
#include "PracticeBuenoAnimInstance.generated.h"

/**
 * 
 */
UCLASS(transient, Blueprintable, hideCategories=AnimInstance, BlueprintType)
class UPracticeBuenoAnimInstance: public UAnimInstance 
{

	GENERATED_BODY()

public:
	UPracticeBuenoAnimInstance();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generic")
		bool Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generic")
		bool IsInAir;


	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
