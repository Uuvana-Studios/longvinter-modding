// This file is part of the FidelityFX Super Resolution 2.2 Unreal Engine Plugin.
//
// Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
#pragma once

#include "Runtime/Launch/Resources/Version.h"
#define FSR2_ENGINE_SUPPORTS_SCREENPERCENTAGE (((ENGINE_MAJOR_VERSION == 5) && (ENGINE_MINOR_VERSION == 0)) || (ENGINE_MINOR_VERSION >= 26))
#define FSR2_ENGINE_SUPPORTS_SCREENPERCENTAGEDATA (((ENGINE_MAJOR_VERSION == 5) && (ENGINE_MINOR_VERSION == 0)) || (ENGINE_MINOR_VERSION >= 27))

#if FSR2_ENGINE_SUPPORTS_SCREENPERCENTAGE
#include "CustomStaticScreenPercentage.h"
#if WITH_EDITOR
#include "CustomEditorStaticScreenPercentage.h"
#endif
#endif

#if FSR2_ENGINE_SUPPORTS_SCREENPERCENTAGEDATA
//-------------------------------------------------------------------------------------
// Used for the Editor viewport, where the user can select the FSR2 quality if it is enabled.
//-------------------------------------------------------------------------------------
class FFSR2CustomStaticScreenPercentageData final : public ICustomStaticScreenPercentageData
{
public:
	int32 QualityMode = 0;
};
#endif
