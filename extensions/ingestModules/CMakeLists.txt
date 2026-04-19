
add_subdirectory(txtCounterModule)
add_subdirectory(detectMIMEType)

find_package(Qt6 REQUIRED COMPONENTS Core)

add_executable(run_ingest_plugin
	run_plugin.cpp
)

target_include_directories(run_ingest_plugin
	PRIVATE
		${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(run_ingest_plugin
	PRIVATE
		Qt6::Core
)

target_sources(run_ingest_plugin PRIVATE
	${CMAKE_SOURCE_DIR}/src/extensions/ingestModules/ingestModuleFactory.cpp
)

target_sources(run_ingest_plugin PRIVATE
	${CMAKE_SOURCE_DIR}/src/core/logging/logger.cpp
)

set_target_properties(run_ingest_plugin PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Desktop-Debug
)
