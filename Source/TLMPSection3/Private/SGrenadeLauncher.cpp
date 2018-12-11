// Fill out your copyright notice in the Description page of Project Settings.

#include "SGrenadeLauncher.h"
#include "Engine/World.h"
#include "SProjectile.h"
#include "Components/SkeletalMeshComponent.h"
#include "SCharacter.h"



void ASGrenadeLauncher::Fire()
{
	//Check to be sure a projectile is set
	if (Projectile)
	{
		
		
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		FRotator MuzzleRotation = MeshComp->GetSocketRotation(MuzzleSocketName);

		FVector EyeLocation;
		FRotator EyeRotation;
		GetOwner()->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.Instigator = Instigator;

		GetWorld()->SpawnActor<ASProjectile>(Projectile, MuzzleLocation, EyeRotation, ActorSpawnParams);
	}


}

void ASGrenadeLauncher::StopFire()
{
	//nothing to do but need to override parent class
}

void ASGrenadeLauncher::StartFire()
{
	Fire();
}
