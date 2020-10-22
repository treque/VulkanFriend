#include <vulkan/vulkan.h>
#include <assert.h>
#include <string.h>
#include<stdio.h>
#include <memory>
VkInstance instance = VK_NULL_HANDLE;

/// <summary>
///
/// </summary>
/// <param name="physical_device"></param>
/// <param name="required_extentions"></param>
/// <param name="extension_count">amount of extensions we want</param>
/// <returns></returns>
bool HasRequiredExtensions(const VkPhysicalDevice& physical_device, const char** required_extensions, uint32_t extension_count)
{
	uint32_t device_extension_count = 0;
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, nullptr);
	VkExtensionProperties* extensions = new VkExtensionProperties[device_extension_count];
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, extensions);

	for (uint32_t i = 0; i < extension_count; ++i)
	{
		bool extension_found = false;
		// are you the swapchain?
		for (uint32_t j = 0; j < device_extension_count; ++j)
		{
			if(strcmp(required_extensions[i], extensions[j].extensionName) == 0)
			{
				extension_found = true;
				break;
			}
		}
		if (!extension_found)
		{
			delete[] extensions;
			return false;
		}
	}

	return true;

}

/// <summary>
/// Does it have the queue family requested? If so, returns it.
/// </summary>
/// <returns></returns>
bool GetQueueFamily(const VkPhysicalDevice& physical_device, VkQueueFlags flags, uint32_t& queue_family_index)
{
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
	VkQueueFamilyProperties* queue_families = new VkQueueFamilyProperties[queue_family_count];
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);

	for (uint32_t i = 0; i < queue_family_count; ++i)
	{
		//does this q fam actually have a q
		if (queue_families[i].queueCount > 0)
		{
			if ((queue_families[i].queueFlags & flags) == flags)
			{
				queue_family_index = i;
				delete[] queue_families;
				return true;
			}
		}
	}

	delete[] queue_families;
	return false;
}

int main(int argc, char** argv)
{
	// layers and extensions we're telling vulkan to use (like android.. debug extension)
	const char* instance_layers[] = { "VK_LAYER_LUNARG_standard_validation" };
	const char* instance_extensions[] = { "VK_EXT_debug_report" };

	//
	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Vulkan Renderer";
	app_info.pEngineName = "The Future";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_MAKE_VERSION(1, 1, 108);

	VkInstanceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;
	create_info.enabledExtensionCount = 1;
	create_info.ppEnabledExtensionNames = instance_extensions;
	create_info.enabledLayerCount = 1;
	create_info.ppEnabledLayerNames = instance_layers;

	// creating the instance
	VkResult result = vkCreateInstance(&create_info, NULL, &instance);
	assert(result == VK_SUCCESS);


	// find physical devices
	uint32_t device_count = 0;
	// finding how many devices there are
	vkEnumeratePhysicalDevices(
		instance,
		&device_count,
		nullptr
	);

	VkPhysicalDevice* devices = new VkPhysicalDevice[device_count];
	vkEnumeratePhysicalDevices(
		instance,
		&device_count,
		devices					//will populate devices
	);

	const uint32_t extension_count = 1;
	const char* device_extensions[extension_count] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME }; // platform specific. raster



	VkPhysicalDevice chosen_device = VK_NULL_HANDLE;
	uint32_t chosen_queue_family_index = 0;
	VkPhysicalDeviceProperties chosen_physical_device_properties;
	VkPhysicalDeviceFeatures chosen_physical_device_features;
	VkPhysicalDeviceMemoryProperties chosen_physical_device_memory_properties;
	for (uint32_t i = 0; i < device_count; ++i)
	{
		if (HasRequiredExtensions(devices[i], device_extensions, extension_count)) // we find a graphics card that supports the extensions we need
		{
			// check if the gpu has a queue family.
			uint32_t queue_family_index = 0;
			if (GetQueueFamily(devices[i], VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, queue_family_index)) // found a graphics card capable of rendering
			{
				VkPhysicalDeviceProperties physical_device_properties;
				vkGetPhysicalDeviceProperties(
					devices[i],
					&physical_device_properties
				);

				VkPhysicalDeviceFeatures physical_device_features;
				vkGetPhysicalDeviceFeatures(
					devices[i],
					&physical_device_features
				);

				VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
				vkGetPhysicalDeviceMemoryProperties(
					devices[i],
					&physical_device_memory_properties
				);

				if (chosen_device == VK_NULL_HANDLE || physical_device_properties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				{
					chosen_device = devices[i];
					chosen_physical_device_properties = physical_device_properties;
					chosen_physical_device_features = physical_device_features;
					chosen_physical_device_memory_properties = physical_device_memory_properties;
					chosen_queue_family_index = queue_family_index;
				}
			}
		}
	}





	assert(chosen_device != VK_NULL_HANDLE);


	vkDestroyInstance(instance, NULL);

	return 0;
}