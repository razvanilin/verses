// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "VeRSeS.h"
#include "VeRSeSCharacter.h"
#include "VeRSeSProjectile.h"
#include "VersesNode.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AVeRSeSCharacter

AVeRSeSCharacter::AVeRSeSCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// set the verses nodes values
	NumberOfNodesPerLevel = 8;
	VersesRadius = 400.f;
	XOffset = 140.f;
	YOffset = 50.f;
	TileSizeX = 1.f;
	TileSizeZ = 1.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 30.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P are set in the
	// derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void AVeRSeSCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint")); //Attach gun mesh component to Skeleton, doing it here because the skelton is not yet created in the constructor
}

//////////////////////////////////////////////////////////////////////////
// Input

void AVeRSeSCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	check(InputComponent);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	//InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AVeRSeSCharacter::TouchStarted);
	if (EnableTouchscreenMovement(InputComponent) == false)
	{
		//InputComponent->BindAction("Fire", IE_Pressed, this, &AVeRSeSCharacter::OnFire);
	}

	// Action to start the poem (spawn the nodes)
	InputComponent->BindAction("Fire", IE_Pressed, this, &AVeRSeSCharacter::OnStartPoem);

	InputComponent->BindAxis("MoveForward", this, &AVeRSeSCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AVeRSeSCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &AVeRSeSCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AVeRSeSCharacter::LookUpAtRate);
}

void AVeRSeSCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		const FRotator SpawnRotation = GetControlRotation();
		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
		const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);
		
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			// spawn the projectile at the muzzle
			World->SpawnActor<AVeRSeSProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
		}
	}

	// try and play the sound if specified
	/*if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}*/

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}

}

