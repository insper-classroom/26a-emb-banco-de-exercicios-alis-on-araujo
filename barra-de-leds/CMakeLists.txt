add_executable(barra-de-leds
        main.c
)

target_link_libraries(barra-de-leds pico_stdlib hardware_adc)
pico_add_extra_outputs(barra-de-leds)
