#include "ImageSource.h"
#include "../System/ResLoader.h"

#include "../Platform/DxGraphDevice.h"
#include "../System/Log.h"
#include "../SDK/stc/s3tc.h"
#include "../SDK/png/include/win32/png.h"
#include "../SDK/jpeg/include/win32/jpeglib.h"
#include "../SDK/tiff/include/win32/tiffio.h"
#include "../Container/macro.h"
#include <algorithm>
#include <boost/assert.hpp>

#define CC_UNUSED_PARAM(unusedparam) (void)unusedparam
struct DDColorKey
{
	uint32_t colorSpaceLowValue;
	uint32_t colorSpaceHighValue;
};

struct DDSCaps
{
	uint32_t caps;
	uint32_t caps2;
	uint32_t caps3;
	uint32_t caps4;
};

struct DDPixelFormat
{
	uint32_t size;
	uint32_t flags;
	uint32_t fourCC;
	uint32_t RGBBitCount;
	uint32_t RBitMask;
	uint32_t GBitMask;
	uint32_t BBitMask;
	uint32_t ABitMask;
};

struct zbDDSURFACEDESC2
{
	uint32_t size;
	uint32_t flags;
	uint32_t height;
	uint32_t width;

	union
	{
		uint32_t pitch;
		uint32_t linearSize;
	} DUMMYUNIONNAMEN1;

	union
	{
		uint32_t backBufferCount;
		uint32_t depth;
	} DUMMYUNIONNAMEN5;

	union
	{
		uint32_t mipMapCount;
		uint32_t refreshRate;
		uint32_t srcVBHandle;
	} DUMMYUNIONNAMEN2;

	uint32_t alphaBitDepth;
	uint32_t reserved;
	uint32_t surface;

	union
	{
		DDColorKey ddckCKDestOverlay;
		uint32_t emptyFaceColor;
	} DUMMYUNIONNAMEN3;

	DDColorKey ddckCKDestBlt;
	DDColorKey ddckCKSrcOverlay;
	DDColorKey ddckCKSrcBlt;

	union
	{
		DDPixelFormat ddpfPixelFormat;
		uint32_t FVF;
	} DUMMYUNIONNAMEN4;

	DDSCaps ddsCaps;
	uint32_t textureStage;
};

struct S3TCTexHeader
{
	char fileCode[4];
	zbDDSURFACEDESC2 ddsd;
};

typedef struct
{
	const uint8_t * pData;
	std::size_t nSize;
	std::size_t  nOffset;
}tImageSource;

static void pngReadCallback(png_structp png_ptr, png_bytep data, png_size_t length)
{
	tImageSource* isource = (tImageSource*)png_get_io_ptr(png_ptr);
	if ((int)(isource->nOffset + length) <= isource->nSize)
	{
		memcpy(data, isource->pData + isource->nOffset, length);
		isource->nOffset += length;
	}
	else
	{
		png_error(png_ptr, "pngReaderCallback failed");
	}
}

struct MyErrorMgr
{
	struct jpeg_error_mgr pub;  /* "public" fields */
	jmp_buf setjmp_buffer;  /* for return to caller */
};
typedef struct MyErrorMgr * MyErrorPtr;
METHODDEF(void)
myErrorExit(j_common_ptr cinfo)
{
	/* cinfo->err really points to a MyErrorMgr struct, so coerce pointer */
	MyErrorPtr myerr = (MyErrorPtr)cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	/* internal message function can't show error message in some platforms, so we rewrite it here.
	* edit it if has version conflict.
	*/
	//(*cinfo->err->output_message) (cinfo);
	char buffer[JMSG_LENGTH_MAX];
	(*cinfo->err->format_message) (cinfo, buffer);
	//g_pCore->Trace_Log(LOG_ERROR,"jpeg error: %s", buffer);

	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}

