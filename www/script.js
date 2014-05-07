var page = 'home';

window.onload = init();

function init(){
	var ip = document.getElementById("host-ip").innerHTML = window.location.host;
}

function configureBrowserRequest(xmlhttp){
	if (window.XMLHttpRequest) { // code for IE7+, Firefox, Chrome, Opera, Safari
		xmlhttp = new XMLHttpRequest();
	}
	else { // code for IE6, IE5
		xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
	}
	
	return xmlhttp;
}

function menuOpClicked(option){
	var request = defineRequest(option.id);
	var xmlhttp;
	page = option.id;
	
	xmlhttp = configureBrowserRequest(xmlhttp);
	
	xmlhttp.onreadystatechange = function() {
		if(xmlhttp.readyState == 4 && xmlhttp.status == 200){
			document.getElementById("content").innerHTML = xmlhttp.responseText;
		}
	}

	xmlhttp.open("POST", request, true);
	xmlhttp.send();
}

function addButtonClicked(){
	var action = "new-" + page.substring(7);
	document.getElementById(action).style.display = 'block';
}

function addOpClicked(option){
	var request = option.id;
	var xmlhttp;
	
	xmlhttp = configureBrowserRequest(xmlhttp);	
}

function defineRequest(ID){
	switch (ID){
		case 'menuop-home':
			changeContentHeader(false, '#fff', '');
			return 'home.html'; 
		break;
		case 'menuop-user': 		
			changeContentHeader(true, '#f00', '#FF5C5C');
			return 'listUsers.html'; 
		break;
		case 'menuop-file': 		
			changeContentHeader(true, '#00f', '#7070FF');
			return 'listFiles.html'; 
		break;
		case 'menuop-host': 		
			changeContentHeader(false, '#00a118', '');
			return 'listHosts.html'; 
		break;
		case 'menuop-presentation': 
			changeContentHeader(false, '#F5E900', '');
			return 'presentation.html'; 
		break;
		default: alert("Link inválido"); break;
	}
}

function changeContentHeader(addOption, color, button){
	document.getElementById('content-header').style.background = color;
	document.getElementById('add-button').style.background = button;
	
	if (addOption){
		document.getElementById("add-button").style.display = 'block';
		document.getElementById("content-header-offset").style.display = 'none';
	} else{
		document.getElementById("add-button").style.display = 'none';
		document.getElementById("content-header-offset").style.display = 'block';
	}
}

function addingUser(){
	var username = document.getElementById("input-user").value;
	var password = document.getElementById("input-pass").value;
	var msgArea = document.getElementById("form-message");
	var xmlhttp;
	
	if( !username || !password ){
		msgArea.innerHTML = "Preencha todos os campos!";
		msgArea.style.color = '#f00';
	} else{
		
		xmlhttp = configureBrowserRequest(xmlhttp);
		
		xmlhttp.onreadystatechange = function() {
			if(xmlhttp.readyState == 4 && xmlhttp.status == 200){
				if (xmlhttp.responseText == '1'){
					msgArea.innerHTML = "Usuário cadastrado com sucesso!";
					msgArea.style.color = '#00ff00';
				} else{
					msgArea.innerHTML = "Ocorreu um erro no cadastro. Tente novamente";
					msgArea.style.color = '#f00';
				}
			}
		}

		xmlhttp.open("POST", "?add-user", true);
		xmlhttp.send("user=" + username + "&pass=" + password);
	}
}

function addFileInput(){
	var plusField = document.createElement("input");
	plusField.setAttribute('type', 'file');
	plusField.setAttribute('name', 'files[]');
	document.getElementById("file-inputs").appendChild(plusField);
}

function submitFilesButtonClicked(){
	var fileSelectors = document.getElementsByName("files[]");
	var client;
	var formData = new FormData();
	
	client = configureBrowserRequest(client);
	
	for(var i = 0; i < fileSelectors.length; i++){
		var fileSelector = fileSelectors[i];
		var fileName = fileSelector.files[0].name;
		alert(fileName);
		
		formData.append("upload", fileSelector.files[0]);
		
		client.open("post", "/upload", true);
		client.setRequestHeader("Content-Type", "multipart/form-data");
		client.send(formData);
		
	}
}