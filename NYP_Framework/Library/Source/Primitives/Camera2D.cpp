#include "Camera2D.h"

// include relevant files
#include "../GameControl/Settings.h"
#include "../Inputs/MouseController.h"

#include <ctime>
#include <iostream>

Camera2D::Camera2D(void) :
	pos(0, 0),
	targetPos(0, 0),
	targetZoom(1.5f),
	SetupState(false),
	zoom(1.f)
{
}

Camera2D::~Camera2D(void)
{
}

void Camera2D::Update(float dElapsedTime) {
	if (!SetupState) {
		pos = targetPos;
		zoom = targetZoom;
		SetupState = true;
		return;
	}

	srand(static_cast <unsigned> (time(0)));

	if (noiseOn) {
		timeElapsedHor += double(dElapsedTime) * kScreenShakeSpeedHor;
		amplitude += adjustAmplitudePerSecond * dElapsedTime;
		targetPos.x += glm::sin(timeElapsedHor);

		timeElapsedVert += double(dElapsedTime) * kScreenShakeSpeedVert;
		amplitude += adjustAmplitudePerSecond * dElapsedTime;
		targetPos.y += glm::sin(timeElapsedVert);

	}

	//Lerp
	if (!ConstraintX) {
		pos.x = lerp(pos.x, targetPos.x, kCameraSpeed);
	} 
	if (!ConstraintY) {
		pos.y = lerp(pos.y, targetPos.y, kCameraSpeed);
	}

	zoom = lerp(zoom, targetZoom, kCameraSpeed);
	zoom = glm::clamp(zoom, 1.f, 2.f);
	targetZoom = glm::clamp(targetZoom, 1.f, 2.f);
}

void Camera2D::Reset()
{
	SetupState = false;
	ConstraintX = false;
	ConstraintY = false;
	noiseOn = false;
}

float Camera2D::getZoom()
{
	return zoom;
}

glm::vec2 Camera2D::getPos()
{
	return pos;
}

void Camera2D::setTargetPos(glm::vec2 targetPos)
{
	this->targetPos = targetPos;
}

void Camera2D::setTargetZoom(float targetZoom)
{
	this->targetZoom = targetZoom;
}

void Camera2D::setContraint(bool newConstraint, bool isX)
{
	(isX) ? (ConstraintX = newConstraint) : (ConstraintY = newConstraint);
}

glm::vec2 Camera2D::getMousePosition()
{
	// Create stuff
	CMouseController* cMouseController = CMouseController::GetInstance();
	CSettings* cSettings = CSettings::GetInstance();

	// Mouse Position calculator
	glm::vec2 vMousePosInWindow = glm::vec2(cMouseController->GetMousePositionX(), cSettings->iWindowHeight - cMouseController->GetMousePositionY());
	glm::vec2 vMousePosConvertedRatio = glm::vec2(vMousePosInWindow.x - cSettings->iWindowWidth * 0.5, vMousePosInWindow.y - cSettings->iWindowHeight * 0.5);
	glm::vec2 vMousePosWorldSpace = glm::vec2(vMousePosConvertedRatio.x / cSettings->iWindowWidth * cSettings->NUM_TILES_XAXIS, vMousePosConvertedRatio.y / cSettings->iWindowHeight * cSettings->NUM_TILES_YAXIS);
	glm::vec2 vMousePosRelativeToCamera = pos + vMousePosWorldSpace / zoom;

	return vMousePosRelativeToCamera;
}

glm::vec2 Camera2D::getBlockSelected()
{
	glm::vec2 mouseSelect = getMousePosition();
	return glm::vec2(int(mouseSelect.x), int(mouseSelect.y));
}

bool Camera2D::isInMap()
{
	CSettings* cSettings = CSettings::GetInstance();
	getMousePosition();

	return (0 <= vMousePosRelativeToCamera.x && vMousePosRelativeToCamera.x <= cSettings->NUM_TILES_XAXIS && 0 <= vMousePosRelativeToCamera.y && vMousePosRelativeToCamera.y <= cSettings->NUM_TILES_YAXIS);
}
