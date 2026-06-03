add_library(RinEngineCore STATIC
    Logger.h
    Logger.cpp
    SettingsManager.h
    SettingsManager.cpp
    ResourceManager.h
    ResourceManager.cpp
    VariableManager.h
    VariableManager.cpp
    ScriptParser.h
    ScriptParser.cpp
    ScriptRunner.h
    ScriptRunner.cpp
    GameStateManager.h
    GameStateManager.cpp
)

target_include_directories(RinEngineCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(RinEngineCore PUBLIC Qt6::Quick Qt6::Concurrent)
