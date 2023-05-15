// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordAchievementWindow.h"
#include "Http.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "DiscordSettings.h"
#include "DiscordAchievementEditor.h"
#include "Interfaces/IMainFrameModule.h"
#include "DiscordSdkSettingsEditor.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "DiscordSdkEditorStyle.h"
#include "IDesktopPlatform.h"
#include "DesktopPlatformModule.h"
#include "Misc/Base64.h"
#include "Misc/SlowTask.h"
#include "Framework/Application/SlateApplication.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonValue.h"
#include "Dom/JsonObject.h"
#include "Widgets/Text/STextBlock.h"
#include "Misc/FileHelper.h"
#include "Engine.h"


const FLinearColor	   SDiscordAchievementWindow::FontColor(FLinearColor(220, 221, 222));
const FLinearColor	   SDiscordAchievementWindow::OddRowColor(RGBToSRGB(24, 25, 28));
const FLinearColor	   SDiscordAchievementWindow::EvenRowColor(RGBToSRGB(18, 19, 21));
const FSlateColorBrush SDiscordAchievementWindow::BlackStyle(FColor::Black);
const FSlateColorBrush SDiscordAchievementWindow::BackgroundStyle(RGBToSRGB(54, 57, 63));
const FSlateColorBrush SDiscordAchievementWindow::AchievementHeadRowStyle(RGBToSRGB(32, 34, 37));
const FSlateColorBrush SDiscordAchievementWindow::AchievementOddRowStyle(OddRowColor);
const FSlateColorBrush SDiscordAchievementWindow::AchievementEvenRowStyle(EvenRowColor);

namespace
{
	void RegisterWindow(TSharedRef<SWindow> Window)
	{
		IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));

		if (MainFrameModule.GetParentWindow().IsValid())
		{
			FSlateApplication::Get().AddWindowAsNativeChild(Window, MainFrameModule.GetParentWindow().ToSharedRef());
		}
		else
		{
			FSlateApplication::Get().AddWindow(Window);
		}
	}

	FHttpRequestRef CreateRequestForDiscordApi()
	{
		FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
		Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bot %s"), *UDiscordEditorSettings::GetBotToken()));
		return Request;
	}
}


FAchievementManager::FAchievementManager()
{

}

void FAchievementManager::LoadAchievements(const FOnGetAchievementsResponse& OnResponse)
{
	FHttpRequestRef Request = CreateRequestForDiscordApi();
	const FString Url = TEXT("https://discord.com/api/v6/applications") / UDiscordSettings::GetClientIdAsString() / TEXT("achievements");
	
	Request->SetURL(Url);
	Request->SetVerb(TEXT("GET"));

	Request->OnProcessRequestComplete().BindLambda([OnResponse](FHttpRequestPtr Request, FHttpResponsePtr Response, const bool bSuccess) -> void
	{
		UE_LOG(LogDiscordAchievementEditor, Verbose, TEXT(" Achievements: %s"), *Response->GetContentAsString());
		if (!bSuccess)
		{
			UE_LOG(LogDiscordAchievementEditor, Error, TEXT("Failed to reach Discord server."));
			OnResponse.Execute(TArray<FAchievementInfo>(), false);
			return;
		}

		TSharedPtr<FJsonObject> JsonObject;

		TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(FString::Printf(TEXT("{\"r\":%s}"), *Response->GetContentAsString()));

		if (!FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			UE_LOG(LogDiscordAchievementEditor, Error, TEXT("Failed to deserialize server response."));
			OnResponse.Execute(TArray<FAchievementInfo>(), false);
			return;
		}		

		if (!JsonObject->HasTypedField<EJson::Array>(TEXT("r")))
		{
			UE_LOG(LogDiscordAchievementEditor, Error, TEXT("Invalid response from server: %s."), *Response->GetContentAsString());
			OnResponse.Execute(TArray<FAchievementInfo>(), false);
			return;
		}

		TArray<TSharedPtr<FJsonValue> > JsonAchievements = JsonObject->GetArrayField(TEXT("r"));

		TArray<FAchievementInfo> Achievements;

		for (const TSharedPtr<FJsonValue>& JsonAchievement : JsonAchievements)
		{
			Achievements.Add(CreateAchievementFromJsonObject(JsonAchievement->AsObject()));
		}

		OnResponse.Execute(Achievements, true);
	});

	if (!Request->ProcessRequest())
	{
		UE_LOG(LogDiscordAchievementEditor, Error, TEXT("Failed to launch achievement update request."));
		OnResponse.Execute(TArray<FAchievementInfo>(), false);
	}
}

