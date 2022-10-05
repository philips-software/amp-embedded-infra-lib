add_executable(examples.http_server)
target_sources(examples.http_server PRIVATE TimeHttpPage.cpp TimeHttpPage.hpp)
target_link_libraries(examples.http_server PUBLIC hal.generic)

if (EMIL_BUILD_WIN)
    target_sources(examples.http_server PRIVATE MainWin.cpp)
    target_link_libraries(examples.http_server PUBLIC services.network_win)
endif()

if (EMIL_BUILD_UNIX OR EMIL_BUILD_DARWIN)
    target_sources(examples.http_server PRIVATE MainUnix.cpp)
    target_link_libraries(examples.http_server PUBLIC services.network_bsd)
endif()
