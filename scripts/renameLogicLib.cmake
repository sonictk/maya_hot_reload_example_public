# This script is responsible for renaming the exsting logic.dll file,
# thus allowing the plugin to be built normally.
if(EXISTS ${LOGIC_LIB_NAME})
    # string(RANDOM RANDOM_NUMBER)
    set(NEW_LOGIC_LIB_NAME "${LOGIC_LIB_NAME}.temp")
    message(STATUS "Renaming ${LOGIC_LIB_NAME} to ${NEW_LOGIC_LIB_NAME}")
    file(RENAME ${LOGIC_LIB_NAME} ${NEW_LOGIC_LIB_NAME})
endif()
