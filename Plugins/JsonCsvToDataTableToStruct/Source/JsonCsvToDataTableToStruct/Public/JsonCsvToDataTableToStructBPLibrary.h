/************************************************************************/
/* Author: YWT20                                                        */
/* Expected release year : 2020                                         */
/************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "Policies/PrettyJsonPrintPolicy.h"
#include "Dom/JsonValue.h"
#include "Dom/JsonObject.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "JsonCsvToDataTableToStructBPLibrary.generated.h"



UENUM(BlueprintType)
enum class EEncodingOptions : uint8
{
	AutoDetect,
	ForceAnsi,
	ForceUnicode,
	ForceUTF8,
	ForceUTF8WithoutBOM
};
/*
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu.
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/
UCLASS()
class UJsonCsvToDataTableToStructBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

		UFUNCTION(BlueprintCallable, Category = "Json or csv to datatable to struct")
		static bool LoadFileToString(const FString& FileAbsolutePath, FString& Content);

	UFUNCTION(BlueprintCallable, Category = "Json or csv to datatable to struct")
		static bool SaveStringToFile(const FString& FileAbsolutePath, const FString& Content, EEncodingOptions EncodingOptions);

	UFUNCTION(BlueprintCallable, Category = "Json or csv to datatable to struct")
		static UDataTable* ReadJsonToDataTable(const FString& InData, UScriptStruct* TableType);

	UFUNCTION(BlueprintCallable, Category = "Json or csv to datatable to struct")
		static UDataTable* ReadCsvToDataTable(const FString& InData, UScriptStruct* TableType);

	UFUNCTION(BlueprintCallable, Category = "Json or csv to datatable to struct", CustomThunk, meta = (CustomStructureParam = "OutRow"))
		static bool GetDataTableFirstRow(UDataTable* InDataTable, FTableRowBase& OutRow);
	static bool Generic_GetDataTableFirstRow(UDataTable* InDataTable, UScriptStruct* TableType, void* OutRowPtr);
	DECLARE_FUNCTION(execGetDataTableFirstRow) {
		P_GET_OBJECT(UDataTable, InDataTable);

		Stack.StepCompiledIn<FStructProperty>(NULL);
		void* OutRowPtr = Stack.MostRecentPropertyAddress;

		P_FINISH;
		bool bSuccess = false;

		FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
		if (StructProp && OutRowPtr)
		{
			UScriptStruct* OutputType = StructProp->Struct;

			P_NATIVE_BEGIN;
			bSuccess = Generic_GetDataTableFirstRow(InDataTable, OutputType, OutRowPtr);
			P_NATIVE_END;
		}
		*(bool*)RESULT_PARAM = bSuccess;
	}

	UFUNCTION(BlueprintCallable, Category = "Json or csv to datatable to struct")
		static bool RemoveDataTableRow(UDataTable* InDataTable, const FName RowName);

	UFUNCTION(BlueprintCallable, Category = "Json or csv to datatable to struct", CustomThunk, meta = (CustomStructureParam = "OutRow"))
		static bool AddDataTableRow(UDataTable* InDataTable, const FName RowName, const FTableRowBase& OutRow);
	static bool Generic_AddDataTableRow(UDataTable* InDataTable, const FName RowName, void* OutRowPtr);
	DECLARE_FUNCTION(execAddDataTableRow) {
		P_GET_OBJECT(UDataTable, InDataTable);
		P_GET_PROPERTY_REF(FNameProperty, RowName);

		Stack.StepCompiledIn<FStructProperty>(NULL);
		void* OutRowPtr = Stack.MostRecentPropertyAddress;

		P_FINISH;
		bool bSuccess = false;

		FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
		if (StructProp && OutRowPtr)
		{
			UScriptStruct* OutputType = StructProp->Struct;

			P_NATIVE_BEGIN;
			bSuccess = Generic_AddDataTableRow(InDataTable, RowName, OutRowPtr);
			P_NATIVE_END;
		}
		*(bool*)RESULT_PARAM = bSuccess;
	}

	UFUNCTION(BlueprintCallable, Category = "Json or csv to datatable to struct", CustomThunk, meta = (CustomStructureParam = "Struct"))
		static bool StructToJsonObjectString(FString& OutJsonString, const UScriptStruct* Struct);
	static bool Generic_StructToJsonObjectString(FString& OutJsonString, UScriptStruct* StructDefinition, void* Struct);
	DECLARE_FUNCTION(execStructToJsonObjectString) {
		P_GET_PROPERTY_REF(FStrProperty, OutJsonString);

		Stack.StepCompiledIn<FStructProperty>(NULL);
		void* InStruct = Stack.MostRecentPropertyAddress;

		P_FINISH;
		bool bSuccess = false;

		FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
		if (StructProp && InStruct)
		{
			UScriptStruct* StructType = StructProp->Struct;

			P_NATIVE_BEGIN;
			bSuccess = Generic_StructToJsonObjectString(OutJsonString, StructType, InStruct);
			P_NATIVE_END;
		}
		*(bool*)RESULT_PARAM = bSuccess;
	}

	UFUNCTION(BlueprintCallable, Category = "Json or csv to datatable to struct", CustomThunk, meta = (ArrayParm = "StructArray"))
		static bool StructArrayToJsonObjectString(FString& OutJsonString, const TArray<UScriptStruct*> StructArray);
	static bool Generic_StructArrayToJsonObjectString(FString& OutJsonString, FArrayProperty* StructDefinition, void* Struct);
	DECLARE_FUNCTION(execStructArrayToJsonObjectString) {
		P_GET_PROPERTY_REF(FStrProperty, OutJsonString);

		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(NULL);
		void* SrcArrayAddr = Stack.MostRecentPropertyAddress;
		FArrayProperty* SrcArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

		if (!SrcArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		P_FINISH;
		bool bSuccess = false;

		P_NATIVE_BEGIN;
		bSuccess = Generic_StructArrayToJsonObjectString(OutJsonString, SrcArrayProperty, SrcArrayAddr);
		P_NATIVE_END;

		*(bool*)RESULT_PARAM = bSuccess;
	}

	UFUNCTION(BlueprintCallable, Category = "Json or csv to datatable to struct", CustomThunk, meta = (CustomStructureParam = "OutRow"))
		static bool JsonStringToStruct(FString InJsonString, FTableRowBase& OutRow);
	static bool Generic_JsonStringToStruct(FString InJsonString, UScriptStruct* TableType, void* OutRowPtr);
	DECLARE_FUNCTION(execJsonStringToStruct) {
		P_GET_PROPERTY(FStrProperty, InJsonString);

		Stack.StepCompiledIn<FStructProperty>(NULL);
		void* OutRowPtr = Stack.MostRecentPropertyAddress;

		P_FINISH;
		bool bSuccess = false;

		FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
		if (StructProp && OutRowPtr)
		{
			UScriptStruct* OutputType = StructProp->Struct;

			P_NATIVE_BEGIN;
			bSuccess = Generic_JsonStringToStruct(InJsonString, OutputType, OutRowPtr);
			P_NATIVE_END;
		}
		*(bool*)RESULT_PARAM = bSuccess;
	}

	UFUNCTION(BlueprintCallable, Category = "Json or csv to datatable to struct", CustomThunk, meta = (ArrayParm = "OutRows"))
		static bool JsonStringToStructArray(FString InJsonString, TArray<FTableRowBase>& OutRows);
	static bool Generic_JsonStringToStructArray(FString InJsonString, FArrayProperty* TableType, void* OutRowPtr);
	DECLARE_FUNCTION(execJsonStringToStructArray) {
		P_GET_PROPERTY(FStrProperty, InJsonString);

		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(NULL);
		void* SrcArrayAddr = Stack.MostRecentPropertyAddress;
		FArrayProperty* SrcArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

		if (!SrcArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		P_FINISH;
		bool bSuccess = false;

		if (SrcArrayAddr)
		{
			P_NATIVE_BEGIN;
			bSuccess = Generic_JsonStringToStructArray(InJsonString, SrcArrayProperty, SrcArrayAddr);
			P_NATIVE_END;
		}
		*(bool*)RESULT_PARAM = bSuccess;
	}


	UFUNCTION(BlueprintCallable, Category = "Json or csv to datatable to struct")
		static bool WriteDataTableToCsv(UDataTable* InDataTable, FString& ExportedText);

	UFUNCTION(BlueprintCallable, Category = "Json or csv to datatable to struct")
		static bool WriteDataTableToJson(UDataTable* InDataTable, FString& ExportedText);

	UFUNCTION(BlueprintCallable, Category = "Json or csv to datatable to struct")
		static void CopyMessageToClipboard(FString text);

	UFUNCTION(BlueprintCallable, Category = "Json or csv to datatable to struct")
		static FString PasteMessageFromClipboard();
};


class UDataTable;
enum class EDataTableExportFlags : uint8;

template <class CharType> struct TPrettyJsonPrintPolicy;

// forward declare JSON writer
template <class CharType>
struct TPrettyJsonPrintPolicy;
template <class CharType, class PrintPolicy>
class TJsonWriter;

namespace DataTableJSONUtils
{
	/** Returns what string is used as the key/name field for a data table */
	FString YGetKeyFieldName(const UDataTable& InDataTable);
}


