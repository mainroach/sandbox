
#include "imgHelper.h"

//devil
#ifdef __cplusplus
extern "C" {
#endif
#include "./contrib/FreeImagePlus/FreeImagePlus.h"
#ifdef __cplusplus
}
#endif

#pragma comment(lib, "./contrib/FreeImagePlus/freeimageplus.lib")


static bool readFileIntoMemory(const char* pFilename, unsigned int& dataSize, char** pData)
{
   //read the input file list into memory
   FILE* f = fopen(pFilename, "rb");

	if(!f)  
      return false;


		fseek( f, 0L, SEEK_END );
		dataSize = ftell( f );
		rewind ( f );

		*pData = new char[dataSize + 1];

		const unsigned int v = (unsigned int)fread(*pData, dataSize, 1, f);
		assert(v == 1);
		if(v != 1)
		{
			delete[] *pData;
			return false;
		}

		(*pData) [dataSize] = '\0'; //string must be null terminated for RAPIDXML to work..
		fclose( f );

      return true;
}

//----------------------------------------------
bool loadImageData(const char* pFilename, ImageData& id)
{


   id.imgFileName = pFilename;
   {
		
       fipImage src_image;
         
      if (!src_image.load(pFilename, 0))
         return false;
               
      if (!src_image.convertTo32Bits())
         return false;

      if (src_image.getBitsPerPixel() != 32)
         return false;
         
      id.width = src_image.getWidth();
      id.height = src_image.getHeight();

		
		id.imgSizeInBytes = id.width * id.height * 4;
		id.pImgData = new char[id.imgSizeInBytes];
                  
		char* pDest = id.pImgData;
      for (unsigned int y = 0; y < id.height; y++)
      {
         const unsigned char* pSrc = src_image.getScanLine((WORD)(id.height - 1 - y));
			RGBAColor* pD = (RGBAColor*)pDest;
            
         for (uint x = id.width; x; x--)
         {
            RGBAColor c;
            c.r = pSrc[FI_RGBA_RED];
            c.g = pSrc[FI_RGBA_GREEN];
            c.b = pSrc[FI_RGBA_BLUE];
            c.a = pSrc[FI_RGBA_ALPHA];
            
            pSrc += 4;
            *pD++ = c;
         }
         

			pDest += (id.width*4);
      }  

		        

   }

   return true;
}


bool saveImageData(const char* pFilename, ImageData& id)
{
   fipImage dst_image(FIT_BITMAP, (WORD)id.width, (WORD)id.height, 32);
   
   for (uint y = 0; y < id.height; y++)
   {
		int idx = y* id.width * 4;
      for (uint x = 0; x < id.width; x++, idx+=4)
      {
         RGBAColor c;
         
         RGBQUAD quad;
         quad.rgbRed = id.pImgData[idx +0];
         quad.rgbGreen = id.pImgData[idx +1];
         quad.rgbBlue = id.pImgData[idx +2];
         quad.rgbReserved = id.pImgData[idx +3];
         
         dst_image.setPixelColor(x, id.height - 1 - y, &quad);
      } 
   }
   
   if (!dst_image.save(pFilename, 0))
      return false; 

   return true;
}
