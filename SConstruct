import os

dlldst = '../out/'
reldir = '\\"./vkEngine\\"'

env = Environment(CC = 'cl',
                   CCFLAGS = ['/std:c++17','/EHsc','/O2'])

def GetFileWithExt(dir,ext):
    ret = []
    for _,dir,files in os.walk(dir):
        for filename in files:
            _,suf = os.path.splitext(filename)
            if suf == ext:
                ret.append(filename)
    return ret

selflibs = GetFileWithExt("./vkEngine/libs",".lib")

libs = ['msvcrtd', 'libcmt', 'Gdi32', 'shell32', 'user32', 'ole32', 'vulkan-1', 'vkengine', 'nfd'] + selflibs
libpath = ['./libs','./vkEngine/libs','D:/VulkanSDK/Lib']
cpppath = ['./include','./vkEngine/include','D:/VulkanSDK/Include','./imgui','./vkEngine/include/physx/']
cppdefines = ['NDEBUG']
commonsrc = Glob('./imgui/*.cpp') + ['./imgui/backends/imgui_impl_vulkan.cpp','./imgui/backends/imgui_impl_glfw.cpp','./src/editor.cpp','./src/ui_render.cpp','./src/component_ui.cpp']


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