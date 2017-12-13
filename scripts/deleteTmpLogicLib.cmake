# This script is responsible for deleting the exsting temp_logic.dll after
# everything has been built.
file(TO_NATIVE_PATH ${DELETE_LIB_NAME} REAL_LIBPATH)
message(STATUS "Deleting ${REAL_LIBPATH}...")
file(REMOVE ${REAL_LIBPATH})
