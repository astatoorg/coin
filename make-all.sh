export PATH=/bin:/usr/bin:../mxe:../mxe/usr/bin:$PATH
make -f qt-win
cd release
upx -9 astato-qt.exe
cd ..
make -f qt-linux
#cp astato-qt release/astato-qt
tar -czvf release/astato-qt.tar.gz astato-qt
cp astato-qt ../astato-qt
rm astato-qt
cd src
make -f makefile.unix
#cp astatod ../release/astatod
tar -czvf ../release/astatod.tar.gz astatod
rm astatod
cd ..
rm release/qrc_bitcoin.cpp



