#define GLFW_INCLUDE_VULKAN
//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <cstring>
#include <iostream>
#include <cstring>
#include <optional>
#include <vector>
#include <set>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
using namespace std;

const vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const vector<const char*> deviceExtensions ={
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct QueueFamilyIndices{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,const VkAllocationCallbacks* pAllocator,VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,"vkCreateDebugUtilsMessengerEXT");
    if(func !=nullptr)
    {
        return func(instance,pCreateInfo,pAllocator,pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,VkDebugUtilsMessengerEXT debugMessenger,const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,"vkDestroyDebugUtilsMessengerEXT");
    if(func!=nullptr)
    {
        func(instance,debugMessenger,pAllocator);
    }
}


    
class Entrance{
public:
    static void init()
    {
        if(!glfwInit())
        {
            cerr<<"Failed to init glfw"<<endl;
            exit(EXIT_FAILURE);
        }
        glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);
        window = glfwCreateWindow(WINDOW_WIDTH,WINDOW_HEIGHT,"April",nullptr,nullptr);
        if(window==nullptr){
           cerr<<"Failed to create window"<<endl; 
           exit(EXIT_FAILURE);
        }

        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();                
    }

    static void render()
    {
        while(!glfwWindowShouldClose(window))
        {
            update();
            glfwPollEvents();
        }
        cleanUp();
    }

    static void update()
    {
    }

    static void cleanUp()
    {
        if(enableValidationLayers)
        {
            DestroyDebugUtilsMessengerEXT(instance,debugMessenger,nullptr);
        }
        vkDestroyDevice(device,nullptr);
        vkDestroySurfaceKHR(instance,surface,nullptr);
        vkDestroyInstance(instance,nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
    }

private:
    static bool checkValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount,nullptr);
        vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount,availableLayers.data());
        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;
            for (const auto& layerProperty : availableLayers)
            {
                if(strcmp(layerName,layerProperty.layerName)==0){
                    layerFound=true;
                    break;
                }
            }
            if(!layerFound)
                return false;
        }
        return true;
    }

    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device,&queueFamilyCount,nullptr);
        vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device,&queueFamilyCount,queueFamilies.data());
        int i = 0;
        for(const auto& queueFamily : queueFamilies)
        {
            if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily=i;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device,i,surface,&presentSupport);
            if(presentSupport){
                indices.presentFamily = i;
            }
            if(indices.isComplete()){
                break;
            }
            i++;
        }
        return indices;
    }

    static void pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance,&deviceCount,nullptr);
        if(deviceCount==0)
        {
            cerr<<"failed to finde GPU with vulkan support"<<endl;
            exit(EXIT_FAILURE);
        }
        vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance,&deviceCount,devices.data());
        for (const auto& device : devices){
            if(isDeviceSuitable(device)){
                physicalDevice = device;
                break;
            }
        }
        if(physicalDevice==VK_NULL_HANDLE){
            cerr<<"failed to find a suitable GPU"<<endl;
            exit(EXIT_FAILURE);
        }
    }

    static bool isDeviceSuitable(VkPhysicalDevice device)
    {   
        QueueFamilyIndices indices = findQueueFamilies(device);
        return indices.isComplete() && checkDeviceExtensionSupport(device);
    }
    
    static bool checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device,nullptr,&extensionCount,nullptr);
        vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device,nullptr,&extensionCount,availableExtensions.data());

        set<string> requiredExtensions(deviceExtensions.begin(),deviceExtensions.end());

        for(const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }
        return requiredExtensions.empty();
    }

    static void createInstance()
    {
        if(enableValidationLayers&&!checkValidationLayerSupport()){
            cerr<<"Validation layer requested,but not available"<<endl;
            exit(EXIT_FAILURE);  
        }
        //create appInfo
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "April";
        appInfo.applicationVersion = VK_MAKE_VERSION(0,0,1);
        appInfo.pEngineName = "NO Engine";
        appInfo.apiVersion = VK_API_VERSION_1_2;
        //create insanceInfo
        VkInstanceCreateInfo instInfo = {};
        instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instInfo.pApplicationInfo = &appInfo;
        uint32_t ExtensionCount = 0;
        auto extensions = getRequiredExtensions();
        instInfo.enabledExtensionCount =  static_cast<uint32_t>(extensions.size());
        instInfo.ppEnabledExtensionNames = extensions.data();
        if(enableValidationLayers){
            instInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            instInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            instInfo.enabledLayerCount=0;
        }
        if(vkCreateInstance(&instInfo,nullptr,&instance)!=VK_SUCCESS)
        {
            cerr<<"Failed to create vkInst"<<endl;
            exit(EXIT_FAILURE);
        }
    }
    static void createLogicalDevice(){

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),indices.presentFamily.value()};
        
        float queuePriority = 1.0f;
        for(uint32_t queueFamily : uniqueQueueFamilies){
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
            
        }
                       
        VkPhysicalDeviceFeatures deviceFeatures = {} ;
        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos= queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = 0;
        
        if(enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }
        if(vkCreateDevice(physicalDevice,&createInfo,nullptr,&device)!=VK_SUCCESS)
        {
            cerr<<"failed to create logical device"<<endl;
            exit(EXIT_FAILURE);
        }
        vkGetDeviceQueue(device,indices.graphicsFamily.value(),0,&graphicsQueue);
        vkGetDeviceQueue(device,indices.presentFamily.value(),0,&presentQueue);
    }

    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    static void setupDebugMessenger(){
        if(!enableValidationLayers)
            return;
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);
        if(CreateDebugUtilsMessengerEXT(instance,&createInfo,nullptr,&debugMessenger)!=VK_SUCCESS){
            cerr<<"failed to setup debug messenger"<<endl;
            exit(EXIT_FAILURE);
        }
    }

    static void createSurface()
    {
        if(glfwCreateWindowSurface(instance,window,nullptr,&surface)!=VK_SUCCESS) {
            cerr<<"failed to create window surface"<<endl;
            exit(EXIT_FAILURE);
        }    

    }

    static vector<const char*> getRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        
        vector<const char*> extensions(glfwExtensions,glfwExtensions + glfwExtensionCount);
        if(enableValidationLayers){
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        return extensions;

    }
    
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData)
    {
        cerr<<"Validation_layer:"<<pCallbackData->pMessage<<endl;
        return VK_FALSE;
    }

    static VkInstance instance;
    static GLFWwindow* window;
    static VkDebugUtilsMessengerEXT debugMessenger;
    static VkPhysicalDevice physicalDevice;
    static VkDevice device;
    static VkQueue graphicsQueue;
    static VkQueue presentQueue;
    static VkSurfaceKHR surface;
};

VkInstance Entrance::instance;
GLFWwindow* Entrance::window;
VkDebugUtilsMessengerEXT Entrance::debugMessenger;
VkPhysicalDevice Entrance::physicalDevice = VK_NULL_HANDLE;
VkDevice Entrance::device;
VkQueue Entrance::graphicsQueue;
VkQueue Entrance::presentQueue;
VkSurfaceKHR Entrance::surface;

int main(){
    
    Entrance::init();
    Entrance::render();
    return 0;
}