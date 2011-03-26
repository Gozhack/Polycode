/*
 *  PolySkeleton.cpp
 *  Poly
 *
 *  Created by Ivan Safrin on 9/4/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "PolySkeleton.h"

using namespace Polycode;

Skeleton::Skeleton(String fileName) : SceneEntity() {
	loadSkeleton(fileName);
	currentAnimation = NULL;
}

Skeleton::Skeleton() {
	currentAnimation = NULL;	
}

Skeleton::~Skeleton() {
}

int Skeleton::getNumBones() {
	return bones.size();
}

Bone *Skeleton::getBoneByName(String name) {
	for(int i=0; i < bones.size(); i++) {
		if(bones[i]->getName() == name)
			return bones[i];
	}
	return NULL;
}

Bone *Skeleton::getBone(int index) {
	return bones[index];
}

void Skeleton::enableBoneLabels(String labelFont, Number size, Number scale, Color labelColor) {
	for(int i=0; i < bones.size(); i++) {
		bones[i]->enableBoneLabel(labelFont, size, scale,labelColor);
	}	
	
	SceneLabel *label = new SceneLabel(labelFont, "Skeleton", size, scale, Label::ANTIALIAS_FULL);
	label->setColor(labelColor);
	label->billboardMode = true;
	label->setDepthWrite(false);
	addEntity(label);
	
}

void Skeleton::playAnimationByIndex(int index) {
	if(index > animations.size()-1)
		return;
		
	SkeletonAnimation *anim = animations[index];
	if(!anim)
		return;
	
	if(anim == currentAnimation)
		return;
	
	if(currentAnimation)
		currentAnimation->Stop();
	
	currentAnimation = anim;
	anim->Play();	
}

void Skeleton::playAnimation(String animName) {
	SkeletonAnimation *anim = getAnimation(animName);
	if(!anim)
		return;
	
	if(anim == currentAnimation)
		return;
	
	if(currentAnimation)
		currentAnimation->Stop();
		
	currentAnimation = anim;
	anim->Play();
}

SkeletonAnimation *Skeleton::getAnimation(String name) {
	for(int i=0; i < animations.size(); i++) {
		if(animations[i]->getName() == name)
			return animations[i];
	}
	return NULL;
}

void Skeleton::Update() {

	if(currentAnimation != NULL) {
		currentAnimation->Update();
	}
}

void Skeleton::loadSkeleton(String fileName) {
	OSFILE *inFile = OSBasics::open(fileName.c_str(), "rb");
	if(!inFile) {
		return;
	}
	
	bonesEntity	= new SceneEntity();
	bonesEntity->visible = false;
	addChild(bonesEntity);
	
	unsigned int numBones;
	float t[3],rq[4],s[3];
	
	OSBasics::read(&numBones, sizeof(unsigned int), 1, inFile);
	unsigned int namelen;
	char buffer[1024];
	
	Matrix4 mat;
	unsigned int hasParent, boneID;
	for(int i=0; i < numBones; i++) {
		
		OSBasics::read(&namelen, sizeof(unsigned int), 1, inFile);
		memset(buffer, 0, 1024);
		OSBasics::read(buffer, 1, namelen, inFile);
		
		Bone *newBone = new Bone(string(buffer));		
		
		OSBasics::read(&hasParent, sizeof(unsigned int), 1, inFile);
		if(hasParent == 1) {
			OSBasics::read(&boneID, sizeof(unsigned int), 1, inFile);
			newBone->parentBoneId = boneID;
		} else {
			newBone->parentBoneId = -1;
		}

		OSBasics::read(t, sizeof(float), 3, inFile);
		OSBasics::read(s, sizeof(float), 3, inFile);
		OSBasics::read(rq, sizeof(float), 4, inFile);
		
		bones.push_back(newBone);
		
		newBone->setPosition(t[0], t[1], t[2]);
		newBone->setRotationQuat(rq[0], rq[1], rq[2], rq[3]);
		newBone->setScale(s[0], s[1], s[2]);
		newBone->rebuildTransformMatrix();
		
		newBone->setBaseMatrix(newBone->getTransformMatrix());
		newBone->setBoneMatrix(newBone->getTransformMatrix());

		OSBasics::read(t, sizeof(float), 3, inFile);
		OSBasics::read(s, sizeof(float), 3, inFile);
		OSBasics::read(rq, sizeof(float), 4, inFile);
		
		Quaternion q;
		q.set(rq[0], rq[1], rq[2], rq[3]);
		Matrix4 m = q.createMatrix();
		m.setPosition(t[0], t[1], t[2]);
		
		newBone->setRestMatrix(m);
		
	}

	Bone *parentBone;
//	SceneEntity *bProxy;
	
	for(int i=0; i < bones.size(); i++) {
		if(bones[i]->parentBoneId != -1) {
			parentBone = bones[bones[i]->parentBoneId];
			parentBone->addChildBone(bones[i]);
			bones[i]->setParentBone(parentBone);
			parentBone->addEntity(bones[i]);			
//			addEntity(bones[i]);										
			
			SceneLine *connector = new SceneLine(bones[i], parentBone);
			connector->depthTest = false;
			bonesEntity->addEntity(connector);				
			connector->setColor(((Number)(rand() % RAND_MAX)/(Number)RAND_MAX),((Number)(rand() % RAND_MAX)/(Number)RAND_MAX),((Number)(rand() % RAND_MAX)/(Number)RAND_MAX),1.0f);
		} else {
//			bProxy = new SceneEntity();
//			addEntity(bProxy);			
//			bProxy->addEntity(bones[i]);
			bonesEntity->addChild(bones[i]);
		}
	//	bones[i]->visible = false;			
	}
	/*
	unsigned int numAnimations, activeBones,boneIndex,numPoints,numCurves, curveType;
	OSBasics::read(&numAnimations, sizeof(unsigned int), 1, inFile);
	//Logger::log("numAnimations: %d\n", numAnimations);
	for(int i=0; i < numAnimations; i++) {
		OSBasics::read(&namelen, sizeof(unsigned int), 1, inFile);
		memset(buffer, 0, 1024);
		OSBasics::read(buffer, 1, namelen, inFile);
		float length;
		OSBasics::read(&length, 1, sizeof(float), inFile);
		SkeletonAnimation *newAnimation = new SkeletonAnimation(buffer, length);
		
		OSBasics::read(&activeBones, sizeof(unsigned int), 1, inFile);

	//	Logger::log("activeBones: %d\n", activeBones);		
		for(int j=0; j < activeBones; j++) {
			OSBasics::read(&boneIndex, sizeof(unsigned int), 1, inFile);
			BoneTrack *newTrack = new BoneTrack(bones[boneIndex], length);
			
			BezierCurve *curve;
			float vec1[2]; //,vec2[2],vec3[2];

			OSBasics::read(&numCurves, sizeof(unsigned int), 1, inFile);
//			Logger::log("numCurves: %d\n", numCurves);					
			for(int l=0; l < numCurves; l++) {
				curve = new BezierCurve();
				OSBasics::read(&curveType, sizeof(unsigned int), 1, inFile);
				OSBasics::read(&numPoints, sizeof(unsigned int), 1, inFile);
				for(int k=0; k < numPoints; k++) {					
					OSBasics::read(vec1, sizeof(float), 2, inFile);					
					curve->addControlPoint2d(vec1[1], vec1[0]);
//					curve->addControlPoint(vec1[1]-10, vec1[0], 0, vec1[1], vec1[0], 0, vec1[1]+10, vec1[0], 0);
				}
				switch(curveType) {
					case 0:
						newTrack->scaleX = curve;
					break;
					case 1:
						newTrack->scaleY = curve;
					break;
					case 2:
						newTrack->scaleZ = curve;					
					break;
					case 3:
						newTrack->QuatW = curve;					
					break;
					case 4:
						newTrack->QuatX = curve;					
					break;
					case 5:
						newTrack->QuatY = curve;					
					break;
					case 6:
						newTrack->QuatZ = curve;					
					break;
					case 7:;
						newTrack->LocX = curve;					
					break;
					case 8:
						newTrack->LocY = curve;					
					break;
					case 9:
						newTrack->LocZ = curve;					
					break;
				}
			}
			
			newAnimation->addBoneTrack(newTrack);
		}
		animations.push_back(newAnimation);
	}
	*/
	OSBasics::close(inFile);
}

