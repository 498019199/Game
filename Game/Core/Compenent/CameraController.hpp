// CameraController.hpp
// 2018年9月23日 摄像机控制器类移植klayGE @zhangbei

#ifndef _CAMERACONTROLLER_HPP
#define _CAMERACONTROLLER_HPP
#pragma once

#include "../Math/Math.h"
#include "../Util/Timer.h"
#include "../Platform/Intput/Input.hpp"
// 摄像机控制器
//////////////////////////////////////////////////////////////////////////////////
class CameraController :public IEntity
{
public:
	STX_ENTITY(CameraController, IEntity);

	explicit CameraController(Context* pContext);

	virtual ~CameraController();

	void Scalers(float rotationScaler, float moveScaler);

	virtual void AttachCamera(ICamera& camera);
	virtual void DetachCamera();

protected:
	float		m_fRotationScaler;	// Scaler for rotation
	float		m_fMoveScaler;		// Scaler for movement
	ICamera*		m_pCamera;
};

class FirstPersonCameraController : public CameraController
{
public:
	STX_ENTITY(FirstPersonCameraController, CameraController);

	explicit FirstPersonCameraController(Context* pContext, bool use_input_engine = true);

	static void RegisterObject(Context* pContext);

	virtual void AttachCamera(ICamera& camera);

	void RequiresLeftButtonDown(bool lbd);

	void Move(float x, float y, float z);
	void RotateRel(float yaw, float pitch, float roll);
	void RotateAbs(Quaternion const & quat);

private:
	float2		rot_x_;
	float2		rot_y_;
	float2		rot_z_;
	Quaternion	inv_rot_;

	bool left_button_down_;

	enum
	{
		TurnLeftRight,
		TurnUpDown,
		Turn,
		RollLeft,
		RollRight,

		Forward,
		Backward,
		MoveLeft,
		MoveRight
	};

private:
	void InputHandler(InputEngine const & sender, InputAction const & action);
};

//class TrackballCameraController : public CameraController
//{
//public:
//	explicit TrackballCameraController(bool use_input_engine = true,
//		uint32_t rotate_button = MB_Left, uint32_t zoom_button = MB_Right,
//		uint32_t move_button = MB_Middle);
//
//	virtual void AttachCamera(ICamera& camera);
//
//	void Move(float offset_x, float offset_y);
//	void Rotate(float offset_x, float offset_y);
//	void Zoom(float offset_x, float offset_y);
//
//private:
//	bool reverse_target_;
//	float3 target_;
//	float3 right_;
//	uint32_t move_button_;
//	uint32_t rotate_button_;
//	uint32_t zoom_button_;
//
//	enum
//	{
//		Turn,
//		ZoomInOut
//	};
//
//private:
//	void InputHandler(InputEngine const & sender, InputAction const & action);
//};

//class CameraPathController : public CameraController
//{
//public:
//	enum InterpolateType
//	{
//		IT_Linear,
//		IT_CatmullRom,
//		IT_BSpline,
//		IT_Bezier
//	};
//
//public:
//	CameraPathController();
//
//	uint32_t NumCurves() const;
//	uint32_t AddCurve(InterpolateType type, uint32_t num_frames);
//	void DelCurve(uint32_t curve_id);
//	InterpolateType CurveType(uint32_t curve_id) const;
//	uint32_t NumCurveFrames(uint32_t curve_id) const;
//	uint32_t NumControlPoints(uint32_t curve_id) const;
//	uint32_t AddControlPoint(uint32_t curve_id, float frame_id,
//		float3 const & eye_ctrl_pt, float3 const & target_ctrl_pt,
//		float3 const & up_ctrl_pt, bool corner);
//	void DelControlPoint(uint32_t curve_id, uint32_t pt_id);
//	float FrameID(uint32_t curve_id, uint32_t pt_id) const;
//	float3 const & EyeControlPoint(uint32_t curve_id, uint32_t pt_id) const;
//	float3 const & TargetControlPoint(uint32_t curve_id, uint32_t pt_id) const;
//	float3 const & UpControlPoint(uint32_t curve_id, uint32_t pt_id) const;
//	bool Corner(uint32_t curve_id, uint32_t pt_id) const;
//	void FrameID(uint32_t curve_id, uint32_t pt_id, float frame_id);
//	void EyeControlPoint(uint32_t curve_id, uint32_t pt_id, float3 const & pt);
//	void TargetControlPoint(uint32_t curve_id, uint32_t pt_id, float3 const & pt);
//	void UpControlPoint(uint32_t curve_id, uint32_t pt_id, float3 const & pt);
//	void Corner(uint32_t curve_id, uint32_t pt_id, bool corner);
//
//	uint32_t FrameRate() const;
//	void FrameRate(uint32_t frame_rate);
//
//	float Frame() const;
//	void Frame(float frame);
//
//	virtual void AttachCamera(ICamera& camera);
//	virtual void DetachCamera();
//
//private:
//	void UpdateCameraFunc(ICamera& camera, float app_time, float elapsed_time);
//	void UpdateCamera();
//
//private:
//	struct CameraCurve
//	{
//		InterpolateType type;
//		uint32_t num_frames;
//
//		std::vector<float> frame_ids;
//		std::vector<float3> eye_ctrl_pts;
//		std::vector<float3> target_ctrl_pts;
//		std::vector<float3> up_ctrl_pts;
//		std::vector<bool> corners;
//	};
//
//	std::vector<CameraCurve> curves_;
//
//	float start_time_;
//	float curr_frame_;
//	uint32_t frame_rate_;
//};
//
//CameraPathControllerPtr LoadCameraPath(ResIdentifierPtr const & res);
//void SaveCameraPath(std::ostream& os, CameraPathControllerPtr const & path);

#endif		// _CAMERACONTROLLER_HPP
