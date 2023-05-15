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

#include "FSR2ScreenPercentageData.h"
#include "PostProcess/PostProcessUpscale.h"
#include "PostProcess/TemporalAA.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFSR2Editor, Log, All);

#if FSR2_ENGINE_SUPPORTS_SCREENPERCENTAGE && WITH_EDITOR
class FFSR2TemporalUpscalerEditor final : public ICustomEditorStaticScreenPercentage, public TSharedFromThis<FFSR2TemporalUpscalerEditor>
#else
class FFSR2TemporalUpscalerEditor final : public TSharedFromThis<FFSR2TemporalUpscalerEditor>
#endif
{
public:
	FFSR2TemporalUpscalerEditor(class IFSR2TemporalUpscalingModule* Module);
	virtual ~FFSR2TemporalUpscalerEditor() {}

#if FSR2_ENGINE_SUPPORTS_SCREENPERCENTAGE && WITH_EDITOR
	void SetupEditorViewFamily(FSceneViewFamily& ViewFamily, FEditorViewportClient* ViewportClient);
	bool GenerateEditorViewportOptionsMenuEntry(const ICustomEditorStaticScreenPercentage::FViewportMenuEntryArguments& Arguments);
#endif

private:
	class IFSR2TemporalUpscalingModule* TemporalUpscalingModule;
};