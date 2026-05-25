//////////////////////////////////////////
//		jieunlee@hansung.ac.kr			//
//		2026. 04. 06					//
//////////////////////////////////////////

#ifndef HUMAN_H
#define HUMAN_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Bones in a Human skeleton
enum Human_Bone {		// 20 bones
    pelvis, spine, 
	neck, head, 
	clavicleL, upperarmL, forearmL, handL, 
	clavicleR, upperarmR, forearmR, handR,
	thighL, calfL, footL, toeL, 
	thighR, calfR, footR, toeR
};

// several poses
enum Human_Pose {
	base, /*armLeftUp, armRightUp, legLeftUp, legRightUp,*/
	walk1, walk2
};

// Default values
const int BONENUM = 20;
const int POSENUM = /*7*/3;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Human
{
public:
    // Skeleton Attributes
   glm::quat	BoneRotate[BONENUM];

    // Constructor 
    Human()
    {
		// bone length
		BoneLength[pelvis] = 1.0f;		BoneLength[spine] = 3.0f;
		BoneLength[neck] = 0.5f;		BoneLength[head] = 1.0f;
		BoneLength[clavicleL] = 1.0f;	BoneLength[upperarmL] = 2.0f;	BoneLength[forearmL] = 1.5f;	BoneLength[handL] = 1.0f;
		BoneLength[clavicleR] = 1.0f;	BoneLength[upperarmR] = 2.0f;	BoneLength[forearmR] = 1.5f;	BoneLength[handR] = 1.0f;
		BoneLength[thighL] = 2.5f;		BoneLength[calfL] = 2.0f;		BoneLength[footL] = 1.0f;		BoneLength[toeL] = 0.5f;
		BoneLength[thighR] = 2.5f;		BoneLength[calfR] = 2.0f;		BoneLength[footR] = 1.0f;		BoneLength[toeR] = 0.5f;

		// setup poses
		SetupPoses();
		// bone rotation - base pose
		SetPose(base);
	}

	void SetPose(Human_Pose pose)
	{
		for (int i = pelvis; i <= toeR; i++)
			BoneRotate[i] = Pose[pose][i];
	}

	void SetBoneRotation(Human_Bone bone, glm::quat q)
	{
		BoneRotate[bone] = q;
	}

	void MixPose(Human_Pose p, Human_Pose q, float t)
	{
		for (int i = pelvis; i <= toeR; i++)
			BoneRotate[i] = glm::mix(Pose[p][i], Pose[q][i], t);
	}

	void DrawHuman(Shader shader, unsigned int sphereVAO, int nSphereVert, glm::mat4 model)
	{
		glm::mat4 bone = model;
		glm::mat4 mpelvis = model;
		glm::mat4 mspine;


		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		// draw pelvis
		bone = bone * glm::mat4_cast(BoneRotate[pelvis]);
		bone = glm::scale(bone, glm::vec3(1.0f, BoneLength[pelvis], 1.0f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 1.0f, 1.0f, 0.0f);
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
		bone = glm::scale(bone, glm::vec3(1.0f, 1.0f / BoneLength[pelvis], 1.0f));
		// draw spine
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[pelvis], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[spine]);
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[spine], 0.0f));
		bone = glm::scale(bone, glm::vec3(1.0f, BoneLength[spine], 1.0f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 1.0f, 0.5f, 0.3f);
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
		bone = glm::scale(bone, glm::vec3(1.0f, 1.0f / BoneLength[spine], 1.0f));
		mspine = bone;
		// draw neck
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[spine], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[neck]);
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[neck], 0.0f));
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[neck], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.0f, 0.5f, 1.0f);
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[neck], 2.0f));
		// draw head
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[neck], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[head]);
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[head], 0.0f));
		bone = glm::scale(bone, glm::vec3(1.0f, BoneLength[head], 1.0f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.5f, 0.5f, 1.0f);
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
		bone = glm::scale(bone, glm::vec3(1.0f, 1.0f / BoneLength[head], 1.0f));
		// draw clavicleL
		bone = mspine;
		bone = glm::translate(bone, glm::vec3(1.0f, BoneLength[spine], 0.0f));
		bone = glm::rotate(bone, glm::radians(190.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		bone = bone * glm::mat4_cast(BoneRotate[clavicleL]);
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[clavicleL], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.0f, 0.7f, 0.0f);
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[clavicleL], 2.0f));
		// draw upperarmL
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[clavicleL], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[upperarmL]);
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[upperarmL], 0.0f));
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[upperarmL], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.3f, 0.0f, 0.7f);
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[upperarmL], 2.0f));
		// draw forearmL
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[upperarmL], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[forearmL]);
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[forearmL], 0.0f));
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[forearmL], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.7f, 0.0f, 0.5f);
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[forearmL], 2.0f));
		// draw handL
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[forearmL], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[handL]);
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[handL], 0.0f));
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[handL], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.0f, 0.5f, 0.5f);
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[handL], 2.0f));
		// draw clavicleR
		bone = mspine;
		bone = glm::translate(bone, glm::vec3(-1.0f, BoneLength[spine], 0.0f));
		bone = glm::rotate(bone, glm::radians(170.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		bone = bone * glm::mat4_cast(BoneRotate[clavicleR]);
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[clavicleR], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.0f, 0.7f, 0.0f);
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[clavicleR], 2.0f));
		// draw upperarmR
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[clavicleR], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[upperarmR]);
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[upperarmR], 0.0f));
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[upperarmR], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.3f, 0.0f, 0.7f);
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[upperarmR], 2.0f));
		// draw forearmR
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[upperarmR], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[forearmR]);
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[forearmR], 0.0f));
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[forearmR], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.7f, 0.0f, 0.5f);
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[forearmR], 2.0f));
		// draw handR
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[forearmR], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[handR]);
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[handR], 0.0f));
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[handR], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.0f, 0.5f, 0.5f);
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[handR], 2.0f));
		// draw thighL
		bone = mpelvis;
		bone = glm::translate(bone, glm::vec3(-1.0f, BoneLength[pelvis], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[thighL]);
		bone = glm::translate(bone, glm::vec3(0.0f, -BoneLength[thighL], 0.0f));
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[thighL], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.0f, 0.7f, 0.0f);
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
		bone = glm::scale(bone, glm::vec3(1.0f, 1.0f / BoneLength[thighL], 1.0f));
		// draw calfL
		bone = glm::translate(bone, glm::vec3(0.0f, -BoneLength[thighL], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[calfL]);
		bone = glm::translate(bone, glm::vec3(0.0f, -BoneLength[calfL], 0.0f));
		bone = glm::scale(bone, glm::vec3(1.0f, BoneLength[calfL], 1.0f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.3f, 0.0f, 0.7f);
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
		bone = glm::scale(bone, glm::vec3(1.0f, 1.0f / BoneLength[calfL], 1.0f));
		// draw footL
		bone = glm::translate(bone, glm::vec3(0.0f, -BoneLength[calfL], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[footL]);
		bone = glm::translate(bone, glm::vec3(0.0f, -BoneLength[footL], 0.0f));
		bone = glm::scale(bone, glm::vec3(1.0f, BoneLength[footL], 1.0f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.7f, 0.0f, 0.5f);
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
		bone = glm::scale(bone, glm::vec3(1.0f, 1.0f / BoneLength[footL], 1.0f));
		// draw toeL
		bone = glm::translate(bone, glm::vec3(0.0f, -BoneLength[footL], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[toeL]);
		bone = glm::translate(bone, glm::vec3(0.0f, -BoneLength[toeL], 0.0f));
		bone = glm::scale(bone, glm::vec3(1.0f, BoneLength[toeL], 1.0f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.0f, 0.5f, 0.5f);
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[toeL], 2.0f));
		// draw thighR
		bone = mpelvis;
		bone = glm::translate(bone, glm::vec3(1.0f, BoneLength[pelvis], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[thighR]);
		bone = glm::translate(bone, glm::vec3(0.0f, -BoneLength[thighR], 0.0f));
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[thighR], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.0f, 0.7f, 0.0f);
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
		bone = glm::scale(bone, glm::vec3(1.0f, 1.0f / BoneLength[thighR], 1.0f));
		// draw calfR
		bone = glm::translate(bone, glm::vec3(0.0f, -BoneLength[thighR], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[calfR]);
		bone = glm::translate(bone, glm::vec3(0.0f, -BoneLength[calfR], 0.0f));
		bone = glm::scale(bone, glm::vec3(1.0f, BoneLength[calfR], 1.0f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.3f, 0.0f, 0.7f);
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
		bone = glm::scale(bone, glm::vec3(1.0f, 1.0f / BoneLength[calfR], 1.0f));
		// draw footR
		bone = glm::translate(bone, glm::vec3(0.0f, -BoneLength[calfR], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[footR]);
		bone = glm::translate(bone, glm::vec3(0.0f, -BoneLength[footR], 0.0f));
		bone = glm::scale(bone, glm::vec3(1.0f, BoneLength[footR], 1.0f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.7f, 0.0f, 0.5f);
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
		bone = glm::scale(bone, glm::vec3(1.0f, 1.0f / BoneLength[footR], 1.0f));
		// draw toeR
		bone = glm::translate(bone, glm::vec3(0.0f, -BoneLength[footR], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[toeR]);
		bone = glm::translate(bone, glm::vec3(0.0f, -BoneLength[toeR], 0.0f));
		bone = glm::scale(bone, glm::vec3(1.0f, BoneLength[toeR], 1.0f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.0f, 0.5f, 0.5f);
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
		bone = glm::scale(bone, glm::vec3(1.0f, 1.0f / BoneLength[toeR], 1.0f));
		// glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

private:
	float		BoneLength[BONENUM];
	glm::quat	Pose[POSENUM][BONENUM];

	void SetupPoses()
	{
		for (int p = 0; p < POSENUM; p++)
		{
			for (int i = 0; i < BONENUM; i++)
				Pose[p][i] = glm::angleAxis(glm::radians(0.f), glm::vec3(0.f, 0.f, 1.f));

			switch (p) {
				case base:
					//Pose[base][clavicleL] = glm::angleAxis(glm::radians(-180.f), glm::vec3(0.f, 0.f, 1.f));
					//Pose[base][clavicleR] = glm::angleAxis(glm::radians( 180.f), glm::vec3(0.f, 0.f, 1.f));
					break;

				/*
				case armLeftUp:
					Pose[armRightUp][upperarmR] = glm::angleAxis(glm::radians(-50.f), glm::vec3(0.f, 0.f, 1.f));
					Pose[armRightUp][forearmR] = glm::angleAxis(glm::radians(-50.f), glm::vec3(0.f, 0.f, 1.f));
					break;
				case armRightUp:
					Pose[armRightUp][upperarmR] = glm::angleAxis(glm::radians(-50.f), glm::vec3(0.f, 0.f, 1.f));
					Pose[armRightUp][forearmR] = glm::angleAxis(glm::radians(-50.f), glm::vec3(0.f, 0.f, 1.f));
					break;
				case legLeftUp:
					Pose[legLeftUp][thighL] = glm::angleAxis(glm::radians(45.f), glm::vec3(1.f, 0.f, 0.f));
					Pose[legLeftUp][calfL] = glm::angleAxis(glm::radians(30.f), glm::vec3(1.f, 0.f, 0.f));
					break;

				case legRightUp:
					Pose[legRightUp][thighR] = glm::angleAxis(glm::radians(45.f), glm::vec3(1.f, 0.f, 0.f));
					Pose[legRightUp][calfR] = glm::angleAxis(glm::radians(30.f), glm::vec3(1.f, 0.f, 0.f));
					break;
				*/

				case walk1:
					Pose[walk1][upperarmL] = glm::angleAxis(glm::radians(30.f), glm::vec3(1.f, 0.f, 0.f));
					Pose[walk1][upperarmR] = glm::angleAxis(glm::radians(-30.f), glm::vec3(1.f, 0.f, 0.f));
					Pose[walk1][forearmL] = glm::angleAxis(glm::radians(60.f), glm::vec3(1.f, 0.f, 0.f));
					Pose[walk1][thighL] = glm::angleAxis(glm::radians(-30.f), glm::vec3(1.f, 0.f, 0.f));
					Pose[walk1][thighR] = glm::angleAxis(glm::radians(30.f), glm::vec3(1.f, 0.f, 0.f));
					Pose[walk1][calfL] = glm::angleAxis(glm::radians(60.f), glm::vec3(1.f, 0.f, 0.f));
					Pose[walk1][footL] = glm::angleAxis(glm::radians(-60.f), glm::vec3(1.f, 0.f, 0.f));
					break;
				case walk2:
					Pose[walk2][upperarmL] = glm::angleAxis(glm::radians(-30.f), glm::vec3(1.f, 0.f, 0.f));
					Pose[walk2][upperarmR] = glm::angleAxis(glm::radians(30.f), glm::vec3(1.f, 0.f, 0.f));
					Pose[walk2][forearmR] = glm::angleAxis(glm::radians(60.f), glm::vec3(1.f, 0.f, 0.f));
					Pose[walk2][thighL] = glm::angleAxis(glm::radians(30.f), glm::vec3(1.f, 0.f, 0.f));
					Pose[walk2][thighR] = glm::angleAxis(glm::radians(-30.f), glm::vec3(1.f, 0.f, 0.f));
					Pose[walk2][calfR] = glm::angleAxis(glm::radians(60.f), glm::vec3(1.f, 0.f, 0.f));
					Pose[walk2][footR] = glm::angleAxis(glm::radians(-60.f), glm::vec3(1.f, 0.f, 0.f));
					break;
			}
		}
	}
};
#endif