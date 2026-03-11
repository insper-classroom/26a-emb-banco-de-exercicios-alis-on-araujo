add_executable(senha
        main.c
)

target_link_libraries(senha pico_stdlib hardware_adc)
pico_add_extra_outputs(senha)