/**
 * Class to serialize a DataTable to a TJsonWriter.
 */
template<typename CharType = TCHAR>
class TYDataTableExporterJSON
{
public:
	typedef TJsonWriter<CharType, TPrettyJsonPrintPolicy<CharType>> FDataTableJsonWriter;

	TYDataTableExporterJSON(const EDataTableExportFlags InDTExportFlags, TSharedRef<FDataTableJsonWriter> InJsonWriter);

	~TYDataTableExporterJSON();

	/** Writes the data table out as an array of objects */
	bool WriteTable(const UDataTable& InDataTable);

	/** Writes the data table out as a named object with each row being a sub value on that object */
	bool WriteTableAsObject(const UDataTable& InDataTable);

	/** Writes out a single row */
	bool WriteRow(const UScriptStruct* InRowStruct, const void* InRowData, const FString* FieldToSkip = nullptr);

	/** Writes the contents of a single row */
	bool WriteStruct(const UScriptStruct* InStruct, const void* InStructData, const FString* FieldToSkip = nullptr);

protected:
	bool WriteStructEntry(const void* InRowData, const FProperty* InProperty, const void* InPropertyData);

	bool WriteContainerEntry(const FProperty* InProperty, const void* InPropertyData, const FString* InIdentifier = nullptr);