void FAchievementManager::CreateAchievement(const FAchievementInfo& Achievement, const FString& ImageData, const FOnAchievementCreated& OnCreated)
{
	FHttpRequestRef Request = CreateRequestForDiscordApi();

	const FString Url = TEXT("https://discord.com/api/v6/applications") / UDiscordSettings::GetClientIdAsString() / TEXT("achievements");

	const FString Data =
		FString::Printf(
			TEXT("{")
			TEXT(    "\"name\":")
			TEXT(    "{")
			TEXT(        "\"default\": \"%s\"")
			TEXT(     "},")
			TEXT(     "\"description\":")
			TEXT(     "{")
			TEXT(         "\"default\":\"%s\"")
			TEXT(      "},")
			TEXT(      "\"secret\":%s,")
			TEXT(      "\"secure\":%s,")
			TEXT(      "\"icon\":\"data:image/png;base64,%s\"")
			TEXT("}"),
			*Achievement.Name,
			*Achievement.Description,
			(Achievement.bSecret ? TEXT("true") : TEXT("false")),
			(Achievement.bSecure ? TEXT("true") : TEXT("false")),
			*ImageData
		);

	Request->SetVerb(TEXT("POST"));
	Request->SetURL(Url);
	Request->AppendToHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(Data);


	Request->OnProcessRequestComplete().BindLambda([OnCreated](FHttpRequestPtr Request, FHttpResponsePtr Response, const bool bSuccess)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Achievement Response: %s"), *Response->GetContentAsString());

		if (!bSuccess)
		{
			UE_LOG(LogDiscordAchievementEditor, Error, TEXT("Failed to reach Discord server."));
			OnCreated.ExecuteIfBound(false);
			return;
		}

		TSharedPtr<FJsonObject> JsonObject;

		TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(FString::Printf(TEXT("{\"r\":%s}"), *Response->GetContentAsString()));

		if (!FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			UE_LOG(LogDiscordAchievementEditor, Error, TEXT("Failed to read Discord response."));
			OnCreated.ExecuteIfBound(false);
			return;
		}	

		const bool bValid = JsonObject->GetObjectField(TEXT("r"))->HasTypedField<EJson::String>(TEXT("application_id"));
		UE_LOG(LogTemp, Error, TEXT("%d"), bValid);
		OnCreated.ExecuteIfBound(bValid);
	});
	Request->ProcessRequest();
}

FAchievementInfo FAchievementManager::CreateAchievementFromJsonObject(const TSharedPtr<FJsonObject> & Object)
{
	FAchievementInfo Info;

	{
		const TSharedPtr<FJsonObject>* NameObjectPtr = nullptr;
		if (Object->TryGetObjectField(TEXT("name"), NameObjectPtr))
		{
			(*NameObjectPtr)->TryGetStringField(TEXT("default"), Info.Name);
		}
	}

	{
		const TSharedPtr<FJsonObject>* DescObjectPtr = nullptr;
		if (Object->TryGetObjectField(TEXT("description"), DescObjectPtr))
		{
			(*DescObjectPtr)->TryGetStringField(TEXT("default"), Info.Description);
		}
	}

	Object->TryGetStringField(TEXT("id"), Info.Id);
	Object->TryGetBoolField(TEXT("secure"), Info.bSecure);
	Object->TryGetBoolField(TEXT("secret"), Info.bSecret);

	return Info;
}

SDiscordAchievementWindow::SDiscordAchievementWindow() 
	: SCompoundWidget()
	, AchievementManager(MakeUnique<FAchievementManager>())
{
}

SDiscordAchievementWindow::~SDiscordAchievementWindow()
{
}

