
log = function(msg) { wscript.echo(msg); }

function Args() {
  this.All=(new VBArray(VBArgs())).toArray();
  this.Unnamed=[];
  this.Named=[];
  this.Named.length=0;
  var i=0, j=0;

  while (i < this.All.length) {
    if (this.All[i].substring(0, 1) === "/") {
      n=this.All[i].split(':');
      this.Named[n[0].substr(1)]="";
      if (typeof n[1] != "undefined") this.Named[n[0].substr(1)]=n[1];
      this.Named.length++;
    } else {
      this.Unnamed[j++]=this.All[i];
    }

    i++;
  }


  this.ListAll = function () {
    log("AllArgs:"+this.All.length);
    for (var k in this.All) log(k+":"+this.All[k]);
  }

  this.ListNamed = function () {
    log("NamedArgs:"+this.Named.length);
    for (var k in this.Named) log(k+":"+this.Named[k]);
  }

  this.ListUnnamed = function () {
    log("UnnamedArgs:"+this.Unnamed.length);
    for (var i=0; i < this.Unnamed.length; i++) log(i+":"+this.Unnamed[i]);
  }
}

