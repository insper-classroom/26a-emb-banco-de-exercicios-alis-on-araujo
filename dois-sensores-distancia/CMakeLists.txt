add_executable(dois-sensores-distancia
        main.c
)

target_link_libraries(dois-sensores-distancia
                     pico_stdlib 
                     hardware_pwm
                     hardware_rtc
                 )
pico_add_extra_outputs(dois-sensores-distancia)
