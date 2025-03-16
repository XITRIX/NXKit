project(NXKit)

add_library(NXKit
        lib/NXControl.cpp
        lib/NXNavigationController.cpp
        lib/NXSeparatorView.cpp
        lib/NXTabBarController.cpp
)

target_include_directories(NXKit PUBLIC
        include
        lib
)

target_link_libraries(NXKit PUBLIC UIKit)