#include "GpuInfo.h"
#include "gl_core_3_3.h"

namespace gpuinfo
{
	int GetAvailableMemory()
	{
		int outData[4] = { 0, 0, 0, 0 };
		glGetIntegerv(GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, outData);
		glGetError();

		if (outData[0] != 0)
			return outData[0];

		glGetIntegerv(TEXTURE_FREE_MEMORY_ATI, outData);
		glGetError();

		return outData[0];
	}
}