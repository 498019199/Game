#include "ITexture.h"
#include "../System/ResLoader.h"
#include "../Tool/ImageSource.h"
#include "../Core/CHelper.h"
#include "../Util/UtilTool.h"
#include "../Container/Hash.h"

#include <boost/assert.hpp>

static ITexture::PixelFormat g_defaultPixelFormat = ITexture::PixelFormat::DEFAULT;
typedef ITexture::PixelFormatInfoMap::value_type PixelFormatInfoMapValue;
static const PixelFormatInfoMapValue TexturePixelFormatInfoTablesValue[] =
{
	PixelFormatInfoMapValue(ITexture::PixelFormat::BGRA8888, ITexture::PixelFormatInfo(32, false, true)),
	PixelFormatInfoMapValue(ITexture::PixelFormat::RGBA8888, ITexture::PixelFormatInfo(32, false, true)),
	PixelFormatInfoMapValue(ITexture::PixelFormat::RGBA4444, ITexture::PixelFormatInfo(16, false, true)),
	PixelFormatInfoMapValue(ITexture::PixelFormat::RGB5A1, ITexture::PixelFormatInfo(16, false, true)),
	PixelFormatInfoMapValue(ITexture::PixelFormat::RGB565, ITexture::PixelFormatInfo(16, false, false)),
	PixelFormatInfoMapValue(ITexture::PixelFormat::RGB888, ITexture::PixelFormatInfo(24, false, false)),
	PixelFormatInfoMapValue(ITexture::PixelFormat::A8, ITexture::PixelFormatInfo(8, false, false)),
	PixelFormatInfoMapValue(ITexture::PixelFormat::I8, ITexture::PixelFormatInfo(8, false, false)),
	PixelFormatInfoMapValue(ITexture::PixelFormat::AI88, ITexture::PixelFormatInfo(16, false, true)),
	PixelFormatInfoMapValue(ITexture::PixelFormat::ETC, ITexture::PixelFormatInfo(4, true, false)),
	PixelFormatInfoMapValue(ITexture::PixelFormat::S3TC_DXT1, ITexture::PixelFormatInfo(4, true, false)),
	PixelFormatInfoMapValue(ITexture::PixelFormat::S3TC_DXT3, ITexture::PixelFormatInfo(8, true, false)),
	PixelFormatInfoMapValue(ITexture::PixelFormat::S3TC_DXT5, ITexture::PixelFormatInfo(8, true, false)),
	PixelFormatInfoMapValue(ITexture::PixelFormat::ATC_RGB, ITexture::PixelFormatInfo(4, true, false)),
	PixelFormatInfoMapValue(ITexture::PixelFormat::ATC_EXPLICIT_ALPHA, ITexture::PixelFormatInfo(8, true, false)),
	PixelFormatInfoMapValue(ITexture::PixelFormat::ATC_INTERPOLATED_ALPHA, ITexture::PixelFormatInfo(8, true, false)),
};

const ITexture::PixelFormatInfoMap ITexture::m_pixelFormatInfoTables(TexturePixelFormatInfoTablesValue,
	TexturePixelFormatInfoTablesValue + sizeof(TexturePixelFormatInfoTablesValue) / sizeof(TexturePixelFormatInfoTablesValue[0]));

void ITexture::setDefaultAlphaPixelFormat(PixelFormat format)
{
	g_defaultPixelFormat = format;
}

ITexture::PixelFormat ITexture::GetDefaultAlphaPixelFormat()
{
	return g_defaultPixelFormat;
}

ITexture::ITexture(const char* szTexure)
	:m_strTexture(szTexure),m_pData(nullptr), m_nWidth(0),m_nHight(0)
{}

ITexture::~ITexture()
{
	delete[] (m_pData);
}

