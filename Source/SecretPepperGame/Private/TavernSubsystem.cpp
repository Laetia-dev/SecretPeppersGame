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

	State.Coins = 10;
	State.PreparedFood = 0;
}

void UTavernSubsystem::Deinitialize()
{
	Inventory.Empty();
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
	return FText::FromString(Info);
}

void UTavernSubsystem::SetDayNr(int32 NewDayNr)
{
	NewDayNr = FMath::Max(1, NewDayNr);

	if (State.DayNr == NewDayNr)
	{
		return;
	}

	State.DayNr = NewDayNr;
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

	// Notify once after every ingredient has been cleared.
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
