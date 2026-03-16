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
    Lista1/Ex1
    Lista1/Ex2
    Lista1/Ex6
    Lista1/Ex7
    Lista1/Ex8
    Lista1/Ex9
    Lista2/Ex2-1
)

# Se formos usar classes separadas, adicionar aqui
add_library(cube
    src/Lista1/Cube/Cube.cpp
)

target_include_directories(cube PUBLIC
    src/Lista1
    src/Lista2
)

target_link_libraries(cube PUBLIC glfw)

# Camera
add_library(camera
    src/Lista1/Camera/Camera.cpp
)

target_include_directories(camera PUBLIC
    src/Lista1
    src/Lista2
)

target_link_libraries(camera PUBLIC glfw)


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
        camera
        glfw
        glm
        OpenGL::GL
    )

endforeach()

add_compile_options(-Wno-pragmas)