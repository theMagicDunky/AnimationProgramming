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
	
	a3_ClipControl.c
	Implementation of clip controller.
*/

#include "a3_ClipControl.h"

#include <stdlib.h>
#include <string.h>


//-----------------------------------------------------------------------------

// allocate clip group
extern inline int a3clipCreateGroup(a3_ClipGroup *clipGroup_out, const unsigned int clipCount)
{
	if (clipGroup_out && !clipGroup_out->clips && clipCount)
	{
		const unsigned int bytes = clipCount * sizeof(a3_Clip);
		clipGroup_out->clipCount = clipCount;
		clipGroup_out->clips = (a3_Clip *)malloc(bytes);
		memset(clipGroup_out->clips, 0, bytes);
		return clipCount;
	}
	return -1;
}

// release clip group
extern inline int a3clipReleaseGroup(a3_ClipGroup *clipGroup)
{
	if (clipGroup && clipGroup->clips)
	{
		const unsigned int clipCount = clipGroup->clipCount;
		free(clipGroup->clips);
		clipGroup->clips = 0;
		clipGroup->clipCount = 0;
		return clipCount;
	}
	return -1;
}

// get clip index by name
extern inline int a3clipGetIndexInGroup(const a3_ClipGroup *clipGroup, const char name[32])
{
	if (clipGroup && clipGroup->clips)
	{
		unsigned int i;
		for (i = 0; i < clipGroup->clipCount; ++i)
			if (strncmp(name, clipGroup->clips[i].name, 32) == 0)
				return i;
	}
	return -1;
}

// initialize clip in group
extern inline int a3clipInit(const a3_ClipGroup *clipGroup, const unsigned int clipIndex, const char name[32], unsigned int firstFrameIndex, unsigned int lastFrameIndex, float duration)
{
	if (clipGroup && clipGroup->clips && clipIndex < clipGroup->clipCount && firstFrameIndex <= lastFrameIndex)
	{
		a3_Clip *clip = clipGroup->clips + clipIndex;

		strncpy(clip->name, name, sizeof(clip->name));
		clip->first = firstFrameIndex;
		clip->last = lastFrameIndex;
		clip->count = 1 + clip->last - clip->first;
		clip->clipDuration = clip->clipDurationInv = (firstFrameIndex != lastFrameIndex && duration > 0.0f) ? duration : 0.0f;
		clip->frameDuration = clip->frameDurationInv = clip->clipDuration / (float)(clip->count);
		if (clip->clipDuration > 0.0f)
		{
			clip->clipDurationInv = 1.0f / clip->clipDuration;
			clip->frameDurationInv = 1.0f / clip->frameDuration;
		}
		return clipIndex;
	}
	return -1;
}

// set clip controller to clip
//	(may want additional parameters)
extern inline int a3clipCtrlSet(a3_ClipController *ctrl, const a3_ClipGroup *clipGroup, const unsigned int clipIndex)
{
	if (ctrl && clipGroup && clipGroup->clips && clipIndex < clipGroup->clipCount)
	{
		const a3_Clip *clip = clipGroup->clips + clipIndex;

		ctrl->clipGroup = clipGroup;
		ctrl->clipIndex = clipIndex;
		ctrl->playbackSpeed = 1.0f;
		ctrl->clipTime = 0.0f;
		ctrl->frameParam = 0.0f;
		
		ctrl->frameIndex = clip->first;
		ctrl->nextIndex = clip->first + (clip->count > 1);

		return clipIndex;
	}
	return -1;
}

// update clip controller
extern inline int a3clipCtrlUpdate(a3_ClipController *ctrl, const float dt)
{
	if (ctrl && ctrl->clipGroup)
	{
		// ****TO-DO: 
		// if the clip being played *can* update (e.g. has duration)
		//	add to clip time and calculate parameter
		// if sample time exceeds sample duration, move to next frame
		const a3_Clip *clip = ctrl->clipGroup->clips + ctrl->clipIndex;
		if (clip->clipDuration)
		{
			// accumulate time
			float dtFinal = dt * ctrl->playbackSpeed;
			ctrl->frameTime += dtFinal;

			// play forward
			if (dtFinal > 0.0f)
			{			
				if (ctrl->frameTime >= clip->frameDuration)
				{
					ctrl->frameTime -= clip->frameDuration;
					ctrl->frameIndex = ctrl->nextIndex;
					ctrl->nextIndex = (1 + ctrl->nextIndex - clip->first) % (clip->count) + clip->first;
				}

			}
			// play reverse
			else if (dtFinal < 0.0f)
			{
				if (ctrl->frameTime < 0.0f)
				{
					ctrl->frameTime += clip->frameDuration;
					ctrl->nextIndex = ctrl->frameIndex;
				}
			}

			// calc frame param
			ctrl->frameParam = ctrl->frameTime * clip->frameDurationInv;

			ctrl->clipTime = (ctrl->frameIndex - clip->first) * clip->frameDuration + ctrl->frameTime;
			ctrl->clipParam = ctrl->clipTime * clip->clipDurationInv;
		}
		return ctrl->frameIndex;
		
	}
	return -1;
}


//-----------------------------------------------------------------------------
