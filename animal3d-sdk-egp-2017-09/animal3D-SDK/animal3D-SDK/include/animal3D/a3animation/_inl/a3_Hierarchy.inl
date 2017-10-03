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
	
	a3_Hierarchy.inl
	Inline definitions for hierarchy.

	**DO NOT MODIFY THIS FILE**
*/

#ifdef __ANIMAL3D_HIERARCHY_H
#ifndef __ANIMAL3D_HIERARCHY_INL
#define __ANIMAL3D_HIERARCHY_INL


//-----------------------------------------------------------------------------

inline int a3hierarchyIsParentNode(const a3_Hierarchy *hierarchy, const unsigned int parentIndex, const unsigned int otherIndex)
{
	if (hierarchy && hierarchy->nodes && otherIndex < hierarchy->numNodes && parentIndex < hierarchy->numNodes)
		return (hierarchy->nodes[otherIndex].parentIndex == parentIndex);
	return -1;
}

inline int a3hierarchyIsChildNode(const a3_Hierarchy *hierarchy, const unsigned int childIndex, const unsigned int otherIndex)
{
	return a3hierarchyIsParentNode(hierarchy, otherIndex, childIndex);
}

inline int a3hierarchyIsSiblingNode(const a3_Hierarchy *hierarchy, const unsigned int siblingIndex, const unsigned int otherIndex)
{
	if (hierarchy && hierarchy->nodes && otherIndex < hierarchy->numNodes && siblingIndex < hierarchy->numNodes)
		return (hierarchy->nodes[otherIndex].parentIndex == hierarchy->nodes[siblingIndex].parentIndex);
	return -1;
}

inline int a3hierarchyIsAncestorNode(const a3_Hierarchy *hierarchy, const unsigned int ancestorIndex, const unsigned int otherIndex)
{
	unsigned int i = otherIndex;
	if (hierarchy && hierarchy->nodes && otherIndex < hierarchy->numNodes && ancestorIndex < hierarchy->numNodes)
	{
		while (i > ancestorIndex)
			i = hierarchy->nodes[i].parentIndex;
		return (i == ancestorIndex);
	}
	return -1;
}

inline int a3hierarchyIsDescendantNode(const a3_Hierarchy *hierarchy, const unsigned int descendantIndex, const unsigned int otherIndex)
{
	return a3hierarchyIsAncestorNode(hierarchy, otherIndex, descendantIndex);
}


//-----------------------------------------------------------------------------


#endif	// !__ANIMAL3D_HIERARCHY_INL
#endif	// __ANIMAL3D_HIERARCHY_H