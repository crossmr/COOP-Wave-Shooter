// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

enum class EWaveState : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled, AActor*, DeadActor, AActor*, ScoringActor, AController*, ScoringController);


/**
 * 
 */
UCLASS()
class TLMPSECTION3_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
	
protected:

	//This allows blueprint to spawn a single bot
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnNewBot();
	
	//start spawning bots for each wave
	void StartWave();

	void SpawnBotTimeElapsed();

	//stop spawning bots
	void EndWave();

	//Set Time for next wave
	void PrepareForNextWave();

	void CheckWaveState();

	void CheckAnyPlayerAlive();

	void GameOver();

	void SetWaveState(EWaveState NewState);

	void RespawnDeadPlayers();

	FTimerHandle TimeHandle_BotSpawner;

	FTimerHandle TimerHandle_NextWaveStart;

	//How many bots to spawn in current wave
	int32 NrOfBotsToSpawn;

	int32 WaveCount; 

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	float TimeBetweenWaves;

public:

	ASGameMode();

	virtual void StartPlay() override;
	
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintAssignable, Category = "GameMode")
	FOnActorKilled OnActorKilled;

};
