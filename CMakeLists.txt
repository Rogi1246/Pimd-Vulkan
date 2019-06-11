cmake_minimum_required(VERSION 3.14)
project(VulkanTutorial)

set(CMAKE_CXX_STANDARD 14)

#including GLFW
add_subdirectory(glfw)
include_directories(glfw/deps)
include_directories(glfw/docs)
include_directories(glfw/include/GLFW)
include_directories(glfw/lib-vc2019)
include_directories(glfw/CMake)
include_directories(glfw/src)
include_directories(${GLFW_INCLUDE_DIRS})

link_directories(glfw/lib-mingw-w64)

add_definitions(-DGLFW_INCLUDE_VULKAN)
add_executable(VulkanTutorial
               VulkanTutorial.cpp)

find_package(Vulkan REQUIRED)
find_package(OpenGL REQUIRED)
target_compile_definitions(VulkanTutorial PRIVATE VK_USE_PLATFORM_WIN32_KHR)
#Linking GLFW
target_link_libraries(VulkanTutorial PRIVATE Vulkan::Vulkan
        glfw ${GLFW_LIBRARIES}
        ${OPENGL_LIBRARIES}
        ${COCOA_LIBRARY}
        ${COREVID_LIBRARY}
        ${IOKIT_LIBRARY}
        ${GLFW3_LIBRARY} glfw)