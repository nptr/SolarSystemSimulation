#include "GpuInfo.h"
#include "gl_core_3_3.h"

const GLenum GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX = 0x9047;
const GLenum GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX = 0x9048;
const GLenum GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX = 0x9049;
const GLenum GPU_MEMORY_INFO_EVICTION_COUNT_NVX = 0x904A;
const GLenum GPU_MEMORY_INFO_EVICTED_MEMORY_NVX = 0x904B;

const GLenum VBO_FREE_MEMORY_ATI = 0x87FB;
const GLenum TEXTURE_FREE_MEMORY_ATI = 0x87FC;
const GLenum RENDERBUFFER_FREE_MEMORY_ATI = 0x87FD;

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