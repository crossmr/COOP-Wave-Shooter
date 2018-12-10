// Fill out your copyright notice in the Description page of Project Settings.

#include "SProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Engine/World.h"


// Sets default values
ASProjectile::ASProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Component"));
	SetRootComponent(CollisionComp);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	CollisionComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	CollisionComp->SetSimulatePhysics(true);

	ProjectileBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Body"));
	ProjectileBody->SetupAttachment(CollisionComp);

	MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	MovementComp->InitialSpeed = 3000.0f;
	MovementComp->MaxSpeed = 3000.0f;



}

// Called when the game starts or when spawned
void ASProjectile::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().ClearTimer(TimerHandle_Explode);
	GetWorldTimerManager().SetTimer(TimerHandle_Explode, this, &ASProjectile::Explode, 1.0f);

	
}

void ASProjectile::Explode()
{
	if (ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation(), FRotator (0.0f, 0.0f, 0.0f));
	}
	
	UGameplayStatics::ApplyRadialDamage(GetWorld(), 50.f, GetActorLocation(), 500.f, DamageType, TArray<AActor*>(), this, NULL, true);
	Destroy();
}

// Called every frame
void ASProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

