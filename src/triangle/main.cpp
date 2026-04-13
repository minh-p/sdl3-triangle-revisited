#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

SDL_Window *window;
SDL_GPUDevice *device;
SDL_GPUBuffer *vertexBuffer;
SDL_GPUTransferBuffer *transferBuffer;

struct Vertex {
    float x, y, z;
    float r, g, b, a;
};

static Vertex vertices[]{
    {0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},   // top vertex
    {-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f}, // bottom left vertex
    {0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f}   // bottom right vertex
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    window =
        SDL_CreateWindow("Hello, Triangle!", 960, 450, SDL_WINDOW_RESIZABLE);

    device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
    SDL_ClaimWindowForGPUDevice(device, window);

    SDL_GPUBufferCreateInfo bufferInfo{};
    bufferInfo.size = sizeof(vertices);
    vertexBuffer = SDL_CreateGPUBuffer(device, &bufferInfo);
    SDL_GPUTransferBufferCreateInfo transferBufferInfo{};
    transferBufferInfo.size = sizeof(vertices);
    transferBufferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferBufferInfo);

    // Map transfer buffer to a pointer
    // "Maps a transfer buffer into application address space"
    // Application address space refers to computer RAM accessible by this
    // program's CPU code.
    // This allows CPU to access memory otherwise managed by GPU!
    Vertex *data =
        (Vertex *)SDL_MapGPUTransferBuffer(device, transferBuffer, false);

    // Copy data from vertices struct to data.
    SDL_memcpy(data, vertices, sizeof(vertices));

    // We don't expect the triangle to change so we're just going to initiate
    // copy pass at the beginning of the program.
    SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(device);

    // Copy pass starts here before render pass
    // Transfer data from the transfer buffer to the vertex buffer.
    SDL_GPUCopyPass *copyPass = SDL_BeginGPUCopyPass(commandBuffer);

    // Where's the data to be uploaded
    SDL_GPUTransferBufferLocation location{};
    location.transfer_buffer = transferBuffer;
    // Begins from the starting byte of the transfer buffer.
    location.offset = 0;

    // Where should the data go to?
    SDL_GPUBufferRegion region{};
    region.buffer = vertexBuffer;
    region.size = sizeof(vertices);
    // Paste beginning from the starting byte of the vertex buffer
    region.offset = 0;

    // 4th parameter cycle: true
    // Cycling forces CPU to wait for GPU to complete its work before releasing
    // a lock on memory. Leave the block of memory the GPU is working alone and
    // use another block of memory for the CPU to work on This means
    // asynchronous operations!
    SDL_UploadToGPUBuffer(copyPass, &location, &region, true);
    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(commandBuffer);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    // A command buffer is a place to put GPU instructions.
    // It's a container for a long list of non-trivial data that tells the GPU
    // where and what to draw (position, color)
    SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(device);

    // Color target is data that tells the GPU where to draw, what to do with
    // previous target's content
    // and how to deal with new data.
    SDL_GPUColorTargetInfo colorTargetInfo{};

    colorTargetInfo.clear_color = {0 / 255.0f, 0 / 255.0f, 0 / 255.0f,
                                   0 / 255.0f};

    colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;

    // Contents generated will be written to memory.
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

    // Swapchain texture - working storage before its usage to the command
    // buffer.
    SDL_GPUTexture *swapchainTexture;
    Uint32 width, height;
    SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, window,
                                          &swapchainTexture, &width, &height);

    colorTargetInfo.texture = swapchainTexture;

    // render pass
    // The command buffer are composed of units called passes.
    // Of these passes, a copy pass uploads data to GPU like updating a buffer
    // or uploading texture A render pass then would raw something onto a color
    // target A compute pass is where heavy calculation can be done onto GPU
    // using compute shaders
    // A pass is begun and it is ended.
    // Passes cannot be sequentially used at the same time.
    SDL_GPURenderPass *renderPass =
        SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, NULL);

    SDL_EndGPURenderPass(renderPass);
    SDL_SubmitGPUCommandBuffer(commandBuffer);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    SDL_UnmapGPUTransferBuffer(device, transferBuffer);
    SDL_ReleaseGPUBuffer(device, vertexBuffer);
    SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
}
