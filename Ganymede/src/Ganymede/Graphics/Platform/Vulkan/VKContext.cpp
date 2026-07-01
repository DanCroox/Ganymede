#include "VKContext.h"

#include "Ganymede/Core/Core.h"
#include "Ganymede/Platform/Window.h"
#include "Ganymede/Core/Application.h"
#include "Ganymede/Graphics/Material.h"
#include "Ganymede/Graphics/ShaderBinary.h"

#include "VKCommandList.h"
#include "VKDescriptorSetPoolManager.h"
#include "VKSSBO.h"
#include "VKVertexObject.h"
#include "VKGPUTexture.h"
#include "VKFrameBuffer.h"
#include "VKRenderTarget.h"

namespace Ganymede
{
	struct FrameInFlightData
	{
		VkFence m_FrameDataAvailableFence = VK_NULL_HANDLE;
		VkSemaphore m_SCImageAvailableSemaphore = VK_NULL_HANDLE;
		VkSemaphore m_QueueSubmissionFinishedSemaphore = VK_NULL_HANDLE;
	};

	std::vector<FrameInFlightData> m_FiFData;

	VKContext& VKContext::GetInstance()
	{
		static VKContext context;
		return context;
	}

	void VKContext::Initialize(const char* appName, GLFWwindow& glfwWindow)
	{
		GM_CORE_ASSERT(!m_IsInitialized, "VKContext is already initialized!");

		// ---------------------------
		// 1) Vulkan-Instanz erstellen
		// ---------------------------
		// Eine VkInstance stellt die Verbindung zwischen der Anwendung und der Vulkan-Library her.
		// Hier werden auch Informationen ueber die Anwendung (optional) und die benoetigten
		// Extensions (z.B. fuer GLFW / WSI) angegeben.
		// GLFW benoetigt bestimmte Instance-Extensions, damit wir ein Fenster-Surface erstellen koennen.
		// Hier holen wir die Extension-Namen von GLFW und uebergeben sie beim Instanz-Create.
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = appName;
		appInfo.apiVersion = GetApiVersion();
		uint32_t glfwExtCount = 0;
		const char** glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);
		std::vector<const char*> extensions(glfwExt, glfwExt + glfwExtCount);
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		// Instanz erstellen. In realen Apps sollte man Fehlerpruefung und ggf. Validation Layers setzen.
		GM_CORE_ASSERT(volkInitialize() == VkResult::VK_SUCCESS, "volkInitialize() failed.");
		vkCreateInstance(&createInfo, nullptr, &instance);

		// Init volk Vulkan wrapper
		volkLoadInstance(instance); // volk: dynamische Loader-Funktionalitaet
		m_DescriptorSetPoolManager = new VKDescriptorSetPoolManager();

		// ---------------------------
		// 2) Fenster-Surface erzeugen
		// ---------------------------
		// Das Surface verknuepft die Plattform-Fenster-API (GLFW) mit Vulkan. Es ist erforderlich,
		// um spaeter die Swapchain darauf zu erstellen (Presentation).
		if (glfwCreateWindowSurface(instance, &glfwWindow, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}

		// ---------------------------
		// 3) Physisches + Logisches Device
		// ---------------------------
		// Hier wird eine Hilfsklasse 'vkutil::VulkanDevice' verwendet, die:
		// - ein geeignetes physisches Geraet (GPU) waehlt,
		// - ein logisches Device erstellt,
		// - Queues (Graphics, Present) bereitstellt,
		// - einen Command Pool anlegt.
		// Wir geben die benoetigten Device-Extensions an (z.B. VK_KHR_swapchain).
		std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		device.Initialize(instance, surface, deviceExtensions, /*enableValidationLayers=*/false);
		// Die Hilfsklasse stellt Methoden bereit wie device.device(), device.graphicsQueue(), usw.

		// ---------------------------
		// 4) Swapchain erstellen
		// ---------------------------
		// Die Swapchain ist eine Sammlung von Images, die fuer Presentation verwendet werden.
		// Ablauf:
		//  - Query Surface-Capabilities (Groesse, Anzahl Images, Transform, usw.)
		//  - Waehle Format + Extent
		//  - Erstelle Swapchain mit geeigneten Parametern
		//
		// Hier ist die Version vereinfacht: kein dynamisches Format-Picking oder Resize-Handling.
		VkSurfaceCapabilitiesKHR caps;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physicalDeviceHandle(), surface, &caps);

