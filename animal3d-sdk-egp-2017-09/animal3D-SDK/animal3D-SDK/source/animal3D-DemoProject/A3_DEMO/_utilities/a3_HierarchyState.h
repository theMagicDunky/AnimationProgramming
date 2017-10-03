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
	
	a3_HierarchyState.h
	Hierarchy transformation state.
*/

#ifndef __ANIMAL3D_HIERARCHYSTATE_H
#define __ANIMAL3D_HIERARCHYSTATE_H


// A3 hierarchy
#include "animal3D/a3animation/a3_Hierarchy.h"

// math library
#include "P3DM/P3DM.h"


//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#else	// !__cplusplus
	typedef struct a3_HierarchyState	a3_HierarchyState;
#endif	// __cplusplus

	
//-----------------------------------------------------------------------------

	// hierarchy state structure, with a pointer to a hierarchy (decoupled) 
	//	and transformations for kinematics
	struct a3_HierarchyState
	{
		// pointer to hierarchy tree
		const a3_Hierarchy *hierarchy;

		// local transformations (relative to parent's space)
		p3mat4 *localSpaceTransforms;

		// object transformations (relative to root's parent's space)
		p3mat4 *objectSpaceTransforms;
	};


//-----------------------------------------------------------------------------

	// initialize hierarchy state given an initialized hierarchy
	inline int a3hierarchyStateCreate(a3_HierarchyState *state_out, const a3_Hierarchy *hierarchy);

	// release hierarchy state
	inline int a3hierarchyStateRelease(a3_HierarchyState *state);


//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif	// __cplusplus


#endif	// !__ANIMAL3D_HIERARCHYSTATE_H