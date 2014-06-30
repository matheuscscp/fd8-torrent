function submitLogin(){
	var username = document.getElementById('input').value;
	if (username == "") return;
	
	var server;
	if (window.XMLHttpRequest) // code for IE7+, Firefox, Chrome, Opera, Safari
		server = new XMLHttpRequest();
	else // code for IE6, IE5
		server = new ActiveXObject("Microsoft.server");
		
	server.onreadystatechange = function() {
		if(server.readyState == 4 && server.status == 200){
			var resp = parseInt(server.response);
			if(resp == 0){
				var campoMsg = document.getElementById('msg');
				campoMsg.innerHTML = "Não foi possível efetuar login!";
				campoMsg.style.color = '#f00';
			}
			else if (resp == 1) {
				window.location.href = "/";
			}
			else if (resp == 2) {
				var campoMsg = document.getElementById('msg');
				campoMsg.innerHTML = "Inicializando, espere um momento...";
				campoMsg.style.color = '#ff0';
			}
		}
	}
	
	server.open("POST", "?login=" + username, true);
	server.send();
	
}