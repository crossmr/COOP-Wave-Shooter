// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ASWeapon;
class USHealthComponent;

UCLASS()
class TLMPSECTION3_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FVector GetCameraForwardVector();

	FVector GetCameraLocation();

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Components")
	ASWeapon* Gun;

	UPROPERTY(BlueprintReadOnly, Category = "Weapons")
	bool bIsReloading;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void StartFire();

	void StopFire();

	void StartReload();

	void CompleteReload();

	void BeginCrouch();

	void EndCrouch();

	void PullTrigger();

	void ZoomIn();

	void ZoomOut();

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,  Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USHealthComponent* HealthComp;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<ASWeapon> GunToUse;

	UPROPERTY(EditDefaultsOnly, Category = "Weapons", meta = (ClampMin = 0.1, ClampMax = 100))
	float ZoomInterSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	float ZoomedFOV;

	UPROPERTY(VisibleDefaultsOnly, Category = "Weapons")
	FName WeaponSocketName;

	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	float ReloadTime;

	//Pawn is dead
	UPROPERTY(BlueprintReadOnly, Category = "Pawn")
	bool bIsDead;

	UPROPERTY(BlueprintReadOnly, Category = "Pawn")
	bool bWantsToZoom;
	
	//Default FOV for regular shooting
	float DefaultFOV;

	UPROPERTY(BlueprintReadOnly, Category = "Pawn")
	bool bIsFiring;

	float LastTimeReloaded;
	
	FTimerHandle TimerHandle_TimeToReload;

	UPROPERTY(Replicated)
	bool bIsAutomaticWeapon;

	virtual void Restart() override;
	
};
