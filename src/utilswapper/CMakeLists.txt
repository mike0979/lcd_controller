aux_source_directory(. utilswapper_src)
include_directories(/usr/local/TVSServer/include)
link_directories(/usr/local/TVSServer/lib)
add_library(utilswapper SHARED ${utilswapper_src})
target_link_libraries(utilswapper curlwapper)