bool ITexture::AddImage(ResIdentifierPtr fp)
{
	ImagePtr pImagePtr = MakeSharedPtr<ImageResource>();
	pImagePtr->LoadImageRes(fp);

	int imageWidth = pImagePtr->GetWidth();
	int imageHeight = pImagePtr->GetHeight();
	float2 imageSize = float2((float)imageWidth, (float)imageHeight);
	uint8_t*   tempData = pImagePtr->GetData();
	PixelFormat format = g_defaultPixelFormat;
	PixelFormat pixelFormat = ((PixelFormat::NONE == format) || (PixelFormat::AUTO == format)) ? pImagePtr->getRenderFormat() : format;
	PixelFormat renderFormat = pImagePtr->getRenderFormat();
	uint32_t tempDataLen = pImagePtr->GetDataLen();

	if (pImagePtr->getNumberOfMipmaps() > 1)
	{
		if (pixelFormat != pImagePtr->getRenderFormat())
		{
			//CCLOG("cocos2d: WARNING: This image has more than 1 mipmaps and we will not convert the data format");
		}

		uint8_t* outTempData = nullptr;
		uint32_t outTempDataLen = 0;
		pixelFormat = convertDataToFormat(tempData, tempDataLen, renderFormat, pixelFormat, &outTempData, &outTempDataLen);
		initWithData(outTempData, outTempDataLen, pixelFormat, imageWidth, imageHeight, imageSize);

		if (outTempData != nullptr && outTempData != tempData)
		{
			m_pData = outTempData;
			m_DataLen = outTempDataLen;
		}
		m_hasPremultipliedAlpha = pImagePtr->hasPremultipliedAlpha();
		return true;
	}
	else if (pImagePtr->IsCompressed())
	{
		if (pixelFormat != pImagePtr->getRenderFormat())
		{
			//g_pCore->Trace2("cocos2d: WARNING: This image is compressed and we cann't convert it for now");
		}
		initWithData(tempData, tempDataLen, pImagePtr->getRenderFormat(), imageWidth, imageHeight, imageSize);
		return true;
	}

	return false;
}

const ITexture::PixelFormatInfoMap& ITexture::GetPixelFormatInfoMap()
{
	return m_pixelFormatInfoTables;
}

Color ITexture::GetTextureColor(float fu, float fv, float z, float fMaxZ)
{
	Color color(0,0,0,0);
	uint32_t* data = (reinterpret_cast<uint32_t*>(m_Mipmaps[0].pAddress));
	int nWidth = Context::Instance()->GetWidth();
	int nHeight = Context::Instance()->GetHeight();

	if (0)
	{
		int nTmipLevels = MathLib::RoundToInt(MathLib::Log(nWidth * 1.f));
		int nMipLevel = MathLib::RoundToInt(nTmipLevels * (z / fMaxZ));
		if (nMipLevel > nTmipLevels) nMipLevel = nTmipLevels;
		if (nMipLevel > MIPMAP_MAX) nMipLevel = MIPMAP_MAX;
		data = (reinterpret_cast<uint32_t*>(m_Mipmaps[nMipLevel].pAddress));
		for (int ts = 0; ts < nMipLevel; ts++)
		{
			nWidth = nWidth >> 1;
			nHeight = nHeight >> 1;
		}
	}

	// wrap ·½Ê½
	fu = (fu - static_cast<int>(fu)) * (nWidth - 1);
	fv = (fv - static_cast<int>(fv)) * (nHeight - 1);
	int uint = static_cast<int>(fu);
	int vint = static_cast<int>(fv);
	int uint_pls_1 = uint + 1;
	int vint_pls_1 = vint + 1;
	uint_pls_1 = MathLib::Clamp(uint_pls_1, 0, nWidth - 1);
	vint_pls_1 = MathLib::Clamp(vint_pls_1, 0, nHeight - 1);

	int textel00, textel10, textel01, textel11;
	textel00 = data[(vint + 0)*nWidth + (uint + 0)];
	textel10 = data[(vint_pls_1)*nWidth + (uint + 0)];
	textel01 = data[(vint + 0)*nWidth + (uint_pls_1)];
	textel11 = data[(vint_pls_1)*nWidth + (uint_pls_1)];

	int textel00_a = (textel00 >> 24) & 0xff;
	int textel00_r = (textel00 >> 16) & 0xff;
	int textel00_g = (textel00 >> 8) & 0xff;
	int textel00_b = textel00 & 0xff;

	int textel10_a = (textel10 >> 24) & 0xff;
	int textel10_r = (textel10 >> 16) & 0xff;
	int textel10_g = (textel10 >> 8) & 0xff;
	int textel10_b = textel10 & 0xff;

	int textel01_a = (textel01 >> 24) & 0xff;
	int textel01_r = (textel01 >> 16) & 0xff;
	int textel01_g = (textel01 >> 8) & 0xff;
	int textel01_b = textel01 & 0xff;

	int textel11_a = (textel11 >> 24) & 0xff;
	int textel11_r = (textel11 >> 16) & 0xff;
	int textel11_g = (textel11 >> 8) & 0xff;
	int textel11_b = textel11 & 0xff;

	float dtu = fu - (float)uint;
	float dtv = fv - (float)vint;
	float one_minus_dtu = 1.0f - dtu;
	float one_minus_dtv = 1.0f - dtv;
	float one_minus_dtu_x_one_minus_dtv = (one_minus_dtu) * (one_minus_dtv);
	float dtu_x_one_minus_dtv = (dtu) * (one_minus_dtv);
	float dtu_x_dtv = (dtu) * (dtv);
	float one_minus_dtu_x_dtv = (one_minus_dtu) * (dtv);

	color.a() = one_minus_dtu_x_one_minus_dtv * textel00_a +
		dtu_x_one_minus_dtv * textel01_a +
		dtu_x_dtv * textel11_a +
		one_minus_dtu_x_dtv * textel10_a;

	color.r() = one_minus_dtu_x_one_minus_dtv * textel00_r +
		dtu_x_one_minus_dtv * textel01_r +
		dtu_x_dtv * textel11_r +
		one_minus_dtu_x_dtv * textel10_r;

	color.g() = one_minus_dtu_x_one_minus_dtv * textel00_g +
		dtu_x_one_minus_dtv * textel01_g +
		dtu_x_dtv * textel11_g +
		one_minus_dtu_x_dtv * textel10_g;

	color.b() = one_minus_dtu_x_one_minus_dtv * textel00_b +
		dtu_x_one_minus_dtv * textel01_b +
		dtu_x_dtv * textel11_b +
		one_minus_dtu_x_dtv * textel10_b;
	return color;
}

