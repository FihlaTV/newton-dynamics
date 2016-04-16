/* Copyright (c) <2009> <Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/



// CustomGear.cpp: implementation of the CustomGear class.
//
//////////////////////////////////////////////////////////////////////
#include "CustomJointLibraryStdAfx.h"
#include "CustomGear.h"

//dInitRtti(CustomGear);
IMPLEMENT_CUSTON_JOINT(CustomGear);
//IMPLEMENT_CUSTON_JOINT(CustomGearAndSlide);
IMPLEMENT_CUSTON_JOINT(CustomSatelliteGear);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CustomGear::CustomGear(dFloat gearRatio, const dVector& childPin, const dVector& parentPin, NewtonBody* const child, NewtonBody* const parent)
	:CustomJoint(1, child, parent)
	,m_gearRatio (gearRatio)
{
	dMatrix dommyMatrix;
	// calculate the local matrix for body body0
	dMatrix pinAndPivot0 (dGrammSchmidt(childPin));

	CalculateLocalMatrix (pinAndPivot0, m_localMatrix0, dommyMatrix);
	m_localMatrix0.m_posit = dVector (0.0f, 0.0f, 0.0f, 1.0f);

	// calculate the local matrix for body body1  
	dMatrix pinAndPivot1 (dGrammSchmidt(parentPin));
	CalculateLocalMatrix (pinAndPivot1, dommyMatrix, m_localMatrix1);
	m_localMatrix1.m_posit = dVector (0.0f, 0.0f, 0.0f, 1.0f);
}

CustomGear::CustomGear(int dof, NewtonBody* const child, NewtonBody* const parent)
	:CustomJoint(dof, child, parent)
{
}

CustomGear::~CustomGear()
{
}

CustomGear::CustomGear (NewtonBody* const child, NewtonBody* const parent, NewtonDeserializeCallback callback, void* const userData)
	:CustomJoint(child, parent, callback, userData)
{
	callback (userData, &m_gearRatio, sizeof (dFloat));
}

void CustomGear::Serialize (NewtonSerializeCallback callback, void* const userData) const
{
	CustomJoint::Serialize (callback, userData);
	callback (userData, &m_gearRatio, sizeof (dFloat));
}


void CustomGear::SubmitConstraints (dFloat timestep, int threadIndex)
{
	dMatrix matrix0;
	dMatrix matrix1;
	dVector omega0(0.0f);
	dVector omega1(0.0f);
	dFloat jacobian0[6];
	dFloat jacobian1[6];

	// calculate the position of the pivot point and the Jacobian direction vectors, in global space. 
	CalculateGlobalMatrix (matrix0, matrix1);

	// calculate the angular velocity for both bodies
	const dVector& dir0 = matrix0.m_right;
	const dVector& dir1 = matrix1.m_right;

	dVector dir0Cross (matrix0.m_up * dir0.Scale (m_gearRatio));
	dVector dir1Cross(matrix1.m_up * dir1);
	jacobian0[0] = dir0.m_x;
	jacobian0[1] = dir0.m_y;
	jacobian0[2] = dir0.m_z;
	jacobian0[3] = dir0Cross.m_x;
	jacobian0[4] = dir0Cross.m_y;
	jacobian0[5] = dir0Cross.m_z;

	jacobian1[0] = dir1.m_x;
	jacobian1[1] = dir1.m_y;
	jacobian1[2] = dir1.m_z;
	jacobian1[3] = dir1Cross.m_x;
	jacobian1[4] = dir1Cross.m_y;
	jacobian1[5] = dir1Cross.m_z;

	NewtonBodyGetOmega(m_body0, &omega0[0]);
	NewtonBodyGetOmega(m_body1, &omega1[0]);

	dFloat w0 = omega0 % dir0Cross;
	dFloat w1 = omega1 % dir1Cross;
	dFloat relOmega = w0 + w1;
	dFloat invTimestep = (timestep > 0.0f) ? 1.0f / timestep : 1.0f;
	dFloat relAccel = -0.5f * relOmega * invTimestep;
	NewtonUserJointAddGeneralRow(m_joint, jacobian0, jacobian1);
	NewtonUserJointSetRowAcceleration(m_joint, relAccel);
}

void CustomGear::GetInfo (NewtonJointRecord* const info) const
{
	strcpy (info->m_descriptionType, GetTypeName());

	info->m_extraParameters[0] = m_gearRatio;

	info->m_attachBody_0 = m_body0;
	info->m_attachBody_1 = m_body1;

	info->m_minLinearDof[0] = -D_CUSTOM_LARGE_VALUE;
	info->m_maxLinearDof[0] = D_CUSTOM_LARGE_VALUE;

	info->m_minLinearDof[1] = -D_CUSTOM_LARGE_VALUE;
	info->m_maxLinearDof[1] = D_CUSTOM_LARGE_VALUE;

	info->m_minLinearDof[2] = -D_CUSTOM_LARGE_VALUE;
	info->m_maxLinearDof[2] = D_CUSTOM_LARGE_VALUE;

	info->m_minAngularDof[0] = -D_CUSTOM_LARGE_VALUE;
	info->m_maxAngularDof[0] =  D_CUSTOM_LARGE_VALUE;

	info->m_minAngularDof[1] = -D_CUSTOM_LARGE_VALUE;;
	info->m_maxAngularDof[1] =  D_CUSTOM_LARGE_VALUE;

	info->m_minAngularDof[2] = -D_CUSTOM_LARGE_VALUE;;
	info->m_maxAngularDof[2] =  D_CUSTOM_LARGE_VALUE;

	memcpy (info->m_attachmenMatrix_0, &m_localMatrix0, sizeof (dMatrix));
	memcpy (info->m_attachmenMatrix_1, &m_localMatrix1, sizeof (dMatrix));
}


CustomSatelliteGear::CustomSatelliteGear(dFloat gearRatio, const dVector& childPin, const dVector& parentPin, NewtonBody* const child, NewtonBody* const parent, NewtonBody* const referenceBody, dFloat salleliteSide)
	:CustomGear(gearRatio, childPin, parentPin, child, parent)
	,m_referenceBody(referenceBody)
	,m_salleliteSide((salleliteSide > 0.0f) ? 1.0f : -1.0f)
{
	dMatrix matrix0;
	dMatrix matrix1;
	CalculateGlobalMatrix(matrix0, matrix1);

	dMatrix referenceMatrix (dGetIdentityMatrix());
	if (m_referenceBody) {
		NewtonBodyGetMatrix (m_referenceBody, &referenceMatrix[0][0]);
	}
	m_referenceLocalMatrix = matrix1 * referenceMatrix.Inverse();
}

CustomSatelliteGear::CustomSatelliteGear(NewtonBody* const child, NewtonBody* const parent, NewtonDeserializeCallback callback, void* const userData)
	:CustomGear(child, parent, callback, userData)
{
	dAssert (0);
	//remember load refrence body;
	callback (userData, &m_salleliteSide, sizeof (dFloat));
}

void CustomSatelliteGear::Serialize(NewtonSerializeCallback callback, void* const userData) const
{
	CustomGear::Serialize(callback, userData);
	callback(userData, &m_salleliteSide, sizeof (dFloat));
	dAssert(0);
	//remember save refrence body;
}

void CustomSatelliteGear::GetInfo(NewtonJointRecord* const info) const
{
	CustomGear::GetInfo (info);
	strcpy(info->m_descriptionType, GetTypeName());
	info->m_extraParameters[0] = m_gearRatio;
	info->m_extraParameters[1] = m_salleliteSide;
}

void CustomSatelliteGear::SubmitConstraints(dFloat timestep, int threadIndex)
{
	dMatrix matrix0;
	dMatrix matrix1;
	dVector omega0(0.0f);
	dVector omega1(0.0f);
	dFloat jacobian0[6];
	dFloat jacobian1[6];

	// calculate the position of the pivot point and the Jacobian direction vectors, in global space. 
	CalculateGlobalMatrix(matrix0, matrix1);

	matrix1 = m_referenceLocalMatrix;
	if (m_referenceBody) {
		dMatrix tmpMatrix;
		NewtonBodyGetMatrix(m_referenceBody, &tmpMatrix[0][0]);
		matrix1 = m_referenceLocalMatrix * tmpMatrix;
	}

	// calculate the angular velocity for both bodies
	const dVector& dir0 = matrix0.m_right;
	const dVector& dir1 = matrix1.m_right;

	dVector dir0Cross(matrix0.m_up * dir0.Scale(m_gearRatio));
//	dVector dir1Cross(matrix1.m_up * dir1);
//	dVector dir1Cross(matrix1.m_up * dir1 + matrix1.m_front.Scale (m_salleliteSide) * dir1);
	dVector dir1Cross(matrix1.m_front.Scale (m_salleliteSide) * dir1);
	jacobian0[0] = dir0.m_x;
	jacobian0[1] = dir0.m_y;
	jacobian0[2] = dir0.m_z;
	jacobian0[3] = dir0Cross.m_x;
	jacobian0[4] = dir0Cross.m_y;
	jacobian0[5] = dir0Cross.m_z;

	jacobian1[0] = dir1.m_x;
	jacobian1[1] = dir1.m_y;
	jacobian1[2] = dir1.m_z;
	jacobian1[3] = dir1Cross.m_x;
	jacobian1[4] = dir1Cross.m_y;
	jacobian1[5] = dir1Cross.m_z;

	NewtonBodyGetOmega(m_body0, &omega0[0]);
	NewtonBodyGetOmega(m_body1, &omega1[0]);

	dFloat w0 = omega0 % dir0Cross;
	dFloat w1 = omega1 % dir1Cross;

if (m_salleliteSide != 0.0) {
//dTrace(("%f %f (%f %f %f %f %f %f)\n", omega1.m_y, w1, jacobian1[0], jacobian1[1], jacobian1[2], jacobian1[3], jacobian1[4], jacobian1[5]));
dTrace(("(%f %f %f %f %f %f) (%f %f %f %f %f %f)\n", jacobian0[0], jacobian0[1], jacobian0[2], jacobian0[3], jacobian0[4], jacobian0[5], 
													 jacobian1[0], jacobian1[1], jacobian1[2], jacobian1[3], jacobian1[4], jacobian1[5]));
}

	dFloat relOmega = w0 + w1;
	dFloat invTimestep = (timestep > 0.0f) ? 1.0f / timestep : 1.0f;
	dFloat relAccel = -0.5f * relOmega * invTimestep;
	NewtonUserJointAddGeneralRow(m_joint, jacobian0, jacobian1);
	NewtonUserJointSetRowAcceleration(m_joint, relAccel);
}


/*
CustomGearAndSlide::CustomGearAndSlide (dFloat gearRatio, dFloat slideRatio, const dVector& childPin, const dVector& parentPin, NewtonBody* const child, NewtonBody* const parent)
	:CustomGear(2, child, parent)
	,m_slideRatio (slideRatio)
{
	dAssert (0);
	m_gearRatio = gearRatio;

	dMatrix dommyMatrix;
	// calculate the local matrix for body body0
	dMatrix pinAndPivot0 (dGrammSchmidt(childPin));
	CalculateLocalMatrix (pinAndPivot0, m_localMatrix0, dommyMatrix);

	// calculate the local matrix for body body1  
	dMatrix pinAndPivot1 (dGrammSchmidt(parentPin));
	CalculateLocalMatrix (pinAndPivot1, dommyMatrix, m_localMatrix1);
}

CustomGearAndSlide::~CustomGearAndSlide()
{
}

CustomGearAndSlide::CustomGearAndSlide (NewtonBody* const child, NewtonBody* const parent, NewtonDeserializeCallback callback, void* const userData)
	:CustomGear(child, parent, callback, userData)
{
	callback (userData, &m_slideRatio, sizeof (dFloat));
}

void CustomGearAndSlide::Serialize (NewtonSerializeCallback callback, void* const userData) const
{
	CustomGear::Serialize (callback, userData);
	callback (userData, &m_slideRatio, sizeof (dFloat));
}


void CustomGearAndSlide::SubmitConstraints (dFloat timestep, int threadIndex)
{
	dMatrix matrix0;
	dMatrix matrix1;
	dVector omega0(0.0f);
	dVector veloc1(0.0f);
	dFloat jacobian0[6];
	dFloat jacobian1[6];

	// calculate the position of the pivot point and the Jacobian direction vectors, in global space. 
	CalculateGlobalMatrix (matrix0, matrix1);

	// calculate the angular velocity for both bodies
	NewtonBodyGetOmega(m_body0, &omega0[0]);
	NewtonBodyGetVelocity(m_body1, &veloc1[0]);

	// get angular velocity relative to the pin vector
	dFloat w0 = omega0 % matrix0.m_front;
	dFloat w1 = veloc1 % matrix1.m_front;

	// establish the gear equation.
	dFloat relVeloc = w0 + m_gearRatio * w1;
	if (m_gearRatio > dFloat(1.0f)) {
		relVeloc = w0 / m_gearRatio  + w1;
	}

	// calculate the relative angular acceleration by dividing by the time step

	// ideally relative acceleration should be zero, but is practice there will always 
	// be a small difference in velocity that need to be compensated. 
	// using the full acceleration will make the to over show a oscillate 
	// so use only fraction of the acceleration

	// check if this is an impulsive time step
	dFloat invTimestep = (timestep > 0.0f) ? 1.0f / timestep: 1.0f;
	dFloat relAccel = - 0.3f * relVeloc * invTimestep;

	// set the linear part of Jacobian 0 to zero	
	jacobian0[0] = 0.0f;
	jacobian0[1] = 0.0f;	 
	jacobian0[2] = 0.0f;

	// set the angular part of Jacobian 0 pin vector		
	jacobian0[3] = matrix0.m_front[0];
	jacobian0[4] = matrix0.m_front[1];
	jacobian0[5] = matrix0.m_front[2];

	// set the linear part of Jacobian 1 to translational pin vector	
	jacobian1[0] = matrix1.m_front[0];
	jacobian1[1] = matrix1.m_front[1];
	jacobian1[2] = matrix1.m_front[2];

	// set the rotational part of Jacobian 1 to zero
	jacobian1[3] = 	0.0f;
	jacobian1[4] = 	0.0f;
	jacobian1[5] = 	0.0f;

	// add a angular constraint
	NewtonUserJointAddGeneralRow (m_joint, jacobian0, jacobian1);

	// set the desired angular acceleration between the two bodies
	NewtonUserJointSetRowAcceleration (m_joint, relAccel);

	// add the angular relation constraint form the base class
	CustomGear::SubmitConstraints (timestep, threadIndex);
}

void CustomGearAndSlide::GetInfo (NewtonJointRecord* const info) const
{
	dAssert(0);
}
*/