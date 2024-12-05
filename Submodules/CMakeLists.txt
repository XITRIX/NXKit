project(GLAD)

add_library(GLAD
        glad/src/glad.c
)

target_include_directories(GLAD PUBLIC
        glad/include
)
