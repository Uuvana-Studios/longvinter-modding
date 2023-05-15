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
#include "FSR2TemporalUpscalingVulkan.h"

#include "CoreMinimal.h"
#include "Interfaces/IPluginManager.h"

#include "VulkanRHI/Private/VulkanRHIPrivate.h"
#include "VulkanRHI/Private/VulkanContext.h"
#include "VulkanRHI/Private/VulkanResources.h"

IMPLEMENT_MODULE(FFSR2TemporalUpscalingVulkanModule, FSR2TemporalUpscalingVulkan)

#define LOCTEXT_NAMESPACE "FSR2"

void FFSR2TemporalUpscalingVulkanModule::StartupModule()
{
}

void FFSR2TemporalUpscalingVulkanModule::ShutdownModule()
{
}

void* FFSR2TemporalUpscalingVulkan::GetNativeCommandBuffer(FRHICommandListImmediate& RHICmdList)
{
	FVulkanCommandListContext& Context = ((FVulkanCommandListContext&)(RHICmdList.GetContext().GetLowestLevelContext()));
	void* CmdList = Context.GetCommandBufferManager()->GetActiveCmdBufferDirect()->GetHandle();
	return CmdList;
}

void* FFSR2TemporalUpscalingVulkan::GetNativeTextureResource(FRHITexture* Texture)
{
	return (void*)((FVulkanTexture2D*)(Texture->GetTexture2D()))->DefaultView.View;
}

uint32 FFSR2TemporalUpscalingVulkan::GetNativeTextureFormat(FRHITexture* Texture)
{
	return (uint32)((FVulkanTexture2D*)(Texture->GetTexture2D()))->Surface.ViewFormat;
}

FSR2ShaderModel FFSR2TemporalUpscalingVulkan::GetSupportedShaderModel()
{
	FSR2ShaderModel ShaderModel = FSR2_SHADER_MODEL_5_1;
	return ShaderModel;
}

bool FFSR2TemporalUpscalingVulkan::IsFloat16Supported()
{
	bool bIsFP16Supported = false;

	FVulkanDynamicRHI* VulkanRHI = (FVulkanDynamicRHI*)GDynamicRHI;
	TArray<VkExtensionProperties> Extensions;
	{
		uint32 Count = 0;
		VulkanRHI::vkEnumerateDeviceExtensionProperties(VulkanRHI->GetDevice()->GetPhysicalHandle(), nullptr, &Count, nullptr);
		if (Count > 0)
		{
			Extensions.Empty(Count);
			Extensions.AddUninitialized(Count);
			VulkanRHI::vkEnumerateDeviceExtensionProperties(VulkanRHI->GetDevice()->GetPhysicalHandle(), nullptr, &Count, Extensions.GetData());
		}
		for (VkExtensionProperties& Ext : Extensions)
		{
			if (strcmp(Ext.extensionName, VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME) == 0)
			{
				// check for fp16 support
				VkPhysicalDeviceShaderFloat16Int8Features shaderFloat18Int8Features = {};
				shaderFloat18Int8Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES;

				VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
				physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
				physicalDeviceFeatures2.pNext = &shaderFloat18Int8Features;

				VulkanRHI::vkGetPhysicalDeviceFeatures2KHR(VulkanRHI->GetDevice()->GetPhysicalHandle(), &physicalDeviceFeatures2);

				bIsFP16Supported = (bool)shaderFloat18Int8Features.shaderFloat16;
			}
		}
	}
	return bIsFP16Supported;
}

void FFSR2TemporalUpscalingVulkan::ForceUAVTransition(FRHICommandListImmediate& RHICmdList, FRHITexture* OutputTexture, FRHIStructuredBuffer* DummyBuffer)
{
	// This code path is intended for transferring resources between the graphics queue and the async compute queue.  Pass in NoFence to avoid the synchronization costs of that behavior.
	// Source transition has to be unknown as we can't tell from here what state it was previously in - we should not assume anything in particular.
	FRHITransitionInfo Info(OutputTexture, ERHIAccess::Unknown, ERHIAccess::UAVMask);
	const FRHITransition* Transition = RHICreateTransition(ERHIPipeline::Graphics, ERHIPipeline::AsyncCompute, ERHICreateTransitionFlags::NoSplit | ERHICreateTransitionFlags::NoFence, MakeArrayView(&Info, 1));

	// only fire the BeginTransition, because we are just trying to exploit the fact that it will actually put RTVs in the UAV state without actually doing any cross-queue sync.
	RHICmdList.BeginTransition(Transition);

	// Release the transition once issued
	RHIReleaseTransition(const_cast<FRHITransition*>(Transition));

	RHICmdList.LockStructuredBuffer(DummyBuffer, 0, sizeof(float), EResourceLockMode::RLM_WriteOnly);
	RHICmdList.UnlockStructuredBuffer(DummyBuffer);
}

#undef LOCTEXT_NAMESPACE