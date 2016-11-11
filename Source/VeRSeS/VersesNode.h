// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "VersesNode.generated.h"

UCLASS(config=Game)
class AVersesNode : public AActor
{
	GENERATED_BODY()

		/* Box collision component */
		UPROPERTY(VisibleDefaultsOnly, Category=Plane)
		class USphereComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		class URotatingMovementComponent* RotatingMovement;
	
public:	
	// Sets default values for this actor's properties
	AVersesNode();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	/** Returns CollisionComp subobject **/
	FORCEINLINE class USphereComponent* GetCollisionComp() const { return CollisionComp; }
	
};
