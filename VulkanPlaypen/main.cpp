#include "Platform.h"
#include "Renderer.h"


void TestCommandPoolWithFence( Renderer &r )
{
	// Create the VkCommandPool
	VkCommandPool command_pool;
	VkCommandPoolCreateInfo pool_info {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = r.getGraphicsFamilyIndex();
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	vkCreateCommandPool( r.getDevice(), &pool_info, nullptr, &command_pool );

	// Allocate a VkCommandBuffer, assigned to the pool
	VkCommandBuffer command_buffer;
	VkCommandBufferAllocateInfo command_buffer_info {};
	command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_info.commandPool = command_pool;
	command_buffer_info.commandBufferCount = 1;

	vkAllocateCommandBuffers( r.getDevice(), &command_buffer_info, &command_buffer );


	// Begin a command buffer
	VkCommandBufferBeginInfo begin_command_buffer_info {};
	begin_command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vkBeginCommandBuffer( command_buffer, &begin_command_buffer_info );


	// Add a command to the command buffer
	VkViewport viewport;
	viewport.width = 512;
	viewport.height = 512;
	viewport.x = 0;
	viewport.y = 0;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport( command_buffer, 0, 1, &viewport );


	// End a command buffer
	vkEndCommandBuffer( command_buffer );


	// Create VkFence to synchronise the submit queue below
	VkFence fence;
	VkFenceCreateInfo fence_info {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkCreateFence( r.getDevice(), &fence_info, nullptr, &fence );


	// Submit the command buffer to the device queue
	VkSubmitInfo submit_info {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	vkQueueSubmit( r.getQueue(), 1, &submit_info, fence );


	// Wait for the fence given to the submit queue to expire
	vkWaitForFences( r.getDevice(), 1, &fence, VK_TRUE, UINT64_MAX );


	// Wait for device idle - not needed because the fence above does this instead
	// vkQueueWaitIdle( r.getQueue() );

	// Detroy created Vulkan objects
	vkDestroyCommandPool( r.getDevice(), command_pool, nullptr );
	vkDestroyFence( r.getDevice(), fence, nullptr );
}

void TestCommandPoolWithSemaphore( Renderer &r )
{
	// Create a VkSemaphore for use to sync
	VkSemaphore semaphore;
	VkSemaphoreCreateInfo semaphore_info {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore( r.getDevice(), &semaphore_info, nullptr, &semaphore );
	

	// Create the VkCommandPool
	VkCommandPool command_pool;
	VkCommandPoolCreateInfo pool_info {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = r.getGraphicsFamilyIndex();
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	vkCreateCommandPool( r.getDevice(), &pool_info, nullptr, &command_pool );

	// Allocate a VkCommandBuffer, assigned to the pool
	VkCommandBuffer command_buffer[2];
	VkCommandBufferAllocateInfo command_buffer_info {};
	command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_info.commandPool = command_pool;
	command_buffer_info.commandBufferCount = 2;

	vkAllocateCommandBuffers( r.getDevice(), &command_buffer_info, command_buffer );
	
	// Command buffer 1
	{
		// Begin a command buffer
		VkCommandBufferBeginInfo begin_command_buffer_info {};
		begin_command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		vkBeginCommandBuffer( command_buffer[0], &begin_command_buffer_info );

		// Create a pipeline barrier for later
		vkCmdPipelineBarrier( command_buffer[0], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr );

		// Add a command to the command buffer
		VkViewport viewport;
		viewport.width = 512;
		viewport.height = 512;
		viewport.x = 0;
		viewport.y = 0;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport( command_buffer[0], 0, 1, &viewport );


		// End a command buffer
		vkEndCommandBuffer( command_buffer[0] );
	}

	// Command buffer 2
	{
		// Begin a command buffer
		VkCommandBufferBeginInfo begin_command_buffer_info {};
		begin_command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		vkBeginCommandBuffer( command_buffer[1], &begin_command_buffer_info );


		// Add a command to the command buffer
		VkViewport viewport;
		viewport.width = 512;
		viewport.height = 512;
		viewport.x = 0;
		viewport.y = 0;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport( command_buffer[1], 0, 1, &viewport );


		// End a command buffer
		vkEndCommandBuffer( command_buffer[1] );
	}
	
	// Submit the command buffer(s) to the device queue
	{
		VkSubmitInfo submit_info {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer[0];
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &semaphore;

		vkQueueSubmit( r.getQueue(), 1, &submit_info, VK_NULL_HANDLE );
	}

	{
		VkPipelineStageFlags flags[] { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };
		VkSubmitInfo submit_info {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer[1];
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &semaphore;
		submit_info.pWaitDstStageMask = flags;

		vkQueueSubmit( r.getQueue(), 1, &submit_info, VK_NULL_HANDLE );
	}


	// Wait for device idle
	vkQueueWaitIdle( r.getQueue() );

	// Detroy created Vulkan objects
	
	vkDestroyCommandPool( r.getDevice(), command_pool, nullptr );
	vkDestroySemaphore( r.getDevice(), semaphore, nullptr );
	
	
}

int main()
{
	Renderer r;

	TestCommandPoolWithFence( r );

	TestCommandPoolWithSemaphore( r );

	r.OpenWindow( 800, 600, "Vulkan Playpen" );

	while( r.Run() ) {

	}

	return 0;
}