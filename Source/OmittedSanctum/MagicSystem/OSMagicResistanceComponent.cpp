// Fill out your copyright notice in the Description page of Project Settings.


#include "OSMagicResistanceComponent.h"

// Sets default values for this component's properties
UOSMagicResistanceComponent::UOSMagicResistanceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Resistances.Empty();
}


// Called when the game starts
void UOSMagicResistanceComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

EOSMagicResWeak UOSMagicResistanceComponent::FindResistance(FString Element)
{
	EOSMagicResWeak FoundRes = EOSMagicResWeak::Zero;
	if (Resistances.Num() >= 1)
	{
		for (FOSMagicElementRelation elem : Resistances)
		{
			if (elem.ElementName == Element) {
				FoundRes = elem.Relation;
				break;
			}
		}
	}
	return FoundRes;
}

TPair<int, float> UOSMagicResistanceComponent::CalculateResistanceModifier(EOSMagicResWeak FoundResistance)
{
	int chance = 0;
	float damageReduction = 1;
	switch (FoundResistance)
	{
	case EOSMagicResWeak::Minor:
		chance = 5;
		damageReduction = .95f;
		break;
	case EOSMagicResWeak::Small:
		chance = 10;
		damageReduction = .9f;
		break;
	case EOSMagicResWeak::Moderate:
		chance = 25;
		damageReduction = .75f;
		break;
	case EOSMagicResWeak::Great:
		chance = 50;
		damageReduction = .5f;
		break;
	default:
		break;
	}
	return TPair<int, float>(chance, damageReduction);
}


