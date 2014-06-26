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
	setInterval(function(){refreshSideInfo()}, 2000);
	optionListFiles();
}

function logout() {
	var client;
	client = configureBrowserRequest(client);	
	client.onreadystatechange = function() {
		if(client.readyState == 4 && client.status == 200){
			if (parseInt(client.responseText) == 1)
				window.location.href = "/";
		}
	}
	client.open("POST", "?logout", true);
	client.send();
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
			if(parseInt(client.responseText) <= 2) {
				document.getElementById("n-hosts").style.color = "#f00";
				document.getElementById("n-hosts").innerHTML = "<strong>" + client.responseText + "</strong>";
			} else {
				document.getElementById("n-hosts").style.color = "#fff";
				document.getElementById("n-hosts").innerHTML = client.responseText;
			}
		}
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
			document.getElementById("server-status").innerHTML = "Conectado";
		}
		else
			document.getElementById("server-status").innerHTML = "Desconectado";
	}
}

// Funcao para atualizar as informacoes na barra lateral do sistema
function refreshSideInfo(){
	getHostIP();
	getNumberOfHosts();
	getServerStatus();
	if(page == 2)
		requestAndPutHTML("?list-users", "users-list");
	requestAndPutHTML("?total-files", "total-files");
	requestAndPutHTML("?total-folders", "total-folders");
	requestAndPutHTML("?total-size", "total-size");
	requestAndPutHTML("?username", "username");
	requestAndPutHTML("?folder-tfolders=" + currPath, "folder-tfolders");
	requestAndPutHTML("?folder-tfiles=" + currPath, "folder-tfiles");
	requestAndPutHTML("?folder-tsize=" + currPath, "folder-tsize");
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
	document.getElementById("separator").style.display = 'none';
	document.getElementById("folder-info").style.display = 'none';
	requestAndPutHTML("listUsers.html", "content");
	requestAndPutHTML("?list-users", "users-list");
}

function optionListFiles(){
	page = 1;
	requestAndPutHTML("listFiles.html", "content");
	document.getElementById("separator").style.display = 'block';
  document.getElementById("folder-info").style.display = 'block';
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
	else if (folderFormAction == 3)
		updateFile();
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
		if(server.readyState == 4 && server.status == 200) {
			if (parseInt(server.responseText) == 1)
				optionListFiles();
			else
				alert('Não foi possível criar a pasta!');
		}
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
		if(server.readyState == 4 && server.status == 200) {
			if (parseInt(server.responseText) == 1)
				optionListFiles();
			else
				alert('Não foi possível atualizar o nome da pasta!');
		}
	}
	server.open("POST", "?Ufolder=" + folderPath + "?&" + newName, true);
	server.send();
}

function deleteFolder(folderPath){
	var server;
	server = configureBrowserRequest(server);	
	server.onreadystatechange = function() {
		if(server.readyState == 4 && server.status == 200) {
			if (parseInt(server.responseText) == 1)
				optionListFiles();
			else
				alert('Não foi possível apagar a pasta!');
		}
	}
	server.open("POST", "?Dfolder=" + currPath + (currPath == "/" ? "" : "/") + folderPath, true);
	server.send();
}

// --------------------------------------------------------------
// --------------- FILE CRUD ------------------------------------

function showPopUpAndPrint(html, title){
	document.getElementById('pop-content').innerHTML = html;
	document.getElementById('pop-header-title').innerHTML = title;
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
	showPopUpAndPrint(html, 'Adicionar arquivo');
}

function getExtension(fileName){
	var extension = "";
	var i;
	for (i = fileName.length-1; i >= 0 && fileName[i] != '.'; i--)
		extension = fileName[i] + extension;
	if (i < 0 || extension.length > 4)
		return "";
	return "." + extension;
}

function createFile(){
	var fileSelector = document.getElementById("file-input");
	var fileName = fileSelector.files[0].name;
	if(document.getElementById("use-other").checked){
		fileName = document.getElementById("other-name").value;
		if (getExtension(fileName) == "")
			fileName = fileName + getExtension(fileSelector.files[0].name);
	}
	fileName = currPath + (currPath != "/" ? "/" : "") + fileName;

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
			if(client.readyState == 4 && client.status == 200) {
				if (parseInt(client.responseText) == 1) {
					closePopUp();
					optionListFiles();
				}
				else {
					alert('Não foi possível subir o arquivo!');
				}
			}
		}
	}
}

function editFile( oldName ){
	var input = document.getElementById("folderform-input");
	var button = document.getElementById("folderform-button");
	
	folderFormAction = 3;
	folderToEdit = oldName;
	
	button.style.display = "table-cell";
	button.innerHTML = "Editar";
	input.style.display = "table-cell";
	input.placeholder = "Novo nome do arquivo";
}

function updateFile(){
	if(currPath == "/")
		filePath = currPath + folderToEdit;
	else
		filePath = currPath + "/" + folderToEdit;
	var newName = document.getElementById("folderform-input").value;
	if (getExtension(newName) == "")
		newName = newName + getExtension(filePath);
	var server;
	server = configureBrowserRequest(server);	
	server.onreadystatechange = function() {
		if(server.readyState == 4 && server.status == 200) {
			if (parseInt(server.responseText) == 1)
				optionListFiles();
			else
				alert('Não foi possível atualizar o nome do arquivo!');
		}
	}
	server.open("POST", "?Ufile=" + filePath + "?&" + newName, true);
	server.send();
}

function deleteFile(filePath) {
	var server;
	server = configureBrowserRequest(server);	
	server.onreadystatechange = function() {
		if(server.readyState == 4 && server.status == 200) {
			if (parseInt(server.responseText) == 1)
				optionListFiles();
			else
				alert('Não foi possível apagar o arquivo!');
		}
	}
	server.open("POST", "?Dfile=" + filePath, true);
	server.send();
}
