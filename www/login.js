function submitLogin(){
	var username = document.getElementById('input').value;
	var client;
	var xmlhttp;
	
	if (window.XMLHttpRequest) // code for IE7+, Firefox, Chrome, Opera, Safari
		xmlhttp = new XMLHttpRequest();
	else // code for IE6, IE5
		xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
	
	client = configureBrowserRequest(client);	
	client.onreadystatechange = function() {
		if(client.readyState == 4 && client.status == 200){
			if(client.response == 0){
				var campoMsg = document.getElementById('msg');
				campoMsg.innerHTML = "Este usuário já existe no sistema!";
				campoMsg.style.color = '#f00';
			}
		}
		else{
			alert("Houve um erro com a conexão com o servidor.");
		}
	}
	
	client.open("POST", "?" + username, true);
	client.send();
	
}