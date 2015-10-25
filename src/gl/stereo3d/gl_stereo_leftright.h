#ifndef GL_STEREO_LEFTRIGHT_H_
#define GL_STEREO_LEFTRIGHT_H_

#include "gl_stereo3d.h"

namespace s3d {

class ShiftedEyePose : public EyePose
{
public:
	ShiftedEyePose(float shift) : shift(shift) {};
	float getShift() const { return shift; }
	void setShift(float shift) { this->shift = shift; }
	virtual void GetProjection(float fov, float aspectRatio, float fovRatio, GLdouble outMatrix[4][4]) const;
	virtual void GetViewShift(float yaw, float outViewShift[3]) const;
protected:
	float shift;
};

class LeftEyePose : public ShiftedEyePose
{
public:
	LeftEyePose(float ipd) : ShiftedEyePose(-0.5*ipd) {}
	float getIpd() const { return -2.0*getShift(); }
	void setIpd(float ipd) { setShift(-0.5*ipd); }
};

class RightEyePose : public ShiftedEyePose
{
public:
	RightEyePose(float ipd) : ShiftedEyePose(+0.5*ipd) {}
	float getIpd() const { return +2.0*shift; }
	void setIpd(float ipd) { setShift(+0.5*ipd); }
};

/**
 * As if viewed through the left eye only
 */
class LeftEyeView : public Stereo3DMode
{
public:
	static const LeftEyeView& getInstance(float ipd);

	LeftEyeView(float ipd) : eye(ipd) {}
	float getIpd() const { return eye.getIpd(); }
	void setIpd(float ipd) { eye.setIpd(ipd); }
	virtual const_iterator begin() const { return &eye; }
	virtual const_iterator end() const {
		EyePose const * penum = &eye;
		return ++penum; 
	}
protected:
	LeftEyePose eye;
};

} /* namespace s3d */

#endif /* GL_STEREO_LEFTRIGHT_H_ */
