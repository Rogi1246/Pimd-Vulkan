//VulkanTutorial.cpp : Defines the entry point for the console application
//
//#define GLFW_INCLUDE_VULKAN
#include "vulkan\vulkan.h"
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <limits>


#define ASSERT_VULKAN(val)\
    if(val != VK_SUCCESS){\
    DebugBreak();\
    }


    //In real program use namespaces instead of global variables
VkInstance instance;
VkSurfaceKHR surface;
    //Erstelle logisches Device
VkDevice device;
VkSwapchainKHR swapchain;
VkImageView *imageViews;
VkFramebuffer *framebuffers;
    //basically to delete/destroy them after creating/using them
VkShaderModule shaderModuleVert;
VkShaderModule shaderModuleFrag;
VkPipelineLayout pipelineLayout;
VkRenderPass renderPass;
VkPipeline pipeline;
VkCommandPool commandPool;
VkCommandBuffer *commandBuffers;
VkQueue queue; //again: dummy
VkSemaphore semaphoreImageAvailable;
VkSemaphore semaphoreRenderingDone;
uint32_t amountOfImagesInSwapchain = 0;
GLFWwindow *window;

const uint32_t WIDTH = 400;
const uint32_t HEIGHT = 300;
const VkFormat usedFormat = VK_FORMAT_B8G8R8A8_UNORM;

void printStats(VkPhysicalDevice &device) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);

    std::cout << "Name :                      " << properties.deviceName << std::endl;
    uint32_t apiVer = properties.apiVersion;
    std::cout << "API Version :               " << VK_VERSION_MAJOR(apiVer) << "." << VK_VERSION_MINOR(apiVer) << "." << VK_VERSION_PATCH(apiVer) << std::endl;
    std::cout << "Driver Version :            " << properties.driverVersion << std::endl;
    std::cout << "Vendor ID :                 " << properties.vendorID << std::endl;
    std::cout << "Device ID :                 " << properties.deviceID << std::endl;
    std::cout << "Device Type :               " << properties.deviceType << std::endl;
    std::cout << "Discrete Queue Priorities : " << properties.limits.discreteQueuePriorities << std::endl;
    std::cout << std::endl;

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);
    std::cout << "Geometry Shader :           " << features.geometryShader << std::endl;
    std::cout << std::endl;

    VkPhysicalDeviceMemoryProperties memProp;
    vkGetPhysicalDeviceMemoryProperties(device, &memProp);

    uint32_t amountOfQueueFamilies = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &amountOfQueueFamilies, nullptr);
    VkQueueFamilyProperties *familyProperties = new VkQueueFamilyProperties[amountOfQueueFamilies];

    vkGetPhysicalDeviceQueueFamilyProperties(device, &amountOfQueueFamilies, familyProperties);
    //richtiges Befüllen

    std::cout << "Amount of Queue Families : " << amountOfQueueFamilies << std::endl;

    for (int i = 0; i < amountOfQueueFamilies; i++){
        std::cout << std::endl;
        std::cout << "Queue Family # " << i << std::endl;
        std::cout << "VK_QUEUE_GRAPHICS_BIT            " << ((familyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) << std::endl;
        std::cout << "VK_QUEUE_COMPUTE_BIT             " << ((familyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0) << std::endl;
        std::cout << "VK_QUEUE_TRANSFER_BIT            " << ((familyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0) << std::endl;
        std::cout << "VK_QUEUE_SPARSE_BINDING_BIT      " << ((familyProperties[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) != 0) << std::endl;
        std::cout << "Queue Count :                    " << familyProperties[i].queueCount << std::endl;
        std::cout << "Timestamp Valid Bits:            " << familyProperties[i].timestampValidBits << std::endl;
        uint32_t width = familyProperties[i].minImageTransferGranularity.width;
        uint32_t height = familyProperties[i].minImageTransferGranularity.height;
        uint32_t depth = familyProperties[i].minImageTransferGranularity.depth;
        std::cout << "MinImage Timestamp Granularity : " << width << " , " << height << " , " << depth << std::endl;
    }

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &surfaceCapabilities);
    std::cout << std::endl;
    std::cout << "SURFACE CAPABILITIES      " << std::endl;
    std::cout << "minImageCount :           " << surfaceCapabilities.minImageCount << std::endl;
    std::cout << "maxImageCount :           " << surfaceCapabilities.maxImageCount << std::endl;
    std::cout << "currentExtent :           " << surfaceCapabilities.currentExtent.width << "/" << surfaceCapabilities.currentExtent.height << std::endl;
    std::cout << "minImageExtent :          " << surfaceCapabilities.minImageExtent.width << "/" << surfaceCapabilities.minImageExtent.height << std::endl;
    std::cout << "maxImageExtent :          " << surfaceCapabilities.maxImageExtent.width << "/" << surfaceCapabilities.maxImageExtent.height << std::endl;
    std::cout << "maxImageArrayLayers :     " << surfaceCapabilities.maxImageArrayLayers << std::endl;
    std::cout << "supportedTransforms :     " << surfaceCapabilities.supportedTransforms << std::endl;
    std::cout << "currentTransform :        " << surfaceCapabilities.currentTransform << std::endl;
    std::cout << "supportedCompositeAlpha : " << surfaceCapabilities.supportedCompositeAlpha << std::endl;
    std::cout << "supportedUsageFlags :     " << surfaceCapabilities.supportedUsageFlags << std::endl;

    uint32_t amountOfFormats = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &amountOfFormats, nullptr);
    VkSurfaceFormatKHR *surfaceFormats = new VkSurfaceFormatKHR[amountOfFormats];
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &amountOfFormats, surfaceFormats);

    std::cout << std::endl;
    std::cout << "Amount of Formats :   " << amountOfFormats << std::endl;
    for(int i = 0; i < amountOfFormats; i++){
        std::cout << "Format :          " << surfaceFormats[i].format << std::endl;
    }

    uint32_t amountOfPresentationModes = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &amountOfPresentationModes, nullptr);
    VkPresentModeKHR *presentModes = new VkPresentModeKHR[amountOfPresentationModes];
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &amountOfPresentationModes, presentModes);

    std::cout << std::endl;
    std::cout << "Amount of Presentation Modes :    " << amountOfPresentationModes << std::endl;
    std::cout << std::endl;
    for(int i = 0; i < amountOfPresentationModes; i++){
        std::cout << "Supported presentation mode :     " << presentModes[i] << std::endl;
    }

    std::cout << std::endl;
    delete[] familyProperties;
    delete[] surfaceFormats;
    delete[] presentModes;
}

