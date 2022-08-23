#pragma once

// Include template for singleton
#include "..\DesignPatterns\SingletonTemplate.h"

// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include mouse control handler
#include "../Inputs/MouseController.h"


// Include CEntity2D
#include "Entity2D.h"
#include <cmath>

class Camera2D : public CSingletonTemplate<Camera2D>
{
	friend CSingletonTemplate<Camera2D>;

public:
	// constants
	const float kCameraSpeed = 0.15f;
	const int kScreenShakeSpeedHor = 70.0f;
	const int kScreenShakeSpeedVert = 60.0f;

	Camera2D(void);
	~Camera2D(void);

	void Update(float dElapsedTime);
	void Reset();
	
	//getters and settersss
	float getZoom();
	glm::vec2 getPos();
	
	void setTargetPos(glm::vec2 targetPos);
	void setTargetZoom(float targetZoom);
	void setContraint(bool newConstraint, bool isX = true);

	glm::vec2 getMousePosition();
	glm::vec2 getBlockSelected();
	glm::vec2 getMousePositionWindow();
	glm::vec2 getBlockPositionWindow(glm::vec2 blockPos);
	bool isInMap();
	bool noiseOn;
	// screen shake stream bamboleo by red velvet
	double timeElapsedHor = 0.0f;
	double timeElapsedVert = 0.0f;
	float amplitude = 5.0f;
	float adjustAmplitudePerSecond = -1.0f;

	float lerp(float x, float y, float t) {
		return x * (1.f - t) + y * t;
	}

private:
	glm::vec2 pos;
	glm::vec2 targetPos;
	float zoom;
	float targetZoom;
	bool SetupState;
	bool ConstraintX;
	bool ConstraintY;

	// mouse mouse control control
	glm::vec2 vMousePosInWindow;
	glm::vec2 vMousePosConvertedRatio;
	glm::vec2 vMousePosWorldSpace;
	glm::vec2 vMousePosRelativeToCamera;
};