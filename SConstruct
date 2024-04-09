dlldst = '../out/'
reldir = '\\"./vkEngine\\"'

env = Environment(CC = 'cl',
                   CCFLAGS = ['/std:c++17','/EHsc','/O2'])


libs = ['msvcrtd', 'libcmt', 'Gdi32', 'shell32', 'user32', 'vulkan-1', 'glfw3', 'vkengine','assimp'] 
libpath = ['./libs','./vkEngine/libs','D:/VulkanSDK/Lib']
cpppath = ['./include','./vkEngine/include','D:/VulkanSDK/Include','./imgui']
cppdefines = ['NDEBUG']
commonsrc = Glob('./imgui/*.cpp') + ['./imgui/backends/imgui_impl_vulkan.cpp','./imgui/backends/imgui_impl_glfw.cpp','./src/editor.cpp']


Export({"dlldst":dlldst,"reldir":reldir,"env":env})

SConscript(['./vkEngine/SConscript'])

targetinfo = [
    ["out/test", ["./tests/test.cpp" ]],
]

for info in targetinfo:
    env.Program(info[0],
        info[1]+commonsrc,
        LIBS=libs, LIBPATH=libpath, CPPPATH=cpppath, CPPDEFINES=cppdefines,
        SCONS_CXX_STANDARD="c++17")