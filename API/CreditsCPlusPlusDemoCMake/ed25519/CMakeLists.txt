cmake_minimum_required (VERSION 2.6)

project (ed25519)

file(GLOB SRC
    src/*.h
    src/*.c
)

add_library(${PROJECT_NAME} ${SRC})