// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "SWeapon.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/InputSettings.h"
#include "SWeapon.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "TLMPSection3.h"
#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Controller.h"


// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->bUsePawnControlRotation = true;

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComp->SetupAttachment(SpringArmComp);

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	ZoomedFOV = 50.0f;
	ZoomInterSpeed = 20.0f;

	WeaponSocketName = "WeaponSocket";

	ReloadTime = 2.167f;
	bIsReloading = false;
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);
	DefaultFOV = CameraComp->FieldOfView;
	LastTimeReloaded = GetWorld()->TimeSeconds;

	if (Role == ROLE_Authority)
	{
		//Spawn a Default Weapon
		if (GunToUse)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.Instigator = Instigator;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnInfo.Owner = this;
			Gun = GetWorld()->SpawnActor<ASWeapon>(GunToUse, SpawnInfo);
			if (Gun)
			{
				Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), WeaponSocketName);
			}
		}

	}

	LastTimeReloaded = GetWorld()->TimeSeconds;

}

void ASCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);
}

void ASCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}

void ASCharacter::BeginCrouch()
{
	Crouch();
}

void ASCharacter::EndCrouch()
{
	UnCrouch();
}

void ASCharacter::ZoomIn()
{
	bWantsToZoom = true;
}

void ASCharacter::ZoomOut()
{
	bWantsToZoom = false;
}

void ASCharacter::OnHealthChanged(USHealthComponent* HealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && !bIsDead)
	{
		//Die
		bIsDead = true;

		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();

		SetLifeSpan(10.0f);


	}
}

void ASCharacter::StartFire()
{
	//Make sure the player isn't reloading before firing
	if (Gun)
	{
		Gun->StartFire();
		bIsFiring = true;
	}
}

void ASCharacter::StopFire()
{
	if (Gun)
	{
		Gun->StopFire();
		bIsFiring = false;		
	}
}

void ASCharacter::CompleteReload()
{
	if (Gun)
	{
		//Complete the reload and allow the player to fire
		Gun->Reload();
		bIsReloading = false;
	}
}

void ASCharacter::StartReload()
{
	//Check to make sure the player isn't already reloading
	if (!bIsReloading)
	{
		//Create a delay before completing the reload
		GetWorldTimerManager().SetTimer(TimerHandle_TimeToReload, this, &ASCharacter::CompleteReload, ReloadTime, false);
		bIsReloading = true;
	}
}


// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;

	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterSpeed);

	CameraComp->SetFieldOfView(NewFOV);
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::ZoomIn);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::ZoomOut);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASCharacter::StartReload);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);
}

FVector ASCharacter::GetCameraForwardVector()
{
	if (CameraComp == nullptr) { return GetActorForwardVector(); }
	FVector VectorToReturn = CameraComp->GetForwardVector();
	return VectorToReturn;
}

FVector ASCharacter::GetCameraLocation()
{
	return CameraComp->GetComponentLocation();
}

void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASCharacter, Gun);
}
