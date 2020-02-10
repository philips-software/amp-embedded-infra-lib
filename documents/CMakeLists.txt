add_custom_target(embedded_infra_lib.documents
    COMMAND sphinx-build -b html ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/documents
    DEPENDS conf.py index.rst
    SOURCES index.rst
)
