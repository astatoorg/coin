#export PATH=/bin:/usr/bin:../mxe:../mxe/usr/bin:$PATH
make -f qt-win
cd release
upx -9 astato-qt.exe
cd ..
make -f qt-linux
cp astato-qt release/astato-qt
cd src
make -f makefile.unix
cp astatod ../release/astatod
rm astatod
cd ..


