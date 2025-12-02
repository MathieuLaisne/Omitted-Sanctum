// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "MagicHelper.h"
#include "OSStatusEffectComponent.h"
#include "MagicEntity.generated.h"

UCLASS()
class OMITTEDSANCTUM_API AMagicEntity : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMagicEntity();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FOSMagicElement Element;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraComponent* NiagaraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectDuration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UOSStatusEffectComponent* StatusComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SetMagicElement(FOSMagicElement elem);

	UFUNCTION()
	void ApplyEffectToCaster();

	UFUNCTION()
	void ApplyEffectToSelf();

};
