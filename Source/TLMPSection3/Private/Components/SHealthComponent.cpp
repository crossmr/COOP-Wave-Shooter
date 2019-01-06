// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"
#include "GameFramework/Actor.h"
#include "Math/UnrealMathUtility.h"
#include "Net/UnrealNetwork.h"
#include "SGameMode.h"
#include "Engine/World.h"




// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...

	DefaultHealth = 100.f;

	SetIsReplicated(true);

	bIsDead = false;
}


void USHealthComponent::Heal(float HealAmount)
{
	//if no healing amount or player is dead, return
	if (HealAmount <= 0.0f || Health <= 0.0f)
	{
		return;
	}

	Health = FMath::Clamp(Health + HealAmount, 0.0f, DefaultHealth);

	OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);
	 
}

float USHealthComponent::GetHealth() const
{
	return Health;
}

// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// Only do this if we are the server
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
		}
	}
	
	Health = DefaultHealth;
}

void USHealthComponent::OnRep_Health(float OldHealth)
{
	float Damage = Health - OldHealth;
	OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}

void USHealthComponent::HandleTakeAnyDamage(AActor * DamagedActor, float Damage, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	if (Damage <= 0.f || bIsDead)
	{
		return;
	}

	//Update Health Clamped
	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

	bIsDead = Health <= 0.0f;

	//UE_LOG(LogTemp, Warning, TEXT("Damage Taken: %s"), *FString::SanitizeFloat(Health));
	if (bIsDead)
	{
		ASGameMode* GM = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}
	}
	
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComponent, Health);
}

