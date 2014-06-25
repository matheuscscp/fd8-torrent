// variavel global para indicar a pagina que o sistema se encontra
var currPath = '/';
var page = 1;
var folderToEdit = '';
var folderFormAction;

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
	setInterval(function(){refreshSideInfo()}, 3000);
	optionListFiles();
}

// Funcao que pergunta ao servidor o IP do host e retorna o mesmo
function getHostIP(){
	var client;
	client = configureBrowserRequest(client);	
	
	client.onreadystatechange = function() {
		if(client.readyState == 4 && client.status == 200){
			document.getElementById("host-ip").innerHTML = client.responseText;
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
		if(client.readyState == 4 && client.status == 200){
			if(parseInt(client.responseText) <= 2)
				document.getElementById("n-hosts").style.color = "#f00";
			else
				document.getElementById("n-hosts").style.color = "#000";
			document.getElementById("n-hosts").innerHTML = client.responseText;
		}
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
	//getHostIP();
	//getNumberOfHosts();
	//getServerStatus();
	//if(page == 2)
	//	requestAndPutHTML("?list-users", "users-list");
	//requestAndPutHTML("?total-files", "total-files");
	//requestAndPutHTML("?total-folders", "total-folders");
	//requestAndPutHTML("?total-size", "total-size");
}

function requestAndPutHTML(command, areaId){
	var server;
	server = configureBrowserRequest(server);
	server.onreadystatechange = function() {
		if(server.readyState == 4 && server.status == 200){
			document.getElementById(areaId).innerHTML = server.responseText;
			if (page == 1)
				document.getElementById("filesystem-local").innerHTML = currPath;
		}
	}
	server.open("POST", command, true);
	server.send();
}

function optionListUser(){
	page = 2;
	requestAndPutHTML("listUsers.html", "content");
	requestAndPutHTML("?list-users", "users-list");
}

function optionListFiles(){
	page = 1;
	requestAndPutHTML("listFiles.html", "content");
	// get a VALID current path from server
	{
		var server;
		server = configureBrowserRequest(server);
		server.onreadystatechange = function() {
			if(server.readyState == 4 && server.status == 200)
				currPath = server.responseText;
		}
		server.open("POST", "?RfolderPath=" + currPath, false);
		server.send();
	}
	requestAndPutHTML("?Rfolder=" + currPath, "file-system-body");
}

function addFile(){
	var plusField = document.createElement("input");
	plusField.setAttribute('type', 'file');
	plusField.setAttribute('name', 'files[]');
	document.getElementById("file-inputs").appendChild(plusField);
}

function previousFolder(){
	var i;
	if(currPath == "/") return;
	for(i = currPath.length-1; i >= 0; i--){
		if (currPath[i] == "/")
			break;
	}
	currPath = currPath.substring(0, i <= 0 ? i + 1 : i);
	retrieveFolder(currPath);
}

// --------------------------------------------------------------
// ------------- FOLDER CRUD ------------------------------------

function formFolderAction(){
	if (folderFormAction == 1)
		addFolder();
	else if (folderFormAction == 2)
		updateFolder();
}

function newFolder(){
	var input = document.getElementById("folderform-input");
	var button = document.getElementById("folderform-button");

	folderFormAction = 1;
	
	button.style.display = "table-cell";
	button.innerHTML = "Criar";
	input.style.display = "table-cell";
	input.placeholder = "Nome da nova pasta";
}

function addFolder(){
	var folderPath;
	if(currPath == "/")
		folderPath = currPath + document.getElementById("folderform-input").value;
	else
		folderPath = currPath + "/" + document.getElementById("folderform-input").value;
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
	optionListFiles();
}

function editFolder( oldName ){
	var input = document.getElementById("folderform-input");
	var button = document.getElementById("folderform-button");
	
	folderFormAction = 2;
	folderToEdit = oldName;
	
	button.style.display = "table-cell";
	button.innerHTML = "Editar";
	input.style.display = "table-cell";
	input.placeholder = "Novo nome da pasta";
}

function updateFolder(){
	if(currPath == "/")
		folderPath = currPath + folderToEdit;
	else
		folderPath = currPath + "/" + folderToEdit;
	var newName = document.getElementById("folderform-input").value;
	var server;
	server = configureBrowserRequest(server);	
	server.onreadystatechange = function() {
		if(server.readyState == 4 && server.status == 200)
			optionListFiles();
	}
	server.open("POST", "?Ufolder=" + folderPath + "?&" + newName, true);
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

// --------------------------------------------------------------
// --------------- FILE CRUD ------------------------------------

function showPopUpAndPrint(html){
	document.getElementById('pop-content').innerHTML = html;
	document.getElementById('pop-header-title').value = "";
	document.getElementById('pop-back').style.display = 'block';
}

function closePopUp(){
	document.getElementById('pop-content').innerHTML = "";
	document.getElementById('pop-back').style.display = 'none';
}

function addFile(){
	var html = '<input id="file-input" type="file"><br>';
	html	+= '<input id="use-other" type="checkbox"> <label>Usar este nome:</label> <input id="other-name" type="text"><br>';
	html	+= '<button class="submit" onclick="createFile()">Enviar</button>';
	showPopUpAndPrint(html);
}

function getExtention(fileName){
	var extention = "";
	var j = 0;
	for (var i = fileName.length; fileName[i] != '.'; i--){
		if(fileName[i])
			extention += fileName[i];
	}
	if(j < 5)
		return '.' + extention.split("").reverse().join("");
	else
		return "";
}

function createFile(){
	var fileSelector = document.getElementById("file-input");
	var fileName = fileSelector.files[0].name;
	var extention = getExtention(fileName);
	var checkbox = document.getElementById("use-other");
	if(checkbox.checked){
		fileName = document.getElementById("other-name").value + extention;
	}
	fileName = currPath + fileName;

	var formData = new FormData();
	var client;
	client = configureBrowserRequest(client);
	if( fileName ){	
		formData.append("upload", fileSelector.files[0]);
		client.open("post", "?Cfile=" + fileName, true);
		client.setRequestHeader("Content-Type", "multipart/form-data");
		client.setRequestHeader("Tamanho", fileSelector.files[0].size);
		client.send(formData);
		client.onreadystatechange = function() {
			if(client.readyState == 4 && client.status == 200)
				closePopUp();
				optionListFiles();
		}
	}
}