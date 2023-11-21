// © 2021 Uuvana Studios Oy. All Rights Reserved.


#include "LongvinterFunctionsCPP.h"
#include "HighResScreenshot.h"

int64 ULongvinterFunctionsCPP::ToUnixTimestampBPC(FDateTime DateTime) {
	return DateTime.ToUnixTimestamp();
}

FDateTime ULongvinterFunctionsCPP::FromUnixTimestampBPC(int64 Timestamp) {
	return FDateTime::FromUnixTimestamp(Timestamp);
}

FString ULongvinterFunctionsCPP::DateTimeToString(FDateTime DateTime) {
	return DateTime.ToString();
}