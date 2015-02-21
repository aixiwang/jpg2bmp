# you can use ar cru libjpeg.a *.o to ar all .o to static a
cd jpeg-9a
make all
ar cru ../libjpeg.a *.o
cd ..

g++ -c jpg2bmp.cpp -o jpg2bmp.o
g++ jpg2bmp.o libjpeg.a -o jpg2bmp
ls jpg2bmp -l
cp jpg2bmp ..

