#include "DeckWallComponent.h"

#include "TavernLayoutSubsystem.h"
#include "TavernSubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UDeckWallComponent::UDeckWallComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDeckWallComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			LayoutSubsystem = GameInstance->GetSubsystem<UTavernLayoutSubsystem>();
		}
	}

	if (LayoutSubsystem)
	{
		LayoutSubsystem->OnDeckChanged.AddUniqueDynamic(this, &UDeckWallComponent::HandleDeckChanged);
	}
}

void UDeckWallComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(LayoutSubsystem.Get()))
	{
		LayoutSubsystem->OnDeckChanged.RemoveDynamic(this, &UDeckWallComponent::HandleDeckChanged);
	}

	Super::EndPlay(EndPlayReason);
}

TArray<FName> UDeckWallComponent::GetDefinedMajorArcanaIds() const
{
	return MajorArcanaTable ? MajorArcanaTable->GetRowNames() : TArray<FName>();
}

bool UDeckWallComponent::IsValidSlot(int32 SlotIndex) const
{
	return SlotIndex >= 0 && SlotIndex < SlotCount;
}

bool UDeckWallComponent::PlaceMajorArcanaInSlot(int32 SlotIndex, FName CardId)
{
	if (!LayoutSubsystem || !IsValidSlot(SlotIndex) || CardId.IsNone() || !MajorArcanaTable)
	{
		return false;
	}

	if (!MajorArcanaTable->GetRowNames().Contains(CardId))
	{
		return false;
	}

	// This changes the deck only. Rooms are regenerated at the next night,
	// not immediately while the player may be inside a temporary room.
	return LayoutSubsystem->PlaceCardInSlot(SlotIndex, CardId);
}

bool UDeckWallComponent::PlaceInventoryCardInWallSlot(int32 FromInventorySlot, int32 ToWallSlot)
{
	if (!LayoutSubsystem || !MajorArcanaTable || !IsValidSlot(ToWallSlot) ||
		!LayoutSubsystem->GetCardInSlot(ToWallSlot).IsNone())
	{
		return false;
	}

	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UTavernSubsystem* Inventory = GameInstance ? GameInstance->GetSubsystem<UTavernSubsystem>() : nullptr;
	if (!Inventory)
	{
		return false;
	}

	const FTavernInventorySlot Source = Inventory->GetInventorySlot(FromInventorySlot);
	if (FromInventorySlot < 0 || FromInventorySlot >= Inventory->GetInventorySlotCount() ||
		Source.ItemID.IsNone() || Source.Quantity < 1 ||
		!MajorArcanaTable->GetRowNames().Contains(Source.ItemID))
	{
		return false;
	}

	// Reserve the destination before consuming inventory. Roll back if the
	// inventory removal unexpectedly fails (e.g. a reentrant UI callback).
	if (!LayoutSubsystem->PlaceCardCopyInEmptySlot(ToWallSlot, Source.ItemID))
	{
		return false;
	}
	if (!Inventory->RemoveInventoryItemAtSlot(FromInventorySlot, 1))
	{
		LayoutSubsystem->RemoveCardFromSlot(ToWallSlot);
		return false;
	}
	return true;
}

bool UDeckWallComponent::ReturnWallCardToInventory(int32 WallSlot)
{
	if (!LayoutSubsystem || !IsValidSlot(WallSlot))
	{
		return false;
	}

	const FName CardId = LayoutSubsystem->GetCardInSlot(WallSlot);
	if (CardId.IsNone())
	{
		return false;
	}

	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UTavernSubsystem* Inventory = GameInstance ? GameInstance->GetSubsystem<UTavernSubsystem>() : nullptr;
	if (!Inventory || !Inventory->AddInventoryItem(CardId, 1, 1))
	{
		return false;
	}

	if (!LayoutSubsystem->RemoveCardFromSlot(WallSlot))
	{
		// This should not happen after the successful presence check.
		Inventory->RemoveInventoryItem(CardId, 1);
		return false;
	}
	return true;
}

bool UDeckWallComponent::MoveWallCardBetweenSlots(int32 FromWallSlot, int32 ToWallSlot)
{
	return LayoutSubsystem && IsValidSlot(FromWallSlot) && IsValidSlot(ToWallSlot) &&
		LayoutSubsystem->MoveCardBetweenSlots(FromWallSlot, ToWallSlot);
}

bool UDeckWallComponent::RemoveCardFromWallSlot(int32 SlotIndex)
{
	if (!LayoutSubsystem || !IsValidSlot(SlotIndex))
	{
		return false;
	}

	return LayoutSubsystem->RemoveCardFromSlot(SlotIndex);
}

FName UDeckWallComponent::GetCardInWallSlot(int32 SlotIndex) const
{
	return LayoutSubsystem && IsValidSlot(SlotIndex)
		? LayoutSubsystem->GetCardInSlot(SlotIndex)
		: NAME_None;
}

TArray<FName> UDeckWallComponent::GetWallCardsInSlotOrder() const
{
	TArray<FName> Cards;
	Cards.Init(NAME_None, FMath::Max(0, SlotCount));

	if (LayoutSubsystem)
	{
		for (const TPair<int32, FName>& Pair : LayoutSubsystem->GetPlacedCards())
		{
			if (IsValidSlot(Pair.Key))
			{
				Cards[Pair.Key] = Pair.Value;
			}
		}
	}

	return Cards;
}

void UDeckWallComponent::HandleDeckChanged()
{
	OnWallChanged.Broadcast();
}
