@REM  # In debug build, we want to prebuild memcheck redirecter
@REM  # before running the tests
if "%CONFIGURATION%"=="Debug" (
  cmake -Hmisc -Bbuild-misc -A%PLATFORM%
  cmake --build build-misc
  cmake -H. -BBuild -A%PLATFORM% -DMEMORYCHECK_COMMAND=build-misc\Debug\CoverageHelper.exe -DMEMORYCHECK_COMMAND_OPTIONS=--sep-- -DMEMORYCHECK_TYPE=Valgrind
)
if "%CONFIGURATION%"=="Release" (
  cmake -H. -BBuild -A%PLATFORM%
)
