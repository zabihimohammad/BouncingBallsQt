cmake_minimum_required(VERSION 3.16)
project(BouncingBalls LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets Multimedia Network OpenGLWidgets REQUIRED)

set(SOURCES
    src/main.cpp
    src/core/Ball.cpp
    src/core/GridManager.cpp
    src/core/ScoreManager.cpp
    src/core/SoundManager.cpp
    src/gameplay/GameScene.cpp
    src/gameplay/GameView.cpp
    src/gameplay/CannonItem.cpp
    src/gameplay/BallItem.cpp
    src/gameplay/AimLineItem.cpp
    src/ui/MainWindow.cpp
    src/ui/MainMenuWidget.cpp
    src/ui/StartGameWidget.cpp
    src/ui/SettingsWidget.cpp
    src/ui/ScoreboardWidget.cpp
    src/ui/PauseDialog.cpp
    src/ui/GameOverDialog.cpp
    src/ui/waterbackgroundwidget.cpp
    src/ui/poolballswidget.cpp
    src/ui/AdvancedSettingsWidget.cpp
    src/ui/HelpWidget.cpp
    src/ui/LevelSelectWidget.cpp
    src/ui/DifficultyDialog.cpp
)

set(HEADERS
    src/core/Ball.h
    src/core/GridManager.h
    src/core/ScoreManager.h
    src/core/SoundManager.h
    src/gameplay/GameScene.h
    src/gameplay/GameView.h
    src/gameplay/CannonItem.h
    src/gameplay/BallItem.h
    src/gameplay/AimLineItem.h
    src/ui/MainWindow.h
    src/ui/MainMenuWidget.h
    src/ui/StartGameWidget.h
    src/ui/SettingsWidget.h
    src/ui/ScoreboardWidget.h
    src/ui/PauseDialog.h
    src/ui/GameOverDialog.h
    src/ui/waterbackgroundwidget.h
    src/ui/poolballswidget.h
    src/ui/AdvancedSettingsWidget.h
    src/ui/HelpWidget.h
    src/ui/LevelSelectWidget.h
    src/ui/ThemeManager.h
    src/ui/DifficultyDialog.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Multimedia Qt6::Network Qt6::OpenGLWidgets)
