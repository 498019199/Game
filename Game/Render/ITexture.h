// 2018年6月16日 zhangbei 纹理
#ifndef _TEXTURE_H_
#define _TEXTURE_H_
#pragma once
#include "../Core/predefine.h"
#include "../Container/var_type.h"
#include "../Math/Math.h"

#include <memory>
#include <map>
#include <vector>
struct MipmapInfo;
class ITexture 
{
public:
	enum class TextureType
	{
		// 1D texture, used in combination with 1D texture coordinates
		TT_1D,
		// 2D texture, used in combination with 2D texture coordinates
		TT_2D,
		// 3D texture, used in combination with 3D texture coordinates
		TT_3D,
		// cube map, used in combination with 3D texture coordinates
		TT_Cube
	};

	enum class IMAGE_TYPE
	{
		IMAGE_UNKNOW = -1,
		IMAGE_S3TC = 0, // .dds
		IMAGE_PNG = 1,	// .png
		IMAGE_TIFF = 2, // .tif
		IMAGE_JPN = 3, // .jpn
	};

	enum class PixelFormat
	{
		//! auto detect the type
		AUTO,
		//! 32-bit texture: BGRA8888
		BGRA8888,
		//! 32-bit texture: RGBA8888
		RGBA8888,
		//! 24-bit texture: RGBA888
		RGB888,
		//! 16-bit texture without Alpha channel
		RGB565,
		//! 8-bit textures used as masks
		A8,
		//! 8-bit intensity texture
		I8,
		//! 16-bit textures used as masks
		AI88,
		//! 16-bit textures: RGBA4444
		RGBA4444,
		//! 16-bit textures: RGB5A1
		RGB5A1,
		//! 4-bit PVRTC-compressed texture: PVRTC4
		PVRTC4,
		//! 4-bit PVRTC-compressed texture: PVRTC4 (has alpha channel)
		PVRTC4A,
		//! 2-bit PVRTC-compressed texture: PVRTC2
		PVRTC2,
		//! 2-bit PVRTC-compressed texture: PVRTC2 (has alpha channel)
		PVRTC2A,
		//! ETC-compressed texture: ETC
		ETC,
		//! S3TC-compressed texture: S3TC_Dxt1
		S3TC_DXT1,
		//! S3TC-compressed texture: S3TC_Dxt3
		S3TC_DXT3,
		//! S3TC-compressed texture: S3TC_Dxt5
		S3TC_DXT5,
		//! ATITC-compressed texture: ATC_RGB
		ATC_RGB,
		//! ATITC-compressed texture: ATC_EXPLICIT_ALPHA
		ATC_EXPLICIT_ALPHA,
		//! ATITC-compressed texture: ATC_INTERPOLATED_ALPHA
		ATC_INTERPOLATED_ALPHA,
		//! Default texture format: AUTO
		DEFAULT = AUTO,

		NONE = -1
	};
	
	struct PixelFormatInfo
	{
		PixelFormatInfo(int aBpp, bool aCompressed, bool anAlpha)
			:bpp(aBpp), compressed(aCompressed), alpha(anAlpha)
		{}

		int bpp;
		bool compressed;
		bool alpha;
	};
	typedef std::map<ITexture::PixelFormat, const PixelFormatInfo> PixelFormatInfoMap;

	static void setDefaultAlphaPixelFormat(ITexture::PixelFormat format);
	static ITexture::PixelFormat GetDefaultAlphaPixelFormat();

	ITexture(const char* szTexure);

	~ITexture();

	// 载入图片数据
	bool AddImage(ResIdentifierPtr fp);

	// **获取像素信息图，键值对是PixelFormat和PixelFormatInfo
	static const PixelFormatInfoMap& GetPixelFormatInfoMap();

	// 根据坐标获取颜色(双线性插值和mipmap)
	Color GetTextureColor(float fu, float fv, float z);
	template<typename T>
	inline T* GetData()
	{
		return reinterpret_cast<T*>(m_pData);
	}
	template<typename T>
	inline const T* GetData() const
	{
		return reinterpret_cast<T*>(m_pData);
	}
	uint32_t ArraySize(){ return 0; }
	uint32_t NumMipMaps() { return 0; }
	int GetWidth() const { return m_nWidth; };
	int GetHight() const { return m_nHight; };
	uint32_t GetUseMemoryLength() { return m_DataLen; }
private:
	//用带数据的texture2d初始化。
	//@param data指定指向内存中图像数据的指针。
	//@param dataLen图像数据长度。
	//@param pixelFormat图像pixelFormat。
	//@param pixelsWide图像宽度。
	//@param pixelsHigh图像高度。
	//@param contentSize图像内容大小。
	bool initWithData(const void *data, uint32_t dataLen, ITexture::PixelFormat pixelFormat, int pixelsWide, int pixelsHigh, const float2& contentSize);
	
