#include <math.h>
#include <string.h>

#include "utils/math.h"

void loadIdentity44(float* m)
{
	if(!m)return;

	memset(m, 0x00, 16*4);
	m[0]=m[5]=m[10]=m[15]=1.0f;
}

void loadIdentity33(float* m)
{
	if(!m)return;

	memset(m, 0x00, 9*4);
	m[0]=m[4]=m[8]=1.0f;
}

void transposeMatrix33(float* m1, float* m2) //3x3
{
	int i, j;
	for(i=0;i<3;i++)for(j=0;j<3;j++)m2[j+i*3]=m1[i+j*3];
}

void multMatrix33(float* m1, float* m2, float* m) //3x3
{
	int i, j;
	for(i=0;i<3;i++)for(j=0;j<3;j++)m[j+i*3]=(m1[0+i*3]*m2[j+0*3])+(m1[1+i*3]*m2[j+1*3])+(m1[2+i*3]*m2[j+2*3]);
}

void addMatrix33(float* m1, float* m2, float* m) //3x3
{
	int i, j;
	for(i=0;i<3;i++)for(j=0;j<3;j++)m[j+i*3]=m1[j+i*3]+m2[j+i*3];
}

void projectVectorPlane(vect3Df_s* v, vect3Df_s n)
{
	if(!v)return;
	float r=vdotf(*v,n);
	*v=vsubf(*v,vmulf(n,r));
}

void fixMatrix33(float* m) //3x3
{
	if(!m)return;
	vect3Df_s x=vect3Df(m[0],m[3],m[6]);
	vect3Df_s y=vect3Df(m[1],m[4],m[7]);
	vect3Df_s z=vect3Df(m[2],m[5],m[8]);
	
	projectVectorPlane(&x,y);
	projectVectorPlane(&z,y);
	projectVectorPlane(&z,x);
	
	x=vnormf(x);
	y=vnormf(y);
	z=vnormf(z);
	
	m[0]=x.x;m[3]=x.y;m[6]=x.z;
	m[1]=y.x;m[4]=y.y;m[7]=y.z;
	m[2]=z.x;m[5]=z.y;m[8]=z.z;
}

void transposeMatrix44(float* m1, float* m2) //4x4
{
	int i, j;
	for(i=0;i<4;i++)for(j=0;j<4;j++)m2[j+i*4]=m1[i+j*4];
}

void multMatrix44(float* m1, float* m2, float* m) //4x4
{
	int i, j;
	for(i=0;i<4;i++)for(j=0;j<4;j++)m[i+j*4]=(m1[0+j*4]*m2[i+0*4])+(m1[1+j*4]*m2[i+1*4])+(m1[2+j*4]*m2[i+2*4])+(m1[3+j*4]*m2[i+3*4]);
}

vect3Df_s multMatrix44Vect3(float* m, vect3Df_s v, bool transpose)
{
	if(!transpose) return vect3Df(vdotf(v, vect3Df(m[0+0*4], m[1+0*4], m[2+0*4])), vdotf(v, vect3Df(m[0+1*4], m[1+1*4], m[2+1*4])), vdotf(v, vect3Df(m[0+2*4], m[1+2*4], m[2+2*4])));
	else           return vect3Df(vdotf(v, vect3Df(m[0+0*4], m[0+1*4], m[0+2*4])), vdotf(v, vect3Df(m[1+0*4], m[1+1*4], m[1+2*4])), vdotf(v, vect3Df(m[2+0*4], m[2+1*4], m[2+2*4])));
}

vect4Df_s multMatrix44Vect4(float* m, vect4Df_s v, bool transpose)
{
	if(!transpose) return vect4Df(vdotf4(v, vect4Df(m[0+0*4], m[1+0*4], m[2+0*4], m[3+0*4])), vdotf4(v, vect4Df(m[0+1*4], m[1+1*4], m[2+1*4], m[3+1*4])), vdotf4(v, vect4Df(m[0+2*4], m[1+2*4], m[2+2*4], m[3+2*4])), vdotf4(v, vect4Df(m[0+3*4], m[1+3*4], m[2+3*4], m[3+3*4])));
	else           return vect4Df(vdotf4(v, vect4Df(m[0+0*4], m[0+1*4], m[0+2*4], m[0+3*4])), vdotf4(v, vect4Df(m[1+0*4], m[1+1*4], m[1+2*4], m[1+3*4])), vdotf4(v, vect4Df(m[2+0*4], m[2+1*4], m[2+2*4], m[2+3*4])), vdotf4(v, vect4Df(m[3+0*4], m[3+1*4], m[3+2*4], m[3+3*4])));
}

void translateMatrix(float* tm, float x, float y, float z)
{
	float rm[16], m[16];

	loadIdentity44(rm);
	rm[3]=x;
	rm[7]=y;
	rm[11]=z;
	
	multMatrix44(tm,rm,m);
	memcpy(tm,m,16*sizeof(float));
}

// 00 01 02 03
// 04 05 06 07
// 08 09 10 11
// 12 13 14 15

