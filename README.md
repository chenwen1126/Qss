# Qss
Qt Style Sheets Example With Custom Title Bar!!!
# How to use
step 1: add Qss.h,Qss.cpp,qss.qrc to your project.<br>
step 2: add code like this:	<br>
```C++
QFile file(":/qss/css/qss.css");
if (!file.open(QIODevice::ReadOnly))
	exit(0);

QTextStream in(&file);
QString css = in.readAll();
qApp->setStyleSheet(css);
 ```
 to you main function.<br>
 step3: your UI class inherit from QssDialog, QssMainWindow provided by Qss.
 # Screenshot
 QssDemo Screenshot like this:<br>
 ![](https://github.com/chenwen1126/Qss/blob/master/ScreenShot/01.png)

