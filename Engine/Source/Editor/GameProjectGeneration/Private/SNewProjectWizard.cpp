// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.


#include "GameProjectGenerationPrivatePCH.h"
#include "MainFrame.h"
#include "DesktopPlatformModule.h"
#include "SourceCodeNavigation.h"
#include "SScrollBorder.h"
#include "TemplateCategory.h"
#include "GameProjectGenerationModule.h"
#include "SWizard.h"


#define LOCTEXT_NAMESPACE "NewProjectWizard"

FName SNewProjectWizard::TemplatePageName = TEXT("Template");
FName SNewProjectWizard::NameAndLocationPageName = TEXT("NameAndLocation");


namespace
{
	const float ThumbnailSize = 64.f, ThumbnailPadding = 5.f;
	const float ItemWidth = ThumbnailSize + 2*ThumbnailPadding;
	const float ItemHeight = ItemWidth + 30;
}

/** Struct describing a single template project */
struct FTemplateItem
{
	FText		Name;
	FText		Description;
	bool		bGenerateCode;
	FName		Type;

	FString		SortKey;
	FString		ProjectFile;

	TSharedPtr<FSlateBrush> Thumbnail;
	TSharedPtr<FSlateBrush> PreviewImage;

	FTemplateItem(FText InName, FText InDescription, bool bInGenerateCode, FName InType, FString InSortKey, FString InProjectFile, TSharedPtr<FSlateBrush> InThumbnail, TSharedPtr<FSlateBrush> InPreviewImage)
		: Name(InName), Description(InDescription), bGenerateCode(bInGenerateCode), Type(InType), SortKey(MoveTemp(InSortKey)), ProjectFile(MoveTemp(InProjectFile)), Thumbnail(InThumbnail), PreviewImage(InPreviewImage)
	{}
};

/**
 * Simple widget used to display a folder path, and a name of a file:
 * __________________________  ____________________
 * | C:\Users\Joe.Bloggs    |  | SomeFile.txt     |
 * |-------- Folder --------|  |------ Name ------|
 */
class SFilepath : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS( SFilepath )
		: _LabelBackgroundColor(FLinearColor::Black)
		, _LabelBackgroundBrush(FEditorStyle::GetBrush("WhiteBrush"))
	{}
		/** Attribute specifying the text to display in the folder input */
		SLATE_ATTRIBUTE(FText, FolderPath)

		/** Attribute specifying the text to display in the name input */
		SLATE_ATTRIBUTE(FText, Name)

		/** Background label tint for the folder/name labels */
		SLATE_ATTRIBUTE(FSlateColor, LabelBackgroundColor)

		/** Background label brush for the folder/name labels */
		SLATE_ATTRIBUTE(const FSlateBrush*, LabelBackgroundBrush)

		/** Event that is triggered when the browser for folder button is clicked */
		SLATE_EVENT(FOnClicked, OnBrowseForFolder)

		/** Events for when the name field is manipulated */
		SLATE_EVENT(FOnTextChanged, OnNameChanged)
		SLATE_EVENT(FOnTextCommitted, OnNameCommitted)
		
		/** Events for when the folder field is manipulated */
		SLATE_EVENT(FOnTextChanged, OnFolderChanged)
		SLATE_EVENT(FOnTextCommitted, OnFolderCommitted)

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct( const FArguments& InArgs )
	{
		ChildSlot
		[
			SNew(SGridPanel)
			.FillColumn(0, 2.f)
			.FillColumn(1, 1.f)

			// Folder input
			+ SGridPanel::Slot(0, 0)
			[
				SNew(SOverlay)

				+ SOverlay::Slot()
				[
					SNew(SEditableTextBox)
					.Text(InArgs._FolderPath)
					// Large right hand padding to make room for the browse button
					.Padding(FMargin(5.f, 3.f, 25.f, 3.f))
					.OnTextChanged(InArgs._OnFolderChanged)
					.OnTextCommitted(InArgs._OnFolderCommitted)
				]
					
				+ SOverlay::Slot()
				.HAlign(HAlign_Right)
				[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "FilePath.FolderButton")
					.ContentPadding(FMargin(4.f, 0.f))
					.OnClicked(InArgs._OnBrowseForFolder)
					.ToolTipText(LOCTEXT("BrowseForFolder", "Browse for a folder"))
					.Text(LOCTEXT("...", "..."))
				]
			]

			// Folder label
			+ SGridPanel::Slot(0, 1)
			[
				SNew(SOverlay)

				+ SOverlay::Slot()
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.HeightOverride(3)
					[
						SNew(SBorder)
						.BorderImage(FEditorStyle::GetBrush("FilePath.GroupIndicator"))
						.BorderBackgroundColor(FLinearColor(1.f, 1.f, 1.f, 0.5f))
						.Padding(FMargin(150.f, 0.f))
					]
				]
					
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SBorder)
					.Padding(5.f)
					.BorderImage(InArgs._LabelBackgroundBrush)
					.BorderBackgroundColor(InArgs._LabelBackgroundColor)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Folder", "Folder"))
					]
				]
			]

			// Name input
			+ SGridPanel::Slot(1, 0)
			.Padding(FMargin(5.f, 0.f, 0.f, 0.f))
			.VAlign(VAlign_Center)
			[
				SNew(SEditableTextBox)
				.Text(InArgs._Name)
				.Padding(FMargin(5.f, 3.f))
				.OnTextChanged(InArgs._OnNameChanged)
				.OnTextCommitted(InArgs._OnNameCommitted)
			]

			// Name label
			+ SGridPanel::Slot(1, 1)
			.Padding(FMargin(5.f, 0.f, 0.f, 0.f))
			[
				SNew(SOverlay)

				+ SOverlay::Slot()
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.HeightOverride(3)
					[
						SNew(SBorder)
						.BorderImage(FEditorStyle::GetBrush("FilePath.GroupIndicator"))
						.BorderBackgroundColor(FLinearColor(1.f, 1.f, 1.f, 0.5f))
						.Padding(FMargin(75.f, 0.f))
					]
				]
					
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SBorder)
					.Padding(5.f)
					.BorderImage(InArgs._LabelBackgroundBrush)
					.BorderBackgroundColor(InArgs._LabelBackgroundColor)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Name", "Name"))
					]
				]
			]
		];
	}
};