void Skeleton::addAnimation(String name, String fileName) {
	OSFILE *inFile = OSBasics::open(fileName.c_str(), "rb");
	if(!inFile) {
		return;
	}
	
		unsigned int activeBones,boneIndex,numPoints,numCurves, curveType;	
		float length;
		OSBasics::read(&length, 1, sizeof(float), inFile);
		SkeletonAnimation *newAnimation = new SkeletonAnimation(name, length);
		
		OSBasics::read(&activeBones, sizeof(unsigned int), 1, inFile);
		
		//	Logger::log("activeBones: %d\n", activeBones);		
		for(int j=0; j < activeBones; j++) {
			OSBasics::read(&boneIndex, sizeof(unsigned int), 1, inFile);
			BoneTrack *newTrack = new BoneTrack(bones[boneIndex], length);
			
			BezierCurve *curve;
			float vec1[2]; //,vec2[2],vec3[2];
			
			OSBasics::read(&numCurves, sizeof(unsigned int), 1, inFile);
			//			Logger::log("numCurves: %d\n", numCurves);					
			for(int l=0; l < numCurves; l++) {
				curve = new BezierCurve();
				OSBasics::read(&curveType, sizeof(unsigned int), 1, inFile);
				OSBasics::read(&numPoints, sizeof(unsigned int), 1, inFile);
				for(int k=0; k < numPoints; k++) {					
					OSBasics::read(vec1, sizeof(float), 2, inFile);					
					curve->addControlPoint2d(vec1[1], vec1[0]);
					//					curve->addControlPoint(vec1[1]-10, vec1[0], 0, vec1[1], vec1[0], 0, vec1[1]+10, vec1[0], 0);
				}
				switch(curveType) {
					case 0:
						newTrack->scaleX = curve;
						break;
					case 1:
						newTrack->scaleY = curve;
						break;
					case 2:
						newTrack->scaleZ = curve;					
						break;
					case 3:
						newTrack->QuatW = curve;					
						break;
					case 4:
						newTrack->QuatX = curve;					
						break;
					case 5:
						newTrack->QuatY = curve;					
						break;
					case 6:
						newTrack->QuatZ = curve;					
						break;
					case 7:;
						newTrack->LocX = curve;					
						break;
					case 8:
						newTrack->LocY = curve;					
						break;
					case 9:
						newTrack->LocZ = curve;					
						break;
				}
			}
			
			newAnimation->addBoneTrack(newTrack);
		}
		animations.push_back(newAnimation);
	
	
	OSBasics::close(inFile);	
}

