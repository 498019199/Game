// 2018��1��27��10��40�� zhangbei boostҲ�ǵ���std�����ĵ���ʱ�仹�����Լ�дһ��
// ��Ҫ��C++11chrono���װʹ��
#ifndef TIMER_H
#define TIMER_H
#pragma once

class Timer
{
public:
	Timer();

	void ReStart();

	// ��ǰʱ��
	double CurrentTime();

	// ��ȥʱ��
	double Elapsed();

	double ElapseMax();

	double ElapseMin();
private:
	double m_fTime;
};
#endif//TIMER_H