/** Slate tile widget for template projects */
class STemplateTile : public STableRow<TSharedPtr<FTemplateItem>>
{
public:
	SLATE_BEGIN_ARGS( STemplateTile ){}
		SLATE_ARGUMENT( TSharedPtr<FTemplateItem>, Item )
	SLATE_END_ARGS()

private:
	TWeakPtr<FTemplateItem> Item;

public:
	/** Static build function */
	static TSharedRef<ITableRow> BuildTile(TSharedPtr<FTemplateItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
	{
		if (!ensure(Item.IsValid()))
		{
			return SNew(STableRow<TSharedPtr<FTemplateItem>>, OwnerTable);
		}

		return SNew(STemplateTile, OwnerTable).Item(Item);
	}

	/** Constructs this widget with InArgs */
	void Construct( const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable )
	{
		check(InArgs._Item.IsValid())
		Item = InArgs._Item;

		STableRow::Construct(
			STableRow::FArguments()
			.Style(FEditorStyle::Get(), "GameProjectDialog.TemplateListView.TableRow")
			.Content()
			[
				SNew(SVerticalBox)

				// Thumbnail
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(SBox)
					.Padding(ThumbnailPadding)
					.WidthOverride(ThumbnailSize)
					.HeightOverride(ThumbnailSize)
					[
						SNew(SImage)
						.Image(this, &STemplateTile::GetThumbnail)
					]
				]

				// Name
				+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				[
					SNew(SRichTextBlock)
					.WrapTextAt(ItemWidth - ThumbnailPadding*2)
					.Justification(ETextJustify::Center)
					//.HighlightText(this, &SNewProjectWizard::GetItemHighlightText)
					.Text(InArgs._Item->Name)
				]
			],
			OwnerTable
		);
	}

private:

	/** Get this item's thumbnail or return the default */
	const FSlateBrush* GetThumbnail() const
	{
		auto ItemPtr = Item.Pin();
		if (ItemPtr.IsValid() && ItemPtr->Thumbnail.IsValid())
		{
			return ItemPtr->Thumbnail.Get();
		}
		return FEditorStyle::GetBrush("GameProjectDialog.DefaultGameThumbnail");
	}

};

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SNewProjectWizard::Construct( const FArguments& InArgs )
{
	LastValidityCheckTime = 0;
	ValidityCheckFrequency = 4;
	bLastGlobalValidityCheckSuccessful = true;
	bLastNameAndLocationValidityCheckSuccessful = true;
	bPreventPeriodicValidityChecksUntilNextChange = false;
	 
	// Find all template projects
	FindTemplateProjects();
	SetDefaultProjectLocation();

	SAssignNew(TemplateListView, STileView< TSharedPtr<FTemplateItem> >)
	.ListItemsSource(&FilteredTemplateList)
	.SelectionMode(ESelectionMode::Single)
	.ClearSelectionOnClick(false)
	.OnGenerateTile_Static(&STemplateTile::BuildTile)
	.ItemHeight(ItemHeight)
	.ItemWidth(ItemWidth)
	.OnMouseButtonDoubleClick(this, &SNewProjectWizard::HandleTemplateListViewDoubleClick)
	.OnSelectionChanged(this, &SNewProjectWizard::HandleTemplateListViewSelectionChanged);

	const FMargin StandardVerticalPad(0.f, 0.f, 0.f, 10.f);
	const EVisibility StarterContentVisiblity = GameProjectUtils::IsStarterContentAvailableForNewProjects() ? EVisibility::Visible : EVisibility::Collapsed;

	ChildSlot
	[
		SNew(SOverlay)

		// Wizard
		+SOverlay::Slot()
		[
			SAssignNew(MainWizard, SWizard)
			.ShowPageList(false)
			.ShowCancelButton(false)
			.CanFinish(this, &SNewProjectWizard::HandleCreateProjectWizardCanFinish)
			.FinishButtonText(LOCTEXT("FinishButtonText", "Create Project"))
			.FinishButtonToolTip(LOCTEXT("FinishButtonToolTip", "Creates your new project in the specified location with the specified template and name.") )
			.OnFinished(this, &SNewProjectWizard::HandleCreateProjectWizardFinished)
			.OnFirstPageBackClicked(InArgs._OnBackRequested)

			// Choose Template
			+SWizard::Page()
			.OnEnter(this, &SNewProjectWizard::OnPageVisited, TemplatePageName)
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("NoBorder"))
				.Padding(10.f)
				[
					SNew(SHorizontalBox)

					// Template category tabs
					+ SHorizontalBox::Slot()
					[
						SNew(SVerticalBox)

						+SVerticalBox::Slot()
						.Padding(FMargin(5.f, 0.f))
						.AutoHeight()
						[
							BuildCategoryTabs()
						]

						// Templates list
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						// Manual padding offset to align to the bottom of the file path widget
						.Padding(FMargin(0,0,0,13.f))
						[
							SNew(SBorder)
							.Padding(10)
							.BorderImage(FEditorStyle::GetBrush("GameProjectDialog.TabBackground"))
							[
								SNew(SScrollBorder, TemplateListView.ToSharedRef())
								[
									TemplateListView.ToSharedRef()
								]
							]
						]
					]

					// Selected template details
					+ SHorizontalBox::Slot()
					.AutoWidth()
					// top-offset fudge to account for size of tabs
					.Padding(FMargin(20.0f, 42.f, 0.f, 0.f))
					[
						// This pane is always 512px wide - the width of the template preview
						SNew(SBox)
						.WidthOverride(512)
						[
							SNew(SVerticalBox)

							// Preview image
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.Padding(StandardVerticalPad)
							[
								SNew(SOverlay)
							
								+ SOverlay::Slot()
								[
									SNew(SBorder)
									.Padding(FMargin(0, 0, 0, 5.f))
									.BorderImage(FEditorStyle::GetBrush("ContentBrowser.ThumbnailShadow"))
									[
										SNew(SImage)
										.Image(this, &SNewProjectWizard::GetSelectedTemplatePreviewImage)
									]
								]

								+ SOverlay::Slot()
								.HAlign(HAlign_Right)
								.VAlign(VAlign_Top)
								.Padding(10)
								[
									SNew(SBox)
									.WidthOverride(48)
									.HeightOverride(48)
									[
										SNew(SImage)
										.Image(this, &SNewProjectWizard::GetSelectedTemplateTypeImage)
									]
								]
							]

							// Template Name
							+ SVerticalBox::Slot()
							.Padding(StandardVerticalPad)
							.AutoHeight()
							[
								SNew(STextBlock)
								.AutoWrapText(true)
								.TextStyle(FEditorStyle::Get(), "GameProjectDialog.FeatureText")
								.Text(this, &SNewProjectWizard::GetSelectedTemplateProperty<FText>, &FTemplateItem::Name)
							]
						
							// Template Description
							+ SVerticalBox::Slot()
							.Padding(StandardVerticalPad)
							[
								SNew(SScrollBox)
								+ SScrollBox::Slot()
								[
									SNew(STextBlock)
									.AutoWrapText(true)
									.Text(this, &SNewProjectWizard::GetSelectedTemplateProperty<FText>, &FTemplateItem::Description)
								]
							]
						
							// Include Starter Content?
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(StandardVerticalPad)
							[
								SNew(SCheckBox)
								.Visibility(StarterContentVisiblity)
								.ToolTipText( LOCTEXT("CopyStarterContent_ToolTip", "Adds example content to your new project, including simple placeable meshes with basic materials and textures.\nYou can turn this off to start with a project that is only has the bare essentials for the selected project template."))
								.OnCheckStateChanged(this, &SNewProjectWizard::OnCopyStarterContentChanged)
								.IsChecked(this, &SNewProjectWizard::IsCopyStarterContentChecked)
								.Content()
								[
									SNew(STextBlock)
									.Text(LOCTEXT("CopyStarterContent", "Include starter content" ))
 								]
							]

							// Project name error text
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(StandardVerticalPad)
							[
								SNew(SBorder)
								.BorderImage(FEditorStyle::GetBrush("GameProjectDialog.ErrorLabelBorder"))
								.Visibility(this, &SNewProjectWizard::GetNameAndLocationErrorLabelVisibility)
								.Padding(5.f)
								.Content()
								[
									SNew(STextBlock)
									.Text(this, &SNewProjectWizard::GetNameAndLocationErrorLabelText)
									.TextStyle(FEditorStyle::Get(), "GameProjectDialog.ErrorLabelFont")
								]
							]

							// File path widget
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(SFilepath)
								.OnBrowseForFolder(this, &SNewProjectWizard::HandleBrowseButtonClicked)
								.LabelBackgroundBrush(FEditorStyle::GetBrush("ProjectBrowser.Background"))
								.LabelBackgroundColor(FLinearColor::White)
								.FolderPath(this, &SNewProjectWizard::GetCurrentProjectFilePath)
								.Name(this, &SNewProjectWizard::GetCurrentProjectFileName)
								.OnFolderChanged(this, &SNewProjectWizard::OnCurrentProjectFilePathChanged)
								.OnNameChanged(this, &SNewProjectWizard::OnCurrentProjectFileNameChanged)
							]
						]
					]
				]
			]
		]

		// Global Error label
		+SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding( FMargin( 0.0f, 5.0f, 0.0f, 0.0f ) )
		[
			// Constant height, whether the label is visible or not
			SNew(SBox)
			.HeightOverride(20.0f)
			[
				SNew(SBorder)
				.Visibility(this, &SNewProjectWizard::GetGlobalErrorLabelVisibility)
				.BorderImage(FEditorStyle::GetBrush("GameProjectDialog.ErrorLabelBorder"))
				.Content()
				[
					SNew(SHorizontalBox)
										
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.FillWidth(1.0f)
					[
						SNew(STextBlock)
						.Text(this, &SNewProjectWizard::GetGlobalErrorLabelText)
						.TextStyle(FEditorStyle::Get(), TEXT("GameProjectDialog.ErrorLabelFont"))
					]

					// A link to a platform-specific IDE, only shown when a compiler is not available
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(SHyperlink)
						.Text(FText::Format(LOCTEXT("IDEDownloadLinkText", "Download {0}"), FSourceCodeNavigation::GetSuggestedSourceCodeIDE()))
						.OnNavigate(this, &SNewProjectWizard::OnDownloadIDEClicked, FSourceCodeNavigation::GetSuggestedSourceCodeIDEDownloadURL())
						.Visibility(this, &SNewProjectWizard::GetGlobalErrorLabelIDELinkVisibility)
					]
									
					// A button to close the persistent global error text
					+SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.ButtonStyle(FEditorStyle::Get(), "NoBorder")
						.ContentPadding(0.0f) 
						.OnClicked(this, &SNewProjectWizard::OnCloseGlobalErrorLabelClicked)
						.Visibility(this, &SNewProjectWizard::GetGlobalErrorLabelCloseButtonVisibility)
						[
							SNew(SImage)
							.Image(FEditorStyle::GetBrush("GameProjectDialog.ErrorLabelCloseButton"))
						]
					]
				]
			]
		]
	];

	// Initialize the current page name. Assuming the template page.
	CurrentPageName = TemplatePageName;

	HandleCategoryChanged(ESlateCheckBoxState::Checked, ActiveCategory);

	UpdateProjectFileValidity();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION


TSharedRef<SWidget> SNewProjectWizard::BuildCategoryTabs()
{
	TSharedRef<SHorizontalBox> TabStrip = SNew(SHorizontalBox);

	TArray<FName> Categories;
	Templates.GenerateKeyArray(Categories);

	for (const FName& CategoryName : Categories)
	{
		TSharedPtr<const FTemplateCategory> Category = FGameProjectGenerationModule::Get().GetCategory(CategoryName);

		TSharedPtr<SHorizontalBox> HorizontalBox;

		TabStrip->AddSlot().AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(FMargin(0,0,2.f,0))
		[
			SNew(SBox)
			// Constrain the height to 32px (for the image) plus 5px padding vertically
			.HeightOverride(32.f + 5.f*2)
			[
				SNew( SCheckBox )
				.Style( FEditorStyle::Get(), "GameProjectDialog.Tab" )
				.OnCheckStateChanged(this, &SNewProjectWizard::HandleCategoryChanged, CategoryName)
				.IsChecked(this, &SNewProjectWizard::GetCategoryTabCheckState, CategoryName)
				.ToolTipText(Category.IsValid() ? Category->Description : FText())
				.Padding(FMargin(5.f))
				[
					SAssignNew(HorizontalBox, SHorizontalBox)
				]
			]
		];


		if (Category.IsValid())
		{
			HorizontalBox->AddSlot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(32)
				.HeightOverride(32)
				[
					SNew(SImage)
					.Image(Category->Thumbnail)
				]
			];
		}

		HorizontalBox->AddSlot()
		.Padding(5.f, 0.f)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(STextBlock)
			.TextStyle(FEditorStyle::Get(), "GameProjectDialog.FeatureText")
			.Text(Category.IsValid() ? Category->Name : FText::FromString(CategoryName.ToString()))
		];
	}
	return TabStrip;
}

void SNewProjectWizard::OnCopyStarterContentChanged(ESlateCheckBoxState::Type InNewState)
{
	bCopyStarterContent = InNewState == ESlateCheckBoxState::Checked;
}
ESlateCheckBoxState::Type SNewProjectWizard::IsCopyStarterContentChecked() const
{
	return bCopyStarterContent ? ESlateCheckBoxState::Checked : ESlateCheckBoxState::Unchecked;
}

void SNewProjectWizard::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// Every few seconds, the project file path is checked for validity in case the disk contents changed and the location is now valid or invalid.
	// After project creation, periodic checks are disabled to prevent a brief message indicating that the project you created already exists.
	// This feature is re-enabled if the user did not restart and began editing parameters again.
	if ( !bPreventPeriodicValidityChecksUntilNextChange && (InCurrentTime > LastValidityCheckTime + ValidityCheckFrequency) )
	{
		UpdateProjectFileValidity();
	}
}


void SNewProjectWizard::HandleTemplateListViewSelectionChanged(TSharedPtr<FTemplateItem> TemplateItem, ESelectInfo::Type SelectInfo)
{
	UpdateProjectFileValidity();
}


TSharedPtr<FTemplateItem> SNewProjectWizard::GetSelectedTemplateItem() const
{
	TArray< TSharedPtr<FTemplateItem> > SelectedItems = TemplateListView->GetSelectedItems();
	if ( SelectedItems.Num() > 0 )
	{
		return SelectedItems[0];
	}
	
	return NULL;
}

const FSlateBrush* SNewProjectWizard::GetSelectedTemplatePreviewImage() const
{
	auto PreviewImage = GetSelectedTemplateProperty(&FTemplateItem::PreviewImage);
	return PreviewImage.IsValid() ? PreviewImage.Get() : nullptr;
}

const FSlateBrush* SNewProjectWizard::GetSelectedTemplateTypeImage() const
{
	TSharedPtr<FTemplateItem> SelectedItem = GetSelectedTemplateItem();
	if (SelectedItem.IsValid())
	{
		auto Category = FGameProjectGenerationModule::Get().GetCategory(SelectedItem->Type);
		if (Category.IsValid())
		{
			return Category->Thumbnail;
		}
	}
	return nullptr;
}


FText SNewProjectWizard::GetCurrentProjectFileName() const
{
	return FText::FromString( CurrentProjectFileName );
}


FString SNewProjectWizard::GetCurrentProjectFileNameStringWithExtension() const
{
	return CurrentProjectFileName + TEXT(".") + IProjectManager::GetProjectFileExtension();
}


void SNewProjectWizard::OnCurrentProjectFileNameChanged(const FText& InValue)
{
	CurrentProjectFileName = InValue.ToString();
	UpdateProjectFileValidity();
}


FText SNewProjectWizard::GetCurrentProjectFilePath() const
{
	return FText::FromString(CurrentProjectFilePath);
}


FString SNewProjectWizard::GetCurrentProjectFileParentFolder() const
{
	if ( CurrentProjectFilePath.EndsWith(TEXT("/")) || CurrentProjectFilePath.EndsWith("\\") )
	{
		return FPaths::GetCleanFilename( CurrentProjectFilePath.LeftChop(1) );
	}
	else
	{
		return FPaths::GetCleanFilename( CurrentProjectFilePath );
	}
}


void SNewProjectWizard::OnCurrentProjectFilePathChanged(const FText& InValue)
{
	CurrentProjectFilePath = InValue.ToString();
	FPaths::MakePlatformFilename(CurrentProjectFilePath);
	UpdateProjectFileValidity();
}


FString SNewProjectWizard::GetProjectFilenameWithPathLabelText() const
{
	return GetProjectFilenameWithPath();
}


FString SNewProjectWizard::GetProjectFilenameWithPath() const
{
	if ( CurrentProjectFilePath.IsEmpty() )
	{
		// Don't even try to assemble the path or else it may be relative to the binaries folder!
		return TEXT("");
	}
	else
	{
		const FString ProjectName = CurrentProjectFileName;
		const FString ProjectPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*CurrentProjectFilePath);
		const FString Filename = ProjectName + TEXT(".") + IProjectManager::GetProjectFileExtension();
		FString ProjectFilename = FPaths::Combine( *ProjectPath, *ProjectName, *Filename );
		FPaths::MakePlatformFilename(ProjectFilename);
		return ProjectFilename;
	}
}