void SDiscordAchievementWindow::Construct(const FArguments& InArgs)
{
	LoadAchievements();

	ChildSlot
		.VAlign(VAlign_Fill) 
	[
		SNew(SBorder)
			.BorderImage(&BackgroundStyle)
			.VAlign(VAlign_Fill)
			.Padding(20.f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
				.VAlign(VAlign_Top)
				.AutoHeight()
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.FillWidth(2.f)
				[
					MakeText(TEXT("Discord Achievements"), 25)
				]

				+ SHorizontalBox::Slot()
					.HAlign(HAlign_Right)
					.AutoWidth()
					.Padding(FMargin(7.f, 0.f))
				[
					SNew(SBorder)
						.BorderImage(&BlackStyle)
					[
						SNew(SButton)
							.ButtonStyle(FDiscordSdkEditorStyle::GetDiscordButtonStyle())
							.OnClicked(FOnClicked::CreateRaw(this, &SDiscordAchievementWindow::OnButtonCreateClicked))
							.Cursor(EMouseCursor::Hand)
						[
							SNew(SBox)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.WidthOverride(200.f)
							[
								MakeText(TEXT("CREATE"), 13)
							]
						]
					]
				]

				+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
				[
					SNew(SBorder)
						.BorderImage(&BlackStyle)
					[
						SNew(SButton)
							.ButtonStyle(FDiscordSdkEditorStyle::GetDiscordButtonStyle())
							.OnClicked(FOnClicked::CreateRaw(this, &SDiscordAchievementWindow::OnButtonRefreshClicked))
							.Cursor(EMouseCursor::Hand)
						[
							SNew(SBox)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.WidthOverride(200.f)
							[
								MakeText(TEXT("REFRESH"), 13)
							]
						]
					]
				]
			]

			+ SVerticalBox::Slot()
				.AutoHeight()
			[
				SNew(SSpacer)
					.Size(FVector2D{0.f, 20.f})
			]

			+ SVerticalBox::Slot()	
				.VAlign(VAlign_Top)
				.AutoHeight()
			[
				SNew(SBorder)
						.BorderImage(&BlackStyle)
				[
					SNew(SBorder)
						.BorderImage(&AchievementHeadRowStyle)
						.Padding(FMargin{10.f, 10.f})
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
							.FillWidth(1.f)
						[
							MakeText(TEXT("Name"), 18)
						]
						
						+ SHorizontalBox::Slot()
							.FillWidth(1.5f)
						[
							MakeText(TEXT("Description"), 18)
						]
						
						+ SHorizontalBox::Slot()
							.FillWidth(0.7f)
							.HAlign(HAlign_Center)
						[
							MakeText(TEXT("ID"), 18)
						]
						
						+ SHorizontalBox::Slot()
							.FillWidth(0.3f)
							.HAlign(HAlign_Center)
						[
							MakeText(TEXT("Secure"), 18)
						]
						
						+ SHorizontalBox::Slot()
							.FillWidth(0.3f)
							.HAlign(HAlign_Center)
						[
							MakeText(TEXT("Secret"), 18)
						]
					]
				]
			]
			
			+ SVerticalBox::Slot()
				.VAlign(VAlign_Fill)
			[	

				SNew(SBorder)
					.BorderImage(&BlackStyle)
				[
					MakeAchievementList()
				]
			]
			
			+ SVerticalBox::Slot()
				.AutoHeight()
			[
				SNew(SSpacer)
					.Size(FVector2D{0.f, 20.f})
			]
		]
	];
}

TSharedRef<STextBlock> SDiscordAchievementWindow::MakeText(const FString Text, const uint16 Size)
{
	return
		SNew(STextBlock)
			.Text(FText::FromString(Text))
			.ColorAndOpacity(FontColor)
			.Font(GetDefaultFont(Size))
			.Clipping(EWidgetClipping::ClipToBoundsWithoutIntersecting);
}

