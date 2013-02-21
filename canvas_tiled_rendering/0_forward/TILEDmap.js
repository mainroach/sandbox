
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
  tileImg:null,
  imgLoadCount:0,
  preCacheCanvasArray:null,
  fullyLoaded:false,
  //---------------------------
  load: function (map) 
  {
	
	xhrGet(map, false,function(data)
	{
		gMap.parseMapJSON(data.response);
			
	});
  },
  //---------------------------
  _tempInput:null,
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
						gMap.fullyLoaded = true;
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
	intersectRect:function (r1, r2) {
	return !(r2.left > r1.right || 
           r2.right < r1.left || 
           r2.top > r1.bottom ||
           r2.bottom < r1.top);
	},
  //---------------------------
  draw: function (ctx) 
  { //
	if(!this.fullyLoaded) return;
	

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
        if ((worldX + this.tileSize.x) < this.viewRect.x || (worldY + this.tileSize.y) < this.viewRect.y || worldX > this.viewRect.x + this.viewRect.w || worldY > this.viewRect.y + this.viewRect.h) continue;

        //adjust all the visible tiles to draw at canvas origin.
        worldX -= this.viewRect.x;
        worldY -= this.viewRect.y;

        // Nine arguments: the element, source (x,y) coordinates, source width and 
        // height (for cropping), destination (x,y) coordinates, and destination width 
        // and height (resize).
        ctx.drawImage(tPKT.img, tPKT.px, tPKT.py, this.tileSize.x, this.tileSize.y, worldX, worldY, this.tileSize.x, this.tileSize.y);

      }
    }
  },
});

var gMap = new TILEDmap();