FReply SNewProjectWizard::HandleBrowseButtonClicked()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if ( DesktopPlatform )
	{
		void* ParentWindowWindowHandle = NULL;

		IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
		const TSharedPtr<SWindow>& MainFrameParentWindow = MainFrameModule.GetParentWindow();
		if ( MainFrameParentWindow.IsValid() && MainFrameParentWindow->GetNativeWindow().IsValid() )
		{
			ParentWindowWindowHandle = MainFrameParentWindow->GetNativeWindow()->GetOSWindowHandle();
		}

		FString FolderName;
		const FString Title = LOCTEXT("NewProjectBrowseTitle", "Choose a project location").ToString();
		const bool bFolderSelected = DesktopPlatform->OpenDirectoryDialog(
			ParentWindowWindowHandle,
			Title,
			LastBrowsePath,
			FolderName
			);

		if ( bFolderSelected )
		{
			if ( !FolderName.EndsWith(TEXT("/")) )
			{
				FolderName += TEXT("/");
			}
			
			FPaths::MakePlatformFilename(FolderName);
			LastBrowsePath = FolderName;
			CurrentProjectFilePath = FolderName;
		}
	}

	return FReply::Handled();
}


void SNewProjectWizard::OnDownloadIDEClicked(FString URL)
{
	FPlatformProcess::LaunchURL( *URL, NULL, NULL );
}


void SNewProjectWizard::HandleTemplateListViewDoubleClick( TSharedPtr<FTemplateItem> TemplateItem )
{
	// Advance to the name/location page
	const int32 NamePageIdx = 1;
	if ( MainWizard->CanShowPage(NamePageIdx) )
	{
		MainWizard->ShowPage(NamePageIdx);
	}
}


bool SNewProjectWizard::IsCreateProjectEnabled() const
{
	if ( CurrentPageName == NAME_None )//|| CurrentPageName == TemplatePageName )
	{
		return false;
	}

	return bLastGlobalValidityCheckSuccessful && bLastNameAndLocationValidityCheckSuccessful;
}


bool SNewProjectWizard::HandlePageCanShow( FName PageName ) const
{
	if (PageName == NameAndLocationPageName)
	{
		return bLastGlobalValidityCheckSuccessful;
	}

	return true;
}


void SNewProjectWizard::OnPageVisited(FName NewPageName)
{
	CurrentPageName = NewPageName;
}


EVisibility SNewProjectWizard::GetGlobalErrorLabelVisibility() const
{
	return GetGlobalErrorLabelText().IsEmpty() ? EVisibility::Hidden : EVisibility::Visible;
}


EVisibility SNewProjectWizard::GetGlobalErrorLabelCloseButtonVisibility() const
{
	return PersistentGlobalErrorLabelText.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible;
}


EVisibility SNewProjectWizard::GetGlobalErrorLabelIDELinkVisibility() const
{
	return (IsCompilerRequired() && !FSourceCodeNavigation::IsCompilerAvailable()) ? EVisibility::Visible : EVisibility::Collapsed;
}


FText SNewProjectWizard::GetGlobalErrorLabelText() const
{
	if ( !PersistentGlobalErrorLabelText.IsEmpty() )
	{
		return PersistentGlobalErrorLabelText;
	}

	if ( !bLastGlobalValidityCheckSuccessful )
	{
		return LastGlobalValidityErrorText;
	}

	return FText::GetEmpty();
}


FReply SNewProjectWizard::OnCloseGlobalErrorLabelClicked()
{
	PersistentGlobalErrorLabelText = FText();

	return FReply::Handled();
}


EVisibility SNewProjectWizard::GetNameAndLocationErrorLabelVisibility() const
{
	return GetNameAndLocationErrorLabelText().IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible;
}


FText SNewProjectWizard::GetNameAndLocationErrorLabelText() const
{
	if ( !bLastNameAndLocationValidityCheckSuccessful )
	{
		return LastNameAndLocationValidityErrorText;
	}

	return FText::GetEmpty();
}

