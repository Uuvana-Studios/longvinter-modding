// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "FSR2TemporalUpscaling/Public/FSR2Settings.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeFSR2Settings() {}
// Cross Module References
	FSR2TEMPORALUPSCALING_API UEnum* Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2LandscapeHISMMode();
	UPackage* Z_Construct_UPackage__Script_FSR2TemporalUpscaling();
	FSR2TEMPORALUPSCALING_API UEnum* Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2DeDitherMode();
	FSR2TEMPORALUPSCALING_API UEnum* Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2HistoryFormat();
	FSR2TEMPORALUPSCALING_API UEnum* Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2QualityMode();
	FSR2TEMPORALUPSCALING_API UClass* Z_Construct_UClass_UFSR2Settings_NoRegister();
	FSR2TEMPORALUPSCALING_API UClass* Z_Construct_UClass_UFSR2Settings();
	DEVELOPERSETTINGS_API UClass* Z_Construct_UClass_UDeveloperSettings();
	ENGINE_API UEnum* Z_Construct_UEnum_Engine_EMaterialShadingModel();
// End Cross Module References
	static UEnum* EFSR2LandscapeHISMMode_StaticEnum()
	{
		static UEnum* Singleton = nullptr;
		if (!Singleton)
		{
			Singleton = GetStaticEnum(Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2LandscapeHISMMode, Z_Construct_UPackage__Script_FSR2TemporalUpscaling(), TEXT("EFSR2LandscapeHISMMode"));
		}
		return Singleton;
	}
	template<> FSR2TEMPORALUPSCALING_API UEnum* StaticEnum<EFSR2LandscapeHISMMode>()
	{
		return EFSR2LandscapeHISMMode_StaticEnum();
	}
	static FCompiledInDeferEnum Z_CompiledInDeferEnum_UEnum_EFSR2LandscapeHISMMode(EFSR2LandscapeHISMMode_StaticEnum, TEXT("/Script/FSR2TemporalUpscaling"), TEXT("EFSR2LandscapeHISMMode"), false, nullptr, nullptr);
	uint32 Get_Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2LandscapeHISMMode_Hash() { return 3948903944U; }
	UEnum* Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2LandscapeHISMMode()
	{
#if WITH_HOT_RELOAD
		UPackage* Outer = Z_Construct_UPackage__Script_FSR2TemporalUpscaling();
		static UEnum* ReturnEnum = FindExistingEnumIfHotReloadOrDynamic(Outer, TEXT("EFSR2LandscapeHISMMode"), 0, Get_Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2LandscapeHISMMode_Hash(), false);
#else
		static UEnum* ReturnEnum = nullptr;
#endif // WITH_HOT_RELOAD
		if (!ReturnEnum)
		{
			static const UE4CodeGen_Private::FEnumeratorParam Enumerators[] = {
				{ "EFSR2LandscapeHISMMode::Off", (int64)EFSR2LandscapeHISMMode::Off },
				{ "EFSR2LandscapeHISMMode::AllStatic", (int64)EFSR2LandscapeHISMMode::AllStatic },
				{ "EFSR2LandscapeHISMMode::StaticWPO", (int64)EFSR2LandscapeHISMMode::StaticWPO },
			};
#if WITH_METADATA
			const UE4CodeGen_Private::FMetaDataPairParam Enum_MetaDataParams[] = {
				{ "AllStatic.DisplayName", "All Instances" },
				{ "AllStatic.Name", "EFSR2LandscapeHISMMode::AllStatic" },
				{ "Comment", "//-------------------------------------------------------------------------------------\n// The modes for forcing Landscape Hierachical Instance Static Model to not be Static.\n//-------------------------------------------------------------------------------------\n" },
				{ "ModuleRelativePath", "Public/FSR2Settings.h" },
				{ "Off.DisplayName", "Off" },
				{ "Off.Name", "EFSR2LandscapeHISMMode::Off" },
				{ "StaticWPO.DisplayName", "Instances with World-Position-Offset" },
				{ "StaticWPO.Name", "EFSR2LandscapeHISMMode::StaticWPO" },
				{ "ToolTip", "The modes for forcing Landscape Hierachical Instance Static Model to not be Static." },
			};
#endif
			static const UE4CodeGen_Private::FEnumParams EnumParams = {
				(UObject*(*)())Z_Construct_UPackage__Script_FSR2TemporalUpscaling,
				nullptr,
				"EFSR2LandscapeHISMMode",
				"EFSR2LandscapeHISMMode",
				Enumerators,
				UE_ARRAY_COUNT(Enumerators),
				RF_Public|RF_Transient|RF_MarkAsNative,
				EEnumFlags::None,
				UE4CodeGen_Private::EDynamicType::NotDynamic,
				(uint8)UEnum::ECppForm::EnumClass,
				METADATA_PARAMS(Enum_MetaDataParams, UE_ARRAY_COUNT(Enum_MetaDataParams))
			};
			UE4CodeGen_Private::ConstructUEnum(ReturnEnum, EnumParams);
		}
		return ReturnEnum;
	}
	static UEnum* EFSR2DeDitherMode_StaticEnum()
	{
		static UEnum* Singleton = nullptr;
		if (!Singleton)
		{
			Singleton = GetStaticEnum(Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2DeDitherMode, Z_Construct_UPackage__Script_FSR2TemporalUpscaling(), TEXT("EFSR2DeDitherMode"));
		}
		return Singleton;
	}
	template<> FSR2TEMPORALUPSCALING_API UEnum* StaticEnum<EFSR2DeDitherMode>()
	{
		return EFSR2DeDitherMode_StaticEnum();
	}
	static FCompiledInDeferEnum Z_CompiledInDeferEnum_UEnum_EFSR2DeDitherMode(EFSR2DeDitherMode_StaticEnum, TEXT("/Script/FSR2TemporalUpscaling"), TEXT("EFSR2DeDitherMode"), false, nullptr, nullptr);
	uint32 Get_Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2DeDitherMode_Hash() { return 2513618200U; }
	UEnum* Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2DeDitherMode()
	{
#if WITH_HOT_RELOAD
		UPackage* Outer = Z_Construct_UPackage__Script_FSR2TemporalUpscaling();
		static UEnum* ReturnEnum = FindExistingEnumIfHotReloadOrDynamic(Outer, TEXT("EFSR2DeDitherMode"), 0, Get_Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2DeDitherMode_Hash(), false);
#else
		static UEnum* ReturnEnum = nullptr;
#endif // WITH_HOT_RELOAD
		if (!ReturnEnum)
		{
			static const UE4CodeGen_Private::FEnumeratorParam Enumerators[] = {
				{ "EFSR2DeDitherMode::Off", (int64)EFSR2DeDitherMode::Off },
				{ "EFSR2DeDitherMode::Full", (int64)EFSR2DeDitherMode::Full },
				{ "EFSR2DeDitherMode::HairOnly", (int64)EFSR2DeDitherMode::HairOnly },
			};
#if WITH_METADATA
			const UE4CodeGen_Private::FMetaDataPairParam Enum_MetaDataParams[] = {
				{ "Comment", "//-------------------------------------------------------------------------------------\n// The support texture formats for the FSR2 history data.\n//-------------------------------------------------------------------------------------\n" },
				{ "Full.DisplayName", "Full" },
				{ "Full.Name", "EFSR2DeDitherMode::Full" },
				{ "HairOnly.DisplayName", "Hair Only" },
				{ "HairOnly.Name", "EFSR2DeDitherMode::HairOnly" },
				{ "ModuleRelativePath", "Public/FSR2Settings.h" },
				{ "Off.DisplayName", "Off" },
				{ "Off.Name", "EFSR2DeDitherMode::Off" },
				{ "ToolTip", "The support texture formats for the FSR2 history data." },
			};
#endif
			static const UE4CodeGen_Private::FEnumParams EnumParams = {
				(UObject*(*)())Z_Construct_UPackage__Script_FSR2TemporalUpscaling,
				nullptr,
				"EFSR2DeDitherMode",
				"EFSR2DeDitherMode",
				Enumerators,
				UE_ARRAY_COUNT(Enumerators),
				RF_Public|RF_Transient|RF_MarkAsNative,
				EEnumFlags::None,
				UE4CodeGen_Private::EDynamicType::NotDynamic,
				(uint8)UEnum::ECppForm::EnumClass,
				METADATA_PARAMS(Enum_MetaDataParams, UE_ARRAY_COUNT(Enum_MetaDataParams))
			};
			UE4CodeGen_Private::ConstructUEnum(ReturnEnum, EnumParams);
		}
		return ReturnEnum;
	}
	static UEnum* EFSR2HistoryFormat_StaticEnum()
	{
		static UEnum* Singleton = nullptr;
		if (!Singleton)
		{
			Singleton = GetStaticEnum(Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2HistoryFormat, Z_Construct_UPackage__Script_FSR2TemporalUpscaling(), TEXT("EFSR2HistoryFormat"));
		}
		return Singleton;
	}
	template<> FSR2TEMPORALUPSCALING_API UEnum* StaticEnum<EFSR2HistoryFormat>()
	{
		return EFSR2HistoryFormat_StaticEnum();
	}
	static FCompiledInDeferEnum Z_CompiledInDeferEnum_UEnum_EFSR2HistoryFormat(EFSR2HistoryFormat_StaticEnum, TEXT("/Script/FSR2TemporalUpscaling"), TEXT("EFSR2HistoryFormat"), false, nullptr, nullptr);
	uint32 Get_Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2HistoryFormat_Hash() { return 2595833206U; }
	UEnum* Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2HistoryFormat()
	{
#if WITH_HOT_RELOAD
		UPackage* Outer = Z_Construct_UPackage__Script_FSR2TemporalUpscaling();
		static UEnum* ReturnEnum = FindExistingEnumIfHotReloadOrDynamic(Outer, TEXT("EFSR2HistoryFormat"), 0, Get_Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2HistoryFormat_Hash(), false);
#else
		static UEnum* ReturnEnum = nullptr;
#endif // WITH_HOT_RELOAD
		if (!ReturnEnum)
		{
			static const UE4CodeGen_Private::FEnumeratorParam Enumerators[] = {
				{ "EFSR2HistoryFormat::FloatRGBA", (int64)EFSR2HistoryFormat::FloatRGBA },
				{ "EFSR2HistoryFormat::FloatR11G11B10", (int64)EFSR2HistoryFormat::FloatR11G11B10 },
			};
#if WITH_METADATA
			const UE4CodeGen_Private::FMetaDataPairParam Enum_MetaDataParams[] = {
				{ "Comment", "//-------------------------------------------------------------------------------------\n// The support texture formats for the FSR2 history data.\n//-------------------------------------------------------------------------------------\n" },
				{ "FloatR11G11B10.DisplayName", "PF_FloatR11G11B10" },
				{ "FloatR11G11B10.Name", "EFSR2HistoryFormat::FloatR11G11B10" },
				{ "FloatRGBA.DisplayName", "PF_FloatRGBA" },
				{ "FloatRGBA.Name", "EFSR2HistoryFormat::FloatRGBA" },
				{ "ModuleRelativePath", "Public/FSR2Settings.h" },
				{ "ToolTip", "The support texture formats for the FSR2 history data." },
			};
#endif
			static const UE4CodeGen_Private::FEnumParams EnumParams = {
				(UObject*(*)())Z_Construct_UPackage__Script_FSR2TemporalUpscaling,
				nullptr,
				"EFSR2HistoryFormat",
				"EFSR2HistoryFormat",
				Enumerators,
				UE_ARRAY_COUNT(Enumerators),
				RF_Public|RF_Transient|RF_MarkAsNative,
				EEnumFlags::None,
				UE4CodeGen_Private::EDynamicType::NotDynamic,
				(uint8)UEnum::ECppForm::EnumClass,
				METADATA_PARAMS(Enum_MetaDataParams, UE_ARRAY_COUNT(Enum_MetaDataParams))
			};
			UE4CodeGen_Private::ConstructUEnum(ReturnEnum, EnumParams);
		}
		return ReturnEnum;
	}
	static UEnum* EFSR2QualityMode_StaticEnum()
	{
		static UEnum* Singleton = nullptr;
		if (!Singleton)
		{
			Singleton = GetStaticEnum(Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2QualityMode, Z_Construct_UPackage__Script_FSR2TemporalUpscaling(), TEXT("EFSR2QualityMode"));
		}
		return Singleton;
	}
	template<> FSR2TEMPORALUPSCALING_API UEnum* StaticEnum<EFSR2QualityMode>()
	{
		return EFSR2QualityMode_StaticEnum();
	}
	static FCompiledInDeferEnum Z_CompiledInDeferEnum_UEnum_EFSR2QualityMode(EFSR2QualityMode_StaticEnum, TEXT("/Script/FSR2TemporalUpscaling"), TEXT("EFSR2QualityMode"), false, nullptr, nullptr);
	uint32 Get_Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2QualityMode_Hash() { return 732089468U; }
	UEnum* Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2QualityMode()
	{
#if WITH_HOT_RELOAD
		UPackage* Outer = Z_Construct_UPackage__Script_FSR2TemporalUpscaling();
		static UEnum* ReturnEnum = FindExistingEnumIfHotReloadOrDynamic(Outer, TEXT("EFSR2QualityMode"), 0, Get_Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2QualityMode_Hash(), false);
#else
		static UEnum* ReturnEnum = nullptr;
#endif // WITH_HOT_RELOAD
		if (!ReturnEnum)
		{
			static const UE4CodeGen_Private::FEnumeratorParam Enumerators[] = {
				{ "EFSR2QualityMode::Unused", (int64)EFSR2QualityMode::Unused },
				{ "EFSR2QualityMode::Quality", (int64)EFSR2QualityMode::Quality },
				{ "EFSR2QualityMode::Balanced", (int64)EFSR2QualityMode::Balanced },
				{ "EFSR2QualityMode::Performance", (int64)EFSR2QualityMode::Performance },
				{ "EFSR2QualityMode::UltraPerformance", (int64)EFSR2QualityMode::UltraPerformance },
			};
#if WITH_METADATA
			const UE4CodeGen_Private::FMetaDataPairParam Enum_MetaDataParams[] = {
				{ "Balanced.DisplayName", "Balanced" },
				{ "Balanced.Name", "EFSR2QualityMode::Balanced" },
				{ "Comment", "//-------------------------------------------------------------------------------------\n// The official FSR2 quality modes.\n//-------------------------------------------------------------------------------------\n" },
				{ "ModuleRelativePath", "Public/FSR2Settings.h" },
				{ "Performance.DisplayName", "Performance" },
				{ "Performance.Name", "EFSR2QualityMode::Performance" },
				{ "Quality.DisplayName", "Quality" },
				{ "Quality.Name", "EFSR2QualityMode::Quality" },
				{ "ToolTip", "The official FSR2 quality modes." },
				{ "UltraPerformance.DisplayName", "Ultra Performance" },
				{ "UltraPerformance.Name", "EFSR2QualityMode::UltraPerformance" },
				{ "Unused.Hidden", "" },
				{ "Unused.Name", "EFSR2QualityMode::Unused" },
			};
#endif
			static const UE4CodeGen_Private::FEnumParams EnumParams = {
				(UObject*(*)())Z_Construct_UPackage__Script_FSR2TemporalUpscaling,
				nullptr,
				"EFSR2QualityMode",
				"EFSR2QualityMode",
				Enumerators,
				UE_ARRAY_COUNT(Enumerators),
				RF_Public|RF_Transient|RF_MarkAsNative,
				EEnumFlags::None,
				UE4CodeGen_Private::EDynamicType::NotDynamic,
				(uint8)UEnum::ECppForm::EnumClass,
				METADATA_PARAMS(Enum_MetaDataParams, UE_ARRAY_COUNT(Enum_MetaDataParams))
			};
			UE4CodeGen_Private::ConstructUEnum(ReturnEnum, EnumParams);
		}
		return ReturnEnum;
	}
	void UFSR2Settings::StaticRegisterNativesUFSR2Settings()
	{
	}
	UClass* Z_Construct_UClass_UFSR2Settings_NoRegister()
	{
		return UFSR2Settings::StaticClass();
	}
	struct Z_Construct_UClass_UFSR2Settings_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bEnabled_MetaData[];
