// TavernSubsystem.cpp
#include "TavernSubsystem.h"

void UTavernSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	State.TimeOfDay = ETavernTimeOfDay::Day;
	State.DayNr = 1;
	State.Vibe = 0;
	State.Cleanliness = 0;

	Inventory.Empty();
	Inventory.Add(ETavernIngredient::Generic, 0);
	Inventory.Add(ETavernIngredient::CatFloof, 0);
	Inventory.Add(ETavernIngredient::Herb, 0);
	Inventory.Add(ETavernIngredient::Ash, 0);

	ItemSlots.SetNum(20);

	State.Coins = 10;
	State.PreparedFood = 0;

	// Year 1 is generated once at run initialization.
	CelestialYearPlans.Empty();
	EnsureCelestialYearPlan(GetCurrentLunarDate().Year);
}

void UTavernSubsystem::Deinitialize()
{
	Inventory.Empty();
	ItemSlots.Empty();
	CelestialYearPlans.Empty();
	Super::Deinitialize();
}

const FTavernState& UTavernSubsystem::GetState() const
{
	return State;
}

int32 UTavernSubsystem::GetDayNr() const
{
	return State.DayNr;
}

FTavernLunarDate UTavernSubsystem::GetCurrentLunarDate() const
{
	return UTavernLunarCalendarLibrary::GetLunarCalendarDate(State.DayNr);
}


FTavernCelestialYearPlan UTavernSubsystem::GetCurrentCelestialYearPlan() const
{
	const int32 CurrentYear = GetCurrentLunarDate().Year;
	if (const FTavernCelestialYearPlan* Found = CelestialYearPlans.Find(CurrentYear))
	{
		return *Found;
	}

	// Should only occur before subsystem initialization; never roll from a getter.
	FTavernCelestialYearPlan Empty;
	Empty.Year = CurrentYear;
	return Empty;
}

ETavernCelestialEvent UTavernSubsystem::GetTodaysCelestialEvent() const
{
	const int32 DayOfYear = GetCurrentLunarDate().DayOfYear;
	const FTavernCelestialYearPlan Plan = GetCurrentCelestialYearPlan();

	if (DayOfYear == Plan.SolarEclipseDay) return ETavernCelestialEvent::SolarEclipse;
	if (DayOfYear == Plan.MercuryTransitDay) return ETavernCelestialEvent::MercuryTransit;
	if (DayOfYear == Plan.VenusTransitDay) return ETavernCelestialEvent::VenusTransit;
	return ETavernCelestialEvent::None;
}

void UTavernSubsystem::EnsureCelestialYearPlan(int32 Year)
{
	if (Year < 1 || CelestialYearPlans.Contains(Year))
	{
		return;
	}

	FTavernCelestialYearPlan Plan;
	Plan.Year = Year;

	// The chances are separate annual rolls, NOT a choice between three outcomes.
	const bool bEclipse = FMath::FRand() * 100.0f < FMath::Clamp(SolarEclipseChancePercent, 0.0f, 100.0f);
	const bool bMercury = FMath::FRand() * 100.0f < FMath::Clamp(MercuryTransitChancePercent, 0.0f, 100.0f);
	const bool bVenus = FMath::FRand() * 100.0f < FMath::Clamp(VenusTransitChancePercent, 0.0f, 100.0f);

	TArray<int32> FreeDays;
	TArray<int32> EclipseDays;

	for (int32 Day = 1; Day <= 48; ++Day)
	{
		// Preserve all eight Sabbats on days 1, 7, 13, ... 43.
		if ((Day - 1) % 6 == 0)
		{
			continue;
		}

		FreeDays.Add(Day);
		// A solar eclipse can happen only on New Moon (day 1 of any zodiac month).
		if ((Day - 1) % 4 == 0)
		{
			EclipseDays.Add(Day);
		}
	}

	if (bEclipse && EclipseDays.Num() > 0)
	{
		Plan.SolarEclipseDay = EclipseDays[FMath::RandHelper(EclipseDays.Num())];
		FreeDays.Remove(Plan.SolarEclipseDay);
	}

	// The planets transit the Sun independently of the lunar phase.
	// Distinct dates make each alignment an occasion of its own.
	if (bMercury && FreeDays.Num() > 0)
	{
		const int32 Index = FMath::RandHelper(FreeDays.Num());
		Plan.MercuryTransitDay = FreeDays[Index];
		FreeDays.RemoveAtSwap(Index);
	}

	if (bVenus && FreeDays.Num() > 0)
	{
		const int32 Index = FMath::RandHelper(FreeDays.Num());
		Plan.VenusTransitDay = FreeDays[Index];
	}

	CelestialYearPlans.Add(Year, Plan);
}

