// Fill out your copyright notice in the Description page of Project Settings.

#include "VeRSeS.h"
#include "VersesNode.h"
#include "GameFramework/RotatingMovementComponent.h"


// Sets default values
AVersesNode::AVersesNode()
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("NodeComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;
	RootComponent = CollisionComp;
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