//////////////////////////////////////////////////////////////////////////
//convertor function

// IIIIIIII -> RRRRRRRRGGGGGGGGGBBBBBBBB
void ITexture::convertI8ToRGB888(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	for (uint32_t i = 0; i < dataLen; ++i)
	{
		*outData++ = data[i];     //R
		*outData++ = data[i];     //G
		*outData++ = data[i];     //B
	}
}

// IIIIIIIIAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBB
void ITexture::convertAI88ToRGB888(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	for (uint32_t i = 0, l = dataLen - 1; i < l; i += 2)
	{
		*outData++ = data[i];     //R
		*outData++ = data[i];     //G
		*outData++ = data[i];     //B
	}
}

// IIIIIIII -> RRRRRRRRGGGGGGGGGBBBBBBBBAAAAAAAA
void ITexture::convertI8ToRGBA8888(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	for (uint32_t i = 0; i < dataLen; ++i)
	{
		*outData++ = data[i];     //R
		*outData++ = data[i];     //G
		*outData++ = data[i];     //B
		*outData++ = 0xFF;        //A
	}
}

// IIIIIIIIAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA
void ITexture::convertAI88ToRGBA8888(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	for (uint32_t i = 0, l = dataLen - 1; i < l; i += 2)
	{
		*outData++ = data[i];     //R
		*outData++ = data[i];     //G
		*outData++ = data[i];     //B
		*outData++ = data[i + 1]; //A
	}
}

// IIIIIIII -> RRRRRGGGGGGBBBBB
void ITexture::convertI8ToRGB565(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (uint32_t i = 0; i < dataLen; ++i)
	{
		*out16++ = (data[i] & 0x00F8) << 8    //R
			| (data[i] & 0x00FC) << 3         //G
			| (data[i] & 0x00F8) >> 3;        //B
	}
}

// IIIIIIIIAAAAAAAA -> RRRRRGGGGGGBBBBB
void ITexture::convertAI88ToRGB565(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (uint32_t i = 0, l = dataLen - 1; i < l; i += 2)
	{
		*out16++ = (data[i] & 0x00F8) << 8    //R
			| (data[i] & 0x00FC) << 3         //G
			| (data[i] & 0x00F8) >> 3;        //B
	}
}

// IIIIIIII -> RRRRGGGGBBBBAAAA
void ITexture::convertI8ToRGBA4444(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (uint32_t i = 0; i < dataLen; ++i)
	{
		*out16++ = (data[i] & 0x00F0) << 8    //R
			| (data[i] & 0x00F0) << 4             //G
			| (data[i] & 0x00F0)                  //B
			| 0x000F;                             //A
	}
}

// IIIIIIIIAAAAAAAA -> RRRRGGGGBBBBAAAA
void ITexture::convertAI88ToRGBA4444(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (uint32_t i = 0, l = dataLen - 1; i < l; i += 2)
	{
		*out16++ = (data[i] & 0x00F0) << 8    //R
			| (data[i] & 0x00F0) << 4             //G
			| (data[i] & 0x00F0)                  //B
			| (data[i + 1] & 0x00F0) >> 4;          //A
	}
}