void SDiscordAchievementWindow::BuildAchievements()
{
	AchievementsContainer->ClearChildren();

	if (AchievementsList.Num() <= 0)
	{
		FString StatusText = TEXT("Invalid");
		switch (Status)
		{
		case EDiscordAchievementStatus::Over :   StatusText = TEXT("Achievement list is empty");  break;
		case EDiscordAchievementStatus::Error:   StatusText = TEXT("Failed to get Achievements"); break;
		case EDiscordAchievementStatus::Loading: StatusText = TEXT("Loading..."); break;
		}

		AchievementsContainer->AddSlot()
			.Padding(FMargin{40.f})
		[
			MakeText(StatusText, 24)
		];
		return;
	}

	
	for (int32 i = 0; i < AchievementsList.Num(); ++i)
	{
		const FAchievementInfo& Achievement = AchievementsList[i];

		AchievementsContainer->AddSlot()
			.VAlign(VAlign_Top)
			.AutoHeight()
		[
			SNew(SBorder)
				.BorderImage((i % 2 ? &AchievementEvenRowStyle : &AchievementOddRowStyle))
				.Padding(FVector2D{12.f, 7.f})
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
					.FillWidth(1.f)
				[
					MakeText(Achievement.Name)
				]
				
				+ SHorizontalBox::Slot()
					.FillWidth(1.5f)
				[
					MakeText(Achievement.Description)
				]
				
				+ SHorizontalBox::Slot()
					.FillWidth(0.7f)
					.HAlign(HAlign_Center)
				[
					SNew(SEditableTextBox)
						.Text(FText::FromString(Achievement.Id))
						.Font(GetDefaultFont(12))
						.IsEnabled(true)
						.IsReadOnly(true)
						.BackgroundColor(FLinearColor::Transparent)
				]
				
				+ SHorizontalBox::Slot()
					.FillWidth(0.3f)
					.HAlign(HAlign_Center)
				[
					MakeText((Achievement.bSecure ? TEXT("Yes") : TEXT("No")), 14)
				]
				
				+ SHorizontalBox::Slot()
					.HAlign(HAlign_Center)
					.FillWidth(0.3f)
				[
					MakeText((Achievement.bSecret ? TEXT("Yes") : TEXT("No")), 14)
				]
			]
		];
	}
}

TSharedRef<SWidget> SDiscordAchievementWindow::MakeAchievementList()
{
	return 

	SNew(SScrollBox)
		.Orientation(EOrientation::Orient_Vertical)
	+ SScrollBox::Slot()
		.VAlign(VAlign_Fill)
	[
		SAssignNew(AchievementsContainer, SVerticalBox)
		
		+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.Padding(FMargin{ 40.f })
		[
			MakeText(TEXT("Loading Achievements..."), 24)
		]
	];
}

FSlateFontInfo SDiscordAchievementWindow::GetDefaultFont(const uint16 Size)
{
	static const FString DefaultFontPath = FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf");
	return FSlateFontInfo{ DefaultFontPath , Size };
}

void SDiscordAchievementWindow::LoadAchievements()
{
	Status = EDiscordAchievementStatus::Loading;
	AchievementManager->LoadAchievements(FOnGetAchievementsResponse::CreateRaw(this, &SDiscordAchievementWindow::OnAchievementsReceived));
}

void SDiscordAchievementWindow::OnAchievementsReceived(const TArray<FAchievementInfo>& Achievements, const bool bSuccess)
{
	if (bSuccess)
	{
		Status = EDiscordAchievementStatus::Over;
		AchievementsList = Achievements;
	}
	else
	{
		AchievementsList.Empty();
		Status = EDiscordAchievementStatus::Error;
	}
	BuildAchievements();
}

FReply SDiscordAchievementWindow::OnButtonRefreshClicked()
{
	Status = EDiscordAchievementStatus::Loading;
	LoadAchievements();
	return FReply::Handled();
}

FReply SDiscordAchievementWindow::OnButtonCreateClicked()
{
	TSharedPtr<SWindow> Window;

	SAssignNew(Window, SWindow)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.Title(FText::FromString(TEXT("Create an Achievement")))
		.SizingRule(ESizingRule::FixedSize)
		.ClientSize(FVector2D{ 800.f, 400.f });

	Window->SetContent(
		SNew(SDiscordCreateAchievement)
			.Window(Window)
			.AchievementWindow(StaticCastSharedRef<SDiscordAchievementWindow>(AsShared()))
	);
	
	RegisterWindow(Window.ToSharedRef());

	return FReply::Handled();
}

