// 2018��6��16�� zhangbei ���嶯������

#ifndef _IACTOR_H_
#define _IACTOR_H_
#pragma once
#include "../../Core/Visible/IVisBase.h"

class IActor :public Renderable
{
public:
	int m_nNumVertics;		//	 ÿ֡�����Ķ�����
	int m_nNumFrames;	// ֡��
	int m_nTotalVertics;		// ȫ������
	int m_nCurrFrame;		// ��ǰ֡
};
#endif//_IACTOR_H_
