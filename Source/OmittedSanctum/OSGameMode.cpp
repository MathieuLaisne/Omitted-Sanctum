// Fill out your copyright notice in the Description page of Project Settings.

#include "OSGameMode.h"
#include "OSGameInstance.h"

void AOSGameMode::PostLogin(APlayerController* NewPlayer)
{
  Super::PostLogin(NewPlayer);

  UOSGameInstance* GI = Cast<UOSGameInstance>(GetGameInstance());
  if (GI && GI->ClassDataTable)
  {
    EOSPlayerClass ClassToSpawn = GI->GetSelectedClass();

    
  }
}