// Fill out your copyright notice in the Description page of Project Settings.


#include "OSPlayerState.h"

void AOSPlayerState::OSTakeDamage(int dmg)
{
  float damageReduction = StatusComponent->GetIncomingDamageMultiplier();
  CurrentHP -= FMath::RoundToInt(dmg * damageReduction);
  if (CurrentHP <= 0)
    GameOver();
  else
    OnTakingDamage();
}