void AVeRSeSCharacter::OnStartPoem()
{
	// try and fire a projectile
	if (NodeClass != NULL)
	{
		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
		// const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

		int levelNodeCounter1 = 1;
		int levelNodeCounter2 = 1;// this resets everytime it reaches 8
		float yawOffset = 360 / NumberOfNodesPerLevel; // this determines the angle between each consecutive tile
		int level = 1;
		float x = 0.f, y = 0.f, z = 200.f, yaw = 0.f, x1 = 0.f, x2 = 0.f, y1 = 0.f, y2 = 0.f;

		for (int i = 0; i < 80; ++i)
		{
			UE_LOG(LogTemp, Warning, TEXT("i: %d"), i);
			UE_LOG(LogTemp, Warning, TEXT("NumberOfNodesPerLevel: %d"), NumberOfNodesPerLevel * (level - 1));
			if (i == 0 || NumberOfNodesPerLevel * (level-1) == i) {
				x1 = XOffset;
				y1 = -YOffset;
				yaw = 0.f;

			}

			if (i == 1 || NumberOfNodesPerLevel * (level-1) == i - 1)
			{
				x2 = XOffset;
				y2 = VersesRadius - YOffset;
				yaw = 180.f;
			}

			//UE_LOG(LogTemp, Warning, TEXT("levelNodeCounter: %f"), levelNodeCounter);
			//UE_LOG(LogTemp, Warning, TEXT("Division: %d"), NumberOfNodesPerLevel/4 + 1);
			if (i % 2 == 0 && i != 0 && NumberOfNodesPerLevel * (level-1) != i) 
			{

				// check to see if the axis quadrant changes (changes every quarter of the total circle - or of the total number of nodes)
				if (levelNodeCounter1 < NumberOfNodesPerLevel / 4 + 1)
				{
					/*UE_LOG(LogTemp, Warning, TEXT(" Hello 1 one"));
					UE_LOG(LogTemp, Warning, TEXT("levelNodeCounter: %d"), levelNodeCounter1);
					UE_LOG(LogTemp, Warning, TEXT("Division: %d"), NumberOfNodesPerLevel/4 + 1);*/

					x1 += XOffset;
					y1 += YOffset;
				}
				else if (levelNodeCounter1 == NumberOfNodesPerLevel / 4 + 1)
				{
					/*UE_LOG(LogTemp, Warning, TEXT(" Hello 2 one "));
					UE_LOG(LogTemp, Warning, TEXT("levelNodeCounter: %d"), levelNodeCounter1);
					UE_LOG(LogTemp, Warning, TEXT("Division: %d"), NumberOfNodesPerLevel / 4 + 1);*/

					x1 += YOffset;
					y1 += XOffset;
				} 
				else
				{
					/*UE_LOG(LogTemp, Warning, TEXT(" Hello 3 one"));
					UE_LOG(LogTemp, Warning, TEXT("levelNodeCounter: %d"), levelNodeCounter1);
					UE_LOG(LogTemp, Warning, TEXT("Division: %d"), NumberOfNodesPerLevel / 4 + 1);*/

					x1 -= YOffset;
					y1 += XOffset;
				}

				if (yaw != 0.f)
					yaw = yawOffset * (levelNodeCounter1 - 1);
			}
			else if (i % 2 != 0 && i != 1 && NumberOfNodesPerLevel * (level-1) != i - 1)
			{
				// check to see if the axis quadrant changes (changes every quarter of the total circle - or of the total number of nodes)
				if (levelNodeCounter2  < NumberOfNodesPerLevel / 4 + 1)
				{
					/*UE_LOG(LogTemp, Warning, TEXT(" Hello 1 two"));
					UE_LOG(LogTemp, Warning, TEXT("levelNodeCounter: %d"), levelNodeCounter2);
					UE_LOG(LogTemp, Warning, TEXT("Division: %d"), NumberOfNodesPerLevel / 4 + 1);*/

					x2 -= XOffset;
					y2 -= YOffset;
				}
				else if (levelNodeCounter2 == NumberOfNodesPerLevel / 4 + 1)
				{
					/*UE_LOG(LogTemp, Warning, TEXT(" Hello 2 two"));
					UE_LOG(LogTemp, Warning, TEXT("levelNodeCounter: %d"), levelNodeCounter2);
					UE_LOG(LogTemp, Warning, TEXT("Division: %d"), NumberOfNodesPerLevel / 4 + 1);*/

					x2 -= YOffset;
					y2 -= XOffset;
				}
				else
				{
					/*UE_LOG(LogTemp, Warning, TEXT(" Hello 3 two"));
					UE_LOG(LogTemp, Warning, TEXT("levelNodeCounter: %d"), levelNodeCounter2);
					UE_LOG(LogTemp, Warning, TEXT("Division: %d"), NumberOfNodesPerLevel / 4 + 1);
*/
					x2 += YOffset;
					y2 -= XOffset;
				}

				yaw = -180.f + (yawOffset * (levelNodeCounter2 - 1));
			}

			if (i % 2 == 0) {
				x = x1;
				y = y1;
				++levelNodeCounter1;
			}
			else {
				x = x2;
				y = y2;
				++levelNodeCounter2;
			}

			FRotator SpawnRotation = FRotator(0.f, yaw, 0.f);
			FVector SpawnLocation = FVector(x, y, z * level);

			UE_LOG(LogTemp, Warning, TEXT("X: %f"), x);
			UE_LOG(LogTemp, Warning, TEXT("Y: %f"), y);
			UE_LOG(LogTemp, Warning, TEXT("Z: %f"), z * level);
			UE_LOG(LogTemp, Warning, TEXT(" ----------- "));


			UWorld* const World = GetWorld();
			if (World != NULL)
			{
				// spawn the projectile at the muzzle
				AVersesNode* Node = World->SpawnActor<AVersesNode>(NodeClass, SpawnLocation, SpawnRotation);
				Node->SetText(FString::FormatAsNumber(i));
				Node->SetTileSize(TileSizeX, 0.15f, TileSizeZ);
			}

			if ((levelNodeCounter1 + levelNodeCounter2) == NumberOfNodesPerLevel + 2) {
				levelNodeCounter1 = 1;
				levelNodeCounter2 = 1;
				x = 0.f, y = 0.f, yaw = 0.f, x1 = 0.f, x2 = 0.f, y1 = 0.f, y2 = 0.f;
				++level;
			}

			/*if (NumberOfNodesPerLevel % (i + 1) == 0) {
				levelNodeCounter1 = 1;
				levelNodeCounter2 = 1;// this resets everytime it reaches 8
				x = 0.f, y = 0.f, yaw = 0.f, x1 = 0.f, x2 = 0.f, y1 = 0.f, y2 = 0.f;
			}*/
		}
	}
	
	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void AVeRSeSCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AVeRSeSCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = false;
}

void AVeRSeSCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
	{
		if (TouchItem.bIsPressed)
		{
			if (GetWorld() != nullptr)
			{
				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
				if (ViewportClient != nullptr)
				{
					FVector MoveDelta = Location - TouchItem.Location;
					FVector2D ScreenSize;
					ViewportClient->GetViewportSize(ScreenSize);
					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
					{
						TouchItem.bMoved = true;
						float Value = ScaledDelta.X * BaseTurnRate;
						AddControllerYawInput(Value);
					}
					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
					{
						TouchItem.bMoved = true;
						float Value = ScaledDelta.Y * BaseTurnRate;
						AddControllerPitchInput(Value);
					}
					TouchItem.Location = Location;
				}
				TouchItem.Location = Location;
			}
		}
	}
}

void AVeRSeSCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AVeRSeSCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AVeRSeSCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AVeRSeSCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AVeRSeSCharacter::EnableTouchscreenMovement(class UInputComponent* InputComponent)
{
	bool bResult = false;
	if (FPlatformMisc::GetUseVirtualJoysticks() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		bResult = true;
		InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AVeRSeSCharacter::BeginTouch);
		InputComponent->BindTouch(EInputEvent::IE_Released, this, &AVeRSeSCharacter::EndTouch);
		InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AVeRSeSCharacter::TouchUpdate);
	}
	return bResult;
}
