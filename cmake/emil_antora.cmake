function(emil_generate_antora_docs_target playbook)
    add_custom_target(generate_antora_docs
        COMMAND antora --cache-dir=.cache --stacktrace --fetch --generator=antora-site-generator-lunr ${playbook}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endfunction()
