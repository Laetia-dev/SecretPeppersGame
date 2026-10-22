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

// The four zodiac elements, used for both Sun and Moon influences.
UENUM(BlueprintType)
enum class ETavernLunarElement : uint8
{
    Fire,
    Earth,
    Air,
    Water
};

// Seasons are based on tropical zodiac month, not on element associations.
// Assigning an element to each season is intentionally left for game design.
UENUM(BlueprintType)
enum class ETavernSeason : uint8
{
    Spring,
    Summer,
    Autumn,
    Winter
};

// Eight solar/cross-quarter Sabbats in the compressed 48-day fantasy year.
// This enum identifies calendar dates only; it does not trigger gameplay events.
UENUM(BlueprintType)
enum class ETavernYearFestival : uint8
{
    None UMETA(DisplayName = "None"),
    Ostara UMETA(DisplayName = "Ostara - Spring Equinox"),
    Beltane UMETA(DisplayName = "Beltane"),
    Litha UMETA(DisplayName = "Litha - Summer Solstice"),
    Lughnasadh UMETA(DisplayName = "Lughnasadh - First Harvest"),
    Mabon UMETA(DisplayName = "Mabon - Autumn Equinox"),
    Samhain UMETA(DisplayName = "Samhain"),
    Yule UMETA(DisplayName = "Yule - Winter Solstice"),
    Imbolc UMETA(DisplayName = "Imbolc")
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

    // 1..48, repeats each year.
    UPROPERTY(BlueprintReadOnly, Category = "Tavern|Calendar")
    int32 DayOfYear = 1;

    UPROPERTY(BlueprintReadOnly, Category = "Tavern|Calendar")
    ETavernSeason Season = ETavernSeason::Spring;

    // None on ordinary days. Existing OnLunarDateChanged broadcasts the date.
    UPROPERTY(BlueprintReadOnly, Category = "Tavern|Calendar")
    ETavernYearFestival Festival = ETavernYearFestival::Ostara;

    UPROPERTY(BlueprintReadOnly, Category = "Tavern|Calendar")
    bool bIsFestival = true;

    UPROPERTY(BlueprintReadOnly, Category = "Tavern|Calendar")
    ETavernMoonPhase MoonPhase = ETavernMoonPhase::NewMoon;

    UPROPERTY(BlueprintReadOnly, Category = "Tavern|Calendar")
    ETavernZodiacSign SunSign = ETavernZodiacSign::Aries;

    UPROPERTY(BlueprintReadOnly, Category = "Tavern|Calendar")
    ETavernZodiacSign MoonSign = ETavernZodiacSign::Aries;

    // The month-long dominant elemental influence.
    UPROPERTY(BlueprintReadOnly, Category = "Tavern|Calendar")
    ETavernLunarElement SunElement = ETavernLunarElement::Fire;

    // The daily secondary elemental influence.
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
