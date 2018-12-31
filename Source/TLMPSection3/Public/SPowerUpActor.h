// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPowerUpActor.generated.h"

UCLASS()
class TLMPSECTION3_API ASPowerUpActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPowerUpActor();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnExpired();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnPowerupTicked();

	void ActivatePowerup();


protected:


	/*Time Between Power up ticks*/
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	float PowerupInterval;

	/*Total times we apply the powerup effect*/
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	int32 TotalNrOfTicks;

	//How many ticks already done
	int32 TicksProcessed;

	FTimerHandle TimerHandle_PowerupTick;

	UFUNCTION()
	void OnTickPowerup();

	//Allows state to be replicated to client
	UPROPERTY(ReplicatedUsing=OnRep_PowerupActive)
	bool bIsPowerupActive;
	
	UFUNCTION()
	void OnRep_PowerupActive();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnPowerupStateChanged(bool bNewIsActive);
};
