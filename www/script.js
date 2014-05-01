var page = 'home';

window.onload = init();

function init(){
	var ip = document.getElementById("host-ip").innerHTML = window.location.host;
}

function addButtonClicked(){
	var action = "new-" + page.substring(7);
	document.getElementById(action).style.display = 'block';
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

function addOpClicked(option){
	var request = option.id;
	var xmlhttp;
	
	xmlhttp = configureBrowserRequest(xmlhttp);
	
}

function defineRequest(ID){
	switch (ID){
		case 'menuop-home':
			changeContentHeader(false, '#fff');
			return 'home'; 
		break;
		case 'menuop-user': 		
			changeContentHeader(true, '#f00');
			return 'listUsers.html'; 
		break;
		case 'menuop-file': 		
			changeContentHeader(true, '#00f');
			return 'listFiles.html'; 
		break;
		case 'menuop-host': 		
			changeContentHeader(false, '#00a118');
			return 'listHosts.html'; 
		break;
		case 'menuop-presentation': 
			changeContentHeader(false, '#bfb600');
			return 'presentation.html'; 
		break;
		default: alert("Link inválido"); break;
	}
}

function changeContentHeader(addOption, color){
	document.getElementById('content-header').style.background = color;
	
	if (addOption){
		document.getElementById("add-button").style.display = 'block';
	} else{
		document.getElementById("add-button").style.display = 'none';
	}
}

function addingUser(){
	var username = document.getElementById("input-user").value;
	var password = document.getElementById("input-pass").value;
	
	var xmlhttp;
	page = option.id;
	
	xmlhttp = configureBrowserRequest(xmlhttp);
	
	xmlhttp.onreadystatechange = function() {
		if(xmlhttp.readyState == 4 && xmlhttp.status == 200){
			if (xmlhttp.responseText == '1'){
				
			} else{
				
			}
		}
	}

	xmlhttp.open("POST", "add-user", true);
	xmlhttp.send("user=" + username + "&pass=" + password);
}