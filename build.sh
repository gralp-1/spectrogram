#! /bin/sh

FLAGS="-Wall -Wextra -Werror"
# if the the first command line option is "debug" then add the fsanitize flag
if [ "$1" = "debug" ]; then
    FLAGS="$FLAGS -g -fsanitize=address"
fi
RAYLIB=`pkg-config --cflags --libs raylib`
OSX_OPT="-Llib/ -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL"
gcc $FLAGS -o complex ./src/main.c $OSX_OPT $RAYLIB
