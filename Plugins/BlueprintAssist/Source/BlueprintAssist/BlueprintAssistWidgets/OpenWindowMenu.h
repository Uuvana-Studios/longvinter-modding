// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BAFilteredList.h"
#include "Framework/Commands/UIAction.h"
#include "Textures/SlateIcon.h"

class FUICommandInfo;
struct FTabSpawnerEntry;
class ISettingsSection;
class ISettingsCategory;
class ISettingsContainer;
class UToolMenu;
class FTabManager;
struct FToolMenuEntry;
struct FToolMenuSection;

struct FOpenWindowItem_Base : IBAFilteredListItem
{
	virtual void SelectItem() = 0;

	virtual const FSlateBrush* GetIcon();

	virtual const FString* GetDetailsString() { return nullptr; }
};

struct FOpenTabItem final : FOpenWindowItem_Base
{
	FName TabName;
	FName TabIconStyle;
	FName TabDisplayName;
	TSharedPtr<FTabManager> AlternateTabManager;

	FSlateIcon Icon;

	FOpenTabItem() = default;

	FOpenTabItem(const FOpenTabItem& Other)
	{
		TabName = Other.TabName;
		TabIconStyle = Other.TabIconStyle;
		TabDisplayName = Other.TabDisplayName;
		AlternateTabManager = Other.AlternateTabManager;
		Icon = Other.Icon;
	};

	FOpenTabItem(FName InTabName, FName InTabIcon, TSharedPtr<FTabManager> InAlternateTabManager);

	FOpenTabItem(FName InTabName, FName InTabIcon, FName InTabDisplayName = "", TSharedPtr<FTabManager> InAlternateTabManager = nullptr);

	FOpenTabItem(FName InTabName, const FSlateIcon& InIcon, FName InTabDisplayName = "", TSharedPtr<FTabManager> InAlternateTabManager = nullptr);

	virtual FString ToString() const override;

	virtual void SelectItem() override;

	virtual const FSlateBrush* GetIcon() override { return Icon.GetIcon(); }
};

struct FOpenTabSpawnerItem final : FOpenWindowItem_Base
{
	TSharedPtr<FTabSpawnerEntry> TabSpawnerEntry;
	FOpenTabSpawnerItem(TSharedPtr<FTabSpawnerEntry> Entry) { TabSpawnerEntry = Entry; }
	virtual FString ToString() const override;
	virtual void SelectItem() override;
	virtual const FSlateBrush* GetIcon() override;
};

struct FToolsMenuItem final : FOpenWindowItem_Base
{
	UToolMenu* Menu = nullptr;
	UToolMenu* ParentContext = nullptr;
	FToolMenuEntry* Entry = nullptr;
	FToolMenuSection* Section = nullptr;

	virtual FString ToString() const override;

	virtual void SelectItem() override;
};

struct FExecuteCommandItem final : FOpenWindowItem_Base
{
	TSharedPtr<FUICommandInfo> Command;

	FExecuteCommandItem() = default;

	FExecuteCommandItem(TSharedPtr<FUICommandInfo> InCommand) : Command(InCommand) { }

	virtual FString ToString() const override;

	virtual void SelectItem() override;

	virtual const FSlateBrush* GetIcon() override;

	virtual const FString* GetDetailsString() override;
};

struct FOpenWindowItem_Action final : FOpenWindowItem_Base
{
	FString Name;
	FExecuteAction Action;

	FOpenWindowItem_Action(const FString& InName, FExecuteAction InAction)
		: Name(InName)
		, Action(InAction) {}

	virtual FString ToString() const override { return Name; }
	virtual void SelectItem() override;
};

struct FOpenSettingItem final : FOpenWindowItem_Base
{
	TSharedPtr<ISettingsContainer> Container;
	TSharedPtr<ISettingsCategory> Category;
	TSharedPtr<ISettingsSection> Section;

	FOpenSettingItem(TSharedPtr<ISettingsContainer> InContainer, TSharedPtr<ISettingsCategory> InCategory, TSharedPtr<ISettingsSection> InSection)
		: Container(InContainer)
		, Category(InCategory)
		, Section(InSection) { }

	FOpenSettingItem() = default;

	virtual FString ToString() const override;

	virtual FString GetSearchText() const override;

	FString GetCategoryString() const;

	virtual const FString* GetDetailsString() override
	{
		return new FString(GetCategoryString());
	}

	virtual const FSlateBrush* GetIcon() override;
	virtual void SelectItem() override;
};

class BLUEPRINTASSIST_API SOpenWindowMenu final : public SCompoundWidget
{
	// @formatter:off
	SLATE_BEGIN_ARGS(SOpenWindowMenu) { }
	SLATE_END_ARGS()
	// @formatter:on

	static FVector2D GetWidgetSize() { return FVector2D(400, 300); }

	void Construct(const FArguments& InArgs);

	void InitListItems(TArray<TSharedPtr<FOpenWindowItem_Base>>& Items);

	void AddOpenTabItems(TArray<TSharedPtr<FOpenWindowItem_Base>>& Items);

	void AddOpenSettingsItems(TArray<TSharedPtr<FOpenWindowItem_Base>>& Items);

	// void AddEditorUtilityWidgets(TArray<FOpenTabItem>& OutTabInfos);

	void AddCommandItems(TArray<TSharedPtr<FOpenWindowItem_Base>>& Items);

	void AddToolItems(TArray<TSharedPtr<FOpenWindowItem_Base>>& Items);

	void AddWorkspaceMenuStructure(TArray<TSharedPtr<FOpenWindowItem_Base>>& Items);

	void AddActionItems(TArray<TSharedPtr<FOpenWindowItem_Base>>& Items);

	TSharedRef<ITableRow> CreateItemWidget(TSharedPtr<FOpenWindowItem_Base> Item, const TSharedRef<STableViewBase>& OwnerTable) const;

	void SelectItem(TSharedPtr<FOpenWindowItem_Base> Item);
};
