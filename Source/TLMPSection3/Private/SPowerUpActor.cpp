// Fill out your copyright notice in the Description page of Project Settings.

#include "SPowerUpActor.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"



// Sets default values
ASPowerUpActor::ASPowerUpActor()
{
	PowerupInterval = 0.0f;
	TotalNrOfTicks = 0.0f;

	bIsPowerupActive = false;

	SetReplicates(true);
}

void ASPowerUpActor::ActivatePowerup()
{
	OnActivated();

	bIsPowerupActive = true;
	OnRep_PowerupActive();

	if (PowerupInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ASPowerUpActor::OnTickPowerup, PowerupInterval, true);
	}
	else
	{
		OnTickPowerup();
	}
}

void ASPowerUpActor::OnTickPowerup()
{
	TicksProcessed++;

	OnPowerupTicked();

	if (TicksProcessed >= TotalNrOfTicks)
	{
		OnExpired();

		bIsPowerupActive = false;
		OnRep_PowerupActive();

		//Delete Timer
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);

	}
}

void ASPowerUpActor::OnRep_PowerupActive()
{
	OnPowerupStateChanged(bIsPowerupActive);
}

void ASPowerUpActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerUpActor, bIsPowerupActive);
}