// IIIIIIII -> RRRRRGGGGGBBBBBA
void ITexture::convertI8ToRGB5A1(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (uint32_t i = 0; i < dataLen; ++i)
	{
		*out16++ = (data[i] & 0x00F8) << 8    //R
			| (data[i] & 0x00F8) << 3         //G
			| (data[i] & 0x00F8) >> 2         //B
			| 0x0001;                         //A
	}
}

// IIIIIIIIAAAAAAAA -> RRRRRGGGGGBBBBBA
void ITexture::convertAI88ToRGB5A1(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (uint32_t i = 0, l = dataLen - 1; i < l; i += 2)
	{
		*out16++ = (data[i] & 0x00F8) << 8    //R
			| (data[i] & 0x00F8) << 3         //G
			| (data[i] & 0x00F8) >> 2         //B
			| (data[i + 1] & 0x0080) >> 7;    //A
	}
}

// IIIIIIII -> IIIIIIIIAAAAAAAA
void ITexture::convertI8ToAI88(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (uint32_t i = 0; i < dataLen; ++i)
	{
		*out16++ = 0xFF00     //A
			| data[i];            //I
	}
}

// IIIIIIIIAAAAAAAA -> AAAAAAAA
void ITexture::convertAI88ToA8(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	for (uint32_t i = 1; i < dataLen; i += 2)
	{
		*outData++ = data[i]; //A
	}
}

