#pragma once

#include "Ganymede/Core/Core.h"

namespace Ganymede
{
	class RenderContext;

	class GANYMEDE_API RenderPass2
	{
	public:
		RenderPass2(const RenderPass2&) = delete;
		RenderPass2& operator=(const RenderPass2&) = delete;

		RenderPass2();
		virtual ~RenderPass2() = default;

		virtual bool Initialize(RenderContext& renderContext) = 0;
		virtual void Execute(RenderContext& renderContext) = 0;

		inline bool IsBypassed() const{ return m_IsBypassed; }
		inline void SetIsBypassed(bool isBypassed) { m_IsBypassed = isBypassed; }

	private:
		bool m_IsBypassed;
	};
}