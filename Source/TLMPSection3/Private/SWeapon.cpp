// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "SCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TLMPSection3.h"
#include "TimerManager.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing (
	TEXT("COOP.DebugWeapons"), 
	DebugWeaponDrawing, 
	TEXT("Draw Debug Lines for Weapons"), 
	ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);

	MuzzleSocketName = "MuzzleFlashSocket";

	BaseDamage = 20.0f;

	RateOfFire = 600.0f;

	MagazineSize = 30;

	SetReplicates(true);
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;

	CurrentNumberOfBullets = MagazineSize;
}


void ASWeapon::Fire()
{
	//trace from pawn eyes to crosshair location
	if (CurrentNumberOfBullets == 0) { return; }

	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		ASCharacter* MyPawn = Cast<ASCharacter>(MyOwner);
		if (MyPawn == nullptr || MyPawn->bIsReloading == true) { return; }
		FVector CameraForward = MyPawn->GetCameraForwardVector();
		FVector CameraLocation = MyPawn->GetCameraLocation();

		FVector TraceEnd = CameraLocation + (CameraForward * 10000);

		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		FVector TracerEndPoint = TraceEnd;

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, CameraLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			//Block hit, apply damage
			AActor* HitActor = Hit.GetActor();

			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			float DamageToApply = BaseDamage;

			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				DamageToApply *= 4.0f;
			}

			UGameplayStatics::ApplyPointDamage(HitActor, DamageToApply, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);
			TracerEndPoint = Hit.ImpactPoint;
		

			UParticleSystem* SelectedEffect = nullptr;
			FVector ParticleScale = FVector(1.0f, 1.0f, 1.0f);
			switch (SurfaceType)
			{
			case SURFACE_FLESHDEFAULT:
				SelectedEffect = FleshImpactEffect;
				ParticleScale = FVector(0.5f, 0.5f, 0.5f);
				break;
			case SURFACE_FLESHVULNERABLE:
				SelectedEffect = FleshImpactEffect;
				ParticleScale = FVector(0.5f, 0.5f, 0.5f);
				break;
			default:
				SelectedEffect = DefaultImpactEffect;
				break;
			}

			if (SelectedEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), ParticleScale);
			}
			
		}
		//Debug line for affordance while testing
		if (DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), MeshComp->GetSocketLocation(MuzzleSocketName), TracerEndPoint, FColor::White, false, 1.0f, 0, 1.0f);
		}

		PlayFireEffects(TracerEndPoint, MyPawn);

		LastFiredTime = GetWorld()->TimeSeconds;

		CurrentNumberOfBullets = CurrentNumberOfBullets - 1;
	}
	
}

void ASWeapon::StartFire()
{
		float FirstDelay = FMath::Max(LastFiredTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

		GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::Reload()
{
	CurrentNumberOfBullets = MagazineSize;
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}



void ASWeapon::PlayFireEffects(FVector TracerEndPoint, ASCharacter* MyPawn)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TraceEffect)
	{
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TraceEffect, MeshComp->GetSocketLocation(MuzzleSocketName));
		if (TracerComp)
		{
			TracerComp->SetVectorParameter("BeamEnd", TracerEndPoint);
		}
	}

	if (MyPawn)
	{
		APlayerController* PC = Cast<APlayerController>(MyPawn->GetController());
		if (PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}



