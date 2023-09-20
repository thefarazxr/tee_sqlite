project (optee_tee_sqlite C)

set (SRC host/main.c host/sqlite3.c host/demovfs.c)

add_executable (${PROJECT_NAME} ${SRC})

target_include_directories(${PROJECT_NAME}
			   PRIVATE ta/include
			   PRIVATE include)
target_compile_options(${PROJECT_NAME} PRIVATE -DSQLITE_OMIT_LOCALTIME=1 -DSQLITE_OS_OTHER=1 -DSQLITE_THREADSAFE=0 -DSQLITE_TEMP_STORE=3)
target_link_libraries (${PROJECT_NAME} PRIVATE teec)


install (TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
