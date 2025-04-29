// Tri state values are :
// -1 ==> -1/question mark/indeterminate
//  0 ==> false/cross mark/unchecked
//  1 ==> true/check mark/checked
// On left click, cycle from one of tri state to the next
function setTriState() {
  //console.log(`Click triState: ${this.value}`);
  switch (this.value) {
    case '?':
      this.style.fontWeight = "normal";
      this.value = '✘';
      break;
    case '✘':
      this.style.fontWeight = "bold";
      this.value = '✓';
      break;
    case '✓':
    default:
      this.style.fontWeight = "bold";
      this.value = '?';
      break;
  }
}
// On right click, cycle from one of tri state to the second next
function setBiState(evt) {
  if (evt.button === 2) {
    switch (this.value) {
      case '?':
        this.style.fontWeight = "bold";
        this.value = '✓';
        break;
      case '✘':
        this.style.fontWeight = "bold";
        this.value = '?';
        break;
      case '✓':
      default:
        this.style.fontWeight = "normal";
        this.value = '✘';
        break;
    }
    console.log(`but: ${evt.button}, value: ${this.value}`);
  }
}
// Dispatch the event on the element
function triState(id) {
  var el = document.createElement('input');
  el.type = 'text';
  //el.size = 1;
  el.setAttribute("readonly", "readonly");
  el.setAttribute("onfocus", "this.blur()");
  el.id = id;
  el.title = "Left click to cycle in the order ?, ✘, ✓\nRight click to cycle in the order ?, ✓, ✘";
  el.value = '✘';
  el.onclick = setTriState;
  el.style.margin = "4px 4px 4px 4px";
  // top | right | bottom | left
  el.style.padding = "0 3px 0 3px";
  el.style.width = "8px";
  //el.style.height = "12px";
  el.style.border = "1px transparent";
  el.style.color = "white";
  el.style.background = "grey";
  el.style.borderRadius = "3px";
  el.style.textAlign = "center";
  el.style.verticalAlign = "middle";
  el.style.cursor = "default";
  el.addEventListener('contextmenu', event => {
    event.preventDefault();
  });
  el.addEventListener("mouseup", setBiState);
  output.appendChild(el);
  return el;
}
//var ts=triState("ts0");
//output.appendChild(document.createElement("br"));