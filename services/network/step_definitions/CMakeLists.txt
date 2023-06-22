add_library(services.network_step_definitions SHARED)

target_link_libraries(services.network_step_definitions PRIVATE
    services.cucumber
    services.network
    services.network_instantiations
)

target_sources(services.network_step_definitions PRIVATE
    HttpSteps.cpp
)