void Skeleton::bonesVisible(bool val) {
	bonesEntity->visible = val;
}

BoneTrack::BoneTrack(Bone *bone, Number length) {
	this->length = length;
	targetBone = bone;
	scaleX = NULL;
	scaleY = NULL;
	scaleZ = NULL;
	QuatW = NULL;
	QuatX = NULL;
	QuatY = NULL;
	QuatZ = NULL;
	LocX = NULL;			
	LocY = NULL;
	LocZ = NULL;
	initialized = false;
}

BoneTrack::~BoneTrack() {
}


void BoneTrack::Stop() {
	if(initialized) {
		for(int i=0; i < pathTweens.size(); i++) {
			pathTweens[i]->Pause(true);
		}	
		quatTween->Pause(true);		
	}
}

void BoneTrack::Play() {

	if(!initialized ) {
	// TODO: change it so that you can set the tweens to not manually restart so you can calculate the
	// time per tween
	
		Number durTime = length; //(LocX->getControlPoint(LocX->getNumControlPoints()-1)->p2.x);//25.0f;
					
	BezierPathTween *testTween;		
	if(LocX) {
		testTween = new BezierPathTween(&LocXVec, LocX, Tween::EASE_NONE, durTime, true);
		pathTweens.push_back(testTween);
	}
	if(LocY) {		
		testTween = new BezierPathTween(&LocYVec, LocY, Tween::EASE_NONE, durTime, true);
		pathTweens.push_back(testTween);
	}
		
	if(LocZ) {
		testTween = new BezierPathTween(&LocZVec, LocZ, Tween::EASE_NONE, durTime, true);
		pathTweens.push_back(testTween);
	}
	testTween = new BezierPathTween(&ScaleXVec, scaleX, Tween::EASE_NONE, durTime, true);
	pathTweens.push_back(testTween);
	testTween = new BezierPathTween(&ScaleYVec, scaleY, Tween::EASE_NONE, durTime, true);
	pathTweens.push_back(testTween);
	testTween = new BezierPathTween(&ScaleZVec, scaleZ, Tween::EASE_NONE, durTime, true);
	pathTweens.push_back(testTween);
		
		
	if(QuatW)
		quatTween = new QuaternionTween(&boneQuat, QuatW, QuatX, QuatY, QuatZ, Tween::EASE_NONE, durTime, true);

	initialized = true;
	} else {
		for(int i=0; i < pathTweens.size(); i++) {
			pathTweens[i]->Pause(false);
		}	
		quatTween->Pause(false);
	}
/*
	if(QuatW) {
		testTween = new BezierPathTween(&QuatWVec, QuatW, Tween::EASE_NONE, durTime, true);
		pathTweens.push_back(testTween);
	}
	
	if(QuatX) {
		testTween = new BezierPathTween(&QuatXVec, QuatX, Tween::EASE_NONE, durTime, true);
		pathTweens.push_back(testTween);
	}
	
	if(QuatY) {
		testTween = new BezierPathTween(&QuatYVec, QuatY, Tween::EASE_NONE, durTime, true);
		pathTweens.push_back(testTween);
	}
	
	if(QuatZ) {
		testTween = new BezierPathTween(&QuatZVec, QuatZ, Tween::EASE_NONE, durTime, true);
		pathTweens.push_back(testTween);
	}
*/
}


