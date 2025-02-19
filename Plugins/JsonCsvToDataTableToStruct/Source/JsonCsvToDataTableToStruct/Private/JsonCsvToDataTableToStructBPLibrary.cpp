/************************************************************************/
/* Author: YWT20                                                        */
/* Expected release year : 2020                                         */
/************************************************************************/

#include "JsonCsvToDataTableToStructBPLibrary.h"
#include "JsonCsvToDataTableToStruct.h"
#include "JsonObjectConverter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "UObject/UnrealType.h"
#include "UObject/EnumProperty.h"
#include "Misc/FileHelper.h"
#include "DataTableUtils.h"
#include "Engine/DataTable.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/UserDefinedStruct.h"

#if PLATFORM_MAC
#include "Mac/MacPlatformApplicationMisc.h"
#elif PLATFORM_WINDOWS
#include "Windows/WindowsPlatformApplicationMisc.h"
#elif PLATFORM_LINUX
#include "Linux/LinuxPlatformApplicationMisc.h"
#elif PLATFORM_ANDROID
#include "Android/AndroidPlatformApplicationMisc.h"
#elif PLATFORM_IOS
#include "IOS/IOSPlatformApplicationMisc.h"
#endif 
#include "Kismet/KismetArrayLibrary.h"


UJsonCsvToDataTableToStructBPLibrary::UJsonCsvToDataTableToStructBPLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

bool UJsonCsvToDataTableToStructBPLibrary::LoadFileToString(const FString& FileAbsolutePath, FString& Content)
{
	return FFileHelper::LoadFileToString(Content, *FileAbsolutePath);
}

bool UJsonCsvToDataTableToStructBPLibrary::SaveStringToFile(const FString& FileAbsolutePath, const FString& Content, EEncodingOptions EncodingOptions)
{
	return FFileHelper::SaveStringToFile(Content, *FileAbsolutePath, (FFileHelper::EEncodingOptions)((uint8)EncodingOptions));
}

UDataTable* UJsonCsvToDataTableToStructBPLibrary::ReadJsonToDataTable(const FString& InData, UScriptStruct* TableType)
{
	if (InData.IsEmpty() || !TableType) return nullptr;

	FString newInData = InData;

	UDataTable* OutDataTable = NewObject<UDataTable>(GWorld, FName(FGuid::NewGuid().ToString()));

	if (!OutDataTable) return nullptr;

	OutDataTable->RowStruct = TableType;

	TArray<FString> OutError = OutDataTable->CreateTableFromJSONString(newInData);

	if (OutError.Num() != 0)
	{
		newInData.InsertAt(newInData.Find("{") + 1, "\r\n\t\t\"Name\": \"_MyTempName\",");
		newInData = "[" + newInData + "]";
		OutError = OutDataTable->CreateTableFromJSONString(newInData);
	}

	if (OutError.Num() != 0) return nullptr;

	return OutDataTable;
}

UDataTable* UJsonCsvToDataTableToStructBPLibrary::ReadCsvToDataTable(const FString& InData, UScriptStruct* TableType)
{
	if (InData.IsEmpty() || !TableType) return nullptr;

	FString newInData = InData;

#if !WITH_EDITOR
	auto* ptr = TableType->Children;
	FString LName, DisplayName, TFrom, TTo, Lstr, Rstr;
	InData.Split("\n", &Lstr, &Rstr);

	while (ptr)
	{
		LName = ptr->GetName();
		DisplayName = ptr->GetAuthoredName();

		TFrom = ("," + DisplayName + ",");
		TTo = ("," + LName + ",");

		if (!Lstr.Contains(TFrom))
		{
			TFrom = ("," + DisplayName + '\n');
			TTo = ("," + LName + '\n');
		}

		Lstr = Lstr.Replace(*TFrom, *TTo);
		ptr = ptr->Next;
	}
	Lstr.AppendChar('\n');
	newInData = Lstr.Append(Rstr);
#endif //WITH_EDITOR

	UDataTable* OutDataTable = NewObject<UDataTable>(GWorld, FName(FGuid::NewGuid().ToString()));

	if (!OutDataTable) return nullptr;

	OutDataTable->RowStruct = TableType;

	TArray<FString> OutError = OutDataTable->CreateTableFromCSVString(newInData);

	if (OutError.Num() != 0) return nullptr;

	return OutDataTable;
}

bool UJsonCsvToDataTableToStructBPLibrary::GetDataTableFirstRow(UDataTable* InDataTable, FTableRowBase& OutRow)
{
	// We should never hit this!  stubs to avoid NoExport on the class.
	check(0);
	return false;
}

bool UJsonCsvToDataTableToStructBPLibrary::Generic_GetDataTableFirstRow(UDataTable* InDataTable, UScriptStruct* TableType, void* OutRowPtr)
{
	if (!InDataTable) return false;

	void* RowPtr = InDataTable->FindRowUnchecked(InDataTable->GetRowNames()[0]);

	if (!RowPtr) return false;

	UScriptStruct* StructType = InDataTable->RowStruct;

	if (!StructType) return false;

	StructType->CopyScriptStruct(OutRowPtr, RowPtr);

	return true;
}

bool UJsonCsvToDataTableToStructBPLibrary::RemoveDataTableRow(UDataTable* InDataTable, const FName RowName)
{
	if (RowName.IsNone()) return false;
	InDataTable->RemoveRow(RowName);
	return true;
}

bool UJsonCsvToDataTableToStructBPLibrary::AddDataTableRow(UDataTable* InDataTable, const FName RowName, const FTableRowBase& OutRow)
{
	// We should never hit this!  stubs to avoid NoExport on the class.
	check(0);
	return false;
}

bool UJsonCsvToDataTableToStructBPLibrary::Generic_AddDataTableRow(UDataTable* InDataTable, const FName RowName, void* OutRowPtr)
{
	if (!InDataTable || RowName.IsNone()) return false;

	UScriptStruct* StructType = InDataTable->RowStruct;

	if (!StructType) return false;

	uint8* NewRawRowData = (uint8*)FMemory::Malloc(StructType->GetStructureSize());

	StructType->InitializeStruct(NewRawRowData);
	StructType->CopyScriptStruct(NewRawRowData, OutRowPtr);

	InDataTable->AddRow(RowName, *(FTableRowBase*)NewRawRowData);
	return true;
}

bool UJsonCsvToDataTableToStructBPLibrary::StructToJsonObjectString(FString& OutJsonString, const UScriptStruct* Struct)
{
	// We should never hit this!  stubs to avoid NoExport on the class.
	check(0);
	return false;
}

