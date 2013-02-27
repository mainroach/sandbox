
var CanvasTile = Class.extend({
	x:0,
	y:0,
	w:100,
	h:100,
	cvsHdl:null,
	ctx:null,
	
	//-----------------------------
	create:function(width,height)
	{
		this.x = -1;
		this.y = -1;
		this.w = width;
		this.h = height;
		var can2 = document.createElement('canvas');
		can2.width = width;
		can2.height = height;
		this.cvsHdl = can2;
		this.ctx = can2.getContext('2d');
		
	
	},
	//-----------------------------
	isVisible:function()
	{	
		var r2 = gMap.viewRect;
		var r1 = this;
		return gMap.intersectRect(	{top:r1.y,
											left:r1.x,
											bottom:r1.y+r1.h,
											right:r1.x+r1.w},
											{top:r2.y,
											left:r2.x,
											bottom:r2.y+r2.h,
											right:r2.x+r2.w});
			
	
	}

 });
 
var TILEDmap = Class.extend({
  currMapData: null,
  tileSets: new Array(),
  viewRect: {
    "x": 0,
    "y": 0,
    "w": 1000,
    "h": 1000
  },
  numXTiles: 100,
  numYTiles: 100,
  tileSize: {
    "x": 64,
    "y": 64
  },
  pixelSize: {
    "x": 64,
    "y": 64
  },
  
  imgLoadCount:0,
  fullyLoaded:false,
  canvasTileSize:{"x":1024,"y":1024},
  canvasTileArray:[],
  //---------------------------
  load: function (map) 
  {
	
	xhrGet(map, false,function(data)
	{
		gMap.parseMapJSON(data.response);
			
	});
  },
  //---------------------------
  
  parseMapJSON:function(mapJSON)
  {
	this.currMapData = JSON.parse( mapJSON );
	var map = this.currMapData;
    this.numXTiles = map.width;
    this.numYTiles = map.height;
    this.tileSize.x = map.tilewidth;
    this.tileSize.y = map.tileheight;
    this.pixelSize.x = this.numXTiles * this.tileSize.x;
    this.pixelSize.y = this.numYTiles * this.tileSize.y;
	
    //load our tilesets if we are a client.
	var gMap = this;
      for (var i = 0; i < map.tilesets.length; i++) 
	  {
        var img = new Image();
		img.onload = new function() 		{gMap.imgLoadCount++;};
        img.src = "../data/" + map.tilesets[i].image.replace(/^.*[\\\/]/, '');
        var ts = {
          "firstgid": map.tilesets[i].firstgid,
          "image": img,
          "imageheight": map.tilesets[i].imageheight,
          "imagewidth": map.tilesets[i].imagewidth,
          "name": map.tilesets[i].name,
          "numXTiles": Math.floor(map.tilesets[i].imagewidth / this.tileSize.x),
          "numYTiles": Math.floor(map.tilesets[i].imageheight / this.tileSize.y)
        };
        this.tileSets.push(ts);
      
	  
	  //clm precache the bg
	  checkWait(
					function()
					{
						return gMap.imgLoadCount == gMap.tileSets.length;
					},
					function () 
					{
						
						gMap.preDrawCache();
					});
	
    }
  },
  //---------------------------
  getTilePacket: function (tileIndex) {
    var pkt = {
      "img": null,
      "px": 0,
      "py": 0
    };
    var i = 0;
    for (i = this.tileSets.length - 1; i >= 0; i--) {
      if (this.tileSets[i].firstgid <= tileIndex) break;
    }

    pkt.img = this.tileSets[i].image;
    var localIdx = tileIndex - this.tileSets[i].firstgid;
    var lTileX = Math.floor(localIdx % this.tileSets[i].numXTiles);
    var lTileY = Math.floor(localIdx / this.tileSets[i].numXTiles);
    pkt.px = (lTileX * this.tileSize.x);
    pkt.py = (lTileY * this.tileSize.y);

    return pkt;
  },
  //---------------------------
	intersectRect:function (r1, r2) 
	{
		return !(r2.left > r1.right || 
			r2.right < r1.left || 
			r2.top > r1.bottom ||
			r2.bottom < r1.top);
	},
  //---------------------------
  draw: function (ctx) 
  { //
	if(!this.fullyLoaded) return;
	
	//aabb test to see if our view-rect intersects with this canvas.
	for(var q =0; q < this.canvasTileArray.length; q++)
	{
		var r1 = this.canvasTileArray[q];
		
		if(r1.isVisible())
			ctx.drawImage(r1.cvsHdl, r1.x-this.viewRect.x,r1.y-this.viewRect.y);
	}
  },
 //--------------------------- 
  preDrawCache:function()
  {
	var xCanvasCount = 1 + Math.floor(this.pixelSize.x / this.canvasTileSize.x);
	var yCanvasCount = 1 + Math.floor(this.pixelSize.y / this.canvasTileSize.y);
	var numSubCanv = xCanvasCount*yCanvasCount;
	
	for(var yC = 0; yC <yCanvasCount; yC ++)
	{
		for(var xC = 0; xC <xCanvasCount; xC ++)
		{
			var k = new CanvasTile();
			k.create(this.canvasTileSize.x,this.canvasTileSize.y);
			k.x = xC * this.canvasTileSize.x;
			k.y = yC * this.canvasTileSize.y;
			this.canvasTileArray.push(k);
		
			this.fillCanvasTile(k);
		}
	}
	
	gMap.fullyLoaded = true;
  },
  //------------------------------------
   fillCanvasTile:function(ctile)
  {
	var ctx = ctile.ctx;
	ctx.fillRect(0,0,ctile.w, ctile.h);
		var vRect={	top:ctile.y,
					left:ctile.x,
					bottom:ctile.y+ctile.h,
					right:ctile.x+ctile.w};
		
		for (var layerIdx = 0; layerIdx < this.currMapData.layers.length; layerIdx++) 
		{
		  if (this.currMapData.layers[layerIdx].type != "tilelayer") continue;

		  var dat = this.currMapData.layers[layerIdx].data;
		  //find what the tileIndexOffset is for this layer
		  for (var tileIDX = 0; tileIDX < dat.length; tileIDX++) {
			var tID = dat[tileIDX];
			if (tID == 0) continue;

			var tPKT = this.getTilePacket(tID);

			//test if this tile is within our world bounds
			var worldX = Math.floor(tileIDX % this.numXTiles) * this.tileSize.x;
			var worldY = Math.floor(tileIDX / this.numXTiles) * this.tileSize.y;

			var visible= this.intersectRect(	vRect,
										{top:worldY,left:worldX,bottom:worldY + this.tileSize.y,right:worldX + this.tileSize.x});
			if(!visible)	
				continue;
				
			// Nine arguments: the element, source (x,y) coordinates, source width and 
			// height (for cropping), destination (x,y) coordinates, and destination width 
			// and height (resize).
	//		ctx.fillRect(worldX,worldY,this.tileSize.x, this.tileSize.y);
			
			ctx.drawImage(tPKT.img,
							tPKT.px, tPKT.py, 
							this.tileSize.x, this.tileSize.y, 
							worldX - vRect.left, 
							worldY - vRect.top, 
							this.tileSize.x, this.tileSize.y);
			
			

		  }
		}
  }
  
});

var gMap = new TILEDmap();