#endif
		static void NewProp_bEnabled_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bEnabled;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_AutoExposure_MetaData[];
#endif
		static void NewProp_AutoExposure_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_AutoExposure;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_EnabledInEditorViewport_MetaData[];
#endif
		static void NewProp_EnabledInEditorViewport_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_EnabledInEditorViewport;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_UseSSRExperimentalDenoiser_MetaData[];
#endif
		static void NewProp_UseSSRExperimentalDenoiser_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_UseSSRExperimentalDenoiser;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_UseNativeDX12_MetaData[];
#endif
		static void NewProp_UseNativeDX12_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_UseNativeDX12;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_UseNativeVulkan_MetaData[];
#endif
		static void NewProp_UseNativeVulkan_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_UseNativeVulkan;
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_QualityMode_Underlying;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_QualityMode_MetaData[];
#endif
		static const UE4CodeGen_Private::FEnumPropertyParams NewProp_QualityMode;
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_HistoryFormat_Underlying;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_HistoryFormat_MetaData[];
#endif
		static const UE4CodeGen_Private::FEnumPropertyParams NewProp_HistoryFormat;
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_DeDither_Underlying;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_DeDither_MetaData[];
#endif
		static const UE4CodeGen_Private::FEnumPropertyParams NewProp_DeDither;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Sharpness_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_Sharpness;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_AdjustMipBias_MetaData[];
