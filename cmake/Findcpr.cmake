include(FetchContent)

FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.10.5
    GIT_SHALLOW YES)

FetchContent_MakeAvailable(cpr)