void SNewProjectWizard::FindTemplateProjects()
{
	// Default to showing the blueprint category
	ActiveCategory = FTemplateCategory::BlueprintCategoryName;

	// Add some default non-data driven templates
	Templates.FindOrAdd(FTemplateCategory::BlueprintCategoryName).Add(MakeShareable(new FTemplateItem(
		LOCTEXT("BlankProjectName", "Blank"),
		LOCTEXT("BlankProjectDescription", "A clean empty project with no code."),
		false, FTemplateCategory::BlueprintCategoryName,
		TEXT("_1"),			// SortKey
		TEXT(""),			// No filename, this is a generation template
		MakeShareable( new FSlateBrush( *FEditorStyle::GetBrush("GameProjectDialog.BlankProjectThumbnail") ) ),
		MakeShareable( new FSlateBrush( *FEditorStyle::GetBrush("GameProjectDialog.BlankProjectPreview") ) )
		)) );

	Templates.FindOrAdd(FTemplateCategory::CodeCategoryName).Add(MakeShareable(new FTemplateItem(
		LOCTEXT("BasicCodeProjectName", "Basic Code"),
		LOCTEXT("BasicCodeProjectDescription", "An empty project with some basic game framework code classes created."),
		true, FTemplateCategory::CodeCategoryName,
		TEXT("_2"),			// SortKey
		TEXT(""),			// No filename, this is a generation template
		MakeShareable( new FSlateBrush( *FEditorStyle::GetBrush("GameProjectDialog.BasicCodeThumbnail") ) ),
		MakeShareable( new FSlateBrush( *FEditorStyle::GetBrush("GameProjectDialog.BlankProjectPreview") ) )
		)) );

	// Now discover and all data driven templates
	TArray<FString> TemplateRootFolders;

	// @todo rocket make template folder locations extensible.
	TemplateRootFolders.Add( FPaths::RootDir() + TEXT("Templates") );

	// allow plugins to define templates
	TArray<FPluginStatus> PluginStatuses = IPluginManager::Get().QueryStatusForAllPlugins();
	for (const auto& PluginStatus : PluginStatuses)
	{
		if (PluginStatus.bIsEnabled && !PluginStatus.PluginDirectory.IsEmpty())
		{
			const FString PluginTemplatesDirectory = FPaths::Combine(*PluginStatus.PluginDirectory, TEXT("Templates"));

			if (IFileManager::Get().DirectoryExists(*PluginTemplatesDirectory))
			{
				TemplateRootFolders.Add(PluginTemplatesDirectory);
			}
		}
	}

	// Form a list of all folders that could contain template projects
	TArray<FString> AllTemplateFolders;
	for ( auto TemplateRootFolderIt = TemplateRootFolders.CreateConstIterator(); TemplateRootFolderIt; ++TemplateRootFolderIt )
	{
		const FString Root = *TemplateRootFolderIt;
		const FString SearchString = Root / TEXT("*");
		TArray<FString> TemplateFolders;
		IFileManager::Get().FindFiles(TemplateFolders, *SearchString, /*Files=*/false, /*Directories=*/true);
		for ( auto TemplateFolderIt = TemplateFolders.CreateConstIterator(); TemplateFolderIt; ++TemplateFolderIt )
		{
			AllTemplateFolders.Add( Root / (*TemplateFolderIt) );
		}
	}

	// Add a template item for every discovered project
	for ( auto TemplateFolderIt = AllTemplateFolders.CreateConstIterator(); TemplateFolderIt; ++TemplateFolderIt )
	{
		const FString SearchString = (*TemplateFolderIt) / TEXT("*.") + IProjectManager::GetProjectFileExtension();
		TArray<FString> FoundProjectFiles;
		IFileManager::Get().FindFiles(FoundProjectFiles, *SearchString, /*Files=*/true, /*Directories=*/false);
		if ( FoundProjectFiles.Num() > 0 )
		{
			if ( ensure(FoundProjectFiles.Num() == 1) )
			{
				// Make sure a TemplateDefs ini file exists
				const FString Root = *TemplateFolderIt;
				UTemplateProjectDefs* TemplateDefs = GameProjectUtils::LoadTemplateDefs(Root);
				if ( TemplateDefs )
				{
					// Found a template. Add it to the template items list.
					const FString ProjectFilename = Root / FoundProjectFiles[0];
					FText TemplateName = TemplateDefs->GetDisplayNameText();
					FText TemplateDescription = TemplateDefs->GetLocalizedDescription();

					// If no template name was specified for the current culture, just use the project name
					if ( TemplateName.IsEmpty() )
					{
						TemplateName = FText::FromString(FPaths::GetBaseFilename(ProjectFilename));
					}

					// Only generate code if the template has a source folder
					const bool bGenerateCode = TemplateDefs->GeneratesCode(Root);

					TSharedPtr<FSlateDynamicImageBrush> ThumbnailBrush;
					const FString ThumbnailPNGFile = FPaths::GetBaseFilename(ProjectFilename, false) + TEXT(".png");
					if ( FPlatformFileManager::Get().GetPlatformFile().FileExists(*ThumbnailPNGFile) )
					{
						const FName BrushName = FName(*ThumbnailPNGFile);
						ThumbnailBrush = MakeShareable( new FSlateDynamicImageBrush(BrushName , FVector2D(128,128) ) );
					}

					TSharedPtr<FSlateDynamicImageBrush> PreviewBrush;
					const FString PreviewPNGFile = FPaths::GetBaseFilename(ProjectFilename, false) + TEXT("_Preview.png");
					if ( FPlatformFileManager::Get().GetPlatformFile().FileExists(*PreviewPNGFile) )
					{
						const FName BrushName = FName(*PreviewPNGFile);
						PreviewBrush = MakeShareable( new FSlateDynamicImageBrush(BrushName , FVector2D(512,256) ) );
					}

					// Get the sort key
					FString SortKey = TemplateDefs->SortKey;
					if(SortKey.Len() == 0)
					{
						SortKey = FPaths::GetCleanFilename(ProjectFilename);
					}
					if (FPaths::GetCleanFilename(ProjectFilename) == GameProjectUtils::GetDefaultProjectTemplateFilename())
					{
						SortKey = TEXT("_0");
					}

					// Assign the template to the correct category. If the template has no explicit category assigned, we assign it to either C++ or blueprint
					FName Category = TemplateDefs->Category;
					if (Category.IsNone())
					{
						Category = bGenerateCode ? FTemplateCategory::CodeCategoryName : FTemplateCategory::BlueprintCategoryName;
					}

					Templates.FindOrAdd(Category).Add(MakeShareable(new FTemplateItem(
						TemplateName,
						TemplateDescription,
						bGenerateCode,
						Category,
						MoveTemp(SortKey),
						MoveTemp(ProjectFilename),
						ThumbnailBrush,
						PreviewBrush
					)));
				}
			}
			else
			{
				// More than one project file in this template? This is not legal, skip it.
				continue;
			}
		}
	}
}


