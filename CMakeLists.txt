cmake_minimum_required(VERSION 3.11...4.0)
project(CGA20261)

# Define o padrão do C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Ativa o FetchContent
include(FetchContent)

# Configura o FetchContent para baixar e compilar GLFW automaticamente
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
  # Define a versão desejada da GLFW
)

# Faz o download e compila as bibliotecas
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# stb_image
FetchContent_Declare(
  stb_image
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG master
)
FetchContent_MakeAvailable(stb_image)

# Adiciona as pastas de cabeçalhos
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/common)
include_directories(${CMAKE_SOURCE_DIR}/include/glad)
include_directories(${glm_SOURCE_DIR})
include_directories(${stb_image_SOURCE_DIR})

# Lista de exemplos/exercícios podem ser colocados aqui também
set(EXERCISES
    Lista2/Ex2-1
    Lista2/Ex2-2
    Lista2/Ex2-6
    Lista2/Ex2-7
    Lista2/Ex2-8
    Lista2/Ex2-9
    Lista3/Ex3-1
    Lista3/Ex3-2
    Lista3/Ex3-3
    Lista3/Ex3-4
    Lista3/Ex3-5
    Lista6/Parte1
    Lista7/Ex7
)

# Se formos usar classes separadas, adicionar aqui
add_library(cube
    src/Lista2/Cube/Cube.cpp
)

target_include_directories(cube PUBLIC
    src/Lista2
    src/Lista3
    src/Lista6
)

target_link_libraries(cube PUBLIC glfw)

# Camera
add_library(camera
    src/Lista2/Camera/Camera.cpp
)

target_include_directories(camera PUBLIC
    src/Lista2
    src/Lista3
    src/Lista6
)

target_link_libraries(camera PUBLIC glfw)

# Triângulo
add_library(triangle
    src/Lista2/Triangle/Triangle.cpp
)

target_include_directories(triangle PUBLIC
    src/Lista2
    src/Lista3
    src/Lista6
)

target_link_libraries(triangle PUBLIC glfw)

# Raio
add_library(ray
    src/Lista2/Ray/Ray.cpp
)

target_include_directories(ray PUBLIC
    src/Lista2
    src/Lista3
    src/Lista6
)

target_link_libraries(ray PUBLIC glfw)

# Esfera
add_library(sphere
    src/Lista2/Sphere/Sphere.cpp
)

target_include_directories(sphere PUBLIC
    src/Lista2
    src/Lista3
    src/Lista6
    src/Lista7
)

target_link_libraries(sphere PUBLIC glfw)


find_package(OpenGL REQUIRED)

# Caminho esperado para a GLAD
set(GLAD_C_FILE "${CMAKE_SOURCE_DIR}/common/glad.c")

# Verifica se os arquivos da GLAD estão no lugar
if (NOT EXISTS ${GLAD_C_FILE})
    message(FATAL_ERROR "Arquivo glad.c não encontrado!")
endif()

# Cria os executáveis
foreach(EXERCISE ${EXERCISES})
    # Extrai o nome do arquivo sem o diretório para o executável
    get_filename_component(EXE_NAME ${EXERCISE} NAME)
    
    # Adiciona o executável usando o nome do arquivo como nome do executável
    add_executable(${EXE_NAME}
        src/${EXERCISE}.cpp
        ${GLAD_C_FILE}
    )

    target_link_libraries(${EXE_NAME}
        cube
        triangle
        camera
        ray
        sphere
        glfw
        glm
        OpenGL::GL
    )

endforeach()

add_compile_options(-Wno-pragmas)