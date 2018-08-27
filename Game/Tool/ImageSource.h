// 2018��6��26�� ��ȡͼƬ��Դ�ļ� zhangbei
// @��ֲcocos����Ĵ���
#ifndef _IMAGE_SOURCE_H_
#define _IMAGE_SOURCE_H_
#pragma once
#include "../Render/ITexture.h"


// premultiply alpha, or the effect will wrong when want to use other pixel format in Texture2D,
// such as RGB888, RGB5A1
#define CC_RGB_PREMULTIPLY_ALPHA(vr, vg, vb, va) \
    (unsigned)(((unsigned)((uint8_t)(vr) * ((uint8_t)(va) + 1)) >> 8) | \
    ((unsigned)((uint8_t)(vg) * ((uint8_t)(va) + 1) >> 8) << 8) | \
    ((unsigned)((uint8_t)(vb) * ((uint8_t)(va) + 1) >> 8) << 16) | \
    ((unsigned)(uint8_t)(va) << 24))

class Configuration
{
public:
	static bool IssupportsS3TC();
};

typedef struct MipmapInfo
{
	uint8_t* pAddress;
	int nlength;

	MipmapInfo() 
		:pAddress(nullptr), nlength(0) 
	{}
}MipmapInfo;

class ImageResource
{
public:
	ImageResource();
	~ImageResource();

	bool LoadImageRes(const char* szFilePath);
	bool LoadImageRes(const ResIdentifierPtr& fp);

	inline int GetWidth() { return m_nWidth; }
	inline int GetHeight() { return m_nHeight; }
	inline uint32_t GetDataLen() { return m_nDataLen; }
	inline ITexture::PixelFormat getRenderFormat() { return m_renderFormat; }
	inline ITexture::IMAGE_TYPE GetFileType() { return m_fileType; }
	inline uint8_t* GetData() { return m_pData; }
	inline int               getNumberOfMipmaps() { return m_NumberOfMipmaps; }
	inline MipmapInfo*       getMipmaps() { return m_Mipmaps; }
	inline bool              hasPremultipliedAlpha() { return m_bHhasPremultipliedAlpha; }
	//��ѹ����
	bool                     IsCompressed();
private:
	// �����ļ�
	bool LoadImageData(const uint8_t * data, std::size_t dataLen);

	// ����png�ļ�
	bool LoadImagePNG(const uint8_t * data, std::size_t dataLen);

	// ����tif�ļ�
	bool LoadImageTIFF(const uint8_t * data, std::size_t dataLen);

	// ����dds�ļ�
	bool LoadImageS3TC(const uint8_t * data, std::size_t dataLen);

	// ����jpg�ļ�
	bool LoadImageJpg(const uint8_t * data, std::size_t dataLen);

	// �ж��ļ�����
	ITexture::IMAGE_TYPE QueryImageFormat(const uint8_t * data, std::size_t dataLen);

	// �ж��Ƿ���pngͼƬ��ʽ�ļ�
	bool isPng(const uint8_t * data, std::size_t dataLen);

	// �ж��Ƿ���tifͼƬ��ʽ�ļ�
	bool isTiff(const uint8_t * data, std::size_t dataLen);

	// �ж��Ƿ���ddsͼƬ��ʽ�ļ�
	bool isS3TC(const uint8_t * data, std::size_t dataLen);

	// �ж��Ƿ���jpgͼƬ��ʽ�ļ�
	bool isJpg(const uint8_t * data, std::size_t dataLen);

	void premultipliedAlpha();
private:
	static const int MIPMAP_MAX = 16;
	ITexture::IMAGE_TYPE m_fileType;
	int m_nWidth;
	int m_nHeight;
	uint32_t m_nDataLen;
	uint8_t * m_pData;
	int m_NumberOfMipmaps;
	MipmapInfo m_Mipmaps[MIPMAP_MAX];   // pointer to mipmap images
	ITexture::PixelFormat m_renderFormat;
	bool m_bHhasPremultipliedAlpha;// false if we can't auto detect the image is premultiplied or not.
};

typedef std::shared_ptr<ImageResource> ImagePtr;
#endif//_IMAGE_SOURCE_H_