static tmsize_t tiffReadProc(thandle_t fd, void* buf, tmsize_t size)
{
	tImageSource* isource = (tImageSource*)fd;
	uint8_t* ma;
	uint64 mb;
	unsigned long n;
	unsigned long o;
	tmsize_t p;
	ma = (uint8_t*)buf;
	mb = size;
	p = 0;
	while (mb > 0)
	{
		n = 0x80000000UL;
		if ((uint64)n > mb)
			n = (unsigned long)mb;


		if ((int)(isource->nOffset + n) <= isource->nSize)
		{
			memcpy(ma, isource->pData + isource->nOffset, n);
			isource->nOffset += n;
			o = n;
		}
		else
		{
			return 0;
		}

		ma += o;
		mb -= o;
		p += o;
		if (o != n)
		{
			break;
		}
	}
	return p;
}

static tmsize_t tiffWriteProc(thandle_t fd, void* buf, tmsize_t size)
{
	CC_UNUSED_PARAM(fd);
	CC_UNUSED_PARAM(buf);
	CC_UNUSED_PARAM(size);
	return 0;
}


static uint64 tiffSeekProc(thandle_t fd, uint64 off, int whence)
{
	tImageSource* isource = (tImageSource*)fd;
	uint64 ret = -1;
	do
	{
		if (whence == SEEK_SET)
		{
			IF_BREAK(off >= (uint64)isource->nSize);
			ret = isource->nOffset = (uint32)off;
		}
		else if (whence == SEEK_CUR)
		{
			IF_BREAK(isource->nOffset + off >= (uint64)isource->nSize);
			ret = isource->nOffset += (uint32)off;
		}
		else if (whence == SEEK_END)
		{
			IF_BREAK(off >= (uint64)isource->nSize);
			ret = isource->nOffset = (uint32)(isource->nSize - 1 - off);
		}
		else
		{
			IF_BREAK(off >= (uint64)isource->nSize);
			ret = isource->nOffset = (uint32)off;
		}
	} while (0);

	return ret;
}

static uint64 tiffSizeProc(thandle_t fd)
{
	tImageSource* imageSrc = (tImageSource*)fd;
	return imageSrc->nSize;
}

static int tiffCloseProc(thandle_t fd)
{
	CC_UNUSED_PARAM(fd);
	return 0;
}

static int tiffMapProc(thandle_t fd, void** base, toff_t* size)
{
	CC_UNUSED_PARAM(fd);
	CC_UNUSED_PARAM(base);
	CC_UNUSED_PARAM(size);
	return 0;
}

static void tiffUnmapProc(thandle_t fd, void* base, toff_t size)
{
	CC_UNUSED_PARAM(fd);
	CC_UNUSED_PARAM(base);
	CC_UNUSED_PARAM(size);
}

const static uint32_t makeFourCC(char ch0, char ch1, char ch2, char ch3)
{
	uint32_t fourCC = ((uint32_t)(char)(ch0) | ((uint32_t)(char)(ch1) << 8) | ((uint32_t)(char)(ch2) << 16) | ((uint32_t)(char)(ch3) << 24));
	return fourCC;
}

ImageResource::ImageResource()
	:m_fileType(ITexture::IMAGE_TYPE::IMAGE_UNKNOW), m_nWidth(0),m_nHeight(0),
	m_nDataLen(0), m_pData(nullptr), m_NumberOfMipmaps(0),
	m_renderFormat(ITexture::GetDefaultAlphaPixelFormat()),m_bHhasPremultipliedAlpha(false)
{

}

ImageResource::~ImageResource()
{
	delete[] m_pData;
}

bool ImageResource::LoadImageRes(const char* szFilePath)
{
	FILE* fp = fopen(szFilePath, "rb");
	if (nullptr == fp)
	{
		return false;
	}

	fseek(fp, 0, SEEK_END);
	std::size_t nLength = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	uint8_t* pBuffer = NEW uint8_t[(sizeof(uint8_t) * nLength)]; //(uint8_t*)malloc(sizeof(uint8_t) * nLength);
	std::size_t nReadsize = fread(pBuffer, sizeof(uint8_t), nLength, fp);
	fclose(fp);

	if (nReadsize < nLength)
	{
		pBuffer[nReadsize] = '\0';
	}

	return LoadImageData(pBuffer, nReadsize);
}

