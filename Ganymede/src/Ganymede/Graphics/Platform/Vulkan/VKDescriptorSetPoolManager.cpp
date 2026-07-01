#include "VKDescriptorSetPoolManager.h"
#include "Ganymede/Core/Core.h"
#include "VKContext.h"

namespace Ganymede
{
	VKDescriptorSetPoolManager::VKDescriptorSetPoolContainer::VKDescriptorSetPoolContainer(
		unsigned int numSetsMax) :
		m_NumSetsMax(numSetsMax),
		m_NumSetsInPool(0),
		m_DescriptorPool(VK_NULL_HANDLE)
	{
		VkDescriptorPoolSize poolSizes[] = {
			{ VK_DESCRIPTOR_TYPE_SAMPLER, m_NumSetsMax },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_NumSetsMax },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, m_NumSetsMax },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, m_NumSetsMax },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, m_NumSetsMax },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, m_NumSetsMax },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_NumSetsMax },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, m_NumSetsMax },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, m_NumSetsMax },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, m_NumSetsMax },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, m_NumSetsMax }
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = std::size(poolSizes);
		poolInfo.pPoolSizes = poolSizes;
		poolInfo.maxSets = m_NumSetsMax;
		// Important for bindless
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

		vkCreateDescriptorPool(VKContext::GetInstance().device.device(), &poolInfo, nullptr, &m_DescriptorPool);
	}

	VKDescriptorSetPoolManager::VKDescriptorSetPoolContainer::~VKDescriptorSetPoolContainer()
	{
		vkDestroyDescriptorPool(VKContext::GetInstance().device.device(), m_DescriptorPool, nullptr);
	}

	VkDescriptorSet VKDescriptorSetPoolManager::VKDescriptorSetPoolContainer::CreateDescriptorSets(const VkDescriptorSetLayout* vkDescriptorSetLayouts, uint32_t numDescriptorSetLayouts)
	{
		GM_CORE_ASSERT(m_NumSetsInPool < m_NumSetsMax, "DescriptorSet pool exceeded!");

		VkDescriptorSetAllocateInfo allocDS{};
		allocDS.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocDS.descriptorPool = m_DescriptorPool;
		allocDS.descriptorSetCount = numDescriptorSetLayouts;
		allocDS.pSetLayouts = vkDescriptorSetLayouts;

		VkDescriptorSet descriptorSet;
		vkAllocateDescriptorSets(VKContext::GetInstance().device.device(), &allocDS, &descriptorSet);
		
		++m_NumSetsInPool;

		return descriptorSet;
	}

	VkDescriptorSet VKDescriptorSetPoolManager::CreateDescriptorSet(const VkDescriptorSetLayout* vkDescriptorSetLayouts, uint32_t numDescriptorSetLayouts)
	{
		VKDescriptorSetPoolContainer* poolContainer = nullptr;
		if (m_DescriptorPoolContainers.empty() || m_DescriptorPoolContainers.back().HasAvailableSets() == false)
		{
			poolContainer = &m_DescriptorPoolContainers.emplace_back(1000);
		}
		else
		{
			poolContainer = &m_DescriptorPoolContainers.back();
		}

		return poolContainer->CreateDescriptorSets(vkDescriptorSetLayouts, numDescriptorSetLayouts);
	}
}