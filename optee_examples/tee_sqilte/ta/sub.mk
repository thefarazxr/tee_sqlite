global-incdirs-y += include
# srcs-y += codec.c
# srcs-y += codecext.c
srcs-y += tee_sqlite.c
srcs-y += sqlite3.c
srcs-y += tee_vfs.c
srcs-y += hashmap.c
cflags-sqlite3.c-y += -DSQLITE_OS_OTHER=1
cflags-sqlite3.c-y += -DSQLITE_OMIT_LOCALTIME=1
cflags-sqlite3.c-y += -DSQLITE_THREADSAFE=0
# cflags-sqlite3.c-y += -DSQLITE_HAS_CODEC=1
cflags-sqlite3.c-y += -DSQLITE_TEMP_STORE=3
# cflags-hello_world_ta.c-y += -DSQLITE_OS_OTHER=1
# CFLAGS += -DTESTFLAG=1
cflags-hello_world_ta.c-y += -DTESTFLAG=1
# To remove a certain compiler flag, add a line like this
#cflags-template_ta.c-y += -Wno-strict-prototypes
