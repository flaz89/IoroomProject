#pragma once

#include "CoreMinimal.h"                                                                                                                                                                                                
#include "IoroomTypes.generated.h"

UENUM(BlueprintType)
enum class EGridSnap : uint8
{
	Minimum,	// 1cm
	Fine,		// 5cm
	Small,		// 20cm
	Medium,		// 50cm
	Large		// 100cm
};

UENUM(BlueprintType)
enum class ERotationSnap : uint8
{
	Minimum,	// 1°
	Fine,		// 5°
	Small,		// 15°
	Medium,		// 45°
	Large		// 90°
};

