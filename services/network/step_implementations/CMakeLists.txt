add_library(services.network_step_implementations SHARED)

target_link_libraries(services.network_step_implementations PRIVATE
    services.cucumber
    services.network
    services.network_instantiations
)

target_sources(services.network_step_implementations PRIVATE
    HttpSteps.cpp
)
