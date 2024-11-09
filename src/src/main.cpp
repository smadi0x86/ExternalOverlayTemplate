#include <Windows.h>
#include <dwmapi.h>
#include <d3d11.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>
#include <iostream>

/*
* This is a simple external ESP template that uses ImGui and DirectX 11.
* This template is meant to be used as a starting point for your own cheat.
* 
* For any function that seems strange or you don't understand, please refer to the MSDN documentation for full details.
*/

// Can be found inside the header file imgui_impl_win32.h
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void LogError(const char* message) {
	MessageBoxA(nullptr, message, "Error", MB_ICONERROR | MB_OK);
	std::cerr << "Error: " << message << std::endl;
}

// https://learn.microsoft.com/en-us/windows/win32/learnwin32/writing-the-window-procedure
LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam)) {
		return 0L;
	}
	if (message == WM_DESTROY) {
		PostQuitMessage(EXIT_SUCCESS);
		return 0L;
	}
	return DefWindowProc(window, message, wParam, lParam);
}

// https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-winmain
INT APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT cmdShow) {
	WNDCLASSEXW windowClass = { sizeof(WNDCLASSEXW), CS_CLASSDC, windowProcedure, 0L, 0L, hInstance, nullptr, nullptr, nullptr, nullptr, L"External ESP Template Class", nullptr };

	if (!RegisterClassExW(&windowClass)) {
		LogError("Failed to register window class.");
		return EXIT_FAILURE;
	}

	// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
	const HWND window = CreateWindowExW(
		WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
		windowClass.lpszClassName,
		L"External ESP Template",
		WS_POPUP,
		0, 0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),
		nullptr, nullptr, hInstance, nullptr
	);

	if (!window) {
		LogError("Failed to create window.");
		UnregisterClassW(windowClass.lpszClassName, hInstance);

		return EXIT_FAILURE;
	}

	SetLayeredWindowAttributes(window, RGB(0, 0, 0), 255, LWA_ALPHA);

	RECT clientArea;
	GetClientRect(window, &clientArea);
	RECT windowArea;
	GetWindowRect(window, &windowArea);
	POINT diff;
	ClientToScreen(window, &diff);
	MARGINS margins = { diff.x, diff.y, clientArea.right, clientArea.bottom };

	if (FAILED(DwmExtendFrameIntoClientArea(window, &margins))) {
		LogError("Failed to extend frame into client area.");
		DestroyWindow(window);
		UnregisterClassW(windowClass.lpszClassName, hInstance);
		return EXIT_FAILURE;
	}

	/*
	* Create a swap chain description to create a swap chain for the window.
	* The swap chain is used to present the rendered image to the screen.
	* 
	* https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nn-dxgi-idxgiswapchain
	*/
	DXGI_SWAP_CHAIN_DESC scd = {};

	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.SampleDesc.Count = 1;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 2;
	scd.OutputWindow = window;
	scd.Windowed = TRUE;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Create a D3D11 device, device context, swap chain, render target view, and get the feature level.
	// https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-create-swap-chain
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	IDXGISwapChain* swapChain = nullptr;
	ID3D11RenderTargetView* renderTargetView = nullptr;
	D3D_FEATURE_LEVEL featureLevel;
	constexpr D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

	if (FAILED(D3D11CreateDeviceAndSwapChain(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
		featureLevels, 2, D3D11_SDK_VERSION,
		&scd, &swapChain, &device, &featureLevel, &deviceContext))) {
		LogError("Failed to create D3D11 device and swap chain.");
		DestroyWindow(window);
		UnregisterClassW(windowClass.lpszClassName, hInstance);
		return EXIT_FAILURE;
	}

	// Get the back buffer from the swap chain and create a render target view from it.
	ID3D11Texture2D* backBuffer = nullptr;

	if (FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))) {
		LogError("Failed to get back buffer from swap chain.");
		swapChain->Release();
		device->Release();
		deviceContext->Release();
		DestroyWindow(window);
		UnregisterClassW(windowClass.lpszClassName, hInstance);
		return EXIT_FAILURE;
	}

	if (FAILED(device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView))) {
		LogError("Failed to create render target view.");
		backBuffer->Release();
		swapChain->Release();
		device->Release();
		deviceContext->Release();
		DestroyWindow(window);
		UnregisterClassW(windowClass.lpszClassName, hInstance);
		return EXIT_FAILURE;
	}
	// Release the back buffer after creating the render target view
	backBuffer->Release();

	// Show and update the window before creating the ImGui context
	ShowWindow(window, cmdShow);
	UpdateWindow(window);

	// Initialize ImGui and create a context
	ImGui::CreateContext();
	// Set the style of ImGui to classic (CTRL and press on the function name to see the available styles)
	ImGui::StyleColorsClassic();

	if (!ImGui_ImplWin32_Init(window) || !ImGui_ImplDX11_Init(device, deviceContext)) {
		LogError("Failed to initialize ImGui.");
		renderTargetView->Release();
		swapChain->Release();
		device->Release();
		deviceContext->Release();
		DestroyWindow(window);
		UnregisterClassW(windowClass.lpszClassName, hInstance);
		return EXIT_FAILURE;
	}

	bool running = true;

	// Loop until the window is closed
	while (running) {
		MSG message;
		while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
			if (message.message == WM_QUIT) running = false;
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		// Create a new frame for DX11 and win32  
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		// Create a new frame for ImGui
		ImGui::NewFrame();

		// DEBUG: Draw a red circle at (1000, 300)
		ImGui::GetBackgroundDrawList()->AddCircleFilled({ 1000, 300 }, 10.0f, ImColor(1.0f, 0.0f, 0.0f));

		/*
		* Cheat logic goes here then render...
		*
		* For example, for a kernel driver cheat you would:
		* - Run your cheat
		* - Talk to a driver
		* - Get the data you need
		* - Render the data
		*
		* For a usermode cheat you would:
		* - Run your cheat
		* - Get the data you need
		* - Render the data
		*/

		ImGui::Render();

		const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; 

		deviceContext->OMSetRenderTargets(1, &renderTargetView, nullptr);
		deviceContext->ClearRenderTargetView(renderTargetView, clearColor);

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		if (FAILED(swapChain->Present(1, 0))) {
			LogError("Failed to present the swap chain.");
			running = false;
		}
	}

	// Cleanup and destroy the ImGui context
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// Release the resources used by the swap chain, render target view, device, and device context before destroying the window
	if (renderTargetView) renderTargetView->Release();
	if (swapChain) swapChain->Release();
	if (device) device->Release();
	if (deviceContext) deviceContext->Release();

	// Destroy the window and unregister the window class
	DestroyWindow(window);
	UnregisterClassW(windowClass.lpszClassName, hInstance);

	return EXIT_SUCCESS;
}