bool UJsonCsvToDataTableToStructBPLibrary::Generic_StructToJsonObjectString(FString& OutJsonString, UScriptStruct* StructDefinition, void* Struct)
{
	if (FJsonObjectConverter::UStructToJsonObjectString(StructDefinition, Struct, OutJsonString, 0, 0))
	{
		UDataTable* tempDT = ReadJsonToDataTable(OutJsonString, StructDefinition);
		if (!tempDT) return false;
		WriteDataTableToJson(tempDT, OutJsonString);
		FString Lstr, Rstr;
		OutJsonString.Split(",", &Lstr, &Rstr);
		OutJsonString = "{" + Rstr.Left(Rstr.Len() - 1);
		return true;
	}
	return false;
}


bool UJsonCsvToDataTableToStructBPLibrary::StructArrayToJsonObjectString(FString& OutJsonString, const TArray<UScriptStruct*> StructArray)
{
	// We should never hit this!  stubs to avoid NoExport on the class.
	check(0);
	return false;
}

bool UJsonCsvToDataTableToStructBPLibrary::Generic_StructArrayToJsonObjectString(FString& OutJsonString, FArrayProperty* StructDefinition, void* Struct)
{
	if (!StructDefinition || !Struct) return false;

	UScriptStruct* StructType = CastField<FStructProperty>(StructDefinition->Inner)->Struct;

	if (!StructType) return false;

	UDataTable* InDataTable = NewObject<UDataTable>(GWorld, FName(FGuid::NewGuid().ToString()));
	if (!InDataTable) return false;
	InDataTable->RowStruct = StructType;

	FScriptArrayHelper ArrayHelper(StructDefinition, Struct);

	for (int32 i = 0; i < ArrayHelper.Num(); i++)
	{
		uint8* NewRawRowData = (uint8*)FMemory::Malloc(StructType->GetStructureSize());

		StructType->InitializeStruct(NewRawRowData);
		StructType->CopyScriptStruct(NewRawRowData, ArrayHelper.GetRawPtr(i));

		InDataTable->AddRow(FName(*FString::FromInt(i)), *(FTableRowBase*)NewRawRowData);
	}

	return WriteDataTableToJson(InDataTable, OutJsonString);
}

bool UJsonCsvToDataTableToStructBPLibrary::JsonStringToStruct(FString InJsonString, FTableRowBase& OutRow)
{
	// We should never hit this!  stubs to avoid NoExport on the class.
	check(0);
	return false;
}

bool UJsonCsvToDataTableToStructBPLibrary::Generic_JsonStringToStruct(FString InJsonString, UScriptStruct* TableType, void* OutRowPtr)
{
	if (InJsonString.IsEmpty()) return false;

	UDataTable* InDataTable = NewObject<UDataTable>(GWorld, FName(FGuid::NewGuid().ToString()));

	if (!InDataTable) return false;

	InDataTable->RowStruct = TableType;

	TArray<FString> OutError = InDataTable->CreateTableFromJSONString(InJsonString);
	if (OutError.Num() != 0) {
		InJsonString.InsertAt(InJsonString.Find("{") + 1, "\r\n\t\t\"Name\": \"_MyTempName\",");
		InJsonString = "[" + InJsonString + "]";
		OutError = InDataTable->CreateTableFromJSONString(InJsonString);
	}
	if (OutError.Num() != 0) {
		InJsonString = InJsonString.Replace(TEXT("\":null"), TEXT("\":\"\""));
		OutError = InDataTable->CreateTableFromJSONString(InJsonString);
	}
	if (OutError.Num() != 0) return false;

	void* RowPtr = InDataTable->FindRowUnchecked(InDataTable->GetRowNames()[0]);

	if (!RowPtr) return false;

	UScriptStruct* StructType = InDataTable->RowStruct;

	if (!StructType) return false;

	StructType->CopyScriptStruct(OutRowPtr, RowPtr);

	return true;
}


bool UJsonCsvToDataTableToStructBPLibrary::JsonStringToStructArray(FString InJsonString, TArray<FTableRowBase>& OutRows)
{
	// We should never hit this!  stubs to avoid NoExport on the class.
	check(0);
	return false;
}

bool UJsonCsvToDataTableToStructBPLibrary::Generic_JsonStringToStructArray(FString InJsonString, FArrayProperty* TableType, void* OutRowPtr)
{
	if (!TableType || !OutRowPtr) return false;

	UScriptStruct* StructType = CastField<FStructProperty>(TableType->Inner)->Struct;

	if (!StructType) return false;

	UDataTable* InDataTable = NewObject<UDataTable>(GWorld, FName(FGuid::NewGuid().ToString()));
	if (!InDataTable) return false;
	InDataTable->RowStruct = StructType;

	TArray<FString> OutError = InDataTable->CreateTableFromJSONString(InJsonString);
	if (OutError.Num() != 0) {
		InJsonString.InsertAt(InJsonString.Find("{") + 1, "\r\n\t\t\"Name\": \"_MyTempName\",");
		InJsonString = "[" + InJsonString + "]";
		OutError = InDataTable->CreateTableFromJSONString(InJsonString);
	}
	if (OutError.Num() != 0) {
		InJsonString = InJsonString.Replace(TEXT("\":null"), TEXT("\":\"\""));
		OutError = InDataTable->CreateTableFromJSONString(InJsonString);
	}
	if (OutError.Num() != 0) return false;


	FScriptArrayHelper ArrayHelper(TableType, OutRowPtr);

	TArray<FName> RowNames = InDataTable->GetRowNames();

	if (RowNames.Num() == 0) return false;

	for (auto& tName : RowNames)
	{
		void* RowPtr = InDataTable->FindRowUnchecked(tName);

		if (!RowPtr) continue;

		UKismetArrayLibrary::GenericArray_Add(OutRowPtr, TableType, RowPtr);
	}
	return true;
}

