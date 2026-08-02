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

target_link_libraries(NXKit PUBLIC XITRIXUIKit)

option(NXKIT_BUILD_TESTS "Build NXKit regression tests" OFF)
if (NXKIT_BUILD_TESTS)
    enable_testing()
    add_executable(NXKitTabBarControllerTests
            tests/NXTabBarControllerTests.cpp
    )
    target_link_libraries(NXKitTabBarControllerTests PRIVATE NXKit)
    add_test(NAME NXKitTabBarControllerTests COMMAND NXKitTabBarControllerTests)
endif ()
