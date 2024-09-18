# Добавить проект (исполняемый файл)
add_executable("Rendering"
        main.cpp
        glad.cpp

        scenes/base.h
        scenes/triangle.h
        scenes/triangle.cpp
        scenes/uniforms.h
        scenes/uniforms.cpp
        scenes/textures.h
        scenes/textures.cpp
)

# Конфигурация и флаги по умолчанию
add_default_configurations("Rendering" "rendering")

# Связка с нужными библиотеками
target_link_libraries("Rendering" PRIVATE
        glfw
        nuklear::nuklear
        opengl::opengl
        glm::glm
        stb::stb)

# Копирование библиотек
foreach (DIR ${glfw_BIN_DIRS_DEBUG} ${glfw_BIN_DIRS_RELEASE})
    file(GLOB DLLS ${DIR}/*.dll)
    foreach(DLLFile IN LISTS DLLS)
        add_custom_command(
                TARGET "Rendering" POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLLFile} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
                COMMENT "Copying shared library: ${DLLFile}")
    endforeach()
endforeach()