bool UJsonCsvToDataTableToStructBPLibrary::WriteDataTableToCsv(UDataTable* InDataTable, FString& ExportedText)
{
	if (!InDataTable->RowStruct)
	{
		return false;
	}

	ExportedText.Empty();

	// Write the header (column titles)
	FString ImportKeyField;
	if (!InDataTable->ImportKeyField.IsEmpty())
	{
		// Write actual name if we have it
		ImportKeyField = InDataTable->ImportKeyField;
		ExportedText += ImportKeyField;
	}
	else
	{
		ExportedText += TEXT("---");
	}

	FProperty* SkipProperty = nullptr;
	for (TFieldIterator<FProperty> It(InDataTable->RowStruct); It; ++It)
	{
		FProperty* BaseProp = *It;
		check(BaseProp);

		FString ColumnHeader = DataTableUtils::GetPropertyExportName(BaseProp);

		if (ColumnHeader == ImportKeyField)
		{
			// Don't write header again if this is the name field, and save for skipping later
			SkipProperty = BaseProp;
			continue;
		}

		ExportedText += TEXT(",");
		ExportedText += ColumnHeader;
	}
	ExportedText += TEXT("\n");

	// Write each row
	for (auto RowIt = InDataTable->GetRowMap().CreateConstIterator(); RowIt; ++RowIt)
	{
		FName RowName = RowIt.Key();
		ExportedText += RowName.ToString();

		uint8* RowData = RowIt.Value();

		//WriteRow(InDataTable.RowStruct, RowData);
		{
			if (!InDataTable->RowStruct)
			{
				return false;
			}

			for (TFieldIterator<FProperty> It(InDataTable->RowStruct); It; ++It)
			{
				FProperty* BaseProp = *It;
				check(BaseProp);

				if (BaseProp == SkipProperty)
				{
					continue;
				}

				const void* Data = BaseProp->ContainerPtrToValuePtr<void>(RowData, 0);

				//WriteStructEntry(InRowData, BaseProp, Data);
				{
					ExportedText += TEXT(",");
					const FString PropertyValue = DataTableUtils::GetPropertyValueAsString(BaseProp, (uint8*)RowData, EDataTableExportFlags::None);
					ExportedText += TEXT("\"");
					ExportedText += PropertyValue.Replace(TEXT("\""), TEXT("\"\""));
					ExportedText += TEXT("\"");
				}
			}
		}

		ExportedText += TEXT("\n");
	}

	return true;
}

bool UJsonCsvToDataTableToStructBPLibrary::WriteDataTableToJson(UDataTable* InDataTable, FString& ExportedText)
{
	if (!FYDataTableExporterJSON(EDataTableExportFlags::UseJsonObjectsForStructs, ExportedText).WriteTable(*InDataTable))
	{
		ExportedText = TEXT("Missing RowStruct!\n");
		return false;
	}
	return true;
}

void UJsonCsvToDataTableToStructBPLibrary::CopyMessageToClipboard(FString text)
{
	FPlatformApplicationMisc::ClipboardCopy(*text);
}

FString UJsonCsvToDataTableToStructBPLibrary::PasteMessageFromClipboard()
{
	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);
	return ClipboardContent;
}




namespace
{
	const TCHAR* JSONTypeToString(const EJson InType)
	{
		switch (InType)
		{
		case EJson::None:
			return TEXT("None");
		case EJson::Null:
			return TEXT("Null");
		case EJson::String:
			return TEXT("String");
		case EJson::Number:
			return TEXT("Number");
		case EJson::Boolean:
			return TEXT("Boolean");
		case EJson::Array:
			return TEXT("Array");
		case EJson::Object:
			return TEXT("Object");
		default:
			return TEXT("Unknown");
		}
	}

	template <typename CharType>
	void WriteJSONObjectStartWithOptionalIdentifier(typename TYDataTableExporterJSON<CharType>::FDataTableJsonWriter& InJsonWriter, const FString* InIdentifier)
	{
		if (InIdentifier)
		{
			InJsonWriter.WriteObjectStart(*InIdentifier);
		}
		else
		{
			InJsonWriter.WriteObjectStart();
		}
	}

	template <typename CharType, typename ValueType>
	void WriteJSONValueWithOptionalIdentifier(typename TYDataTableExporterJSON<CharType>::FDataTableJsonWriter& InJsonWriter, const FString* InIdentifier, const ValueType InValue)
	{
		if (InIdentifier)
		{
			InJsonWriter.WriteValue(*InIdentifier, InValue);
		}
		else
		{
			InJsonWriter.WriteValue(InValue);
		}
	}

}

FString DataTableJSONUtils::YGetKeyFieldName(const UDataTable& InDataTable)
{
	FString ExplicitString = InDataTable.ImportKeyField;
	if (ExplicitString.IsEmpty())
	{
		return TEXT("Name");
	}
	else
	{
		return ExplicitString;
	}
}



FYDataTableExporterJSON::FYDataTableExporterJSON(const EDataTableExportFlags InDTExportFlags, FString& OutExportText)
	: TYDataTableExporterJSON<TCHAR>(InDTExportFlags, TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&OutExportText))
{
	bJsonWriterNeedsClose = true;
}

template<typename CharType>
TYDataTableExporterJSON<CharType>::TYDataTableExporterJSON(const EDataTableExportFlags InDTExportFlags, TSharedRef<FDataTableJsonWriter> InJsonWriter)
	: DTExportFlags(InDTExportFlags)
	, JsonWriter(InJsonWriter)
	, bJsonWriterNeedsClose(false)
{
}

template<typename CharType>
TYDataTableExporterJSON<CharType>::~TYDataTableExporterJSON()
{
	if (bJsonWriterNeedsClose)
	{
		JsonWriter->Close();
	}
}

template<typename CharType>
bool TYDataTableExporterJSON<CharType>::WriteTable(const UDataTable& InDataTable)
{
	if (!InDataTable.RowStruct)
	{
		return false;
	}

	FString KeyField = DataTableJSONUtils::YGetKeyFieldName(InDataTable);
	JsonWriter->WriteArrayStart();

	// Iterate over rows
	for (auto RowIt = InDataTable.GetRowMap().CreateConstIterator(); RowIt; ++RowIt)
	{
		JsonWriter->WriteObjectStart();
		{
			// RowName
			const FName RowName = RowIt.Key();
			JsonWriter->WriteValue(KeyField, RowName.ToString());

			// Now the values
			uint8* RowData = RowIt.Value();
			WriteRow(InDataTable.RowStruct, RowData, &KeyField);
		}
		JsonWriter->WriteObjectEnd();
	}

	JsonWriter->WriteArrayEnd();

	return true;
}

template<typename CharType>
bool TYDataTableExporterJSON<CharType>::WriteTableAsObject(const UDataTable& InDataTable)
{
	if (!InDataTable.RowStruct)
	{
		return false;
	}

	JsonWriter->WriteObjectStart(InDataTable.GetName());

	// Iterate over rows
	for (auto RowIt = InDataTable.GetRowMap().CreateConstIterator(); RowIt; ++RowIt)
	{
		// RowName
		const FName RowName = RowIt.Key();
		JsonWriter->WriteObjectStart(RowName.ToString());
		{
			// Now the values
			uint8* RowData = RowIt.Value();
			WriteRow(InDataTable.RowStruct, RowData);
		}
		JsonWriter->WriteObjectEnd();
	}
	JsonWriter->WriteObjectEnd();

	return true;
}