bool ImageResource::LoadImageRes(const ResIdentifierPtr& fp)
{
	fp->seekg(0, std::ios::end);
	std::size_t nLength = static_cast<std::size_t>(fp->tellg());
	fp->seekg(0, std::ios::beg);
	uint8_t* pBuffer = NEW uint8_t[(sizeof(uint8_t) * nLength)];
	fp->read(pBuffer, nLength);

	return LoadImageData(pBuffer, nLength);
}

bool ImageResource::IsCompressed()
{
	return ITexture::GetPixelFormatInfoMap().at(m_renderFormat).compressed;
}

bool ImageResource::LoadImageData(const uint8_t * data, std::size_t dataLen)
{
	if (nullptr == data || dataLen < 0)
	{
		return false;
	}

	m_fileType = QueryImageFormat(data, dataLen);
	bool bSucc =false;
	switch (m_fileType)
	{
	case ITexture::IMAGE_TYPE::IMAGE_S3TC:
		bSucc = LoadImageS3TC(data, dataLen);
		break;
	case ITexture::IMAGE_TYPE::IMAGE_PNG:
		bSucc = LoadImagePNG(data, dataLen);
		break;

	case ITexture::IMAGE_TYPE::IMAGE_TIFF:
		bSucc = LoadImageTIFF(data, dataLen);
		break;

	case ITexture::IMAGE_TYPE::IMAGE_JPN:
		bSucc = LoadImageJpg(data, dataLen);
		break;

	default:
		break;
	}

	delete[] data;
	return bSucc;
}

bool ImageResource::LoadImagePNG(const uint8_t * data, std::size_t dataLen)
{
#define PNGSIGSIZE  8
	bool ret = false;
	png_byte        header[PNGSIGSIZE] = { 0 };
	png_structp     png_ptr = 0;
	png_infop       info_ptr = 0;

	do
	{
		// png header len is 8 bytes
		IF_BREAK(dataLen < PNGSIGSIZE);

		// check the data is png or not
		memcpy(header, data, PNGSIGSIZE);
		IF_BREAK(png_sig_cmp(header, 0, PNGSIGSIZE));

		// init png_struct
		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		IF_BREAK(!png_ptr);

		// init png_info
		info_ptr = png_create_info_struct(png_ptr);
		IF_BREAK(!info_ptr);

		// set the read call back function
		tImageSource imageSource;
		imageSource.pData = (uint8_t*)data;
		imageSource.nSize = dataLen;
		imageSource.nOffset = 0;
		png_set_read_fn(png_ptr, &imageSource, pngReadCallback);

		// read png header info

		// read png file info
		png_read_info(png_ptr, info_ptr);

		m_nWidth = png_get_image_width(png_ptr, info_ptr);
		m_nHeight = png_get_image_height(png_ptr, info_ptr);
		png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
		png_uint_32 color_type = png_get_color_type(png_ptr, info_ptr);

		// force palette images to be expanded to 24-bit RGB
		// it may include alpha channel
		if (color_type == PNG_COLOR_TYPE_PALETTE)
		{
			png_set_palette_to_rgb(png_ptr);
		}
		// low-bit-depth grayscale images are to be expanded to 8 bits
		if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		{
			bit_depth = 8;
			png_set_expand_gray_1_2_4_to_8(png_ptr);
		}
		// expand any tRNS chunk data into a full alpha channel
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		{
			png_set_tRNS_to_alpha(png_ptr);
		}
		// reduce images with 16-bit samples to 8 bits
		if (bit_depth == 16)
		{
			png_set_strip_16(png_ptr);
		}

		// Expanded earlier for grayscale, now take care of palette and rgb
		if (bit_depth < 8)
		{
			png_set_packing(png_ptr);
		}
		// update info
		png_read_update_info(png_ptr, info_ptr);
		bit_depth = png_get_bit_depth(png_ptr, info_ptr);
		color_type = png_get_color_type(png_ptr, info_ptr);

		switch (color_type)
		{
		case PNG_COLOR_TYPE_GRAY:
			m_renderFormat = ITexture::PixelFormat::I8;
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			m_renderFormat = ITexture::PixelFormat::AI88;
			break;
		case PNG_COLOR_TYPE_RGB:
			m_renderFormat = ITexture::PixelFormat::RGB888;
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			m_renderFormat = ITexture::PixelFormat::RGBA8888;
			break;
		default:
			break;
		}

		// read png data
		png_size_t rowbytes;
		png_bytep* row_pointers = NEW png_bytep[m_nHeight];//(png_bytep*)malloc(sizeof(png_bytep) * m_nHeight);

		rowbytes = png_get_rowbytes(png_ptr, info_ptr);

		m_nDataLen = rowbytes * m_nHeight;
		m_pData = NEW uint8_t[(m_nDataLen * sizeof(uint8_t))];//static_cast<uint8_t*>(malloc(m_nDataLen * sizeof(uint8_t)));
		if (!m_nDataLen)
		{
			if (row_pointers != nullptr)
			{
				delete[] row_pointers;//free(row_pointers);
			}
			break;
		}

		for (unsigned short i = 0; i < m_nHeight; ++i)
		{
			row_pointers[i] = m_pData + i * rowbytes;
		}
		png_read_image(png_ptr, row_pointers);

		png_read_end(png_ptr, nullptr);

		// premultiplied alpha for RGBA8888
		if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
		{
			premultipliedAlpha();
		}
		else
		{
			m_bHhasPremultipliedAlpha = false;
		}

		if (row_pointers != nullptr)
		{
			delete[] (row_pointers);
		}

		ret = true;
	} while (0);

	if (png_ptr)
	{
		png_destroy_read_struct(&png_ptr, (info_ptr) ? &info_ptr : 0, 0);
	}
	return ret;
}

