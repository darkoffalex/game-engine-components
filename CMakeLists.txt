# Версия CMAke
cmake_minimum_required(VERSION 3.5)

# Включить управление типом связывания runtime-библиотеки для MSVC
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

# Название проекта (решения)
project(GameEngineComponents)

# Стандарт С/С++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Найти нужные пакеты
find_package(glfw3 REQUIRED)

# Устанавливаем каталоги для бинарников
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Статическое связывание для runtime библиотеками (размер итоговых файлов будет больше)
set(STATIC_RUNTIME OFF)

# Определить архитектуру/разрядность
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(ARCH_NAME "x86")
    set(ARCH "32")
else()
    set(ARCH_NAME "x64")
    set(ARCH "64")
endif()

# Статическая компоновка runtime для MinGW/GCC
# Для WIN32 варианта (MinGW) достаточно включить CXX флаг -static
# Для Linux (GCC) более стабильным будет явно задать флаги связывания стандартных библиотек
if(STATIC_RUNTIME AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
    else()
        set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ ${CMAKE_CXX_STANDARD_LIBRARIES}")
    endif()
endif()

# Вспомогательная функция для конфигурации целей сборки (target)
# Функция также добавляет необходимые флаги целям сборки, может быть вызвана в под-проектах
function(add_default_configurations TARGET_NAME TARGET_BIN_NAME)
    # Меняем название запускаемого файла в зависимости от типа сборки
    set_property(TARGET ${TARGET_NAME} PROPERTY OUTPUT_NAME "${TARGET_BIN_NAME}$<$<CONFIG:Debug>:_d>_${ARCH_NAME}")

    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # Отключение стандартных min-max функций (для MSVC) + отключить предупреждение 4250 для ромбовидного наследования
        target_compile_definitions(${TARGET_NAME} PRIVATE "-DNOMINMAX /wd4250")
        # Установить уровень предупреждений 3 (для MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE /W3 /permissive-)
        # Статическое связывание runtime библиотек (для MSVC)
        if(STATIC_RUNTIME)
            set_property(TARGET ${TARGET_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        endif()
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # Установить максимальный уровень предупреждений (-Wall -Wextra -pedantic) и включить быструю математику (ffast-math)
        target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -pedantic -ffast-math -Wno-unknown-pragmas)
    endif()
endfunction()

# Добавить под-проекты
add_subdirectory(sources/ecs)
add_subdirectory(sources/rendering)