template<typename CharType>
bool TYDataTableExporterJSON<CharType>::WriteRow(const UScriptStruct* InRowStruct, const void* InRowData, const FString* FieldToSkip)
{
	if (!InRowStruct)
	{
		return false;
	}

	return WriteStruct(InRowStruct, InRowData, FieldToSkip);
}

template<typename CharType>
bool TYDataTableExporterJSON<CharType>::WriteStruct(const UScriptStruct* InStruct, const void* InStructData, const FString* FieldToSkip)
{
	for (TFieldIterator<const FProperty> It(InStruct); It; ++It)
	{
		const FProperty* BaseProp = *It;
		check(BaseProp);

		const FString Identifier = DataTableUtils::GetPropertyExportName(BaseProp, DTExportFlags);
		if (FieldToSkip && *FieldToSkip == Identifier)
		{
			// Skip this field
			continue;
		}

		if (BaseProp->ArrayDim == 1)
		{
			const void* Data = BaseProp->ContainerPtrToValuePtr<void>(InStructData, 0);
			WriteStructEntry(InStructData, BaseProp, Data);
		}
		else
		{
			JsonWriter->WriteArrayStart(Identifier);

			for (int32 ArrayEntryIndex = 0; ArrayEntryIndex < BaseProp->ArrayDim; ++ArrayEntryIndex)
			{
				const void* Data = BaseProp->ContainerPtrToValuePtr<void>(InStructData, ArrayEntryIndex);
				WriteContainerEntry(BaseProp, Data);
			}

			JsonWriter->WriteArrayEnd();
		}
	}

	return true;
}

template<typename CharType>
bool TYDataTableExporterJSON<CharType>::WriteStructEntry(const void* InRowData, const FProperty* InProperty, const void* InPropertyData)
{
	const FString Identifier = DataTableUtils::GetPropertyExportName(InProperty, DTExportFlags);

	if (const FEnumProperty* EnumProp = CastField<const FEnumProperty>(InProperty))
	{
		const FString PropertyValue = DataTableUtils::GetPropertyValueAsString(EnumProp, (uint8*)InRowData, DTExportFlags);
		JsonWriter->WriteValue(Identifier, PropertyValue);
	}
	else if (const FNumericProperty* NumProp = CastField<const FNumericProperty>(InProperty))
	{
		if (NumProp->IsEnum())
		{
			const FString PropertyValue = DataTableUtils::GetPropertyValueAsString(InProperty, (uint8*)InRowData, DTExportFlags);
			JsonWriter->WriteValue(Identifier, PropertyValue);
		}
		else if (NumProp->IsInteger())
		{
			const int64 PropertyValue = NumProp->GetSignedIntPropertyValue(InPropertyData);
			JsonWriter->WriteValue(Identifier, PropertyValue);
		}
		else
		{
			const double PropertyValue = NumProp->GetFloatingPointPropertyValue(InPropertyData);
			JsonWriter->WriteValue(Identifier, PropertyValue);
		}
	}
	else if (const FBoolProperty* BoolProp = CastField<const FBoolProperty>(InProperty))
	{
		const bool PropertyValue = BoolProp->GetPropertyValue(InPropertyData);
		JsonWriter->WriteValue(Identifier, PropertyValue);
	}
	else if (const FArrayProperty* ArrayProp = CastField<const FArrayProperty>(InProperty))
	{
		JsonWriter->WriteArrayStart(Identifier);

		FScriptArrayHelper ArrayHelper(ArrayProp, InPropertyData);
		for (int32 ArrayEntryIndex = 0; ArrayEntryIndex < ArrayHelper.Num(); ++ArrayEntryIndex)
		{
			const uint8* ArrayEntryData = ArrayHelper.GetRawPtr(ArrayEntryIndex);
			WriteContainerEntry(ArrayProp->Inner, ArrayEntryData);
		}

		JsonWriter->WriteArrayEnd();
	}
	else if (const FSetProperty* SetProp = CastField<const FSetProperty>(InProperty))
	{
		JsonWriter->WriteArrayStart(Identifier);

		FScriptSetHelper SetHelper(SetProp, InPropertyData);
		for (int32 SetSparseIndex = 0; SetSparseIndex < SetHelper.GetMaxIndex(); ++SetSparseIndex)
		{
			if (SetHelper.IsValidIndex(SetSparseIndex))
			{
				const uint8* SetEntryData = SetHelper.GetElementPtr(SetSparseIndex);
				WriteContainerEntry(SetHelper.GetElementProperty(), SetEntryData);
			}
		}

		JsonWriter->WriteArrayEnd();
	}
	else if (const FMapProperty* MapProp = CastField<const FMapProperty>(InProperty))
	{
		JsonWriter->WriteObjectStart(Identifier);

		FScriptMapHelper MapHelper(MapProp, InPropertyData);
		for (int32 MapSparseIndex = 0; MapSparseIndex < MapHelper.GetMaxIndex(); ++MapSparseIndex)
		{
			if (MapHelper.IsValidIndex(MapSparseIndex))
			{
				const uint8* MapKeyData = MapHelper.GetKeyPtr(MapSparseIndex);
				const uint8* MapValueData = MapHelper.GetValuePtr(MapSparseIndex);

				// JSON object keys must always be strings
				const FString KeyValue = DataTableUtils::GetPropertyValueAsStringDirect(MapHelper.GetKeyProperty(), (uint8*)MapKeyData, DTExportFlags);
				WriteContainerEntry(MapHelper.GetValueProperty(), MapValueData, &KeyValue);
			}
		}

		JsonWriter->WriteObjectEnd();
	}
	else if (const FStructProperty* StructProp = CastField<const FStructProperty>(InProperty))
	{
		if (!!(DTExportFlags & EDataTableExportFlags::UseJsonObjectsForStructs))
		{
			JsonWriter->WriteObjectStart(Identifier);
			WriteStruct(StructProp->Struct, InPropertyData);
			JsonWriter->WriteObjectEnd();
		}
		else
		{
			const FString PropertyValue = DataTableUtils::GetPropertyValueAsString(InProperty, (uint8*)InRowData, DTExportFlags);
			JsonWriter->WriteValue(Identifier, PropertyValue);
		}
	}
	else
	{
		const FString PropertyValue = DataTableUtils::GetPropertyValueAsString(InProperty, (uint8*)InRowData, DTExportFlags);
		JsonWriter->WriteValue(Identifier, PropertyValue);
	}

	return true;
}

