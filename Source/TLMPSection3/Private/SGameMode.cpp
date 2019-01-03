// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameMode.h"
#include "TimerManager.h"

ASGameMode::ASGameMode()
{
	TimeBetweenWaves = 2.0f;
}

void ASGameMode::StartWave()
{
	WaveCount++;

	NrOfBotsToSpawn = 2 * WaveCount; 

	GetWorldTimerManager().SetTimer(TimeHandle_BotSpawner, this, &ASGameMode::SpawnBotTimeElapsed, 1.0f, true, 0.0f);
}

void ASGameMode::SpawnBotTimeElapsed()
{
	SpawnNewBot();

	NrOfBotsToSpawn--;

	if (NrOfBotsToSpawn <= 0)
	{
		EndWave();
	}

}

void ASGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimeHandle_BotSpawner);

	PrepareForNextWave();
}

void ASGameMode::PrepareForNextWave()
{
	FTimerHandle TimerHandle_NextWaveStart;
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimeBetweenWaves, false);

}

void ASGameMode::StartPlay()
{
	Super::StartPlay();

	PrepareForNextWave();
}
