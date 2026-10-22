#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TavernLunarCalendarLibrary.generated.h"

// Tarot Tavern's four-day fantasy lunar cycle.
UENUM(BlueprintType)
enum class ETavernMoonPhase : uint8
{
    NewMoon UMETA(DisplayName = "New Moon"),
    WaxingMoon UMETA(DisplayName = "Waxing Moon"),
    FullMoon UMETA(DisplayName = "Full Moon"),
    WaningMoon UMETA(DisplayName = "Waning Moon")
};

// Zodiac signs, in their conventional order starting with Aries.
UENUM(BlueprintType)
enum class ETavernZodiacSign : uint8
{
    Aries,
    Taurus,
    Gemini,
    Cancer,
    Leo,
    Virgo,
    Libra,
    Scorpio,
    Sagittarius,
    Capricorn,
    Aquarius,
    Pisces
};

// Lunar gardening element associated with the Moon's zodiac sign.
UENUM(BlueprintType)
enum class ETavernLunarElement : uint8
{
    Fire,
    Earth,
    Air,
    Water
};

USTRUCT(BlueprintType)
struct FTavernLunarDate
{
    GENERATED_BODY()

    // Absolute game day. This is the same day number as TavernSubsystem::GetDayNr.
    UPROPERTY(BlueprintReadOnly, Category = "Tavern|Calendar")
    int32 DayNr = 1;

    UPROPERTY(BlueprintReadOnly, Category = "Tavern|Calendar")
    int32 Year = 1;

    UPROPERTY(BlueprintReadOnly, Category = "Tavern|Calendar")
    int32 Month = 1;

    UPROPERTY(BlueprintReadOnly, Category = "Tavern|Calendar")
    int32 DayOfMonth = 1;

    UPROPERTY(BlueprintReadOnly, Category = "Tavern|Calendar")
    ETavernMoonPhase MoonPhase = ETavernMoonPhase::NewMoon;

    UPROPERTY(BlueprintReadOnly, Category = "Tavern|Calendar")
    ETavernZodiacSign SunSign = ETavernZodiacSign::Aries;

    UPROPERTY(BlueprintReadOnly, Category = "Tavern|Calendar")
    ETavernZodiacSign MoonSign = ETavernZodiacSign::Aries;

    UPROPERTY(BlueprintReadOnly, Category = "Tavern|Calendar")
    ETavernLunarElement MoonElement = ETavernLunarElement::Fire;
};

/**
 * Pure date calculations. Does not store state, advance time, or spawn/unload rooms.
 * Use TavernSubsystem::GetDayNr as input in Blueprints.
 */
UCLASS()
class SECRETPEPPERGAME_API UTavernLunarCalendarLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "Tavern|Calendar", meta = (DisplayName = "Get Lunar Calendar Date"))
    static FTavernLunarDate GetLunarCalendarDate(int32 DayNr);
};