void rotateMatrixX(float* tm, float x, bool r)
{
	float rm[16], m[16];
	memset(rm, 0x00, 16*4);
	rm[0]=1.0f;
	rm[5]=cos(x);
	rm[6]=sin(x);
	rm[9]=-sin(x);
	rm[10]=cos(x);
	rm[15]=1.0f;
	if(!r)multMatrix44(tm,rm,m);
	else multMatrix44(rm,tm,m);
	memcpy(tm,m,16*sizeof(float));
}

void rotateMatrixY(float* tm, float x, bool r)
{
	float rm[16], m[16];
	memset(rm, 0x00, 16*4);
	rm[0]=cos(x);
	rm[2]=sin(x);
	rm[5]=1.0f;
	rm[8]=-sin(x);
	rm[10]=cos(x);
	rm[15]=1.0f;
	if(!r)multMatrix44(tm,rm,m);
	else multMatrix44(rm,tm,m);
	memcpy(tm,m,16*sizeof(float));
}

void rotateMatrixZ(float* tm, float x, bool r)
{
	float rm[16], m[16];
	memset(rm, 0x00, 16*4);
	rm[0]=cos(x);
	rm[1]=sin(x);
	rm[4]=-sin(x);
	rm[5]=cos(x);
	rm[10]=1.0f;
	rm[15]=1.0f;
	if(!r)multMatrix44(tm,rm,m);
	else multMatrix44(rm,tm,m);
	memcpy(tm,m,16*sizeof(float));
}

void rotateMatrixAxis(float* tm, float x, vect3Df_s a, bool r)
{
	float rm[16], m[16];

	float cosval=cos(x);
	float sinval=sin(x);
	float onemcosval=1.0f-cosval;

	memset(rm, 0x00, sizeof(rm));

	rm[0]=cosval + a.x*a.x*onemcosval;
	rm[1]=a.x*a.y*onemcosval-a.z*sinval;
	rm[2]=a.x*a.z*onemcosval + a.y*sinval;

	rm[4]=a.x*a.y*onemcosval + a.z*sinval;
	rm[5]=cosval + a.y*a.y*onemcosval;
	rm[6]=a.y*a.z*onemcosval-a.x*sinval;

	rm[8]=a.x*a.z*onemcosval-a.y*sinval;
	rm[9]=a.y*a.z*onemcosval + a.x*sinval;
	rm[10]=cosval + a.z*a.z*onemcosval;

	rm[15]=1.0f;

	if(r)multMatrix44(rm,tm,m);
	else multMatrix44(tm,rm,m);
	memcpy(tm,m,16*sizeof(float));
}

void scaleMatrix(float* tm, float x, float y, float z)
{
	tm[0]*=x; tm[4]*=x; tm[8]*=x; tm[12]*=x;
	tm[1]*=y; tm[5]*=y; tm[9]*=y; tm[13]*=y;
	tm[2]*=z; tm[6]*=z; tm[10]*=z; tm[14]*=z;
}

void initProjectionMatrix(float* m, float fovy, float aspect, float near, float far)
{
	float top = near*tan(fovy/2);
	float right = (top*aspect);

	float mp[4*4];
	
	mp[0x0] = near/right;
	mp[0x1] = 0.0f;
	mp[0x2] = 0.0f;
	mp[0x3] = 0.0f;

	mp[0x4] = 0.0f;
	mp[0x5] = near/top;
	mp[0x6] = 0.0f;
	mp[0x7] = 0.0f;

	mp[0x8] = 0.0f;
	mp[0x9] = 0.0f;
	mp[0xA] = -(far+near)/(far-near);
	mp[0xB] = -2.0f*(far*near)/(far-near);

	mp[0xC] = 0.0f;
	mp[0xD] = 0.0f;
	mp[0xE] = -1.0f;
	mp[0xF] = 0.0f;

	float mp2[4*4];
	loadIdentity44(mp2);
	mp2[0xA]=0.5;
	mp2[0xB]=-0.5;

	multMatrix44(mp2, mp, m);

	rotateMatrixZ(m, M_PI/2, false); //because framebuffer is sideways...
}

vect3Df_s getMatrixColumn(float* m, u8 i)
{
	if(!m || i>=4)return vect3Df(0,0,0);
	return vect3Df(m[0+i*4],m[1+i*4],m[2+i*4]);
}

vect3Df_s getMatrixRow(float* m, u8 i)
{
	if(!m || i>=4)return vect3Df(0,0,0);
	return vect3Df(m[i+0*4],m[i+1*4],m[i+2*4]);
}

vect4Df_s getMatrixColumn4(float* m, u8 i)
{
	if(!m || i>=4)return vect4Df(0,0,0,0);
	return vect4Df(m[0+i*4],m[1+i*4],m[2+i*4],m[3+i*4]);
}

vect4Df_s getMatrixRow4(float* m, u8 i)
{
	if(!m || i>=4)return vect4Df(0,0,0,0);
	return vect4Df(m[i+0*4],m[i+1*4],m[i+2*4],m[i+3*4]);
}
