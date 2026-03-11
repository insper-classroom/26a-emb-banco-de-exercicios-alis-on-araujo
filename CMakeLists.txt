# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.2.0)

set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if(PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
  message(
    FATAL_ERROR
      "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}"
  )
endif()

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})
pico_sdk_init()
include(pico_sdk_import.cmake)

add_compile_options(
  -Wall -Wno-format # int != int32_t as far as the compiler is concerned because
                    # gcc has int32_t as long int
  -Wno-unused-function # we have some for the docs that aren't called
)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-Wno-maybe-uninitialized)
endif()

# Hardware-specific examples in subdirectories:
add_subdirectory(pisca-led-adc)
add_subdirectory(pisca-led-dois-botoes)
add_subdirectory(pisca-led-tempo-botao)
add_subdirectory(pisca-led-tres-botoes)
add_subdirectory(pisca-led-um-botao)
add_subdirectory(pisca-led-um-depois-o-outro)
add_subdirectory(senha)
add_subdirectory(seven-seg)
add_subdirectory(teclado-matricial)
add_subdirectory(barra-de-leds)
add_subdirectory(buzzer-com-timer)
add_subdirectory(dois-sensores-distancia)
add_subdirectory(jogo-reflexo)
