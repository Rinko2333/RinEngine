qt_add_executable(appRinEngine main.cpp)

target_link_libraries(appRinEngine PRIVATE RinEngineCore Qt6::QuickControls2 Qt6::MultimediaWidgets)

qt_add_qml_module(appRinEngine
    URI RinEngine
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/layers/LayerStack.qml
        qml/screens/TitleScreen.qml
        qml/widgets/BackgroundLayer.qml
        qml/widgets/CharacterLayer.qml
        qml/widgets/CharacterSlot.qml
        qml/widgets/ChoiceBox.qml
        qml/widgets/DialogueBox.qml
        qml/widgets/HistoryScreen.qml
        qml/widgets/MenuButton.qml
        qml/widgets/SaveLoadScreen.qml
        qml/widgets/SettingsScreen.qml
        qml/widgets/SystemMenu.qml
        qml/effects/AnimationPlayer.qml
        qml/widgets/VideoPlayer.qml
)

set_target_properties(appRinEngine PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.rinengine.app
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
