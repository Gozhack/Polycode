/*
 *  PolyRenderer.cpp
 *  TAU
 *
 *  Created by Ivan Safrin on 3/12/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "PolyRenderer.h"

using namespace Polycode;

Renderer::Renderer() : currentTexture(NULL), xRes(0), yRes(0), renderMode(0), orthoMode(false), lightingEnabled(false), clearColor(0.2f, 0.2f, 0.2f, 0.0) {
	textureFilteringMode = TEX_FILTERING_LINEAR;
	currentMaterial = NULL;
	numLights = 0;
	exposureLevel = 1;
	shadersEnabled = true;
	currentFrameBufferTexture = NULL;
	previousFrameBufferTexture = NULL;
	currentMaterial = NULL;
	numLights = 0;
	numAreaLights = 0;
	numSpotLights = 0;	
	exposureLevel = 1;
	shadersEnabled = true;
	currentShaderModule = NULL;
	currentFrameBufferTexture = NULL;
	previousFrameBufferTexture = NULL;
	fov = 45.0;
}

Renderer::~Renderer() {
}

void Renderer::enableShaders(bool flag) {
	shadersEnabled = flag;
}

void Renderer::setCameraMatrix(Matrix4 matrix) {
	cameraMatrix = matrix;
}

void Renderer::clearLights() {
	numLights = 0;
	numAreaLights = 0;
	numSpotLights = 0;	
	lights.clear();
	areaLights.clear();
	spotLights.clear();
	shadowMapTextures.clear();
}

void Renderer::addShadowMap(Texture *texture) {
	shadowMapTextures.push_back(texture);
}

void Renderer::setExposureLevel(Number level) {
	exposureLevel = level;
}

bool Renderer::rayTriangleIntersect(Vector3 ray_origin, Vector3 ray_direction, Vector3 vert0, Vector3 vert1, Vector3 vert2, Vector3 *hitPoint)
{
	Number t,u,v;
	t = 0; u = 0; v = 0;
	
	Vector3 edge1 = vert1 - vert0;
	Vector3 edge2 = vert2 - vert0;
	
	Vector3 tvec, pvec, qvec;
	Number det, inv_det;
	
	
	pvec = ray_direction.crossProduct(edge2);
	det = edge1.dot(pvec);
	
	if (det > -0.00001f)
		return false;
	
	inv_det = 1.0f / det;
	
	tvec = ray_origin - vert0;
	
	u = tvec.dot(pvec) * inv_det;
	if (u < -0.001f || u > 1.001f)
		return false;
	
	qvec = tvec.crossProduct(edge1);
	
	v = ray_direction.dot(qvec) * inv_det;
	if (v < -0.001f || u + v > 1.001f)
		return false;
	
	t = edge2.dot(qvec) * inv_det;
	
	if (t <= 0)
		return false;
	
	hitPoint->x = ray_origin.x+t*ray_direction.x;
	hitPoint->y = ray_origin.y+t*ray_direction.y;
	hitPoint->z = ray_origin.z+t*ray_direction.z;	
	
	return true;	
}

void Renderer::addShaderModule(PolycodeShaderModule *module) {
	shaderModules.push_back(module);
}

void Renderer::addLight(Vector3 position, Vector3 direction, int type, Color color, Number distance, Number intensity, Matrix4 *textureMatrix) {
	numLights++;
	
	LightInfo info;
	if(textureMatrix != NULL) {
		info.textureMatrix = *textureMatrix;
	}
	
	info.intensity = intensity;
	info.type = type;
	info.dir = direction;
	info.distance = distance;
	info.color.set(color.r, color.g, color.b);
	info.position = position;
	lights.push_back(info);
	switch(type) {
		case 0: //area light
			areaLights.push_back(info);
			numAreaLights++;
		break;
		case 1: //spot light
			spotLights.push_back(info);			
			numSpotLights++;
		break;			
	}
}

Matrix4 Renderer::getCameraMatrix() {
	return cameraMatrix;
}

void Renderer::setCameraPosition(Vector3 pos) {
	cameraPosition = pos;
	pos = pos * -1;
	this->translate3D(&pos);
}

void Renderer::billboardMatrixWithScale(Vector3 scale) {
	Matrix4 matrix = getModelviewMatrix();
	matrix.m[0][0] = 1.0f*scale.x;
	matrix.m[0][1] = 0;
	matrix.m[0][2] = 0;

	matrix.m[1][0] = 0;
	matrix.m[1][1] = 1.0f*scale.y;
	matrix.m[1][2] = 0;

	matrix.m[2][0] = 0;
	matrix.m[2][1] = 0;
	matrix.m[2][2] = 1.0f*scale.z;
	
	setModelviewMatrix(matrix);
}

void Renderer::billboardMatrix() {
	Matrix4 matrix = getModelviewMatrix();
	matrix.m[0][0] = 1;
	matrix.m[0][1] = 0;
	matrix.m[0][2] = 0;

	matrix.m[1][0] = 0;
	matrix.m[1][1] = 1;
	matrix.m[1][2] = 0;

	matrix.m[2][0] = 0;
	matrix.m[2][1] = 0;
	matrix.m[2][2] = 1;
	setModelviewMatrix(matrix);
}

void Renderer::pushDataArrayForMesh(Mesh *mesh, int arrayType) {
	if(mesh->arrayDirtyMap[arrayType] == true || mesh->renderDataArrays[arrayType] == NULL) {
		if(mesh->renderDataArrays[arrayType] != NULL) {
			free(mesh->renderDataArrays[arrayType]->arrayPtr);
			delete mesh->renderDataArrays[arrayType];
		}
		mesh->renderDataArrays[arrayType] = createRenderDataArrayForMesh(mesh, arrayType);
		mesh->arrayDirtyMap[arrayType] = false;
	}
	pushRenderDataArray(mesh->renderDataArrays[arrayType]);
}

int Renderer::getXRes() {
	return xRes;
}

int Renderer::getYRes() {
	return yRes;
}

void Renderer::setAmbientColor(Number r, Number g, Number b) {
	ambientColor.setColor(r,g,b,1.0f);
}

void Renderer::setClearColor(Color color) {
	setClearColor(color.r, color.g, color.b);
}

void Renderer::setRenderMode(int newRenderMode) {
	renderMode = newRenderMode;
}

void Renderer::setTextureFilteringMode(int mode) {
	textureFilteringMode = mode;
}

int Renderer::getRenderMode() {
	return renderMode;
}