FText UTavernSubsystem::GetCurrentCalendarDebugText() const
{
	// Intentionally plain text for temporary calendar prototyping.
	// The finalized calendar widget can use GetCurrentLunarDate() instead.
	static const TCHAR* const ZodiacLabels[] = {
		TEXT("Aries"), TEXT("Taurus"), TEXT("Gemini"), TEXT("Cancer"),
		TEXT("Leo"), TEXT("Virgo"), TEXT("Libra"), TEXT("Scorpio"),
		TEXT("Sagittarius"), TEXT("Capricorn"), TEXT("Aquarius"), TEXT("Pisces")
	};
	static const TCHAR* const ElementLabels[] = {
		TEXT("Fire"), TEXT("Earth"), TEXT("Air"), TEXT("Water")
	};
	static const TCHAR* const MoonLabels[] = {
		TEXT("New Moon"), TEXT("Waxing Moon"), TEXT("Full Moon"), TEXT("Waning Moon")
	};
	static const TCHAR* const SeasonLabels[] = {
		TEXT("Spring"), TEXT("Summer"), TEXT("Autumn"), TEXT("Winter")
	};
	static const TCHAR* const FestivalLabels[] = {
		TEXT("None"), TEXT("Ostara (Spring Equinox)"), TEXT("Beltane"),
		TEXT("Litha (Summer Solstice)"), TEXT("Lughnasadh (First Harvest)"),
		TEXT("Mabon (Autumn Equinox)"), TEXT("Samhain"), TEXT("Yule (Winter Solstice)"),
		TEXT("Imbolc")
	};

	const FTavernLunarDate D = GetCurrentLunarDate();
	const int32 Sun = FMath::Clamp(static_cast<int32>(D.SunSign), 0, 11);
	const int32 Moon = FMath::Clamp(static_cast<int32>(D.MoonSign), 0, 11);
	const int32 SunElement = FMath::Clamp(static_cast<int32>(D.SunElement), 0, 3);
	const int32 MoonElement = FMath::Clamp(static_cast<int32>(D.MoonElement), 0, 3);
	const int32 Phase = FMath::Clamp(static_cast<int32>(D.MoonPhase), 0, 3);
	const int32 Season = FMath::Clamp(static_cast<int32>(D.Season), 0, 3);
	const int32 Festival = FMath::Clamp(static_cast<int32>(D.Festival), 0, 8);

	const FString Info = FString::Printf(
		TEXT("YEAR %d  |  %s  |  DAY %d/48\n")
		TEXT("%s MONTH  |  DAY %d/4  |  GAME DAY %d\n\n")
		TEXT("SUN (PRIMARY): %s  |  %s\n")
		TEXT("MOON: %s  |  %s  |  %s\n\n")
		TEXT("FESTIVAL: %s\n")
		TEXT("SEASON ELEMENT: TBD (design decision)\n")
		TEXT("ROOM DIRECTIONS (planned): N Earth | E Air | S Fire | W Water"),
		D.Year, SeasonLabels[Season], D.DayOfYear,
		ZodiacLabels[Sun], D.DayOfMonth, D.DayNr,
		ZodiacLabels[Sun], ElementLabels[SunElement],
		MoonLabels[Phase], ZodiacLabels[Moon], ElementLabels[MoonElement],
		FestivalLabels[Festival]
	);
	const FTavernCelestialYearPlan Plan = GetCurrentCelestialYearPlan();
	static const TCHAR* const CelestialLabels[] = {
		TEXT("None"), TEXT("Solar Eclipse"), TEXT("Mercury Transit"), TEXT("Venus Transit")
	};
	const int32 EventIndex = FMath::Clamp(static_cast<int32>(GetTodaysCelestialEvent()), 0, 3);
	const FString SkyInfo = FString::Printf(
		TEXT("\n\nSKY TODAY: %s\n")
		TEXT("YEAR PLAN (0 = no event): ECLIPSE DAY %d | MERCURY DAY %d | VENUS DAY %d"),
		CelestialLabels[EventIndex],
		Plan.SolarEclipseDay, Plan.MercuryTransitDay, Plan.VenusTransitDay
	);
	return FText::FromString(Info + SkyInfo);
}