template<typename CharType>
bool TYDataTableExporterJSON<CharType>::WriteContainerEntry(const FProperty* InProperty, const void* InPropertyData, const FString* InIdentifier)
{
	if (const FEnumProperty* EnumProp = CastField<const FEnumProperty>(InProperty))
	{
		const FString PropertyValue = DataTableUtils::GetPropertyValueAsStringDirect(InProperty, (uint8*)InPropertyData, DTExportFlags);
		WriteJSONValueWithOptionalIdentifier<CharType>(*JsonWriter, InIdentifier, PropertyValue);
	}
	else if (const FNumericProperty* NumProp = CastField<const FNumericProperty>(InProperty))
	{
		if (NumProp->IsEnum())
		{
			const FString PropertyValue = DataTableUtils::GetPropertyValueAsStringDirect(InProperty, (uint8*)InPropertyData, DTExportFlags);
			WriteJSONValueWithOptionalIdentifier<CharType>(*JsonWriter, InIdentifier, PropertyValue);
		}
		else if (NumProp->IsInteger())
		{
			const int64 PropertyValue = NumProp->GetSignedIntPropertyValue(InPropertyData);
			WriteJSONValueWithOptionalIdentifier<CharType>(*JsonWriter, InIdentifier, PropertyValue);
		}
		else
		{
			const double PropertyValue = NumProp->GetFloatingPointPropertyValue(InPropertyData);
			WriteJSONValueWithOptionalIdentifier<CharType>(*JsonWriter, InIdentifier, PropertyValue);
		}
	}
	else if (const FBoolProperty* BoolProp = CastField<const FBoolProperty>(InProperty))
	{
		const bool PropertyValue = BoolProp->GetPropertyValue(InPropertyData);
		WriteJSONValueWithOptionalIdentifier<CharType>(*JsonWriter, InIdentifier, PropertyValue);
	}
	else if (const FStructProperty* StructProp = CastField<const FStructProperty>(InProperty))
	{
		if (!!(DTExportFlags & EDataTableExportFlags::UseJsonObjectsForStructs))
		{
			WriteJSONObjectStartWithOptionalIdentifier<CharType>(*JsonWriter, InIdentifier);
			WriteStruct(StructProp->Struct, InPropertyData);
			JsonWriter->WriteObjectEnd();
		}
		else
		{
			const FString PropertyValue = DataTableUtils::GetPropertyValueAsStringDirect(InProperty, (uint8*)InPropertyData, DTExportFlags);
			WriteJSONValueWithOptionalIdentifier<CharType>(*JsonWriter, InIdentifier, PropertyValue);
		}
	}
	else if (const FArrayProperty* ArrayProp = CastField<const FArrayProperty>(InProperty))
	{
		// Cannot nest arrays
		return false;
	}
	else if (const FSetProperty* SetProp = CastField<const FSetProperty>(InProperty))
	{
		// Cannot nest sets
		return false;
	}
	else if (const FMapProperty* MapProp = CastField<const FMapProperty>(InProperty))
	{
		// Cannot nest maps
		return false;
	}
	else
	{
		const FString PropertyValue = DataTableUtils::GetPropertyValueAsStringDirect(InProperty, (uint8*)InPropertyData, DTExportFlags);
		WriteJSONValueWithOptionalIdentifier<CharType>(*JsonWriter, InIdentifier, PropertyValue);
	}

	return true;
}

template class TYDataTableExporterJSON<TCHAR>;
template class TYDataTableExporterJSON<ANSICHAR>;


FYDataTableImporterJSON::FYDataTableImporterJSON(UDataTable& InDataTable, const FString& InJSONData, TArray<FString>& OutProblems)
	: DataTable(&InDataTable)
	, JSONData(InJSONData)
	, ImportProblems(OutProblems)
{
}

FYDataTableImporterJSON::~FYDataTableImporterJSON()
{
}

bool FYDataTableImporterJSON::YReadTable()
{
	if (JSONData.IsEmpty())
	{
		ImportProblems.Add(TEXT("Input data is empty."));
		return false;
	}

	// Check we have a RowStruct specified
	if (!DataTable->RowStruct)
	{
		ImportProblems.Add(TEXT("No RowStruct specified."));
		return false;
	}

	TArray< TSharedPtr<FJsonValue> > ParsedTableRows;
	{
		const TSharedRef< TJsonReader<TCHAR> > JsonReader = TJsonReaderFactory<TCHAR>::Create(JSONData);
		if (!FJsonSerializer::Deserialize(JsonReader, ParsedTableRows) || ParsedTableRows.Num() == 0)
		{
			ImportProblems.Add(FString::Printf(TEXT("Failed to parse the JSON data. Error: %s"), *JsonReader->GetErrorMessage()));
			return false;
		}
	}

	// Empty existing data
	DataTable->EmptyTable();

	// Iterate over rows
	for (int32 RowIdx = 0; RowIdx < ParsedTableRows.Num(); ++RowIdx)
	{
		const TSharedPtr<FJsonValue>& ParsedTableRowValue = ParsedTableRows[RowIdx];
		TSharedPtr<FJsonObject> ParsedTableRowObject = ParsedTableRowValue->AsObject();
		if (!ParsedTableRowObject.IsValid())
		{
			ImportProblems.Add(FString::Printf(TEXT("Row '%d' is not a valid JSON object."), RowIdx));
			continue;
		}

		YReadRow(ParsedTableRowObject.ToSharedRef(), RowIdx);
	}

	DataTable->Modify(true);

	return true;
}

bool FYDataTableImporterJSON::YReadRow(const TSharedRef<FJsonObject>& InParsedTableRowObject, const int32 InRowIdx)
{
	// Get row name
	FString RowKey = DataTableJSONUtils::YGetKeyFieldName(*DataTable);
	FName RowName = DataTableUtils::MakeValidName(InParsedTableRowObject->GetStringField(RowKey));

	// Check its not 'none'
	if (RowName.IsNone())
	{
		ImportProblems.Add(FString::Printf(TEXT("Row '%d' missing key field '%s'."), InRowIdx, *RowKey));
		return false;
	}

	// Check its not a duplicate
	if (!DataTable->AllowDuplicateRowsOnImport() && DataTable->GetRowMap().Find(RowName) != nullptr)
	{
		ImportProblems.Add(FString::Printf(TEXT("Duplicate row name '%s'."), *RowName.ToString()));
		return false;
	}

	// Allocate data to store information, using UScriptStruct to know its size
	uint8* RowData = (uint8*)FMemory::Malloc(DataTable->RowStruct->GetStructureSize());
	DataTable->RowStruct->InitializeStruct(RowData);
	// And be sure to call DestroyScriptStruct later

	// Add to row map
	//DataTable->AddRowInternal(RowName, RowData);
	DataTable->AddRow(RowName, *(FTableRowBase*)RowData);
	return YReadStruct(InParsedTableRowObject, DataTable->RowStruct, RowName, RowData);
}

