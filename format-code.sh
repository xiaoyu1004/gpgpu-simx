# This bash script formats GPGPU-Sim using clang-format
THIS_DIR="$( cd "$( dirname "$BASH_SOURCE" )" && pwd )"
clang-format -i ${THIS_DIR}/config/*.h
# clang-format -i ${THIS_DIR}/config/*.cpp
# clang-format -i ${THIS_DIR}/driver/*.h
clang-format -i ${THIS_DIR}/driver/*.cpp
clang-format -i ${THIS_DIR}/driver/common/*.h
clang-format -i ${THIS_DIR}/driver/common/*.cpp
clang-format -i ${THIS_DIR}/driver/include/*.h
clang-format -i ${THIS_DIR}/runtime/include/*.h
# clang-format -i ${THIS_DIR}/runtime/include/*.cpp
clang-format -i ${THIS_DIR}/runtime/src/*.h
# clang-format -i ${THIS_DIR}/runtime/src/*.c
clang-format -i ${THIS_DIR}/simx/*.h
clang-format -i ${THIS_DIR}/simx/*.cpp
clang-format -i ${THIS_DIR}/simx/common/*.h
clang-format -i ${THIS_DIR}/simx/common/*.cpp
# clang-format -i ${THIS_DIR}/tests/*.h
# clang-format -i ${THIS_DIR}/tests/*.cpp
clang-format -i ${THIS_DIR}/tests/basic/*.h
clang-format -i ${THIS_DIR}/tests/basic/*.cpp
clang-format -i ${THIS_DIR}/tests/demo/*.h
clang-format -i ${THIS_DIR}/tests/demo/*.cpp