void UTavernSubsystem::SetDayNr(int32 NewDayNr)
{
	NewDayNr = FMath::Max(1, NewDayNr);

	if (State.DayNr == NewDayNr)
	{
		return;
	}

	State.DayNr = NewDayNr;

	// Roll the next year's sky once, before anyone receives OnDayChanged.
	EnsureCelestialYearPlan(GetCurrentLunarDate().Year);
	OnDayChanged.Broadcast(State.DayNr);
	OnLunarDateChanged.Broadcast(GetCurrentLunarDate());
}

void UTavernSubsystem::AdvanceDay()
{
	if (State.DayNr < MAX_int32)
	{
		SetDayNr(State.DayNr + 1);
	}
}

int32 UTavernSubsystem::ClampVibe(int32 InVibe) const
{
	return FMath::Clamp(InVibe, VibeMin, VibeMax);
}

int32 UTavernSubsystem::ClampCleanliness(int32 InCleanliness) const
{
	return FMath::Clamp(InCleanliness, CleanlinessMin, CleanlinessMax);
}

void UTavernSubsystem::SetTimeOfDay(ETavernTimeOfDay NewTime)
{
	if (State.TimeOfDay == NewTime)
	{
		return;
	}

	State.TimeOfDay = NewTime;
	OnTimeChanged.Broadcast(State.TimeOfDay);
}

void UTavernSubsystem::FlipTimeOfDay()
{
	if (State.TimeOfDay == ETavernTimeOfDay::Day)
	{
		SetTimeOfDay(ETavernTimeOfDay::Night);
	}
	else
	{
		AdvanceDay();
		SetTimeOfDay(ETavernTimeOfDay::Day);
	}
}

void UTavernSubsystem::SetVibe(int32 NewVibe)
{
	const int32 Clamped = ClampVibe(NewVibe);
	if (State.Vibe == Clamped)
	{
		return;
	}

	State.Vibe = Clamped;
	OnVibeChanged.Broadcast(State.Vibe);
}

void UTavernSubsystem::AddVibe(int32 Delta)
{
	const int64 Result = static_cast<int64>(State.Vibe) + Delta;
	SetVibe(static_cast<int32>(FMath::Clamp(Result, static_cast<int64>(FMath::Min(VibeMin, VibeMax)), static_cast<int64>(FMath::Max(VibeMin, VibeMax)))));
}

void UTavernSubsystem::SetCleanliness(int32 NewCleanliness)
{
	const int32 Clamped = ClampCleanliness(NewCleanliness);
	if (State.Cleanliness == Clamped)
	{
		return;
	}

	State.Cleanliness = Clamped;
	OnCleanlinessChanged.Broadcast(State.Cleanliness);
}

