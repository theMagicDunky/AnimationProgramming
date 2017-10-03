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
	
	a3_HierarchyState.c
	Implementation of transform hierarchy state.
*/

#include "a3_HierarchyState.h"

#include <stdlib.h>


//-----------------------------------------------------------------------------

// initialize hierarchy state given an initialized hierarchy
extern inline int a3hierarchyStateCreate(a3_HierarchyState *state_out, const a3_Hierarchy *hierarchy)
{
	// validate params and initialization states
	//	(output is not yet initialized, hierarchy is initialized)
	if (state_out && hierarchy && !state_out->hierarchy && hierarchy->nodes)
	{
		const unsigned int count = hierarchy->numNodes;
		const unsigned int count2 = count + count;
		unsigned int i;

		// set hierarchy pointer
		state_out->hierarchy = hierarchy;

		// allocate set of matrices in state
		//	('local' points to contiguous array of all matrices and floats)
		state_out->localSpaceTransforms = (p3mat4 *)malloc(count2 * sizeof(p3mat4) + count * sizeof(float));
		state_out->objectSpaceTransforms = (state_out->localSpaceTransforms + count);

		// set all matrices to identity
		for (i = 0; i < count2; ++i)
			p3real4x4SetIdentity(state_out->localSpaceTransforms[i].m);

		// return number of nodes
		return count;
	}
	return -1;
}


// release hierarchy state
extern inline int a3hierarchyStateRelease(a3_HierarchyState *state)
{
	// validate param exists and is initialized
	if (state && state->hierarchy)
	{
		// release matrices
		//	(local points to contiguous array of all matrices)
		free(state->localSpaceTransforms);

		// reset pointers
		state->hierarchy = 0;
		state->localSpaceTransforms = 0;
		state->objectSpaceTransforms = 0;

		// done
		return 1;
	}
	return -1;
}


//-----------------------------------------------------------------------------
