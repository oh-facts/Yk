#include <renderer/device.h>


enum Q_FAM
{
    Q_FAM_GFX,
    Q_FAM_GFX_COMPUTE,
    Q_FAM_PRESENT,
    Q_FAM_SIZE
};


void mn_device_innit(mn_context* context, VkSurfaceKHR surface, mn_device* device)
{
    //setup device
    VkPhysicalDevice vk_phys_device = { 0 };

#define max_devices 3

    u32 devices = 0;
    vkEnumeratePhysicalDevices(context->vk_instance, &devices, 0);

    Assert(devices <= max_devices, "More than 3 graphics cards? Wth?")

    VkPhysicalDevice device_list[max_devices] = { 0 };

    VkResultAssert(vkEnumeratePhysicalDevices(context->vk_instance, &devices, device_list), "physical device detection")
    vk_phys_device = device_list[0];

    //ToDo(facts) poll device properties properly
    // But I only have one GPU so its fine for now.


    for (i32 i = 0; i < devices; i++)
    {
        VkPhysicalDeviceProperties vk_phys_device_props = { 0 };
        vkGetPhysicalDeviceProperties(device_list[i], &vk_phys_device_props);

        if (vk_phys_device_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            device->phys_device = device_list[i];
            log_device(&vk_phys_device_props)
            break;
        }
    }


    log_extention(check_device_extension_support(device->phys_device))

        //set up queues

#define max_queues 5
        u32 queues = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device->phys_device, &queues, 0);

    Assert(queues <= max_queues, "More queues found than supported")

    VkQueueFamilyProperties vk_q_fam_prop_list[max_queues];
    u32 current_queues = max_queues;
    vkGetPhysicalDeviceQueueFamilyProperties(device->phys_device, &current_queues, vk_q_fam_prop_list);

    i32 present_queue_found = -1;
    for (i32 i = 0; i < current_queues; i++)
    {
        VkQueueFlags qflags = vk_q_fam_prop_list[i].queueFlags;
        if (qflags & VK_QUEUE_GRAPHICS_BIT)
        {
            device->qfams[Q_FAM_GFX] = i;
        }

        if ((qflags & VK_QUEUE_GRAPHICS_BIT) && (qflags & VK_QUEUE_COMPUTE_BIT))
        {
            device->qfams[Q_FAM_GFX_COMPUTE] = i;
        }

        if (present_queue_found == -1)
        {
            VkBool32 present_support = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device->phys_device, i, surface, &present_support);

            if (present_support == VK_TRUE)
            {
                device->qfams[Q_FAM_PRESENT] = i;
            }

            present_queue_found = 1;

        }

    }

    //Note(facts): They're all definitely the same family. Return to this when you understand the API better

    Assert(device->qfams[Q_FAM_GFX] != -1, "Graphics Queue not found")
    Assert(device->qfams[Q_FAM_GFX_COMPUTE] != -1, "Graphics Compute Queue not found")
    Assert(device->qfams[Q_FAM_PRESENT] != -1, "Present Queue not found")

    //create logical device


    //Logical Device starts here
    float queue_priority = 1;

    // Note(facts): The graphic , compute and present queue families are all index 0. And this is common behaviour. For now I am leaving this
    // like this. Later, I will make sure that incase they are different families, each gets its own queue

    VkDeviceQueueCreateInfo vk_device_q_create_info = { 0 };
    vk_device_q_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    vk_device_q_create_info.pNext = 0;
    vk_device_q_create_info.flags = 0;
    vk_device_q_create_info.queueFamilyIndex = Q_FAM_GFX;
    //this is number of queues you want to create. Not how many queues are available in that queue family
    vk_device_q_create_info.queueCount = 1;
    vk_device_q_create_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceDynamicRenderingFeaturesKHR vk_dynamic_rendering_feature = { 0 };
    vk_dynamic_rendering_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    vk_dynamic_rendering_feature.dynamicRendering = VK_TRUE;

    const char* device_extention_names[2] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };

    VkDeviceCreateInfo vk_device_create_info = { 0 };
    vk_device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vk_device_create_info.pNext = &vk_dynamic_rendering_feature;
    vk_device_create_info.flags = 0;
    vk_device_create_info.queueCreateInfoCount = 1;
    vk_device_create_info.pQueueCreateInfos = &vk_device_q_create_info;
    vk_device_create_info.enabledLayerCount = 0;    //ignored
    vk_device_create_info.ppEnabledLayerNames = 0;  //ignored
    vk_device_create_info.enabledExtensionCount = 2;
    vk_device_create_info.ppEnabledExtensionNames = device_extention_names;
    vk_device_create_info.pEnabledFeatures = 0;

    VkResultAssert(vkCreateDevice(device->phys_device, &vk_device_create_info, 0, &device->handle), "Vulkan device creation");

    // ToDo(facts 12/24 0439): Its all one queue. Come back to this later. If even one gfx card is made where the queues are literally different,
    // I will account for them
    vkGetDeviceQueue(device->handle, Q_FAM_GFX, 0, &device->gfx_q);
}