## WebView Application template

Application using the : [webview library](https://github.com/webview/webview.git).

1) Prerequisites to build the app are:
   
   - git to download the webview library.
   
   - make and g++ to build the webview lib and the app under linux, Windows and eventually MacOS.
   
   - Or Visual Studio under Windows.
   
   - WebView2 under Windows
   
   - gtk+-3.0 webkit2gtk-4.1 webkit2gtk-web-extension-4.1 gtkmm-3.0 under Debian and RedHat (AlmaLinux in fact to test)
   
   - gtk4 webkitgtk-6.0 gtkmm-4.0 for Ubuntu, Arch and Fedora
   
   - lsb_release

2) A Makefile is provided that allow the application to be built under the gcc/g++ compiler suite.

3) An .sln/.vcxproj pair of files is provided that allow the application to be built under the MS Visual Studio compiler suite.

4) The application is written in C++.
   
   - It roughly tested under Windows and Linux.
   - It remain to be tested under MacOS although it should works, as stated by the webview library documentation ...
   - An svg icon is provided that can be used to create a .ico file with magick convert. Or provide your own .ico file.
   - The resulting binary size  can be reduced with upx (make upx).

5) Built
   
   - To set the webview library run the "initlib.sh" script under bash or the "initlib.bat" script under Windows cmd.
   
   - Then run make under gcc.
   
   - Or build the webapp.sln under MS Visual Studio.
   
   - And from time to time check the wv_ghrepo/webview/ directory and aligne webview/ directory with it
6. libcurl (optional)
* Download
  
  * git clone https://github.com/curl/curl.git

* Static build under visual studio "x64 Native Tools Command Prompt for VS 2022"
  
  * rename src/tool_hugehelp.c.cvs to src/tool_hugehelp.c
  
  * cd curl\winbuild
  
  * nmake /f Makefile.vc mode=static
  
  * copy ..\builds\libcurl-vc-x64-release-static-ipv6-sspi-schannel where you need it.

ToDo :

Faire des tests avec Dom Examples : [GitHub - mdn/dom-examples: Code examples that accompany various MDN DOM and Web API documentation pages](https://github.com/mdn/dom-examples.git)
