add_library(dinamaite-rt
    textinstrumentation.c
    dinamite_time.c
    #binaryinstrumentation.c
    #nullinstrumentation.c
)

add_library(dinamaite-rt-lib MODULE
    textinstrumentation.c
    dinamite_time.c
    #binaryinstrumentation.c
    #nullinstrumentation.c
)
