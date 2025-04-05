class Queue {
    constructor(...elements) {
        // Initializing the queue with given arguments 
        this.elements = [...elements];
    }
    // Proxying the push/shift methods
    push(...args) {
        return this.elements.push(...args);
    }
    shift(...args) {
        return this.elements.shift(...args);
    }
    // Add some length utility methods
    get length() {
        return this.elements.length;
    }
    set length(length) {
        return this.elements.length = length;
    }
}

/* Usage
const q = new Queue();

q.push(2);

console.log(q.length); / 3

while(q.length)
	console.log(q.shift()); / 0, 1, 2
*/


const q = new Queue();
console.log("Enfilage");
for (x = 0; x < Math.PI; x += Math.PI / 10) {
    var coord = {
        x: x,
        y: Math.sin(x)
    };
    console.log(`(${coord.x}, ${coord.y})`);
    q.push(coord);
}

console.log(`Il y a ${q.length} éléments dans la file`);

console.log("Défilage");
while (q.length) {
    var ncoo = q.shift();
    console.log(`(${ncoo.x}, ${ncoo.y})`);
}


console.log(`Il y a ${q.length} éléments dans la file`);