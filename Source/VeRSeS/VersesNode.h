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
		class UBoxComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		class URotatingMovementComponent* RotatingMovement;

	/* Text overlay component*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Text, meta = (AllowPrivateAccess = "true"))
	class UTextRenderComponent* TextComp;

public:	
	// Sets default values for this actor's properties
	AVersesNode();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	void SetText(FString text);

	void SetTileSize(float sizeX, float sizeY, float sizeZ);

	/** Returns CollisionComp subobject **/
	FORCEINLINE class UBoxComponent* GetCollisionComp() const { return CollisionComp; }
	
};