void SNewProjectWizard::SetDefaultProjectLocation( )
{
	FString DefaultProjectFilePath;
	
	// First, try and use the first previously used path that still exists
	for ( const FString& CreatedProjectPath : GEditor->GetGameAgnosticSettings().CreatedProjectPaths )
	{
		if ( IFileManager::Get().DirectoryExists(*CreatedProjectPath) )
		{
			DefaultProjectFilePath = CreatedProjectPath;
			break;
		}
	}

	if ( DefaultProjectFilePath.IsEmpty() )
	{
		// No previously used path, decide a default path.
		DefaultProjectFilePath = FDesktopPlatformModule::Get()->GetDefaultProjectCreationPath();
		IFileManager::Get().MakeDirectory(*DefaultProjectFilePath, true);
	}

	if ( !DefaultProjectFilePath.IsEmpty() && DefaultProjectFilePath.Right(1) == TEXT("/") )
	{
		DefaultProjectFilePath.LeftChop(1);
	}

	FPaths::NormalizeFilename(DefaultProjectFilePath);
	FPaths::MakePlatformFilename(DefaultProjectFilePath);
	const FString GenericProjectName = LOCTEXT("DefaultProjectName", "MyProject").ToString();
	FString ProjectName = GenericProjectName;

	// Check to make sure the project file doesn't already exist
	FText FailReason;
	if ( !GameProjectUtils::IsValidProjectFileForCreation(DefaultProjectFilePath / ProjectName / ProjectName + TEXT(".") + IProjectManager::GetProjectFileExtension(), FailReason) )
	{
		// If it exists, find an appropriate numerical suffix
		const int MaxSuffix = 1000;
		int32 Suffix;
		for ( Suffix = 2; Suffix < MaxSuffix; ++Suffix )
		{
			ProjectName = GenericProjectName + FString::Printf(TEXT("%d"), Suffix);
			if ( GameProjectUtils::IsValidProjectFileForCreation(DefaultProjectFilePath / ProjectName / ProjectName + TEXT(".") + IProjectManager::GetProjectFileExtension(), FailReason) )
			{
				// Found a name that is not taken. Break out.
				break;
			}
		}

		if (Suffix >= MaxSuffix)
		{
			UE_LOG(LogGameProjectGeneration, Warning, TEXT("Failed to find a suffix for the default project name"));
			ProjectName = TEXT("");
		}
	}

	if ( !DefaultProjectFilePath.IsEmpty() )
	{
		CurrentProjectFileName = ProjectName;
		CurrentProjectFilePath = DefaultProjectFilePath;
		FPaths::MakePlatformFilename(CurrentProjectFilePath);
		LastBrowsePath = CurrentProjectFilePath;
	}
}


void SNewProjectWizard::UpdateProjectFileValidity( )
{
	// Global validity
	{
		bLastGlobalValidityCheckSuccessful = true;

		TSharedPtr<FTemplateItem> SelectedTemplate = GetSelectedTemplateItem();
		if ( !SelectedTemplate.IsValid() )
		{
			bLastGlobalValidityCheckSuccessful = false;
			LastGlobalValidityErrorText = LOCTEXT("NoTemplateSelected", "No Template Selected");
		}
		else
		{
			if (IsCompilerRequired())
			{
				if ( !FSourceCodeNavigation::IsCompilerAvailable() )
				{
					bLastGlobalValidityCheckSuccessful = false;
					LastGlobalValidityErrorText = FText::Format( LOCTEXT("NoCompilerFound", "No compiler was found. In order to use a C++ template, you must first install {0}."), FSourceCodeNavigation::GetSuggestedSourceCodeIDE() );
				}
				else if ( !FModuleManager::Get().IsUnrealBuildToolAvailable() )
				{
					bLastGlobalValidityCheckSuccessful = false;
					LastGlobalValidityErrorText = LOCTEXT("UBTNotFound", "Engine source code was not found. In order to use a C++ template, you must have engine source code in Engine/Source.");
				}
			}
		}
	}

	// Name and Location Validity
	{
		bLastNameAndLocationValidityCheckSuccessful = true;

		if ( !FPlatformMisc::IsValidAbsolutePathFormat(CurrentProjectFilePath) )
		{
			bLastNameAndLocationValidityCheckSuccessful = false;
			LastNameAndLocationValidityErrorText = LOCTEXT( "InvalidFolderPath", "The folder path is invalid" );
		}
		else
		{
			FText FailReason;
			if ( !GameProjectUtils::IsValidProjectFileForCreation(GetProjectFilenameWithPath(), FailReason) )
			{
				bLastNameAndLocationValidityCheckSuccessful = false;
				LastNameAndLocationValidityErrorText = FailReason;
			}
		}

		if ( CurrentProjectFileName.Contains(TEXT("/")) || CurrentProjectFileName.Contains(TEXT("\\")) )
		{
			bLastNameAndLocationValidityCheckSuccessful = false;
			LastNameAndLocationValidityErrorText = LOCTEXT("SlashOrBackslashInProjectName", "The project name may not contain a slash or backslash");
		}
		else
		{
			FText FailReason;
			if ( !GameProjectUtils::IsValidProjectFileForCreation(GetProjectFilenameWithPath(), FailReason) )
			{
				bLastNameAndLocationValidityCheckSuccessful = false;
				LastNameAndLocationValidityErrorText = FailReason;
			}
		}
	}

	LastValidityCheckTime = FSlateApplication::Get().GetCurrentTime();

	// Since this function was invoked, periodic validity checks should be re-enabled if they were disabled.
	bPreventPeriodicValidityChecksUntilNextChange = false;
}


bool SNewProjectWizard::IsCompilerRequired( ) const
{
	TSharedPtr<FTemplateItem> SelectedTemplate = GetSelectedTemplateItem();
	return SelectedTemplate.IsValid() && SelectedTemplate->bGenerateCode;
}


bool SNewProjectWizard::CreateProject( const FString& ProjectFile )
{
	// Get the selected template
	TSharedPtr<FTemplateItem> SelectedTemplate = GetSelectedTemplateItem();

	if (!ensure(SelectedTemplate.IsValid()))
	{
		// A template must be selected.
		return false;
	}

	const bool bAddCode = SelectedTemplate->bGenerateCode;
	FText FailReason;

	if (!GameProjectUtils::CreateProject( ProjectFile, SelectedTemplate->ProjectFile, bAddCode, bCopyStarterContent, FailReason))
	{
		DisplayError(FailReason);
		return false;
	}

	// Successfully created the project. Update the last created location string.
	FString CreatedProjectPath = FPaths::GetPath(FPaths::GetPath(ProjectFile)); 

	// if the original path was the drives root (ie: C:/) the double path call strips the last /
	if (CreatedProjectPath.EndsWith(":"))
	{
		CreatedProjectPath.AppendChar('/');
	}

	GEditor->AccessGameAgnosticSettings().CreatedProjectPaths.Remove(CreatedProjectPath);
	GEditor->AccessGameAgnosticSettings().CreatedProjectPaths.Insert(CreatedProjectPath, 0);
	GEditor->AccessGameAgnosticSettings().bCopyStarterContentPreference = bCopyStarterContent;
	GEditor->AccessGameAgnosticSettings().PostEditChange();

	return true;
}