bool ImageResource::LoadImageTIFF(const uint8_t * data, std::size_t dataLen)
{
	bool ret = false;
	do
	{
		// set the read call back function
		tImageSource imageSource;
		imageSource.pData = data;
		imageSource.nSize = dataLen;
		imageSource.nOffset = 0;

		TIFF* tif = TIFFClientOpen("file.tif", "r", (thandle_t)&imageSource,
			tiffReadProc, tiffWriteProc,
			tiffSeekProc, tiffCloseProc, tiffSizeProc,
			tiffMapProc,
			tiffUnmapProc);
		IF_BREAK(nullptr == tif);

		uint32 w = 0, h = 0;
		uint16 bitsPerSample = 0, samplePerPixel = 0, planarConfig = 0;
		size_t npixels = 0;

		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
		TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
		TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplePerPixel);
		TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &planarConfig);

		npixels = w * h;

		m_renderFormat = ITexture::PixelFormat::RGBA8888;
		m_nWidth = w;
		m_nHeight = h;

		m_nDataLen =static_cast<uint32_t>( npixels * sizeof(uint32_t));
		m_pData = NEW uint8_t[(m_nDataLen * sizeof(uint8_t))];//static_cast<uint8_t*>(malloc(m_nDataLen * sizeof(uint8_t)));

		uint32_t* raster = (uint32_t*)_TIFFmalloc(npixels * sizeof(uint32_t));
		if (raster != nullptr)
		{
			if (TIFFReadRGBAImageOriented(tif, w, h, raster, ORIENTATION_TOPLEFT, 0))
			{
				/* the raster data is pre-multiplied by the alpha component
				after invoking TIFFReadRGBAImageOriented*/
				m_bHhasPremultipliedAlpha = true;

				memcpy(m_pData, raster, npixels * sizeof(uint32));
			}

			_TIFFfree(raster);
		}


		TIFFClose(tif);

		ret = true;
	} while (0);
	return ret;
}

