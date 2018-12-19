// Fill out your copyright notice in the Description page of Project Settings.

#include "SPowerUpActor.h"
#include "TimerManager.h"


// Sets default values
ASPowerUpActor::ASPowerUpActor()
{
	PowerupInterval = 0.0f;
	TotalNrOfTicks = 0.0f;
}

void ASPowerUpActor::ActivatePowerup()
{

	if (PowerupInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ASPowerUpActor::OnTickPowerup, true, 0.0f);
	}
	else
	{
		OnTickPowerup();
	}
}

// Called when the game starts or when spawned
void ASPowerUpActor::BeginPlay()
{
	Super::BeginPlay();
}

void ASPowerUpActor::OnTickPowerup()
{
	TicksProcessed++;

	OnPowerupTicked();

	if (TicksProcessed >= TotalNrOfTicks)
	{
		OnExpired();

		//Delete Timer
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);

	}
}


