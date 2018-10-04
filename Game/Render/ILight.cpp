#include "ILight.h"
#define  ExCHAR(val) static_cast<int>(val)

ILight::ILight(Context* pContext)
	:IEntity(pContext),m_nState(0), m_nAttr(0), m_nType(LT_None), m_Pos(float4::Zero())
{
}

ILight::~ILight()
{

}

bool ILight::OnInit()
{
	return true;
}

bool ILight::OnShut()
{
	return true;
}

void ILight::Update(const Color* color, float4* verts, Color* colors) const
{
}

Color ILight::Update(const Color* base, float4* verts) const
{
	return *base;
}

AmbientLightSource::AmbientLightSource(Context* pContext, Color i3Ambient)
	:ILight(pContext),m_Ambient(i3Ambient)
{

}

Color AmbientLightSource::Update(const Color* base, float4* verts) const
{
	// 环境光强度 * 环境反射颜色
	unsigned char r = ExCHAR((m_Ambient.r() * base->r()) / 256);
	unsigned char g = ExCHAR((m_Ambient.g()* base->g()) / 256);
	unsigned char b = ExCHAR((m_Ambient.b() * base->b()) / 256);
	return Color(0, r, g, b);
}

void AmbientLightSource::Update(const Color* base, float4* verts, Color* colors) const
{
	unsigned char r = ExCHAR((m_Ambient.r() * base->r()) / 256);
	unsigned char g = ExCHAR((m_Ambient.g()* base->g()) / 256);
	unsigned char b = ExCHAR((m_Ambient.b() * base->b()) / 256);

	colors[0].r() += r; colors[0].g() += g; colors[0].b() += b;
	colors[1].r() += r; colors[1].g() += g; colors[1].b() += b;
	colors[2].r() += r; colors[2].g() += g; colors[2].b() += b;
}

DirectionalLightSource::DirectionalLightSource(Context* pContext, float4 f4Dir, Color i3Diffuse)
	:ILight(pContext), m_Direction(f4Dir), m_Diffuse(i3Diffuse)
{

}

Color DirectionalLightSource::Update(const Color* base, float4* verts) const
{
	float4 v = verts[1] - verts[0];
	float4 u = verts[2] - verts[0];
	float4 n = MathLib::Cross(v, u);

	float fLength = MathLib::Length(n);
	float dp = MathLib::Dot(n, m_Direction);

	// 判断是否照射
	if (dp > 0)
	{
		// 光源颜色 * 
		float i = 128 * dp / fLength;
		unsigned char r = ExCHAR((m_Diffuse.r() * base->r() * i) / (256 * 128));
		unsigned char g = ExCHAR((m_Diffuse.g()* base->g() * i) / (256 * 128));
		unsigned char b = ExCHAR((m_Diffuse.b() * base->b() * i) / (256 * 128));
		return Color(0, r, g, b);
	}

	return Color(0, 0, 0, 0);
}

void DirectionalLightSource::Update(const Color* base, float4* verts, Color* colors) const
{
	for (int i = 0; i < 3; ++i)
	{
		float dp = MathLib::Dot(verts[i], m_Direction);
		if (dp > 0)
		{
			float fI = 128 * dp;
			unsigned char r = ExCHAR((m_Diffuse.r() * base->r() * fI) / (256 * 128));
			unsigned char g = ExCHAR((m_Diffuse.g()* base->g() * fI) / (256 * 128));
			unsigned char b = ExCHAR((m_Diffuse.b() * base->b() * fI) / (256 * 128));

			colors[i].r() += r; colors[i].g() += g; colors[i].b() += b;
		}
	}
}

PointLightSource::PointLightSource(Context* pContext, float fKc, float fKl, float fKq, Color i3Diffuse)
	:ILight(pContext), m_fKc(fKc), m_fKl(fKl), m_fKq(fKq), m_Diffuse(i3Diffuse)
{

}

Color PointLightSource::Update(const Color* base, float4* verts) const
{
	//              I0point * Clpoint
	//  I(d)point = ___________________
	//              kc +  kl*d + kq*d2        
	float4 v = verts[1] - verts[0];
	float4 u = verts[2] - verts[0];
	float4 n = MathLib::Cross(v, u);
	// 计算从表面到光源的向量
	float4  l = GetPosition() - verts[0];
	// 多边形法线长度
	float fLength = MathLib::Length(n);
	// 计算长度和衰减
	float fDist = MathLib::Length(l);
	float dp = MathLib::Dot(n, l);

	if (dp > 0)
	{
		float fAtten = m_fKc + m_fKl * fDist + m_fKq * fDist * fDist;
		float i = 128 * dp / (fLength * fDist * fAtten);

		unsigned char r = ExCHAR((m_Diffuse.r() * base->r() * i) / (256 * 128));
		unsigned char g = ExCHAR((m_Diffuse.g()* base->g() * i) / (256 * 128));
		unsigned char b = ExCHAR((m_Diffuse.b() * base->b() * i) / (256 * 128));
		return Color(0, r, g, b);
	}

	return Color(0, 0, 0, 0);
}

void PointLightSource::Update(const Color* base, float4* verts, Color* colors) const
{
	for (int i = 0; i < 3; ++i)
	{
		float4 n = verts[i];
		float4 v = verts[i + 3];
		float4  l = GetPosition() - v;
		// 计算长度和衰减
		float fDist = MathLib::Length(l);
		float dp = MathLib::Dot(n, l);

		if (dp > 0)
		{
			float fAtten = m_fKc + m_fKl * fDist + m_fKq * fDist * fDist;
			float fI = 128 * dp / (fDist * fAtten);

			unsigned char r = ExCHAR((m_Diffuse.r() * base->r() * fI) / (256 * 128));
			unsigned char g = ExCHAR((m_Diffuse.g()* base->g() * fI) / (256 * 128));
			unsigned char b = ExCHAR((m_Diffuse.b() * base->b() * fI) / (256 * 128));

			colors[i].r() += r; colors[i].g() += g; colors[i].b() += b;
		}
	}
}

