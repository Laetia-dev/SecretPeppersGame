#include "TavernLayoutSubsystem.h"

#include "GameFramework/Actor.h"

bool UTavernLayoutSubsystem::PlaceCardInSlot(int32 SlotIndex, FName CardId)
{
	if (SlotIndex < 0 || CardId.IsNone())
	{
		return false;
	}

	if (GetCardInSlot(SlotIndex) == CardId)
	{
		return true;
	}

	// Each card is unique on the wall. Placing it again moves it.
	int32 ExistingSlot = INDEX_NONE;
	for (const TPair<int32, FName>& Pair : PlacedCards)
	{
		if (Pair.Value == CardId)
		{
			ExistingSlot = Pair.Key;
			break;
		}
	}

	if (ExistingSlot != INDEX_NONE)
	{
		PlacedCards.Remove(ExistingSlot);
	}

	PlacedCards.Add(SlotIndex, CardId);
	OnDeckChanged.Broadcast();
	return true;
}

bool UTavernLayoutSubsystem::PlaceCardCopyInEmptySlot(int32 SlotIndex, FName CardId)
{
	if (SlotIndex < 0 || CardId.IsNone() || PlacedCards.Contains(SlotIndex))
	{
		return false;
	}

	PlacedCards.Add(SlotIndex, CardId);
	OnDeckChanged.Broadcast();
	return true;
}

bool UTavernLayoutSubsystem::MoveCardBetweenSlots(int32 FromSlot, int32 ToSlot)
{
	if (FromSlot < 0 || ToSlot < 0 || FromSlot == ToSlot)
	{
		return false;
	}

	const FName* Source = PlacedCards.Find(FromSlot);
	if (!Source || Source->IsNone())
	{
		return false;
	}

	const FName SourceCard = *Source;
	if (const FName* Target = PlacedCards.Find(ToSlot))
	{
		PlacedCards.Add(FromSlot, *Target);
	}
	else
	{
		PlacedCards.Remove(FromSlot);
	}
	PlacedCards.Add(ToSlot, SourceCard);
	OnDeckChanged.Broadcast();
	return true;
}

bool UTavernLayoutSubsystem::RemoveCardFromSlot(int32 SlotIndex)
{
	if (PlacedCards.Remove(SlotIndex) == 0)
	{
		return false;
	}

	OnDeckChanged.Broadcast();
	return true;
}

FName UTavernLayoutSubsystem::GetCardInSlot(int32 SlotIndex) const
{
	return PlacedCards.FindRef(SlotIndex);
}

TMap<int32, FName> UTavernLayoutSubsystem::GetPlacedCards() const
{
	return PlacedCards;
}

void UTavernLayoutSubsystem::RequestLayoutGeneration()
{
	OnLayoutGenerationRequested.Broadcast();
}

bool UTavernLayoutSubsystem::RegisterTemporaryActor(AActor* Actor)
{
	if (!IsValid(Actor) || Actor->IsActorBeingDestroyed())
	{
		return false;
	}

	for (const TWeakObjectPtr<AActor>& Existing : TemporaryActors)
	{
		if (Existing.Get() == Actor)
		{
			return true;
		}
	}

	TemporaryActors.Add(Actor);
	return true;
}

void UTavernLayoutSubsystem::ClearTemporaryRoomsForSleep()
{
	// Only explicitly registered temporary actors are destroyed.
	// The player must be moved to a permanent room before calling this.
	for (const TWeakObjectPtr<AActor>& WeakActor : TemporaryActors)
	{
		if (AActor* Actor = WeakActor.Get())
		{
			if (!Actor->IsActorBeingDestroyed())
			{
				Actor->Destroy();
			}
		}
	}

	TemporaryActors.Empty();
	OnTemporaryRoomsCleared.Broadcast();
}

int32 UTavernLayoutSubsystem::GetRegisteredTemporaryActorCount() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<AActor>& WeakActor : TemporaryActors)
	{
		if (WeakActor.IsValid())
		{
			++Count;
		}
	}
	return Count;
}
