// 2018年6月16日 zhangbei 物体动作管理

#ifndef _IACTOR_H_
#define _IACTOR_H_
#pragma once
#include "../../Core/Visible/IVisBase.h"

class IActor :public Renderable
{
public:
	int m_nNumVertics;		//	 每帧包含的顶点数
	int m_nNumFrames;	// 帧数
	int m_nTotalVertics;		// 全部顶点
	int m_nCurrFrame;		// 当前帧
};
#endif//_IACTOR_H_