SpotLightSource1::SpotLightSource1(Context* pContext, float fKc, float fKl, float fKq, Color i3Diffuse)
	:ILight(pContext), m_fKc(fKc), m_fKl(fKl), m_fKq(fKq), m_Diffuse(i3Diffuse)
{

}

Color SpotLightSource1::Update(const Color* base, float4* verts) const
{
	//              I0point * Clpoint
	//  I(d)point = ___________________
	//              kc +  kl*d + kq*d2              
	//
	float4 v = verts[1] - verts[0];
	float4 u = verts[2] - verts[0];
	float4 n = MathLib::Cross(v, u);
	// 计算从表面到光源的向量
	float4  l = GetPosition() - n;
	// 多边形法线长度
	float fLength = MathLib::Length(n);
	// 计算长度和衰减
	float fDist = MathLib::Length(l);
	float dp = MathLib::Dot(n, l);

	if (dp > 0)
	{
		float fAtten = m_fKc + m_fKl * fDist + m_fKq * fDist * fDist;
		float i = 128 * dp / (fLength * fAtten);

		unsigned char r = ExCHAR((m_Diffuse.r() * base->r() * i) / (256 * 128));
		unsigned char g = ExCHAR((m_Diffuse.g()* base->g() * i) / (256 * 128));
		unsigned char b = ExCHAR((m_Diffuse.b() * base->b() * i) / (256 * 128));
		return Color(0, r, g, b);
	}

	return Color(0, 0, 0, 0);
}

void SpotLightSource1::Update(const Color* base, float4* verts, Color* colors) const
{
	for (int i = 0; i < 3; ++i)
	{
		float4 n = verts[i];
		float4 v = verts[i + 3];
		float4  l = GetPosition() - n;
		// 计算长度和衰减
		float fDist = MathLib::Length(l);
		float dp = MathLib::Dot(n, l);

		if (dp > 0)
		{
			float fAtten = m_fKc + m_fKl * fDist + m_fKq * fDist * fDist;
			float fI = dp / fAtten;

			unsigned char r = ExCHAR((m_Diffuse.r() * base->r() * fI) / 256);
			unsigned char g = ExCHAR((m_Diffuse.g()* base->g() * fI) / 256);
			unsigned char b = ExCHAR((m_Diffuse.b() * base->b() * fI) / 256);

			colors[i].r() += r; colors[i].g() += g; colors[i].b() += b;
		}
	}
}

SpotLightSource2::SpotLightSource2(Context* pContext, float fKc, float fKl, float fKq,
	float fSpotInner, float fSpotOuter, float fPf, 
	float4 f4Dir, Color i3Diffuse)
	:ILight(pContext), m_fKc(fKc),m_fKl(fKl),m_fKq(fKq),
	m_fSpotInner(fSpotInner),m_fSpotOuter(fSpotOuter),
	m_Direction(f4Dir),m_Diffuse(i3Diffuse)
{

}

Color SpotLightSource2::Update(const Color* base, float4* verts) const
{
	//         	     I0spotlight * Clspotlight * MAX( (l . s), 0)^pf                     
	// I(d)spotlight = __________________________________________      
	//               		 kc + kl*d + kq*d2        
	// Where d = |p - s|, and pf = power factor
	float4 v = verts[1] - verts[0];
	float4 u = verts[2] - verts[0];
	float4 n = MathLib::Cross(v, u);
	// 多边形法线长度
	float fLength = MathLib::Length(n);
	float dp = MathLib::Dot(n, m_Direction);
	if (dp > 0)
	{
		float4  s = GetPosition() - verts[0];
		float fDist = MathLib::Length(s);
		float fDpsl = MathLib::Dot(s, m_Direction) / fDist;

		if (fDpsl > 0)
		{
			float fAtten = m_fKc + m_fKl * fDist + m_fKq * fDist * fDist;
			float fDpslExp = std::pow(fDpsl, m_fPf);
			float i = 128 * dp * fDpslExp/ (fLength * fAtten);

			unsigned char r = ExCHAR((m_Diffuse.r() * base->r() * i) / (256 * 128));
			unsigned char g = ExCHAR((m_Diffuse.g()* base->g() * i) / (256 * 128));
			unsigned char b = ExCHAR((m_Diffuse.b() * base->b() * i) / (256 * 128));
			return Color(0, r, g, b);
		}
	}
	return Color(0, 0, 0, 0);
}

void SpotLightSource2::Update(const Color* base, float4* verts, Color* colors) const
{
	for (int i = 0; i < 3; ++i)
	{
		float4 n = verts[i];
		float4 v = verts[i + 3];
		float dp = MathLib::Dot(n, m_Direction);
		if (dp > 0)
		{
			float4  s = GetPosition() - verts[0];
			float fDist = MathLib::Length(s);
			float fDpsl = MathLib::Dot(s, m_Direction) / fDist;

			if (fDpsl > 0)
			{
				float fAtten = m_fKc + m_fKl * fDist + m_fKq * fDist * fDist;
				float fDpslExp = std::pow(fDpsl, m_fPf);
				float fI = dp * fDpslExp / fAtten;

				unsigned char r = ExCHAR((m_Diffuse.r() * base->r() * fI) / (256 ));
				unsigned char g = ExCHAR((m_Diffuse.g()* base->g() * fI) / (256));
				unsigned char b = ExCHAR((m_Diffuse.b() * base->b() * fI) / (256));

				colors[i].r() += r; colors[i].g() += g; colors[i].b() += b;
			}
		}
	}
}