		// Formate abfragen
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDeviceHandle(), surface, &formatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDeviceHandle(), surface, &formatCount, formats.data());

		// Einfach das erste Format nehmen (in realer App ggf. das beste Format waehlen)
		surfaceFormat = formats[0];
		// Extent (Aufloesung) aus den Surface-Capabilities (meistens durch das Window bestimmt)
		extent = caps.currentExtent;

		// We prefer double buffering if possible for less input lag
		uint32_t imageCount = glm::clamp(2u, caps.minImageCount, caps.maxImageCount);

		VkSwapchainCreateInfoKHR swapInfo{};
		swapInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapInfo.surface = surface;
		swapInfo.minImageCount = imageCount;
		swapInfo.imageFormat = surfaceFormat.format;
		swapInfo.imageColorSpace = surfaceFormat.colorSpace;
		swapInfo.imageExtent = extent;
		swapInfo.imageArrayLayers = 1;
		swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		// Exklusive Sharing-Mode: Queue-Familien sind in der Regel separat, hier wird angenommen
		// dass Graphics und Present dieselbe Queue-Familie haben. Sonst waere VK_SHARING_MODE_CONCURRENT noetig.
		swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapInfo.preTransform = caps.currentTransform;
		swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		// Present-Mode: FIFO entspricht VSync (garantiert vorhanden)
		//swapInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		swapInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		swapInfo.clipped = VK_TRUE;

		if (vkCreateSwapchainKHR(device.device(), &swapInfo, nullptr, &swapchain) != VK_SUCCESS)
			throw std::runtime_error("Failed to create swapchain!");

		// --- Swapchain Images auslesen ---
		// Die Swapchain besitzt mehrere VkImage-Handles; diese werden hier abgefragt.
		vkGetSwapchainImagesKHR(device.device(), swapchain, &m_SCImgCount, nullptr);
		swapImages.resize(m_SCImgCount);
		vkGetSwapchainImagesKHR(device.device(), swapchain, &m_SCImgCount, swapImages.data());

		std::vector<VkImageView> scImageViews;
		scImageViews.resize(m_SCImgCount);
		for (uint32_t i = 0; i < m_SCImgCount; ++i)
		{
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = swapImages[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = surfaceFormat.format;
			// Komponenten-Swizzle: IDENTITY bedeutet keine Umordnung der Kanale
			viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
			// Subresource-Range: welche Aspekte (Color/Depth) und welche Miplevels/Layers
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device.device(), &viewInfo, nullptr, &scImageViews[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create image view!");

		}

		//TODO Add proper channel and datattype setup
		VKBackBuffer<VkImage> bbSwapImages;
		VKBackBuffer<VkImageView> bbSwapImageViews;
		std::copy_n(swapImages.begin(), std::min(bbSwapImages.size(), swapImages.size()), bbSwapImages.begin());
		std::copy_n(scImageViews.begin(), std::min(bbSwapImageViews.size(), scImageViews.size()), bbSwapImageViews.begin());

		m_SCImageRenderTargets = std::make_unique<VKRenderTargetImageWrapper>(
			bbSwapImages,
			bbSwapImageViews,
			surfaceFormat.format,
			VK_IMAGE_ASPECT_COLOR_BIT,
			RenderTargetTypes::ComponentType::RGBA,
			RenderTargetTypes::ChannelDataType::UInt,
			RenderTargetTypes::ChannelPrecision::B8,
			glm::uvec2(extent.width, extent.height));

		// ---------------------------
		// 10) Synchronisation-Objekte
		// ---------------------------
		// Semaphoren und Fences steuern die Synchronisation zwischen CPU und GPU sowie
		// zwischen verschiedenen GPU-Operationen. Das typische Pattern:
		//
		// - imageAvailableSemaphore: wird von vkAcquireNextImageKHR signaled, sobald ein Swapchain-Image bereit ist
		// - renderFinishedSemaphore: wird vom Submit signaled, wenn Rendering fertig ist
		// - inFlightFence: CPU wartet auf das Fertigwerden eines Submits (um Ressourcen wiederzuverwenden)
		//
		// Hier eine simple Variante mit einem Fence fuer ein einzelnes in-flight Frame.
		m_FiFData.resize(m_SCImgCount);
		m_NumFIF = m_SCImgCount;

		VkSemaphoreCreateInfo semInfo{};
		semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		for (FrameInFlightData& fifData : m_FiFData)
		{
			vkCreateSemaphore(device.device(), &semInfo, nullptr, &fifData.m_SCImageAvailableSemaphore);
			vkCreateSemaphore(device.device(), &semInfo, nullptr, &fifData.m_QueueSubmissionFinishedSemaphore);

			// Initial signaled: damit der erste vkWaitForFences nicht blockiert
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			vkCreateFence(device.device(), &fenceInfo, nullptr, &fifData.m_FrameDataAvailableFence);
		}

		// ---------------------------
		// 11) Command Buffers allokieren und aufzeichnen
		// ---------------------------
		// Command Buffers enthalten die eigentlichen GPU-Befehle. Hier wird fuer jedes
		// Swapchain-Image ein Command Buffer erstellt und einmal aufgezeichnet.
		// Wenn Framebuffers/Renderpass statisch sind, kann man die Aufzeichnung einmal machen;
		// bei dynamischen Inhalten wuerde man pro Frame aufzeichnen.
		VKBackBuffer<VkCommandBuffer> commandBuffers;
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = device.commandPoolHandle(); // Command Pool von der Device-Hilfsklasse
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(device.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate command buffers!");

		m_CommandList = std::make_unique<VKCommandList>(commandBuffers);
		
		//BINDLESS TEXTURES TEST
		std::vector<VkDescriptorSetLayoutBinding> bindlessDSLayoutBindings;
		uint32_t maxTextures = 4096;
		VkDescriptorSetLayoutBinding& bindingTextures = bindlessDSLayoutBindings.emplace_back();
		bindingTextures.binding = 0;
		bindingTextures.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindingTextures.descriptorCount = maxTextures;
		bindingTextures.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindingTextures.pImmutableSamplers = nullptr;

		uint32_t maxSSBOs = 4096;
		VkDescriptorSetLayoutBinding& bindingSSBOs = bindlessDSLayoutBindings.emplace_back();
		bindingSSBOs.binding = 1;
		bindingSSBOs.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bindingSSBOs.descriptorCount = maxSSBOs;
		bindingSSBOs.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
		bindingSSBOs.pImmutableSamplers = nullptr;

		// Texture binding flags
		std::vector<VkDescriptorBindingFlags> bindlessBindingFlags;
		VkDescriptorBindingFlags& textureBindingFlags = bindlessBindingFlags.emplace_back();
		textureBindingFlags =
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |				// allows "gaps"
			VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |	// runtime-sized array
			VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;			// allows late fill

		VkDescriptorBindingFlags& ssboBindingFlags = bindlessBindingFlags.emplace_back();
		ssboBindingFlags =
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |				// allows "gaps"
			VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |	// runtime-sized array
			VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;			// allows late fill

		GM_CORE_ASSERT(bindlessBindingFlags.size() != bindlessDSLayoutBindings.size(), "Need same amount of binding flags as layout bindings. One flag config for each layout binding.");

		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
		bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		bindingFlagsInfo.bindingCount = bindlessBindingFlags.size();
		bindingFlagsInfo.pBindingFlags = bindlessBindingFlags.data();

		VkDescriptorSetLayoutCreateInfo bindlessDSLayoutCreateInfo{};
		bindlessDSLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		bindlessDSLayoutCreateInfo.bindingCount = bindlessDSLayoutBindings.size();
		bindlessDSLayoutCreateInfo.pBindings = bindlessDSLayoutBindings.data();
		bindlessDSLayoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		bindlessDSLayoutCreateInfo.pNext = &bindingFlagsInfo;

		// Create DSLayout
		vkCreateDescriptorSetLayout(device.device(), &bindlessDSLayoutCreateInfo, nullptr, &m_BindlessDescriptorSetLayout);
		m_BindlessDescriptorSet = m_DescriptorSetPoolManager->CreateDescriptorSet(&m_BindlessDescriptorSetLayout, 1);
		// END BINDLESS TEXTURE

		m_IsInitialized = true;
	}

	void VKContext::Shutdown()
	{
 		vkDeviceWaitIdle(device.device());

		delete m_DescriptorSetPoolManager;
		
		// destroy semaphores / fence
		for (FrameInFlightData& fifData : m_FiFData)
		{
			vkDestroySemaphore(device.device(), fifData.m_SCImageAvailableSemaphore, nullptr);
			vkDestroySemaphore(device.device(), fifData.m_QueueSubmissionFinishedSemaphore, nullptr);
			vkDestroyFence(device.device(), fifData.m_FrameDataAvailableFence, nullptr);
		}

		device.Shutdown();
	}

	void VKContext::BeginFrame()
	{
		// Warte, falls das vorige Frame noch nicht fertig war.
		// Dieses Muster sorgt dafuer, dass wir nicht Command-Buffers / Framebuffers wiederverwenden,
		// waehrend die GPU noch daran arbeitet.
		vkWaitForFences(device.device(), 1, &m_FiFData[m_FiFIndex].m_FrameDataAvailableFence, VK_TRUE, UINT64_MAX);
		// Fence zuruecksetzen, damit wir ihn fuer den naechsten Submit verwenden koennen.
		vkResetFences(device.device(), 1, &m_FiFData[m_FiFIndex].m_FrameDataAvailableFence);

		// Acquire next image: signalisiert imageAvailableSemaphore sobald das Swapchain-Image bereit ist.
		VkResult acquireRes = vkAcquireNextImageKHR(
			device.device(),
			swapchain,
			UINT64_MAX,
			m_FiFData[m_FiFIndex].m_SCImageAvailableSemaphore, // signal when image is available
			VK_NULL_HANDLE,
			&m_SCIndex);

		if (acquireRes == VK_ERROR_OUT_OF_DATE_KHR) {
			// Swapchain muss rekreiert werden (z.B. Fenster wurde geaendert) - hier breaken wir raus.
			// In echten Anwendungen wuerde man eine Swapchain-Recreation-Routine aufrufen.
			throw std::runtime_error("failed to acquire swap chain image!");
		}
		else if (acquireRes != VK_SUCCESS && acquireRes != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}
	}

	void VKContext::EndFrame()
	{
		// Submit command buffer for the acquired image
		// Wir warten auf imageAvailableSemaphore bevor wir Teile der Pipeline starten,
		// und signalisieren renderFinishedSemaphore wenn Rendering fertig ist.
		VkSemaphore waitSemaphores[] = { m_FiFData[m_FiFIndex].m_SCImageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore signalSemaphores[] = { m_FiFData[m_FiFIndex].m_QueueSubmissionFinishedSemaphore };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandList->GetVkCommandBuffer()[m_FiFIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		// Submit zu Graphics-Queue; inFlightFence wird signaled wenn GPU fertig ist.
		if (vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, m_FiFData[m_FiFIndex].m_FrameDataAvailableFence) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		// Present the image (wartet auf renderFinishedSemaphore)
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.pImageIndices = &m_SCIndex;

		VkResult presentRes = vkQueuePresentKHR(device.presentQueue(), &presentInfo);
		if (presentRes == VK_ERROR_OUT_OF_DATE_KHR || presentRes == VK_SUBOPTIMAL_KHR) {
			// Swapchain muss rekreiert werden (z. B. Fenstergroesse geaendert)
		}
		else if (presentRes != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		m_FiFIndex = (m_FiFIndex + 1) % m_FiFData.size();
	}
	
	std::optional<VkImageLayout> ToNativeAttachmentLayout(FrameBufferAttachmentTypee attachmentType)
	{
		switch (attachmentType)
		{
		case FrameBufferAttachmentTypee::Color: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		case FrameBufferAttachmentTypee::Depth: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		default: break;
		}
		
		GM_CORE_ASSERT(false, "Unsupported framebuffer attachment.");
		return std::nullopt;
	}

	uint32_t VKContext::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(device.physicalDeviceHandle(), &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}
		throw std::runtime_error("failed to find suitable memory type!");
	}

	void VKContext::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device.device(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
			throw std::runtime_error("failed to create buffer!");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device.device(), buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device.device(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate buffer memory!");

		vkBindBufferMemory(device.device(), buffer, bufferMemory, 0);
	} 

	void VKContext::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = device.commandPoolHandle();
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer cmd;
		vkAllocateCommandBuffers(device.device(), &allocInfo, &cmd);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(cmd, &beginInfo);

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(cmd, srcBuffer, dstBuffer, 1, &copyRegion);

		vkEndCommandBuffer(cmd);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;

		vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(device.graphicsQueue());

		vkFreeCommandBuffers(device.device(), device.commandPoolHandle(), 1, &cmd);
	}

	std::pair<VkDescriptorSetLayout, VkDescriptorSet> VKContext::CreateDescriptorSetDefinition(std::vector<VkDescriptorSetLayoutBinding> dsLayoutBindings)
	{
		VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
		layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutCreateInfo.bindingCount = dsLayoutBindings.size();
		layoutCreateInfo.pBindings = dsLayoutBindings.data();

		VkDescriptorSetLayout descriptorSetLayout;
		vkCreateDescriptorSetLayout(device.device(), &layoutCreateInfo, nullptr, &descriptorSetLayout);

		VkDescriptorSet descriptorSet = m_DescriptorSetPoolManager->CreateDescriptorSet(&descriptorSetLayout, 1);
		return { descriptorSetLayout, descriptorSet };
	}

	void VKContext::BindToDescriptorSet(VkDescriptorSet descriptorSet, const std::vector<SSBOBindingInfo>& ssboBindings)
	{
		std::vector<VkWriteDescriptorSet> writeDescriptorSets;
		std::vector<VkDescriptorBufferInfo> bufferInfos;
		writeDescriptorSets.reserve(ssboBindings.size());
		bufferInfos.reserve(ssboBindings.size());
		for (unsigned int i = 0; i < ssboBindings.size(); ++i)
		{
			VkDescriptorBufferInfo& bufferInfo = bufferInfos.emplace_back();
			bufferInfo.buffer = ssboBindings[i].m_SSBO.uniformBuffer;
			bufferInfo.offset = ssboBindings[i].m_ByteOffset;
			bufferInfo.range = ssboBindings[i].m_Range;
		}

		for (unsigned int i = 0; i < ssboBindings.size(); ++i)
		{
			VkWriteDescriptorSet& write = writeDescriptorSets.emplace_back();
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = descriptorSet;
			write.dstBinding = ssboBindings[i].m_BindingPoint;
			write.dstArrayElement = 0; // Array offset
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			write.pImageInfo = nullptr;
			write.pBufferInfo = &bufferInfos[i];
			write.pTexelBufferView = nullptr;
		}

		vkUpdateDescriptorSets(device.device(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
	}

	void VKContext::BindToDescriptorSet(VkDescriptorSet descriptorSet, const std::vector<TextureBindingInfo>& textureBindings)
	{
		std::vector<VkWriteDescriptorSet> writeDescriptorSets;
		std::vector<VkDescriptorImageInfo> imageInfos;
		writeDescriptorSets.reserve(textureBindings.size());
		imageInfos.reserve(textureBindings.size());
		for (unsigned int i = 0; i < textureBindings.size(); ++i)
		{
			VkDescriptorImageInfo& descImgInfo = imageInfos.emplace_back();
			descImgInfo.sampler = textureBindings[i].m_VkSampler;
			descImgInfo.imageView = textureBindings[i].m_VkImageView;
			descImgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		for (unsigned int i = 0; i < textureBindings.size(); ++i)
		{
			VkWriteDescriptorSet& write = writeDescriptorSets.emplace_back();
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = descriptorSet;
			write.dstBinding = textureBindings[i].m_BindingPoint;
			write.dstArrayElement = textureBindings[i].m_Offset; // Array offset
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.pImageInfo = &imageInfos[i];
			write.pBufferInfo = nullptr;
			write.pTexelBufferView = nullptr;
		}

		vkUpdateDescriptorSets(device.device(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
	}

	void VKContext::RegisterBindlessVKGPUTexture(VKGPUTexture& vkGPUTexture)
	{
		vkGPUTexture.m_BindlessDescriptorSetIndex = m_BindlessTextureFreeList.Append();

		VkDescriptorImageInfo descImgInfo{};
		descImgInfo.sampler = vkGPUTexture.textureSampler;
		descImgInfo.imageView = vkGPUTexture.vkImageView;
		descImgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = m_BindlessDescriptorSet;
		write.dstBinding = 0;
		write.dstArrayElement = vkGPUTexture.m_BindlessDescriptorSetIndex;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.pImageInfo = &descImgInfo;
		write.pBufferInfo = nullptr;
		write.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(device.device(), 1, &write, 0, nullptr);
	}

	void VKContext::RegisterBindlessVKRenderTarget(VKRenderTarget& vkRenderTarget)
	{
		for (uint32_t scIndex = 0; scIndex < m_SCImgCount; ++scIndex)
		{
			vkRenderTarget.m_BindlessDescriptorSetIndex[scIndex] = m_BindlessTextureFreeList.Append();

			VkDescriptorImageInfo descImgInfo{};
			descImgInfo.sampler = vkRenderTarget.m_VkSampler[scIndex];
			descImgInfo.imageView = vkRenderTarget.m_VkImageView[scIndex];
			descImgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = m_BindlessDescriptorSet;
			write.dstBinding = 0;
			write.dstArrayElement = vkRenderTarget.m_BindlessDescriptorSetIndex[scIndex];
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.pImageInfo = &descImgInfo;
			write.pBufferInfo = nullptr;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(device.device(), 1, &write, 0, nullptr);
		}
	}

	void VKContext::RegisterBindlessVKSSBO(VKSSBO& ssbo)
	{
		ssbo.m_BindlessDescriptorSetIndex = m_BindlessSSBOFreeList.Append();

		VkDescriptorBufferInfo descBufferInfo;
		descBufferInfo.buffer = ssbo.uniformBuffer;
		descBufferInfo.offset = 0;
		descBufferInfo.range = ssbo.GetSize();

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = m_BindlessDescriptorSet;
		write.dstBinding = 1;
		write.dstArrayElement = ssbo.m_BindlessDescriptorSetIndex; // Array offset
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		write.pImageInfo = nullptr;
		write.pBufferInfo = &descBufferInfo;
		write.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(device.device(), 1, &write, 0, nullptr);
	}

	void VKContext::UnregisterBindlessVKGPUTexture(VKGPUTexture& vkGPUTexture)
	{
		GM_CORE_ASSERT(vkGPUTexture.IsBindlessDSIndexValid(), "Trying to unregistered an SSBO from bindless descritorset, which is not registered.");
		if (vkGPUTexture.IsBindlessDSIndexValid())
		{
			m_BindlessTextureFreeList.Free(vkGPUTexture.GetBindlessDSIndex());
			vkGPUTexture.ResetBindlessDSIndex();
		}
	}

	void VKContext::UnregisterBindlessVKRenderTarget(VKRenderTarget& vkRenderTarget)
	{
		GM_CORE_ASSERT(vkRenderTarget.IsBindlessDSIndexValid(), "Trying to unregistered an SSBO from bindless descritorset, which is not registered.");
		if (vkRenderTarget.IsBindlessDSIndexValid())
		{
			//TODO: This is wrong! Will free textures that might still be in use on gpu!!!
			//m_BindlessTextureFreeList.Free(vkRenderTarget.GetBindlessDSIndex());
			vkRenderTarget.ResetBindlessDSIndex();
		}
	}

	void VKContext::UnregisterBindlessVKSSBO(VKSSBO& ssbo)
	{
		GM_CORE_ASSERT(ssbo.IsBindlessDSIndexValid(), "Trying to unregistered an SSBO from bindless descritorset, which is not registered.");
		if (ssbo.IsBindlessDSIndexValid())
		{
			m_BindlessSSBOFreeList.Free(ssbo.GetBindlessDSIndex());
			ssbo.ResetBindlessDSIndex();
		}
	}
}