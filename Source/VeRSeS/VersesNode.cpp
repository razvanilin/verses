// Fill out your copyright notice in the Description page of Project Settings.

#include "VeRSeS.h"
#include "VersesNode.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"


// Sets default values
AVersesNode::AVersesNode()
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("NodeComp"));
	CollisionComp->InitBoxExtent(FVector(5, 5, 5));
	CollisionComp->CastShadow = true;
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;
	RootComponent = CollisionComp;

	TextComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextComp"));
	TextComp->AttachTo(RootComponent);
	TextComp->SetText("Hello");
	TextComp->SetRelativeRotation(CollisionComp->GetComponentRotation());
}

// Called when the game starts or when spawned
void AVersesNode::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AVersesNode::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

// set the text of the TextComp
void AVersesNode::SetText(FString text)
{
	if (TextComp != NULL) {
		TextComp->SetText(text);
	}
}

// set the tile size (static mesh size)
void AVersesNode::SetTileSize(float sizeX, float sizeY, float sizeZ)
{
	UStaticMeshComponent* mesh = FindComponentByClass<UStaticMeshComponent>();
	if (mesh)
	{
		mesh->SetWorldScale3D(FVector(sizeX, sizeY, sizeZ));
	}
}

