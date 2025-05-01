#shader compute

#version 460 core

struct DrawElementsIndirectCommand
{
    uint count;
    uint instanceCount;
    uint firstIndex;
    uint baseVertex;
    uint baseInstance;
};

layout(std430, binding = 13) buffer DrawElementsIndirectCommandBlock
{
    DrawElementsIndirectCommand commandBuffer[];
};

// Assuming you need to process up to 1024 meshes (can be adjusted)
#define MAX_MESHES 1024
shared uint temp[MAX_MESHES * 2];

layout(local_size_x = MAX_MESHES) in;

void main() 
{
    uint tid = gl_LocalInvocationID.x;
    uint n = MAX_MESHES;
    
    // Load the instance counts into shared memory
    if (tid < n) {
        temp[tid] = commandBuffer[tid].instanceCount;
    } else {
        temp[tid] = 0;
    }
    
    barrier();
    
    // Up-sweep (reduction phase)
    for (uint stride = 1; stride < n; stride *= 2) {
        uint index = (tid + 1) * stride * 2 - 1;
        if (index < 2 * n) {
            temp[index] += temp[index - stride];
        }
        barrier();
    }
    
    // Clear the last element (important for exclusive scan)
    if (tid == 0) {
        temp[n * 2 - 1] = 0;
    }
    
    barrier();
    
    // Down-sweep phase
    for (uint stride = n; stride > 0; stride /= 2) {
        uint index = (tid + 1) * stride * 2 - 1;
        if (index < 2 * n) {
            uint t = temp[index];
            temp[index] += temp[index - stride];
            temp[index - stride] = t;
        }
        barrier();
    }
    
    // Write the results back to baseInstance field
    if (tid < n) {
        commandBuffer[tid].baseInstance = temp[tid];
    }
}