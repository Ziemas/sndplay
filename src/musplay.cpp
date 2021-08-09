#include "musplay.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <cstdio>
#include <fstream>
#include <memory>

std::fstream file;
SDL_Window *window;

int initGUI() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    const char *glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window = SDL_CreateWindow("musplay", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    glewInit();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    return 1;
}

int loadSongs() { return 0; }

int loadBank(char *filename) {

    file.open(filename, std::fstream::binary | std::fstream::in);
    file.seekg(0, std::fstream::beg);

    if (!file.is_open()) {
        printf("Failed to open file\n");
        return -1;
    }

    sbHeader header;
    file.read((char *)&header, sizeof(sbHeader));

    // idk if this is what is signified by this at all...
    if (header.Type != FileType::SBv2 && header.Type != FileType::SBlk) {
        printf("%s: Unsupported type: %d\n", filename, (int)header.Type);
        return -1;
    }

    u32 size = header.sbSize;
    if (header.version >= 3) {
        // Don't know why yet
        size += 4;
    }

    SoundBank *soundBank = (SoundBank *)malloc(size);
    if (soundBank == nullptr) {
        printf("malloc failed\n");
        return -1;
    }

    // The following seems pointless because we read into an offset pointer
    // and then never use the base ptr for anything again.
    /*
    u32 *dst = bankBuf;
     if (header.subVer == 3)
        dst = bankBuf + 1;
    file.read((char *)dst, size);
    */

    file.seekg(header.bankOffset, std::fstream::beg);
    file.read((char *)soundBank, header.sbSize);

    if (header.sampleSize == 0) {
        printf("%s: bank size %d\n", filename, header.sampleSize);
        printf("unhandled!\n");
        return 0;
    }

    u8 *sampleBuf = (u8 *)malloc(header.sampleSize);
    if (soundBank == nullptr) {
        printf("sample malloc failed\n");
        return -1;
    }

    file.seekg(header.sampleOffset, std::fstream::beg);
    file.read((char *)sampleBuf, header.sampleSize);

    free(sampleBuf);
    free(soundBank);

    file.close();

    return 0;
}

int main(int argc, char *argv[]) {
    if (!initGUI()) {
        printf("failed to init\n");
        return -1;
    }

    bool done = false;

    while (!done) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_MenuBar;
        bool open = true;

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::MenuItem("Open")) {
            }

            ImGui::EndMainMenuBar();
        }

        //// Main body of the Demo window starts here.
        // if (!ImGui::Begin("Dear ImGui Demo", &open, window_flags)) {
        //    // Early out if the window is collapsed, as an optimization.
        //    ImGui::End();
        //} else {

        //    ImGui::End();
        //}

        ImGui::ShowDemoWindow();

        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    return 0;
}
