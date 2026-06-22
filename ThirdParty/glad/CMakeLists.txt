# glad —— 本地生成的 OpenGL 4.6 Core 加载器（python -m glad --api=gl:core=4.6 c）
add_library(glad STATIC src/gl.c)
target_include_directories(glad PUBLIC include)
