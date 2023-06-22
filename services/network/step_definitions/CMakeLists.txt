add_library(services.network_step_definitions SHARED)
emil_build_for(services.network_step_definitions HOST All)

target_link_libraries(services.network_step_definitions PRIVATE
    services.cucumber
    services.network
    services.network_instantiations
)

target_sources(services.network_step_definitions PRIVATE
    HttpSteps.cpp
)