	EDataTableExportFlags DTExportFlags;
	TSharedRef<FDataTableJsonWriter> JsonWriter;
	bool bJsonWriterNeedsClose;
};

/**
 * TCHAR-specific instantiation of TDataTableExporterJSON that has a convenience constructor to write output to an FString instead of an external TJsonWriter
 */
class FYDataTableExporterJSON : public TYDataTableExporterJSON<TCHAR>
{
public:
	using TYDataTableExporterJSON<TCHAR>::TYDataTableExporterJSON;
	// 
	FYDataTableExporterJSON(const EDataTableExportFlags InDTExportFlags, FString& OutExportText);
};

class FYDataTableImporterJSON
{
public:
	FYDataTableImporterJSON(UDataTable& InDataTable, const FString& InJSONData, TArray<FString>& OutProblems);

	~FYDataTableImporterJSON();

	bool YReadTable();

private:
	bool YReadRow(const TSharedRef<FJsonObject>& InParsedTableRowObject, const int32 InRowIdx);

	bool YReadStruct(const TSharedRef<FJsonObject>& InParsedObject, UScriptStruct* InStruct, const FName InRowName, void* InStructData);

	bool YReadStructEntry(const TSharedRef<FJsonValue>& InParsedPropertyValue, const FName InRowName, const FString& InColumnName, const void* InRowData, FProperty* InProperty, void* InPropertyData);

	bool YReadContainerEntry(const TSharedRef<FJsonValue>& InParsedPropertyValue, const FName InRowName, const FString& InColumnName, const int32 InArrayEntryIndex, FProperty* InProperty, void* InPropertyData);

	UDataTable* DataTable;
	const FString& JSONData;
	TArray<FString>& ImportProblems;
};
