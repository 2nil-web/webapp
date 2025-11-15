## WebView Application template  
*A tool to create standalone applications based on web technology.*  
This application uses the : [webview library](https://github.com/webview/webview.git).  
1. For those who want to try the tool, a tutorial is available in [webapp_tutorial.md](webapp_tutorial.md).  
2. And to dive a little bit deeper into it, a user documentation and reference is available with the file '[webapp_quick_user_doc_and_reference.md](webapp_quick_user_doc_and_reference.md)'. It basically repeats what is already indicated with the -h/--help and -u options and also what you can test in the folder 'examples/01-test_bed' coming with the source package.  
3. The package release with sources and binaries(for Windows and Linux) is available [there](https://github.com/2nil-web/webapp/releases)
4. Prerequisites to build the app are:  
   - git to download the webview library.  
   - make and g++ to build the webview lib and the app under linux, Windows (MSys2) and eventually MacOS (this last one not tested yet).  
   - Or Visual Studio under Windows.  
   - WebView2 under Windows  
   - gtk+-3.0 webkit2gtk-4.1 webkit2gtk-web-extension-4.1 gtkmm-3.0 under Debian.  
   - gtk4 webkitgtk-6.0 gtkmm-4.0 for Ubuntu, Arch and Fedora (compile but does not work ...)  
   - lsb_release  
5. A Makefile is provided that allow the application to be built under the gcc/g++ compiler suite.  
6. An .sln/.vcxproj pair of files is provided that allow the application to be built under the MS Visual Studio compiler suite.  
7. The application is written in C++.  
   - It is (*roughly*) tested under Windows and Linux(mainly Debian), but the binary produced under Debian also works under Fedora, Arch and Ubuntu.  
   - It remains to be tested under MacOS although it should work (or at least compile ...), as stated by the webview library documentation ...  
   - An svg icon is provided that can be used to create a .ico file with magick convert. Or provide your own .ico file.  
   - C++ files are formatted using clang-format  
   - HTML/CSS/JS files are formatted using js-beautify (from nodejs).  
   - The resulting binary size  can be reduced with upx (make upx).  
8. Build  
   - To set the webview library run the "scripts/update_webviews.sh" script under bash. And try to follow the script directives.  
   - Then run make under Windows/MSys2 or Linux.  
   - Or build the webapp.sln under MS Visual Studio.
