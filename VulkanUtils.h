#pragma once

// Indices (locations) of Queue Families (if they exist at all)
struct QueueFamilyIndices
{
	// location of Graphics queue family
	int graphicsFamily = -1;

	// checks if families are valid
	bool isValid()
	{
		return graphicsFamily >= 0;
	}
};