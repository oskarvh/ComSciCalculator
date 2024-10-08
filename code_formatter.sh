#!/bin/bash

# Run formatter
find software/comSciCalc_lib/ -iname *.c -o -iname *.h | xargs clang-format -i
clang-format -i software/display/*.c
clang-format -i software/display/*.h
clang-format -i software/display/fonts/font_library.c
clang-format -i software/display/fonts/font_library.h
find software/unit_tests/ -iname *.c -o -iname *.h | xargs clang-format -i
find firmware/rp2040/hardware/ -iname *.c -o -iname *.h | xargs clang-format -i
find firmware/rp2040/inc/ -iname *.c -o -iname *.h | xargs clang-format -i
find firmware/rp2040/src/ -iname *.c -o -iname *.h | xargs clang-format -i
find firmware/common/ -iname *.c -o -iname *.h | xargs clang-format -i