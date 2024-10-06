# Добавить проект (исполняемый файл)
add_executable("Rendering"
        main.cpp
        glad.cpp

        gui/imgui_impl_glfw.cpp
        gui/imgui_impl_opengl3.cpp

        scenes/scene.h
        scenes/01-triangle/triangle.h
        scenes/01-triangle/triangle.cpp
        scenes/02-uniforms/uniforms.h
        scenes/02-uniforms/uniforms.cpp
        scenes/03-textures/textures.h
        scenes/03-textures/textures.cpp
        scenes/04-perspective/perspective.h
        scenes/04-perspective/perspective.cpp
        scenes/05-passes/passes.h
        scenes/05-passes/passes.cpp
        scenes/06-lighting/lighting.h
        scenes/06-lighting/lighting.cpp
)

# Конфигурация и флаги по умолчанию
add_default_configurations("Rendering" "rendering")

# Связка с нужными библиотеками
target_link_libraries("Rendering" PRIVATE
        glfw
        imgui::imgui
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