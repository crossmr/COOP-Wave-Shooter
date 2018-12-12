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
#include "Net/UnrealNetwork.h"


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

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;

	CurrentNumberOfBullets = MagazineSize;
}

void ASWeapon::OnRep_HitScanTrace()
{
	//Play Cosmetic FX
	PlayFireEffects(HitScanTrace.TraceTo);

	PlayImpactsEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);

}

void ASWeapon::Fire()
{
	if (CurrentNumberOfBullets == 0) { return; }
	CurrentNumberOfBullets = CurrentNumberOfBullets - 1;
	if (Role < ROLE_Authority)
	{
		ServerFire();
		//return;
	}
	//trace from pawn eyes to crosshair location
	

	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		ASCharacter* MyPawn = Cast<ASCharacter>(MyOwner);
		if (MyPawn == nullptr || MyPawn->bIsReloading == true) { return; }
		MyPawn->bIsFiring = true;
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
		EPhysicalSurface SurfaceType = SurfaceType_Default;

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, CameraLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			//Block hit, apply damage
			AActor* HitActor = Hit.GetActor();

			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			float DamageToApply = BaseDamage;

			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				DamageToApply *= 4.0f;
			}

			UGameplayStatics::ApplyPointDamage(HitActor, DamageToApply, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);

			PlayImpactsEffects(SurfaceType, Hit.ImpactPoint);
			TracerEndPoint = Hit.ImpactPoint;

			
		}
		//Debug line for affordance while testing
		if (DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), MeshComp->GetSocketLocation(MuzzleSocketName), TracerEndPoint, FColor::White, false, 1.0f, 0, 1.0f);
		}

		PlayFireEffects(TracerEndPoint);

		if (Role == ROLE_Authority)
		{
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;

		}
		LastFiredTime = GetWorld()->TimeSeconds;

		
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

bool ASWeapon::ServerFire_Validate()
{
	return true;
}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

void ASWeapon::ServerStopFire_Implementation()
{
	StopFire();
}

bool ASWeapon::ServerStopFire_Validate()
{
	return true;
}

void ASWeapon::StopFire()
{
	if (Role < ROLE_Authority)
	{
		ServerStopFire();
		//return;
	}
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
	ASCharacter* MyPawn = Cast<ASCharacter>(GetOwner());
	if (MyPawn == nullptr || MyPawn->bIsReloading == true) { return; }
	MyPawn->bIsFiring = false;
}



void ASWeapon::PlayFireEffects(FVector TracerEndPoint)
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

	APawn* MyPawn = Cast<APawn>(GetOwner());
	if (MyPawn)
	{
		APlayerController* PC = Cast<APlayerController>(MyPawn->GetController());
		if (PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}

void ASWeapon::PlayImpactsEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
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
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation(), ParticleScale);
	}

}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}

