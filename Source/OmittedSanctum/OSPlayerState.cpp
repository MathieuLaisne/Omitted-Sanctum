// Fill out your copyright notice in the Description page of Project Settings.

#include "OSPlayerState.h"
#include "OSGameInstance.h"

void AOSPlayerState::OSTakeDamage(int dmg)
{
  float damageReduction = StatusComponent->GetIncomingDamageMultiplier();
  CurrentHP -= FMath::RoundToInt(dmg * damageReduction);
  if (CurrentHP <= 0)
    GameOver();
  else
    OnTakingDamage();
}

void AOSPlayerState::BeginPlay()
{
  Super::BeginPlay();

  UOSGameInstance* GI = Cast<UOSGameInstance>(GetGameInstance());
  if (GI && GI->GetCurrentSaveData())
  {
    // LOAD HP from Save
    CurrentHP = GI->GetCurrentSaveData()->CurrentHP;
  }
  else
  {
    // NEW RUN defaults
    CurrentHP = 100;
  }
}