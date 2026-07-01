#include "FrameBuffer.h"

namespace Ganymede
{
	FrameBufferAttachmentStorage::FrameBufferAttachmentStorage(std::initializer_list<FrameBufferAttachment> attachments)
	{
		for (const FrameBufferAttachment& attachment : attachments)
		{
			if (m_NumEntries >= MAX_BINDING_LOCATIONS)
			{
				GM_CORE_ASSERT(false, "Cannot add attachment. Maximum number of attachments per framebuffer exceeded.");
				return;
			}

			const uint32_t bindingLocation = attachment.m_AttachmentLocation;
			if (bindingLocation >= MAX_BINDING_LOCATIONS)
			{
				GM_CORE_ASSERT(false, "Binding location not supported.");
				return;
			}

			m_BindingLocations[m_NumEntries++] = attachment.m_AttachmentLocation;
			m_Attachments[bindingLocation] = attachment;
		}
	}

	const FrameBufferAttachment* FrameBufferAttachmentStorage::TryGetByBindingLocation(uint32_t locationBindingID) const
	{
		if (locationBindingID >= MAX_BINDING_LOCATIONS ||
			m_Attachments[locationBindingID].m_RenderTarget == nullptr)
		{
			return nullptr;
		}

		return &m_Attachments[locationBindingID];
	}
}