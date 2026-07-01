#pragma once
#include <unordered_map>
#include <volk.h>

namespace Ganymede
{

	class VKDescriptorSetPoolManager
	{
	public:
		VKDescriptorSetPoolManager() = default;
		~VKDescriptorSetPoolManager() = default;

		VkDescriptorSet CreateDescriptorSet(const VkDescriptorSetLayout* vkDescriptorSetLayouts, uint32_t numDescriptorSetLayouts);

	private:
		class VKDescriptorSetPoolContainer
		{
		public:
			VKDescriptorSetPoolContainer() = delete;
			VKDescriptorSetPoolContainer(unsigned int numSetsMax);
			~VKDescriptorSetPoolContainer();

			bool HasAvailableSets() const { return m_NumSetsInPool < m_NumSetsMax; }

			VkDescriptorSet CreateDescriptorSets(const VkDescriptorSetLayout* vkDescriptorSetLayouts, uint32_t numDescriptorSetLayouts);

		private:
			unsigned int m_NumSetsMax;
			unsigned int m_NumSetsInPool;

			VkDescriptorPool m_DescriptorPool;
		};

		std::vector<VKDescriptorSetPoolContainer> m_DescriptorPoolContainers;
	};
}