void UTavernSubsystem::AddCleanliness(int32 Delta)
{
	const int64 Result = static_cast<int64>(State.Cleanliness) + Delta;
	SetCleanliness(static_cast<int32>(FMath::Clamp(Result, static_cast<int64>(FMath::Min(CleanlinessMin, CleanlinessMax)), static_cast<int64>(FMath::Max(CleanlinessMin, CleanlinessMax)))));
}

int32 UTavernSubsystem::GetIngredientCount(ETavernIngredient Type) const
{
	if (const int32* Found = Inventory.Find(Type))
	{
		return *Found;
	}
	return 0;
}

bool UTavernSubsystem::HasIngredient(ETavernIngredient Type, int32 Amount) const
{
	return Amount >= 0 && GetIngredientCount(Type) >= Amount;
}

void UTavernSubsystem::AddIngredient(ETavernIngredient Type, int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	const int32 NewValue = static_cast<int32>(FMath::Min(static_cast<int64>(GetIngredientCount(Type)) + Amount, static_cast<int64>(MAX_int32)));
	if (NewValue == GetIngredientCount(Type)) return;
	Inventory.FindOrAdd(Type) = NewValue;
	OnInventoryChanged.Broadcast();
}

bool UTavernSubsystem::ConsumeIngredient(ETavernIngredient Type, int32 Amount)
{
	if (Amount < 0) return false;
	if (Amount == 0) return true;

	const int32 Current = GetIngredientCount(Type);
	if (Current < Amount)
	{
		return false;
	}

	Inventory.FindOrAdd(Type) = Current - Amount;
	OnInventoryChanged.Broadcast();
	return true;
}

int32 UTavernSubsystem::GetInventorySlotCount() const
{
	return ItemSlots.Num();
}

FTavernInventorySlot UTavernSubsystem::GetInventorySlot(int32 SlotIndex) const
{
	return ItemSlots.IsValidIndex(SlotIndex) ? ItemSlots[SlotIndex] : FTavernInventorySlot();
}

TArray<FTavernInventorySlot> UTavernSubsystem::GetInventorySlots() const
{
	return ItemSlots;
}

bool UTavernSubsystem::HasInventoryItem(FName ItemID, int32 Quantity) const
{
	if (ItemID.IsNone() || Quantity < 0)
	{
		return false;
	}
	if (Quantity == 0)
	{
		return true;
	}

	int64 Total = 0;
	for (const FTavernInventorySlot& Slot : ItemSlots)
	{
		if (Slot.ItemID == ItemID)
		{
			Total += Slot.Quantity;
			if (Total >= Quantity)
			{
				return true;
			}
		}
	}
	return false;
}

bool UTavernSubsystem::AddInventoryItem(FName ItemID, int32 Quantity, int32 MaxStackSize)
{
	if (ItemID.IsNone() || Quantity <= 0 || MaxStackSize <= 0)
	{
		return false;
	}

	// Work on a copy so a full inventory cannot partially accept a pickup.
	TArray<FTavernInventorySlot> Updated = ItemSlots;
	int32 Remaining = Quantity;

	for (FTavernInventorySlot& Slot : Updated)
	{
		if (Remaining == 0)
		{
			break;
		}
		if (Slot.ItemID == ItemID && Slot.Quantity > 0 && Slot.Quantity < Slot.MaxStackSize)
		{
			const int32 Added = FMath::Min(Remaining, Slot.MaxStackSize - Slot.Quantity);
			Slot.Quantity += Added;
			Remaining -= Added;
		}
	}

	for (FTavernInventorySlot& Slot : Updated)
	{
		if (Remaining == 0)
		{
			break;
		}
		if (Slot.ItemID.IsNone() || Slot.Quantity == 0)
		{
			const int32 Added = FMath::Min(Remaining, MaxStackSize);
			Slot.ItemID = ItemID;
			Slot.Quantity = Added;
			Slot.MaxStackSize = MaxStackSize;
			Remaining -= Added;
		}
	}

	if (Remaining != 0)
	{
		return false;
	}

	ItemSlots = MoveTemp(Updated);
	OnInventoryChanged.Broadcast();
	return true;
}