#endif
		static void NewProp_AdjustMipBias_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_AdjustMipBias;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ReactiveMask_MetaData[];
#endif
		static void NewProp_ReactiveMask_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ReactiveMask;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ForceVertexDeformationOutputsVelocity_MetaData[];
#endif
		static void NewProp_ForceVertexDeformationOutputsVelocity_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ForceVertexDeformationOutputsVelocity;
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_ForceLandscapeHISMMobility_Underlying;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ForceLandscapeHISMMobility_MetaData[];
#endif
		static const UE4CodeGen_Private::FEnumPropertyParams NewProp_ForceLandscapeHISMMobility;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ReflectionScale_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_ReflectionScale;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ReflectionLuminanceBias_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_ReflectionLuminanceBias;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_RoughnessScale_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_RoughnessScale;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_RoughnessBias_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_RoughnessBias;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_RoughnessMaxDistance_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_RoughnessMaxDistance;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ReactiveMaskRoughnessForceMaxDistance_MetaData[];
#endif
		static void NewProp_ReactiveMaskRoughnessForceMaxDistance_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ReactiveMaskRoughnessForceMaxDistance;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_TranslucencyBias_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_TranslucencyBias;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_TranslucencyLuminanceBias_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_TranslucencyLuminanceBias;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_TranslucencyMaxDistance_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_TranslucencyMaxDistance;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_PreDOFTranslucencyScale_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_PreDOFTranslucencyScale;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_PreDOFTranslucencyMax_MetaData[];
