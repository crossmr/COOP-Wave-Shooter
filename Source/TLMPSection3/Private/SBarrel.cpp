// Fill out your copyright notice in the Description page of Project Settings.

#include "SBarrel.h"
#include "Components/StaticMeshComponent.h"
#include "SHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Engine/World.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Net/UnrealNetwork.h"



// Sets default values
ASBarrel::ASBarrel()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	SetRootComponent(MeshComp);

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASBarrel::OnHealthChanged);


	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("Radial Force Component"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->Radius = 250.0f;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false; //we don't want it to fire as soon as the game starts
	RadialForceComp->bIgnoreOwningActor = true;//don't affect yourself

	ExplosiveForce = 400;

	SetReplicates(true);
	SetReplicateMovement(true);

}

// Called when the game starts or when spawned
void ASBarrel::BeginPlay()
{
	Super::BeginPlay();
	

	if (Role == ROLE_Authority)
	{
		bExploded = false;
	}
}

void ASBarrel::OnHealthChanged(USHealthComponent * OwningHealthComp, float Health, float HealthDelta, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	//check to make sure the barrel isn't already exploded
	if (Health <= 0.0f && !bExploded)
	{
		//explode

		bExploded = true;
		OnRep_Exploded();

		//make the barrel pop up when it explodes
		FVector ImpulseIntensity = FVector::UpVector * ExplosiveForce;
		MeshComp->AddImpulse(ImpulseIntensity, NAME_None, true);

		RadialForceComp->FireImpulse();

		UGameplayStatics::ApplyRadialDamage(GetWorld(), 100.f, GetActorLocation(), 500.f, BarrelDamageType, TArray<AActor*>(), this, NULL, true);

	}
}

void ASBarrel::OnRep_Exploded()
{
	//check for and apply exploding effect and materials
	if (ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	}
	if (ExplodedMaterial)
	{
		MeshComp->SetMaterial(0, ExplodedMaterial);
	}
	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
	}
}

void ASBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASBarrel, bExploded);

}