void BoneTrack::Update() {

	Matrix4 newMatrix;
	newMatrix = boneQuat.createMatrix();

	
	Matrix4 scaleMatrix;
	scaleMatrix.m[0][0] *= 1; //ScaleXVec.y;
	scaleMatrix.m[1][1] *= 1; //ScaleYVec.y;
	scaleMatrix.m[2][2] *= 1; //ScaleZVec.y;
	

	Matrix4 posMatrix;

	if(LocX)
		posMatrix.m[3][0] = LocXVec.y;		
	else
		posMatrix.m[3][0] = targetBone->getBaseMatrix()[3][0];

	if(LocY)
		posMatrix.m[3][1] = LocYVec.y;		
	else
		posMatrix.m[3][1] = targetBone->getBaseMatrix()[3][1];
	
	if(LocZ)
		posMatrix.m[3][2] = LocZVec.y;		
	else
		posMatrix.m[3][2] = targetBone->getBaseMatrix()[3][2];
	
	
	newMatrix = scaleMatrix*newMatrix*posMatrix;	
	
	targetBone->setBoneMatrix(newMatrix);
	targetBone->setTransformByMatrixPure(newMatrix);		

}

void BoneTrack::setSpeed(Number speed) {
	for(int i=0; i < pathTweens.size(); i++) {
		pathTweens[i]->setSpeed(speed);
	}	
	quatTween->setSpeed(speed);
}


SkeletonAnimation::SkeletonAnimation(String name, Number duration) {
	this->name = name;
	this->duration = duration;
}

void SkeletonAnimation::setSpeed(Number speed) {
	for(int i=0; i < boneTracks.size(); i++) {
		boneTracks[i]->setSpeed(speed);
	}	
}

void SkeletonAnimation::Update() {
	for(int i=0; i < boneTracks.size(); i++) {
		boneTracks[i]->Update();
	}
}

void SkeletonAnimation::Stop() {
	for(int i=0; i < boneTracks.size(); i++) {
		boneTracks[i]->Stop();
	}
}

void SkeletonAnimation::Play() {
	for(int i=0; i < boneTracks.size(); i++) {
		boneTracks[i]->Play();
	}
}

SkeletonAnimation::~SkeletonAnimation() {

}

String SkeletonAnimation::getName() {
	return name;
}

void SkeletonAnimation::addBoneTrack(BoneTrack *boneTrack) {
	boneTracks.push_back(boneTrack);
}
