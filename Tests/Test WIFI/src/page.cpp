#include <WiFiClient.h>

const char index_html[] = 
R"rawliteral(<!DOCTYPE HTML>

<html>
<head>
    <title>Interface fibre optique </title>
  </head> 
  <body>
    <div class="main">
      <p>
        <input type="text" id="inputText" value="coucou" size="15"/>
        <input type="button" id="port" value="Envoyer" onclick="sendData()" />
      </p>
	  <p>
	  Vitesse de com.
	  <select name="speed" id="speed-select" onchange="changeSpeed()" >
			<option value="1">10</option>
			<option value="2">30</option>
			<option value="3">100</option>
			<option value="4">300</option>
			<option value="5">1000</option>
			<option value="6">3000</option>
			<option value="7">10000</option>
		</select>
		bit/s
		</p>
      <hr />
       <textarea id="outputText" name="w3review" rows="4" cols="30" disabled="true"></textarea>
      <p>
	  <input type="button" id="btclr" value="Clear" onclick="clearOutput()" />
	  <input type="button" id="port" value="getData" onclick="poolData()" />
      </p>
    </div>
  </body>
<script>

document.addEventListener('DOMContentLoaded', setup, false);

function setup() {
    setInterval(executer, 2000);
}

function clearOutput(){
	const out=document.getElementById("outputText");
	out.value ="";
	console.log('output clear!');
}

function changeSpeed(){
	var requete = new XMLHttpRequest(); 
    var url = location + '?speed=';
	url+=document.getElementById("speed-select").value;
	requete.open("GET", url,true);
	requete.send("toto");
	console.log('speed changed! '+ url );
}

function sendData(){
	var requete = new XMLHttpRequest(); 
    var url = location + '?data=';
	url+=document.getElementById("inputText").value;
	requete.open("GET", url,true);
	requete.send(null);
	
	console.log('sent!');
}

function poolData() {
    var requete = new XMLHttpRequest(); 
    var url = location + "?pooldata";
    
    console.log(url) // Pour debugguer l'url formée    
    requete.open("GET", url, true); // On construit la requête
    requete.send(null); // On envoie !
    requete.onreadystatechange = function() { // on attend le retour
        if (requete.readyState == 4) { // Revenu !
            if (requete.status == 200) {// Retour s'est bien passé !
            	console.log(requete.responseText);
                donnees = JSON.parse(requete.responseText);
                console.log(donnees);
                const out=document.getElementById("outputText");
	            out.value+=donnees["data"];
            }
        }
    };
	
	console.log('pool!');
}


</script>
</html>
)rawliteral";

void send_page(WiFiClient client){
    client.print(index_html);
}