std::vector<char> readFile(const std::string& filename){
    std::ifstream file(filename, std::ios::binary | std::ios::ate); //look how big the read file is by starting with a pointer at the end of said file
    if (file) {
        size_t fileSize = (size_t) file.tellg(); //tellg() > where is reading point atm? > at the beginning because of former function > file size
        std::vector<char> fileBuffer(fileSize);
        file.seekg(0); //put reading point at the beginning
        file.read(fileBuffer.data(), fileSize);
        file.close(); //cleaning up because we can only have certain amount of handles at once
        return fileBuffer;
    } else {
        throw std::runtime_error(" ------------------- Failed to open file ------------------- ");
    }
}

void startGLFW() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Tutorial", nullptr, nullptr);
}

void createShaderModule(const std::vector<char>& code, VkShaderModule *shaderModule) {
    VkShaderModuleCreateInfo shaderCreateInfo;
    shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderCreateInfo.pNext = nullptr;
    shaderCreateInfo.flags = 0;
    shaderCreateInfo.codeSize = code.size();
    shaderCreateInfo.pCode = (uint32_t*)code.data(); //casted c-style

    VkResult result = vkCreateShaderModule(device, &shaderCreateInfo, nullptr, shaderModule);
    ASSERT_VULKAN(result);
}

