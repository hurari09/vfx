//////////////////////////////////////////
//		jieunlee@hansung.ac.kr			//
//		2022. 11. 20					//
//////////////////////////////////////////

#ifndef BEZIER_H
#define BEZIER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>


// BezierCrv Curve
class CubicBezierCrv	// cubic Bezier curve
{
public:
	glm::vec3 P[7];		// control points

	// Constructor 
	CubicBezierCrv()		
	{
		P[0] = glm::vec3(3.0f, 3.0f, -0.5f);
		P[1] = glm::vec3(-3.0f, 3.0f, 0.5f);
		P[2] = glm::vec3(3.0f, 3.0f, 0.5f);
		P[3] = glm::vec3(3.0f, -3.0f, -0.5f);
		P[4] = glm::vec3(-3.0f, -3.0f, -0.5f);
		P[5] = glm::vec3(-3.0f, 3.0f, 0.5f);
		P[6] = glm::vec3(3.0f, 3.0f, 0.5f);
	}

	glm::vec3 Eval(float t)
	{
		glm::vec3 p0 = P[(int)t];
		glm::vec3 p1 = P[(int)t + 1];
		glm::vec3 p2 = P[(int)t + 2];
		glm::vec3 p3 = P[(int)t + 3];

		glm::vec3 v1 = 0.5f * (p2 - p0);
		glm::vec3 v2 = 0.5f * (p3 - p1);

		t -= (int)t;

		float t2 = t * t;
		float t3 = t * t * t;

		float s1 = 2.0f * t3 - 3.0f * t2 + 1.0f;
		float s2 = t3 - 2.0f * t2 + t;
		float s3 = -2.0f * t3 + 3.0f * t2;
		float s4 = t3 - t2;

		return s1 * p1 + s2 * v1 + s3 * p2 + s4 * v2;
	}

private:
};
#endif