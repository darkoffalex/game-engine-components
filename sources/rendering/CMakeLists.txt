# Добавить проект (исполняемый файл)
add_executable("Rendering" main.cpp)

# Конфигурация и флаги по умолчанию
add_default_configurations("Rendering" "rendering")

# Связка с нужными библиотеками
target_link_libraries("Rendering" PRIVATE glfw)