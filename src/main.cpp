#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <cstring>
#include <iostream>
#include <vector>
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
using namespace std;

const vector<const char*> validationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

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
                
    }

    static void render()
    {
        window = glfwCreateWindow(WINDOW_WIDTH,WINDOW_HEIGHT,"April",nullptr,nullptr);
        if(window==nullptr){
           cerr<<"Failed to create window"<<endl; 
           exit(EXIT_FAILURE);
        }
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

    void createInstance(){

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

    vector<const char*>getRequiredExtensions()
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


    static VkInstance instance;
    static GLFWwindow* window;
};
VkInstance Entrance::instance;
GLFWwindow* Entrance::window;
int main(){
    
    Entrance::init();
    Entrance::render();

}