void startVulkan() {
    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "Vulkan Tutorial";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.pEngineName = "Super Vulkan Engine Turbo Mega";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_1;

    uint32_t amountOfLayers = 0;
    vkEnumerateInstanceLayerProperties(&amountOfLayers, nullptr);
    VkLayerProperties *layers = new VkLayerProperties[amountOfLayers];
    vkEnumerateInstanceLayerProperties(&amountOfLayers, layers);

    std::cout << "Amount of Instance Layers : " << amountOfLayers << std::endl;
    for(int i = 0; i < amountOfLayers; i++) {
        std::cout << std::endl;
        std::cout << "Name :                  " << layers[i].layerName << std::endl;
        std::cout << "Spec Version :          " << layers[i].specVersion << std::endl;
        std::cout << "Impl Version :          " << layers[i].implementationVersion << std::endl;
        std::cout << "Description :           " << layers[i].description << std::endl;
        std::cout << std::endl;
    }

    uint32_t amountOfExtensions = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &amountOfExtensions, nullptr);
    VkExtensionProperties *extensions = new VkExtensionProperties[amountOfExtensions];
    vkEnumerateInstanceExtensionProperties(nullptr, &amountOfExtensions, extensions);

    std::cout << std::endl;
    std::cout << "Amount of Extensions  : " << amountOfExtensions << std::endl;
    for(int i = 0; i < amountOfExtensions; i++) {
        std::cout << std::endl;
        std::cout << "Name              : " << extensions[i].extensionName << std::endl;
        std::cout << "Spec Version      : " << extensions[i].specVersion << std::endl;
        std::cout << std::endl;
    }

    const std::vector<const char*> validationLayers = {
            "VK_LAYER_LUNARG_standard_validation"
    };

    /*const std::vector<const char*> usedExtensions = {
            "VK_KHR_surface", // Extension which is generally needed ! not only for windows
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };*/


    uint32_t amountOfGlfwExtensions = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&amountOfGlfwExtensions);

    VkInstanceCreateInfo instanceInfo;
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = nullptr;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = validationLayers.size();
    instanceInfo.ppEnabledLayerNames = validationLayers.data();
    instanceInfo.enabledExtensionCount = amountOfGlfwExtensions; // Extensions enabled on Instance-layer
    instanceInfo.ppEnabledExtensionNames = glfwExtensions;

    VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);
    ASSERT_VULKAN(result);

    result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    ASSERT_VULKAN(result);

    /*VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.hinstance = nullptr;
    surfaceCreateInfo.hwnd = nullptr;
    */

    /*VkSurfaceKHR surface;
    result = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
    ASSERT_VULKAN(result);
    */

    uint32_t amountOfPhysicalDevices = 0;
    result = vkEnumeratePhysicalDevices(instance, &amountOfPhysicalDevices, nullptr);
    ASSERT_VULKAN(result);
    //std::cout << amountOfPhysicalDevices;

    //VkPhysicalDevice* physicalDevices = new VkPhysicalDevice[amountOfPhysicalDevices];

    //alternative for classical Array with "new VkPhysicalDevice
    std::vector<VkPhysicalDevice> physicalDevices;
    physicalDevices.resize(amountOfPhysicalDevices);

    //result = vkEnumeratePhysicalDevices(instance, &amountOfPhysicalDevices, physicalDevices) //for array version
    result = vkEnumeratePhysicalDevices(instance, &amountOfPhysicalDevices, physicalDevices.data()); //for vector version
    ASSERT_VULKAN(result);

    for(int i = 0; i < amountOfPhysicalDevices; i++){
        printStats(physicalDevices[i]);
    }

    float queuePrios[] = {1.0f, 1.0f, 1.0f, 1.0f};

    VkDeviceQueueCreateInfo deviceQueueCreateInfo;
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.pNext = nullptr;
    deviceQueueCreateInfo.flags = 0;
    deviceQueueCreateInfo.queueFamilyIndex = 0; //erste Family auf eigener GK
    // TODO Choose correct family index
    deviceQueueCreateInfo.queueCount = 1; //TODO Check if this amount is valid
    deviceQueueCreateInfo.pQueuePriorities = queuePrios; // Array an Floats

    VkPhysicalDeviceFeatures usedFeatures = {}; //alle bools auf false bzw alles "genullt"

    const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = nullptr;
    deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceCreateInfo.pEnabledFeatures = &usedFeatures;

    //für logical device
    //TODO pick best instead of first device
    result = vkCreateDevice(physicalDevices[0], &deviceCreateInfo, nullptr, &device);
    ASSERT_VULKAN(result);

    vkGetDeviceQueue(device, 0, 0, &queue);

    VkBool32 surfaceSupport = false;
    result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[0], 0, surface, &surfaceSupport);
    ASSERT_VULKAN(result);

    if(!surfaceSupport){
        std::cerr << "Surface not supported! " << std::endl;
        DebugBreak();
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo;
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext = nullptr;
    swapchainCreateInfo.flags = 0;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = 3; //TODO check if valid (civ)
    swapchainCreateInfo.imageFormat = usedFormat; //TODO civ
    swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR; //TODO civ
    swapchainCreateInfo.imageExtent = VkExtent2D{WIDTH, HEIGHT};
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //not shared
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; //no preTransform
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //makes transparent pixels opaque
    swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; //always working TODO civ
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE; // Handle : more or less somethin like a pointer

    result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);
    ASSERT_VULKAN(result);

    vkGetSwapchainImagesKHR(device, swapchain, &amountOfImagesInSwapchain, nullptr);
    VkImage *swapchainImages = new VkImage[amountOfImagesInSwapchain];
    result = vkGetSwapchainImagesKHR(device, swapchain, &amountOfImagesInSwapchain, swapchainImages);
    ASSERT_VULKAN(result);

    imageViews = new VkImageView[amountOfImagesInSwapchain];
    for(int i = 0; i < amountOfImagesInSwapchain; i++){
        VkImageViewCreateInfo imageViewCreateInfo;
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.pNext = nullptr;
        imageViewCreateInfo.flags = 0;
        imageViewCreateInfo.image = swapchainImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = usedFormat; //same as in swapchainCreateInfo TODO civ
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        result = vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageViews[i]);
        ASSERT_VULKAN(result);
    }

    auto shaderCodeVert = readFile("C:\\Users\\Vanessa Retz\\CLionProjects\\VulkanTutorial\\vert.spv"); //auto instead of : std::vector<char> .. too much to write apparently
    auto shaderCodeFrag = readFile("C:\\Users\\Vanessa Retz\\CLionProjects\\VulkanTutorial\\frag.spv"); //had to include Path of the files otherwise runtime error
    //those are basically just char arrays


    std::cout << "Size of vert.spv : " << shaderCodeVert.size() << " Bytes" << std::endl; // expected exactly the bite size you can see in document folder
    std::cout << "Size of frag.spv : " << shaderCodeFrag.size() << " Bytes" <<std::endl;

    createShaderModule(shaderCodeVert, &shaderModuleVert);
    createShaderModule(shaderCodeFrag, &shaderModuleFrag);

    VkPipelineShaderStageCreateInfo shaderStageCreateInfoVert;
    shaderStageCreateInfoVert.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfoVert.pNext = nullptr;
    shaderStageCreateInfoVert.flags = 0;
    shaderStageCreateInfoVert.stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStageCreateInfoVert.module = shaderModuleVert;
    shaderStageCreateInfoVert.pName = "main";
    shaderStageCreateInfoVert.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shaderStageCreateInfoFrag;
    shaderStageCreateInfoFrag.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfoFrag.pNext = nullptr;
    shaderStageCreateInfoFrag.flags = 0;
    shaderStageCreateInfoFrag.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStageCreateInfoFrag.module = shaderModuleFrag;
    shaderStageCreateInfoFrag.pName = "main";
    shaderStageCreateInfoFrag.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shaderStages[] = {shaderStageCreateInfoVert, shaderStageCreateInfoFrag};

    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputCreateInfo.pNext = nullptr;
    vertexInputCreateInfo.flags = 0;
    vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
    vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
    vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
    vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo;
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.pNext = nullptr;
    inputAssemblyCreateInfo.flags = 0;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = WIDTH;
    viewport.height = HEIGHT;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = {0,0};
    scissor.extent = {WIDTH, HEIGHT};

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo;
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.pNext = nullptr;
    viewportStateCreateInfo.flags = 0;
    viewportStateCreateInfo.viewportCount = 1; //how many viewports do we have
    viewportStateCreateInfo.pViewports = &viewport; //memory address of viewport
    viewportStateCreateInfo.scissorCount = 1; //how many scissors do we have
    viewportStateCreateInfo.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo;
    rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationCreateInfo.pNext = nullptr;
    rasterizationCreateInfo.flags = 0;
    rasterizationCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizationCreateInfo.depthBiasConstantFactor = 0.0f;
    rasterizationCreateInfo.depthBiasClamp = 0.0f;
    rasterizationCreateInfo.depthBiasSlopeFactor = 0.0f;
    rasterizationCreateInfo.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampleCreateInfo;
    multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleCreateInfo.pNext = nullptr;
    multisampleCreateInfo.flags = 0;
    multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleCreateInfo.minSampleShading = 1.0f;
    multisampleCreateInfo.pSampleMask = nullptr;
    multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleCreateInfo.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo;
    colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendCreateInfo.pNext = nullptr;
    colorBlendCreateInfo.flags = 0;
    colorBlendCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;
    colorBlendCreateInfo.attachmentCount = 1;
    colorBlendCreateInfo.pAttachments = &colorBlendAttachment;
    colorBlendCreateInfo.blendConstants[0] = 0.0f; //Rgba
    colorBlendCreateInfo.blendConstants[1] = 0.0f;
    colorBlendCreateInfo.blendConstants[2] = 0.0f;
    colorBlendCreateInfo.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pNext = nullptr;
    pipelineLayoutCreateInfo.flags = 0;
    pipelineLayoutCreateInfo.setLayoutCount = 0;
    pipelineLayoutCreateInfo.pSetLayouts = nullptr;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

    result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    ASSERT_VULKAN(result);

    VkAttachmentDescription attachmentDescription; //can be reused for different attachments
    attachmentDescription.flags = 0;
    attachmentDescription.format = usedFormat;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //framebuffer is set to a color when attachment's loaded
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //layout before getting into renderpass (so before starting to render)
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //Output is displayable on screen, at the end of renderpass, was made automatically out of attachmentReference.layout

    VkAttachmentReference attachmentReference;
    attachmentReference.attachment = 0; //index in "array", which VkAttachmentDescription we want to use
    attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //being transformed at the beginning of renderpass, so initialLayout is being made into this

    VkSubpassDescription subpassDescription;
    subpassDescription.flags = 0;
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; //describes what our Subpass is ; COMPUTE would be for number-crunching and not directly displaying something (f.e. matrix multiplication)
    subpassDescription.inputAttachmentCount = 0; //for input that falls into shader
    subpassDescription.pInputAttachments = nullptr;
    subpassDescription.colorAttachmentCount = 1; //is the attachment we wrote, how many do we have and a pointer
    subpassDescription.pColorAttachments = &attachmentReference;
    subpassDescription.pResolveAttachments = nullptr; //used for multisampling when doing anti-aliasing
    subpassDescription.pDepthStencilAttachment = nullptr;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = nullptr; //attachment thats not used directly in this subpass but which data is still wanted to be preserved

    VkSubpassDependency subpassDependency;
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL; //Index of the subpasses; first one incoming
    subpassDependency.dstSubpass = 0; //same; where it should go
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; //when to execute
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.srcAccessMask = 0;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependency.dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassCreateInfo;
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.pNext = nullptr;
    renderPassCreateInfo.flags = 0;
    renderPassCreateInfo.attachmentCount = 1; //how many attachments do we have
    renderPassCreateInfo.pAttachments = &attachmentDescription; //pointer to those
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.dependencyCount = 1; //you can make the subpasses dependent on each other; (f.e. draw them one after the other, so one's only drawn when the other's finished)
    renderPassCreateInfo.pDependencies = &subpassDependency;

    result = vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass);
    ASSERT_VULKAN(result);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.pNext = nullptr;
    pipelineCreateInfo.flags = 0;
    pipelineCreateInfo.stageCount = 2; //Shader stages, how many shaders do we have
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    pipelineCreateInfo.pTessellationState = nullptr; //don't have that
    pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    pipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
    pipelineCreateInfo.pDepthStencilState = nullptr;
    pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
    pipelineCreateInfo.pDynamicState = nullptr; /*usually when wanting to change a pipeline you'll have to Create it anew, but some things you can change with this dynamicState /f.e. for viewport when
                                                doing rezisable windows (didn't do that here) */
    pipelineCreateInfo.layout = pipelineLayout; //don't have to use reference because it's a handle
    pipelineCreateInfo.renderPass = renderPass; //just the same
    pipelineCreateInfo.subpass = 0; //index which subpass we want to use; only have one
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; //(both functions) we can make pipelines inherit from one another >> switching between pipelines works faster if their inheritance is close together
    pipelineCreateInfo.basePipelineIndex = -1; /*not using that r/n ; using non-valid-index
                                               if we'd use that >> flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT and the one we can inherit from : flags = [...]_ALLOW_DERIVATIVES_BIT */


    //Handles roughly work like pointers, but we can't use pointer arithmetic (can't add +1 on those or something) ; pointing to Vulkan stuff running in the background

    //PipelineCache >> f.e. saving pipeline into file and load from this file
    result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline);
    ASSERT_VULKAN(result);

    framebuffers = new VkFramebuffer[amountOfImagesInSwapchain];
    //Framebuffer is used for every image, actually able to work with that
    for(size_t i = 0; i < amountOfImagesInSwapchain; i++){
        VkFramebufferCreateInfo framebufferCreateInfo;
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.pNext = nullptr;
        framebufferCreateInfo.flags = 0;
        framebufferCreateInfo.renderPass = renderPass;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = &(imageViews[i]);
        framebufferCreateInfo.width = WIDTH;
        framebufferCreateInfo.height = HEIGHT;
        framebufferCreateInfo.layers = 1;

        result = vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers[i]);
        ASSERT_VULKAN(result);
    }

    //Start doing a renderpass, rendering a frame etc. Communicating with Vulkan
    //CommandPool to get Commandbuffers
    //Commandbuffers : first recording a command (CPU) then sending commandbuffer to GPU to execute what's inside; able to override/reset either all commandBuffers in commandPool or separately

    VkCommandPoolCreateInfo commandPoolCreateInfo;
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.pNext = nullptr;
    commandPoolCreateInfo.flags = 0;
    commandPoolCreateInfo.queueFamilyIndex = 0; //TODO civ - get correct queue with VK_QUEUE_GRAPHICS_BIT
    //queue/family we want to use - important to support Queue Family Propertier(BITS) (see far above)

    result = vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool);
    ASSERT_VULKAN(result);

    VkCommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = nullptr;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //primary (can be put into queue to execute immediately and can call secondary command Buffer) or secondary (not directly submitted, can be called by primary ones)
    commandBufferAllocateInfo.commandBufferCount = amountOfImagesInSwapchain; //as many as images in swapchain because as many framebuffers

    commandBuffers = new VkCommandBuffer[amountOfImagesInSwapchain];
    result = vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers);
    ASSERT_VULKAN(result);

    //to begin recording command
    VkCommandBufferBeginInfo commandBufferBeginInfo;
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    /*first flag (USAGE_ONE_TIME_SUBMIT) : command buffer is directly new recorded after being submitted;
     * second flag (USAGE_RENDER_PASS_CONTINUE BIT) : just in one renderpass
     * third  flag (USAGE_SIMULTANEOUS_USE_BIT) : when we send commandBuffer to GPU, we can send it again even when it's not done processing yet ; USING
     * fourth flag (USAGE_FLAG_BITS_MAX_ENUM) ...*/
    commandBufferBeginInfo.pInheritanceInfo = nullptr; //only important when using a secondary command buffer

    for(size_t i = 0; i < amountOfImagesInSwapchain; i++){
        result = vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo);
        ASSERT_VULKAN(result);

        //starting renderpass in commandbuffer
        VkRenderPassBeginInfo renderPassBeginInfo;
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.pNext = nullptr;
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.framebuffer = framebuffers[i];
        renderPassBeginInfo.renderArea.offset = {0,0}; //we want to render complete window
        renderPassBeginInfo.renderArea.extent = {WIDTH, HEIGHT};
        VkClearValue clearValue = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassBeginInfo.clearValueCount = 1; //what's happening at the beginning of framebuffer
        renderPassBeginInfo.pClearValues = &clearValue;

        //record start and end of renderpass; start recording : vkCmd
        vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        //bind pipeline for graphics
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
        //ending recording
        vkCmdEndRenderPass(commandBuffers[i]);

        result = vkEndCommandBuffer(commandBuffers[i]);
        ASSERT_VULKAN(result);
    }

    VkSemaphoreCreateInfo semaphoreCreateInfo;
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.flags = 0;

    result = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphoreImageAvailable);
    ASSERT_VULKAN(result);
    result = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphoreRenderingDone);
    ASSERT_VULKAN(result);

    //Clean up C++ - delete all "news" created in main function!
    //when using the vector version, delete is no longer needed
    delete[] swapchainImages;
    delete[] layers;
    delete[] extensions;
    //delete[] physicalDevices; laying on a stack, so a destructor is called automatically when the function is finished

}

