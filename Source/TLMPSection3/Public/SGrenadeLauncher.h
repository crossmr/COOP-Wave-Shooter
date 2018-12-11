// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SGrenadeLauncher.generated.h"

class ASProjectile;

/**
 * 
 */
UCLASS()
class TLMPSECTION3_API ASGrenadeLauncher : public ASWeapon
{
	GENERATED_BODY()
	
public:
	//override firing functions to handle single shot vs automatic fire
	virtual void Fire() override;

	virtual void StopFire() override;

	virtual void StartFire() override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<ASProjectile> Projectile;
	
};