void SDiscordCreateAchievement::Construct(const FArguments& InArgs)
{
	constexpr uint16 LabelSize = 17;
	constexpr uint16 InputSize = 13;

	const FSlateFontInfo LabelFont = SDiscordAchievementWindow::GetDefaultFont(LabelSize);
	const FSlateFontInfo InputFont = SDiscordAchievementWindow::GetDefaultFont(InputSize);

	ParentWindow	  = InArgs._Window.Get();
	AchievementWindow = InArgs._AchievementWindow.Get();

	ChildSlot
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
	[
		SNew(SBorder)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.BorderImage(&SDiscordAchievementWindow::BackgroundStyle)
		[
			SNew(SBorder)
				.BorderImage(&SDiscordAchievementWindow::BackgroundStyle)
				.Padding(FMargin{ 10.f })
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoHeight()
					.Padding(FMargin(0.f, 0.f, 0.f, 10.f))
					
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Create a new Achievement")))
						.ColorAndOpacity(SDiscordAchievementWindow::FontColor)
						.Font(SDiscordAchievementWindow::GetDefaultFont(24))
				]

				+ SVerticalBox::Slot()
				[
					SNew(SBorder)
						.Padding(FMargin(80.f, 20.f))
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
								.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
									.Font(LabelFont)
									.Text(FText::FromString(TEXT("Achievement Name")))
									.ColorAndOpacity(SDiscordAchievementWindow::FontColor)
							]
	
							+ SHorizontalBox::Slot()
								.VAlign(VAlign_Center)
							[
								SAssignNew(AchievementName, SEditableTextBox)
									.Font(InputFont)
							]
						]

						+ SVerticalBox::Slot()
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
								.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
									.Font(LabelFont)
									.Text(FText::FromString(TEXT("Achievement Description")))
									.ColorAndOpacity(SDiscordAchievementWindow::FontColor)
							]
	
							+ SHorizontalBox::Slot()
								.VAlign(VAlign_Center)
							[
								SAssignNew(AchievementDescription, SEditableTextBox)
									.Font(InputFont)
							]
						]

						+ SVerticalBox::Slot()
						[
							SNew(SHorizontalBox)
							
							+ SHorizontalBox::Slot()
								.VAlign(VAlign_Center)
								.HAlign(HAlign_Right)
							[
								SNew(STextBlock)
									.Font(LabelFont)
									.Text(FText::FromString(TEXT("Secure")))
									.ColorAndOpacity(SDiscordAchievementWindow::FontColor)
							]
	
							+ SHorizontalBox::Slot()
								.HAlign(HAlign_Left)
								.VAlign(VAlign_Center)
								.Padding(FMargin(5.f, 0.f))
							[
								SAssignNew(AchievementSecure, SCheckBox)
							]
							
							+ SHorizontalBox::Slot()
								.VAlign(VAlign_Center)
								.HAlign(HAlign_Right)
							[
								SNew(STextBlock)
									.Font(LabelFont)
									.Text(FText::FromString(TEXT("Secret")))
									.ColorAndOpacity(SDiscordAchievementWindow::FontColor)
							]
	
							+ SHorizontalBox::Slot()
								.VAlign(VAlign_Center)
								.HAlign(HAlign_Left)
								.Padding(FMargin(5.f, 0.f))
							[
								SAssignNew(AchievementSecret, SCheckBox)
							]
						]
						

						+ SVerticalBox::Slot()
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
								.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
									.Font(LabelFont)
									.Text(FText::FromString(TEXT("Achievement Image")))
									.ColorAndOpacity(SDiscordAchievementWindow::FontColor)
							]
	
							+ SHorizontalBox::Slot()
								.VAlign(VAlign_Center)
								.HAlign(HAlign_Fill)
							[
								SAssignNew(AchievementImage, SEditableTextBox)
									.Font(InputFont)
							]
	
							+ SHorizontalBox::Slot()
								.VAlign(VAlign_Center)
								.HAlign(HAlign_Center)
								.AutoWidth()
								.Padding(FMargin{5.f, 0.f})
							[
								SNew(SButton)
									.ContentPadding(FMargin{5.f, 0.f})
									.Cursor(EMouseCursor::Hand)
									.ButtonStyle(FDiscordSdkEditorStyle::GetDiscordButtonStyle())
									.OnClicked(FOnClicked::CreateRaw(this, &SDiscordCreateAchievement::OnBrowse))
								[
									SNew(STextBlock)
										.ColorAndOpacity(SDiscordAchievementWindow::FontColor)
										.Text(FText::FromString(TEXT("...")))
										.Font(InputFont)
								]
							]
						]

						+ SVerticalBox::Slot()
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
						[
							SNew(SBorder)
								.BorderImage(&SDiscordAchievementWindow::BlackStyle)
							[
								SNew(SButton)
									.ButtonStyle(FDiscordSdkEditorStyle::GetDiscordButtonStyle())
									.Cursor(EMouseCursor::Hand)
									.OnClicked(FOnClicked::CreateRaw(this, &SDiscordCreateAchievement::OnCreate))
								[
									SNew(SBox)
										.HAlign(HAlign_Center)
										.VAlign(VAlign_Center)
										.WidthOverride(200.f)
									[
										SNew(STextBlock)
											.Font(SDiscordAchievementWindow::GetDefaultFont(13))
											.Text(FText::FromString(TEXT("CREATE")))
											.ColorAndOpacity(SDiscordAchievementWindow::FontColor)
									]
								]
							]
						]
					]
				]
			]
		]
	];
}

