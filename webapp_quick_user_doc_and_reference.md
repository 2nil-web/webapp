# A quick user documentation for webapp

### 1. <ins>**The -h option output**</ins>

   **webapp 0.1.1-first-public-release**  
   A tool to create standalone applications based on web technology.  
   <ins>Usage:</ins> webapp [OPTIONS] ARGUMENT  
   <ins>Available options</ins>  
   <code>-h Print this message and eventually exit.
   -v Display version information and eventually exit.
   -b Provide additional browser arguments to the webview2 component.
   -c Provide an html string that will be directly set to the webview.
   -f Provide a url (a remote one must prepended with 'http://').
   -p Provide the path where could be found a file with a name within the following ones: webview.min.html, webview.html, index.min.html, index.html, index.min.js, index.js.
    The use of these 3 previous options is mutually exclusive.
    It is also possible to directly provide their argument as the last one of the command (hence witout the option), but prepended with "html://", for -c.
   -t Set the title of the webview windows, default is to display the url as title if it is provided or nothing if just an html string is provided.
   -j Inject a javascript command before loading html page.
   -r Run the provided javascript command and exits.
   -d Activate the developper mode in the webview.
   -l Set the log level. Their precedence is: ALL < TRACE < DEBUG < INFO < WARN < ERROR < FATAL < OFF.
   -g Set the log file name.
   -q Alias for -l OFF.
    These three previous option may also be set by the environment variables 'LOG' and 'LOGFILE' but the options have precedence on the environment.
    If neither the environment nor the options are set then relies on the debug option if it is used to set the log level to 'DEBUG'.
   -u List and briefly explain all the javascript objects extending the webview.
   -n Set windows icon with the provided .ico file.
   -m The webview window will be minimized at startup.
   -M The webview window will be maximized at startup.
   -s The webview window will not be shown at startup.
   -k Set webview hints => 0: width and height are default size, 1 set them as minimum bound, 2 set them as maximum bound. 3 they are fixed. Any other value is ignored.
   -a Do not generate javascript class instanciation for the webview extension functions</code>

