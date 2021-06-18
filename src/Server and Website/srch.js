function srch(){
    var inp = document.querySelector('#s_br').value.toLowerCase();

    var cntcts = document.getElementById('cntcts');
    var pt_n = cntcts.getElementsByClassName('pt_n');

    for(let i=0;i<pt_n.length;i++){

        var pt_prf = pt_n[i].getElementsByClassName('pt_prf');
        var user_n = pt_prf[0].getElementsByClassName('user_n');
        var btn = user_n[0].getElementsByTagName('button')[0];
        var tv = btn.textContent.toLowerCase() || cntct.innerHTML.toLowerCase();
        
        if(tv.indexOf(inp) > -1){
            pt_n[i].style.display = '';
        }
        else{
            pt_n[i].style.display = 'none';
        }

    }
}

document.querySelector('#s_br').addEventListener('keyup', srch);