bool ImageResource::LoadImageS3TC(const uint8_t * data, std::size_t dataLen)
{
	m_bHhasPremultipliedAlpha = false;
	const uint32_t zbFOURCC_DXT1 = makeFourCC('D', 'X', 'T', '1');
	const uint32_t zbFOURCC_DXT3 = makeFourCC('D', 'X', 'T', '3');
	const uint32_t zbFOURCC_DXT5 = makeFourCC('D', 'X', 'T', '5');

	// 载入dds文件
	S3TCTexHeader *header = (S3TCTexHeader *)data;
	uint8_t *pixelData = NEW uint8_t[((dataLen - sizeof(S3TCTexHeader)) * sizeof(uint8_t))];//static_cast<uint8_t*>(malloc((dataLen - sizeof(S3TCTexHeader)) * sizeof(uint8_t)));
	memcpy((void *)pixelData, data + sizeof(S3TCTexHeader), dataLen - sizeof(S3TCTexHeader));
	m_nWidth = header->ddsd.width;
	m_nHeight = header->ddsd.height;

	//if dds header reports 0 mipmaps, set to 1 to force correct software decoding (if needed).
	m_NumberOfMipmaps = STX_MAX(1, header->ddsd.DUMMYUNIONNAMEN2.mipMapCount);
	m_nDataLen = 0;
	int blockSize = (FOURCC_DXT1 == header->ddsd.DUMMYUNIONNAMEN4.ddpfPixelFormat.fourCC) ? 8 : 16;

	int width = m_nWidth;
	int height = m_nHeight;
	// 压缩数据长度
	if (Configuration::IssupportsS3TC())  
	{
		m_nDataLen = static_cast<uint32_t>(dataLen - sizeof(S3TCTexHeader));
		m_pData = static_cast<uint8_t*>(malloc(m_nDataLen * sizeof(uint8_t)));
		memcpy((void *)m_pData, (void *)pixelData, m_nDataLen);
	}
	// 减压数据长度
	else                                               
	{
		for (int i = 0; i < m_NumberOfMipmaps && (width || height); ++i)
		{
			if (width == 0) width = 1;
			if (height == 0) height = 1;

			m_nDataLen += (height * width * 4);
			width >>= 1;
			height >>= 1;
		}
		m_pData = NEW uint8_t[(m_nDataLen * sizeof(uint8_t))];//static_cast<uint8_t*>(malloc(m_nDataLen * sizeof(uint8_t)));
	}

	//如果硬件支持S3TC，在加载MIPMAP之前设置PixelFrad格式，以支持非MIPMAP纹理
	if (Configuration::IssupportsS3TC())
	{
		if (zbFOURCC_DXT1 == header->ddsd.DUMMYUNIONNAMEN4.ddpfPixelFormat.fourCC)
		{
			m_renderFormat = ITexture::PixelFormat::S3TC_DXT1;
		}
		else if (zbFOURCC_DXT3 == header->ddsd.DUMMYUNIONNAMEN4.ddpfPixelFormat.fourCC)
		{
			m_renderFormat = ITexture::PixelFormat::S3TC_DXT3;
		}
		else if (zbFOURCC_DXT5 == header->ddsd.DUMMYUNIONNAMEN4.ddpfPixelFormat.fourCC)
		{
			m_renderFormat = ITexture::PixelFormat::S3TC_DXT5;
		}
	}
	else
	{
		m_renderFormat = ITexture::PixelFormat::RGBA8888;
	}

	// 载入图
	int encodeOffset = 0;
	int decodeOffset = 0;
	width = m_nWidth;  height = m_nHeight;
	for (int i = 0; i < m_NumberOfMipmaps && (width || height); ++i)
	{
		if (width == 0) width = 1;
		if (height == 0) height = 1;
		int size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;

		if (Configuration::IssupportsS3TC())
		{   //decode texture through hardware
			m_Mipmaps[i].pAddress = (uint8_t *)m_pData + encodeOffset;
			m_Mipmaps[i].nlength = size;
		}
		else
		{
			//g_pCore->Trace_Log(nType, szMsg);

			int bytePerPixel = 4;
			unsigned int stride = width * bytePerPixel;
			std::vector<uint8_t> decodeImageData(stride * height);
			if (FOURCC_DXT1 == header->ddsd.DUMMYUNIONNAMEN4.ddpfPixelFormat.fourCC)
			{
				s3tc_decode(pixelData + encodeOffset, &decodeImageData[0], width, height, S3TCDecodeFlag::DXT1);
			}
			else if (FOURCC_DXT3 == header->ddsd.DUMMYUNIONNAMEN4.ddpfPixelFormat.fourCC)
			{
				s3tc_decode(pixelData + encodeOffset, &decodeImageData[0], width, height, S3TCDecodeFlag::DXT3);
			}
			else if (FOURCC_DXT5 == header->ddsd.DUMMYUNIONNAMEN4.ddpfPixelFormat.fourCC)
			{
				s3tc_decode(pixelData + encodeOffset, &decodeImageData[0], width, height, S3TCDecodeFlag::DXT5);
			}

			m_Mipmaps[i].pAddress = (uint8_t *)m_pData + decodeOffset;
			m_Mipmaps[i].nlength = (stride * height);
			memcpy((void *)m_Mipmaps[i].pAddress, (void *)&decodeImageData[0], m_Mipmaps[i].nlength);
			decodeOffset += stride * height;
		}

		encodeOffset += size;
		width >>= 1;
		height >>= 1;
	}

	if (pixelData != nullptr)
	{
		delete [] (pixelData);
	};
	return true;
}