bool UTavernSubsystem::RemoveInventoryItem(FName ItemID, int32 Quantity)
{
	if (ItemID.IsNone() || Quantity <= 0 || !HasInventoryItem(ItemID, Quantity))
	{
		return false;
	}

	int32 Remaining = Quantity;
	for (FTavernInventorySlot& Slot : ItemSlots)
	{
		if (Remaining == 0)
		{
			break;
		}
		if (Slot.ItemID != ItemID)
		{
			continue;
		}
		const int32 Removed = FMath::Min(Remaining, Slot.Quantity);
		Slot.Quantity -= Removed;
		Remaining -= Removed;
		if (Slot.Quantity == 0)
		{
			Slot = FTavernInventorySlot();
		}
	}

	OnInventoryChanged.Broadcast();
	return true;
}

bool UTavernSubsystem::RemoveInventoryItemAtSlot(int32 SlotIndex, int32 Quantity)
{
	if (!ItemSlots.IsValidIndex(SlotIndex) || Quantity <= 0)
	{
		return false;
	}

	FTavernInventorySlot& Slot = ItemSlots[SlotIndex];
	if (Slot.ItemID.IsNone() || Slot.Quantity < Quantity)
	{
		return false;
	}

	Slot.Quantity -= Quantity;
	if (Slot.Quantity == 0)
	{
		Slot = FTavernInventorySlot();
	}
	OnInventoryChanged.Broadcast();
	return true;
}

bool UTavernSubsystem::MoveInventoryItem(int32 FromSlot, int32 ToSlot)
{
	if (!ItemSlots.IsValidIndex(FromSlot) || !ItemSlots.IsValidIndex(ToSlot) ||
		FromSlot == ToSlot || ItemSlots[FromSlot].ItemID.IsNone())
	{
		return false;
	}

	FTavernInventorySlot& Source = ItemSlots[FromSlot];
	FTavernInventorySlot& Target = ItemSlots[ToSlot];

	if (!Target.ItemID.IsNone() && Source.ItemID == Target.ItemID)
	{
		const int32 Room = FMath::Max(0, Target.MaxStackSize - Target.Quantity);
		const int32 Moved = FMath::Min(Room, Source.Quantity);
		if (Moved == 0)
		{
			return false;
		}
		Target.Quantity += Moved;
		Source.Quantity -= Moved;
		if (Source.Quantity == 0)
		{
			Source = FTavernInventorySlot();
		}
	}
	else
	{
		Swap(Source, Target);
	}

	OnInventoryChanged.Broadcast();
	return true;
}

bool UTavernSubsystem::SplitInventoryStack(int32 FromSlot, int32 ToSlot)
{
	if (!ItemSlots.IsValidIndex(FromSlot) || !ItemSlots.IsValidIndex(ToSlot) || FromSlot == ToSlot)
	{
		return false;
	}

	FTavernInventorySlot& Source = ItemSlots[FromSlot];
	FTavernInventorySlot& Target = ItemSlots[ToSlot];
	if (Source.ItemID.IsNone() || Source.Quantity <= 1 || Source.MaxStackSize <= 0)
	{
		return false;
	}

	const bool bTargetEmpty = Target.ItemID.IsNone() || Target.Quantity == 0;
	if (!bTargetEmpty && Target.ItemID != Source.ItemID)
	{
		// A split never swaps with a different item.
		return false;
	}

	const int32 HalfRoundedUp = Source.Quantity / 2 + Source.Quantity % 2;
	const int32 DestinationCapacity = bTargetEmpty
		? Source.MaxStackSize
		: FMath::Max(0, Target.MaxStackSize - Target.Quantity);
	const int32 TransferAmount = FMath::Min(HalfRoundedUp, DestinationCapacity);
	if (TransferAmount <= 0)
	{
		return false;
	}

	if (bTargetEmpty)
	{
		Target = Source;
		Target.Quantity = TransferAmount;
	}
	else
	{
		Target.Quantity += TransferAmount;
	}
	Source.Quantity -= TransferAmount;
	OnInventoryChanged.Broadcast();
	return true;
}