#endif
		static void NewProp_PreDOFTranslucencyMax_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_PreDOFTranslucencyMax;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ReactiveShadingModelID_MetaData[];
#endif
		static const UE4CodeGen_Private::FBytePropertyParams NewProp_ReactiveShadingModelID;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ForceReactiveMaterialValue_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_ForceReactiveMaterialValue;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ReactiveHistoryTranslucencyBias_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_ReactiveHistoryTranslucencyBias;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ReactiveHistoryTranslucencyLumaBias_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_ReactiveHistoryTranslucencyLumaBias;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UFSR2Settings_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UDeveloperSettings,
		(UObject* (*)())Z_Construct_UPackage__Script_FSR2TemporalUpscaling,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::Class_MetaDataParams[] = {
		{ "Comment", "//-------------------------------------------------------------------------------------\n// Settings for FSR2 exposed through the Editor UI.\n//-------------------------------------------------------------------------------------\n" },
		{ "DisplayName", "FidelityFX Super Resolution 2.2" },
		{ "IncludePath", "FSR2Settings.h" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Settings for FSR2 exposed through the Editor UI." },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_bEnabled_MetaData[] = {
		{ "Category", "General Settings" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.Enabled" },
		{ "DisplayName", "Enabled" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
	};
#endif
	void Z_Construct_UClass_UFSR2Settings_Statics::NewProp_bEnabled_SetBit(void* Obj)
	{
		((UFSR2Settings*)Obj)->bEnabled = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_bEnabled = { "bEnabled", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UFSR2Settings), &Z_Construct_UClass_UFSR2Settings_Statics::NewProp_bEnabled_SetBit, METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_bEnabled_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_bEnabled_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_AutoExposure_MetaData[] = {
		{ "Category", "General Settings" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.AutoExposure" },
		{ "DisplayName", "Auto Exposure" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Enable to use FSR2's own auto-exposure, otherwise the engine's auto-exposure value is used." },
	};
#endif
	void Z_Construct_UClass_UFSR2Settings_Statics::NewProp_AutoExposure_SetBit(void* Obj)
	{
		((UFSR2Settings*)Obj)->AutoExposure = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_AutoExposure = { "AutoExposure", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UFSR2Settings), &Z_Construct_UClass_UFSR2Settings_Statics::NewProp_AutoExposure_SetBit, METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_AutoExposure_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_AutoExposure_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_EnabledInEditorViewport_MetaData[] = {
		{ "Category", "General Settings" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.EnabledInEditorViewport" },
		{ "DisplayName", "Enabled in Editor Viewport" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "When enabled use FSR2 by default in the Editor viewports." },
	};
#endif
	void Z_Construct_UClass_UFSR2Settings_Statics::NewProp_EnabledInEditorViewport_SetBit(void* Obj)
	{
		((UFSR2Settings*)Obj)->EnabledInEditorViewport = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_EnabledInEditorViewport = { "EnabledInEditorViewport", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UFSR2Settings), &Z_Construct_UClass_UFSR2Settings_Statics::NewProp_EnabledInEditorViewport_SetBit, METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_EnabledInEditorViewport_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_EnabledInEditorViewport_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_UseSSRExperimentalDenoiser_MetaData[] = {
		{ "Category", "General Settings" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.UseSSRExperimentalDenoiser" },
		{ "DisplayName", "Use SSR Experimental Denoiser" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Set to 1 to use r.SSR.ExperimentalDenoiser when FSR2 is enabled. This is required when r.FidelityFX.FSR2.CreateReactiveMask is enabled as the FSR2 plugin sets r.SSR.ExperimentalDenoiser to 1 in order to capture reflection data to generate the reactive mask." },
	};
#endif
	void Z_Construct_UClass_UFSR2Settings_Statics::NewProp_UseSSRExperimentalDenoiser_SetBit(void* Obj)
	{
		((UFSR2Settings*)Obj)->UseSSRExperimentalDenoiser = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_UseSSRExperimentalDenoiser = { "UseSSRExperimentalDenoiser", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UFSR2Settings), &Z_Construct_UClass_UFSR2Settings_Statics::NewProp_UseSSRExperimentalDenoiser_SetBit, METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_UseSSRExperimentalDenoiser_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_UseSSRExperimentalDenoiser_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_UseNativeDX12_MetaData[] = {
		{ "Category", "Backend Settings" },
		{ "ConfigRestartRequired", "TRUE" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.UseNativeDX12" },
		{ "DisplayName", "Native DX12" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Whather to use the optimised native DX12 backend for FSR2 rather than the default RHI backend." },
	};
#endif
	void Z_Construct_UClass_UFSR2Settings_Statics::NewProp_UseNativeDX12_SetBit(void* Obj)
	{
		((UFSR2Settings*)Obj)->UseNativeDX12 = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_UseNativeDX12 = { "UseNativeDX12", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UFSR2Settings), &Z_Construct_UClass_UFSR2Settings_Statics::NewProp_UseNativeDX12_SetBit, METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_UseNativeDX12_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_UseNativeDX12_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_UseNativeVulkan_MetaData[] = {
		{ "Category", "Backend Settings" },
		{ "ConfigRestartRequired", "TRUE" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.UseNativeVulkan" },
		{ "DisplayName", "Native Vulkan" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Whather to use the optimised native Vulkan backend for FSR2 rather than the default RHI backend." },
	};
#endif
	void Z_Construct_UClass_UFSR2Settings_Statics::NewProp_UseNativeVulkan_SetBit(void* Obj)
	{
		((UFSR2Settings*)Obj)->UseNativeVulkan = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_UseNativeVulkan = { "UseNativeVulkan", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UFSR2Settings), &Z_Construct_UClass_UFSR2Settings_Statics::NewProp_UseNativeVulkan_SetBit, METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_UseNativeVulkan_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_UseNativeVulkan_MetaData)) };
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_QualityMode_Underlying = { "UnderlyingType", nullptr, (EPropertyFlags)0x0000000000000000, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_QualityMode_MetaData[] = {
		{ "Category", "Quality Settings" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.QualityMode" },
		{ "DisplayName", "Quality Mode" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Selects the default quality mode to be used when upscaling with FSR2." },
	};
#endif
	const UE4CodeGen_Private::FEnumPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_QualityMode = { "QualityMode", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Enum, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UFSR2Settings, QualityMode), Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2QualityMode, METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_QualityMode_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_QualityMode_MetaData)) };
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_HistoryFormat_Underlying = { "UnderlyingType", nullptr, (EPropertyFlags)0x0000000000000000, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_HistoryFormat_MetaData[] = {
		{ "Category", "Quality Settings" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.HistoryFormat" },
		{ "DisplayName", "History Format" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Selects the bit-depth for the FSR2 history texture format, defaults to PF_FloatRGBA but can be set to PF_FloatR11G11B10 to reduce bandwidth at the expense of quality." },
	};
#endif
	const UE4CodeGen_Private::FEnumPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_HistoryFormat = { "HistoryFormat", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Enum, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UFSR2Settings, HistoryFormat), Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2HistoryFormat, METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_HistoryFormat_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_HistoryFormat_MetaData)) };
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_DeDither_Underlying = { "UnderlyingType", nullptr, (EPropertyFlags)0x0000000000000000, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_DeDither_MetaData[] = {
		{ "Category", "Quality Settings" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.DeDither" },
		{ "DisplayName", "De-Dither Rendering" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Enable an extra pass to de-dither rendering before handing over to FSR2 to avoid over-thinning, defaults to Off but can be set to Full for all pixels or to Hair Only for just around Hair (requires Deffered Renderer)." },
	};
#endif
	const UE4CodeGen_Private::FEnumPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_DeDither = { "DeDither", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Enum, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UFSR2Settings, DeDither), Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2DeDitherMode, METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_DeDither_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_DeDither_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_Sharpness_MetaData[] = {
		{ "Category", "Quality Settings" },
		{ "ClampMax", "1" },
		{ "ClampMin", "0" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.Sharpness" },
		{ "DisplayName", "Sharpness" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "When greater than 0.0 this enables Robust Contrast Adaptive Sharpening Filter to sharpen the output image." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_Sharpness = { "Sharpness", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UFSR2Settings, Sharpness), METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_Sharpness_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_Sharpness_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_AdjustMipBias_MetaData[] = {
		{ "Category", "Quality Settings" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.AdjustMipBias" },
		{ "DisplayName", "Adjust Mip Bias & Offset" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Applies negative MipBias to material textures, improving results." },
	};
#endif
	void Z_Construct_UClass_UFSR2Settings_Statics::NewProp_AdjustMipBias_SetBit(void* Obj)
	{
		((UFSR2Settings*)Obj)->AdjustMipBias = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_AdjustMipBias = { "AdjustMipBias", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UFSR2Settings), &Z_Construct_UClass_UFSR2Settings_Statics::NewProp_AdjustMipBias_SetBit, METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_AdjustMipBias_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_AdjustMipBias_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveMask_MetaData[] = {
		{ "Category", "Quality Settings" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.CreateReactiveMask" },
		{ "DisplayName", "Reactive Mask" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Enable to generate a mask from the SceneColor, GBuffer, SeparateTranslucency & ScreenspaceReflections that determines how reactive each pixel should be." },
	};
#endif
	void Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveMask_SetBit(void* Obj)
	{
		((UFSR2Settings*)Obj)->ReactiveMask = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveMask = { "ReactiveMask", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UFSR2Settings), &Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveMask_SetBit, METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveMask_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveMask_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ForceVertexDeformationOutputsVelocity_MetaData[] = {
		{ "Category", "Quality Settings" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.ForceVertexDeformationOutputsVelocity" },
		{ "DisplayName", "Force Vertex Deformation To Output Velocity" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Force enables materials with World Position Offset and/or World Displacement to output velocities during velocity pass even when the actor has not moved." },
	};
#endif
	void Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ForceVertexDeformationOutputsVelocity_SetBit(void* Obj)
	{
		((UFSR2Settings*)Obj)->ForceVertexDeformationOutputsVelocity = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ForceVertexDeformationOutputsVelocity = { "ForceVertexDeformationOutputsVelocity", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UFSR2Settings), &Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ForceVertexDeformationOutputsVelocity_SetBit, METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ForceVertexDeformationOutputsVelocity_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ForceVertexDeformationOutputsVelocity_MetaData)) };
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ForceLandscapeHISMMobility_Underlying = { "UnderlyingType", nullptr, (EPropertyFlags)0x0000000000000000, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ForceLandscapeHISMMobility_MetaData[] = {
		{ "Category", "Quality Settings" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.ForceLandscapeHISMMobility" },
		{ "DisplayName", "Force Landscape HISM Mobility" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Allow FSR2 to force the mobility of Landscape actors Hierarchical Instance Static Mesh components that use World-Position-Offset materials so they render valid velocities.\nSetting 'All Instances' is faster on the CPU, 'Instances with World-Position-Offset' is faster on the GPU." },
	};
#endif
	const UE4CodeGen_Private::FEnumPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ForceLandscapeHISMMobility = { "ForceLandscapeHISMMobility", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Enum, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UFSR2Settings, ForceLandscapeHISMMobility), Z_Construct_UEnum_FSR2TemporalUpscaling_EFSR2LandscapeHISMMode, METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ForceLandscapeHISMMobility_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ForceLandscapeHISMMobility_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReflectionScale_MetaData[] = {
		{ "Category", "Reactive Mask Settings" },
		{ "ClampMax", "1" },
		{ "ClampMin", "0" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.ReactiveMaskReflectionScale" },
		{ "DisplayName", "Reflection Scale" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Scales the Unreal engine reflection contribution to the reactive mask, which can be used to control the amount of aliasing on reflective surfaces." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReflectionScale = { "ReflectionScale", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UFSR2Settings, ReflectionScale), METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReflectionScale_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReflectionScale_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReflectionLuminanceBias_MetaData[] = {
		{ "Category", "Reactive Mask Settings" },
		{ "ClampMax", "1" },
		{ "ClampMin", "0" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.ReactiveMaskReflectionLumaBias" },
		{ "DisplayName", "Reflection Luminance Bias" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Biases the reactive mask by the luminance of the reflection. Use to balance aliasing against ghosting on brightly lit reflective surfaces." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReflectionLuminanceBias = { "ReflectionLuminanceBias", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UFSR2Settings, ReflectionLuminanceBias), METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReflectionLuminanceBias_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReflectionLuminanceBias_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_RoughnessScale_MetaData[] = {
		{ "Category", "Reactive Mask Settings" },
		{ "ClampMax", "1" },
		{ "ClampMin", "0" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.ReactiveMaskRoughnessScale" },
		{ "DisplayName", "Roughness Scale" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Scales the GBuffer roughness to provide a fallback value for the reactive mask when screenspace & planar reflections are disabled or don't affect a pixel." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_RoughnessScale = { "RoughnessScale", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UFSR2Settings, RoughnessScale), METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_RoughnessScale_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_RoughnessScale_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_RoughnessBias_MetaData[] = {
		{ "Category", "Reactive Mask Settings" },
		{ "ClampMax", "1" },
		{ "ClampMin", "0" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.ReactiveMaskRoughnessBias" },
		{ "DisplayName", "Roughness Bias" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Biases the reactive mask value when screenspace/planar reflections are weak with the GBuffer roughness to account for reflection environment captures." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_RoughnessBias = { "RoughnessBias", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UFSR2Settings, RoughnessBias), METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_RoughnessBias_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_RoughnessBias_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_RoughnessMaxDistance_MetaData[] = {
		{ "Category", "Reactive Mask Settings" },
		{ "ClampMin", "0" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.ReactiveMaskRoughnessMaxDistance" },
		{ "DisplayName", "Roughness Max Distance" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Maximum distance in world units for using material roughness to contribute to the reactive mask, the maximum of this value and View.FurthestReflectionCaptureDistance will be used." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_RoughnessMaxDistance = { "RoughnessMaxDistance", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UFSR2Settings, RoughnessMaxDistance), METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_RoughnessMaxDistance_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_RoughnessMaxDistance_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveMaskRoughnessForceMaxDistance_MetaData[] = {
		{ "Category", "Reactive Mask Settings" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.ReactiveMaskRoughnessForceMaxDistance" },
		{ "DisplayName", "Force Roughness Max Distance" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Enable to force the maximum distance in world units for using material roughness to contribute to the reactive mask rather than using View.FurthestReflectionCaptureDistance." },
	};
#endif
	void Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveMaskRoughnessForceMaxDistance_SetBit(void* Obj)
	{
		((UFSR2Settings*)Obj)->ReactiveMaskRoughnessForceMaxDistance = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveMaskRoughnessForceMaxDistance = { "ReactiveMaskRoughnessForceMaxDistance", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UFSR2Settings), &Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveMaskRoughnessForceMaxDistance_SetBit, METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveMaskRoughnessForceMaxDistance_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveMaskRoughnessForceMaxDistance_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_TranslucencyBias_MetaData[] = {
		{ "Category", "Reactive Mask Settings" },
		{ "ClampMax", "1" },
		{ "ClampMin", "0" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.ReactiveMaskTranslucencyBias" },
		{ "DisplayName", "Translucency Bias" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Scales how much contribution translucency makes to the reactive mask. Higher values will make translucent materials less reactive which can reduce smearing." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_TranslucencyBias = { "TranslucencyBias", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UFSR2Settings, TranslucencyBias), METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_TranslucencyBias_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_TranslucencyBias_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_TranslucencyLuminanceBias_MetaData[] = {
		{ "Category", "Reactive Mask Settings" },
		{ "ClampMax", "1" },
		{ "ClampMin", "0" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.ReactiveMaskTranslucencyLumaBias" },
		{ "DisplayName", "Translucency Luminance Bias" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Biases the translucency contribution to the reactive mask by the luminance of the transparency." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_TranslucencyLuminanceBias = { "TranslucencyLuminanceBias", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UFSR2Settings, TranslucencyLuminanceBias), METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_TranslucencyLuminanceBias_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_TranslucencyLuminanceBias_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_TranslucencyMaxDistance_MetaData[] = {
		{ "Category", "Reactive Mask Settings" },
		{ "ClampMin", "0" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.ReactiveMaskTranslucencyMaxDistance" },
		{ "DisplayName", "Translucency Max Distance" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Maximum distance in world units for using translucency to contribute to the reactive mask. This is another way to remove sky-boxes and other back-planes from the reactive mask, at the expense of nearer translucency not being reactive." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_TranslucencyMaxDistance = { "TranslucencyMaxDistance", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UFSR2Settings, TranslucencyMaxDistance), METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_TranslucencyMaxDistance_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_TranslucencyMaxDistance_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_PreDOFTranslucencyScale_MetaData[] = {
		{ "Category", "Reactive Mask Settings" },
		{ "ClampMax", "1" },
		{ "ClampMin", "0" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.ReactiveMaskPreDOFTranslucencyScale" },
		{ "DisplayName", "Pre Depth-of-Field Translucency Scale" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Scales how much contribution pre-Depth-of-Field translucency color makes to the reactive mask. Higher values will make translucent materials less reactive which can reduce smearing." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_PreDOFTranslucencyScale = { "PreDOFTranslucencyScale", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UFSR2Settings, PreDOFTranslucencyScale), METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_PreDOFTranslucencyScale_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_PreDOFTranslucencyScale_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_PreDOFTranslucencyMax_MetaData[] = {
		{ "Category", "Reactive Mask Settings" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.ReactiveMaskPreDOFTranslucencyMax" },
		{ "DisplayName", "Pre Depth-of-Field Translucency Max/Average" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Toggle to determine whether to use the max(SceneColorPostDepthOfField - SceneColorPreDepthOfField) or length(SceneColorPostDepthOfField - SceneColorPreDepthOfField) to determine the contribution of Pre-Depth-of-Field translucency." },
	};
#endif
	void Z_Construct_UClass_UFSR2Settings_Statics::NewProp_PreDOFTranslucencyMax_SetBit(void* Obj)
	{
		((UFSR2Settings*)Obj)->PreDOFTranslucencyMax = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_PreDOFTranslucencyMax = { "PreDOFTranslucencyMax", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UFSR2Settings), &Z_Construct_UClass_UFSR2Settings_Statics::NewProp_PreDOFTranslucencyMax_SetBit, METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_PreDOFTranslucencyMax_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_PreDOFTranslucencyMax_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveShadingModelID_MetaData[] = {
		{ "Category", "Reactive Mask Settings" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.ReactiveMaskReactiveShadingModelID" },
		{ "DisplayName", "Reactive Shading Model" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Treat the specified shading model as reactive, taking the CustomData0.x value as the reactive value to write into the mask. Default is MSM_NUM (Off)." },
	};
#endif
	const UE4CodeGen_Private::FBytePropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveShadingModelID = { "ReactiveShadingModelID", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Byte, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UFSR2Settings, ReactiveShadingModelID), Z_Construct_UEnum_Engine_EMaterialShadingModel, METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveShadingModelID_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveShadingModelID_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ForceReactiveMaterialValue_MetaData[] = {
		{ "Category", "Reactive Mask Settings" },
		{ "ClampMax", "1" },
		{ "ClampMin", "0" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.ReactiveMaskForceReactiveMaterialValue" },
		{ "DisplayName", "Force value for Reactive Shading Model" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Force the reactive mask value for Reactive Shading Model materials, when > 0 this value can be used to override the value supplied in the Material Graph." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ForceReactiveMaterialValue = { "ForceReactiveMaterialValue", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UFSR2Settings, ForceReactiveMaterialValue), METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ForceReactiveMaterialValue_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ForceReactiveMaterialValue_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveHistoryTranslucencyBias_MetaData[] = {
		{ "Category", "Reactive Mask Settings" },
		{ "ClampMax", "1" },
		{ "ClampMin", "0" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.ReactiveHistoryTranslucencyBias" },
		{ "DisplayName", "Translucency Bias" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Scales how much contribution translucency makes to suppress history via the reactive mask. Higher values will make translucent materials less reactive which can reduce smearing." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveHistoryTranslucencyBias = { "ReactiveHistoryTranslucencyBias", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UFSR2Settings, ReactiveHistoryTranslucencyBias), METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveHistoryTranslucencyBias_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveHistoryTranslucencyBias_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveHistoryTranslucencyLumaBias_MetaData[] = {
		{ "Category", "Reactive Mask Settings" },
		{ "ClampMax", "1" },
		{ "ClampMin", "0" },
		{ "ConsoleVariable", "r.FidelityFX.FSR2.ReactiveHistoryTranslucencyLumaBias" },
		{ "DisplayName", "Translucency Luminance Bias" },
		{ "ModuleRelativePath", "Public/FSR2Settings.h" },
		{ "ToolTip", "Biases the translucency contribution to suppress history via the reactive mask by the luminance of the transparency." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveHistoryTranslucencyLumaBias = { "ReactiveHistoryTranslucencyLumaBias", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UFSR2Settings, ReactiveHistoryTranslucencyLumaBias), METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveHistoryTranslucencyLumaBias_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveHistoryTranslucencyLumaBias_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UFSR2Settings_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_bEnabled,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_AutoExposure,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_EnabledInEditorViewport,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_UseSSRExperimentalDenoiser,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_UseNativeDX12,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_UseNativeVulkan,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_QualityMode_Underlying,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_QualityMode,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_HistoryFormat_Underlying,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_HistoryFormat,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_DeDither_Underlying,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_DeDither,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_Sharpness,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_AdjustMipBias,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveMask,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ForceVertexDeformationOutputsVelocity,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ForceLandscapeHISMMobility_Underlying,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ForceLandscapeHISMMobility,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReflectionScale,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReflectionLuminanceBias,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_RoughnessScale,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_RoughnessBias,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_RoughnessMaxDistance,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveMaskRoughnessForceMaxDistance,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_TranslucencyBias,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_TranslucencyLuminanceBias,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_TranslucencyMaxDistance,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_PreDOFTranslucencyScale,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_PreDOFTranslucencyMax,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveShadingModelID,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ForceReactiveMaterialValue,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveHistoryTranslucencyBias,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UFSR2Settings_Statics::NewProp_ReactiveHistoryTranslucencyLumaBias,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_UFSR2Settings_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UFSR2Settings>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_UFSR2Settings_Statics::ClassParams = {
		&UFSR2Settings::StaticClass,
		"Engine",
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_UFSR2Settings_Statics::PropPointers,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		0,
		UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::PropPointers),
		0,
		0x001000A6u,
		METADATA_PARAMS(Z_Construct_UClass_UFSR2Settings_Statics::Class_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UClass_UFSR2Settings_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UFSR2Settings()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_UFSR2Settings_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(UFSR2Settings, 643936530);
	template<> FSR2TEMPORALUPSCALING_API UClass* StaticClass<UFSR2Settings>()
	{
		return UFSR2Settings::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_UFSR2Settings(Z_Construct_UClass_UFSR2Settings, &UFSR2Settings::StaticClass, TEXT("/Script/FSR2TemporalUpscaling"), TEXT("UFSR2Settings"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(UFSR2Settings);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