### 2. **<ins>Some indications</ins>**

   *Regarding the added variables and functions to this webview app:*  
   **a)** To test them you may encapsulated their result in [template literal](https://developer.mozilla.org/docs/Web/JavaScript/Reference/Template_literals) within any output function such as console.log or through the DOM. Something similar to the following:  
&nbsp;&nbsp;&nbsp;<code><ins>cls();console.log("Reworked content of the 'PATH' environment variable (path separators replaced by newlines):");console.log(`${env.path.replace(/;/g, "\n")}`)</ins></code>  
   **b)** Notice that when calling one of the functions, you have to deal with their asynchronous context by using  [promises](https://developer.mozilla.org/docs/Web/JavaScript/Reference/Global_Objects/Promise) or the [await](https://developer.mozilla.org/docs/Web/JavaScript/Reference/Operators/await) keyword in an [asynchronous function](https://developer.mozilla.org/docs/Web/JavaScript/Reference/Statements/async_function). Basically if you expect a return from any of these function you must call it in an asynchronous way. The only context that could allows you to avoid using asynchronous mode would be by calling a function that you do not expect its return, for instance '<code>app.set_title("New title")</code>, if you don't need to know the old title'.  

### 3. **<ins>The -u output</ins>**

   <ins>List of the variables and functions for the objects extending the webview</ins>  
   *<ins>Variables:</ins>*  
   Read-only ones are prepended by a '-'*  
   <code>-app.sysname: provide the name of the system, this may help to assert in your javascript code that you are running a webapp, something like console.log(typeof app.system === 'undefined'?'not a webapp':'is a webapp').
   -app.x: horizontal position of the upper left corner of the webapp window.
   -app.y: vertical position of the upper left corner of the webapp window.
   -app.w: horizontal position of the lower left corner of the webapp window.
   -app.h: vertical position of the upper left corner of the webapp window.
   All these coordinate variables are updated in real time. To assert it, try something like:
   <ins>(async()=>{ elt=document.createElement('i'); document.body.appendChild(elt); new Promise(function (resolve, reject) { (function _(){ coords=`(${app.x}, ${app.y}, ${app.w}, ${app.h})`; if (elt.innerText != coords) { elt.innerText=coords; } timeoutId=setTimeout(_, 10); })(); }) })()</ins>
   And to stop it, later:
   <ins>clearTimeout(timeoutId); elt.remove()</ins>
   -app.title: title of the webapp window, usually displayed one its title bar.
   -app.icon: file name of the icon used by the webapp.
   -app.info: informations about the webapp.
   -app.state: contains the state of the application windows, may have one of the following values: normal, maximised, minimised, hidden, full_screen
   -app.last_error: may contains the last app object error
   -env.path: actual value of the PATH environment variable
   -fs.last_error: may contains the last fs object error
   -fs.cwd: current working directory</code>
   *<ins>Functions:</ins>  
   Asynchronous ones are prepended by a* '\*'  
   <code>&nbsp;app.help: accepts up to 1 parameter, return this help message and the list of all the available variables and functions for the objects extending the app.
   &nbsp;app.echo: accepts up to 10 parameters, echo the parameter
   &nbsp;app.restore: restore the webapp window.
   &nbsp;app.minimize: minimize the webapp window.
   &nbsp;app.maximize: maximize the webapp window.
   &nbsp;app.enter_fullscreen: set the webapp window in fullscreen mode.
   &nbsp;app.exit_fullscreen: exit the webapp window from fullscreen mode.
   &nbsp;app.show: show the webapp window.
   &nbsp;app.hide: hide the webapp window.
   &nbsp;app.set_title: accepts up to 1 parameter, return the actual app title and change it if a new one is provided as a parameter.
   &nbsp;app.set_pos: accepts up to 2 parameters, return the actual app position as a json object (x, y) and if there are parameters then change it.
   &nbsp;app.center: center the window on the screen, the x and y position and width and height will never be out of the screen.
   &nbsp;app.set_size: accepts up to 3 parameters, return the actual app size as a json object (w, h) and if there are parameters then change it.
   &nbsp;app.set_geometry: accepts up to 4 parameters, return the actual app position and size as a json object (x, y, w, h) and if there are parameters then change it.
   &nbsp;app.set_icon: accepts up to 1 parameter, set window top left icon and return the previous one if thre was.
   &nbsp;app.on_geometry: accepts up to 1 parameter, set callback to detect when webapp has moved.
   &nbsp;app.on_exit: accepts up to 1 parameter, set callback to detect when webapp is exiting.
   &nbsp;app.exit: exit from webapp.
   *app.system: needs 1 parameter, run an external command in a thread and return its output.
   *app.pipe: needs 1 parameter, run an external command in a pipe and return its output.
   *app.wpipe: needs 1 parameter, run an external command in a wstring pipe and return its URI encoded output.
   *app.shell: needs 1 parameter, open an external windows command in a thread (See ShellExecute) and return its output.
   *env.get: needs 1 parameter, return the value of an environment variable.
   &nbsp;env.set: needs 2 parameters, set an environment variable to a given value and return the previous one if it existed.
   &nbsp;env.ins: needs 2 parameters, insert a value into an environment variable and return the previous one if it existed.
   &nbsp;env.add: needs 2 parameters, add a value to an environment variable and return the previous one if it existed.
   &nbsp;env.del: needs 1 parameter, delete an environment variable and return its last value if it existed.
   *env.get_path: return the value of the PATH environment variable.
   &nbsp;env.add_path: needs 1 parameter, add a program path to the PATH env variable if is not yet added.
   &nbsp;env.ins_path: needs 1 parameter, insert a program path to the PATH env variable if is not yet added.
   &nbsp;env.set_path: needs 1 parameter, replace the whole value of the PATH environment variable with the provided one.
   &nbsp;env.del_path: delete the value of the PATH environment variable.
   *fs.exists: needs 1 parameter, return true if file exists else false and set 'fs.last_error' variable.
   &nbsp;fs.current_path: accepts up to 1 parameter, change the current path to the one provided, if any, and return it, if ok, else return false and set the variable 'fs.last_error'.
   *fs.absolute: needs 1 parameter, return the corresponding absolute path of a the parameter or false if error.
   *fs.mkdir: needs 1 parameter, create a directory with the provided path if it does not already exists and return true if ok else false and set the variable 'fs.last_error'.
   &nbsp;fs.stat: accepts up to 2 parameters, gives information details on the provided path in json format if second parameter is true (else in text).
   *fs.ls: accepts up to 4 parameters, list path (default is current directory), in a simple (default) or detailled list if second parameter is true, in one of the following format: text(default), html or json as defined by the third parameter andrecursively if fourth parameter is true (default is not).
   *fs.copy: needs 2 parameters, copy a file or directory (recursively) from 'path1' to 'path2'. Already existing files are updated if necessary. Return true if ok else false and set the variable 'fs.last_error'.
   *fs.remove: needs 1 parameter, remove a file or an empty directory, return true if ok else false and set the variable 'fs.last_error'.
   *fs.remove_all: needs 1 parameter, remove a file or a directory RECURSIVELY (BEWARE this erase everything underneath the directory !!)
   *fs.read: needs 1 parameter, read a file with the provided file name and return its content, if possible.
   *fs.write: at least 1 parameter, truncate and write to the file who's name is provided as the first parameter, the content of all the following parameters, return true if the operation was OK, else false.
   *fs.append: at least 1 parameter, append to the file who's name is provided as the first parameter, the content of all the following parameters, return true if the operation was OK, else false.
   *io.write: needs 1 parameter, write a string to stdout.
   *io.writeln: accepts up to 1 parameter, write a string and a carriage return to stdout.
   *io.ewrite: needs 1 parameter, write a string to stderr.
   *io.ewriteln: accepts up to 1 parameter, write a string and a carriage return to stderr.
   *io.read: read a string from stdin and return it.
   *io.readln: read a line ending with carriage return, from stdin and return it.
   *io.has_console: return true if has console else false.
   &nbsp;gui.msgbox: accepts up to 2 parameters, display a message dialog in a 'close to Windows MessageBox style', First parameter is the message and second one indicate whether we need only an 'OK' button (0), or an 'OK' and a 'Cancel' button (1) or a 'Yes' and a 'No' button (2).
   &nbsp;gui.opendlg: accepts up to 3 parameters, display a Windows file selection dialog with the eventually provided filter, initial directory and title. The filter consist in pairs of strings with the first of each providing the name and the second providing the pattern, eventual multiple pattern are separated by a comma (,) and eventual multiple pairs are separated by a pipe (|). If needed ',' & '|' might be neutralized by '\' (a doubled one in javascript).
   &nbsp;gui.savedlg: accepts up to 3 parameters, display a Windows file selection dialog with the eventually provided filter, initial directory and title. The filter consist in pairs of strings with the first of each providing the name and the second providing the pattern, eventual multiple pattern are separated by a comma (,) and eventual multiple pairs are separated by a pipe (|). If needed ',' & '|' might be neutralized by '\' (a doubled one in javascript).
   &nbsp;gui.folderdlg: accepts up to 3 parameters, display a Windows folder selection dialog with the eventually provided initial directory and title (Windows does not allow filter with folder selection).
   &nbsp;win.dark_bar: accepts up to 1 parameter, Set (true) on unset (false) the dark mode for the window bar. It is the responsability of web application to manage a dark mode for the DOM.
   &nbsp;win.printdlg: Open a print or preprint dialog (in this case parameter to true or 'preprint'.)
   *win.regsto: accepts up to 3 parameters, store a string to the Windows registry.
   *win.regget: accepts up to 3 parameters, retrieve a string from the Windows registry.</code>
