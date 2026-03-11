add_executable(pisca-led-tres-botoes
        main.c
)

target_link_libraries(pisca-led-tres-botoes pico_stdlib hardware_adc)
pico_add_extra_outputs(pisca-led-tres-botoes)