FReply SDiscordCreateAchievement::OnBrowse()
{
	IDesktopPlatform* const DesktopPlatform = FDesktopPlatformModule::Get();
	
	if (!DesktopPlatform)
	{
		UE_LOG(LogDiscordAchievementEditor, Error, TEXT("Failed to get Desktop Platform."));
		return FReply::Handled();
	}

	TArray<FString> Files;

	if (!DesktopPlatform->OpenFileDialog(
		GEngine->GameViewport ? GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle() : nullptr,
		TEXT("Select an image for the Achievement"),
		FPaths::GetProjectFilePath(),
		TEXT(""),
		TEXT("PNG Image|*.png"),
		0,
		Files)
	)
	{
		return FReply::Handled();
	}

	const FString File = Files[0];

	AchievementImage->SetText(FText::FromString(File));

	return FReply::Handled();
}

FReply SDiscordCreateAchievement::OnCreate()
{
	TSharedPtr<FSlowTask> Task = MakeShared<FSlowTask>(100.f, FText::FromString(TEXT("Creating Achievement...")));

	Task->Initialize();
	Task->MakeDialog();

	FAchievementManager Manager;

	FAchievementInfo Info;

	TArray<uint8> Image;
	if (!FFileHelper::LoadFileToArray(Image, *AchievementImage->GetText().ToString()))
	{
		Task->Destroy();
		return FReply::Handled();
	}

	Info.bSecret = AchievementSecret->IsChecked();
	Info.bSecure = AchievementSecure->IsChecked();

	Info.Description = AchievementDescription->GetText().ToString();
	Info.Name		 = AchievementName->GetText().ToString();

	TSharedRef<SWidget> Self = AsShared();

	Manager.CreateAchievement(Info, FBase64::Encode(Image), FOnAchievementCreated::CreateLambda([Self, Task](const bool bSuccess) 
	{
		if (bSuccess)
		{
			SDiscordCreateAchievement& This = static_cast<SDiscordCreateAchievement&>(*Self);
			UE_LOG(LogDiscordAchievementEditor, Log, TEXT("New Achievement created."));
			if (This.ParentWindow.IsValid())
			{
				UE_LOG(LogDiscordAchievementEditor, Log, TEXT("Destroying new Achievement window."));
				FSlateApplication::Get().DestroyWindowImmediately(This.ParentWindow.ToSharedRef());
			}
			if (This.AchievementWindow.IsValid())
			{
				This.AchievementWindow->OnButtonRefreshClicked();
			}
		}
		else
		{
			UE_LOG(LogDiscordAchievementEditor, Log, TEXT("Failed to create new Achievement."));
		}
		Task->Destroy();
	}));

	return FReply::Handled();
}