bool FYDataTableImporterJSON::YReadStruct(const TSharedRef<FJsonObject>& InParsedObject, UScriptStruct* InStruct, const FName InRowName, void* InStructData)
{
	// Now read in each property
	for (TFieldIterator<FProperty> It(InStruct); It; ++It)
	{
		FProperty* BaseProp = *It;
		check(BaseProp);

		const FString ColumnName = DataTableUtils::GetPropertyExportName(BaseProp);

		TSharedPtr<FJsonValue> ParsedPropertyValue;
		for (const FString& PropertyName : DataTableUtils::GetPropertyImportNames(BaseProp))
		{
			ParsedPropertyValue = InParsedObject->TryGetField(PropertyName);
			if (ParsedPropertyValue.IsValid())
			{
				break;
			}
		}

		if (!ParsedPropertyValue.IsValid())
		{
			// If the structure has specified the property as optional for import (gameplay code likely doing a custom fix-up or parse of that property),
			// then avoid warning about it
			static const FName DataTableImportOptionalMetadataKey(TEXT("DataTableImportOptional"));
			//if (BaseProp->HasMetaData(DataTableImportOptionalMetadataKey))
			//{
			//	continue;
			//}

			if (!DataTable->bIgnoreMissingFields)
			{
				ImportProblems.Add(FString::Printf(TEXT("Row '%s' is missing an entry for '%s'."), *InRowName.ToString(), *ColumnName));
			}

			continue;
		}

		if (BaseProp->ArrayDim == 1)
		{
			void* Data = BaseProp->ContainerPtrToValuePtr<void>(InStructData, 0);
			YReadStructEntry(ParsedPropertyValue.ToSharedRef(), InRowName, ColumnName, InStructData, BaseProp, Data);
		}
		else
		{
			const TCHAR* const ParsedPropertyType = JSONTypeToString(ParsedPropertyValue->Type);

			const TArray< TSharedPtr<FJsonValue> >* PropertyValuesPtr;
			if (!ParsedPropertyValue->TryGetArray(PropertyValuesPtr))
			{
				ImportProblems.Add(FString::Printf(TEXT("Property '%s' on row '%s' is the incorrect type. Expected Array, got %s."), *ColumnName, *InRowName.ToString(), ParsedPropertyType));
				return false;
			}

			if (BaseProp->ArrayDim != PropertyValuesPtr->Num())
			{
				ImportProblems.Add(FString::Printf(TEXT("Property '%s' on row '%s' is a static sized array with %d elements, but we have %d values to import"), *ColumnName, *InRowName.ToString(), BaseProp->ArrayDim, PropertyValuesPtr->Num()));
			}

			for (int32 ArrayEntryIndex = 0; ArrayEntryIndex < BaseProp->ArrayDim; ++ArrayEntryIndex)
			{
				if (PropertyValuesPtr->IsValidIndex(ArrayEntryIndex))
				{
					void* Data = BaseProp->ContainerPtrToValuePtr<void>(InStructData, ArrayEntryIndex);
					const TSharedPtr<FJsonValue>& PropertyValueEntry = (*PropertyValuesPtr)[ArrayEntryIndex];
					YReadContainerEntry(PropertyValueEntry.ToSharedRef(), InRowName, ColumnName, ArrayEntryIndex, BaseProp, Data);
				}
			}
		}
	}

	return true;
}

