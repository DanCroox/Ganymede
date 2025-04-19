#include "Ganymede/Core/Core.h"

#include "RenderPass.h"
#include <memory>
#include <vector>

namespace Ganymede
{
	class RenderContext;

	class GANYMEDE_API RenderPipeline
	{
	public:
		RenderPipeline() = delete;
		RenderPipeline(const RenderPipeline&) = delete;
		RenderPipeline& operator=(const RenderPipeline&) = delete;

		RenderPipeline(RenderContext& renderContext);
		~RenderPipeline();

		bool Initialize();
		void Execute();

		template <typename T>
		void AddRenderPass()
		{
			if (m_IsInitialized)
			{
				GM_CORE_ASSERT(false, "Cannot add new renderpasses once pipeline is initialized.");
				return;
			}

			m_RenderPasses.emplace_back(std::make_unique<T>());
		}

	private:
		bool m_IsInitialized;
		RenderContext& m_RenderContext;
		std::vector<std::unique_ptr<RenderPass2>> m_RenderPasses;
	};
}