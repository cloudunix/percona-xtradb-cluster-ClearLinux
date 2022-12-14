# TokuDB only supports x86-64 and cmake-2.8.9+
IF(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" AND
    NOT CMAKE_VERSION VERSION_LESS "2.8.9" AND
   NOT WITHOUT_TOKUDB AND NOT WITHOUT_TOKUDB_STORAGE_ENGINE)
CHECK_CXX_SOURCE_COMPILES(
"
struct a {int b; int c; };
struct a d = { .b=1, .c=2 };
int main() { return 0; }
" TOKUDB_OK)
ENDIF()

IF (NOT DEFINED WITH_TOKUDB_BACKUP_PLUGIN)
  IF (WITHOUT_TOKUDB_BACKUP_PLUGIN)
    MESSAGE(STATUS "Skipping tokudb-backup-plugin")
    RETURN()
  ENDIF ()
ENDIF ()

include(CheckSymbolExists)
check_symbol_exists(O_DIRECT "fcntl.h" HAVE_O_DIRECT)
check_symbol_exists(CLOCK_REALTIME "time.h" HAVE_CLOCK_MONOTONIC)

IF(NOT TOKUDB_OK OR NOT HAVE_O_DIRECT OR NOT HAVE_CLOCK_MONOTONIC OR APPLE)
  MESSAGE(STATUS "Not building tokudb-backup-plugin")
  RETURN()
ENDIF()



# Suppress warnings for all compilers
remove_compile_flags(-Wvla -Wextra-semi)


# Suppress warnings for all clang versions
IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  append_cflags_if_supported(-Wno-deprecated-dynamic-exception-spec)
  remove_compile_flags(-Wconditional-uninitialized)
ENDIF()



# compiler_options.cmake sets "-D_FORTIFY_SOURCE=2" for clang what causes
# "error: redefinition of 'read'" for "Percona-TokuBackup/backup/backup.cc"
IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  STRING (REPLACE "-D_FORTIFY_SOURCE=2" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
ENDIF()

IF(DEFINED TOKUDB_BACKUP_PLUGIN_VERSION)
  ADD_DEFINITIONS("-DTOKUDB_BACKUP_PLUGIN_VERSION=${TOKUDB_BACKUP_PLUGIN_VERSION}")
  IF (${TOKUDB_BACKUP_PLUGIN_VERSION} MATCHES "^tokudb-backup-([0-9]+)\\.([0-9]+)")
    ADD_DEFINITIONS("-DTOKUDB_BACKUP_PLUGIN_VERSION_MAJOR=${CMAKE_MATCH_1}")
    ADD_DEFINITIONS("-DTOKUDB_BACKUP_PLUGIN_VERSION_MINOR=${CMAKE_MATCH_2}")
  ENDIF()
ENDIF()
SET(TOKUDB_BACKUP_SOURCES tokudb_backup.cc)
IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Percona-TokuBackup/backup/CMakeLists.txt")
  ADD_SUBDIRECTORY(Percona-TokuBackup/backup)
  INCLUDE_DIRECTORIES(Percona-TokuBackup/backup)
  MESSAGE(STATUS "tokudb-backup-plugin include backup ${HOT_BACKUP_LIBS}")
  set(HOT_BACKUP_LIBS ${HOT_BACKUP_LIBS} PARENT_SCOPE) # export HOT_BACKUP_LIBS to parent
ELSE()
  MESSAGE(STATUS "tokudb-backup-plugin no backup ${CMAKE_CURRENT_SOURCE_DIR}")
ENDIF()
MYSQL_ADD_PLUGIN(tokudb_backup ${TOKUDB_BACKUP_SOURCES} MODULE_ONLY MODULE_OUTPUT_NAME "tokudb_backup" LINK_LIBRARIES stdc++)
#
# RPM installs documentation directly from the source tree
#
IF(NOT INSTALL_LAYOUT MATCHES "RPM")
  INSTALL(FILES README_tokudb_backup DESTINATION ${INSTALL_DOCDIR})
ENDIF()
