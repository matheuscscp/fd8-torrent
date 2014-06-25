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
			
			if(parseInt(server.response) == 0){
				var campoMsg = document.getElementById('msg');
				campoMsg.innerHTML = "Não foi possível efetuar login!";
				campoMsg.style.color = '#f00';
			}
			else {
				window.location.href = "/";
			}
		}
	}
	
	server.open("POST", "?" + username, true);
	server.send();
	
}