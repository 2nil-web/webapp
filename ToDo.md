# ToDo / Task list

## Project management/GitHub use/GitLab use

- [ ] Reset the repositories ⚠️

- [ ] And work at least with one main/master branch (for delivery) and develop branch (for development and merge)

- [ ] Improve setup/package management (Windows and Linux ...) target setup should build more than a zip (standard packages like winget, rpm ... )

- [ ] Improve delivery

## Documentation

- [ ] A rework of the README.md is needed

- [ ] A Javascript developper's manual/tutorial may be useful

- [ ] A Javascript developper reference is needed (see app.help())

- [ ] C++ developper code reference/explainations

- [ ] Improve code documentation

- [ ] 

## Code

- [ ] Cleaning up code, especially dead code

- [ ] Improve GTK 3

- [ ] Ensure that the GTK 4 implementation is working correctly and eventually improve it
  
   *A good latest Ubuntu or Fedora or Arch machines is needed, as the ones coming with Wsl2 on Windows 10 gives very poor results, unusable at the GUI level*

- [ ] Consider moving from GTK C to GTKMM

- [ ] CUI : trying to improve this feature under Windows
    *Utf issues*
    *cpp/gui_cons (see also cpp/redirectio.cpp) - In progress but may be not possible due to Windows poor console management ...

- [ ] May be a reworking/reorganization/refactoring of the conditional compilation

- [ ] Fusionner parse_ini et read_ini dans wrap.cpp 

- [ ] Ajouter des fonctions pure js dans l'init webapp pour notamment (pas sûr):

```javascript
String.prototype.toHtmlEntities = function() {
  return this.replace(/./gm, function(s) {
    return (s.match(/[a-z0-9\s]+/i)) ? s : "&#" + s.charCodeAt(0) + ";";
  });
};
```

```javascript
String.prototype.fromHtmlEntities = function() {
  return this.replace(/&#\d+;/gm,function(s) {
    return String.fromCharCode(s.match(/\d+/gm)[0]);
  })
};
```