// IIIIIIIIAAAAAAAA -> IIIIIIII
void ITexture::convertAI88ToI8(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	for (uint32_t i = 0, l = dataLen - 1; i < l; i += 2)
	{
		*outData++ = data[i]; //R
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA
void ITexture::convertRGB888ToRGBA8888(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	for (uint32_t i = 0, l = dataLen - 2; i < l; i += 3)
	{
		*outData++ = data[i];         //R
		*outData++ = data[i + 1];     //G
		*outData++ = data[i + 2];     //B
		*outData++ = 0xFF;            //A
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBB
void ITexture::convertRGBA8888ToRGB888(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	for (uint32_t i = 0, l = dataLen - 3; i < l; i += 4)
	{
		*outData++ = data[i];         //R
		*outData++ = data[i + 1];     //G
		*outData++ = data[i + 2];     //B
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGGBBBBB
void ITexture::convertRGB888ToRGB565(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (uint32_t i = 0, l = dataLen - 2; i < l; i += 3)
	{
		*out16++ = (data[i] & 0x00F8) << 8    //R
			| (data[i + 1] & 0x00FC) << 3     //G
			| (data[i + 2] & 0x00F8) >> 3;    //B
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> RRRRRGGGGGGBBBBB
void ITexture::convertRGBA8888ToRGB565(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (uint32_t i = 0, l = dataLen - 3; i < l; i += 4)
	{
		*out16++ = (data[i] & 0x00F8) << 8    //R
			| (data[i + 1] & 0x00FC) << 3     //G
			| (data[i + 2] & 0x00F8) >> 3;    //B
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> IIIIIIII
void ITexture::convertRGB888ToI8(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	for (uint32_t i = 0, l = dataLen - 2; i < l; i += 3)
	{
		*outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> IIIIIIII
void ITexture::convertRGBA8888ToI8(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	for (uint32_t i = 0, l = dataLen - 3; i < l; i += 4)
	{
		*outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> AAAAAAAA
void ITexture::convertRGBA8888ToA8(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	for (uint32_t i = 0, l = dataLen - 3; i < l; i += 4)
	{
		*outData++ = data[i + 3]; //A
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> IIIIIIIIAAAAAAAA
void ITexture::convertRGB888ToAI88(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	for (uint32_t i = 0, l = dataLen - 2; i < l; i += 3)
	{
		*outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
		*outData++ = 0xFF;
	}
}


// RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> IIIIIIIIAAAAAAAA
void ITexture::convertRGBA8888ToAI88(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	for (uint32_t i = 0, l = dataLen - 3; i < l; i += 4)
	{
		*outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
		*outData++ = data[i + 3];
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRGGGGBBBBAAAA
void ITexture::convertRGB888ToRGBA4444(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (uint32_t i = 0, l = dataLen - 2; i < l; i += 3)
	{
		*out16++ = ((data[i] & 0x00F0) << 8           //R
			| (data[i + 1] & 0x00F0) << 4     //G
			| (data[i + 2] & 0xF0)            //B
			| 0x0F);                         //A
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> RRRRGGGGBBBBAAAA
void ITexture::convertRGBA8888ToRGBA4444(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (uint32_t i = 0, l = dataLen - 3; i < l; i += 4)
	{
		*out16++ = (data[i] & 0x00F0) << 8    //R
			| (data[i + 1] & 0x00F0) << 4         //G
			| (data[i + 2] & 0xF0)                //B
			| (data[i + 3] & 0xF0) >> 4;         //A
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGBBBBBA
void ITexture::convertRGB888ToRGB5A1(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (uint32_t i = 0, l = dataLen - 2; i < l; i += 3)
	{
		*out16++ = (data[i] & 0x00F8) << 8    //R
			| (data[i + 1] & 0x00F8) << 3     //G
			| (data[i + 2] & 0x00F8) >> 2     //B
			| 0x01;                          //A
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGBBBBBA
void ITexture::convertRGBA8888ToRGB5A1(const uint8_t* data, uint32_t dataLen, uint8_t* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (uint32_t i = 0, l = dataLen - 2; i < l; i += 4)
	{
		*out16++ = (data[i] & 0x00F8) << 8    //R
			| (data[i + 1] & 0x00F8) << 3     //G
			| (data[i + 2] & 0x00F8) >> 2     //B
			| (data[i + 3] & 0x0080) >> 7;   //A
	}
}

ITexture::PixelFormat ITexture::convertI8ToFormat(const uint8_t* data, uint32_t dataLen, PixelFormat format, uint8_t** outData, uint32_t* outDataLen)
{
	switch (format)
	{
	case PixelFormat::RGBA8888:
		*outDataLen = dataLen * 4;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))];//NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertI8ToRGBA8888(data, dataLen, *outData);
		break;
	case PixelFormat::RGB888:
		*outDataLen = dataLen * 3;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertI8ToRGB888(data, dataLen, *outData);
		break;
	case PixelFormat::RGB565:
		*outDataLen = dataLen * 2;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertI8ToRGB565(data, dataLen, *outData);
		break;
	case PixelFormat::AI88:
		*outDataLen = dataLen * 2;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertI8ToAI88(data, dataLen, *outData);
		break;
	case PixelFormat::RGBA4444:
		*outDataLen = dataLen * 2;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertI8ToRGBA4444(data, dataLen, *outData);
		break;
	case PixelFormat::RGB5A1:
		*outDataLen = dataLen * 2;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertI8ToRGB5A1(data, dataLen, *outData);
		break;
	default:
		// unsupported conversion or don't need to convert
		if (format != PixelFormat::AUTO && format != PixelFormat::I8)
		{
			//g_pCore->Trace2("Can not convert image format PixelFormat::I8 to format ID:%d, we will use it's origin format PixelFormat::I8", format);
		}

		*outData = (uint8_t*)data;
		*outDataLen = dataLen;
		return PixelFormat::I8;
	}

	return format;
}

ITexture::PixelFormat ITexture::convertAI88ToFormat(const uint8_t* data, uint32_t dataLen, PixelFormat format, uint8_t** outData, uint32_t* outDataLen)
{
	switch (format)
	{
	case PixelFormat::RGBA8888:
		*outDataLen = dataLen * 2;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertAI88ToRGBA8888(data, dataLen, *outData);
		break;
	case PixelFormat::RGB888:
		*outDataLen = dataLen / 2 * 3;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertAI88ToRGB888(data, dataLen, *outData);
		break;
	case PixelFormat::RGB565:
		*outDataLen = dataLen;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertAI88ToRGB565(data, dataLen, *outData);
		break;
	case PixelFormat::A8:
		*outDataLen = dataLen / 2;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertAI88ToA8(data, dataLen, *outData);
		break;
	case PixelFormat::I8:
		*outDataLen = dataLen / 2;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertAI88ToI8(data, dataLen, *outData);
		break;
	case PixelFormat::RGBA4444:
		*outDataLen = dataLen;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertAI88ToRGBA4444(data, dataLen, *outData);
		break;
	case PixelFormat::RGB5A1:
		*outDataLen = dataLen;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertAI88ToRGB5A1(data, dataLen, *outData);
		break;
	default:
		// unsupported conversion or don't need to convert
		if (format != PixelFormat::AUTO && format != PixelFormat::AI88)
		{
			//g_pCore->Trace2("Can not convert image format PixelFormat::AI88 to format ID:%d, we will use it's origin format PixelFormat::AI88", format);
		}

		*outData = (uint8_t*)data;
		*outDataLen = dataLen;
		return PixelFormat::AI88;
		break;
	}

	return format;
}

ITexture::PixelFormat ITexture::convertRGB888ToFormat(const uint8_t* data, uint32_t dataLen, PixelFormat format, uint8_t** outData, uint32_t* outDataLen)
{
	switch (format)
	{
	case PixelFormat::RGBA8888:
		*outDataLen = dataLen / 3 * 4;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))];// NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertRGB888ToRGBA8888(data, dataLen, *outData);
		break;
	case PixelFormat::RGB565:
		*outDataLen = dataLen / 3 * 2;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertRGB888ToRGB565(data, dataLen, *outData);
		break;
	case PixelFormat::I8:
		*outDataLen = dataLen / 3;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertRGB888ToI8(data, dataLen, *outData);
		break;
	case PixelFormat::AI88:
		*outDataLen = dataLen / 3 * 2;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertRGB888ToAI88(data, dataLen, *outData);
		break;
	case PixelFormat::RGBA4444:
		*outDataLen = dataLen / 3 * 2;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertRGB888ToRGBA4444(data, dataLen, *outData);
		break;
	case PixelFormat::RGB5A1:
		*outDataLen = dataLen;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertRGB888ToRGB5A1(data, dataLen, *outData);
		break;
	default:
		// unsupported conversion or don't need to convert
		if (format != PixelFormat::AUTO && format != PixelFormat::RGB888)
		{
			//g_pCore->Trace2("Can not convert image format PixelFormat::RGB888 to format ID:%d, we will use it's origin format PixelFormat::RGB888", format);
		}

		*outData = (uint8_t*)data;
		*outDataLen = dataLen;
		return PixelFormat::RGB888;
	}
	return format;
}

ITexture::PixelFormat ITexture::convertRGBA8888ToFormat(const uint8_t* data, uint32_t dataLen, PixelFormat format, uint8_t** outData, uint32_t* outDataLen)
{

	switch (format)
	{
	case PixelFormat::RGB888:
		*outDataLen = dataLen / 4 * 3;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertRGBA8888ToRGB888(data, dataLen, *outData);
		break;
	case PixelFormat::RGB565:
		*outDataLen = dataLen / 2;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertRGBA8888ToRGB565(data, dataLen, *outData);
		break;
	case PixelFormat::A8:
		*outDataLen = dataLen / 4;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertRGBA8888ToA8(data, dataLen, *outData);
		break;
	case PixelFormat::I8:
		*outDataLen = dataLen / 4;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertRGBA8888ToI8(data, dataLen, *outData);
		break;
	case PixelFormat::AI88:
		*outDataLen = dataLen / 2;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertRGBA8888ToAI88(data, dataLen, *outData);
		break;
	case PixelFormat::RGBA4444:
		*outDataLen = dataLen / 2;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertRGBA8888ToRGBA4444(data, dataLen, *outData);
		break;
	case PixelFormat::RGB5A1:
		*outDataLen = dataLen / 2;
		*outData = NEW uint8_t[(sizeof(uint8_t) * (*outDataLen))]; //(uint8_t*)malloc(sizeof(uint8_t) * (*outDataLen));
		convertRGBA8888ToRGB5A1(data, dataLen, *outData);
		break;
	default:
		// unsupported conversion or don't need to convert
		if (format != PixelFormat::AUTO && format != PixelFormat::RGBA8888)
		{
			//g_pCore->Trace2("Can not convert image format PixelFormat::RGBA8888 to format ID:%d, we will use it's origin format PixelFormat::RGBA8888", format);
		}

		*outData = (uint8_t*)data;
		*outDataLen = dataLen;
		return PixelFormat::RGBA8888;
	}

	return format;
}

bool ITexture::initWithData(const void *data, uint32_t dataLen, PixelFormat pixelFormat, int pixelsWide, int pixelsHigh, const float2& contentSize)
{
	BOOST_ASSERT_MSG(dataLen>0 && pixelsWide>0 && pixelsHigh>0, "Invalid size");

	MipmapInfo mipmap;
	mipmap.pAddress = (uint8_t*)data;
	mipmap.nlength = static_cast<int>(dataLen);
	return initWithMipmaps(&mipmap, 1, pixelFormat, pixelsWide, pixelsHigh);
}

bool ITexture::initWithMipmaps(MipmapInfo* mipmaps, int mipmapsNum, PixelFormat pixelFormat, int pixelsWide, int pixelsHigh)
{
	BOOST_ASSERT_MSG(pixelFormat != PixelFormat::NONE && pixelFormat != PixelFormat::AUTO, "the \"pixelFormat\" param must be a certain value!");
	BOOST_ASSERT_MSG(pixelsWide > 0 && pixelsHigh > 0, "Invalid size");

	if (mipmapsNum <= 0)
	{
		//g_pCore->Trace2("cocos2d: WARNING: mipmap number is less than 1");
		return false;
	}

	if (m_pixelFormatInfoTables.find(pixelFormat) == m_pixelFormatInfoTables.end())
	{
		//g_pCore->Trace2("cocos2d: WARNING: unsupported pixelformat: %lx", (unsigned long)pixelFormat);
		return false;
	}
	const PixelFormatInfo& info = m_pixelFormatInfoTables.at(pixelFormat);
	if (mipmapsNum == 1 && !info.compressed)
	{
		unsigned int bytesPerRow = pixelsWide * info.bpp / 8;
	}

	m_nWidth = pixelsWide;
	m_nHight = pixelsHigh;
	for (int i = 0; i < mipmapsNum; ++i)
	{
		m_Mipmaps[i].pAddress = mipmaps[i].pAddress;
		m_Mipmaps[i].nlength = mipmaps[i].nlength;
	}
	m_pixelFormat = pixelFormat;
	m_hasPremultipliedAlpha = false;

	return true;
}

/*
convert map:
1.PixelFormat::RGBA8888
2.PixelFormat::RGB888
3.PixelFormat::RGB565
4.PixelFormat::A8
5.PixelFormat::I8
6.PixelFormat::AI88
7.PixelFormat::RGBA4444
8.PixelFormat::RGB5A1

gray(5) -> 1235678
gray alpha(6) -> 12345678
rgb(2) -> 1235678
rgba(1) -> 12345678

*/
ITexture::PixelFormat ITexture::convertDataToFormat(const uint8_t* data, uint32_t dataLen, PixelFormat originFormat, PixelFormat format, uint8_t** outData, uint32_t* outDataLen)
{
	// don't need to convert
	if (format == originFormat || format == PixelFormat::AUTO)
	{
		*outData = (uint8_t*)data;
		*outDataLen = dataLen;
		return originFormat;
	}

	switch (originFormat)
	{
	case PixelFormat::I8:
		return convertI8ToFormat(data, dataLen, format, outData, outDataLen);
	case PixelFormat::AI88:
		return convertAI88ToFormat(data, dataLen, format, outData, outDataLen);
	case PixelFormat::RGB888:
		return convertRGB888ToFormat(data, dataLen, format, outData, outDataLen);
	case PixelFormat::RGBA8888:
		return convertRGBA8888ToFormat(data, dataLen, format, outData, outDataLen);
	default:
		//g_pCore->Trace2("unsupported conversion from format %d to format %d", originFormat, format);
		*outData = (uint8_t*)data;
		*outDataLen = dataLen;
		return originFormat;
	}
}

class TextureLoadingDesc : public ResLoadingDesc
{
	struct TexData
	{
		ITexture::TextureType type;
		ITexture::IMAGE_TYPE filetype;
		uint32_t width;
		uint32_t height;
		uint32_t depth;
		uint32_t num_mipmaps;
		uint32_t array_size;
		ITexture::PixelFormat format;
		uint32_t nDataLen;
		std::vector<uint8_t> pData;
	};

	struct TexDesc
	{
		std::string strResName;
		uint32_t nAttr;
		std::shared_ptr<TexData> texData;
		std::shared_ptr<TexturePtr> texPtr;
	};
public:
	TextureLoadingDesc(const std::string& strResName, uint32_t nAttr)
	{
		tex_desc_.strResName = strResName;
		tex_desc_.nAttr = nAttr;
		tex_desc_.texData = MakeSharedPtr<TexData>();
		tex_desc_.texPtr = MakeSharedPtr<TexturePtr>();
	}

	uint64_t Type() const override
	{
		static uint64_t const type = CT_HASH("TextureLoadingDesc");
		return type;
	}

	bool StateLess() const override
	{
		return true;
	}

	virtual std::shared_ptr<void> CreateResource() override
	{
		//TexData& tex_data = *tex_desc_.texData;
		//GetImageInfo(tex_desc_.strResName, tex_data.type, tex_data.filetype, tex_data.width, tex_data.height, tex_data.depth,
		//	tex_data.num_mipmaps, tex_data.array_size, tex_data.format, tex_data.pData, tex_data.nDataLen);

		*tex_desc_.texPtr = this->CreateTexture();
		return *tex_desc_.texPtr;
	}

	void SubThreadStage() override
	{
		std::lock_guard<std::mutex> lock(main_thread_stage_mutex_);
		TexturePtr const & tex = *tex_desc_.texPtr;
		if (tex)
		{
			return;
		}

		this->LoadImage();
	}

	void MainThreadStage() override
	{
		std::lock_guard<std::mutex> lock(main_thread_stage_mutex_);
		this->MainThreadStageNoLock();
	}

	bool HasSubThreadStage() const override
	{
		return true;
	}

	bool Match(ResLoadingDesc const & rhs) const override
	{
		if (this->Type() == rhs.Type())
		{
			TextureLoadingDesc const & tld = static_cast<TextureLoadingDesc const &>(rhs);
			return (tex_desc_.strResName == tld.tex_desc_.strResName)
				&& (tex_desc_.nAttr == tld.tex_desc_.nAttr);
		}
		return false;
	}

	void CopyDataFrom(ResLoadingDesc const & rhs) override
	{
		BOOST_ASSERT(this->Type() == rhs.Type());

		TextureLoadingDesc const & tld = static_cast<TextureLoadingDesc const &>(rhs);
		tex_desc_.strResName = tld.tex_desc_.strResName;
		tex_desc_.nAttr = tld.tex_desc_.nAttr;
		tex_desc_.texData = tld.tex_desc_.texData;
		tex_desc_.texPtr = tld.tex_desc_.texPtr;
	}

	std::shared_ptr<void> CloneResourceFrom(std::shared_ptr<void> const & resource) override
	{
		return resource;
	}

	std::shared_ptr<void> Resource() const override
	{
		return *tex_desc_.texPtr;
	}
private:
	void LoadImage()
	{}

	TexturePtr CreateTexture()
	{
		TexturePtr tex = MakeSharedPtr<ITexture>(tex_desc_.strResName.c_str());
		ResIdentifierPtr fp = ResLoader::Instance()->Open(tex_desc_.strResName);
		tex->AddImage(fp);

		return tex;
	}

	void MainThreadStageNoLock()
	{
		TexturePtr const & tex = *tex_desc_.texPtr;
		if (!tex)
		{
			//tex->CreateHWResource(tex_desc_.texData->init_data, nullptr);
			tex_desc_.texData.reset();
		}
	}
private:
	TexDesc tex_desc_;
	std::mutex main_thread_stage_mutex_;
};

void GetImageInfo(const std::string & strTexName, ITexture::TextureType& tex_type, ITexture::IMAGE_TYPE& filetype,
	uint32_t& width, uint32_t& height, uint32_t& depth,
	uint32_t& num_mipmaps, uint32_t& array_size, ITexture::PixelFormat& format,
	std::vector<uint8_t>& pData, uint32_t pDataLen)
{
	ResIdentifierPtr fp = ResLoader::Instance()->Open(strTexName);

	GetImageInfo(fp, tex_type, filetype, width, height, depth, num_mipmaps, array_size, format, pData, pDataLen);
}

void GetImageInfo(const ResIdentifierPtr& fp, ITexture::TextureType& type, ITexture::IMAGE_TYPE& filetype,
	uint32_t& width, uint32_t& height, uint32_t& depth,
	uint32_t& num_mipmaps, uint32_t& array_size, ITexture::PixelFormat& format,
	std::vector<uint8_t>& pData, uint32_t pDataLen)
{
	ImageResource image;
	image.LoadImageRes(fp);

	type = ITexture::TextureType::TT_2D;
	filetype = image.GetFileType();
	width = image.GetWidth();
	height = image.GetHeight();
	//depth = ;
	num_mipmaps = image.getNumberOfMipmaps();
	array_size = 1;
	format = image.getRenderFormat();
	pDataLen = image.GetDataLen();
	pData.resize(pDataLen);
	memcpy(&pData[0], image.GetData(), sizeof(uint8_t) * pDataLen);

	BOOST_ASSERT(ITexture::IMAGE_TYPE::IMAGE_UNKNOW != filetype);
}

TexturePtr SyncLoadTexture(std::string const & tex_name, uint32_t nAttr)
{
	return ResLoader::Instance()->SyncQueryT<ITexture>(MakeSharedPtr<TextureLoadingDesc>(tex_name, nAttr));
}

TexturePtr ASyncLoadTexture(std::string const & tex_name, uint32_t nAttr)
{
	return ResLoader::Instance()->ASyncQueryT<ITexture>(MakeSharedPtr<TextureLoadingDesc>(tex_name, nAttr));
}


