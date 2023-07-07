#!/usr/bin/fish
#
gcc --shared -fPIC http-parser.c -lpicohttpparser -lcgab -o libcgabhttp-parser.so

if [ -e libcgabhttp-parser.so ]
    echo "libcgabhttp-parser.so created"
else
    echo "libcgabhttp-parser.so not created"
end

if [ "$argv[1]" = "install" ]
    sudo cp libcgabhttp-parser.so /usr/lib/
    echo "libcgabhttp-parser installed"
end
