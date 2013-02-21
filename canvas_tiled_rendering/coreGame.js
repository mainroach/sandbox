
//---------------------------------
function xhrGet(reqUri,reqCred,callback)
{
	var contentType = "application/x-www-form-urlencoded; charset=utf-8";
	var xhr = new XMLHttpRequest();
    xhr.open("GET", reqUri, true);
	xhr.overrideMimeType('text/plain; charset=x-user-defined');
    xhr.onreadystatechange = function() 
	{
      if (xhr.readyState == 4 && xhr.responseText !="")
	  {
		if(callback != null)
			callback(xhr);
      }
    };
	if(reqCred)
		xhr.withCredentials = "true";
    xhr.send();
}


//------------------
var cv;
var gCanvas_context ;
var pPos={x:3193,y: 3178};
var pDir={x:1,y:1};
var pRotAngle = 0.0;
var vDist = 50;
var speed = 3;
var mrdoob_stats = null;
//-------------------------------------

function init()
{
	cv = document.getElementById('cvs');
	gCanvas_context  = cv.getContext('2d');
	
	
		
	window.addEventListener('keydown', this.keydown, false);
    window.addEventListener('keyup', this.keyup, false);
	
	//ISSUE a command to fetch all the data linearly by name
	gMap.load("../data/map.json");
	
	mrdoob_stats = new Stats();
	mrdoob_stats.getDomElement().style.position = 'absolute';
	mrdoob_stats.getDomElement().style.left = cv.style.left;
	mrdoob_stats.getDomElement().style.top = "155px";
	document.body.appendChild(mrdoob_stats.getDomElement());
	 
}
//-------------------------------------
function adjustRot()
{
	var cs = Math.cos(pRotAngle);
	var ss = Math.sin(pRotAngle);
	
	var x = 1;
	var y = 0;
	var x0 = cs * x + ss * y;
	var y0 = -ss * x + cs * y;
	
	pDir.x = x0;
	pDir.y = y0;
	
}
//-------------------------------------
var kDown={};
function keydown(event)
{
	kDown[event.keyCode]=1;	
}
//-------------------------------------
function keyup(event)
{
	kDown[event.keyCode]=0;
}

//-------------------------------------
function step()
{
	requestAnimFrame( step );
	mrdoob_stats.update();
	draw();
	
	//adjust inputs!
	
	//if(kDown[87])
	{
		pPos.x += pDir.x * speed;
		pPos.y += pDir.y * speed;
	}
/*	else if(kDown[83])
	{
		pPos.x += pDir.x * -speed;
		pPos.y += pDir.y * -speed;
	}
	*/
	if(kDown[65])
	{
		pRotAngle-=0.05;
		adjustRot();
	}
	else if(kDown[68])
	{
		pRotAngle+=0.05;
		adjustRot();
	}	
	
	
	//floor our values to keep our positions in integer space
	pPos.x = Math.floor(pPos.x);
	pPos.y = Math.floor(pPos.y);
		
	if(pPos.x <=0) pPos.x = 0;
	if(pPos.y <=0) pPos.y = 0;
	if(pPos.x >=gMap.pixelSize.x) pPos.x = gMap.pixelSize.x;
	if(pPos.y >=gMap.pixelSize.y) pPos.y = gMap.pixelSize.y;
}

//-------------------------------------
function draw()
{
	var cWidth = 1000;
	var cHeight = 1000;
	var hW = cWidth/2;
	var hH = cHeight/2;
	
	gCanvas_context.fillStyle = "White";
	gCanvas_context.fillRect(0,0,cWidth,cHeight);
	
	gMap.viewRect.x = (pPos.x - hW);
	gMap.viewRect.y = (pPos.y - hH);
	gMap.viewRect.w = cWidth;
	gMap.viewRect.h = cHeight;
	gMap.draw(gCanvas_context);
	
	//draw our little dude guy
	gCanvas_context.fillStyle = "Red";
	gCanvas_context.fillRect(pPos.x-2 - gMap.viewRect.x ,pPos.y-2- gMap.viewRect.y ,5,5);
	//draw directional line
	gCanvas_context.strokeStyle = "Blue";
	gCanvas_context.beginPath();
	gCanvas_context.moveTo(pPos.x - gMap.viewRect.x, pPos.y - gMap.viewRect.y);
	gCanvas_context.lineTo((pPos.x + pDir.x *vDist)- gMap.viewRect.x, (pPos.y  + pDir.y *vDist)- gMap.viewRect.y);
	gCanvas_context.stroke();
}
	