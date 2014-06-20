// variavel global para indicar a pagina que o sistema se encontra
var page = 'home';

// Funcao para inicializar a variavel de requisicao para o servidor
function configureBrowserRequest(xmlhttp){
	if (window.XMLHttpRequest) // code for IE7+, Firefox, Chrome, Opera, Safari
		xmlhttp = new XMLHttpRequest();
	else // code for IE6, IE5
		xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
	return xmlhttp;
}

function init(){
	refreshSideInfo();
	setInterval(function(){refreshSideInfo()}, 10000);
}

// Funcao que pergunta ao servidor o IP do host e retorna o mesmo
function getHostIP(){
	var client;
	client = configureBrowserRequest(client);	
	
	client.onreadystatechange = function() {
		if(client.readyState == 4 && client.status == 200){
			document.getElementById("host-ip").innerHTML = client.responseText;
		}
		else{
			document.getElementById("host-ip").innerHTML = "-";
		}
	}
	
	client.open("POST", "?Rhost-ip", true);
	client.send();
}

// Funcao que pergunta ao servidor o numero de hosts e retorna o mesmo
function getNumberOfHosts(){ 
	var client;
	client = configureBrowserRequest(client);	
	client.open("POST", "?Rn-hosts", true);
	client.send();
	
	client.onreadystatechange = function() {
		if(client.readyState == 4 && client.status == 200)
			document.getElementById("n-hosts").innerHTML = client.responseText;
		else
			document.getElementById("n-hosts").innerHTML = "-";
	}
	
	client.open("POST", "?Rn-hosts", true);
  client.send();
}

// Funcao que pergunta ao servidor o estado do servidor e retorna-o
function getServerStatus(){ 
	var client;
	client = configureBrowserRequest(client);	
	client.open("POST", "?Rserver-state", true);
	client.send();
	
	client.onreadystatechange = function() {
		if(client.readyState == 4 && client.status == 200) {
			document.getElementById("server-status").innerHTML = "On";
		}
		else
			document.getElementById("server-status").innerHTML = "Off";
	}
}

// Funcao para atualizar as informacoes na barra lateral do sistema
function refreshSideInfo(){
	getHostIP();
	getNumberOfHosts();
	getServerStatus();
}

// Funcao que da funcionalidade ao menu. Ao clicar em uma opcao ele faz a 
// requisicao da pagina correspondente e manda para a div 'content'
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

// Funcao que abre a div de cadastro de novo usuario/arquivo
function addButtonClicked(){
	var action = "new-" + page.substring(7);
	alert(action);
	document.getElementById(action).style.display = 'block';
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
	var loader = document.getElementById("loader");
	var xmlhttp;
	
	if( !username || !password ){
		msgArea.innerHTML = "Preencha todos os campos!";
		msgArea.style.color = '#f00';
	} else{
		loader.style.display = 'block';
		alert("oi");
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

		xmlhttp.open("POST", "?Sadd-user", true);
		xmlhttp.send("user=" + username + "&pass=" + password);
		loader.style.display = 'none';
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
	var formMsg = document.getElementById("form-message");
	var formData = new FormData();
	var client;
	
	client = configureBrowserRequest(client);
	
	for(var i = 0; i < fileSelectors.length; i++){
		var fileSelector = fileSelectors[i];
		var fileName = fileSelector.files[0].name;
		
		if( fileName ){	
			formData.append("upload", fileSelector.files[0]);
			
			client.open("post", "/upload", true);
			client.setRequestHeader("Content-Type", "multipart/form-data");
			client.setRequestHeader("Tamanho", fileSelector.files[0].size);
			client.setRequestHeader("Nome", fileName);
			client.send(formData);
		}
	}
}