bool FYDataTableImporterJSON::YReadStructEntry(const TSharedRef<FJsonValue>& InParsedPropertyValue, const FName InRowName, const FString& InColumnName, const void* InRowData, FProperty* InProperty, void* InPropertyData)
{
	const TCHAR* const ParsedPropertyType = JSONTypeToString(InParsedPropertyValue->Type);

	if (FEnumProperty* EnumProp = CastField<FEnumProperty>(InProperty))
	{
		FString EnumValue;
		if (InParsedPropertyValue->TryGetString(EnumValue))
		{
			FString Error = DataTableUtils::AssignStringToProperty(EnumValue, InProperty, (uint8*)InRowData);
			if (!Error.IsEmpty())
			{
				ImportProblems.Add(FString::Printf(TEXT("Property '%s' on row '%s' has invalid enum value: %s."), *InColumnName, *InRowName.ToString(), *EnumValue));
				return false;
			}
		}
		else
		{
			int64 PropertyValue = 0;
			if (!InParsedPropertyValue->TryGetNumber(PropertyValue))
			{
				ImportProblems.Add(FString::Printf(TEXT("Property '%s' on row '%s' is the incorrect type. Expected Integer, got %s."), *InColumnName, *InRowName.ToString(), ParsedPropertyType));
				return false;
			}

			EnumProp->GetUnderlyingProperty()->SetIntPropertyValue(InPropertyData, PropertyValue);
		}
	}
	else if (FNumericProperty* NumProp = CastField<FNumericProperty>(InProperty))
	{
		FString EnumValue;
		if (NumProp->IsEnum() && InParsedPropertyValue->TryGetString(EnumValue))
		{
			FString Error = DataTableUtils::AssignStringToProperty(EnumValue, InProperty, (uint8*)InRowData);
			if (!Error.IsEmpty())
			{
				ImportProblems.Add(FString::Printf(TEXT("Property '%s' on row '%s' has invalid enum value: %s."), *InColumnName, *InRowName.ToString(), *EnumValue));
				return false;
			}
		}
		else if (NumProp->IsInteger())
		{
			int64 PropertyValue = 0;
			if (!InParsedPropertyValue->TryGetNumber(PropertyValue))
			{
				ImportProblems.Add(FString::Printf(TEXT("Property '%s' on row '%s' is the incorrect type. Expected Integer, got %s."), *InColumnName, *InRowName.ToString(), ParsedPropertyType));
				return false;
			}

			NumProp->SetIntPropertyValue(InPropertyData, PropertyValue);
		}
		else
		{
			double PropertyValue = 0.0;
			if (!InParsedPropertyValue->TryGetNumber(PropertyValue))
			{
				ImportProblems.Add(FString::Printf(TEXT("Property '%s' on row '%s' is the incorrect type. Expected Double, got %s."), *InColumnName, *InRowName.ToString(), ParsedPropertyType));
				return false;
			}

			NumProp->SetFloatingPointPropertyValue(InPropertyData, PropertyValue);
		}
	}
	else if (FBoolProperty* BoolProp = CastField<FBoolProperty>(InProperty))
	{
		bool PropertyValue = false;
		if (!InParsedPropertyValue->TryGetBool(PropertyValue))
		{
			ImportProblems.Add(FString::Printf(TEXT("Property '%s' on row '%s' is the incorrect type. Expected Boolean, got %s."), *InColumnName, *InRowName.ToString(), ParsedPropertyType));
			return false;
		}

		BoolProp->SetPropertyValue(InPropertyData, PropertyValue);
	}
	else if (FArrayProperty* ArrayProp = CastField<FArrayProperty>(InProperty))
	{
		const TArray< TSharedPtr<FJsonValue> >* PropertyValuesPtr;
		if (!InParsedPropertyValue->TryGetArray(PropertyValuesPtr))
		{
			ImportProblems.Add(FString::Printf(TEXT("Property '%s' on row '%s' is the incorrect type. Expected Array, got %s."), *InColumnName, *InRowName.ToString(), ParsedPropertyType));
			return false;
		}

		FScriptArrayHelper ArrayHelper(ArrayProp, InPropertyData);
		ArrayHelper.EmptyValues();
		for (const TSharedPtr<FJsonValue>& PropertyValueEntry : *PropertyValuesPtr)
		{
			const int32 NewEntryIndex = ArrayHelper.AddValue();
			uint8* ArrayEntryData = ArrayHelper.GetRawPtr(NewEntryIndex);
			YReadContainerEntry(PropertyValueEntry.ToSharedRef(), InRowName, InColumnName, NewEntryIndex, ArrayProp->Inner, ArrayEntryData);
		}
	}
	else if (FSetProperty* SetProp = CastField<FSetProperty>(InProperty))
	{
		const TArray< TSharedPtr<FJsonValue> >* PropertyValuesPtr;
		if (!InParsedPropertyValue->TryGetArray(PropertyValuesPtr))
		{
			ImportProblems.Add(FString::Printf(TEXT("Property '%s' on row '%s' is the incorrect type. Expected Array, got %s."), *InColumnName, *InRowName.ToString(), ParsedPropertyType));
			return false;
		}

		FScriptSetHelper SetHelper(SetProp, InPropertyData);
		SetHelper.EmptyElements();
		for (const TSharedPtr<FJsonValue>& PropertyValueEntry : *PropertyValuesPtr)
		{
			const int32 NewEntryIndex = SetHelper.AddDefaultValue_Invalid_NeedsRehash();
			uint8* SetEntryData = SetHelper.GetElementPtr(NewEntryIndex);
			YReadContainerEntry(PropertyValueEntry.ToSharedRef(), InRowName, InColumnName, NewEntryIndex, SetHelper.GetElementProperty(), SetEntryData);
		}
		SetHelper.Rehash();
	}
	else if (FMapProperty* MapProp = CastField<FMapProperty>(InProperty))
	{
		const TSharedPtr<FJsonObject>* PropertyValue;
		if (!InParsedPropertyValue->TryGetObject(PropertyValue))
		{
			ImportProblems.Add(FString::Printf(TEXT("Property '%s' on row '%s' is the incorrect type. Expected Object, got %s."), *InColumnName, *InRowName.ToString(), ParsedPropertyType));
			return false;
		}

		FScriptMapHelper MapHelper(MapProp, InPropertyData);
		MapHelper.EmptyValues();
		for (const auto& PropertyValuePair : (*PropertyValue)->Values)
		{
			const int32 NewEntryIndex = MapHelper.AddDefaultValue_Invalid_NeedsRehash();
			uint8* MapKeyData = MapHelper.GetKeyPtr(NewEntryIndex);
			uint8* MapValueData = MapHelper.GetValuePtr(NewEntryIndex);

			// JSON object keys are always strings
			const FString KeyError = DataTableUtils::AssignStringToPropertyDirect(PropertyValuePair.Key, MapHelper.GetKeyProperty(), MapKeyData);
			if (KeyError.Len() > 0)
			{
				MapHelper.RemoveAt(NewEntryIndex);
				ImportProblems.Add(FString::Printf(TEXT("Problem assigning key '%s' to property '%s' on row '%s' : %s"), *PropertyValuePair.Key, *InColumnName, *InRowName.ToString(), *KeyError));
				return false;
			}

			if (!YReadContainerEntry(PropertyValuePair.Value.ToSharedRef(), InRowName, InColumnName, NewEntryIndex, MapHelper.GetValueProperty(), MapValueData))
			{
				MapHelper.RemoveAt(NewEntryIndex);
				return false;
			}
		}
		MapHelper.Rehash();
	}
	else if (FStructProperty* StructProp = CastField<FStructProperty>(InProperty))
	{
		const TSharedPtr<FJsonObject>* PropertyValue = nullptr;
		if (InParsedPropertyValue->TryGetObject(PropertyValue))
		{
			return YReadStruct(PropertyValue->ToSharedRef(), StructProp->Struct, InRowName, InPropertyData);
		}
		else
		{
			// If the JSON does not contain a JSON object for this struct, we try to use the backwards-compatible string deserialization, same as the "else" block below
			FString PropertyValueString;
			if (!InParsedPropertyValue->TryGetString(PropertyValueString))
			{
				ImportProblems.Add(FString::Printf(TEXT("Property '%s' on row '%s' is the incorrect type. Expected String, got %s."), *InColumnName, *InRowName.ToString(), ParsedPropertyType));
				return false;
			}

			const FString Error = DataTableUtils::AssignStringToProperty(PropertyValueString, InProperty, (uint8*)InRowData);
			if (Error.Len() > 0)
			{
				ImportProblems.Add(FString::Printf(TEXT("Problem assigning string '%s' to property '%s' on row '%s' : %s"), *PropertyValueString, *InColumnName, *InRowName.ToString(), *Error));
				return false;
			}

			return true;
		}
	}
	else
	{
		FString PropertyValue;
		if (!InParsedPropertyValue->TryGetString(PropertyValue))
		{
			ImportProblems.Add(FString::Printf(TEXT("Property '%s' on row '%s' is the incorrect type. Expected String, got %s."), *InColumnName, *InRowName.ToString(), ParsedPropertyType));
			return false;
		}

		const FString Error = DataTableUtils::AssignStringToProperty(PropertyValue, InProperty, (uint8*)InRowData);
		if (Error.Len() > 0)
		{
			ImportProblems.Add(FString::Printf(TEXT("Problem assigning string '%s' to property '%s' on row '%s' : %s"), *PropertyValue, *InColumnName, *InRowName.ToString(), *Error));
			return false;
		}
	}

	return true;
}