	//使用mipmap初始化。
	//@param mipmaps指定指向内存中图像数据的指针。
	//@param mipmapsNum mipmaps编号。
	//@param pixelFormat图像pixelFormat。
	//@param pixelsWide图像宽度。
	//@param pixelsHigh图像高度。
	bool initWithMipmaps(MipmapInfo* mipmaps, int mipmapsNum, ITexture::PixelFormat pixelFormat, int pixelsWide, int pixelsHigh);
	/**convert functions*/
	/**
	Convert the format to the format param you specified, if the format is PixelFormat::Automatic, it will detect it automatically and convert to the closest format for you.
	It will return the converted format to you. if the outData != data, you must delete it manually.
	*/
	static PixelFormat convertDataToFormat(const uint8_t* data, uint32_t dataLen, PixelFormat originFormat, PixelFormat format, uint8_t** outData, uint32_t* outDataLen);
	static PixelFormat convertI8ToFormat(const uint8_t* data, uint32_t dataLen, PixelFormat format, uint8_t** outData, uint32_t* outDataLen);
	static PixelFormat convertAI88ToFormat(const uint8_t* data, uint32_t dataLen, PixelFormat format, uint8_t** outData, uint32_t* outDataLen);
	static PixelFormat convertRGB888ToFormat(const uint8_t* data, uint32_t dataLen, PixelFormat format, uint8_t** outData, uint32_t* outDataLen);
	static PixelFormat convertRGBA8888ToFormat(const uint8_t* data, uint32_t dataLen, PixelFormat format, uint8_t** outData, uint32_t* outDataLen);

	//I8 to XXX
	static void convertI8ToRGB888(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertI8ToRGBA8888(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertI8ToRGB565(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertI8ToRGBA4444(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertI8ToRGB5A1(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertI8ToAI88(const uint8_t* data, uint32_t dataLen, uint8_t* outData);

	//AI88 to XXX
	static void convertAI88ToRGB888(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertAI88ToRGBA8888(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertAI88ToRGB565(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertAI88ToRGBA4444(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertAI88ToRGB5A1(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertAI88ToA8(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertAI88ToI8(const uint8_t* data, uint32_t dataLen, uint8_t* outData);

	//RGB888 to XXX
	static void convertRGB888ToRGBA8888(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertRGB888ToRGB565(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertRGB888ToI8(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertRGB888ToAI88(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertRGB888ToRGBA4444(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertRGB888ToRGB5A1(const uint8_t* data, uint32_t dataLen, uint8_t* outData);

	//RGBA8888 to XXX
	static void convertRGBA8888ToRGB888(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertRGBA8888ToRGB565(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertRGBA8888ToI8(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertRGBA8888ToA8(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertRGBA8888ToAI88(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertRGBA8888ToRGBA4444(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
	static void convertRGBA8888ToRGB5A1(const uint8_t* data, uint32_t dataLen, uint8_t* outData);
private:
	// 名字
	std::string m_strTexture;
	// 资源指针
	uint8_t *m_pData;
	uint32_t m_DataLen;
	// 大小
	int m_nWidth;
	int m_nHight;
	bool m_hasMipmaps;
	// 纹理是否有Alpha预乘
	bool m_hasPremultipliedAlpha;
	ITexture::PixelFormat m_pixelFormat;
	static const PixelFormatInfoMap m_pixelFormatInfoTables;
};

void GetImageInfo(const std::string& strTexName, ITexture::TextureType& tex_type, ITexture::IMAGE_TYPE& filetype,
	uint32_t& width, uint32_t& height, uint32_t& depth, uint32_t& num_mipmaps, uint32_t& array_size, ITexture::PixelFormat& format,
	std::vector<uint8_t>& pData, uint32_t pDataLen);
void GetImageInfo(const ResIdentifierPtr& fp, ITexture::TextureType& tex_type, ITexture::IMAGE_TYPE& filetype,
	uint32_t& width, uint32_t& height, uint32_t& depth, uint32_t& num_mipmaps, uint32_t& array_size, ITexture::PixelFormat& format,
	std::vector<uint8_t>& pData, uint32_t pDataLen);
TexturePtr SyncLoadTexture(std::string const & tex_name, uint32_t nAttr);
TexturePtr ASyncLoadTexture(std::string const & tex_name, uint32_t nAttr);
#endif//_TEXTURE_H_
