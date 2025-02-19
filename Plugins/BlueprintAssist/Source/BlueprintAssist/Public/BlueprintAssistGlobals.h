// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"

BLUEPRINTASSIST_API DECLARE_LOG_CATEGORY_EXTERN(LogBlueprintAssist, Log, All);

#define BA_UE_MINOR_VERSION_OR_LATER(major, minor) ENGINE_MAJOR_VERSION == major && ENGINE_MINOR_VERSION >= minor
#define BA_UE_VERSION_OR_LATER(major, minor) ENGINE_MAJOR_VERSION == major && ENGINE_MINOR_VERSION >= minor || ENGINE_MAJOR_VERSION > major

#if BA_UE_VERSION_OR_LATER(5, 0)
#define BA_SUBOBJECT_EDITOR_TREE_NODE FSubobjectEditorTreeNode
#define BA_SUBOBJECT_EDITOR SSubobjectEditor
#define BA_GET_STYLE_SET_NAME FAppStyle::GetAppStyleSetName
#define BA_STYLE_CLASS FAppStyle
#else
#define BA_SUBOBJECT_EDITOR_TREE_NODE FSCSEditorTreeNode
#define BA_SUBOBJECT_EDITOR SSCSEditor
#define BA_GET_STYLE_SET_NAME FEditorStyle::GetStyleSetName
#define BA_STYLE_CLASS FEditorStyle
#endif

#if BA_UE_VERSION_OR_LATER(5, 1)
#define BA_GET_FONT_STYLE FAppStyle::GetFontStyle
#else
#define BA_GET_FONT_STYLE FEditorStyle::GetFontStyle
#endif