void drawFrame(){
    //get Image from swapchain, render into it and give it back to swapchain so it will be displayed
    //semaphores : objects that can be triggered to either let stuff through or blocks, to force keeping to sequence
    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, swapchain, std::numeric_limits<uint64_t>::max(), semaphoreImageAvailable, VK_NULL_HANDLE, &imageIndex); //timeout : how many nano seconds are given at max to acquire image, numeric limits : maximum time given

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &semaphoreImageAvailable;
    VkPipelineStageFlags waitStageMask[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }; //could add commata and add several more
    submitInfo.pWaitDstStageMask = waitStageMask; //where in the pipeline should it wait for the semaphore; Here : when color attachment starts
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &(commandBuffers[imageIndex]);
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &semaphoreRenderingDone; //on false, is blocking when other commands are waiting

    VkResult result = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    ASSERT_VULKAN(result);

    //vulkan distinguishes between rendering and presenting on screen
    VkPresentInfoKHR presentInfo;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &semaphoreRenderingDone;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(queue, &presentInfo);
    ASSERT_VULKAN(result);
}

void gameLoop() {
    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        //so the frame actually shows
        drawFrame();
    }
}

void shutdownVulkan() {
    //make sure all functions on device are finished / everything's returned
    vkDeviceWaitIdle(device);

    vkDestroySemaphore(device, semaphoreImageAvailable, nullptr);
    vkDestroySemaphore(device, semaphoreRenderingDone, nullptr);
    vkFreeCommandBuffers(device, commandPool, amountOfImagesInSwapchain, commandBuffers); //not necessary BUT f.e. if in your game needs one special command buffer thats being allocated before the level and then freed
    delete[] commandBuffers;
    vkDestroyCommandPool(device, commandPool, nullptr);

    for(size_t i = 0; i < amountOfImagesInSwapchain; i++){
        vkDestroyFramebuffer(device, framebuffers[i], nullptr);
    }
    delete[] framebuffers;
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    for(int i = 0; i < amountOfImagesInSwapchain; i++){
        vkDestroyImageView(device, imageViews[i], nullptr);
    }
    delete[] imageViews;
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyShaderModule(device, shaderModuleVert, nullptr);
    vkDestroyShaderModule(device, shaderModuleFrag, nullptr);
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    //in reversed order in which they were created
    vkDestroyDevice(device, nullptr); //the logical device
    //vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr); //also destroys all physical devices because they're part of instance
}

void shutdownGLFW() {
    glfwDestroyWindow(window);
}

int main()
{
    startGLFW();
    startVulkan();
    gameLoop();
    shutdownVulkan(); //shutdown in reverse order as well as delete
    shutdownGLFW();

    return 0;
}