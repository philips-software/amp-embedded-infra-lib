add_custom_target(embedded_infra_lib.documents
    COMMAND docker run --rm --entrypoint=sphinx-build -v ${CMAKE_CURRENT_SOURCE_DIR}:/source:ro -v ${CMAKE_CURRENT_BINARY_DIR}:/dest registry.it4rnd.philips.com/public_base_images/spirit/jenkins-swarm-client-sphinx-doc -b html /source /dest
    DEPENDS conf.py index.rst
    SOURCES index.rst
)