void SNewProjectWizard::CreateAndOpenProject( )
{
	if( IsCreateProjectEnabled() )
	{
		FString ProjectFile = GetProjectFilenameWithPath();
		if ( CreateProject(ProjectFile) )
		{
			// Prevent periodic validity checks. This is to prevent a brief error message about the project already existing while you are exiting.
			bPreventPeriodicValidityChecksUntilNextChange = true;

			const bool bCodeAdded = GetSelectedTemplateItem()->bGenerateCode;
			if ( bCodeAdded )
			{
				// In non-rocket, the engine executable may need to be built in order to build the game binaries,
				// just open the code editing ide now instead of automatically building for them since it is not safe to do so.
				const bool bPromptForConfirmation = FApp::HasGameName(); /** Only prompt for project switching if we are already in a project */
				OpenCodeIDE( ProjectFile, bPromptForConfirmation );
			}
			else
			{
				// Successfully created a content only project. Now open it.
				const bool bPromptForConfirmation = false;
				OpenProject( ProjectFile, bPromptForConfirmation );
			}
		}
	}
}


bool SNewProjectWizard::OpenProject( const FString& ProjectFile, bool bPromptForConfirmation )
{
	if ( bPromptForConfirmation )
	{
		// Notify the user of the success, and ask to switch projects.
		FText SuccessMessage = FText::Format( LOCTEXT("NewProjectSuccessful", "Project '{0}' was successfully created. Would you like to open it now?"), FText::FromString(FPaths::GetBaseFilename(ProjectFile)) );
		if ( FMessageDialog::Open( EAppMsgType::YesNo, SuccessMessage ) == EAppReturnType::No )
		{
			// The user opted out of opening the new project. Just close the window.
			CloseWindowIfAppropriate();
			return false;
		}
	}

	FText FailReason;
	if ( GameProjectUtils::OpenProject( ProjectFile, FailReason ) )
	{
		// Successfully opened the project, the editor is closing.
		// Close this window in case something prevents the editor from closing (save dialog, quit confirmation, etc)
		CloseWindowIfAppropriate();
		return true;
	}

	DisplayError( FailReason );
	return false;
}


bool SNewProjectWizard::OpenCodeIDE( const FString& ProjectFile, bool bPromptForConfirmation )
{
	if ( bPromptForConfirmation )
	{
		// Notify the user of the success, and ask to switch projects.
		FText SuccessMessage = FText::Format( LOCTEXT("NewProjectSuccessfulIDE", "Project '{0}' was successfully created. Would you like to open it in {1}?"), FText::FromString(FPaths::GetBaseFilename(ProjectFile)), FSourceCodeNavigation::GetSuggestedSourceCodeIDE() );
		if ( FMessageDialog::Open( EAppMsgType::YesNo, SuccessMessage ) == EAppReturnType::No )
		{
			// The user opted out of opening the new project. Just close the window.
			CloseWindowIfAppropriate(true);
			return false;
		}
	}

	FText FailReason;

	if ( GameProjectUtils::OpenCodeIDE( ProjectFile, FailReason ) )
	{
		// Successfully opened code editing IDE, the editor is closing
		// Close this window in case something prevents the editor from closing (save dialog, quit confirmation, etc)
		CloseWindowIfAppropriate(true);
		return true;
	}

	DisplayError( FailReason );
	return false;
}


void SNewProjectWizard::CloseWindowIfAppropriate( bool ForceClose )
{
	if ( ForceClose || FApp::HasGameName() )
	{
		FWidgetPath WidgetPath;
		TSharedPtr<SWindow> ContainingWindow = FSlateApplication::Get().FindWidgetWindow( AsShared(), WidgetPath);

		if ( ContainingWindow.IsValid() )
		{
			ContainingWindow->RequestDestroyWindow();
		}
	}
}


void SNewProjectWizard::DisplayError( const FText& ErrorText )
{
	FString ErrorString = ErrorText.ToString();
	UE_LOG(LogGameProjectGeneration, Log, TEXT("%s"), *ErrorString);
	if(ErrorString.Contains("\n"))
	{
		FMessageDialog::Open(EAppMsgType::Ok, ErrorText);
	}
	else
	{
		PersistentGlobalErrorLabelText = ErrorText;
	}
}


/* SNewProjectWizard event handlers
 *****************************************************************************/

bool SNewProjectWizard::HandleCreateProjectWizardCanFinish( ) const
{
	return IsCreateProjectEnabled();
}


void SNewProjectWizard::HandleCreateProjectWizardFinished( )
{
	CreateAndOpenProject();
}

ESlateCheckBoxState::Type SNewProjectWizard::GetCategoryTabCheckState(FName Category) const
{
	return Category == ActiveCategory ? ESlateCheckBoxState::Checked : ESlateCheckBoxState::Unchecked;
}

void SNewProjectWizard::HandleCategoryChanged(ESlateCheckBoxState::Type CheckState, FName Category)
{
	if (CheckState != ESlateCheckBoxState::Checked)
	{
		return;
	}

	ActiveCategory = Category;
	FilteredTemplateList = Templates.FindRef(Category);

	// Sort the template folders
	FilteredTemplateList.Sort([](const TSharedPtr<FTemplateItem>& A, const TSharedPtr<FTemplateItem>& B){
		return A->SortKey < B->SortKey;
	});

	if (FilteredTemplateList.Num() > 0)
	{
		TemplateListView->SetSelection(FilteredTemplateList[0]);
	}
	TemplateListView->RequestListRefresh();
}


#undef LOCTEXT_NAMESPACE
