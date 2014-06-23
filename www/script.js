// variavel global para indicar a pagina que o sistema se encontra
var currPath = '/';

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
	
	client.open("POST", "?host-ip", true);
	client.send();
}

// Funcao que pergunta ao servidor o numero de hosts e retorna o mesmo
function getNumberOfHosts(){ 
	var client;
	client = configureBrowserRequest(client);	
	client.open("POST", "?n-hosts", true);
	client.send();
	
	client.onreadystatechange = function() {
		if(client.readyState == 4 && client.status == 200)
			document.getElementById("n-hosts").innerHTML = client.responseText;
		else
			document.getElementById("n-hosts").innerHTML = "-";
	}
	
	client.open("POST", "?n-hosts", true);
	client.send();
}

// Funcao que pergunta ao servidor o estado do servidor e retorna-o
function getServerStatus(){ 
	var client;
	client = configureBrowserRequest(client);	
	client.open("POST", "?server-state", true);
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
	if(page == 3)
		requestAndPutHTML("?list-users", "users-list");
	requestAndPutHTML("?total-file", "total-files");
	requestAndPutHTML("?total-folder", "total-folders");
	requestAndPutHTML("?total-size", "total-size");
}

function requestAndPutHTML(command, areaId){
	var server;
	server = configureBrowserRequest(server);	
	server.onreadystatechange = function() {
		if(server.readyState == 4 && server.status == 200){
			document.getElementById(areaId).innerHTML = server.responseText;
		}
	}
	server.open("POST", command, true);
	server.send();
}

function optionListUser(){
	requestAndPutHTML("listUsers.html", "content");
	requestAndPutHTML("?list-users", "users-list");
}

function optionListFiles(){
	requestAndPutHTML("listFiles.html", "content");
	requestAndPutHTML("?Rfolder=" + currPath, "file-system-body");
}

function optionUploadFile(){
	requestAndPutHTML("uploadFile.html", "content");
}

function addFileInput(){
	var plusField = document.createElement("input");
	plusField.setAttribute('type', 'file');
	plusField.setAttribute('name', 'files[]');
	document.getElementById("file-inputs").appendChild(plusField);
}

function newFolder(){
	document.getElementById("newfolder-input").style.display = "block";
	document.getElementById("newfolder-button").style.display = "block";
}

function addFolder(){
	var folderPath = currPath + '' + document.getElementById("newfolder-input").value;
	var server;
	server = configureBrowserRequest(server);	
	server.onreadystatechange = function() {
		if(server.readyState == 4 && server.status == 200)
			optionListFiles();
	}
	server.open("POST", "?Cfolder=" + folderPath, true);
	server.send();
}

function retrieveFolder(folderPath){
	currPath = folderPath;
	requestAndPutHTML("?Rfolder=" + currPath, "file-system-body");
}

function updateFolder(){
	var folderPath = currPath + '' + document.getElementById("newfolder-input").value;
	var server;
	server = configureBrowserRequest(server);	
	server.onreadystatechange = function() {
		if(server.readyState == 4 && server.status == 200)
			optionListFiles();
	}
	server.open("POST", "?Ufolder=" + folderPath + "&new=" + newName, true);
	server.send();
}

function deleteFolder(folderPath){
	var server;
	server = configureBrowserRequest(server);	
	server.onreadystatechange = function() {
		if(server.readyState == 4 && server.status == 200)
			optionListFiles();
	}
	server.open("POST", "?Dfolder=" + folderPath, true);
	server.send();
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