bool FYDataTableImporterJSON::YReadContainerEntry(const TSharedRef<FJsonValue>& InParsedPropertyValue, const FName InRowName, const FString& InColumnName, const int32 InArrayEntryIndex, FProperty* InProperty, void* InPropertyData)
{
	const TCHAR* const ParsedPropertyType = JSONTypeToString(InParsedPropertyValue->Type);

	if (FEnumProperty* EnumProp = CastField<FEnumProperty>(InProperty))
	{
		FString EnumValue;
		if (InParsedPropertyValue->TryGetString(EnumValue))
		{
			FString Error = DataTableUtils::AssignStringToPropertyDirect(EnumValue, InProperty, (uint8*)InPropertyData);
			if (!Error.IsEmpty())
			{
				ImportProblems.Add(FString::Printf(TEXT("Entry %d on property '%s' on row '%s' has invalid enum value: %s."), InArrayEntryIndex, *InColumnName, *InRowName.ToString(), *EnumValue));
				return false;
			}
		}
		else
		{
			int64 PropertyValue = 0;
			if (!InParsedPropertyValue->TryGetNumber(PropertyValue))
			{
				ImportProblems.Add(FString::Printf(TEXT("Entry %d on property '%s' on row '%s' is the incorrect type. Expected Integer, got %s."), InArrayEntryIndex, *InColumnName, *InRowName.ToString(), ParsedPropertyType));
				return false;
			}

			EnumProp->GetUnderlyingProperty()->SetIntPropertyValue(InPropertyData, PropertyValue);
		}
	}
	else if (FNumericProperty* NumProp = CastField<FNumericProperty>(InProperty))
	{
		FString EnumValue;
		if (NumProp->IsEnum() && InParsedPropertyValue->TryGetString(EnumValue))
		{
			FString Error = DataTableUtils::AssignStringToPropertyDirect(EnumValue, InProperty, (uint8*)InPropertyData);
			if (!Error.IsEmpty())
			{
				ImportProblems.Add(FString::Printf(TEXT("Entry %d on property '%s' on row '%s' has invalid enum value: %s."), InArrayEntryIndex, *InColumnName, *InRowName.ToString(), *EnumValue));
				return false;
			}
		}
		else if (NumProp->IsInteger())
		{
			int64 PropertyValue = 0;
			if (!InParsedPropertyValue->TryGetNumber(PropertyValue))
			{
				ImportProblems.Add(FString::Printf(TEXT("Entry %d on property '%s' on row '%s' is the incorrect type. Expected Integer, got %s."), InArrayEntryIndex, *InColumnName, *InRowName.ToString(), ParsedPropertyType));
				return false;
			}

			NumProp->SetIntPropertyValue(InPropertyData, PropertyValue);
		}
		else
		{
			double PropertyValue = 0.0;
			if (!InParsedPropertyValue->TryGetNumber(PropertyValue))
			{
				ImportProblems.Add(FString::Printf(TEXT("Entry %d on property '%s' on row '%s' is the incorrect type. Expected Double, got %s."), InArrayEntryIndex, *InColumnName, *InRowName.ToString(), ParsedPropertyType));
				return false;
			}

			NumProp->SetFloatingPointPropertyValue(InPropertyData, PropertyValue);
		}
	}
	else if (FBoolProperty* BoolProp = CastField<FBoolProperty>(InProperty))
	{
		bool PropertyValue = false;
		if (!InParsedPropertyValue->TryGetBool(PropertyValue))
		{
			ImportProblems.Add(FString::Printf(TEXT("Entry %d on property '%s' on row '%s' is the incorrect type. Expected Boolean, got %s."), InArrayEntryIndex, *InColumnName, *InRowName.ToString(), ParsedPropertyType));
			return false;
		}

		BoolProp->SetPropertyValue(InPropertyData, PropertyValue);
	}
	else if (FArrayProperty* ArrayProp = CastField<FArrayProperty>(InProperty))
	{
		// Cannot nest arrays
		return false;
	}
	else if (FSetProperty* SetProp = CastField<FSetProperty>(InProperty))
	{
		// Cannot nest sets
		return false;
	}
	else if (FMapProperty* MapProp = CastField<FMapProperty>(InProperty))
	{
		// Cannot nest maps
		return false;
	}
	else if (FStructProperty* StructProp = CastField<FStructProperty>(InProperty))
	{
		const TSharedPtr<FJsonObject>* PropertyValue = nullptr;
		if (InParsedPropertyValue->TryGetObject(PropertyValue))
		{
			return YReadStruct(PropertyValue->ToSharedRef(), StructProp->Struct, InRowName, InPropertyData);
		}
		else
		{
			// If the JSON does not contain a JSON object for this struct, we try to use the backwards-compatible string deserialization, same as the "else" block below
			FString PropertyValueString;
			if (!InParsedPropertyValue->TryGetString(PropertyValueString))
			{
				ImportProblems.Add(FString::Printf(TEXT("Property '%s' on row '%s' is the incorrect type. Expected String, got %s."), *InColumnName, *InRowName.ToString(), ParsedPropertyType));
				return false;
			}

			const FString Error = DataTableUtils::AssignStringToPropertyDirect(PropertyValueString, InProperty, (uint8*)InPropertyData);
			if (Error.Len() > 0)
			{
				ImportProblems.Add(FString::Printf(TEXT("Problem assigning string '%s' to entry %d on property '%s' on row '%s' : %s"), InArrayEntryIndex, *PropertyValueString, *InColumnName, *InRowName.ToString(), *Error));
				return false;
			}

			return true;
		}
	}
	else
	{
		FString PropertyValue;
		if (!InParsedPropertyValue->TryGetString(PropertyValue))
		{
			ImportProblems.Add(FString::Printf(TEXT("Entry %d on property '%s' on row '%s' is the incorrect type. Expected String, got %s."), InArrayEntryIndex, *InColumnName, *InRowName.ToString(), ParsedPropertyType));
			return false;
		}

		const FString Error = DataTableUtils::AssignStringToPropertyDirect(PropertyValue, InProperty, (uint8*)InPropertyData);
		if (Error.Len() > 0)
		{
			ImportProblems.Add(FString::Printf(TEXT("Problem assigning string '%s' to entry %d on property '%s' on row '%s' : %s"), InArrayEntryIndex, *PropertyValue, *InColumnName, *InRowName.ToString(), *Error));
			return false;
		}
	}

	return true;
}

