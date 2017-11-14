/*
	Copyright 2011-2017 Daniel S. Buckstein

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

/*
	animal3D SDK: Minimal 3D Animation Framework
	By Daniel S. Buckstein
	
	a3_ClipControl.h
	Animation clip and clip controller. Basically a frame index manager. Very 
	limited in what one can do with this; could potentially be so much more.
*/

#ifndef __ANIMAL3D_CLIPCONTROL_H
#define __ANIMAL3D_CLIPCONTROL_H


//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#else	// !__cplusplus
	typedef struct a3_Clip				a3_Clip;
	typedef struct a3_ClipGroup			a3_ClipGroup;
	typedef struct a3_ClipController	a3_ClipController;
#endif	// __cplusplus


//-----------------------------------------------------------------------------

	// description of single clip
	struct a3_Clip
	{
		// name
		char name[32];

		// first index in sequence of frames
		unsigned int first;

		// last index
		unsigned int last;

		// number of indices (including first and last)
		unsigned int count;

		// duration of clip (assuming clip is evenly divided)
		float clipDuration, clipDurationInv;

		// duration of frame (ditto)
		float frameDuration, frameDurationInv;
	};

	// group of clips
	struct a3_ClipGroup
	{
		a3_Clip *clips;
		unsigned int clipCount;
	};

	// clip control
	struct a3_ClipController
	{
		const a3_ClipGroup *clipGroup;
		unsigned int clipIndex;
		unsigned int frameIndex;
		unsigned int nextIndex;
		float playbackSpeed;
		float frameTime;
		float frameParam;
		float clipTime;
		float clipParam;
	};


//-----------------------------------------------------------------------------

	// allocate clip group
	inline int a3clipCreateGroup(a3_ClipGroup *clipGroup_out, const unsigned int clipCount);

	// release clip group
	inline int a3clipReleaseGroup(a3_ClipGroup *clipGroup);

	// get clip index by name
	inline int a3clipGetIndexInGroup(const a3_ClipGroup *clipGroup, const char name[32]);

	// initialize clip in group
	inline int a3clipInit(const a3_ClipGroup *clipGroup, const unsigned int clipIndex, const char name[32], unsigned int firstFrameIndex, unsigned int lastFrameIndex, float duration);

	// set clip controller to clip
	//	(may want additional parameters)
	inline int a3clipCtrlSet(a3_ClipController *ctrl, const a3_ClipGroup *clipGroup, const unsigned int clipIndex);

	// update clip controller
	inline int a3clipCtrlUpdate(a3_ClipController *ctrl, const float dt);


//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif	// __cplusplus


#endif	// !__ANIMAL3D_CLIPCONTROL_H