bool ImageResource::LoadImageJpg(const uint8_t * data, std::size_t dataLen)
{
	/* these are standard libjpeg structures for reading(decompression) */
	struct jpeg_decompress_struct cinfo;
	/* We use our private extension JPEG error handler.
	* Note that this struct must live as long as the main JPEG parameter
	* struct, to avoid dangling-pointer problems.
	*/
	struct MyErrorMgr jerr;
	/* libjpeg data structure for storing one row, that is, scanline of an image */
	JSAMPROW row_pointer[1] = { 0 };
	unsigned long location = 0;
	bool ret = false;

	do
	{
		/* We set up the normal JPEG error routines, then override error_exit. */
		cinfo.err = jpeg_std_error(&jerr.pub);
		jerr.pub.error_exit = myErrorExit;
		/* Establish the setjmp return context for MyErrorExit to use. */
		if (setjmp(jerr.setjmp_buffer))
		{
			/* If we get here, the JPEG code has signaled an error.
			* We need to clean up the JPEG object, close the input file, and return.
			*/
			jpeg_destroy_decompress(&cinfo);
			break;
		}

		/* setup decompression process and source, then read JPEG header */
		jpeg_create_decompress(&cinfo);

#ifndef CC_TARGET_QT5
		jpeg_mem_src(&cinfo, const_cast<uint8_t*>(data), dataLen);
#endif /* CC_TARGET_QT5 */

		/* reading the image header which contains image information */
#if (JPEG_LIB_VERSION >= 90)
		// libjpeg 0.9 adds stricter types.
		jpeg_read_header(&cinfo, TRUE);
#else
		jpeg_read_header(&cinfo, TRUE);
#endif
		// we only support RGB or grayscale
		if (cinfo.jpeg_color_space == JCS_GRAYSCALE)
		{
			m_renderFormat = ITexture::PixelFormat::I8;
		}
		else
		{
			cinfo.out_color_space = JCS_RGB;
			m_renderFormat = ITexture::PixelFormat::RGB888;
		}

		/* Start decompression jpeg here */
		jpeg_start_decompress(&cinfo);

		/* init image info */
		m_nWidth = cinfo.output_width;
		m_nHeight = cinfo.output_height;
		m_bHhasPremultipliedAlpha = false;

		m_nDataLen = cinfo.output_width*cinfo.output_height*cinfo.output_components;
		m_pData = NEW uint8_t[(m_nDataLen * sizeof(uint8_t))];//static_cast<uint8_t*>(malloc(m_nDataLen * sizeof(uint8_t)));
		IF_BREAK(!m_pData);

		/* now actually read the jpeg into the raw buffer */
		/* read one scan line at a time */
		while (cinfo.output_scanline < cinfo.output_height)
		{
			row_pointer[0] = m_pData + location;
			location += cinfo.output_width*cinfo.output_components;
			jpeg_read_scanlines(&cinfo, row_pointer, 1);
		}

		/* When read image file with broken data, jpeg_finish_decompress() may cause error.
		* Besides, jpeg_destroy_decompress() shall deallocate and release all memory associated
		* with the decompression object.
		* So it doesn't need to call jpeg_finish_decompress().
		*/
		//jpeg_finish_decompress( &cinfo );
		jpeg_destroy_decompress(&cinfo);
		/* wrap up decompression, destroy objects, free pointers and close open files */
		ret = true;
	} while (0);

	return ret;
}