int32 UTavernSubsystem::GetPreparedFood() const
{
	return State.PreparedFood;
}

void UTavernSubsystem::AddPreparedFood(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}
	const int32 NewValue = static_cast<int32>(FMath::Min(static_cast<int64>(State.PreparedFood) + Amount, static_cast<int64>(MAX_int32)));
	if (NewValue == State.PreparedFood) return;
	State.PreparedFood = NewValue;
	OnPreparedFoodChanged.Broadcast(State.PreparedFood);
}

bool UTavernSubsystem::ConsumePreparedFood(int32 Amount)
{
	if (Amount < 0) return false;
	if (Amount == 0) return true;

	if (State.PreparedFood < Amount)
	{
		return false;
	}

	State.PreparedFood -= Amount;
	OnPreparedFoodChanged.Broadcast(State.PreparedFood);
	return true;
}

void UTavernSubsystem::ClearInventory()
{
	bool bChanged = false;
	for (auto& Pair : Inventory)
	{
		bChanged |= (Pair.Value != 0);
		Pair.Value = 0;
	}

	for (FTavernInventorySlot& Slot : ItemSlots)
	{
		bChanged |= !Slot.ItemID.IsNone() || Slot.Quantity != 0;
		Slot = FTavernInventorySlot();
	}

	// Notify once after every ingredient or grid slot has been cleared.
	if (bChanged)
	{
		OnInventoryChanged.Broadcast();
	}
}

int32 UTavernSubsystem::GetCoins() const
{
	return State.Coins;
}

void UTavernSubsystem::AddCoins(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}
	const int32 NewValue = static_cast<int32>(FMath::Min(static_cast<int64>(State.Coins) + Amount, static_cast<int64>(MAX_int32)));
	if (NewValue == State.Coins) return;
	State.Coins = NewValue;
	OnCoinsChanged.Broadcast(State.Coins);
}

bool UTavernSubsystem::SpendCoins(int32 Amount)
{
	if (Amount < 0) return false;
	if (Amount == 0) return true;

	if (State.Coins < Amount)
	{
		return false;
	}

	State.Coins -= Amount;
	OnCoinsChanged.Broadcast(State.Coins);
	return true;
}

void UTavernSubsystem::ResetRun(bool bResetDayNr)
{
	State.TimeOfDay = ETavernTimeOfDay::Day;

	if (bResetDayNr)
	{
		State.DayNr = 1;
	}

	State.Vibe = 0;
	State.Cleanliness = 0;

	State.Coins = 10;
	State.PreparedFood = 0;

	for (auto& Pair : Inventory)
	{
		Pair.Value = 0;
	}
	for (FTavernInventorySlot& Slot : ItemSlots)
	{
		Slot = FTavernInventorySlot();
	}

	// A new run gets a fresh sky, including when DayNr is intentionally retained.
	CelestialYearPlans.Empty();
	EnsureCelestialYearPlan(GetCurrentLunarDate().Year);

	// All run data is reset before observers clear or rebuild their own state.
	OnRunReset.Broadcast();
	OnTimeChanged.Broadcast(State.TimeOfDay);
	OnDayChanged.Broadcast(State.DayNr);
	OnLunarDateChanged.Broadcast(GetCurrentLunarDate());
	OnVibeChanged.Broadcast(State.Vibe);
	OnCleanlinessChanged.Broadcast(State.Cleanliness);
	OnInventoryChanged.Broadcast();
	OnCoinsChanged.Broadcast(State.Coins);
	OnPreparedFoodChanged.Broadcast(State.PreparedFood);
}
