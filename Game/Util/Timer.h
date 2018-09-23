// 2018年1月27日10点40分 zhangbei boost也是调用std，看文档的时间还不如自己写一下
// 主要对C++11chrono类封装使用
#ifndef TIMER_H
#define TIMER_H
#pragma once

class Timer
{
public:
	Timer();

	void ReStart();

	// 当前时间
	double CurrentTime();

	// 逝去时间
	double Elapsed();

	double ElapseMax();

	double ElapseMin();
private:
	double m_fTime;
};
#endif//TIMER_H