ITexture::IMAGE_TYPE ImageResource::QueryImageFormat(const uint8_t * data, std::size_t dataLen)
{
	if (isPng(data, dataLen))
		return ITexture::IMAGE_TYPE::IMAGE_PNG;
	else if (isTiff(data, dataLen))
		return ITexture::IMAGE_TYPE::IMAGE_TIFF;
	else if (isS3TC(data, dataLen))
		return ITexture::IMAGE_TYPE::IMAGE_S3TC;
	else if (isJpg(data, dataLen))
		return ITexture::IMAGE_TYPE::IMAGE_JPN;
	else
	{
		//g_pCore->Trace_Log2(LOG_WARING, "zb:can't detect image format");
		return ITexture::IMAGE_TYPE::IMAGE_UNKNOW;
	}
}

bool ImageResource::isPng(const uint8_t * data, std::size_t dataLen)
{
	if (dataLen <= 8)
	{
		return false;
	}

	static const uint8_t PNG_SIGNATURE[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
	return memcmp(PNG_SIGNATURE, data, sizeof(PNG_SIGNATURE)) == 0;
}

bool ImageResource::isTiff(const uint8_t * data, std::size_t dataLen)
{
	if (dataLen <= 4)
	{
		return false;
	}

	static const char* TIFF_II = "II";
	static const char* TIFF_MM = "MM";
	return (memcmp(data, TIFF_II, 2) == 0 && *(static_cast<const uint8_t*>(data) + 2) == 42 && *(static_cast<const uint8_t*>(data) + 3) == 0) ||
		(memcmp(data, TIFF_MM, 2) == 0 && *(static_cast<const uint8_t*>(data) + 2) == 0 && *(static_cast<const uint8_t*>(data) + 3) == 42);
}

bool ImageResource::isS3TC(const uint8_t * data, std::size_t dataLen)
{
	S3TCTexHeader *header = (S3TCTexHeader *)data;
	if (strncmp(header->fileCode, "DDS", 3) != 0)
	{
		return false;
	}
	return true;
}

bool ImageResource::isJpg(const uint8_t * data, std::size_t dataLen)
{
	if (dataLen <= 4)
	{
		return false;
	}

	static const uint8_t JPG_SOI[] = { 0xFF, 0xD8 };
	return memcmp(data, JPG_SOI, 2) == 0;
}

void ImageResource::premultipliedAlpha()
{
	BOOST_ASSERT_MSG(m_renderFormat == ITexture::PixelFormat::RGBA8888, "The pixel format should be RGBA8888!");
	unsigned int* fourBytes = (unsigned int*)m_pData;
	for (int i = 0; i < m_nWidth * m_nHeight; i++)
	{
		uint8_t* p = m_pData + i * 4;
		fourBytes[i] = CC_RGB_PREMULTIPLY_ALPHA(p[0], p[1], p[2], p[3]);
	}

	m_bHhasPremultipliedAlpha = true;
}

bool Configuration::IssupportsS3TC()
{
	return true;
}
