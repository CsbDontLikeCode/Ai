#pragma once
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

ImGuiIO& imguiSetupContext();

void imguiSetupPlatform(GLFWwindow* window);

void imguiStartFrame();

void imguiRendering(ImGuiIO& io);

void imguiTerminate();