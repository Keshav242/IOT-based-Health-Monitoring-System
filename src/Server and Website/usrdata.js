function opn(id){
    id = id.srcElement.getAttribute('id');
    let rght = document.getElementById('right_col');
    let x = rght.querySelectorAll("*");
    for(let i=0; i<x.length; i++){
        var s = x[i];
        if(i==0){
            s.style.display = "none";
        }
        else{
            s.remove();
        }
    }

    var ifr = document.createElement('iframe');
    ifr.setAttribute('id','charts');
    ifr.setAttribute('src', `patientr/${id}.html`);
    ifr.style.cssText = 'width: 100%; height: 85%; border: none; margin-top: 7%;';
    rght.appendChild(ifr);
}

for(let i=1; i<=document.getElementsByClassName("cntct").length; i++){
    var n = "pt"+i;
    var p = document.getElementById(n);
    p.addEventListener("click", opn);
    var id = n;
}